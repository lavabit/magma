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

START_TEST (check_regression_imap_search_range_s) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	/// MEDIUM: Write a unit test to prevent the regression of the IMAP search range parsing logic.
	///
	/// A01 LOGIN magma password
	/// A02 SELECT Inbox
	///
	/// -- This search command would have triggered the parser issue in the past because the start sequence is
	/// 	higher than the first available message. Note the start token could have been a UID or sequence and triggered
	/// 	the improper token handling. It just needed to be higher than the first legit message. Now this should return
	/// 	an empty set, assuming the start range is higher then we'll ever see during a unit test. (Make it higher?)
	/// A03 SEARCH UID 531870239:532870239 NOT DELETE
	///
	/// -- Ensure that even if the first message set doesn't match, because it starts beyond the valid range, a second
	/// 	valid sequence will still be considered, and thus return messages.
	/// A04 SEARCH UID 531870239:532870239,1:10000 NOT DELETE
	///
	/// A05 LOGOUT
	///

	st_sprint(errmsg, "This check needs love. Touch me tender, and finish me off.");
	outcome = false;

	//if (status()) outcome = check_regression_imap_search_range_sthread(errmsg);

	log_test("REGRESSION / IMAP / SEARCH RANGE PARSING / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

Suite * suite_check_regression(void) {

	Suite *s = suite_create("\tRegression");

	suite_check_testcase(s, "REGRESSION", "Regression File Descriptors Leak/M", check_regression_file_descriptors_leak_m);
	suite_check_testcase(s, "REGRESSION", "Regression SMTP Dot Stuffing/S", check_regression_smtp_dot_stuffing_s);
	suite_check_testcase(s, "REGRESSION", "Regression IMAP Search Range Ranges/S", check_regression_imap_search_range_s);

	return s;
}
