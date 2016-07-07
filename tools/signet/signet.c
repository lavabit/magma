#include <unistd.h>
#include <getopt.h>

#include <dime/common/misc.h>
#include <dime/signet/keys.h>
#include <dime/signet/signet.h>

#include "symbols.h"

/**
 * @brief	Display the app options to the user.
 * @param	progname	Tool name.
*/
static void usage(const char *progname) {

	fprintf(stderr, "\nUsage: %s <-g signet_id> <-s signet_id> <-d signet_filename> [-o output_filename] [-k keys_filename] [-r ssr_filename] [-c custody_filename] <-x signet_type>\n", progname);
	fprintf(stderr, "  --generate|-g   Uses provided signet id to generate a signet or SSR (signet signing request) and a keys files, may be used with a -c option to create an SSR for rotation of user signets.\n");
	fprintf(stderr, "  --sign    |-s   Signs an SSR file specifed by the -r option with an org signing key specified by -k option and creates a user signet file.\n");
	fprintf(stderr, "  --dump    |-d   Specifies filename of a signet file, the contents of which are dumped to the terminal.\n");
	fprintf(stderr, "  --keys    |-k   Specifies an organizational .keys file used to sign the user signet signing request.\n");
	fprintf(stderr, "  --ssr     |-r   Specifies the SSR filename to be signed.\n");
	fprintf(stderr, "  --custody |-c   Specifies the custody keys filename used to create the user signet's chain of custody signature.\n");
	fprintf(stderr, "  --examine |-x   Lists all field types for the specified signet type (user, org, ssr).\n");
	fprintf(stderr, "  --help    |-h   Help, currently limited to this usage message.\n");
}


/**
 * @brief	Prompt the user for a line of input from the console.
 * @param	prompt		an optional NULL-terminated string containing a prompt message for the user.
 * @param	buf		a pointer to a buffer into which the user's input will be read.
 * @param	bufsize		the size, in bytes, of the user input buffer.
*/
static void wizard_get_input(const char *prompt, char *buf, size_t bufsize) {

	if (!buf || !bufsize) {
		fprintf(stderr, "Error: No buffer specified, or buffer size is 0.\n");
		exit(EXIT_FAILURE);
	}

	if (prompt) {
		fprintf(stdout, "%s ", prompt);
		fflush(stdout);
	}

	memset(buf, 0, bufsize);

	if (!fgets(buf, bufsize - 1, stdin)) {
		if (errno == 0) {
			printf("\n");
		} else {
			perror("fgets");
		}
		exit(EXIT_FAILURE);
	}

	buf[strcspn(buf, "\n")] = '\0';
}

/**
 * @brief	Wizard for generating new signet. Creates signet and keys files.
 * @param	signet_name	NULL terminated string containing signet id.
 * @param	signet_file	NULL terminated string containing preferred signet filename, if NULL default filename is used.
 * @param	keys_file	NULL terminated string containing preferred keys filename, if NULL default keysname is used.
 * @param	old_keys	NULL terminated string containing the filename to the previous keys file required to perform the chain of custody signing.
 *                              If NULL, no chain of custody signing occurs, if not NULL and desired signet is an org signet, returns an error.
*/
static void generate_signet(const char *signet_name, const char *signet_file, const char *keys_file, const char *old_keys) {

	char *domain, wizard_string[256], *signet_f = NULL, *keys_f = NULL;
	int keys_alloc = 0, signet_alloc = 0;
	ED25519_KEY *key, *oldkey;
	signet_t *signet;
	signet_type_t type;

	if(!signet_name) {
		fprintf(stderr, "generate_signet requires signet_name.\n");
		exit(EXIT_FAILURE);
	}

	if((domain = strchr(signet_name, '@'))) {
		domain++;
		type = SIGNET_TYPE_SSR;
	} else {
		type = SIGNET_TYPE_ORG;
		domain = (char *)signet_name;
	}

	if(type != SIGNET_TYPE_SSR && old_keys) {
		fprintf(stderr, "you do not need to specify the old keys to rotate a non-user signet.");
		exit(EXIT_FAILURE);
	}

	if(!keys_file) {

		keys_alloc = 1;
		if(str_printf(&keys_f, "%s.keys", signet_name) < 0) {
			fprintf(stderr, "could not concatenate strings");
			exit(EXIT_FAILURE);
		}

		if(type == SIGNET_TYPE_SSR) {
			keys_f[(domain - 1 - signet_name)] = '-';
		}
	} else {
		keys_f = (char *)keys_file;
	}

	if(!(signet = dime_sgnt_signet_create_w_keys(type, keys_f))) {
		fprintf(stderr, "Could not create new signet signet.\n");

		if(keys_alloc) {
			free(keys_f);
		}

		exit(EXIT_FAILURE);
	}

	if(!(key = dime_keys_signkey_fetch(keys_f))) {
		fprintf(stderr, "Could not retrieve signing key from keys file %s.\n", keys_f);

		if(keys_alloc) {
			free(keys_f);
		}

		exit(EXIT_FAILURE);
	}

	if(keys_alloc) {
		free(keys_f);
	}

	dime_sgnt_sig_crypto_sign(signet, key);

	switch(type) {
	case SIGNET_TYPE_ORG:

		wizard_get_input("Organization name:", wizard_string, sizeof(wizard_string));
		dime_sgnt_field_defined_create(signet, SIGNET_ORG_NAME, strlen(wizard_string), (const unsigned char *)wizard_string);

		wizard_get_input("Organization address:", wizard_string, sizeof(wizard_string));
		dime_sgnt_field_defined_create(signet, SIGNET_ORG_ADDRESS, strlen(wizard_string), (const unsigned char *)wizard_string);

		wizard_get_input("Organization country:", wizard_string, sizeof(wizard_string));
		dime_sgnt_field_defined_create(signet, SIGNET_ORG_COUNTRY, strlen(wizard_string), (const unsigned char *)wizard_string);

		wizard_get_input("Organization postal code:", wizard_string, sizeof(wizard_string));
		dime_sgnt_field_defined_create(signet, SIGNET_ORG_POSTAL, strlen(wizard_string), (const unsigned char *)wizard_string);

		wizard_get_input("Organization phone number:", wizard_string, sizeof(wizard_string));
		dime_sgnt_field_defined_create(signet, SIGNET_ORG_PHONE,  strlen(wizard_string), (const unsigned char *)wizard_string);

		dime_sgnt_sig_full_sign(signet, key);
		dime_sgnt_id_set(signet, strlen(signet_name), (const unsigned char *)signet_name);
		dime_sgnt_sig_id_sign(signet, key);
		break;
	case SIGNET_TYPE_SSR:

		if(old_keys) {

			if(!(oldkey = dime_keys_signkey_fetch(old_keys))) {
				fprintf(stderr, "Could not retrieve the signing key from the keys file.\n");
				free_ed25519_key(key);
				dime_sgnt_signet_destroy(signet);
				exit(EXIT_FAILURE);
			}

			dime_sgnt_sig_coc_sign(signet, oldkey);
			free_ed25519_key(oldkey);
		}

		dime_sgnt_sig_ssr_sign(signet, key);
		break;
	case SIGNET_TYPE_USER:
		fprintf(stderr, "To create a user signet, an organization needs to sign an ssr.\n");
		free_ed25519_key(key);
		exit(EXIT_FAILURE);
	default:
		fprintf(stderr, "Invalid signet type.\n");
		free_ed25519_key(key);
		exit(EXIT_FAILURE);
	}

	free_ed25519_key(key);

	if(!signet_file) {
		signet_alloc = 1;

		if(type == SIGNET_TYPE_SSR) {

			if(str_printf(&signet_f, "%s.ssr", signet_name) < 0) {
				fprintf(stderr, "Could not concatenate strings.\n");
				exit(EXIT_FAILURE);
			}
			signet_f[(domain - 1 - signet_name)] = '-';

		} else if (type == SIGNET_TYPE_ORG) {

			if(str_printf(&signet_f, "%s.signet", signet_name) < 0) {
				fprintf(stderr, "Could not concatenate strings.\n");
				exit(EXIT_FAILURE);
			}
		}
	} else {
		signet_f = (char *)signet_file;
	}

	if(dime_sgnt_file_create(signet, signet_f) < 0) {
		fprintf(stderr, "Could not store signet in file.\n");

		if(signet_alloc) {
			free(signet_f);
		}

		dime_sgnt_signet_destroy(signet);
		exit(EXIT_FAILURE);
	}

	if(signet_alloc) {
		free(signet_f);
	}

	dime_sgnt_signet_destroy(signet);
}

/**
 * @brief	Signs an existing ssr with organizational pok to create a user signet.
 * @param	signet_name	Signet Id to be given to the new user signet.
 * @param	ssr_f		Specified ssr filename, required for signing to occur.
 * @param	keys_f		Specified org keys file, required for signing to occur.
 * @param	signet_file	Filename for the output signet, if NULL default name based on signet_name is used.
*/
static void sign_signet(const char *signet_name, const char *ssr_f, const char *keys_f, const char *signet_file) {

	int signet_alloc = 0;
	char *signet_f = NULL, wizard_string[256], *domain = NULL;
	ED25519_KEY *key;
	signet_t *signet;

	if(!signet_name) {
		fprintf(stderr, "No signet id.\n");
		exit(EXIT_FAILURE);
	}

	if(!keys_f) {
		fprintf(stderr, "No key filename.\n");
		exit(EXIT_FAILURE);
	}

	if(!ssr_f) {
		fprintf(stderr, "No signet filename.\n");
		exit(EXIT_FAILURE);
	}

	if((domain = strchr(signet_name, '@'))) {
		domain++;
	} else {
		fprintf(stderr, "You must provide the signet name of the user signet.\n");
		exit(EXIT_FAILURE);
	}

	if(!(signet = dime_sgnt_signet_load(ssr_f))) {
		fprintf(stderr, "Could not load signet from specified file: %s\n", ssr_f);
		exit(EXIT_FAILURE);
	}

	if(dime_sgnt_validate_all(signet, NULL, NULL, NULL) != SS_SSR) {
		fprintf(stderr, "The signet is not a valid SSR.\n");
		dime_sgnt_signet_destroy(signet);
		exit(EXIT_FAILURE);
	}

	if(!(key = dime_keys_signkey_fetch(keys_f))) {
		fprintf(stderr, "Could not retrieve the signing key from the keys binary.\n");
		dime_sgnt_signet_destroy(signet);
		exit(EXIT_FAILURE);
	}

	dime_sgnt_sig_crypto_sign(signet, key);
	wizard_get_input("User name:", wizard_string, sizeof(wizard_string));
	dime_sgnt_field_defined_create(signet, SIGNET_USER_NAME, strlen(wizard_string), (const unsigned char *)wizard_string);
	wizard_get_input("User address:", wizard_string, sizeof(wizard_string));
	dime_sgnt_field_defined_create(signet, SIGNET_USER_ADDRESS, strlen(wizard_string), (const unsigned char *)wizard_string);
	wizard_get_input("User country:", wizard_string, sizeof(wizard_string));
	dime_sgnt_field_defined_create(signet, SIGNET_USER_COUNTRY, strlen(wizard_string), (const unsigned char *)wizard_string);
	wizard_get_input("User postal code:", wizard_string, sizeof(wizard_string));
	dime_sgnt_field_defined_create(signet, SIGNET_USER_POSTAL, strlen(wizard_string), (const unsigned char *)wizard_string);
	wizard_get_input("User phone number:", wizard_string, sizeof(wizard_string));
	dime_sgnt_field_defined_create(signet, SIGNET_USER_PHONE, strlen(wizard_string), (const unsigned char *)wizard_string);
	dime_sgnt_sig_full_sign(signet, key);
	dime_sgnt_id_set(signet, strlen(signet_name), (const unsigned char *)signet_name);
	dime_sgnt_sig_id_sign(signet, key);

	_free_ed25519_key(key);

	if(!signet_file) {
		signet_alloc = 1;

		if(str_printf(&signet_f, "%s.signet", signet_name) < 0) {
			fprintf(stderr, "Could not concatenate strings.\n");
			dime_sgnt_signet_destroy(signet);
			exit(EXIT_FAILURE);
		}

		signet_f[domain - 1 - signet_name] = '-';
	} else {
		signet_f = (char *)signet_file;
	}

	if(dime_sgnt_file_create(signet, signet_f) < 0) {
		fprintf(stderr, "Could not store signet in file.\n");
		dime_sgnt_signet_destroy(signet);

		if(signet_alloc) {
			free(signet_f);
		}

		exit(EXIT_FAILURE);
	}

	dime_sgnt_signet_destroy(signet);

	if(signet_alloc) {
		free(signet_f);
	}
}

/**
 * @brief	Dumps signet or ssr.
 * @param	signet_file	Filename of the signet to be dumped.
*/
static void dump_signet(const char *signet_file) {                      // TODO needs to dump in HEX format rather than b64

	char *fingerprint, *signet_f = NULL;
	signet_type_t type;
	signet_t *signet;

	if(!signet_file) {
		fprintf(stderr, "No signet file specified.\n");
		exit(EXIT_FAILURE);
	}

	if(!(signet = dime_sgnt_signet_load(signet_file))) {
		fprintf(stderr, "Could not load signet from specified file: %s\n", signet_f);
		exit(EXIT_FAILURE);
	}

	dime_sgnt_signet_dump(stdout, signet);
	fprintf(stderr, "-------------------------------------------------------------------------------------------------------------------------------\n");
	type = dime_sgnt_type_get(signet);

	if(type == SIGNET_TYPE_SSR || type == SIGNET_TYPE_USER) {
		fingerprint = dime_sgnt_fingerprint_ssr(signet);
		fprintf(stderr, "%-34.34s :  %s\n", "*** SSR fingerprint", fingerprint);
		free(fingerprint);
	}

	if(type == SIGNET_TYPE_USER || type == SIGNET_TYPE_ORG) {
		fingerprint = dime_sgnt_fingerprint_crypto(signet);
		fprintf(stderr, "%-34.34s :  %s\n", "*** Cryptographic fingerprint", fingerprint);
		free(fingerprint);
	}

	if(type == SIGNET_TYPE_USER || type == SIGNET_TYPE_ORG) {
		fingerprint = dime_sgnt_fingerprint_full(signet);
		fprintf(stderr, "%-34.34s :  %s\n", "*** Full fingerprint", fingerprint);
		free(fingerprint);
		fingerprint = dime_sgnt_fingerprint_id(signet);
		fprintf(stderr, "%-34.34s :  %s\n", "*** ID fingerprint", fingerprint);
		free(fingerprint);
	}

	dime_sgnt_signet_destroy(signet);
}

/**
 * @brief	Dumps field names and descriptions for the specified signet type.
 * @param	type	Signet type to be examined.
*/
static void examine_signet(signet_type_t type) {

	const char *strtype;
	signet_field_key_t *keys;

	switch(type) {

	case SIGNET_TYPE_ORG:
		keys = signet_org_field_keys;
		strtype = "Organizational signet";
		break;
	case SIGNET_TYPE_USER:
		keys = signet_user_field_keys;
		strtype = "User signet";
		break;
	case SIGNET_TYPE_SSR:
		keys = signet_ssr_field_keys;
		strtype = "SSR";
		break;
	default:
		fprintf(stderr, "Invalid signet type specified to be examined.\n");
		exit(EXIT_FAILURE);
		break;
	}

	fprintf(stdout, "\n%s field types:\n", strtype);

	for(int i = 0; i < SIGNET_FID_MAX; ++i) {

		if(keys[i].name) {
			fprintf(stdout, "--- %-*d %-30.30s -> %-90.90s\n", 3, i, keys[i].name, keys[i].description);
		}
	}

}

int main(int argc, char **argv) {

	char *signet_id = NULL, *dump_file = NULL, *out_file = NULL, *ssr_file = NULL, *keys_file = NULL, *custody_file = NULL, *examine_type = NULL;
	int is_help = 0, opt, opt_index = 0;;
	typedef enum {GENERATE = 0, DUMP, SIGN, EXAMINE, NONE} COMMAND_T;
	COMMAND_T command = NONE;
	signet_type_t type;

	static struct option long_options[] = {
		{"generate", required_argument, NULL, 'g'},
		{"sign", required_argument, NULL, 's'},
		{"dump", required_argument, NULL, 'd'},
		{"output", required_argument, NULL, 'o'},
		{"keys", required_argument, NULL, 'k'},
		{"ssr", required_argument, NULL, 'r'},
		{"custody", required_argument, NULL, 'c'},
		{"examine", required_argument, NULL, 'x'},
		{"help", no_argument, NULL, 'h'},
		{0, 0, 0, 0}
	};

	if(argc == 1) {
		usage(argv[0]);
		return 0;
	}

	while((opt = getopt_long(argc, argv, "g:s:d:o:k:r:c:x:h", long_options, &opt_index)) != -1) {

		switch(opt) {

		case 'g':

			if(command != NONE) {
				fprintf(stderr, "Conflicting options (generate, sign, dump)\n");
				usage(argv[0]);
				return 0;
			}

			signet_id = optarg;
			command = GENERATE;
			break;
		case 's':

			if(command != NONE) {
				fprintf(stderr, "Conflicting options (generate, sign, dump)\n");
				usage(argv[0]);
				return 0;
			}

			signet_id = optarg;
			command = SIGN;
			break;
		case 'd':

			if(command != NONE) {
				fprintf(stderr, "Conflicting options (generate, sign, dump)\n");
				usage(argv[0]);
				return 0;
			}

			dump_file = optarg;
			command = DUMP;
			break;
		case 'o':

			if(out_file) {
				fprintf(stderr, "More than a single signet filename specified.\n");
				usage(argv[0]);
				return 0;
			}

			out_file = optarg;
			break;
		case 'k':

			if(keys_file) {
				fprintf(stderr, "More than a single keys filename specified.\n");
				usage(argv[0]);
				return 0;
			}

			keys_file = optarg;
			break;
		case 'r':

			if(ssr_file) {
				fprintf(stderr, "More than a single ssr filename specified.\n");
				usage(argv[0]);
			}

			ssr_file = optarg;
			break;
		case 'c':

			if(custody_file) {
				fprintf(stderr, "More than a single custody keys filename specified.\n");
				usage(argv[0]);
				return 0;
			}

			custody_file = optarg;
			break;
		case 'x':

			if(examine_type) {
				fprintf(stderr, "Only one signet type can be specified at a time to be examined.\n");
				usage(argv[0]);
				return 0;
			}

			examine_type = optarg;
			command = EXAMINE;
			break;
		case 'h':
			is_help = 1;
			break;
		default:
			fprintf(stderr, "Invalid option. Use -h for help.\n");
			break;

		}
	}

	if(is_help) {
		usage(argv[0]);
		return 0;
	}

	// Load the OpenSSL symbols used by libdime.
	if (lib_load()) {
		fprintf(stderr, "Error: unable to bind the program to the required dynamic symbols.\n");
		exit(EXIT_FAILURE);
	}

	crypto_init();

	switch(command) {

	case GENERATE:

		if(ssr_file) {
			fprintf(stderr, "You should not specify an ssr file to generate a signet. Use --output [-o] for output signet.\n");
			usage(argv[0]);
			return 0;
		}

		generate_signet(signet_id, out_file, keys_file, custody_file);
		break;
	case SIGN:

		if(!ssr_file) {
			fprintf(stderr, "No ssr filename specified for signing.\n");
			usage(argv[0]);
			return 0;
		}

		if(!keys_file) {
			fprintf(stderr, "No org keys file specified to sign the ssr.\n");
			usage(argv[0]);
			return 0;
		}

		if(custody_file) {
			fprintf(stderr, "You should not specify custody keys file to sign an ssr. Use --gen [-g] to create an ssr signed with custody keys.\n");
			usage(argv[0]);
			return 0;
		}

		sign_signet(signet_id, ssr_file, keys_file, out_file);
		break;
	case DUMP:

		if(ssr_file) {
			fprintf(stderr, "You should not specify an ssr file to dump a signet, use --dump [-d] <filename> to specify signet file.\n");
			usage(argv[0]);
			return 0;
		}

		if(out_file) {
			fprintf(stderr, "Dump command does not use an output file option.\n");
			usage(argv[0]);
			return 0;
		}

		if(keys_file) {
			fprintf(stderr, "Dump command does not use keys file option.\n");
			usage(argv[0]);
			return 0;
		}

		if(custody_file) {
			fprintf(stderr, "Dump command does not use custody keys file option.\n");
			usage(argv[0]);
			return 0;
		}

		dump_signet(dump_file);
		break;
	case EXAMINE:

		if(!strcmp(examine_type, "org")) {
			type = SIGNET_TYPE_ORG;
		} else if (!strcmp(examine_type, "user")) {
			type = SIGNET_TYPE_USER;
		} else if (!strcmp(examine_type, "ssr")) {
			type = SIGNET_TYPE_SSR;
		} else {
			fprintf(stderr, "Invalid signet type specified to be examined. Use one of the following: 'org' 'user' 'ssr'.\n");
			usage(argv[0]);
			return 0;
		}

		examine_signet(type);
		break;
	default:
		fprintf(stderr, "Invalid command specified. This tool can be used to generate (-g), sign (-s) and dump (-d) signets.\n");
		usage(argv[0]);
		return 0;
		break;

	}

	return 0;
}
