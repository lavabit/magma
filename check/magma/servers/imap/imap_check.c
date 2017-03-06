/*
 * @file /check/magma/pop/pop_check.c
 *
 * @brief IMAP interface test functions.
 */

#include "magma_check.h"

/**
 * Calls client_read_line on a client until the last line is found, checking for errors.
 *
 * @param client The client to read from (which should be connected to an IMAP server).
 *
 * @return Returns true if client_read_line was successful until the last line was found.
 * Otherwise returns false.
 */
bool_t imap_client_read_line_to_end(client_t *client) {
	while (client_read_line(client) > 0) {
		// Test if the last 9 characters in the line are "completed"
		if (pl_length_get(client->line) > 9 && st_cmp_cs_ends(&client->line, NULLER("completed"))) return true;
	}
	return false;
}

START_TEST (check_imap_network_simple_s) {

	log_disable();
	client_t *client = NULL;
	stringer_t *errmsg = NULL;
	// In the future we want to programatically determine this by protocol.
	const uint32_t port = 9000;

	if (status() && !(client = client_connect("localhost", port))) {
		errmsg = NULLER("Failed to establish a client connection.");
	}
	else if (client_read_line(client) <= 0 || client->status != 1) {
		errmsg = NULLER("Failed to return successful status initially.");
	}

	// Test the LOGIN command.
	else if (status() && client_write(client, PLACER("A1 LOGIN princess password\r\n", 28)) <= 0) {
		errmsg = NULLER("Failed to write the A1 LOGIN command.");
	}
	else if (!imap_client_read_line_to_end(client) || client->status != 1) {
		errmsg = NULLER("Failed to return successful status after LOGIN.");
	}

	// Test the SELECT command.
	else if (status() && client_write(client, PLACER("A2 SELECT Inbox\r\n", 28)) <= 0) {
		errmsg = NULLER("Failed to write the A2 SELECT command.");
	}
	else if (!imap_client_read_line_to_end(client) || client->status != 1) {
		errmsg = NULLER("Failed to return successful status after SELECT.");
	}

	// Test the LOGOUT command.
	else if (status() && client_write(client, PLACER("A3 LOGOUT\r\n", 28)) <= 0) {
		errmsg = NULLER("Failed to write the A3 LOGOUT command.");
	}
	else if (!imap_client_read_line_to_end(client) || client->status != 1) {
		errmsg = NULLER("Failed to return successful status after LOGOUT.");
	}

	client_close(client);

	log_test("IMAP / NETWORK / SIMPLE CHECK:", errmsg);
	ck_assert_msg(!errmsg, st_char_get(errmsg));
}
END_TEST

Suite * suite_check_imap(void) {

	Suite *s = suite_create("\tIMAP");

	suite_check_testcase(s, "IMAP", "IMAP Network Simple Check/S", check_imap_network_simple_s);

	return s;
}
