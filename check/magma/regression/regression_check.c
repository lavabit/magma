/**
 * @file /check/magma/regression/regression_check.c
 *
 * @brief The heart of the regression test suite
 */

#include "magma_check.h"

START_TEST (check_regression_file_descriptors_leak_m) {

	log_disable();
	void *result = NULL;
	bool_t outcome = true;
	pthread_t *threads = NULL;
	stringer_t *errmsg = NULL, *path = NULL;
	int_t folders_before, folder_difference = 0;

	if (status() && !(threads = mm_alloc(sizeof(pthread_t) * REGRESSION_CHECK_FILE_DESCRIPTORS_LEAK_MTHREADS))) {
		errmsg = NULLER("Thread allocation failed.");
		outcome = false;
	}
	else if (status()) {

		path = st_quick(MANAGEDBUF(255), "/proc/%i/fd/", process_my_pid());
		folders_before = folder_count(path, false, false);

		// Fork a bunch of processes that open file descriptors.
		for (uint64_t counter = 0; counter < REGRESSION_CHECK_FILE_DESCRIPTORS_LEAK_MTHREADS; counter++) {
			if (thread_launch(threads + counter, &check_regression_file_descriptors_leak_test, NULL)) {
				errmsg = NULLER("Thread launch failed.");
				outcome = false;
			}
		}

		// Join them, each process should handle its own cleanup.
		for (uint64_t counter = 0; counter < REGRESSION_CHECK_FILE_DESCRIPTORS_LEAK_MTHREADS; counter++) {
			if (thread_result(*(threads + counter), &result)) {
				if (!errmsg) errmsg = NULLER("Thread join error.");
				outcome = false;
			}
			else if (!result) {
				if (!errmsg) errmsg = NULLER("Thread test failed.");
				outcome = false;
			}
			else {
				mm_free(result);
			}
		}

		folder_difference = folder_count(path, false, false) - folders_before;

		if (folder_difference) {
			outcome = false;
			if (!errmsg) errmsg = NULLER("Failed to properly clean file handles.");
		}
	}

	mm_free(threads);

	log_test("REGRESSION / MAPPED / FILE DESCRIPTOR LEAK / MULTI THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_regression_smtp_dot_stuffing_s) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (status()) outcome = check_regression_smtp_dot_stuffing_sthread(errmsg);

	log_test("REGRESSION / SMTP / DOT STUFFING / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_regression_imap_search_range_parsing_s) {

	log_disable();
	bool_t outcome = true;
	server_t *server = NULL;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (!(server = servers_get_by_protocol(IMAP, false))) {
		st_sprint(errmsg, "No IMAP servers were configured and available for testing.");
		outcome = false;
	}
	else if (status()) {
		outcome = check_regression_imap_search_range_parsing_sthread(errmsg, server->network.port);
	}

	log_test("REGRESSION / IMAP / SEARCH RANGE PARSING / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

Suite * suite_check_regression(void) {

	Suite *s = suite_create("\tRegression");

	suite_check_testcase(s, "REGRESSION", "Regression File Descriptors Leak/M", check_regression_file_descriptors_leak_m);
	suite_check_testcase(s, "REGRESSION", "Regression SMTP Dot Stuffing/S", check_regression_smtp_dot_stuffing_s);
	suite_check_testcase(s, "REGRESSION", "Regression IMAP Search Range Parsing/S", check_regression_imap_search_range_parsing_s);

	return s;
}
