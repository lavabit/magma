
/**
 * @file /magma.check/providers/provide_check.c
 *
 * @brief The heart of the suite of unit tests for the Magma provide module.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma_check.h"

typedef struct
{
	const char		*domain;
	int				 rr_type;
	SPF_dns_stat_t	 herrno;
	const char		*data;
} SPF_dns_test_data_t;

extern pool_t *spf_pool;
extern bool_t do_tank_check, do_virus_check, do_dspam_check, do_spf_check;
extern chr_t *virus_check_data_path;

//! Compression Engine Tests
START_TEST (check_compress_lzo_s)
	{
		bool_t outcome;
		check_compress_opt_t opts = {
			.engine = COMPRESS_ENGINE_LZO
		};

		log_unit("%-64.64s", "COMPRESSION / LZO / SINGLE THREADED:");
		outcome = check_compress_sthread(&opts);
		log_unit("%10.10s\n", (outcome ? (status() ? "PASSED" : "SKIPPED") : "FAILED"));
		fail_unless(outcome, "check_compress_sthread failed");
	}
END_TEST

START_TEST (check_compress_lzo_m)
	{
		bool_t outcome;
		check_compress_opt_t opts = {
			.engine = COMPRESS_ENGINE_LZO
		};
		log_unit("%-64.64s", "COMPRESSION / LZO / MULTITHREADED:");
		outcome = check_compress_mthread(&opts);
		log_unit("%10.10s\n", (outcome ? (status() ? "PASSED" : "SKIPPED") : "FAILED"));
		fail_unless(outcome, "check_compress_mthread failed");
	}
END_TEST

START_TEST (check_compress_zlib_s)
	{
		bool_t outcome;
		check_compress_opt_t opts = {
			.engine = COMPRESS_ENGINE_ZLIB
		};
		log_unit("%-64.64s", "COMPRESSION / ZLIB / SINGLE THREADED:");
		outcome = check_compress_sthread(&opts);
		log_unit("%10.10s\n", (outcome ? (status() ? "PASSED" : "SKIPPED") : "FAILED"));
		fail_unless(outcome, "check_compress_sthread failed");
	}
END_TEST

START_TEST (check_compress_zlib_m)
	{
		bool_t outcome;
		check_compress_opt_t opts = {
			.engine = COMPRESS_ENGINE_ZLIB
		};
		log_unit("%-64.64s", "COMPRESSION / ZLIB / MULTITHREADED:");
		outcome = check_compress_mthread(&opts);
		log_unit("%10.10s\n", (outcome ? (status() ? "PASSED" : "SKIPPED") : "FAILED"));
		fail_unless(outcome, "check_compress_mthread failed");
	}
END_TEST

START_TEST (check_compress_bzip_s)
	{
		bool_t outcome;
		check_compress_opt_t opts = {
			.engine = COMPRESS_ENGINE_BZIP
		};
		log_unit("%-64.64s", "COMPRESSION / BZIP / SINGLE THREADED:");
		outcome = check_compress_sthread(&opts);
		log_unit("%10.10s\n", (outcome ? (status() ? "PASSED" : "SKIPPED") : "FAILED"));
		fail_unless(outcome, "check_compress_sthread failed");
	}
END_TEST

START_TEST (check_compress_bzip_m)
	{
		bool_t outcome;
		check_compress_opt_t opts = {
			.engine = COMPRESS_ENGINE_BZIP
		};
		log_unit("%-64.64s", "COMPRESSION / BZIP / MULTITHREADED:");
		outcome = check_compress_mthread(&opts);
		log_unit("%10.10s\n", (outcome ? (status() ? "PASSED" : "SKIPPED") : "FAILED"));
		fail_unless(outcome, "check_compress_mthread failed");
	}
END_TEST

//! Storage Tank Tests
START_TEST (check_tank_lzo_s)
	{
		bool_t outcome;
		check_tank_opt_t opts = {
			.engine = TANK_COMPRESS_LZO
		};
		log_unit("%-64.64s", "STORAGE / LZO / SINGLE THREADED:");
		outcome = check_tokyo_tank_sthread(&opts);
		log_unit("%10.10s\n", (outcome ? (status() ? "PASSED" : "SKIPPED") : "FAILED"));
		fail_unless(outcome, "check_tokyo_tank_sthread failed");
		tank_maintain();
	}
END_TEST

START_TEST (check_tank_lzo_m)
	{
		bool_t outcome;
		check_tank_opt_t opts = {
			.engine = TANK_COMPRESS_LZO
		};
		log_unit("%-64.64s", "STORAGE / LZO / MULTITHREADED:");
		outcome = check_tokyo_tank_mthread(&opts);
		log_unit("%10.10s\n", (outcome ? (status() ? "PASSED" : "SKIPPED") : "FAILED"));
		fail_unless(outcome, "check_tokyo_tank_mthread failed");
		tank_maintain();
	}
END_TEST

START_TEST (check_tank_zlib_s)
	{
		bool_t outcome;
		check_tank_opt_t opts = {
			.engine = TANK_COMPRESS_ZLIB
		};
		log_unit("%-64.64s", "STORAGE / ZLIB / SINGLE THREADED:");
		outcome = check_tokyo_tank_sthread(&opts);
		log_unit("%10.10s\n", (outcome ? (status() ? "PASSED" : "SKIPPED") : "FAILED"));
		fail_unless(outcome, "check_tokyo_tank_sthread failed");
		tank_maintain();
	}
END_TEST

START_TEST (check_tank_zlib_m)
	{
		bool_t outcome;
		check_tank_opt_t opts = {
			.engine = TANK_COMPRESS_ZLIB
		};
		log_unit("%-64.64s", "STORAGE / ZLIB / MULTITHREADED:");
		outcome = check_tokyo_tank_mthread(&opts);
		log_unit("%10.10s\n", (outcome ? (status() ? "PASSED" : "SKIPPED") : "FAILED"));
		fail_unless(outcome, "check_tokyo_tank_mthread failed");
		tank_maintain();
	}
END_TEST

START_TEST (check_tank_bzip_s)
	{
		bool_t outcome;
		check_tank_opt_t opts = {
			.engine = TANK_COMPRESS_BZIP
		};
		log_unit("%-64.64s", "STORAGE / BZIP / SINGLE THREADED:");
		outcome = check_tokyo_tank_sthread(&opts);
		log_unit("%10.10s\n", (outcome ? (status() ? "PASSED" : "SKIPPED") : "FAILED"));
		fail_unless(outcome, "check_tokyo_tank_sthread failed");
		tank_maintain();
	}
END_TEST

START_TEST (check_tank_bzip_m)
	{
		bool_t outcome;
		check_tank_opt_t opts = {
			.engine = TANK_COMPRESS_BZIP
		};
		log_unit("%-64.64s", "STORAGE / BZIP / MULTITHREADED:");
		outcome = check_tokyo_tank_mthread(&opts);
		log_unit("%10.10s\n", (outcome ? (status() ? "PASSED" : "SKIPPED") : "FAILED"));
		fail_unless(outcome, "check_tokyo_tank_mthread failed");
		tank_maintain();
	}
END_TEST

//! Cryptography Tests
START_TEST (check_ecies_s)
	{
		bool_t outcome;
		log_unit("%-64.64s", "CRYPTOGRAPHY / ECIES / SINGLE THREADED:");
		outcome = check_ecies_sthread();
		log_unit("%10.10s\n", (outcome ? (status() ? "PASSED" : "SKIPPED") : "FAILED"));
		fail_unless(outcome, "check_ecies_sthread failed");
	}
END_TEST

START_TEST (check_digest_s)
	{
		bool_t outcome = true;
		chr_t errmsg[1024];

		// Note that MD2 and WHIRLPOOL are not available. Although WHIRLPOOL may be available as whirlpool.
		chr_t *digest_list[] = {
			"MD4", "MD5", "SHA", "SHA1", "SHA224", "SHA256", "SHA384", "SHA512", "RIPEMD160"
		};

		mm_wipe(errmsg, sizeof(errmsg));

		log_unit("%-64.64s", "CRYPTOGRAPHY / DIGEST / SINGLE THREADED:");

		if (status() && !(outcome = check_hash_simple())) {
			snprintf(errmsg, 1024, "digest methods failed to return the expected result...");
		}

		for (uint64_t i = 0; status() && outcome == true && i < (sizeof(digest_list) / sizeof(chr_t *)); i++) {
			if (!(outcome = check_hash_sthread(digest_list[i]))) {
				snprintf(errmsg, 1024, "%s failed...", digest_list[i]);
			}
		}
		log_unit("%10.10s\n", (outcome ? (status() ? "PASSED" : "SKIPPED") : "FAILED"));
		fail_unless(outcome, errmsg);
	}
END_TEST

START_TEST (check_symmetric_s)
	{

		bool_t outcome = true;
		chr_t errmsg[1024], *cipher_list[] = {
			"AES-128-CBC", "AES-128-CFB", "AES-128-CFB1", "AES-128-CFB8", "AES-128-ECB", "AES-128-OFB", "AES-192-CBC", "AES-192-CFB",
			"AES-192-CFB1", "AES-192-CFB8", "AES-192-ECB", "AES-192-OFB", "AES-256-CBC", "AES-256-CFB", "AES-256-CFB1", "AES-256-CFB8",
			"AES-256-ECB", "AES-256-OFB", "BF-CBC", "BF-CFB", "BF-ECB", "BF-OFB", "CAMELLIA-128-CBC", "CAMELLIA-128-CFB", "CAMELLIA-128-CFB1",
			"CAMELLIA-128-CFB8", "CAMELLIA-128-ECB", "CAMELLIA-128-OFB", "CAMELLIA-192-CBC", "CAMELLIA-192-CFB", "CAMELLIA-192-CFB1",
			"CAMELLIA-192-CFB8", "CAMELLIA-192-ECB", "CAMELLIA-192-OFB", "CAMELLIA-256-CBC", "CAMELLIA-256-CFB", "CAMELLIA-256-CFB1",
			"CAMELLIA-256-CFB8", "CAMELLIA-256-ECB", "CAMELLIA-256-OFB", "DES-CBC", "DES-CFB", "DES-CFB1", "DES-CFB8", "DES-ECB", "DES-EDE",
			"DES-EDE-CBC", "DES-EDE-CFB", "DES-EDE-OFB", "DES-EDE3", "DES-EDE3-CFB", "DES-EDE3-CFB1", "DES-EDE3-CFB8", "DES-EDE3-OFB", "DES-OFB",
			"IDEA-CBC", "IDEA-CFB", "IDEA-ECB", "IDEA-OFB", "RC2-40-CBC", "RC2-64-CBC", "RC2-CBC", "RC2-CFB", "RC2-ECB", "RC2-OFB", "RC4-40",
			"SEED-CBC", "SEED-CFB", "SEED-ECB", "SEED-OFB"
		};

		mm_wipe(errmsg, sizeof(errmsg));

		log_unit("%-64.64s", "CRYPTOGRAPHY / SYMMETRIC / SINGLE THREADED:");
		for (uint64_t i = 0; status() && outcome == true && i < (sizeof(cipher_list) / sizeof(chr_t *)); i++) {
			if (!(outcome = check_symmetric_sthread(cipher_list[i]))) {
				snprintf(errmsg, 1024, "%s failed...", cipher_list[i]);
			}
		}
		log_unit("%10.10s\n", (outcome ? (status() ? "PASSED" : "SKIPPED") : "FAILED"));
		fail_unless(outcome, errmsg);
	}
END_TEST

START_TEST (check_scramble_s)
	{
		bool_t outcome;

		log_unit("%-64.64s", "CRYPTOGRAPHY / SCRAMBLE / SINGLE THREADED:");
		outcome = check_scramble_sthread();
		log_unit("%10.10s\n", (outcome ? (status() ? "PASSED" : "SKIPPED") : "FAILED"));
		fail_unless(outcome, "check_scramble_sthread failed");
	}
END_TEST

START_TEST (check_rand_s) {

	stringer_t *errmsg = NULL;
	log_unit("%-64.64s", "CRYPTOGRAPHY / RAND / SINGLE THREADED:");

	if (status()) {
		errmsg = check_rand_sthread();
	}

	log_unit("%10.10s\n", (!errmsg ? (status() ? "PASSED" : "SKIPPED") : "FAILED"));
	fail_unless(!errmsg, st_char_get(errmsg));
	st_cleanup(errmsg);
}
END_TEST

START_TEST (check_rand_m) {

	stringer_t *errmsg = NULL;
	log_unit("%-64.64s", "CRYPTOGRAPHY / RAND / MULTITHREADED:");

	if (status()) {
		errmsg = check_rand_mthread();
	}

	log_unit("%10.10s\n", (!errmsg ? (status() ? "PASSED" : "SKIPPED") : "FAILED"));
	fail_unless(!errmsg, st_char_get(errmsg));
	st_cleanup(errmsg);
}
END_TEST

//! SPF Test
START_TEST (check_spf_s) {

	chr_t *errmsg = NULL;
	SPF_server_t *object;
	SPF_dns_server_t *spf_dns_zone = NULL;
	ip_t ip[] = {
			{ .family = AF_INET, { .ip4.s_addr = 0x01010101 }},
			{ .family = AF_INET, { .ip4.s_addr = 0x0100007f }}
	};
	SPF_dns_test_data_t spf_test_data[] = {
			{ "pass.lavabit.com", ns_t_txt, NETDB_SUCCESS, "v=spf1 ip4:1.1.1.1 -all" },
			{ "fail.lavabit.com", ns_t_txt, NETDB_SUCCESS, "v=spf1 -all" },
			{ "neutral.lavabit.com", ns_t_txt, NETDB_SUCCESS, "v=spf1 ~all" }
	};


	log_unit("%-64.64s", "CHECKERS / SPF / SINGLE THREADED:");

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
			for (uint32_t i = 0; !errmsg && i < (sizeof(spf_test_data) / sizeof(*spf_test_data)); i++ ) {
				if (SPF_dns_zone_add_str_d(spf_dns_zone, spf_test_data[i].domain, spf_test_data[i].rr_type, spf_test_data[i].herrno, spf_test_data[i].data) != SPF_E_SUCCESS) {
					errmsg = "Unable to insert the sample DNS record into the zone used for testing different SPF record types.";
				}
			}
		}

		// Pass
		if (!errmsg && spf_check(&ip[0], NULLER("mx.lavabit.com"), NULLER("support@pass.lavabit.com")) != 1)
			errmsg = "Valid SPF record check failed. {support@pass.lavabit.com / 1.1.1.1}";

		// Neutral
		if (!errmsg && spf_check(&ip[0], NULLER("mx.lavabit.com"), NULLER("support@neutral.lavabit.com")) != -1)
			errmsg = "Neutral SPF record check failed. {support@neutral.lavabit.com / 1.1.1.1}";

		// Fail
		if (!errmsg && spf_check(&ip[0], NULLER("mx.lavabit.com"), NULLER("support@fail.lavabit.com")) != -2)
			errmsg = "Invalid SPF record check failed. {support@fail.lavabit.com / 1.1.1.1}";

		// Ensure the localhost always gets through
		if (!errmsg && spf_check(&ip[1], NULLER("mx.lavabit.com"), NULLER("support@fail.lavabit.com")) != 1)
			errmsg = "The localhost address matched a failure record instead of being whitelisted. {support@fail.lavabit.com / 127.0.0.1}";

	}

	log_unit("%10.10s\n", (!errmsg ? (status() ? "PASSED" : "SKIPPED") : "FAILED"));
	fail_unless(!errmsg, errmsg);

} END_TEST

//! Virus Test
START_TEST (check_virus_s) {

	chr_t *errmsg = NULL;
	log_unit("%-64.64s", "CHECKERS / VIRUS / SINGLE THREADED:");


	if (status() && magma.iface.virus.available) {
		log_disable();
		errmsg = check_virus_sthread(virus_check_data_path);
		log_enable();
	}

	log_unit("%10.10s\n", (!errmsg ? (status() && magma.iface.virus.available ? "PASSED" : "SKIPPED") : "FAILED"));
	fail_unless(!errmsg, errmsg);

}
END_TEST

START_TEST (check_dspam_mail_s) {

	bool_t outcome = true;

	log_unit("%-64.64s", "CHECKERS / DSPAM / MAIL / SINGLE THREADED:");

	if (status()) {
		outcome = check_dspam_mail_sthread(NULL);
	}

	log_unit("%10.10s\n", (outcome ? (status() ? "PASSED" : "SKIPPED") : "FAILED"));
	fail_unless(outcome, "check_dspam_mail_s failed");
}
END_TEST

START_TEST (check_dspam_bin_s) {

	bool_t outcome = true;

	log_unit("%-64.64s", "CHECKERS / DSPAM / BINARY / SINGLE THREADED:");

	if (status()) {
		log_disable();
		outcome = check_dspam_binary_sthread(NULL);
		log_enable();
	}

	log_unit("%10.10s\n", (outcome ? (status() ? "PASSED" : "SKIPPED") : "FAILED"));
	fail_unless(outcome, "check_dspam_bin_s failed");
}
END_TEST

Suite * suite_check_provide(void) {

	TCase *tc;
	Suite *s = suite_create("\tProviders");

	testcase(s, tc, "Compression LZO/S", check_compress_lzo_s);
	testcase(s, tc, "Compression LZO/M", check_compress_lzo_m);
	testcase(s, tc, "Compression ZLIB/S", check_compress_zlib_s);
	testcase(s, tc, "Compression ZLIB/M", check_compress_zlib_m);
	testcase(s, tc, "Compression BZIP/S", check_compress_bzip_s);
	testcase(s, tc, "Compression BZIP/M", check_compress_bzip_m);

	testcase(s, tc, "Cryptography RAND/S", check_rand_s);
	testcase(s, tc, "Cryptography RAND/M", check_rand_m);
	testcase(s, tc, "Cryptography ECIES/S", check_ecies_s);
	testcase(s, tc, "Cryptography DIGEST/S", check_digest_s);
	testcase(s, tc, "Cryptography SYMMETRIC/S", check_symmetric_s);
	testcase(s, tc, "Cryptography SCRAMBLE/S", check_scramble_s);

	// Tank functionality is temporarily disabled.

	do_tank_check = false;

	if (do_tank_check) {
		testcase(s, tc, "Tank LZO/S", check_tank_lzo_s);
		testcase(s, tc, "Tank LZO/M", check_tank_lzo_m);
		testcase(s, tc, "Tank ZLIB/S", check_tank_zlib_s);
		testcase(s, tc, "Tank ZLIB/M", check_tank_zlib_m);
		testcase(s, tc, "Tank BZIP/S", check_tank_bzip_s);
		testcase(s, tc, "Tank BZIP/M", check_tank_bzip_m);
	} else {
		log_unit("Skipping tank checks...\n");
	}

	if (do_spf_check) {
		testcase(s, tc, "SPF/S", check_spf_s);
	} else {
		log_unit("Skipping SPF checks...\n");
	}

	if (do_virus_check) {
		testcase(s, tc, "Virus/S", check_virus_s);
	} else {
		log_unit("Skipping virus checks...\n");
	}

	if (do_dspam_check) {
		testcase(s, tc, "DSPAM Mail/S", check_dspam_mail_s);
		testcase(s, tc, "DSPAM Binary/S", check_dspam_bin_s);
	} else {
		log_unit("Skipping DSPAM checks...\n");
	}

	return s;
}

