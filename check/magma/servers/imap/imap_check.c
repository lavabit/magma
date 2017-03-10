/*
 * @file /check/magma/pop/pop_check.c
 *
 * @brief IMAP interface test functions.
 */

#include "magma_check.h"

START_TEST (check_imap_network_basic_s) {

	log_disable();
	bool_t outcome = true;
	server_t *server = NULL;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (!(server = servers_get_by_protocol(IMAP, false))) {
		st_sprint(errmsg, "No IMAP servers were configured and available for testing.");
		outcome = false;
	}
	else if (status()) {
		outcome = check_imap_network_basic_sthread(errmsg, server->network.port);
	}

	log_test("IMAP / NETWORK / BASIC / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

Suite * suite_check_imap(void) {

	Suite *s = suite_create("\tIMAP");

	suite_check_testcase(s, "IMAP", "IMAP Network Basic/S", check_imap_network_basic_s);

	return s;
}
