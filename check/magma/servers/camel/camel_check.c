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
	client_t *client = NULL;
	server_t *server = NULL;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (status() && !(server = servers_get_by_protocol(HTTP, false))) {
		st_sprint(errmsg, "No HTTP servers were configured to support TLS connections.");
		outcome = false;
	}
	else if (!(client = client_connect("localhost", server->network.port))) {

		st_sprint(errmsg, "Failed to connect client securely to HTTP server.");
		outcome = false;
	}
	else if (!check_camel_auth_sthread(client, errmsg)){
		outcome = false;
	}

	log_test("CAMEL / LOGIN / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_camel_basic_s) {

	log_disable();
	bool_t outcome = true;
	client_t *client = NULL;
	server_t *server = NULL;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (status() && !(server = servers_get_by_protocol(HTTP, false))) {
		st_sprint(errmsg, "No HTTP servers were configured to support TLS connections.");
		outcome = false;
	}
	else if (!(client = client_connect("localhost", server->network.port))) {

		st_sprint(errmsg, "Failed to connect client securely to HTTP server.");
		outcome = false;
	}
	else if (!check_camel_basic_sthread(client, errmsg)){
		outcome = false;
	}

	log_test("CAMEL / BASIC / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

Suite * suite_check_camel(void) {

	Suite *s = suite_create("\tCAMEL");

	suite_check_testcase(s, "CAMEL", "Camel Auth/S", check_camel_auth_s);
	suite_check_testcase(s, "CAMEL", "Camel Basic/S", check_camel_basic_s);

	return s;
}
