#include <dime/signet-resolver/cache.h>
#include <dime/signet-resolver/dmtp.h>
#include <dime/signet-resolver/dns.h>
#include <dime/signet-resolver/mrec.h>
#include <dime/signet-resolver/signet-ssl.h>

#include <dime/signet/keys.h>
#include <dime/signet/signet.h>

#include <dime/common/misc.h>
#include <dime/common/network.h>

#include "symbols.h"

static void __attribute__((noreturn)) usage(const char *progname)  {

	fprintf(stderr, "\nUsage: %s [-d dxserver] [-p port] [-i dimefile [-0] [-f fingerprint] [-h or -c] [-e endfp] [-n] [-4 or -6] [-v] <signet>    where\n", progname);
	fprintf(stderr, " signet is the name of the user or organizational signet to be looked up.\n");
	fprintf(stderr, " -h   looks up the chain of custody for the specified signet using the HIST command (requires -f).\n");
	fprintf(stderr, " -e   can be used with [-h] to specify an optional ending fingerprint for the chain of custody request.\n");
	fprintf(stderr, " -c   verifies that a particular signet is current using the VRFY command (requires -f).\n");
	fprintf(stderr, " -d   specifies an optional DX server hostname to be used instead of the one found from the DIME record.\n");
	fprintf(stderr, " -p   forces a DX port to be used (only %u [ssl] or %u [dual mode] are allowed values).\n", DMTP_PORT, DMTP_PORT_DUAL);
	fprintf(stderr, " -i   reads the DIME management record from the specified file and skips the DIME resolver process.\n");
	fprintf(stderr, " -0   when used with the -i option, the file-supplied DIME record is not treated as trusted, +dnssec validated data.\n");
	fprintf(stderr, " -f   can specifiy an optional signet fingerprint to be used for a standard signet lookup, or\n");
	fprintf(stderr, "      the mandatory fingerprint needed for a history or verify signet operation.\n");
	fprintf(stderr, " -n   disables use of the persistent object cache.\n");
	fprintf(stderr, " -4   forces ipv4 address resolution (-6 for ipv6).\n");
	fprintf(stderr, " -v   turns on verbose output (-v can be specified multiple times to increase the debugging level).\n");
	fprintf(stderr, "\n");

	exit(EXIT_FAILURE);
}

static void show_coc(const char *cocstr) {

	signet_t *signet;
	char *datastr, *tokens, *token, *dptr;
	size_t i = 1;

	if (!cocstr || (*cocstr != '[')) {
		fprintf(stderr, "Error: could not parse chain of custody response.\n");
		return;
	}

	if (!(datastr = strdup(cocstr))) {
		perror("strdup");
		fprintf(stderr, "Error: could not display signet chain of custody.\n");
		return;
	}

	if (!(token = strtok_r(datastr, "\n", &tokens))) {
		free(datastr);
		fprintf(stderr, "Error: could not parse chain of custody response.\n");
		return;
	}

	fprintf(stderr, "Signet history:\n");

	while (token) {
		dptr = token;

		while (chr_isspace(*dptr)) {
			dptr++;
		}

		if (!strcasecmp(dptr, "OK")) {
			fprintf(stdout, "--- End chain of custody.\n");
			break;
		}

		if (*dptr != '[') {
			free(datastr);
			fprintf(stderr, "Error: unexpected leading data encountered while parsing chain of custody response.\n");
			return;
		}

		token = dptr + 1;
		dptr = token + strlen(token) - 1;

		while ((dptr > token) && chr_isspace(*dptr)) {
			dptr--;
		}

		if ((dptr <= token) || (*dptr != ']')) {
			free(datastr);
			fprintf(stderr, "Error: unexpected trailing data encountered while parsing chain of custody response.\n");
			return;
		}

		*dptr = 0;

		fprintf(stdout, "%zu. %s\n", i, token);

		if (!(signet = dime_sgnt_signet_b64_deserialize(token))) {
			fprintf(stderr, "Error: could not deserialize signet.\n");
			dump_error_stack();
		} else {
			dime_sgnt_signet_dump(stdout, signet);
		}

		token = strtok_r(NULL, "\n", &tokens);
		i++;
	}

	free(datastr);

}

int main(int argc, char *argv[]) {

	dime_record_t *drec;
	dmtp_session_t *session;
	signet_t *signet, *org_signet = NULL;
	cached_object_t *obj;
	char *dxname = NULL, *dimefile = NULL, *fingerprint = NULL, *endfp = NULL, *newprint = NULL, *signame, *domain, *line;
	unsigned long ttl = 0;
	unsigned short port = 0;
	int opt, is_org = 0, family = 0, do_hist = 0, do_vrfy = 0, no_trust = 0, no_cache = 0, result, vres;

	while ((opt = getopt(argc, argv, "046d:e:f:i:hcnp:v")) != -1) {

		switch (opt) {
		case '0':
			no_trust = 1;
			break;
		case '4':
			family = AF_INET;
			break;
		case '6':
			family = AF_INET6;
			break;
		case 'c':
			do_vrfy = 1;
			break;
		case 'd':
			dxname = optarg;
			break;
		case 'e':
			endfp = optarg;
			break;
		case 'f':
			fingerprint = optarg;
			break;
		case 'i':
			dimefile = optarg;
			break;
		case 'h':
			do_hist = 1;
			break;
		case 'n':
			no_cache = 1;
			break;
		case 'p':

			if (!(port = atoi(optarg))) {
				fprintf(stderr, "Error: specified invalid port number.\n");
				exit(EXIT_FAILURE);
			}

			if ((port != DMTP_PORT) && (port != DMTP_PORT_DUAL)) {
				fprintf(stderr, "Error: invalid port specified; must be %u or %u.\n", DMTP_PORT, DMTP_PORT_DUAL);
				exit(EXIT_FAILURE);
			}

			break;
		case 'v':
			_verbose++;
			break;
		default:
			usage(argv[0]);
			break;
		}

	}

	// Load the OpenSSL symbols used by libdime.
	if (lib_load()) {
		fprintf(stderr, "Error: unable to bind the program to the required dynamic symbols.\n");
		exit(EXIT_FAILURE);
	}

	if (no_cache) {

		dbgprint(1, "Disabling object cache.\n");

		// We need to be able to use the cache, but not to load or save to disk.
		if (set_cache_permissions(CACHE_PERM_READ | CACHE_PERM_ADD | CACHE_PERM_DELETE) < 0) {
			fprintf(stderr, "Error: could not adjust cache permissions.\n");
			dump_error_stack();
			exit(EXIT_FAILURE);
		}

	}

	if (argc == 1) {
		usage(argv[0]);
	} else if (argc == optind) {
		fprintf(stderr, "Error: no signet name was specified!\n");
		exit(EXIT_FAILURE);
	}

	if (do_hist && do_vrfy) {
		fprintf(stderr, "Error: -h or -c cannot be specified together. Please choose one.\n");
		exit(EXIT_FAILURE);
	}

	if (do_hist && !fingerprint) {
		fprintf(stderr, "Error: the -h option requires a signet fingerprint to be supplied with -f.\n");
		exit(EXIT_FAILURE);
	} else if (do_vrfy && !fingerprint) {
		fprintf(stderr, "Error: the -c option requires a signet fingerprint to be supplied with -f.\n");
		exit(EXIT_FAILURE);
	}

	if (endfp && !do_hist) {
		fprintf(stderr, "Error: the -e option can only be specified together with -h.\n");
		exit(EXIT_FAILURE);
	}

	if (no_trust && !dimefile) {
		fprintf(stderr, "Error: the -0 option can only be used in conjunction with -i.\n");
		exit(EXIT_FAILURE);
	}

	// Are we a user or organizational signet?
	signame = argv[optind];

	if ((domain = strchr(signame, '@'))) {
		domain++;
	} else {
		is_org = 1;
		domain = signame;
	}

	if (is_org && do_hist) {
		fprintf(stderr, "Error: chain of custody query is not allowed for org signets.\n");
		exit(EXIT_FAILURE);
	}

	dbgprint(1, "Running with verbose = %u ...\n", _verbose);
	dbgprint(1, "Performing query on %s signet.\n", (is_org ? "organizational" : "user"));

	if (load_cache_contents() < 0) {
		fprintf(stderr, "Error: unable to load cache contents from disk.\n");
		dump_error_stack();
	} else if (_verbose >= 5) {
		dbgprint(5, "Loaded object cache; dumping contents:\n");
		_dump_cache(cached_data_unknown, 1, 1);
	}

	if ((obj = find_cached_object(signame, &(cached_stores[cached_data_signet])))) {
		dbgprint(1, "Found signet data in cache.\n");

		// If all we're doing is querying the signet, we can do that now and exit.
		if (!(do_hist || do_vrfy)) {
			dime_sgnt_signet_dump(stdout, obj->data);
			exit(EXIT_SUCCESS);
		}

	}

	printf("Querying DIME management record for: %s\n", domain);

	if (dimefile) {

		dbgprint(1, "Loading DIME management record from file: %s\n", dimefile);

		if (!(drec = get_dime_record_from_file(dimefile, domain))) {
			fprintf(stderr, "Failed to retrieve DIME management record from file.\n");
			dump_error_stack();
			exit(EXIT_FAILURE);
		}

		// Since loaded locally, it is trusted with absolute authority by default.
		drec->validated = no_trust ? 0 : 1;

	} else {

		if (!(drec = get_dime_record(domain, &ttl, 1))) {
			fprintf(stderr, "Failed to query DIME management record.\n");
			dump_error_stack();

			// ?? Keep this?
			if (save_cache_contents() < 0) {
				fprintf(stderr, "Error: unable to save cache contents to disk.\n");
				dump_error_stack();
			}

			exit(EXIT_FAILURE);
		}

	}

	if (save_cache_contents() < 0) {
		fprintf(stderr, "Error: unable to save cache contents to disk.\n");
		dump_error_stack();
	}

	if (_verbose >= 3) {
		_dump_dime_record_cb(stderr, drec, 0);
	}

	if (family == AF_INET) {
		dbgprint(0, "Forcing connection to DX server over IPv4.\n");
	} else if (family == AF_INET6) {
		dbgprint(0, "Forcing connection to DX server over IPv6.\n");
	}

	// Now we need to connect to the DX server for the dark domain.
	// We can either use the command line input, if a DX server is explicitly provided, or use the normal route.
	if (dxname) {
		// If no port was specified, we assume the default DMTP port.
		if (!port) {
			port = DMTP_PORT;
		}

		fprintf(stderr, "Connecting to DX at %s:%d ...\n", dxname, port);

		// Only the dual mode port is considered an SSL service.
		if (port == DMTP_PORT_DUAL) {
			session = dx_connect_dual(dxname, domain, family, drec, 0);
		} else {
			session = dx_connect_standard(dxname, domain, family, drec);
		}

	} else {
		fprintf(stderr, "Establishing connection to DX server...\n");
		session = sgnt_resolv_dmtp_connect(domain, family);
	}

	if (!session) {
		fprintf(stderr, "Error: could not connect to DX server.\n");
		dump_error_stack();
		exit(EXIT_FAILURE);
	}

	printf("DX connection succeeded.\n");

	if ((vres = verify_dx_certificate(session)) < 0) {
		fprintf(stderr, "Error: an error was encountered during the DX certificate verification process.\n");
		dump_error_stack();
		exit(EXIT_FAILURE);
	} else if (!vres) {
		fprintf(stderr, "Error: DX certificate verification failed.\n");
		exit(EXIT_FAILURE);
	}

	dbgprint(1, "DX certificate successfully verified.\n");

/*	printf("Executing command tests...\n");
        printf("STAT returned %s\n", dmtp_stats(session, NULL));
        printf("MODE returned %d\n", dmtp_get_mode(session));
        printf("NOOP returned %d\n", dmtp_noop(session));
        printf("RSET returned %d\n", dmtp_reset(session));
        printf("Done.\n");

printf("XXX: aborting\n");
        _dump_cache(cached_data_unknown, 1);
exit(0); */

	// Depending on the command line options supplied, there's 3 things we can do: signet resolution, signet verification, or signet history.
	if (do_vrfy) {
		dbgprint(1, "Attempting to verify fingerprint (%s) for signet: %s\n", fingerprint, signame);

		if ((result = sgnt_resolv_dmtp_verify_signet(session, signame, fingerprint, &newprint)) < 0) {
			fprintf(stderr, "Error: signet verification failed.\n");
			dump_error_stack();
			sgnt_resolv_destroy_dmtp_session(session);
			exit(EXIT_FAILURE);
		}

		if (result) {
			printf("Signet fingerprint is current.\n");
		} else {
			printf("Signet fingerprint is out of date. The most recent fingerprint is: %s\n", newprint);
		}

		sgnt_resolv_destroy_dmtp_session(session);
		return 0;
	} else if (do_hist) {
		dbgprint(1, "Attempting to retrieve the CoC history between (%s) and (%s) for signet: %s\n", fingerprint,
		         (endfp ? endfp : "[current]"), signame);

		if (!(line = sgnt_resolv_dmtp_history(session, signame, fingerprint, endfp))) {
			fprintf(stderr, "Signet history command failed.\n");
		} else {
			show_coc(line);
		}

		sgnt_resolv_destroy_dmtp_session(session);
		return 0;
	} else {
		dbgprint(1, "Attempting to retrieve data for signet: %s\n", signame);

		// If this is a user signet, we always need to retrieve the org signet first.
		if (!is_org) {
			dbgprint(1, "Fetching org signature for domain: %s\n", domain);

			if (!(line = sgnt_resolv_dmtp_get_signet(session, domain, NULL))) {
				fprintf(stderr, "Error: org signet retrieval failed.\n");
				dump_error_stack();
				exit(EXIT_FAILURE);
			}

			if (!(org_signet = dime_sgnt_signet_b64_deserialize(line))) {
				free(line);
				fprintf(stderr, "Error: could not retrieve org signet to verify requested user signet.\n");
				dump_error_stack();
				exit(EXIT_FAILURE);
			}

			free(line);

			if (dime_sgnt_validate_all(org_signet, NULL, NULL, (const unsigned char **)session->drec->pubkey) < SS_CRYPTO) {
				fprintf(stderr, "Error: org signet could not be verified against DIME management record POK.\n");
				dump_error_stack();
				exit(EXIT_FAILURE);
			}

			dbgprint(1, "Org signet validation succeeded for: %s\n", domain);
		}

		if (!(line = sgnt_resolv_dmtp_get_signet(session, signame, fingerprint))) {
			fprintf(stderr, "Error: signet retrieval failed.\n");
			dump_error_stack();
			sgnt_resolv_destroy_dmtp_session(session);
			exit(EXIT_FAILURE);
		}

		_dbgprint(1, "Signet data: %s\n", line);
	}

	if ((signet = dime_sgnt_signet_b64_deserialize(line))) {
		dime_sgnt_signet_dump(stdout, signet);
	} else {
		fprintf(stderr, "Error: unable to decode signet received from server.\n");
		dump_error_stack();
		exit(EXIT_FAILURE);
	}

	free(line);

	// We have deserialized the signet. Now make sure its POK matches up against its corresponding DIME management record.
	if (is_org && (dime_sgnt_validate_all(signet, NULL, NULL, (const unsigned char **)session->drec->pubkey) < SS_CRYPTO)) {

		fprintf(stderr, "Error: org signet could not be verified against DIME management record POK.\n");
		dump_error_stack();
		exit(EXIT_FAILURE);
	} else if (is_org) {
		dbgprint(1, "Org signet passed POK comparison.\n");
	} else if (!is_org && (dime_sgnt_validate_all(signet, NULL, org_signet, NULL) < SS_CRYPTO)) {
		fprintf(stderr, "Error: user signet could not be verified against org signet.\n");
		dump_error_stack();
		exit(EXIT_FAILURE);
	} else if (!is_org) {
		dbgprint(1, "User signet validation succeeded for: %s\n", signame);
	}

	if (!add_cached_object(signame, &(cached_stores[cached_data_signet]), 0, 0, signet, 1, 0)) {
		fprintf(stderr, "Error: unable to add signet to cache.\n");
		dump_error_stack();
		exit(EXIT_FAILURE);
	}

	if (save_cache_contents() < 0) {
		fprintf(stderr, "Error: unable to save cache contents to disk.\n");
		dump_error_stack();
	}

	// TODO: need to incorporate this into code branch where the app exits.
	sgnt_resolv_destroy_dmtp_session(session);


/*	printf("Entering loop...\n");
        _ssl_fd_loop(session->con);
        printf("Exited loop.. Terminating.\n"); */

/*	if (_verbose >= 2) {
                dump_dnskey_entries();
                dump_ds_entries();
        } */


	return 0;
}
