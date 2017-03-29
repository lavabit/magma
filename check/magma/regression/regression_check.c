/**
 * @file /check/magma/regression/regression_check.c
 *
 * @brief The heart of the regression test suite
 */

#include "magma_check.h"

void check_regression_file_descriptors_leak_test(void) {

	stringer_t *m;
	bool_t *outcome;

	if (!thread_start() || !(outcome = mm_alloc(sizeof(bool_t)))) {
		log_error("Unable to setup the thread context.");
		pthread_exit(NULL);
		return;
	}

	if (!(m = st_alloc_opts(MAPPED_T | JOINTED | HEAP, 1024))) {
		*outcome = false;
	}
	else {
		st_free(m);
		*outcome = true;
	}

	thread_stop();
	pthread_exit(outcome);
	return;

}

/**
 * @brief	Reads lines from a client_t* until a line is reached containing token, and if a line is reached
 * 			that starts with a period, checks if the line starts with two periods.
 *
 * @param	client	The client_t* to read lines from.
 * @param	token	A chr_t*. When a line is reached that contains with this token, the function returns.
 * @return	True if all lines read until token that start with '.' start with '..', false otherwise.
 */
bool_t check_client_dot_stuff(client_t *client, chr_t *token) {

	while (client_read_line(client) > 0 && st_search_cs(&(client->line), NULLER(token), NULL)) {

		if (st_cmp_cs_starts(&(client->line), NULLER(".")) == 0 &&
			st_cmp_cs_starts(&(client->line), NULLER("..")) != 0) {

			return false;
		}
	}
	return true;
}

bool_t check_regression_smtp_dot_stuffing_sthread(stringer_t *errmsg) {

	client_t *client = NULL;
	server_t *server = NULL;
	uint64_t message_num = 0;
	stringer_t *top_command = NULL, *mailfrom = "magma@lavabit.com", *rcptto = NULLER("princess@example.com"),
		*message = NULLER(
		"To: \"Magma\" <magma@lavabit.com>\r\n"\
		"From: \"Princess\" <princess@example.com\r\n"\
		"Subject: Dot Stuffing Regression Test\r\n"\
		"This is an SMTP message whose body has a period at the start of a line\r\n"\
		". In fact, there are two instances of this in the body of this message\r\n"\
		". The SMTP client code should stuff an extra period after each of them.\r\n"\
		".\r\n");

	// First, send the message with periods at the beginning of lines in the body.
	if (!(client = smtp_client_connect(0))) {
		st_sprint(errmsg, "Failed to connect to an SMTP server.");
		return false;
	}
	else if (smtp_client_send_helo(client) != 1) {
		st_sprint(errmsg, "Failed to return successful state after HELO.");
		smtp_client_close(client);
		return false;
	}
	else if (smtp_client_send_mailfrom(client, mailfrom, 0) != 1) {
		st_sprint(errmsg, "Failed to return successful state after MAIL FROM.");
		smtp_client_close(client);
		return false;
	}
	else if (smtp_client_send_rcptto(client, rcptto) != 1) {
		st_sprint(errmsg, "Failed to return successful state after RCPT TO.");
		smtp_client_close(client);
		return false;
	}
	else if (smtp_client_send_data(client, message, false) != 1) {
		st_sprint(errmsg, "Failed to return successful state after DATA.");
		smtp_client_close(client);
		return false;
	}

	smtp_client_close(client);

	// Next, check if the entire message was sent to the recipient.
	if (!(server = servers_get_by_protocol(POP, false)) || !(client = client_connect("localhost", server->network.port)) ||
		!net_set_timeout(client->sockd, 20, 20) || client_read_line(client) <= 0 || client_status(client) != 1 ||
		st_cmp_cs_starts(&(client->line), NULLER("+OK"))) {

		st_sprint(errmsg, "Failed to connect to POP server.");
		return false;
	}
	else if (!check_pop_client_auth(client, "princess", "password", errmsg)) {

		client_close(client);
		return false;
	}
	else if (client_print(client, "LIST\r\n") != 6 || (message_num = check_pop_client_read_list(client, errmsg)) == 0 ||
		client_status(client) != 1) {

		if (st_empty(errmsg)) st_sprint(errmsg, "Failed to return successful state after LIST.");
		client_close(client);
		return false;
	}
	else if (!(top_command = st_aprint_opts(MANAGED_T | CONTIGUOUS | STACK, "TOP %lu 0\r\n", message_num)) ||
		client_print(client, st_char_get(top_command)) != st_length_get(top_command) || client_status(client) != 1) {

		st_sprint(errmsg, "Failed to return successful status after TOP.");
		client_close(client);
		return false;
	}
	else if (!check_client_dot_stuff(client, "Date:")) {

		st_sprint(errmsg, "The received message failed to be properly dot stuffed.");
		client_close(client);
		return false;
	}

	return true;
}

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
			errmsg = NULLER("Failed to properly clean file handles.");
		}
	}

	mm_free(threads);

	log_test("REGRESSION / FILE DESCRIPTORS LEAK / MULTI THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_regression_smtp_dot_stuffing_s) {

	bool_t outcome = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (status()) outcome = check_regression_smtp_dot_stuffing_sthread(errmsg);

	log_test("REGRESSION / SMTP DOT STUFFING / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

Suite * suite_check_regression(void) {

	Suite *s = suite_create("\tRegression");

	suite_check_testcase(s, "REGRESSION", "Regression File Descriptors Leak/M", check_regression_file_descriptors_leak_m);
	suite_check_testcase(s, "REGRESSION", "Regression SMTP Dot Stuffing/S", check_regression_smtp_dot_stuffing_s);

	return s;
}
