
/**
 * @file /magma/check/magma/servers/pop/pop_check_network.c
 *
 * @brief Functions used to test POP connections over a network connection.
 *
 */

#include "magma_check.h"

bool_t check_servers_line_presence(client_t *client, stringer_t *line, stringer_t *last) {

	while (st_cmp_cs_eq(&(client->line), last) != 0 && client_read_line(client) > 0) {
		if (st_cmp_cs_eq(&(client->line), line) == 0) return true;
	}
	return false;
}

/**
 * @brief 	Calls client_read_line on a client until it reaches a period only line, returning
 * 			the number of messages in the inbox.
 *
 * @param 	client 	The client_t pointer to read from (which should be connected to a POP server)
 * @param	size	A uint64_t pointer. If not NULL, the total size of the lines read will be placed
 * 					at this address.
 * @param	token	If not NULL, then the size variable will only include the number of bytes read after the token.
 * @return 	true if a line containing a single period is found, false if not.
 */
bool_t check_pop_client_read_end(client_t *client, uint64_t *size, chr_t *token) {

	bool_t token_found = false;

	if (size) *size = 0;
	else if (!token) token_found = true;

	// There shouldn't be a token, if we aren't also supposed to be counting the number of bytes.
	else if (!size && token) return false;

	while (client_read_line(client) > 0) {

		// Break when a line with just a period is found.
		if (!st_cmp_cs_eq(&(client->line), NULLER(".\r\n"))) return true;

		// If we have a size and a token, then keep checking for the token until its found.
		else if (size && token && !token_found && st_cmp_cs_starts(&(client->line), NULLER(token)) == 0) token_found = true;

		if (size && token_found) *size += pl_length_get(client->line);
	}

	return false;
}

/**
 * @brief	Calls client_read_line until it reaches a line containing only a period, then returns the number
 * 			of messages it encountered.
 *
 * @param 	client 	The client_t pointer to read from (which should be connected to a POP server).
 * @param 	errmsg	The stringer_t pointer to which error messages will be printed in event of an error.
 * @return 	an uint32_t containing the number of messages in the inbox.
 */
uint64_t check_pop_client_read_list(client_t *client, stringer_t *errmsg) {

	placer_t fragment = pl_null();
	uint64_t counter = 1, sequence = 0;


	/// LOW: Parse out the total message number from the first line returned and check against that at the end of the
	/// 	function, returning an error if it and the counter do not match.
	if (client_read_line(client) <= 0 || !pl_starts_with_char(client->line, '+')) {
		st_sprint(errmsg, "The message list response failed to return a valid response.");
		return 0;
	}
	while (client_read_line(client) > 0 && !pl_starts_with_char(client->line, '.')) {

		// If the sequence number doesn't match our counter variable, we'll indicate an error.
		if (tok_get_st(&(client->line), ' ', 0, &fragment) >= 0 && !uint64_conv_pl(fragment, &sequence) == 0 && sequence != counter) {
			st_sprint(errmsg, "The message sequence appears to have skipped, because the internal counter no longer matches the sequence.");
			return 0;
		}

		counter++;
	}

	return counter - 1;
}

/// LOW: This should use stringer parameters.
bool_t check_pop_client_auth(client_t *client, chr_t *user, chr_t *pass, stringer_t *errmsg) {

	if (client_print(client, "USER %s\r\n", user) != (ns_length_get(user) + 7) || client_read_line(client) <= 0 ||
		client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("+OK"))) {

		st_sprint(errmsg, "Failed to return a successful state after USER.");
		return false;
	}
	else if (client_print(client, "PASS %s\r\n", pass) != (ns_length_get(pass) + 7) || client_read_line(client) <= 0 ||
		client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("+OK"))) {

		st_sprint(errmsg, "Failed to return a successful state after PASS.");
		return false;
	}
	return true;
}

bool_t check_pop_network_basic_sthread(stringer_t *errmsg, uint32_t port, bool_t secure) {

	uint64_t message_num;
	client_t *client = NULL;

	// Connect the client.
	if (!(client = client_connect("localhost", port)) || (secure && (client_secure(client) == -1)) ||
		!net_set_timeout(client->sockd, 20, 20) || client_read_line(client) <= 0 || client_status(client) != 1 ||
		st_cmp_cs_starts(&(client->line), NULLER("+OK"))) {

		st_sprint(errmsg, "Failed to connect with the POP server.");
		client_close(client);
		return false;
	}

	// Test the USER and PASS commands with incorrect credentials.
	else if (client_write(client, PLACER("USER princess\r\n", 15)) != 15 || client_read_line(client) <= 0 ||
		client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("+OK"))) {

		st_sprint(errmsg, "Failed to return a successful state after USER.");
		client_close(client);
		return false;
	}
	else if (client_write(client, PLACER("PASS lavabit\r\n", 14)) != 14 || client_read_line(client) <= 0 ||
		client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("-ERR"))) {

		st_sprint(errmsg, "Failed to return an error state after PASS with incorrect credentials.");
		client_close(client);
		return false;
	}

	// Test the USER and PASS commands with correct credentials.
	else if (client_write(client, PLACER("USER princess\r\n", 15)) != 15 || client_read_line(client) <= 0 ||
		client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("+OK"))) {

		st_sprint(errmsg, "Failed to return a successful state after USER.");
		client_close(client);
		return false;
	}
	else if (client_write(client, PLACER("PASS password\r\n", 15)) != 15 || client_read_line(client) <= 0 ||
		client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("+OK"))) {

		st_sprint(errmsg, "Failed to return a successful state after USER.");
		client_close(client);
		return false;
	}

	// Test the LIST command.
	else if (client_write(client, PLACER("LIST\r\n", 6)) != 6 || !(message_num = check_pop_client_read_list(client, errmsg)) ||
		client_status(client) != 1) {

		if (!errmsg) st_sprint(errmsg, "Failed to return a successful state after LIST.");
		client_close(client);
		return false;
	}

	// Test the RETR command.
	else if (client_write(client, PLACER("RETR 1\r\n", 8)) != 8 || !check_pop_client_read_end(client, NULL, NULL) ||
		client_status(client) != 1) {

		st_sprint(errmsg, "Failed to return a successful state after RETR.");
		client_close(client);
		return false;
	}

	// Test the DELE command.
	else if (client_write(client, PLACER("DELE 1\r\n", 8)) != 8 || client_read_line(client) <= 0 || client_status(client) != 1 ||
		st_cmp_cs_starts(&(client->line), NULLER("+OK"))) {

		st_sprint(errmsg, "Failed to return a successful state after DELE.");
		client_close(client);
		return false;
	}

	// Test the NOOP command.
	else if (client_write(client, PLACER("NOOP\r\n", 6)) != 6 || client_read_line(client) <= 0 || client_status(client) != 1 ||
		st_cmp_cs_starts(&(client->line), NULLER("+OK"))) {

		st_sprint(errmsg, "Failed to return a successful state after NOOP.");
		client_close(client);
		return false;
	}

	// Test the TOP command.
	else if (client_print(client, "TOP %lu 0\r\n", message_num) != (uint16_digits(message_num) + 8) ||
		client_status(client) != 1 || client_read_line(client) <= 0 || st_cmp_cs_starts(&(client->line), NULLER("+OK"))||
		!check_pop_client_read_end(client, NULL, NULL)) {

		st_sprint(errmsg, "Failed to return a successful state after TOP.");
		client_close(client);
		return false;
	}

	// Test the RSET command.
	else if (client_write(client, PLACER("RSET\r\n", 6)) != 6 || client_read_line(client) <= 0 || client_status(client) != 1 ||
		st_cmp_cs_eq(&(client->line), NULLER("+OK All messages were reset.\r\n"))) {

		st_sprint(errmsg, "Failed to return a successful state after RSET.");
		client_close(client);
		return false;
	}

	// Test the QUIT command.
	else if (client_write(client, PLACER("QUIT 1\r\n", 8)) != 8 || client_read_line(client) <= 0 || client_status(client) != 1 ||
		st_cmp_cs_starts(&(client->line), NULLER("+OK"))) {

		st_sprint(errmsg, "Failed to return a successful state after QUIT.");
		client_close(client);
		return false;
	}

	client_close(client);

	return true;
}

bool_t check_pop_network_stls_ad_sthread(stringer_t *errmsg, uint32_t tcp_port, uint32_t tls_port) {

	client_t *client = NULL;

	// Connect the client over TCP.
	if (!(client = client_connect("localhost", tcp_port)) || !net_set_timeout(client->sockd, 20, 20) ||
		client_read_line(client) <= 0 || client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("+OK"))) {

		st_sprint(errmsg, "Failed to connect with the POP server.");
		client_close(client);
		return false;
	}
	// Check for the presence of the STLS capability in the CAPA list over an insecure connection.
	else if (client_write(client, PLACER("CAPA\r\n", 6)) != 6 ||
		!check_servers_line_presence(client, PLACER("STLS\r\n", 6), PLACER(".\r\n", 3)) ||
		!check_pop_client_read_end(client, NULL, NULL)) {

		st_sprint(errmsg, "Failed to find the STLS capability in the CAPA list over TCP.");
		client_close(client);
		return false;
	}

	client_close(client);
	client = NULL;

	// Connect the client over TLS.
	if (!(client = client_connect("localhost", tls_port)) || client_secure(client) != 0) {

		st_sprint(errmsg, "Failed to connect with the POP server.");
		client_close(client);
		return false;
	}

	// Check for the absence of the STLS capability.
	else if (client_write(client, PLACER("CAPA\r\n", 6)) != 6 ||
		check_servers_line_presence(client, PLACER("STLS\r\n", 6), PLACER(".\r\n", 3)) ||
		!check_pop_client_read_end(client, NULL, NULL)) {

		st_sprint(errmsg, "The STLS capability is still advertised over TLS.");
		client_close(client);
		return false;
	}

	client_close(client);
	return true;
}
