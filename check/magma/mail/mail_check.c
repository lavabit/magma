
/**
 * @file /check/magma/mail/mail_check.c
 */

#include "magma_check.h"

START_TEST (check_mail_load_s) {

	log_disable();
	bool_t result = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (status()) result = check_mail_load_sthread(errmsg);

	// Because libcheck will sometimes fork the process to protect against segmentation faults
	// the normal thread cleanup code won't be called, so we do it explicitly here to avoid valgrind
	// complaints about a memory leak.
	mail_cache_reset();

	log_test("MAIL / LOAD / SINGLE THREADED:", errmsg);
	ck_assert_msg(result, st_char_get(errmsg));
}
END_TEST

START_TEST (check_mail_store_s) {

	log_disable();
	bool_t result = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (status()) result = check_mail_store_plaintext_sthread(errmsg);
	if (status() && result) result = check_mail_store_encrypted_sthread(errmsg);

	log_test("MAIL / STORE / SINGLE THREADED:", errmsg);
	ck_assert_msg(result, st_char_get(errmsg));
}
END_TEST

START_TEST (check_mail_headers_s) {

	log_disable();
	bool_t result = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (status()) result = check_mail_headers_sthread(errmsg);

	log_test("MAIL / HEADERS / SINGLE THREADED:", errmsg);
	ck_assert_msg(result, st_char_get(errmsg));
}
END_TEST

Suite * suite_check_mail(void) {

	Suite *s = suite_create("\tMail");

	suite_check_testcase(s, "MAIL", "Mail Store/S", check_mail_store_s);
	suite_check_testcase(s, "MAIL", "Mail Load/S", check_mail_load_s);
	suite_check_testcase(s, "MAIL", "Mail Headers/S", check_mail_headers_s);

	return s;
}
