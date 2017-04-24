/**
 * @file /check/magma/pop/pop_check.c
 *
 * @brief POP interface test functions.
 */

#include "magma_check.h"

START_TEST (check_pop_network_basic_tcp_s) {

	log_disable();
	server_t *tcp = NULL;
	bool_t outcome = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (status() && !(tcp = servers_get_by_protocol(POP, false))) {
		st_sprint(errmsg, "No POP servers were configured to support TCP connections.");
		outcome = false;
	}
	else if (status() && !check_pop_network_basic_sthread(errmsg, tcp->network.port, false)) {
		outcome = false;
	}

	log_test("POP / NETWORK / BASIC / TCP / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_pop_network_basic_tls_s) {

	log_disable();
	server_t *tls = NULL;
	bool_t outcome = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (status() && !(tls = servers_get_by_protocol(POP, true))) {
		st_sprint(errmsg, "No POP servers were configured to support TLS connections.");
		outcome = false;
	}
	else if (status() && !check_pop_network_basic_sthread(errmsg, tls->network.port, true)) {
		outcome = false;
	}

	log_test("POP / NETWORK / BASIC / TLS / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_pop_network_stls_ad_s) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = MANAGEDBUF(1024);
	server_t *tcp_server = NULL, *tls_server = NULL;

	if (status() && (!(tcp_server = servers_get_by_protocol(POP, false)) || !(tls_server = servers_get_by_protocol(POP, true)))) {
		st_sprint(errmsg, "No POP servers were configured to support TLS connections.");
		outcome = false;
	}
	else if (status() && !check_pop_network_stls_ad_sthread(errmsg, tcp_server->network.port, tls_server->network.port)) {
		outcome = false;
	}

	log_test("POP / NETWORK / STLS ADVERTISEMENT / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

Suite * suite_check_pop(void) {

	Suite *s = suite_create("\tPOP");

	suite_check_testcase(s, "POP", "POP Network Basic / TCP/S", check_pop_network_basic_tcp_s);
	suite_check_testcase(s, "POP", "POP Network Basic / TLS/S", check_pop_network_basic_tls_s);
	suite_check_testcase(s, "POP", "POP Network STLS Ad/S", check_pop_network_stls_ad_s);

	return s;
}
