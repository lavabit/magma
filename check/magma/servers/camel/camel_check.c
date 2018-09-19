/**
 * @file /check/magma/camel/camel_check.c
 *
 * @brief Camelface test functions.
 */

#include "magma_check.h"

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

	log_disable();
	int64_t result = 0;
	bool_t outcome = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	// Reset the alerts and make the insecure attempt.
	if (status() && (result = sql_write(PLACER("UPDATE `Alerts` SET `acknowledged` = NULL WHERE `usernum` < 10;", 63))) < 0) {
		st_sprint(errmsg, "Unable to configure the alerts for the basic camelface test. { result = %li }", result);
		outcome = false;
	}
	else if (status() && !check_camel_basic_sthread(false, errmsg)) {
		outcome = false;
	}

	// Reset the alerts again, and make the secure attempt.
	else if (status() && (result = sql_write(PLACER("UPDATE `Alerts` SET `acknowledged` = NULL WHERE `usernum` < 10;", 63))) < 0) {
		st_sprint(errmsg, "Unable to configure the alerts for the basic camelface test. { result = %li }", result);
		outcome = false;
	}
	else if (status() && outcome && !check_camel_basic_sthread(true, errmsg)) {
		outcome = false;
	}

	log_test("HTTP / NETWORK / CAMEL / BASIC / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

Suite * suite_check_camel(void) {

	Suite *s = suite_create("\tCAMEL");

	suite_check_testcase(s, "HTTP CAMEL", "HTTP Network Camel Auth/S", check_camel_auth_s);

	// The Camel tests will occassionally timeout on slower systems, in part because of how many camelface requests
	// are made inside this single "testcase" and in part because the camelface makes heavy use of secure memory,
	// which has significantly more overhead for each allocate/free oepration. As a result we give the camelface
	// tests twice as much time to complete when applicable. (The timeout is disabled when a debugger, or profiler
	// is in use.)
	suite_check_testcase_timeout(s, "HTTP CAMEL", "HTTP Network Camel Basic/S", check_camel_basic_s, ( case_timeout * 2 ));

	return s;
}
