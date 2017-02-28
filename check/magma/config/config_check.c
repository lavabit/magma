/**
 * @file /check/magma/config/config_check.c
 *
 * @brief config test functions.
 */

#include "magma_check.h"

START_TEST (check_config_server_from_protocol_s) {

	bool_t outcome = true;
	stringer_t *errmsg = MANAGEDBUF(1024);



	log_test("CONFIG / SERVER / FROM PROTOCOL / SINGLE THREADED", NULLER("SKIPPED"));
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

Suite * suite_check_config(void) {

	TCase *tc;
	Suite *s = suite_create("\tConfig");

	testcase(s, tc, "Config Server From Protocol/S", check_config_server_from_protocol_s);

	return s;
}
