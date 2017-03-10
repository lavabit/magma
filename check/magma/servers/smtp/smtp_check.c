
/**
 * @file /check/magma/smtp/smtp_check.c
 *
 * @brief SMTP interface test functions.
 */

#include "magma_check.h"

START_TEST (check_smtp_network_basic_s) {

	log_disable();
	bool_t outcome = true;
	server_t *server = NULL;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (!(server = servers_get_by_protocol(SMTP, false))) {
		st_sprint(errmsg, "No SMTP servers were configured and available for testing.");
		outcome = false;
	}
	else if (status()) {
		outcome = check_smtp_network_simple_sthread(errmsg, server->network.port);
	}

	log_test("SMTP / NETWORK / BASIC / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_smtp_accept_store_message_s) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = MANAGEDBUF(2048);

	outcome = check_smtp_accept_message_sthread(errmsg);

	log_test("SMTP / ACCEPT / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_smtp_checkers_greylist_s) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	outcome = check_smtp_checkers_greylist_sthread(errmsg);

	log_test("SMTP / CHECKERS / GREYLIST / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_smtp_checkers_filters_s) {

//	log_disable();
	log_enable();
	bool_t outcome = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (status()) 				outcome = check_smtp_checkers_filters_sthread(errmsg, SMTP_FILTER_ACTION_DELETE, -2);
	if (status() && outcome) 	outcome = check_smtp_checkers_filters_sthread(errmsg, SMTP_FILTER_ACTION_MOVE, 2);
	if (status() && outcome) 	outcome = check_smtp_checkers_filters_sthread(errmsg, SMTP_FILTER_ACTION_LABEL, 3);
	if (status() && outcome) 	outcome = check_smtp_checkers_filters_sthread(errmsg, SMTP_FILTER_ACTION_MARK_READ, 4);

	log_test("SMTP / CHECKERS / FILTERS / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

Suite * suite_check_smtp(void) {

	Suite *s = suite_create("\tSMTP");

	suite_check_testcase(s, "SMTP", "SMTP Accept Message/S", check_smtp_accept_store_message_s);
	suite_check_testcase(s, "SMTP", "SMTP Checkers Greylist/S", check_smtp_checkers_greylist_s);
	suite_check_testcase(s, "SMTP", "SMTP Checkers Filters/S", check_smtp_checkers_filters_s);
	suite_check_testcase(s, "SMTP", "SMTP Network Basic/S", check_smtp_network_basic_s);

	return s;
}


