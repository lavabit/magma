
/**
 * @file /check/magma/smtp/smtp_check.c
 *
 * @brief SMTP interface test functions.
 */

#include "magma_check.h"\

bool_t smtp_client_read_line_to_end(client_t *client) {
	while (pl_char_get(client->line)[3] != ' ') {
		if (client_read_line(client) <= 0) return false;
	}
	return true;
}

START_TEST (check_smtp_network_simple_s) {

	log_disable();
	client_t *client = NULL;
	stringer_t *errmsg = NULL;
	// In the future we want to programatically determine this by protocol.
	const uint32_t port = 7000;

	// Test the initial response.
	if (status() && !(client = client_connect("localhost", port))) {
		errmsg = NULLER("Failed to establish a client connection.");
	}
	else if (client_read_line(client) <= 0 || client->status != 1 || pl_empty(client->line) ||
			*pl_char_get(client->line) != '2' || !smtp_client_read_line_to_end(client)) {
		errmsg = NULLER("Failed to return successful status initially.");
	}

	// Test EHLO.
	else if (status() && client_print(client, "EHLO princess\r\n") <= 0) {
		errmsg = NULLER("Failed to write the EHLO command.");
	}
	else if (client_read_line(client) <= 0 || client->status != 1 || pl_empty(client->line) ||
			*pl_char_get(client->line) != '2' || !smtp_client_read_line_to_end(client)) {
		errmsg = NULLER("Failed to return successful status after EHLO.");
	}

	// Test HELO.
	else if (status() && client_print(client, "HELO princess\r\n") <= 0) {
		errmsg = NULLER("Failed to write the HELO command.");
	}
	else if (client_read_line(client) <= 0 || client->status != 1 || pl_empty(client->line) ||
			*pl_char_get(client->line) != '2' || !smtp_client_read_line_to_end(client)) {
		errmsg = NULLER("Failed to return successful status after HELO.");
	}

	// Test MAIL.
	else if (status() && client_print(client, "MAIL FROM: <>\r\n") <= 0) {
		errmsg = NULLER("Failed to write the MAIL command.");
	}
	else if (client_read_line(client) <= 0 || client->status != 1 || pl_empty(client->line) ||
			*pl_char_get(client->line) != '2' || !smtp_client_read_line_to_end(client)) {
		errmsg = NULLER("Failed to return successful status after MAIL.");
	}

	// Test RCPT.
	else if (status() && client_print(client, "RCPT TO: <ladar@lavabit.com>\r\n") <= 0) {
		errmsg = NULLER("Failed to write the RCPT command.");
	}
	else if (client_read_line(client) <= 0 || client->status != 1 || pl_empty(client->line) ||
			*pl_char_get(client->line) != '2' || !smtp_client_read_line_to_end(client)) {
		errmsg = NULLER("Failed to return successful status after RCPT.");
	}

	// Test DATA.
	else if (status() && client_print(client, "DATA\r\n") <= 0) {
		errmsg = NULLER("Failed to write the DATA command.");
	}
	else if (client_read_line(client) <= 0 || client->status != 1 || pl_empty(client->line) ||
			*pl_char_get(client->line) != '3' || !smtp_client_read_line_to_end(client)) {
		errmsg = NULLER("Failed to return successful status after DATA.");
	}

	// Test sending the contents of an email.
	else if (status() && client_print(client, "FROM: Magma\nSUBJECT: Unit Tests\nAren't unit tests great?\n.\r\n") <= 0) {
		errmsg = NULLER("Failed to write email contents.");
	}
	else if (client_read_line(client) <= 0 || client->status != 1 || pl_empty(client->line) ||
			*pl_char_get(client->line) != '2' || !smtp_client_read_line_to_end(client)) {
		errmsg = NULLER("Failed to return successful status after sending email contents.");
	}

	// Test QUIT.
	else if (status() && client_print(client, "QUIT\r\n") <= 0) {
		errmsg = NULLER("Failed to write email contents.");
	}
	else if (client_read_line(client) <= 0 || client->status != 1 || pl_empty(client->line) ||
			*pl_char_get(client->line) != '2' || !smtp_client_read_line_to_end(client)) {
		errmsg = NULLER("Failed to return successful status after sending email contents.");
	}

	client_close(client);

	log_test("SMTP / NETWORK / SIMPLE CHECK:", errmsg);
	ck_assert_msg(!errmsg, st_char_get(errmsg));
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

	log_test("SMTP / ACCEPT / SIGNATURES / SINGLE THREADED:", NULLER("SKIPPED"));
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_smtp_checkers_greylist_s) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	outcome = check_smtp_checkers_greylist_sthread(errmsg);

	log_test("SMTP / GREYLIST / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

Suite * suite_check_smtp(void) {

	Suite *s = suite_create("\tSMTP");

	suite_check_testcase(s, "SMTP", "SMTP Network Simple Check/S", check_smtp_network_simple_s);
	suite_check_testcase(s, "SMTP", "SMTP Accept/S", check_smtp_accept_store_message_s);
	suite_check_testcase(s, "SMTP", "SMTP Rollout/S", check_smtp_accept_rollout_s);
	suite_check_testcase(s, "SMTP", "SMTP Signatres/S", check_smtp_accept_store_spamsig_s);
	suite_check_testcase(s, "SMTP", "SMTP Greylist/S", check_smtp_checkers_greylist_s);
	return s;
}


