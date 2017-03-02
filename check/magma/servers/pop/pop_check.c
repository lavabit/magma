/**
 * @file /check/magma/pop/pop_check.c
 *
 * @brief POP interface test functions.
 */

START_TEST (check_pop_network_simple_s) {
	log_disable();
	stringer_t errmsg = NULL;

	log_test("IMAP / NETWORK / SIMPLE CHECK:", NULLER("SKIPPED"));
	ck_check_msg(!errmsg, errmsg);
}

Suite * suite_check_pop(void) {

	TCase *tc;
	Suite *s = suite_create("\tPOP");

	testcase(s, tc, "POP Network Simple Check/S", check_pop_network_simple_s);
	return s;
}
