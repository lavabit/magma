
/**
 * @file /check/magma/smtp/smtp_check.c
 *
 * @brief SMTP interface test functions.
 */

#include "magma_check.h"

/// smtp/accept.c

START_TEST (check_smtp_accept_store_message_s) {

	//log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	outcome = check_smtp_accept_store_message_sthread(errmsg);

	log_test("SMTP / ACCEPT / STORE MESSAGE / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_smtp_accept_rollout_s) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	outcome = check_smtp_accept_rollout_sthread(errmsg);

	log_test("SMTP / ACCEPT / ROLLOUT / SINGLE THREADED:", NULLER("SKIPPED"));
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_smtp_accept_store_spamsig_s) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	outcome = check_smtp_accept_store_spamsig_sthread(errmsg);

	log_test("SMTP / ACCEPT / STORE SPAMSIG / SINGLE THREADED:", NULLER("SKIPPED"));
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_smtp_accept_accept_message_s) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	outcome = check_smtp_accept_accept_message_sthread(errmsg);

	log_test("SMTP / ACCEPT / ACCEPT MESSAGE / SINGLE THREADED:", NULLER("SKIPPED"));
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST


/// smtp/checkers.c

START_TEST (check_smtp_checkers_greylist_s) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	outcome = check_smtp_checkers_greylist_sthread(errmsg);

	log_test("SMTP / CHECKERS / GREYLIST / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

Suite * suite_check_smtp(void) {

	TCase *tc;
	Suite *s = suite_create("\tSMTP");

	testcase(s, tc, "SMTP Accept Store Message/S", check_smtp_accept_store_message_s);
	testcase(s, tc, "SMTP Accept Store Message/S", check_smtp_accept_rollout_s);
	testcase(s, tc, "SMTP Accept Store Message/S", check_smtp_accept_store_spamsig_s);
	testcase(s, tc, "SMTP Accept Store Message/S", check_smtp_accept_accept_message_s);
	testcase(s, tc, "SMTP Checkers Greylist/S", check_smtp_checkers_greylist_s);

	return s;
}


