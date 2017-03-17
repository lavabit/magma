
/**
 * @file /magma/check/magma/servers/smtp/smtp_check_helpers.c
 *
 * @brief Functions used to test SMTP connections over a network connection.
 *
 */

#include "magma_check.h"

/**
 * Calls client_read_line on a client until the last line is found, checking for errors.
 *
 * @param client The client to read from (which should be connected to an SMTP server).
 *
 * @return Returns true if client_read_line was successful until the last line was found.
 * Otherwise returns false.
 */
bool_t check_smtp_client_read_line_to_end(client_t *client) {

	while (client_read_line(client) > 0) {
		if (pl_char_get(client->line)[3] == ' ') return true;
	}
	return false;
}

bool_t check_smtp_network_simple_sthread(stringer_t *errmsg, uint32_t port) {

	size_t location = 0;
	client_t *client = NULL;

	// Test the connect banner.
	if (!(client = client_connect("localhost", port)) || !net_set_timeout(client->sockd, 20, 20) ||
		client_read_line(client) <= 0 || client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("220")) ||
		!st_search_cs(&(client->line), NULLER(" ESMTP "), &location)) {

		st_sprint(errmsg, "Failed to connect with the SMTP server.");
		client_close(client);
		return false;
	}

	// Test the HELO command.
	else if (client_print(client, "HELO localhost\r\n") != 16 || client_read_line(client) <= 0 ||
		client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("250"))) {
		st_sprint(errmsg, "Failed to return successful status after HELO.");
		client_close(client);
		return false;
	}


	// Test the EHLO command.
	else if (client_print(client, "EHLO localhost\r\n") != 16 || !check_smtp_client_read_line_to_end(client) ||
		client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("250"))) {
		st_sprint(errmsg, "Failed to return successful status after EHLO.");
		client_close(client);
		return false;
	}

	// Test the MAIL command.
	else if (client_print(client, "MAIL FROM: <>\r\n") != 15 || client_read_line(client) <= 0 ||
		client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("250"))) {
		st_sprint(errmsg, "Failed to return successful status after MAIL.");
		client_close(client);
		return false;
	}

	// Test the RCPT command.
	else if (client_print(client, "RCPT TO: <princess@example.com>\r\n") != 33 || client_read_line(client) <= 0 ||
		client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("250"))) {
		st_sprint(errmsg, "Failed to return successful status after RCPT.");
		client_close(client);
		return false;
	}

	// Test the DATA command.
	else if (client_print(client, "DATA\r\n") != 6 || client_read_line(client) <= 0 ||
		client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("354"))) {
		st_sprint(errmsg, "Failed to return a proceed status code after DATA.");
		client_close(client);
		return false;
	}

	// Test sending the contents of an email.
	else if (client_print(client, "To: magma@lavabit.com\r\nFrom: princess@example.com\r\nSubject: Unit Tests\r\n\r\n" \
		"Aren't unit tests great?\r\n.\r\n") != 103 || client_read_line(client) <= 0 ||
		client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("250"))) {
		st_sprint(errmsg, "Failed to get a successful status code after email submission.");
		client_close(client);
		return false;
	}

	// Test the QUIT command.
	else if (client_print(client, "QUIT\r\n") != 6 || client_read_line(client) <= 0 ||
		client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("221"))) {
		st_sprint(errmsg, "Failed to return successful status following the QUIT command.");
		client_close(client);
		return false;
	}

	else if (client_read_line(client) > 0) {
		st_sprint(errmsg, "The server failed to close the connection after issuing a QUIT command.");
		client_close(client);
		return false;
	}

	client_close(client);

	return true;
}
