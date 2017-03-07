
/**
 * @file /check/magma/smtp/smtp_check.c
 *
 * @brief SMTP interface test functions.
 */

#include "magma_check.h"\

/**
 * Calls client_read_line on a client until the last line is found, checking for errors.
 *
 * @param client The client to read from (which should be connected to an SMTP server).
 *
 * @return Returns true if client_read_line was successful until the last line was found.
 * Otherwise returns false.
 */
bool_t check_smtp_client_read_line_to_end(client_t *client) {
	while (client_read_line(client) > 0) {
		if (pl_char_get(client->line)[3] == ' ') return true;
	}
	return false;
}

bool_t check_smtp_network_simple_sthread(stringer_t *errmsg) {

	bool_t outcome = true;
	client_t *client = NULL;

	// Test the initial response.
	if (status() && !(client = client_connect("localhost", 7000))) {
		st_sprint(errmsg, "Failed to establish a client connection.");
		outcome = false;
	}
	if (status() && outcome && (!check_smtp_client_read_line_to_end(client) || (client->status != 1) || (*pl_char_get(client->line) != '2'))) {
		st_sprint(errmsg, "Failed to return successful status initially.");
		outcome = false;
	}

	// Test the EHLO command.
	if (status() && outcome && client_print(client, "EHLO princess\r\n") <= 0) {
		st_sprint(errmsg, "Failed to write the EHLO command.");
		outcome = false;
	}
	if (status() && outcome && (!check_smtp_client_read_line_to_end(client) || (client->status != 1) || (*pl_char_get(client->line) != '2'))) {
		st_sprint(errmsg, "Failed to return successful status after EHLO.");
		outcome = false;
	}

	// Test the HELO command.
	if (status() && outcome && client_print(client, "HELO princess\r\n") <= 0) {
		st_sprint(errmsg, "Failed to write the HELO command.");
		outcome = false;
	}
	if (status() && outcome && (!check_smtp_client_read_line_to_end(client) || (client->status != 1) || (*pl_char_get(client->line) != '2'))) {
		st_sprint(errmsg, "Failed to return successful status after HELO.");
		outcome = false;
	}

	// Test the MAIL command.
	if (status() && outcome && client_print(client, "MAIL FROM: <>\r\n") <= 0) {
		st_sprint(errmsg, "Failed to write the MAIL command.");
		outcome = false;
	}
	if (status() && outcome && (!check_smtp_client_read_line_to_end(client) || (client->status != 1) || (*pl_char_get(client->line) != '2'))) {
		st_sprint(errmsg, "Failed to return successful status after MAIL.");
		outcome = false;
	}

	// Test the RCPT command.
	if (status() && outcome && client_print(client, "RCPT TO: <ladar@lavabit.com>\r\n") <= 0) {
		st_sprint(errmsg, "Failed to write the RCPT command.");
		outcome = false;
	}
	if (status() && outcome && (!check_smtp_client_read_line_to_end(client) || (client->status != 1) || (*pl_char_get(client->line) != '2'))) {
		st_sprint(errmsg, "Failed to return successful status after RCPT.");
		outcome = false;
	}

	// Test the DATA command.
	if (status() && outcome && client_print(client, "DATA\r\n") <= 0) {
		st_sprint(errmsg, "Failed to write the DATA command.");
		outcome = false;
	}
	if (status() && outcome && (!check_smtp_client_read_line_to_end(client) || (client->status != 1) || (*pl_char_get(client->line) != '3'))) {
		st_sprint(errmsg, "Failed to return successful status after DATA.");
		outcome = false;
	}

	// Test sending the contents of an email.
	if (status() && outcome && client_print(client, "FROM: Princess\nSUBJECT: Unit Tests\nAren't unit tests great?\n.\r\n") <= 0) {
		st_sprint(errmsg, "Failed to write email contents.");
		outcome = false;
	}
	if (status() && outcome && (!check_smtp_client_read_line_to_end(client) || (client->status != 1) || (*pl_char_get(client->line) != '2'))) {
		st_sprint(errmsg, "Failed to return successful status after sending email contents.");
		outcome = false;
	}

	// Test the QUIT command.
	if (status() && outcome && client_print(client, "QUIT\r\n") <= 0) {
		st_sprint(errmsg, "Failed to write email contents.");
		outcome = false;
	}
	if (status() && outcome && (!check_smtp_client_read_line_to_end(client) || (client->status != 1) || (*pl_char_get(client->line) != '2'))) {
		st_sprint(errmsg, "Failed to return successful status after sending email contents.");
		outcome = false;
	}

	client_close(client);
	return outcome;
}

START_TEST (check_smtp_network_simple_s) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (status()) outcome = check_smtp_network_simple_sthread(errmsg);

	log_test("SMTP / NETWORK / SIMPLE CHECK:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_smtp_accept_store_message_s) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = MANAGEDBUF(2048);

	outcome = check_smtp_accept_store_message_sthread(errmsg);

	log_test("SMTP / ACCEPT / STORAGE / SINGLE THREADED:", errmsg);
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

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	outcome = check_smtp_checkers_filters_sthread(errmsg);

	log_test("SMTP / CHECKERS / FILTERS / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

Suite * suite_check_smtp(void) {

	Suite *s = suite_create("\tSMTP");

	suite_check_testcase(s, "SMTP", "SMTP Network Simple Check/S", check_smtp_network_simple_s);
	suite_check_testcase(s, "SMTP", "SMTP Accept Store Message/S", check_smtp_accept_store_message_s);
	suite_check_testcase(s, "SMTP", "SMTP Checkers Greylist/S", check_smtp_checkers_greylist_s);
	suite_check_testcase(s, "SMTP", "SMTP Checkers Filters/S", check_smtp_checkers_filters_s);
	return s;
}


