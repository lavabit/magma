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

START_TEST (check_regression_http_append_string_s) {

	log_disable();
	bool_t outcome = true;
	server_t *server = NULL;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (!(server = servers_get_by_protocol(HTTP, false))) {
		st_sprint(errmsg, "No HTTP servers were configured and available for testing.");
		outcome = false;
	}
	else if (status()) {
		outcome = check_regression_http_append_string(errmsg, server->network.port);
	}

	log_test("REGRESSION / HTTP / APPEND STRING / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_regression_st_merge_terminator_s) {

	log_disable();
	bool_t outcome = true;
	chr_t *ns = "ABCDEFGHIJ";
	stringer_t *holder = NULL, *st = PLACER("1234567890", 10), *buf = MANAGEDBUF(100), *errmsg = MANAGEDBUF(1024);

	if (status()) {

		// Null string by itself.
		if (!(holder = st_merge("n", ns)) || (st_length_get(holder) != 10 && st_avail_get(holder) >= 11)) {
				st_sprint(errmsg, "The string merge function failed to produce a properly terminated buffer. " \
				"{ length = %zu / avail = %zu / expected = 11 }", st_length_get(holder), st_avail_get(holder));
		}
		st_cleanup(holder);

		// Managed string by itself.
		if (st_empty(errmsg) && (!(holder = st_merge("s", st)) || (st_length_get(holder) != 10 && st_avail_get(holder) >= 11))) {
				st_sprint(errmsg, "The string merge function failed to produce a properly terminated buffer. " \
				"{ length = %zu / avail = %zu / expected = 11 }", st_length_get(holder), st_avail_get(holder));
		}
		st_cleanup(holder);

		// Null terminated string followed by managed string.
		if (st_empty(errmsg) && (!(holder = st_merge("ns", ns, st)) || (st_length_get(holder) != 20 && st_avail_get(holder) >= 21))) {
			st_sprint(errmsg, "The string merge function failed to produce a properly terminated buffer. " \
				"{ length = %zu / avail = %zu / expected = 21 }", st_length_get(holder), st_avail_get(holder));
		}
		st_cleanup(holder);

		// A managed string followed by a null terminated string.
		if (st_empty(errmsg) && (!(holder = st_merge("sn", st, ns)) || (st_length_get(holder) != 20 && st_avail_get(holder) >= 21))) {
			st_sprint(errmsg, "The string merge function failed to produce a properly terminated buffer. " \
				"{ length = %zu / avail = %zu / expected = 21 }", st_length_get(holder), st_avail_get(holder));
		}
		st_cleanup(holder);

		// A complicated compbination of null and managed strings.
		if (st_empty(errmsg) && (!(holder = st_merge("snnns", st, ns, ns, ns, st)) || (st_length_get(holder) != 50 && st_avail_get(holder) >= 51))) {
			st_sprint(errmsg, "The string merge function failed to produce a properly terminated buffer. " \
				"{ length = %zu / avail = %zu / expected = 51 }", st_length_get(holder), st_avail_get(holder));
		}
		st_cleanup(holder);

		// A complicated compbination of null and managed strings, with a large binary buffer right in the middle.
		if (st_empty(errmsg) && (rand_write(MANAGEDBUF(100)) != 100 || !(holder = st_merge("snnsnns", st, ns, ns, buf, ns, ns, st)) ||
			(st_length_get(holder) != 160 && st_avail_get(holder) >= 161))) {
			st_sprint(errmsg, "The string merge function failed to produce a properly terminated buffer. " \
				"{ length = %zu / avail = %zu / expected = 161 }", st_length_get(holder), st_avail_get(holder));
		}
		st_cleanup(holder);
	}

	log_test("REGRESSION / STRINGS / MERGE TERMINATOR / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

Suite * suite_check_regression(void) {

	Suite *s = suite_create("\tRegression");

	suite_check_testcase(s, "REGRESSION", "Regression File Descriptors Leak/M", check_regression_file_descriptors_leak_m);
	suite_check_testcase(s, "REGRESSION", "Regression String Merge Terminator/S", check_regression_st_merge_terminator_s);
	suite_check_testcase(s, "REGRESSION", "Regression SMTP Dot Stuffing/S", check_regression_smtp_dot_stuffing_s);
	suite_check_testcase(s, "REGRESSION", "Regression IMAP Search Range Parsing/S", check_regression_imap_search_range_parsing_s);
	suite_check_testcase(s, "REGRESSION", "Regression HTTP Append String/S", check_regression_http_append_string_s);

	return s;
}
