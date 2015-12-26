
/**
 * @file /check/objects/objects_check.c
 *
 * @brief Low level object utility tests.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma_check.h"


START_TEST (check_object_serials_s)
	{

	uint64_t num = 1;
	bool_t outcome = true;

	log_unit("%-64.64s", "OBJECTS / SERIALS / SINGLE THREADED:");

	// Flush the cache, otherwise we won't get what we expect back.
	cache_flush();

	for (uint32_t i = 0; i < 128; i++) {

		num += rand_get_uint8() + 1;

		// Users
		if (serial_get(OBJECT_USER, num) != 0) outcome = false;
		else if (serial_increment(OBJECT_USER, num) != 1) outcome = false;
		else if (serial_increment(OBJECT_USER, num) != 2) outcome = false;
		else if (serial_get(OBJECT_USER, num) != 2) outcome = false;
		else if (serial_get(OBJECT_USER, num) != 2) outcome = false;
		else if (serial_reset(OBJECT_USER, num) != 1) outcome = false;
		else if (serial_get(OBJECT_USER, num) != 1) outcome = false;
		else if (serial_increment(OBJECT_USER, num) != 2) outcome = false;
		else if (serial_increment(OBJECT_USER, num) != 3) outcome = false;

		// Folders
		else if (serial_get(OBJECT_FOLDERS, num) != 0) outcome = false;
		else if (serial_increment(OBJECT_FOLDERS, num) != 1) outcome = false;
		else if (serial_increment(OBJECT_FOLDERS, num) != 2) outcome = false;
		else if (serial_get(OBJECT_FOLDERS, num) != 2) outcome = false;
		else if (serial_get(OBJECT_FOLDERS, num) != 2) outcome = false;
		else if (serial_reset(OBJECT_FOLDERS, num) != 1) outcome = false;
		else if (serial_get(OBJECT_FOLDERS, num) != 1) outcome = false;
		else if (serial_increment(OBJECT_FOLDERS, num) != 2) outcome = false;
		else if (serial_increment(OBJECT_FOLDERS, num) != 3) outcome = false;

		// Messages
		else if (serial_get(OBJECT_MESSAGES, num) != 0) outcome = false;
		else if (serial_increment(OBJECT_MESSAGES, num) != 1) outcome = false;
		else if (serial_increment(OBJECT_MESSAGES, num) != 2) outcome = false;
		else if (serial_get(OBJECT_MESSAGES, num) != 2) outcome = false;
		else if (serial_get(OBJECT_MESSAGES, num) != 2) outcome = false;
		else if (serial_reset(OBJECT_MESSAGES, num) != 1) outcome = false;
		else if (serial_get(OBJECT_MESSAGES, num) != 1) outcome = false;
		else if (serial_increment(OBJECT_MESSAGES, num) != 2) outcome = false;
		else if (serial_increment(OBJECT_MESSAGES, num) != 3) outcome = false;
	}

	// Check the edge cases.
	if (outcome && serial_get(OBJECT_MESSAGES, UINT64_MAX) != 0) outcome = false;
	else if (outcome && serial_increment(OBJECT_MESSAGES, UINT64_MAX) != 1) outcome = false;
	else if (outcome && serial_increment(OBJECT_MESSAGES, UINT64_MAX) != 2) outcome = false;
	else if (outcome && serial_get(OBJECT_MESSAGES, UINT64_MAX) != 2) outcome = false;
	else if (outcome && serial_reset(OBJECT_MESSAGES, UINT64_MAX) != 1) outcome = false;

	log_unit("%10.10s\n", (outcome ? (status() ? "PASSED" : "SKIPPED") : "FAILED"));
	fail_unless(outcome, "check_object_serials_s failed");
}
END_TEST

START_TEST (check_warehouse_domains_s)
{
	char *errmsg = NULL;
	bool_t outcome = true;

	log_unit("%-64.64s", "OBJECTS / WAREHOUSE / DOMAINS / SINGLE THREADED:");

	if (status() && (domain_wildcard(CONSTANT("lavabit.com")) != 0 || domain_dkim(CONSTANT("lavabit.com")) != 1 ||	domain_spf(CONSTANT("lavabit.com")) != 1 ||
		domain_wildcard(CONSTANT("mailshack.com")) != 0 || domain_dkim(CONSTANT("mailshack.com")) != 1 ||	domain_spf(CONSTANT("mailshack.com")) != 1 ||
		domain_wildcard(CONSTANT("nerdshack.com")) != 0 || domain_dkim(CONSTANT("nerdshack.com")) != 1 ||	domain_spf(CONSTANT("nerdshack.com")) != 1 ||
		domain_wildcard(CONSTANT("squeak-seo.com")) != 1 || domain_dkim(CONSTANT("squeak-seo.com")) != 0 ||	domain_spf(CONSTANT("squeak-seo.com")) != 1 ||
		domain_wildcard(CONSTANT("texasteenage.org")) != 1 || domain_dkim(CONSTANT("texasteenage.org")) != 0 ||	domain_spf(CONSTANT("texasteenage.org")) != 0 ||
		domain_wildcard(CONSTANT("ronweb.net")) != 1 || domain_dkim(CONSTANT("ronweb.net")) != 1 ||	domain_spf(CONSTANT("ronweb.net")) != 1 ||
		domain_wildcard(CONSTANT("slashdot.org")) != -1 || domain_dkim(CONSTANT("slashdot.org")) != -1 ||	domain_spf(CONSTANT("slashdot.org")) != -1 ||

		domain_wildcard(NULLER("lavabit.com")) != 0 || domain_dkim(NULLER("lavabit.com")) != 1 ||	domain_spf(NULLER("lavabit.com")) != 1 ||
		domain_wildcard(NULLER("mailshack.com")) != 0 || domain_dkim(NULLER("mailshack.com")) != 1 ||	domain_spf(NULLER("mailshack.com")) != 1 ||
		domain_wildcard(NULLER("nerdshack.com")) != 0 || domain_dkim(NULLER("nerdshack.com")) != 1 ||	domain_spf(NULLER("nerdshack.com")) != 1 ||
		domain_wildcard(NULLER("squeak-seo.com")) != 1 || domain_dkim(NULLER("squeak-seo.com")) != 0 ||	domain_spf(NULLER("squeak-seo.com")) != 1 ||
		domain_wildcard(NULLER("texasteenage.org")) != 1 || domain_dkim(NULLER("texasteenage.org")) != 0 ||	domain_spf(NULLER("texasteenage.org")) != 0 ||
		domain_wildcard(NULLER("ronweb.net")) != 1 || domain_dkim(NULLER("ronweb.net")) != 1 ||	domain_spf(NULLER("ronweb.net")) != 1 ||
		domain_wildcard(NULLER("slashdot.org")) != -1 || domain_dkim(NULLER("slashdot.org")) != -1 ||	domain_spf(NULLER("slashdot.org")) != -1 ||

		domain_wildcard(PLACER("lavabit.com", 11)) != 0 || domain_dkim(PLACER("lavabit.com", 11)) != 1 ||	domain_spf(PLACER("lavabit.com", 11)) != 1 ||
		domain_wildcard(PLACER("mailshack.com", 13)) != 0 || domain_dkim(PLACER("mailshack.com", 13)) != 1 ||	domain_spf(PLACER("mailshack.com", 13)) != 1 ||
		domain_wildcard(PLACER("nerdshack.com", 13)) != 0 || domain_dkim(PLACER("nerdshack.com", 13)) != 1 ||	domain_spf(PLACER("nerdshack.com", 13)) != 1 ||
		domain_wildcard(PLACER("squeak-seo.com", 14)) != 1 || domain_dkim(PLACER("squeak-seo.com", 14)) != 0 ||	domain_spf(PLACER("squeak-seo.com", 14)) != 1 ||
		domain_wildcard(PLACER("texasteenage.org", 16)) != 1 || domain_dkim(PLACER("texasteenage.org", 16)) != 0 ||	domain_spf(PLACER("texasteenage.org", 16)) != 0 ||
		domain_wildcard(PLACER("ronweb.net", 10)) != 1 || domain_dkim(PLACER("ronweb.net", 10)) != 1 ||	domain_spf(PLACER("ronweb.net", 10)) != 1 ||
		domain_wildcard(PLACER("slashdot.org", 12)) != -1 || domain_dkim(PLACER("slashdot.org", 12)) != -1 ||	domain_spf(PLACER("slashdot.org", 12)) != -1)) {
		errmsg = "Domain checks failed.";
		outcome = false;
	}

	outcome = errmsg ? false : true;
	log_unit("%10.10s\n", (outcome ? "PASSED" : status() ? "FAILED" : "SKIPPED"));
	fail_unless(outcome, errmsg);

}
END_TEST

START_TEST (check_credential_address_s) {

	stringer_t *cred;
	char *errmsg = NULL;

	log_unit("%-64.64s", "OBJECTS / USERS / CERDENTIALS / ADDRESS / SINGLE THREADED:");

	if (!errmsg && (!(cred = credential_address(CONSTANT("TEST@DOMAIN.COM"))) || st_cmp_ci_eq(cred, CONSTANT("test@domain.com")))) {
		errmsg = "Address boiler failed [1].";
		//printf("r1 = %lx\n", (unsigned long) cred);	if (cred) printf("xxx: [%.*s] [%u]\n", (int)st_length_get(cred), st_char_get(cred), (int)st_length_get(cred));
	}

	st_cleanup(cred);
	cred = NULL;

	if (!errmsg && (!(cred = credential_address(CONSTANT("  TEST  @  DOMAIN.COM  "))) || st_cmp_ci_eq(cred, CONSTANT("test@domain.com")))) {
		errmsg = "Address boiler failed [2].";
		//printf("r2 = %lx\n", (unsigned long) cred);	if (cred) printf("xxx: [%.*s] [%u]\n", (int)st_length_get(cred), st_char_get(cred), (int)st_length_get(cred));
	}

	st_cleanup(cred);
	cred = NULL;

	if (!errmsg && (!(cred = credential_address(CONSTANT("test.case+tag@sub.domain.com"))) || st_cmp_ci_eq(cred, CONSTANT("test_case@sub.domain.com")))) {
		errmsg = "Address boiler failed [5].";
	}

	st_cleanup(cred);
	cred = NULL;

	if (!errmsg && (!(cred = credential_address(CONSTANT("TEST@DOMAIN.COM"))) || st_cmp_ci_eq(cred, CONSTANT("test@domain.com")))) {
		errmsg = "Address boiler failed [2].";
		//printf("rx2 = %lx\n", (unsigned long) cred);	if (cred) printf("xxx: [%.*s] [%u]\n", (int)st_length_get(cred), st_char_get(cred), (int)st_length_get(cred));
	}

	st_cleanup(cred);
	cred = NULL;

	if (!errmsg && (!(cred = credential_address(CONSTANT("  TEST  @  DOMAIN.COM  "))) || st_cmp_ci_eq(cred, CONSTANT("test@domain.com")))) {
		errmsg = "Address boiler failed [5].";
	}

	st_cleanup(cred);
	cred = NULL;

	if (!errmsg && (!(cred = credential_address(CONSTANT("test.case+tag@sub.domain.com"))) || st_cmp_ci_eq(cred, CONSTANT("test_case@sub.domain.com")))) {
		errmsg = "Address boiler failed [11].";
	}

	st_cleanup(cred);
	cred = NULL;

	log_unit("%10.10s\n", (!status() ? "SKIPPED" : !errmsg ? "PASSED" : "FAILED"));
	fail_unless(!errmsg, errmsg);
} END_TEST

START_TEST (check_credential_username_s) {

	stringer_t *cred;
	char *errmsg = NULL;

	log_unit("%-64.64s", "OBJECTS / USERS / CREDENTIALS / USERNAME / SINGLE THREADED:");

	if (!(cred = credential_username(CONSTANT("TEST"))) || st_cmp_ci_eq(cred, CONSTANT("test"))) {
		errmsg = "Username boiler failed [1].";
		//printf("rx1 = %lx\n", (unsigned long) cred);	if (cred) printf("xxx: [%.*s] [%u]\n", (int)st_length_get(cred), st_char_get(cred), (int)st_length_get(cred));
	}

	st_cleanup(cred);
	cred = NULL;

	if (!errmsg && (!(cred = credential_username(CONSTANT("TEST.CASE@LAVABIT.COM"))) || st_cmp_ci_eq(cred, CONSTANT("test_case")))) {
		errmsg = "Username boiler failed [3].";
	}

	st_cleanup(cred);
	cred = NULL;

	if (!errmsg && (!(cred = credential_username(CONSTANT("test+tag@nerdshack.com"))) || !st_cmp_ci_eq(cred, CONSTANT("test")) ||
		st_cmp_ci_eq(cred, CONSTANT("test@nerdshack.com")))) {
		errmsg = "Username boiler failed [4].";
	}

	st_cleanup(cred);
	cred = NULL;

	if (!errmsg && (!(cred = credential_username(CONSTANT("test.case+tag@sub.domain.com"))) || !st_cmp_ci_eq(cred, CONSTANT("test_case")) ||
		st_cmp_ci_eq(cred, CONSTANT("test_case@sub.domain.com")))) {
		errmsg = "Username boiler failed [6].";
	}

	st_cleanup(cred);
	cred = NULL;

	if (!errmsg && (!(cred = credential_username(CONSTANT("TEST@DOMAIN.COM"))) || !st_cmp_ci_eq(cred, CONSTANT("test")) || st_cmp_ci_eq(cred, CONSTANT("test@domain.com")))) {
			errmsg = "Username boiler failed [3].";
			//printf("rx2 = %lx\n", (unsigned long) cred);	if (cred) printf("xxx: [%.*s] [%u]\n", (int)st_length_get(cred), st_char_get(cred), (int)st_length_get(cred));
		}

		st_cleanup(cred);
		cred = NULL;

	if (!errmsg && (!(cred = credential_username(CONSTANT("  TEST  "))) || st_cmp_ci_eq(cred, CONSTANT("test")))) {
		errmsg = "Username boiler failed [4].";
	}

	st_cleanup(cred);
	cred = NULL;

	if (!errmsg && (!(cred = credential_username(CONSTANT("  TEST  @  DOMAIN.COM  "))) || !st_cmp_ci_eq(cred, CONSTANT("test")) || st_cmp_ci_eq(cred, CONSTANT("test@domain.com")))) {
			errmsg = "Username boiler failed [6].";
		}

		st_cleanup(cred);
		cred = NULL;

	if (!errmsg && (!(cred = credential_username(CONSTANT("TEST.CASE"))) || st_cmp_ci_eq(cred, CONSTANT("test_case")))) {
		errmsg = "Username boiler failed [7].";
	}

	st_cleanup(cred);
	cred = NULL;

	if (!errmsg && (!(cred = credential_username(CONSTANT("TEST.CASE@LAVABIT.COM"))) || st_cmp_ci_eq(cred, CONSTANT("test_case")) || !st_cmp_ci_eq(cred, CONSTANT("test_case@lavabit.com")))) {
		errmsg = "Username boiler failed [8].";
	}

	st_cleanup(cred);
	cred = NULL;

	if (!errmsg && (!(cred = credential_username(CONSTANT("test+tag"))) || st_cmp_ci_eq(cred, CONSTANT("test")))) {
		errmsg = "Username boiler failed [9].";
	}

	st_cleanup(cred);
	cred = NULL;

	if (!errmsg && (!(cred = credential_username(CONSTANT("test+tag@nerdshack.com"))) || !st_cmp_ci_eq(cred, CONSTANT("test")) || st_cmp_ci_eq(cred, CONSTANT("test@nerdshack.com")))) {
		errmsg = "Username boiler failed [10].";
	}

	st_cleanup(cred);
	cred = NULL;

	log_unit("%10.10s\n", (!status() ? "SKIPPED" : !errmsg ? "PASSED" : "FAILED"));
	fail_unless(!errmsg, errmsg);
} END_TEST

START_TEST (check_credential_mail_creation_s) {

	credential_t *cred;
	char *errmsg = NULL;

	log_unit("%-64.64s", "OBJECTS / USERS / CREDENTIALS / MAIL CREATION / SINGLE THREADED:");

	if ((cred = credential_alloc_mail(CONSTANT("ladar")))) {
		errmsg = "Credential creation should have failed but didn't.";
	}

	if (cred) {
		credential_free(cred);
		cred = NULL;
	}

	if ((cred = credential_alloc_mail(CONSTANT("ladar@")))) {
		errmsg = "Credential creation should have failed but didn't.";
	}

	if (cred) {
		credential_free(cred);
		cred = NULL;
	}

	if ((cred = credential_alloc_mail(CONSTANT("@lavabit.com")))) {
		errmsg = "Credential creation should have failed but didn't.";
	}

	if (cred) {
		credential_free(cred);
		cred = NULL;
	}

	if (!errmsg && !(cred = credential_alloc_mail(CONSTANT("ladar+tag@lavabit.com")))) {
		errmsg = "Credential creation failed.";
	}
	else if (!errmsg && (st_cmp_cs_eq(cred->mail.address, CONSTANT("ladar@lavabit.com")) || st_cmp_cs_eq(cred->mail.domain, CONSTANT("lavabit.com")))) {
		errmsg = "The credential password hash doesn't match the expected value.";
	}

	if (cred) {
		credential_free(cred);
	}

	if (!errmsg && !(cred = credential_alloc_mail(CONSTANT("LADAR+TAG@LAVABIT.COM")))) {
		errmsg = "Credential creation failed.";
	}
	else if (!errmsg && (st_cmp_cs_eq(cred->mail.address, CONSTANT("ladar@lavabit.com")) || st_cmp_cs_eq(cred->mail.domain, CONSTANT("lavabit.com")))) {
		errmsg = "The credential password hash doesn't match the expected value.";
	}

	if (cred) {
		credential_free(cred);
	}

	if (!errmsg && !(cred = credential_alloc_mail(CONSTANT("ladar+tag@nerdshack.com")))) {
		errmsg = "Credential creation failed.";
	}
	else if (!errmsg && (st_cmp_cs_eq(cred->mail.address, CONSTANT("ladar@nerdshack.com")) || st_cmp_cs_eq(cred->mail.domain, CONSTANT("nerdshack.com")))) {
		errmsg = "The credential password hash doesn't match the expected value.";
	}

	if (cred) {
		credential_free(cred);
	}

	if (!errmsg && !(cred = credential_alloc_mail(CONSTANT("LADAR+TAG@NERDSHACK.COM")))) {
		errmsg = "Credential creation failed.";
	}
	else if (!errmsg && (st_cmp_cs_eq(cred->mail.address, CONSTANT("ladar@nerdshack.com")) || st_cmp_cs_eq(cred->mail.domain, CONSTANT("nerdshack.com")))) {
		errmsg = "The credential password hash doesn't match the expected value.";
	}

	if (cred) {
		credential_free(cred);
	}

	if (!errmsg && !(cred = credential_alloc_mail(CONSTANT("ladar+tag@mailshack.com")))) {
		errmsg = "Credential creation failed.";
	}
	else if (!errmsg && (st_cmp_cs_eq(cred->mail.address, CONSTANT("ladar@mailshack.com")) || st_cmp_cs_eq(cred->mail.domain, CONSTANT("mailshack.com")))) {
		errmsg = "The credential password hash doesn't match the expected value.";
	}

	if (cred) {
		credential_free(cred);
	}

	if (!errmsg && !(cred = credential_alloc_mail(CONSTANT("LADAR+TAG@MAILSHACK.COM")))) {
		errmsg = "Credential creation failed.";
	}
	else if (!errmsg && (st_cmp_cs_eq(cred->mail.address, CONSTANT("ladar@mailshack.com")) || st_cmp_cs_eq(cred->mail.domain, CONSTANT("mailshack.com")))) {
		errmsg = "The credential password hash doesn't match the expected value.";
	}

	if (cred) {
		credential_free(cred);
	}

	if (!errmsg && !(cred = credential_alloc_mail(CONSTANT("user+tag@domain.com")))) {
		errmsg = "Credential creation failed.";
	}
	else if (!errmsg && (st_cmp_cs_eq(cred->mail.address, CONSTANT("user@domain.com")) || st_cmp_cs_eq(cred->mail.domain, CONSTANT("domain.com")))) {
		errmsg = "The credential password hash doesn't match the expected value.";
	}

	if (cred) {
		credential_free(cred);
	}

	if (!errmsg && !(cred = credential_alloc_mail(CONSTANT("USER+TAG@DOMAIN.COM")))) {
		errmsg = "Credential creation failed.";
	}
	else if (!errmsg && (st_cmp_cs_eq(cred->mail.address, CONSTANT("user@domain.com")) || st_cmp_cs_eq(cred->mail.domain, CONSTANT("domain.com")))) {
		errmsg = "The credential password hash doesn't match the expected value.";
	}

	if (cred) {
		credential_free(cred);
	}

	log_unit("%10.10s\n", (!status() ? "SKIPPED" : !errmsg ? "PASSED" : "FAILED"));
	fail_unless(!errmsg, errmsg);
} END_TEST

START_TEST (check_credential_auth_creation_s) {

	credential_t *cred;
	char *errmsg = NULL;

	log_unit("%-64.64s", "OBJECTS / USERS / CREDENTIALS / AUTH CREATION / SINGLE THREADED:");

	if (!(cred = credential_alloc_auth(CONSTANT("ladar")))) {
		errmsg = "Credential allocation failed.";
	}
	else if(!credential_calc_auth(cred, CONSTANT("test"), NULL)) {
		errmsg = "Credential calculation failed.";
	}
	else if (st_cmp_cs_eq(cred->auth.password, CONSTANT("46c3c0f5c777aacbdb0c25b14d6889b98efa62fa0ae551ec067d7aa126392805e3e3a2ce07d36" \
		"df7e715e24f35c88105fff5a9eebff0532f990644cf07a4751f"))) {
		errmsg = "The credential password hash doesn't match the expected value.";
	}

	if (cred) {
		credential_free(cred);
		cred = NULL;
	}

	if (!errmsg && !(cred = credential_alloc_auth(CONSTANT("ladar@lavabit.com")))) {
		errmsg = "Credential allocation failed.";
	}
	else if(!errmsg && !credential_calc_auth(cred, CONSTANT("test"), NULL)) {
		errmsg = "Credential calculation failed.";
	}
	else if (!errmsg && st_cmp_cs_eq(cred->auth.password, CONSTANT("46c3c0f5c777aacbdb0c25b14d6889b98efa62fa0ae551ec067d7aa126392805e3e3a2ce07d36" \
		"df7e715e24f35c88105fff5a9eebff0532f990644cf07a4751f"))) {
		errmsg = "The credential password hash doesn't match the expected value.";
	}

	if (cred) {
		credential_free(cred);
	}

	log_unit("%10.10s\n", (!status() ? "SKIPPED" : !errmsg ? "PASSED" : "FAILED"));
	fail_unless(!errmsg, errmsg);
} END_TEST

Suite * suite_check_objects(void) {

	TCase *tc;
	Suite *s = suite_create("\tObjects");

	testcase(s, tc, "Address Processing/S", check_credential_address_s);
	testcase(s, tc, "Username Processing/S", check_credential_username_s);
	testcase(s, tc, "Credential Processing/S", check_credential_mail_creation_s);
	testcase(s, tc, "Credential Processing/S", check_credential_auth_creation_s);
	testcase(s, tc, "Object Serials/S", check_object_serials_s);
	testcase(s, tc, "Object Warehouse Domains/S", check_warehouse_domains_s);

	return s;
}
