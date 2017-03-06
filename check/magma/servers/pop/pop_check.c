/**
 * @file /check/magma/pop/pop_check.c
 *
 * @brief POP interface test functions.
 */

#include "magma_check.h"

/**
 * Calls client_read_line on a client a specified number of times, checking for errors.
 *
 * @param client The client to read from (which should be connected to a POP server).
 * @param num The number of times client_read_line should be called. This depends on the
 * last command sent to the client.
 *
 * @return Returns true if client_read_line was successful for the number of lines
 * specified in num and there was no -ERR. Otherwise returns false.
 */
bool_t pop_client_read_lines(client_t *client, uint32_t num) {
	for (uint32_t i = 0; i < num; i++) {
		if (client_read_line(client) <= 0 || *pl_char_get(client->line) == '-') return false;
	}
	return true;
}

START_TEST (check_pop_network_simple_s) {

	log_disable();
	client_t *client = NULL;
	stringer_t *errmsg = NULL;
	// In the future we want to programatically determine this by protocol.
	const uint32_t port = 8000;

	if (status() && (!(client = client_connect("localhost", port)))) {
		errmsg = NULLER("Failed to establish a client connection.");
	}
	else if (!pop_client_read_lines(client, 1) || client->status != 1) {
		errmsg = NULLER("Failed to return successful status initially.");
	}

	// Test the USER command.
	else if (status() && client_write(client, PLACER("USER princess\r\n", 15)) <= 0) {
		errmsg = NULLER("Failed to write the USER command.");
	}
	else if (!pop_client_read_lines(client, 1) || client->status != 1) {
		errmsg = NULLER("Failed to return successful status after USER.");
	}

	// Test the PASS command.
	else if (status() && client_write(client, PLACER("PASS password\r\n", 15)) <= 0) {
		errmsg = NULLER("Failed to write the PASS command.");
	}
	else if (!pop_client_read_lines(client, 1) || client->status != 1) {
		errmsg = NULLER("Failed to return successful status after PASS.");
	}

	// TODO: Make this a fixture test that populates the princess account with a set amount of mail beforehand.
	/*
	// Test the LIST command.
	else if (status() && client_write(client, PLACER("LIST\r\n", 6)) <= 0) {
		errmsg = NULLER("Failed to write the LIST command.");
	}
	else if (client_read_line(client) <= 0 || client->status != 1 || pl_char_get(client->line)[0] != '1') {
		errmsg = NULLER("Failed to return successful status after LIST.");
	}

	// Test the RETR command.
	else if (status() && client_write(client, PLACER("RETR 1\r\n", 8)) <= 0) {
		errmsg = NULLER("Failed to write the RETR command.");
	}
	else if (client_read_line(client) <= 0 || client->status != 1 || pl_char_get(client->line)[1] != 'O' ||
			pl_char_get(client->line)[2] != 'K') {
		errmsg = NULLER("Failed to return successful status after RETR.");
	}

	// Test the DELE command.
	else if (status() && client_write(client, PLACER("DELE 1\r\n", 8)) <= 0) {
		errmsg = NULLER("Failed to write the DELE command.");
	}
	else if (client_read_line(client) <= 0 || client->status != 1 || pl_char_get(client->line)[1] != 'O' ||
			pl_char_get(client->line)[2] != 'K') {
		errmsg = NULLER("Failed to return successful status after DELE.");
	}
	*/

	// Test the QUIT command.
	else if (status() && client_write(client, PLACER("QUIT\r\n", 6)) <= 0) {
		errmsg = NULLER("Failed to write the QUIT command.");
	}
	else if (!pop_client_read_lines(client, 1) || client->status != 1) {
		errmsg = NULLER("Failed to return successful status after QUIT.");
	}

	if (client) client_close(client);

	log_test("POP / NETWORK / SIMPLE CHECK:", errmsg);
	ck_assert_msg(!errmsg, st_char_get(errmsg));
}
END_TEST

Suite * suite_check_pop(void) {

	Suite *s = suite_create("\tPOP");

	suite_check_testcase(s, "POP", "POP Network Simple Check/S", check_pop_network_simple_s);
	return s;
}
