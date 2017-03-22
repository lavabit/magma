/*
 * @file /check/magma/pop/pop_check.c
 *
 * @brief IMAP interface test functions.
 */

#include "magma_check.h"

START_TEST (check_imap_network_basic_tcp_s) {

	log_disable();
	bool_t outcome = true;
	server_t *server = NULL;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (!(server = servers_get_by_protocol(IMAP, false))) {
		st_sprint(errmsg, "No IMAP servers were configured to support TCP connections.");
		outcome = false;
	}
	else if (status() && !check_imap_network_basic_sthread(errmsg, server->network.port, false)) {
		outcome = false;
	}
	else {
		errmsg = NULL;
	}

	log_test("IMAP / NETWORK / BASIC / TCP / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_imap_network_basic_tls_s) {

	log_disable();
	bool_t outcome = true;
	server_t *server = NULL;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (!(server = servers_get_by_protocol(IMAP, true))) {
		st_sprint(errmsg, "No IMAP servers were configured to support TLS connections.");
		outcome = false;
	}
	else if (status() && !check_imap_network_basic_sthread(errmsg, server->network.port, true)) {
		outcome = false;
	}
	else {
		errmsg = NULL;
	}

	log_test("IMAP / NETWORK / BASIC / TLS / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

Suite * suite_check_imap(void) {

	Suite *s = suite_create("\tIMAP");

	suite_check_testcase(s, "IMAP", "IMAP Network Basic/ TCP/S", check_imap_network_basic_tcp_s);
	suite_check_testcase(s, "IMAP", "IMAP Network Basic/ TLS/S", check_imap_network_basic_tls_s);

	return s;
}
