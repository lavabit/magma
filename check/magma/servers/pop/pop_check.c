/**
 * @file /check/magma/pop/pop_check.c
 *
 * @brief POP interface test functions.
 */

#include "magma_check.h"

/**
 * Calls client_read_line on a client either once or until it reaches a period only line,
 * checking for errors.
 *
 * @param client The client to read from (which should be connected to a POP server).
 * @param multiline Whether or not the response to be read is multiline.
 *
 * @return Returns true if client_read_line was successful for the number of lines
 * specified in num and there was no -ERR. Otherwise returns false.
 */
bool_t check_pop_client_read_lines_to_end(client_t *client, bool_t multiline) {
	// If the response is multiline, loop until a single period line is found.
	if (multiline) {
		while (client_read_line(client) > 0) {
			if (st_cmp_cs_eq(&client->line, NULLER(".\r\n"))) return true;
		}
		return false;
	}
	// Else, check only the next line for errors.
	else if (client_read_line(client) <= 0 || st_cmp_cs_starts(&client->line, NULLER("-ERR")) == 0) {
		return false;
	}
	else {
		return true;
	}
}

bool_t check_pop_network_simple_sthread(stringer_t *errmsg) {

	bool_t outcome = true;
	client_t *client = NULL;

	if (status() && !(client = client_connect("localhost", 8000))) {
		st_sprint(errmsg, "Failed to establish a client connection.");
		outcome = false;
	}
	if (status() && outcome && !check_pop_client_read_lines_to_end(client, false) || client->status != 1) {
		st_sprint(errmsg, "Failed to return successful status initially.");
		outcome = false;
	}

	// Test the USER command.
	if (status() && outcome && client_print(client, "USER princess\r\n") <= 0) {
		st_sprint(errmsg, "Failed to write the USER command.");
		outcome = false;
	}
	if (status() && outcome && !check_pop_client_read_lines_to_end(client, false) || client->status != 1) {
		st_sprint(errmsg, "Failed to return successful status after USER.");
		outcome = false;
	}

	// Test the PASS command.
	if (status() && outcome && client_print(client, "PASS password\r\n") <= 0) {
		st_sprint(errmsg, "Failed to write the PASS command.");
		outcome = false;
	}
	if (status() && outcome && !check_pop_client_read_lines_to_end(client, false) || client->status != 1) {
		st_sprint(errmsg, "Failed to return successful status after PASS.");
		outcome = false;
	}

	// Test the LIST command.
	if (status() && outcome && client_print(client, "LIST\r\n") <= 0) {
		st_sprint(errmsg, "Failed to write the LIST command.");
		outcome = false;
	}
	if (status() && outcome && !check_pop_client_read_lines_to_end(client, true) || client->status != 1) {
		st_sprint(errmsg, "Failed to return successful status after LIST.");
		outcome = false;
	}

	// Test the RETR command.
	if (status() && outcome && client_print(client, "RETR 1\r\n") <= 0) {
		st_sprint(errmsg, "Failed to write the RETR command.");
		outcome = false;
	}
	if (status() && outcome && !check_pop_client_read_lines_to_end(client, true) || client->status != 1) {
		st_sprint(errmsg, "Failed to return successful status after RETR.");
		outcome = false;
	}

	// Test the DELE command.
	if (status() && outcome && client_print(client, "DELE 1\r\n") <= 0) {
		st_sprint(errmsg, "Failed to write the DELE command.");
		outcome = false;
	}
	if (status() && outcome && !check_pop_client_read_lines_to_end(client, false) || client->status != 1) {
		st_sprint(errmsg, "Failed to return successful status after DELE.");
		outcome = false;
	}

	// Test the QUIT command.
	if (status() && outcome && client_print(client, "QUIT\r\n") <= 0) {
		st_sprint(errmsg, "Failed to write the QUIT command.");
		outcome = false;
	}
	if (status() && outcome && !check_pop_client_read_lines_to_end(client, false) || client->status != 1) {
		st_sprint(errmsg, "Failed to return successful status after QUIT.");
		outcome = false;
	}

	client_close(client);
	return outcome;
}

START_TEST (check_pop_network_simple_s) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (status()) outcome = check_pop_network_simple_sthread(errmsg);

	log_test("POP / NETWORK / SIMPLE CHECK:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

Suite * suite_check_pop(void) {

	Suite *s = suite_create("\tPOP");

	suite_check_testcase(s, "POP", "POP Network Simple Check/S", check_pop_network_simple_s);
	return s;
}
