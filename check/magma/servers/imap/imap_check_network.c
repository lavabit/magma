/**
 * @file /magma/check/magma/servers/smtp/smtp_check_helpers.c
 *
 * @brief Functions used to test IMAP connections over a network connection.
 *
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

	// TODO: Add a timeout mechanism to client_read_line and update this function.
	while (!outcome && client_read_line(client) > 0) {
		if (!st_cmp_cs_starts(&client->line, last_line)) outcome = true;
	}

	st_cleanup(last_line);
	return outcome;
}

bool_t check_imap_network_basic_sthread(stringer_t *errmsg, uint32_t port) {

	client_t *client = NULL;

	// Check the initial response.
	if (!(client = client_connect("localhost", port)) || client_read_line(client) <= 0 || (client->status != 1) ||
			st_cmp_cs_starts(&(client->line), NULLER("* OK"))) {

		st_sprint(errmsg, "Failed to connect with the IMAP server.");
		client_close(client);
		return false;
	}

	// Test the LOGIN command.
	else if (client_print(client, "A1 LOGIN princess password\r\n") <= 0 || !check_imap_client_read_lines_to_end(client, "A1") ||
			client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("A1 OK"))) {

		st_sprint(errmsg, "Failed to return a successful state after LOGIN.");
		client_close(client);
		return false;
	}

	// Test the SELECT command.
	else if (client_print(client, "A2 SELECT Inbox\r\n") <= 0 || !check_imap_client_read_lines_to_end(client, "A2") ||
			client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("A2 OK"))) {

		st_sprint(errmsg, "Failed to return a successful state after SELECT.");
		client_close(client);
		return false;
	}

	// Test the FETCH command.
	else if (client_print(client, "A3 FETCH 1 RFC822\r\n") <= 0 || !check_imap_client_read_lines_to_end(client, "A3") ||
			client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("A3 OK"))) {

		st_sprint(errmsg, "Failed to return a successful state after FETCH.");
		client_close(client);
		return false;
	}

	// Test the LOGOUT command.
	else if (client_print(client, "A4 LOGOUT\r\n") <= 0 || !check_imap_client_read_lines_to_end(client, "A4") ||
			client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("A4 OK"))) {

		st_sprint(errmsg, "Failed to return a successful state after LOGOUT.");
		client_close(client);
		return false;
	}

	client_close(client);

	return true;
}
