/**
 * @file /check/magma/json/json_check.c
 *
 * @brief JSON Web API test functions.
 */

#include "magma_check.h"

START_TEST (check_json_network_basic_s) {

	log_disable();
	bool_t outcome = true;
	client_t *client = NULL;
	stringer_t *errmsg = MANAGEDBUF(1024);
	if (status() && !(client = client_connect("localhost/json", 10000))) {

		st_sprint(errmsg, "Failed to connect to JSON Web API endpoint.");
		outcome = false;
	}
	else {
		errmsg = NULL;
	}

	log_test("IMAP / NETWORK / FETCH / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

Suite * suite_check_json(void) {

	Suite *s = suite_create("\tJSON");

	suite_check_testcase(s, "JSON", "JSON Network Basic/S", check_json_network_basic_s);

	return s;
}
