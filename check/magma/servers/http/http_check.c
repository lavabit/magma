
/**
 * @file /check/magma/http_check.c
 *
 * @brief HTTP interface test functions.
 */

#include "magma_check.h"

START_TEST (check_http_network_basic_tcp_s) {

	log_disable();
	bool_t outcome = true;
	server_t *server = NULL;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (!(server = servers_get_by_protocol(HTTP, false))) {
		st_sprint(errmsg, "No HTTP servers were configured to support TCP connections.");
		outcome = false;
	}
	else if (status() && !check_http_network_basic_sthread(errmsg, server->network.port, false)) {
		outcome = false;
	}

	log_test("HTTP / NETWORK / BASIC / TCP / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_http_network_basic_tls_s) {

	log_disable();
	bool_t outcome = true;
	server_t *server = NULL;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (!(server = servers_get_by_protocol(HTTP, true))) {
		st_sprint(errmsg, "No HTTP servers were configured to support TLS connections.");
		outcome = false;
	}
	else if (status() && !check_http_network_basic_sthread(errmsg, server->network.port, true)) {
		outcome = false;
	}

	log_test("HTTP / NETWORK / BASIC / TLS / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_http_network_options_s) {

	log_disable();
	bool_t outcome = true;
	server_t *server = NULL;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (!(server = servers_get_by_protocol(HTTP, true))) {
		st_sprint(errmsg, "No HTTP servers were configured to support TCP connections.");
		outcome = false;
	}
	else if (status() && !check_http_network_options_sthread(errmsg, server->network.port, true)) {
		outcome = false;
	}

	log_test("HTTP / NETWORK / OPTIONS / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

Suite * suite_check_http(void) {

	Suite *s = suite_create("\tHTTP");

	suite_check_testcase(s, "HTTP", "HTTP Network Basic/ TCP/S", check_http_network_basic_tcp_s);
	suite_check_testcase(s, "HTTP", "HTTP Network Basic/ TLS/S", check_http_network_basic_tls_s);
	suite_check_testcase(s, "HTTP", "HTTP Network Options/S", check_http_network_options_s);

	return s;
}


