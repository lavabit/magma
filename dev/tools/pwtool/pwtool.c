
/**
 * @file /magma.utils/pwutil.c
 *
 * @brief	The password change utility.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "pwtool.h"


extern void *lib_magma;


void getsyms(void) {

	static int once = 0;

	if (once) {
		return;
	}

	if (!st_cmp_ci_eq(NULLER(magma.library.file), CONSTANT("NULL"))) {
		lib_magma = dlopen(RTLD_DEFAULT, RTLD_NOW | RTLD_GLOBAL);
	}
	else {
		lib_magma = dlopen(magma.library.file, RTLD_NOW | RTLD_GLOBAL);
	}

	if (!lib_magma) {
		log_critical("Error occurred loading the magma library: %s", dlerror());
		exit(-1);
	}

	once = 1;

	return;
}


stringer_t * get_user_pw_hash(const char *username, const char *password) {

	stringer_t *combo[3], *bin, *hex;

	combo[0] = st_merge("nsn", username, magma.secure.salt, password);
	bin = st_alloc(64);
	hex = st_alloc(128);

	if (!combo[0] || !bin || !hex) {
		fprintf(stderr, "Unexpected error occurred preparing for password hashing operation.\n");
		exit(-1);
	}

	if (digest_sha512(combo[0], bin) != bin || !(combo[1] = st_merge("ns", password, bin)) || digest_sha512(combo[1], bin) != bin ||
		!(combo[2] = st_merge("ns", password, bin)) || digest_sha512(combo[2], bin) != bin || hex_encode_st(bin, hex) != hex) {
		fprintf(stderr, "Unexpected error occurred hashing password.");
		exit(-1);
	}

	return hex;
}


stringer_t * get_user_pw_key(const char *username, const char *password) {

	stringer_t *combo[3], *bin, *hex;

	combo[0] = st_merge("nsn", username, magma.secure.salt, password);
	bin = st_alloc(64);
	hex = st_alloc(128);

	if (!combo[0] || !bin || !hex) {
		fprintf(stderr, "Unexpected error occurred preparing for password hashing operation.\n");
		exit(-1);
	}

	if ((digest_sha512(combo[0], bin)) != bin || (hex_encode_st(bin, hex) != hex)) {
		fprintf(stderr, "Unexpected error deriving password key.");
		exit(-1);
	}

	return hex;
}


void dump_passkey(char *username, char *password) {

	stringer_t *combo[3], *bin, *hex, *tmp;

	combo[0] = st_merge("nsn", username, magma.secure.salt, password);
	bin = st_alloc(64);
	hex = st_alloc(128);
	tmp = st_alloc(128);

	if (!combo[0] || !bin || !hex) {
		fprintf(stderr, "Unexpected error occurred preparing for password hashing operation.\n");
		exit(-1);
	}

	if (digest_sha512(combo[0], bin) != bin || hex_encode_st(bin, tmp) != tmp || !(combo[1] = st_merge("ns", password, bin)) || digest_sha512(combo[1], bin) != bin ||
		!(combo[2] = st_merge("ns", password, bin)) || digest_sha512(combo[2], bin) != bin || hex_encode_st(bin, hex) != hex) {
		fprintf(stderr, "Unexpected error occurred hashing password.");
		exit(-1);
	}

	printf("hash_pass: %.*s\n", (int)st_length_get(hex), st_char_get(hex));
}


void reencrypt_storage_keys(char *username, char *oldpass, char *newpass) {

	row_t *row;
	table_t *table;
	MYSQL_BIND parameters[2];
	scramble_t *scrkey;
	stringer_t *priv_b64, *keydata, *priv, *passkey, *pwhash;
	uint64_t usernum;

	pwhash = get_user_pw_hash(username, oldpass);

	// First map the requested user name to a user id number.
	
	mm_wipe(parameters, sizeof(parameters));
	parameters[0].buffer_type = MYSQL_TYPE_STRING;
	parameters[0].buffer_length = strlen(username);
	parameters[0].buffer = username;
	
	parameters[1].buffer_type = MYSQL_TYPE_STRING;
	parameters[1].buffer_length = st_length_get(pwhash);
	parameters[1].buffer = st_char_get(pwhash);

	if (!(table = stmt_get_result(stmts.select_user, parameters))) {
		fprintf(stderr, "Unexpected error occurred while retrieving user record from database.\n");
		exit(-1);
	} else if (!(row = res_row_next(table))) {
		res_table_free(table);
		fprintf(stderr, "Error retrieving user record from database.\n");
		exit(-1);
	}
	
	usernum = res_field_uint64(row, 2);
	res_table_free(table);

	// Next use the user id number to request a storage key pair.

	mm_wipe(parameters, sizeof(parameters));
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &usernum;
	parameters[0].is_unsigned = true;

	if (!(table = stmt_get_result(stmts.select_user_storage_keys, parameters))) {
		fprintf(stderr, "Unexpected error occurred while retrieving storage keys for user.\n");
		exit(-1);
	}
	else if (!(row = res_row_next(table))) {
		fprintf(stderr, "Error occurred while reading usage storage keys from database.\n");
		res_table_free(table);
		exit(-1);
	}

	if (!res_field_length(row, 0) || !res_field_length(row, 1)) {
		fprintf(stderr, "Error: requested user was missing storage key(s) in database.\n");
		res_table_free(table);
		exit(-1);
	}

	priv_b64 = st_import(res_field_block(row, 1), res_field_length(row, 1));
	res_table_free(table);

	// We have the encrypted private key in base64 format. First, decode it.
//	if (!(keydata = base64_decode_opts(priv_b64, MANAGED_T | CONTIGUOUS | SECURE, true))) {
	if (!(keydata = base64_decode_mod(priv_b64, NULL))) {
		fprintf(stderr, "Error: unable to base64 decode user private storage key in database.\n");
		st_cleanup(priv_b64);
		exit(-1);
	}

	st_free(priv_b64);
	scrkey = (scramble_t *) st_data_get(keydata);

	// Then descramble.
	passkey = get_user_pw_key(username, oldpass);

	if (!(priv = scramble_decrypt(passkey, scrkey))) {
		fprintf(stderr, "Error: unable to descramble user private storage key from database.\n");
		st_free(keydata);
		st_free(passkey);
		exit(-1);
	}

	st_free(keydata);
	st_free(passkey);

	// Then reverse the process. Reencrypt the private storage key with the new password.
	passkey = get_user_pw_key(username, newpass);

	if (!(scrkey = scramble_encrypt(passkey, priv))) {
		fprintf(stderr, "Error: unable to scramble user private storage key with new password.\n");
		st_free(priv);
		st_free(passkey);
		exit(-1);
	}

	st_free(priv);
	st_free(passkey);

	if (!(keydata = st_import(scrkey, scramble_total_length(scrkey)))) {
		fprintf(stderr, "Unexpected error occurred while repacking new encrypted user private storage key.\n");
		scramble_free(scrkey);
		exit(-1);
	}

	scramble_free(scrkey);

//	if (!(priv_b64 = base64_encode_opts(keydata, MANAGED_T | CONTIGUOUS | SECURE, true))) {
	if (!(priv_b64 = base64_encode_mod(keydata, NULL))) {
		fprintf(stderr, "Error: Unable to base64 encode newly encrypted user private storage key.\n");
		st_free(keydata);
		exit(-1);
	}

	printf("db_storage_privkey: %.*s\n", st_length_int(priv_b64), st_char_get(priv_b64));

	st_free(priv_b64);

	return;
}


void usage(char *progname) {

	fprintf(stderr, "\nUsage: %s [options] <config_file>\n", progname);
	fprintf(stderr, "   where [options] MUST be at least one of the following:\n");
	fprintf(stderr, "   -d          dump database schema name\n");
	fprintf(stderr, "   -u          dump database username\n");
	fprintf(stderr, "   -p          dump database password\n");
	fprintf(stderr, "   -h          dump database hostname\n");
	fprintf(stderr, "   -l          dump database port\n");
	fprintf(stderr, "   -k [user]   dump hash of supplied user [password will be read from stdin]\n");
	fprintf(stderr, "   -r [user]   reencrypt public and private keys [passwords will be read from stdin]\n\n");
	exit(-1);
}


int main(int argc, char *argv[]) {

	chr_t passbuf[128], npassbuf[128], npassbuf2[128], *hash_user = NULL, *key_user = NULL;
	int c, ndump = 0;
	bool_t dump_db_name, dump_db_user, dump_db_pass, dump_db_host, dump_db_port, dump_hash, reencrypt;

	dump_db_name = dump_db_user = dump_db_pass = dump_db_host = dump_db_port = dump_hash = reencrypt = false;

	while ((c = getopt(argc,argv,"duphlk:r:")) != -1) {
		switch(c) {
			case 'd':
				dump_db_name = true;
				ndump++;
				break;
			case 'u':
				dump_db_user = true;
				ndump++;
				break;
			case 'p':
				dump_db_pass = true;
				ndump++;
				break;
			case 'h':
				dump_db_host = true;
				ndump++;
				break;
			case 'l':
				dump_db_port= true;
				ndump++;
				break;
			case 'k':
				dump_hash = true;
				hash_user = optarg;
				ndump++;
				break;
			case 'r':
				reencrypt = true;
				key_user = optarg;
				ndump++;
				break;
			default:
				usage(argv[0]);
				break;
		}

	}

	if (!ndump) {
		fprintf(stderr,"Error: must specify at least one option to be dumped.\n");
		usage(argv[0]);
	}

	if (argc != (optind +1)) {
		usage (argv[0]);
	}

	memset(magma.config.file,0,sizeof(magma.config.file));
	strncpy(magma.config.file,argv[optind],sizeof(magma.config.file)-1);
	fprintf(stderr, "Loading settings from: %s\n", magma.config.file);

	if (!config_load_file_settings()) {
		fprintf(stderr, "Error: Failed to load config file settings.\n");
		exit(-1);
	}

	// All of the following options only require from-file configuration options.

	if (dump_db_name) {
		printf("db_name: %s\n", magma.iface.database.schema);
	}
	if (dump_db_user) {
		printf("db_user: %s\n", magma.iface.database.user);
	}

	if (dump_db_pass) {
		printf("db_password: %s\n", magma.iface.database.password);
	}

	if (dump_db_host) {
		printf("db_host: %s\n", magma.iface.database.host);
	}

	if (dump_db_port) {
		printf("db_port: %u\n", magma.iface.database.port);
	}

	// If we want to process keys or passwords, we have to do more work.
	if (!dump_hash && !reencrypt) {
		exit(0);
	}

	getsyms();

	if (!lib_load_openssl()) {
		fprintf (stderr, "Error initializing openssl routines.\n");
		exit(-1);
	}

	if (!lib_load_mysql()) {
		fprintf (stderr, "Error initializing mysql routines.\n");
		exit(-1);
	}

	if (!sql_start()) {
		fprintf(stderr, "Error initializing sql subsystem.\n");
		exit(-1);
	}

	if (gethostname(magma.host.name, MAGMA_HOSTNAME_MAX + 1) != 0) {
		perror("gethostname");
		exit(-1);
	}

	if (!config_load_database_settings()) {
		fprintf(stderr, "Error: Failed to load database settings.\n");
		exit(-1);
	}

	if (dump_hash) {
		memset(passbuf,0,sizeof(passbuf));
		fprintf(stderr, "Please enter new password for user %s:\n", hash_user);

		if (!fgets(passbuf,sizeof(passbuf),stdin) || !strlen(passbuf)) {
			fprintf(stderr,"Error reading password from stdin.\n");
			exit(-1);
		}

		passbuf[strlen(passbuf)-1] = 0;
		dump_passkey(hash_user, passbuf);
	}

	if (reencrypt) {

		if (!ssl_start()) {
			fprintf(stderr, "Error initializing openssl subsystem.\n");
			exit(-1);
		}

		memset(passbuf, 0, sizeof(passbuf));
		fprintf(stderr, "Please enter old password for user %s:\n", key_user);

		if (!fgets(passbuf,sizeof(passbuf),stdin) || !strlen(passbuf)) {
			fprintf(stderr,"Error reading password from stdin.\n");
			exit(-1);
		}

		passbuf[strlen(passbuf)-1] = 0;

		memset(npassbuf, 0, sizeof(npassbuf));
		memset(npassbuf2, 0, sizeof(npassbuf2));

		fprintf(stderr, "Please enter new password for user %s:\n", key_user);

		if (!fgets(npassbuf,  sizeof(npassbuf), stdin) || !strlen(npassbuf)) {
			fprintf(stderr,"Error reading password from stdin.\n");
			exit(-1);
		}

		printf("Please confirm new password for user:\n");

		if (!fgets(npassbuf2,  sizeof(npassbuf2), stdin) || !strlen(npassbuf2)) {
			fprintf(stderr,"Error reading password from stdin.\n");
			exit(-1);
		}

		npassbuf[strlen(npassbuf)-1] = 0;
		npassbuf2[strlen(npassbuf2)-1] = 0;

		if (strcmp(npassbuf, npassbuf2)) {
			fprintf(stderr, "Error: user passwords did not match.\n");
			exit(-1);
		}

		reencrypt_storage_keys(key_user, passbuf, npassbuf);
	}
		


/*	if (!process_start()) {
		log_unit("Initialization error. Exiting.\n");
		status_set(-1);
		process_stop();
		exit(EXIT_FAILURE);
	} */


//	process_stop();
//	system_init_umask();
	exit(0);
}
