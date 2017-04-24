
/**
 * @file /check/magma/smtp/smtp_check.c
 *
 * @brief SMTP interface test functions.
 */

#include "magma_check.h"

START_TEST (check_smtp_network_basic_tcp_s) {

	log_disable();
	bool_t outcome = true;
	server_t *server = NULL;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (!(server = servers_get_by_protocol(SMTP, false))) {
		st_sprint(errmsg, "No SMTP servers were configured to support TCP connections.");
		outcome = false;
	}
	else if (status() && !check_smtp_network_basic_sthread(errmsg, server->network.port, false)) {
		outcome = false;
	}

	log_test("SMTP / NETWORK / BASIC / TCP / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_smtp_network_basic_tls_s) {

	log_disable();
	bool_t outcome = true;
	server_t *server = NULL;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (!(server = servers_get_by_protocol(SMTP, true))) {
		st_sprint(errmsg, "No SMTP servers were configured to support TLS connections.");
		outcome = false;
	}
	else if (status() && !check_smtp_network_basic_sthread(errmsg, server->network.port, true)) {
		outcome = false;
	}

	log_test("SMTP / NETWORK / BASIC / TLS / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_smtp_accept_store_message_s) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = MANAGEDBUF(2048);

	if (status()) outcome = check_smtp_accept_message_sthread(errmsg);

	log_test("SMTP / ACCEPT / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_smtp_checkers_greylist_s) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (status()) outcome = check_smtp_checkers_greylist_sthread(errmsg);

	log_test("SMTP / CHECKERS / GREYLIST / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_smtp_checkers_filters_s) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (status()) outcome = check_smtp_checkers_regex_sthread(errmsg);
	if (status() && outcome) outcome = check_smtp_checkers_filters_sthread(errmsg, SMTP_FILTER_ACTION_DELETE, -2);
	if (status() && outcome) outcome = check_smtp_checkers_filters_sthread(errmsg, SMTP_FILTER_ACTION_MOVE, 2);
	if (status() && outcome) outcome = check_smtp_checkers_filters_sthread(errmsg, SMTP_FILTER_ACTION_LABEL, 3);
	if (status() && outcome) outcome = check_smtp_checkers_filters_sthread(errmsg, SMTP_FILTER_ACTION_MARK_READ, 4);

	log_test("SMTP / CHECKERS / FILTERS / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_smtp_checkers_rbl_s) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (status()) outcome = check_smtp_checkers_rbl_sthread(errmsg);

	log_test("SMTP / CHECKERS / RBL / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_smtp_network_auth_plain_s) {

	log_disable();
	bool_t outcome = true;
	server_t *server = NULL;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (!(server = servers_get_by_protocol(SMTP, false))) {
		st_sprint(errmsg, "No SMTP servers were configured and available for testing.");
		outcome = false;
	}
	else if (status() && !check_smtp_network_auth_sthread(errmsg, server->network.port, false)) {
		outcome = false;
	}

	/// LOW: Add a variation of this test which takes place over TCP and thus fails specifically because the connection
	/// 	lacks transport security (aka TLS). In other words, test for valid credentials first, and that it works via TLS,
	/// 	before ensuring the same inputs fail via TCP.

	log_test("SMTP / NETWORK / AUTH PLAIN / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_smtp_network_auth_login_s) {

	log_disable();
	bool_t outcome = true;
	server_t *server = NULL;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (!(server = servers_get_by_protocol(SMTP, false))) {
		st_sprint(errmsg, "No SMTP servers were configured and available for testing.");
		outcome = false;
	}
	else if (status() && !check_smtp_network_auth_sthread(errmsg, server->network.port, true)) {
		outcome = false;
	}

	/// LOW: Add a variation of this test which takes place over TCP and thus fails specifically because the connection
	/// 	lacks transport security (aka TLS). In other words, test for valid credentials first, and that it works via TLS,
	/// 	before ensuring the same inputs fail via TCP.

	log_test("SMTP / NETWORK / AUTH LOGIN / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_smtp_network_outbound_quota_s) {

	log_disable();
	bool_t outcome = true;
	server_t *server = NULL;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (!(server = servers_get_by_protocol(SMTP, false))) {
		st_sprint(errmsg, "No SMTP servers were configured and available for testing.");
		outcome = false;
	}
	else if (status() && !check_smtp_network_outbound_quota_sthread(errmsg, server->network.port, false)) {
		outcome = false;
	}

	log_test("SMTP / NETWORK / OUTBOUND QUOTA / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_smtp_network_starttls_ad_s) {

	log_disable();
	bool_t outcome = true;
	server_t *tcp_server = NULL, *tls_server = NULL;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (!(tcp_server = servers_get_by_protocol(SMTP, false)) || !(tls_server = servers_get_by_protocol(SMTP, true))) {
		st_sprint(errmsg, "No SMTP servers were configured and available for testing for both TCP and TLS.");
		outcome = false;
	}
	else if (status() && !check_smtp_network_starttls_ad_sthread(errmsg, tcp_server->network.port, tls_server->network.port)) {
		outcome = false;
	}

	log_test("SMTP/ NETWORK / STARTTLS ADVERTISEMENT / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

Suite * suite_check_smtp(void) {

	Suite *s = suite_create("\tSMTP");

	suite_check_testcase(s, "SMTP", "SMTP Accept Message/S", check_smtp_accept_store_message_s);
	suite_check_testcase(s, "SMTP", "SMTP Checkers Greylist/S", check_smtp_checkers_greylist_s);
	suite_check_testcase(s, "SMTP", "SMTP Checkers Filters/S", check_smtp_checkers_filters_s);
	suite_check_testcase(s, "SMTP", "SMTP Checkers RBL", check_smtp_checkers_rbl_s);
	suite_check_testcase(s, "SMTP", "SMTP Network Basic/ TCP/S", check_smtp_network_basic_tcp_s);
	suite_check_testcase(s, "SMTP", "SMTP Network Basic/ TLS/S", check_smtp_network_basic_tls_s);
	suite_check_testcase(s, "SMTP", "SMTP Network Auth Plain/S", check_smtp_network_auth_plain_s);
	suite_check_testcase(s, "SMTP", "SMTP Network Auth Login/S", check_smtp_network_auth_login_s);
	suite_check_testcase(s, "SMTP", "SMTP Network Outbound Quota/S", check_smtp_network_outbound_quota_s);
	suite_check_testcase(s, "SMTP", "SMTP Network STARTTLS A/S", check_smtp_network_starttls_ad_s);

	return s;
}


