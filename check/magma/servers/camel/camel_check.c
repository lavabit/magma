/**
 * @file /check/magma/camel/camel_check.c
 *
 * @brief Camelface test functions.
 */

#include "magma_check.h"

// LOW: Refactor for both HTTP and HTTPS.

START_TEST (check_camel_auth_s) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (status() && !check_camel_auth_sthread(false, errmsg)) outcome = false;
	else if (status() && outcome && !check_camel_auth_sthread(true, errmsg)) outcome = false;

	log_test("HTTP / NETWORK / CAMEL / LOGIN / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_camel_basic_s) {

	//log_disable();
	log_enable();
	bool_t outcome = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (status() && !check_camel_basic_sthread(false, errmsg)) outcome = false;

	log_test("HTTP / NETWORK / CAMEL / BASIC / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

Suite * suite_check_camel(void) {

	Suite *s = suite_create("\tCAMEL");

	suite_check_testcase(s, "CAMEL", "HTTP Network Camel Auth/S", check_camel_auth_s);
	suite_check_testcase(s, "CAMEL", "HTTP Network Camel Basic/S", check_camel_basic_s);

	return s;
}
