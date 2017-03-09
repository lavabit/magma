/**
 * @file /check/magma/pop/pop_check.c
 *
 * @brief POP interface test functions.
 */

#include "magma_check.h"

START_TEST (check_pop_network_simple_s) {

	log_disable();
	bool_t outcome = true;
	server_t *server = NULL;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (!(server = servers_get_by_protocol(POP, false))) {
		st_sprint(errmsg, "No POP servers were configured and available for testing.");
		outcome = false;
	}
	else if (status()) {
		outcome = check_pop_network_simple_sthread(errmsg, server->network.port);
	}

	log_test("POP / NETWORK / BASIC / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

Suite * suite_check_pop(void) {

	Suite *s = suite_create("\tPOP");

	suite_check_testcase(s, "POP", "POP Network Basic/S", check_pop_network_basic_s);

	return s;
}
