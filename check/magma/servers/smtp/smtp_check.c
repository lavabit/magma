
/**
 * @file /check/magma/smtp/smtp_check.c
 *
 * @brief SMTP interface test functions.
 */

#include "magma_check.h"

START_TEST (check_smtp_network_simple_s) {

	log_disable();
//	client_t *client = NULL;
	stringer_t *errmsg = NULL;
//	// In the future we want to programatically determine this by protocol.
//	const uint32_t port = 7000;
//
//	if (!(client = client_connect("localhost", port))) {
//		errmsg = NULLER("Failed to establish a client connection.");
//	}
//	else if (client_read_line(client) <= 0 || client->status != 1 || pl_empty(client->line) || *pl_char_get(client->line) != '2') {
//		errmsg = NULLER("Failed to return successful status initially.");
//	}
//
//	log_pedantic("Initial >>> %.*s <<<", pl_length_int(pl_trim_end(client->line)), pl_char_get(client->line));
//
//	// Test EHLO.
//	if (client_print(client, "EHLO princess\r\n") <= 0) {
//		errmsg = NULLER("Failed to write the EHLO command.");
//	}
//	else if (client_read_line(client) <= 0 || client->status != 1 || pl_empty(client->line) || *pl_char_get(client->line) != '2') {
//		errmsg = NULLER("Failed to return successful status after EHLO.");
//	}
//
//	log_pedantic("EHLO >>> %.*s <<<", pl_length_int(pl_trim_end(client->line)), pl_char_get(client->line));
//
//	// Test HELO.
//	if (client_print(client, "HELO princess\r\n") <= 0) {
//		errmsg = NULLER("Failed to write the HELO command.");
//	}
//	else if (client_read_line(client) <= 0 || client->status != 1 || pl_empty(client->line) || *pl_char_get(client->line) != '2') {
//		errmsg = NULLER("Failed to return successful status after HELO.");
//	}
//
//	log_pedantic("HELO >>> %.*s <<<", pl_length_int(pl_trim_end(client->line)), pl_char_get(client->line));
//
//	// Test MAIL.
//	if (client_print(client, "MAIL FROM: <>\r\n") <= 0) {
//		errmsg = NULLER("Failed to write the MAIL command.");
//	}
//	else if (client_read_line(client) <= 0 || client->status != 1 || pl_empty(client->line) || *pl_char_get(client->line) != '2') {
//		errmsg = NULLER("Failed to return successful status after MAIL.");
//	}
//
//	log_pedantic("MAIL FROM >>> %.*s <<<", pl_length_int(pl_trim_end(client->line)), pl_char_get(client->line));
//
//	// Test RCPT.
//	if (client_print(client, "RCPT TO: <ladar@lavabit.com>\r\n") <= 0) {
//		errmsg = NULLER("Failed to write the RCPT command.");
//	}
//	else if (client_read_line(client) <= 0 || client->status != 1 || pl_empty(client->line) || *pl_char_get(client->line) != '2') {
//		errmsg = NULLER("Failed to return successful status after RCPT.");
//	}
//
//	log_pedantic("RCPT TO >>> %.*s <<<", pl_length_int(pl_trim_end(client->line)), pl_char_get(client->line));
//
//	// Test DATA.
//	if (client_print(client, "DATA\r\n") <= 0) {
//		errmsg = NULLER("Failed to write the DATA command.");
//	}
//	else if (client_read_line(client) <= 0 || client->status != 1 || pl_empty(client->line) || *pl_char_get(client->line) != '3') {
//		errmsg = NULLER("Failed to return successful status after DATA.");
//	}
//
//	log_pedantic("DATA >>> %.*s <<<", pl_length_int(pl_trim_end(client->line)), pl_char_get(client->line));
//
//	// Test sending the contents of an email.
//	if (client_print(client, "FROM: Magma\nSUBJECT: Unit Tests\nAren't unit tests great?\n.\r\n") <= 0) {
//		errmsg = NULLER("Failed to write email contents.");
//	}
//	else if (client_read_line(client) <= 0 || client->status != 1 || pl_empty(client->line) || *pl_char_get(client->line) != '2') {
//		errmsg = NULLER("Failed to return successful status after sending email contents.");
//	}
//
//	log_pedantic("contents of email >>> %.*s <<<", pl_length_int(pl_trim_end(client->line)), pl_char_get(client->line));
//
//	if (client) {
//
//		// Test QUIT
//		if (client_print(client, "QUIT\r\n") <= 0) {
//			errmsg = NULLER("Failed to write email contents.");
//		}
//		else if (client_read_line(client) <= 0 || client->status != 1 || pl_empty(client->line) || *pl_char_get(client->line) != '2') {
//			errmsg = NULLER("Failed to return successful status after sending email contents.");
//		}
//
//		log_pedantic("QUIT >>> %.*s <<<", pl_length_int(pl_trim_end(client->line)), pl_char_get(client->line));
//
//		client_close(client);
//	}

	log_test("SMTP / NETWORK / SIMPLE CHECK:", NULLER("SKIPPED"));
	ck_assert_msg(!errmsg, st_char_get(errmsg));
}
END_TEST

START_TEST (check_smtp_accept_store_message_s) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = MANAGEDBUF(2048);

	if (status()) outcome = check_smtp_accept_store_message_sthread(errmsg);

	log_test("SMTP / ACCEPT / STORAGE / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_smtp_accept_rollout_s) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (status()) outcome = check_smtp_accept_rollout_sthread(errmsg);

	log_test("SMTP / ACCEPT / ROLLOUT / SINGLE THREADED:", NULLER("SKIPPED"));
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_smtp_accept_store_spamsig_s) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (status()) outcome = check_smtp_accept_store_spamsig_sthread(errmsg);

	log_test("SMTP / ACCEPT / SIGNATURES / SINGLE THREADED:", NULLER("SKIPPED"));
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_smtp_accept_greylist_s) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (status()) outcome = check_smtp_checkers_greylist_sthread(errmsg);

	log_test("SMTP / ACCEPT / GREYLIST / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

Suite * suite_check_smtp(void) {

	Suite *s = suite_create("\tSMTP");

	suite_check_testcase(s, "SMTP", "SMTP Storage/S", check_smtp_accept_store_message_s);
	suite_check_testcase(s, "SMTP", "SMTP Rollout/S", check_smtp_accept_rollout_s);
	suite_check_testcase(s, "SMTP", "SMTP Signatres/S", check_smtp_accept_store_spamsig_s);
	suite_check_testcase(s, "SMTP", "SMTP Greylist/S", check_smtp_accept_greylist_s);
	suite_check_testcase(s, "SMTP", "SMTP Network Simple Check/S", check_smtp_network_simple_s);

	return s;
}


