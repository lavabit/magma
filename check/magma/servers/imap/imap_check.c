/*
 * @file /check/magma/pop/pop_check.c
 *
 * @brief IMAP interface test functions.
 */

#include "magma_check.h"

/**
 * Calls client_read_line on a client until it finds a line matching "<token> OK"
 *
 * @param client The client to read from (which should be connected to an IMAP server).
 * @param token The unique token that identifies the current imap command dialogue.
 *
 * @return Returns true if client_read_line was successful until the last line was found.
 * specified in num and there was no error. Otherwise returns false.
 */
bool_t check_imap_client_read_lines_to_end(client_t *client, chr_t *token) {

	bool_t outcome = false;
	stringer_t *last_line = st_merge("ss", NULLER(token), NULLER(" OK"));

	while (!outcome && client_read_line(client) > 0) {
		if (st_cmp_cs_starts(&client->line, last_line) == 0) outcome = true;
	}

	st_cleanup(last_line);
	return outcome;
}

bool_t check_imap_network_simple_sthread(stringer_t *errmsg) {

	client_t *client = NULL;

	// Check the initial response.
	if (!(client = client_connect("localhost", 9000))) {
		st_sprint(errmsg, "Failed to establish a client connection.");
		return false;
	}
	else if ((!check_imap_client_read_lines_to_end(client, "*") || (client->status != 1))) {
		st_sprint(errmsg, "Failed to return successful status initially.");
		return false;
	}

	// Test the LOGIN command.
	if (client_print(client, "A1 LOGIN princess password\r\n") <= 0) {
		st_sprint(errmsg, "Failed to write the A1 LOGIN command.");
		return false;
	}
	else if (!check_imap_client_read_lines_to_end(client, "A1") || (client->status != 1)) {
		st_sprint(errmsg, "Failed to return successful status after LOGIN.");
		return false;
	}

	// Test the SELECT command.
	if (client_print(client, "A2 SELECT Inbox\r\n") <= 0) {
		st_sprint(errmsg, "Failed to write the A2 SELECT command.");
		return false;
	}
	else if ((!check_imap_client_read_lines_to_end(client, "A2") || (client->status != 1))) {
		st_sprint(errmsg, "Failed to return successful status after SELECT.");
		return false;
	}

	// Test the FETCH command.
	if (client_print(client, "A3 FETCH 1 RFC822\r\n") <= 0) {
		st_sprint(errmsg, "Failed to write the A3 FETCH command.");
		return false;
	}
	else if ((!check_imap_client_read_lines_to_end(client, "A3") || (client->status != 1))) {
		st_sprint(errmsg, "Failed to return successful status after SELECT.");
		return false;
	}

	// Test the LOGOUT command.
	if (client_print(client, "A4 LOGOUT\r\n") <= 0) {
		st_sprint(errmsg, "Failed to write the A4 LOGOUT command.");
		return false;
	}
	else if ((!check_imap_client_read_lines_to_end(client, "A4") || (client->status != 1))) {
		st_sprint(errmsg, "Failed to return successful status after LOGOUT.");
		return false;
	}

	client_close(client);

	return true;
}

START_TEST (check_imap_network_simple_s) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (status()) outcome = check_imap_network_simple_sthread(errmsg);

	log_test("IMAP / NETWORK / SIMPLE CHECK:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

Suite * suite_check_imap(void) {

	Suite *s = suite_create("\tIMAP");

	suite_check_testcase(s, "IMAP", "IMAP Network Simple Check/S", check_imap_network_simple_s);

	return s;
}
