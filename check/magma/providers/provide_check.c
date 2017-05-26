
/**
 * @file /check/magma/providers/provide_check.c
 *
 * @brief The heart of the suite of unit tests for the Magma provide module.
 */

#include "magma_check.h"

typedef struct {
	const char		*domain;
	int				 rr_type;
	SPF_dns_stat_t	 herrno;
	const char		*data;
} SPF_dns_test_data_t;

extern pool_t *spf_pool;
extern bool_t do_tank_check, do_virus_check, do_dspam_check, do_spf_check;
extern chr_t *virus_check_data_path;

//! Generic Provider Symbol Tests
START_TEST (check_symbols_s) {
	log_disable();
	void *local = NULL;
	chr_t *errmsg = NULL, *liberr = NULL;

	if (status() && !(local = dlopen(NULL, RTLD_NOW | RTLD_LOCAL)))  {
		errmsg = "Library handle creation failed.";
	}
	else if (status() && (liberr = dlerror())) {
		errmsg = liberr;
	}
	else if (status() && !liberr && dlsym(NULL, "ERR_error_string_d")) {
		errmsg = "Found a reference to ERR_error_string() which isn't thread-safe. Use ssl_error_string() instead.";
	}

	if (local) dlclose(local);

	log_test("PROVIDERS / SYMBOLS / SINGLE THREADED:", NULLER(errmsg));
	if (errmsg) ck_abort_msg(errmsg);

}
END_TEST

//! Compression Engine Tests
START_TEST (check_compress_lzo_s) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = NULL;
	check_compress_opt_t opts = {
		.engine = COMPRESS_ENGINE_LZO
	};

	if (!check_compress_sthread(&opts)) {
		outcome = false;
		errmsg = NULLER("The single-threaded LZO compression test failed.");
	}

	log_test("COMPRESSION / LZO / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_compress_lzo_m) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = NULL;
	check_compress_opt_t opts = {
		.engine = COMPRESS_ENGINE_LZO
	};

	if (!check_compress_mthread(&opts)) {
		outcome = false;
		errmsg = NULLER("The multi-threaded LZO compression test failed.");
	}

	log_test("COMPRESSION / LZO / MULTI THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_compress_zlib_s) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = NULL;
	check_compress_opt_t opts = {
		.engine = COMPRESS_ENGINE_ZLIB
	};

	if (!check_compress_sthread(&opts)) {
		outcome = false;
		errmsg = NULLER("The single-threaded ZLIB compression test failed.");
	}

	log_test("COMPRESSION / ZLIB / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_compress_zlib_m) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = NULL;
	check_compress_opt_t opts = {
		.engine = COMPRESS_ENGINE_ZLIB
	};

	if (!check_compress_mthread(&opts)) {
		outcome = false;
		errmsg = NULLER("The multi-threaded ZLIB compression test failed.");
	}

	log_test("COMPRESSION / ZLIB / MULTI THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_compress_bzip_s) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = NULL;
	check_compress_opt_t opts = {
		.engine = COMPRESS_ENGINE_BZIP
	};

	if (!check_compress_sthread(&opts)) {
		outcome = false;
		errmsg = NULLER("The single-threaded BZIP compression test failed.");
	}

	log_test("COMPRESSION / BZIP / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_compress_bzip_m) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = NULL;
	check_compress_opt_t opts = {
		.engine = COMPRESS_ENGINE_BZIP
	};

	if (!check_compress_mthread(&opts)) {
		outcome = false;
		errmsg = NULLER("The multi-threaded BZIP compression test failed.");
	}

	log_test("COMPRESSION / BZIP / MULTI THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

//! Storage Tank Tests
START_TEST (check_tank_lzo_s) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = NULL;
	check_tank_opt_t opts = {
		.engine = TANK_COMPRESS_LZO
	};

	if (!check_tokyo_tank_sthread(&opts)) {
		outcome = false;
		errmsg = NULLER("The single-threaded LZO storage tank test failed.");
	}

	log_test("TANK / LZO / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
	tank_maintain();
}
END_TEST

START_TEST (check_tank_lzo_m) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = NULL;
	check_tank_opt_t opts = {
		.engine = TANK_COMPRESS_LZO
	};

	if (!check_tokyo_tank_mthread(&opts)) {
		outcome = false;
		errmsg = NULLER("The multi-threaded LZO storage tank test failed.");
	}

	log_test("TANK / LZO / MULTI THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
	tank_maintain();
}
END_TEST

START_TEST (check_tank_zlib_s) {

		log_disable();
		bool_t outcome = true;
		stringer_t *errmsg = NULL;
		check_tank_opt_t opts = {
			.engine = TANK_COMPRESS_ZLIB
		};

		if (!check_tokyo_tank_sthread(&opts)) {
			outcome = false;
			errmsg = NULLER("The single-threaded ZLIB storage tank test failed.");
		}

		log_test("TANK / ZLIB / SINGLE THREADED:", errmsg);
		ck_assert_msg(outcome, st_char_get(errmsg));
		tank_maintain();

	}
END_TEST

START_TEST (check_tank_zlib_m) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = NULL;
	check_tank_opt_t opts = {
		.engine = TANK_COMPRESS_ZLIB
	};

	if (!check_tokyo_tank_mthread(&opts)) {
		outcome = false;
		errmsg = NULLER("The multi-threaded ZLIB storage tank test failed.");
	}

	log_test("TANK / ZLIB / MULTI THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
	tank_maintain();
}
END_TEST

START_TEST (check_tank_bzip_s) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = NULL;
	check_tank_opt_t opts = {
		.engine = TANK_COMPRESS_BZIP
	};

	if (!check_tokyo_tank_sthread(&opts)) {
		outcome = false;
		errmsg = NULLER("The single-threaded BZIP storage tank test failed.");
	}

	log_test("TANK / BZIP / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
	tank_maintain();
}
END_TEST

START_TEST (check_tank_bzip_m) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = NULL;
	check_tank_opt_t opts = {
		.engine = TANK_COMPRESS_BZIP
	};

	if (!check_tokyo_tank_mthread(&opts)) {
		outcome = false;
		errmsg = NULLER("The multi-threaded BZIP storage tank test failed.");
	}

	log_test("TANK / BZIP / MULTI THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
	tank_maintain();

}
END_TEST

//! Cryptography Tests
START_TEST (check_ecies_s) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = NULL;

	if (!check_ecies_sthread()) {
		outcome = false;
		errmsg = NULLER("The ECIES test failed.");
	}

	log_test("CRYPTOGRAPHY / ECIES / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_hash_s) {

	log_disable();
	bool_t outcome = true;
	chr_t errbuffer[1024];

	// Note that MD2 and WHIRLPOOL are not available. Although WHIRLPOOL may be available as whirlpool.
	chr_t *digest_list[] = {
		"MD4", "MD5", "SHA", "SHA1", "SHA224", "SHA256", "SHA384", "SHA512", "RIPEMD160"
	};

	mm_wipe(errbuffer, sizeof(errbuffer));

	if (status() && !(outcome = check_hash_simple())) {
		snprintf(errbuffer, 1024, "digest methods failed to return the expected result...");
	}

	for (uint64_t i = 0; status() && outcome == true && i < (sizeof(digest_list) / sizeof(chr_t *)); i++) {
		if (!(outcome = check_hash_sthread(digest_list[i]))) {
			snprintf(errbuffer, 1024, "%s failed...", digest_list[i]);
		}
	}

	log_test("CRYPTOGRAPHY / HASH / SINGLE THREADED:", NULLER(errbuffer));
	ck_assert_msg(outcome, errbuffer);
}
END_TEST

START_TEST (check_hmac_s) {

	log_disable();
	bool_t outcome = true;
	bool_t (*checks[])(void) = {
		&check_hmac_parameters,
		&check_hmac_simple
	};
	stringer_t *err = NULL;
	stringer_t *errors[] = {
		NULLER("check_hmac_parameters failed"),
		NULLER("check_hmac_simple failed")
	};

	for(uint_t i = 0; status() && !err && i < sizeof(checks)/sizeof((checks)[0]); ++i) {
		if(!(outcome = checks[i]())) {
			err = errors[i];
		}
	}

	log_test("CRYPTOGRAPHY / HMAC / SINGLE THREADED:", err);
	ck_assert_msg(outcome, st_char_get(err));
}
END_TEST

START_TEST (check_symmetric_s) {

	stringer_t *errmsg = NULL, *failed = NULL;
	chr_t *cipher_list[] = {
		"aes-128-cbc", "aes-128-ccm", "aes-128-cfb", "aes-128-cfb1", "aes-128-cfb8", "aes-128-ctr", "aes-128-ecb", "aes-128-gcm",
		"aes-128-ofb", "aes-128-xts", "aes-192-cbc", "aes-192-ccm", "aes-192-cfb", "aes-192-cfb1", "aes-192-cfb8", "aes-192-ctr",
		"aes-192-ecb", "aes-192-gcm", "aes-192-ofb", "aes-256-cbc", "aes-256-ccm", "aes-256-cfb", "aes-256-cfb1", "aes-256-cfb8",
		"aes-256-ctr", "aes-256-ecb", "aes-256-gcm", "aes-256-ofb", "aes-256-xts", "aes128", "aes192", "aes256", "camellia-128-cbc",
		"camellia-128-cfb", "camellia-128-cfb1", "camellia-128-cfb8", "camellia-128-ecb", "camellia-128-ofb", "camellia-192-cbc",
		"camellia-192-cfb", "camellia-192-cfb1", "camellia-192-cfb8", "camellia-192-ecb", "camellia-192-ofb", "camellia-256-cbc",
		"camellia-256-cfb", "camellia-256-cfb1", "camellia-256-cfb8", "camellia-256-ecb", "camellia-256-ofb", "camellia128",
		"camellia192", "camellia256", "idea", "idea-cbc", "idea-cfb", "idea-ecb", "idea-ofb", "rc2-40-cbc", "rc2-64-cbc", "rc2-cbc",
		"rc2-cfb", "rc2-ecb", "rc2-ofb", "rc4-40", "seed-cbc", "seed-cfb", "seed-ecb", "seed-ofb", "des", "des-cbc", "des-cfb",
		"des-cfb1", "des-cfb8", "des-ecb", "des-ede", "des-ede-cbc", "des-ede-cfb", "des-ede-ofb", "des-ede3", "des-ede3-cbc",
		"des-ede3-cfb", "des-ede3-cfb8", "des-ede3-ofb", "des-ofb", "des3", "desx", "desx-cbc", "bf", "bf-cbc",
		"bf-cfb", "bf-ecb", "bf-ofb", "id-aes128-GCM", "id-aes192-GCM", "id-aes256-GCM"
	};

	log_disable();

	for (uint64_t i = 0; status() && i < (sizeof(cipher_list) / sizeof(chr_t *)); i++) {

		if (status() && !(check_symmetric_sthread(cipher_list[i]))) {
			failed = st_append(failed, NULLER(cipher_list[i]));
			failed = st_append(failed, NULLER(" "));
		}

	}

	if (st_populated(failed)) {
		errmsg = st_aprint("%.*sfailed...", st_length_int(failed), st_char_get(failed));
	}

	log_test("CRYPTOGRAPHY / SYMMETRIC / SINGLE THREADED:", errmsg);
	fail_unless(!errmsg, st_char_get(errmsg));
	st_cleanup(failed, errmsg);

}
END_TEST

START_TEST (check_scramble_s) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = NULL;

	if (!check_scramble_sthread()) {
		outcome = false;
		errmsg = NULLER("Failed to check scrable single-threaded.");
	}

	log_test("CRYPTOGRAPHY / SCRAMBLE / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_rand_s) {

	log_disable();
	stringer_t *errmsg = NULL;

	errmsg = check_rand_sthread();

	log_test("CRYPTOGRAPHY / RAND / SINGLE THREADED:", errmsg);
	ck_assert_msg(!errmsg, st_char_get(errmsg));
	st_cleanup(errmsg);
}
END_TEST

START_TEST (check_rand_m) {

	log_disable();
	stringer_t *errmsg = NULL;

	errmsg = check_rand_mthread();

	log_test("CRYPTOGRAPHY / RAND / MULTI THREADED:", errmsg);
	ck_assert_msg(!errmsg, st_char_get(errmsg));
	st_cleanup(errmsg);
}
END_TEST

//! SPF Tests
START_TEST (check_spf_s) {

	log_disable();
	chr_t *errmsg = NULL;
	SPF_server_t *object;
	SPF_dns_server_t *spf_dns_zone = NULL;
	ip_t ip[2] = {
			{ .family = AF_INET, { .ip4.s_addr = 0x01010101 }},
			{ .family = AF_INET, { .ip4.s_addr = 0x0100007f }}
	};
	SPF_dns_test_data_t spf_test_data[3] = {
			{ "pass.lavabit.com", ns_t_txt, NETDB_SUCCESS, "v=spf1 ip4:1.1.1.1 -all" },
			{ "fail.lavabit.com", ns_t_txt, NETDB_SUCCESS, "v=spf1 -all" },
			{ "neutral.lavabit.com", ns_t_txt, NETDB_SUCCESS, "v=spf1 ~all" }
	};

	if (status()) {

		for (uint32_t i = 0; !errmsg && i < magma.iface.spf.pool.connections; i++) {

			// Pull the SPF server object from the pool and append a DNS zone to it.
			if (!(object = pool_get_obj(spf_pool, i)) || !(object->resolver) || !(spf_dns_zone = SPF_dns_zone_new_d(object->resolver, NULL, 0))) {
				errmsg = "Unable to append the DNS zone used for testing different SPF record types onto the SPF server object.";
			}
			else {
				object->resolver = spf_dns_zone;
			}

			// Assuming the DNS zone was created above, iterate through the test data records and append them to the zone object.
			for (uint32_t i = 0; !errmsg && i < (sizeof(spf_test_data) / sizeof(*spf_test_data)); i++) {
				if (SPF_dns_zone_add_str_d(spf_dns_zone, spf_test_data[i].domain, spf_test_data[i].rr_type, spf_test_data[i].herrno, spf_test_data[i].data) != SPF_E_SUCCESS) {
					errmsg = "Unable to insert the sample DNS record into the zone used for testing different SPF record types.";
				}
			}
		}

		// Pass
		if (!errmsg && spf_check(&ip[0], NULLER("mx.lavabit.com"), NULLER("support@pass.lavabit.com")) != 1) {
			errmsg = "Valid SPF record check failed. { support@pass.lavabit.com / 1.1.1.1 }";
		}

		// Neutral
		if (!errmsg && spf_check(&ip[0], NULLER("mx.lavabit.com"), NULLER("support@neutral.lavabit.com")) != -1) {
			errmsg = "Neutral SPF record check failed. { support@neutral.lavabit.com / 1.1.1.1 }";
		}

		// Fail
		if (!errmsg && spf_check(&ip[0], NULLER("mx.lavabit.com"), NULLER("support@fail.lavabit.com")) != -2) {
			errmsg = "Invalid SPF record check failed. { support@fail.lavabit.com / 1.1.1.1 }";
		}

		// Ensure the localhost always gets through
		if (!errmsg && spf_check(&ip[1], NULLER("mx.lavabit.com"), NULLER("support@fail.lavabit.com")) != 1) {
			errmsg = "The localhost address matched a failure record instead of being whitelisted. { support@fail.lavabit.com / 127.0.0.1 }";
		}
	}

	log_test("CHECKERS / SPF / SINGLE THREADED:", NULLER(errmsg));
	fail_unless(!errmsg, errmsg);

}
END_TEST

//! Virus Checker Tests
START_TEST (check_virus_s) {

	log_disable();
	bool_t result = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (status() && magma.iface.virus.available) {
		result = check_virus_sthread(errmsg);
	}

	log_test("CHECKERS / VIRUS / SINGLE THREADED:", (magma.iface.virus.available ? errmsg : NULLER("SKIPPED")));
	ck_assert_msg(result, st_char_get(errmsg));
}
END_TEST

//! Spam Checker Tests
START_TEST (check_dspam_mail_s) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = NULL;

	if (status() && !check_dspam_mail_sthread()) {
		outcome = false;
		errmsg = NULLER("The check_dspam_mail_s test failed");
	}

	log_test("CHECKERS / DSPAM / MAIL / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_dspam_bin_s) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = NULL;

	if (status() && !check_dspam_binary_sthread()) {
		outcome = false;
		errmsg = NULLER("check_dspam_bin_s failed");
	}

	log_test("CHECKERS / DSPAM / BINARY / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

//! DKIM Tests
START_TEST (check_dkim_s) {

	log_disable();
	bool_t result = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	// If the DKIM engine isn't enabled, then we'll skip the unit test.
	if (!(result = magma.dkim.enabled)) st_sprint(errmsg, "SKIPPED");

	// Otherwise, we'll perform the checks... unless the status variable indicates we shouldn't.
	if (status() && result) result = check_dkim_sign_sthread(errmsg);
	if (status() && result) result = check_dkim_verify_sthread(errmsg);

	log_test("CHECKERS / DKIM / SINGLE THREADED:", errmsg);
	ck_assert_msg(result, st_char_get(errmsg));

}
END_TEST

//! Encoding/Parser Tests
START_TEST (check_unicode_s) {

	log_disable();
	bool_t result = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (status()) result = check_unicode_valid(errmsg);
	if (status() && result) result = check_unicode_invalid(errmsg);
	if (status() && result) result = check_unicode_length(errmsg);

	log_test("PARSERS / UNICODE / SINGLE THREADED:", errmsg);
	ck_assert_msg(result, st_char_get(errmsg));

}
END_TEST

Suite * suite_check_provide(void) {

	Suite *s = suite_create("\tProviders");

	suite_check_testcase(s, "PROVIDERS", "Provider Symbols/S", check_symbols_s);

	suite_check_testcase(s, "PROVIDERS", "Parsers Unicode/S", check_unicode_s);

	suite_check_testcase(s, "PROVIDERS", "Compression LZO/S", check_compress_lzo_s);
	suite_check_testcase(s, "PROVIDERS", "Compression LZO/M", check_compress_lzo_m);
	suite_check_testcase(s, "PROVIDERS", "Compression ZLIB/S", check_compress_zlib_s);
	suite_check_testcase(s, "PROVIDERS", "Compression ZLIB/M", check_compress_zlib_m);
	suite_check_testcase(s, "PROVIDERS", "Compression BZIP/S", check_compress_bzip_s);
	suite_check_testcase(s, "PROVIDERS", "Compression BZIP/M", check_compress_bzip_m);

	suite_check_testcase(s, "PROVIDERS", "Cryptography RAND/S", check_rand_s);
	suite_check_testcase(s, "PROVIDERS", "Cryptography RAND/M", check_rand_m);
	suite_check_testcase(s, "PROVIDERS", "Cryptography ECIES/S", check_ecies_s);
	suite_check_testcase(s, "PROVIDERS", "Cryptography HASH/S", check_hash_s);
	suite_check_testcase(s, "PROVIDERS", "Cryptography HMAC/S", check_hmac_s);
	suite_check_testcase(s, "PROVIDERS", "Cryptography SYMMETRIC/S", check_symmetric_s);
	suite_check_testcase(s, "PROVIDERS", "Cryptography SCRAMBLE/S", check_scramble_s);

	// Tank functionality is temporarily disabled.
	if (do_tank_check) {
		suite_check_testcase(s, "PROVIDERS", "Tank LZO/S", check_tank_lzo_s);
		suite_check_testcase(s, "PROVIDERS", "Tank LZO/M", check_tank_lzo_m);
		suite_check_testcase(s, "PROVIDERS", "Tank ZLIB/S", check_tank_zlib_s);
		suite_check_testcase(s, "PROVIDERS", "Tank ZLIB/M", check_tank_zlib_m);
		suite_check_testcase(s, "PROVIDERS", "Tank BZIP/S", check_tank_bzip_s);
		suite_check_testcase(s, "PROVIDERS", "Tank BZIP/M", check_tank_bzip_m);
	}
	else {
		log_unit("Skipping tank checks...\n");
	}

	if (do_spf_check) {
		suite_check_testcase(s, "PROVIDERS", "SPF/S", check_spf_s);
	}
	else {
		log_unit("Skipping SPF checks...\n");
	}

	suite_check_testcase(s, "PROVIDERS", "DKIM/S", check_dkim_s);

	if (do_virus_check) {
		suite_check_testcase(s, "PROVIDERS", "Virus/S", check_virus_s);
	}
	else {
		log_unit("Skipping virus checks...\n");
	}

	if (do_dspam_check) {
		suite_check_testcase(s, "PROVIDERS", "DSPAM Mail/S", check_dspam_mail_s);
		suite_check_testcase(s, "PROVIDERS", "DSPAM Binary/S", check_dspam_bin_s);
	}
	else {
		log_unit("Skipping DSPAM checks...\n");
	}

	return s;
}

