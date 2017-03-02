
/**
 * @file /check/magma/smtp/smtp_check.c
 *
 * @brief SMTP interface test functions.
 */

#include "magma_check.h"

void check_smtp_network_simple_read(client_t *client) {
	if (thread_start()) {
		client_read_line(client);
		thread_stop();
	}
	pthread_exit(NULL);
	return;
}

START_TEST (check_smtp_network_simple_s) {

	//log_disable();
	log_enable();
	client_t *client = NULL;
	stringer_t *errmsg = NULL;
	// In the future we want to programatically determine this by protocol.
	const uint32_t port = 7000;

	if (!(client = client_connect("localhost", port))) {
		errmsg = NULLER("Failed to establish a client connection.");
	}

	if (client_read_line(client) <= 0 || client->status != 1 || pl_char_get(client->line)[0] != '2') {
		errmsg = NULLER("Failed to return successful status initially.");
	}

	log_pedantic("%s", pl_char_get(client->line));

	// Test EHLO.
	if (client_write(client, PLACER("EHLO magma\r\n", 12)) <= 0) {
		errmsg = NULLER("Failed to write the EHLO command.");
	}

	if (client_read_line(client) <= 0 || client->status != 1 || pl_char_get(client->line)[0] != '2') {
		errmsg = NULLER("Failed to return successful status after EHLO.");
	}

	log_pedantic("%s", pl_char_get(client->line));

	// Test HELO.
	if (client_write(client, PLACER("HELO magma\r\n", 12)) <= 0) {
		errmsg = NULLER("Failed to write the HELO command.");
	}

	if (client_read_line(client) <= 0 || client->status != 1 || pl_char_get(client->line)[0] != '2') {
		errmsg = NULLER("Failed to return successful status after HELO.");
	}

	log_pedantic("%s", pl_char_get(client->line));

	// Test MAIL.
	if (client_write(client, PLACER("MAIL FROM: <>\r\n", 15)) <= 0) {
		errmsg = NULLER("Failed to write the MAIL command.");
	}

	if (client_read_line(client) <= 0 || client->status != 1 || pl_char_get(client->line)[0] != '2') {
		errmsg = NULLER("Failed to return successful status after MAIL.");
	}

	log_pedantic("%s", pl_char_get(client->line));

	// Test RCPT.
	if (client_write(client, PLACER("RCPT TO: <ladar@lavabit.com>\r\n", 13)) <= 0) {
		errmsg = NULLER("Failed to write the RCPT command.");
	}

	if (client_read_line(client) <= 0 || client->status != 1 || pl_char_get(client->line)[0] != '2') {
		errmsg = NULLER("Failed to return successful status after RCPT.");
	}

	log_pedantic("%s", pl_char_get(client->line));

	// Test DATA.
	if (client_write(client, PLACER("DATA\r\n", 6)) <= 0) {
		errmsg = NULLER("Failed to write the DATA command.");
	}

	if (client_read_line(client) <= 0 || client->status != 1 || pl_char_get(client->line)[0] != '2') {
		errmsg = NULLER("Failed to return successful status after DATA.");
	}

	log_pedantic("%s", pl_char_get(client->line));

	// Test sending the contents of an email.
	if (client_write(client, PLACER("FROM: Magma\nSUBJECT: Unit Tests\nAren't unit tests great?\n.\r\n", 60)) <= 0) {
		errmsg = NULLER("Failed to write email contents.");
	}

	if (client_read_line(client) <= 0 || client->status != 1 || pl_char_get(client->line)[0] != '2') {
		errmsg = NULLER("Failed to return successful status after sending email contents.");
	}

	log_pedantic("%s", pl_char_get(client->line));

	if (client) client_close(client);

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

	TCase *tc;
	Suite *s = suite_create("\tSMTP");

	testcase(s, tc, "SMTP Network Simple Check/S", check_smtp_network_simple_s);
	testcase(s, tc, "SMTP Accept/S", check_smtp_accept_store_message_s);
	testcase(s, tc, "SMTP Rollout/S", check_smtp_accept_rollout_s);
	testcase(s, tc, "SMTP Signatres/S", check_smtp_accept_store_spamsig_s);
	testcase(s, tc, "SMTP Greylist/S", check_smtp_checkers_greylist_s);
	return s;
}


