
/**
 * @file /magma/check/magma/servers/pop/pop_check_network.c
 *
 * @brief Functions used to test POP connections over a network connection.
 *
 */

#include "magma_check.h"

/**
 * Calls client_read_line on a client until it reaches a period only line.
 *
 * @param client The client to read from (which should be connected to a POP server).
 * @return true if a line containing a single period is found, false if not.
 */
bool_t check_pop_client_read_lines_to_end(client_t *client) {

	// TODO: Add a timeout mechanism to client_read_line and update this function.
	while (client_read_line(client) > 0) {
		if (!st_cmp_cs_eq(&(client->line), NULLER(".\r\n"))) return true;
	}
	return false;
}

bool_t check_pop_network_basic_sthread(stringer_t *errmsg, uint32_t port) {

	client_t *client = NULL;

	// Connect the client.
	if (!(client = client_connect("localhost", port)) || client_read_line(client) <= 0 ||
			client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("+OK"))) {

		st_sprint(errmsg, "Failed to connect with the POP server.");
		client_close(client);
		return false;
	}

	// Test the USER command.
	if (client_print(client, "USER princess\r\n") <= 0 || client_read_line(client) <= 0 ||
			client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("+OK"))) {

		st_sprint(errmsg, "Failed to return a successful state after USER.");
		client_close(client);
		return false;
	}

	// Test the PASS command.
	if (client_print(client, "PASS password\r\n") <= 0 || client_read_line(client) <= 0 ||
			client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("+OK"))) {

		st_sprint(errmsg, "Failed to return a successful state after USER.");
		client_close(client);
		return false;
	}

	// Test the LIST command.
	if (client_print(client, "LIST\r\n") <= 0 || !check_pop_client_read_lines_to_end(client) ||
			client_status(client) != 1) {

		st_sprint(errmsg, "Failed to return a successful state after LIST.");
		client_close(client);
		return false;
	}

	// Test the RETR command.
	if (client_print(client, "RETR 1\r\n") <= 0 || !check_pop_client_read_lines_to_end(client) ||
			client_status(client) != 1) {

		st_sprint(errmsg, "Failed to return a successful state after RETR.");
		client_close(client);
		return false;
	}

	// Test the DELE command.
	if (client_print(client, "DELE 1\r\n") <= 0 || client_read_line(client) <= 0 || client_status(client) != 1 ||
			st_cmp_cs_starts(&(client->line), NULLER("+OK"))) {

		st_sprint(errmsg, "Failed to return a successful state after DELE.");
		client_close(client);
		return false;
	}

	// Test the QUIT command.
	if (client_print(client, "QUIT 1\r\n") <= 0 || client_read_line(client) <= 0 || client_status(client) != 1 ||
			st_cmp_cs_starts(&(client->line), NULLER("+OK"))) {

		st_sprint(errmsg, "Failed to return a successful state after QUIT.");
		client_close(client);
		return false;
	}

	client_close(client);

	return true;
}
