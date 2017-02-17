
/**
 * @file /check/magma/objects/objects_check.c
 *
 * @brief Low level object utility tests.
 */

#include "magma_check.h"


START_TEST (check_object_serials_s) {

	log_disable();
	uint64_t num = 1;
	bool_t result = true;
	stringer_t *errmsg = NULL;

	// Flush the cache, otherwise we won't get what we expect back.
	cache_flush();

	for (uint32_t i = 0; i < 128 && result; i++) {

		num += rand_get_uint8() + 1;

		// Users
		if (serial_get(OBJECT_USER, num) != 0 ||
			serial_increment(OBJECT_USER, num) != 1 ||
			serial_increment(OBJECT_USER, num) != 2 ||
			serial_get(OBJECT_USER, num) != 2 ||
			serial_get(OBJECT_USER, num) != 2 ||
			serial_reset(OBJECT_USER, num) != 1 ||
			serial_get(OBJECT_USER, num) != 1 ||
			serial_increment(OBJECT_USER, num) != 2 ||
			serial_increment(OBJECT_USER, num) != 3) {
			errmsg = NULLER("The interface for handling user serial numbers failed.");
			result = false;
		}

		// Folders
		if (result && (serial_get(OBJECT_FOLDERS, num) != 0 ||
			serial_increment(OBJECT_FOLDERS, num) != 1 ||
			serial_increment(OBJECT_FOLDERS, num) != 2 ||
			serial_get(OBJECT_FOLDERS, num) != 2 ||
			serial_get(OBJECT_FOLDERS, num) != 2 ||
			serial_reset(OBJECT_FOLDERS, num) != 1 ||
			serial_get(OBJECT_FOLDERS, num) != 1 ||
			serial_increment(OBJECT_FOLDERS, num) != 2 ||
			serial_increment(OBJECT_FOLDERS, num) != 3)) {
			errmsg = NULLER("The interface for handling folder serial numbers failed.");
			result = false;
		}

		// Messages
		if (result && (serial_get(OBJECT_MESSAGES, num) != 0 ||
			serial_increment(OBJECT_MESSAGES, num) != 1 ||
			serial_increment(OBJECT_MESSAGES, num) != 2 ||
			serial_get(OBJECT_MESSAGES, num) != 2 ||
			serial_get(OBJECT_MESSAGES, num) != 2 ||
			serial_reset(OBJECT_MESSAGES, num) != 1 ||
			serial_get(OBJECT_MESSAGES, num) != 1 ||
			serial_increment(OBJECT_MESSAGES, num) != 2 ||
			serial_increment(OBJECT_MESSAGES, num) != 3)) {
			errmsg = NULLER("The interface for handling message serial numbers failed.");
			result = false;
		}
	}

	// Check the edge cases.
	if (result && (serial_get(OBJECT_MESSAGES, UINT64_MAX) != 0 ||
		serial_increment(OBJECT_MESSAGES, UINT64_MAX) != 1 ||
		serial_increment(OBJECT_MESSAGES, UINT64_MAX) != 2 ||
		serial_get(OBJECT_MESSAGES, UINT64_MAX) != 2 ||
		serial_reset(OBJECT_MESSAGES, UINT64_MAX) != 1)) {
		errmsg = NULLER("The interface for handling serial numbers failed at an edge case.");
		result = false;
	}

  log_test("OBJECTS / SERIALS / SINGLE THREADED:", errmsg);
  ck_assert_msg(result, st_char_get(errmsg));
}
END_TEST

START_TEST (check_warehouse_domains_s) {

	log_disable();
	bool_t result = true;
	stringer_t *errmsg = NULL;

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
		errmsg = NULLER("Domain checks failed.");
		result = false;
	}

	log_test("OBJECTS / WAREHOUSE / DOMAINS / SINGLE THREADED:", errmsg);
	ck_assert_msg(result, st_char_get(errmsg));
}
END_TEST

Suite * suite_check_objects(void) {

	TCase *tc;
	Suite *s = suite_create("\tObjects");

	/*testcase(s, tc, "Address Processing/S", check_credential_address_s);
	testcase(s, tc, "Username Processing/S", check_credential_username_s);
	testcase(s, tc, "Credential Processing/S", check_credential_mail_creation_s);
	testcase(s, tc, "Credential Processing/S", check_credential_auth_creation_s);*/

	testcase(s, tc, "Object Serials/S", check_object_serials_s);
	testcase(s, tc, "Object Warehouse Domains/S", check_warehouse_domains_s);

	return s;
}
