
/**
 * @file /magma/check/magma/servers/pop/pop_check_network.c
 *
 * @brief Functions used to test POP connections over a network connection.
 *
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

	// If the response is multiline, loop until a line with a single period is found.
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
	return true;
}

bool_t check_pop_network_basic_sthread(stringer_t *errmsg, uint32_t port) {

	client_t *client = NULL;

	// Connect the client.
	if (!(client = client_connect("localhost", port))) {
		st_sprint(errmsg, "Failed to establish a client connection.");
		return false;
	}
	else if (!check_pop_client_read_lines_to_end(client, false) || (client->status != 1)) {
		st_sprint(errmsg, "Failed to return successful status initially.");
		client_close(client);
		return false;
	}

	// Test the USER command.
	if (client_print(client, "USER princess\r\n") <= 0) {
		st_sprint(errmsg, "Failed to write the USER command.");
		client_close(client);
		return false;
	}
	else if (!check_pop_client_read_lines_to_end(client, false) || (client->status != 1)) {
		st_sprint(errmsg, "Failed to return successful status after USER.");
		client_close(client);
		return false;
	}

	// Test the PASS command.
	if (client_print(client, "PASS password\r\n") <= 0) {
		st_sprint(errmsg, "Failed to write the PASS command.");
		client_close(client);
		return false;
	}
	else if (!check_pop_client_read_lines_to_end(client, false) || (client->status != 1)) {
		st_sprint(errmsg, "Failed to return successful status after PASS.");
		client_close(client);
		return false;
	}

	// Test the LIST command.
	if (client_print(client, "LIST\r\n") <= 0) {
		st_sprint(errmsg, "Failed to write the LIST command.");
		client_close(client);
		return false;
	}
	else if (!check_pop_client_read_lines_to_end(client, true) || (client->status != 1)) {
		st_sprint(errmsg, "Failed to return successful status after LIST.");
		client_close(client);
		return false;
	}

	// Test the RETR command.
	if (client_print(client, "RETR 1\r\n") <= 0) {
		st_sprint(errmsg, "Failed to write the RETR command.");
		client_close(client);
		return false;
	}
	else if (!check_pop_client_read_lines_to_end(client, true) || (client->status != 1)) {
		st_sprint(errmsg, "Failed to return successful status after RETR.");
		client_close(client);
		return false;
	}

	// Test the DELE command.
	if (client_print(client, "DELE 1\r\n") <= 0) {
		st_sprint(errmsg, "Failed to write the DELE command.");
		client_close(client);
		return false;
	}
	else if (!check_pop_client_read_lines_to_end(client, false) || (client->status != 1)) {
		st_sprint(errmsg, "Failed to return successful status after DELE.");
		client_close(client);
		return false;
	}

	// Test the QUIT command.
	if (client_print(client, "QUIT\r\n") <= 0) {
		st_sprint(errmsg, "Failed to write the QUIT command.");
		client_close(client);
		return false;
	}
	else if (!check_pop_client_read_lines_to_end(client, false) || (client->status != 1)) {
		st_sprint(errmsg, "Failed to return successful status after QUIT.");
		client_close(client);
		return false;
	}

	client_close(client);

	return true;
}
