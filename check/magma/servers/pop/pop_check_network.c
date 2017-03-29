
/**
 * @file /magma/check/magma/servers/pop/pop_check_network.c
 *
 * @brief Functions used to test POP connections over a network connection.
 *
 */

#include "magma_check.h"

/**
 * @brief 	Calls client_read_line on a client until it reaches a period only line, returning
 * 			the number of messages in the inbox.
 *
 * @param 	client 	The client_t* to read from (which should be connected to a POP server)
 * @param	size	A uint64_t*. If not null, the total size of the lines read will be placed
 * 					at this address.
 * @param	token	If not NULL and size if not NULL, then size will only be incremented after
 * 					reaching a line that begins with token.
 * @return 	true if a line containing a single period is found, false if not.
 */
bool_t check_pop_client_read_end(client_t *client, uint64_t *size, chr_t *token) {

	if (size) *size = 0;
	bool_t token_found = false;

	while (client_read_line(client) > 0) {

		if (!st_cmp_cs_eq(&(client->line), NULLER(".\r\n"))) return true;
		else if (size && st_cmp_cs_starts(&(client->line), NULLER(token)) == 0) token_found = true;

		if (size && token_found) *size += pl_length_get(client->line);
	}
	return false;
}

/**
 * Calls 	client_read_line on a client until it reaches a period only line, returning the
 * 			number of messages in the inbox.
 *
 * @param 	client 	The client_t* to read from (which should be connected to a POP server).
 * @param 	errmsg	The stringer_t* to which error messages will be printed in event of an error.
 * @return 	a uint32_t containing the number of messages in the inbox.
 */
uint64_t check_pop_client_read_list(client_t *client, stringer_t *errmsg) {

	placer_t fragment = pl_null();
	uint64_t counter = 1, sequence = 0;

	client_read_line(client);

	while (client_read_line(client) > 0) {

		if (pl_starts_with_char(client->line, '.')) {
			return counter-2;
		}
		else if (tok_get_st(&(client->line), ' ', 0, &fragment) >= 0 && !uint64_conv_pl(fragment, &sequence) == 0) {
			if (sequence != counter) return 0;
		}
		else {
			return 0;
		}

		counter++;
	}

	return 0;
}

bool_t check_pop_client_auth(client_t *client, chr_t *user, chr_t *pass, stringer_t *errmsg) {

	stringer_t *user_command = st_aprint_opts(MANAGED_T | CONTIGUOUS | STACK, "USER %s\r\n", user),
		*pass_command = st_aprint_opts(MANAGED_T | CONTIGUOUS | STACK, "PASS %s\r\n", pass);

	if (client_print(client, st_char_get(user_command)) != st_length_get(user_command) || client_read_line(client) <= 0 ||
		client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("+OK"))) {

		st_sprint(errmsg, "Failed to return a successful state after USER.");
		return false;
	}
	else if (client_print(client, st_char_get(pass_command)) != st_length_get(pass_command) || client_read_line(client) <= 0 ||
		client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("+OK"))) {

		st_sprint(errmsg, "Failed to return a successful state after PASS.");
		return false;
	}
	return true;
}

bool_t check_pop_network_basic_sthread(stringer_t *errmsg, uint32_t port, bool_t secure) {

	uint64_t message_num;
	client_t *client = NULL;
	stringer_t *top_command = NULL;

	// Connect the client.
	if (!(client = client_connect("localhost", port)) || (secure && (client_secure(client) == -1)) ||
		!net_set_timeout(client->sockd, 20, 20) || client_read_line(client) <= 0 || client_status(client) != 1 ||
		st_cmp_cs_starts(&(client->line), NULLER("+OK"))) {

		st_sprint(errmsg, "Failed to connect with the POP server.");
		client_close(client);
		return false;
	}

	// Test the USER and PASS commands with incorrect credentials.
	else if (client_print(client, "USER princess\r\n") != 15 || client_read_line(client) <= 0 ||
		client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("+OK"))) {

		st_sprint(errmsg, "Failed to return a successful state after USER.");
		client_close(client);
		return false;
	}
	else if (client_print(client, "PASS lavabit\r\n") != 14 || client_read_line(client) <= 0 ||
		client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("-ERR"))) {

		st_sprint(errmsg, "Failed to return an error state after PASS with incorrect credentials.");
		client_close(client);
		return false;
	}

	// Test the USER and PASS commands with correct credentials.
	else if (client_print(client, "USER princess\r\n") != 15 || client_read_line(client) <= 0 ||
		client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("+OK"))) {

		st_sprint(errmsg, "Failed to return a successful state after USER.");
		client_close(client);
		return false;
	}
	else if (client_print(client, "PASS password\r\n") != 15 || client_read_line(client) <= 0 ||
		client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("+OK"))) {

		st_sprint(errmsg, "Failed to return a successful state after USER.");
		client_close(client);
		return false;
	}

	// Test the LIST command.
	else if (client_print(client, "LIST\r\n") != 6 || !(message_num = check_pop_client_read_list(client, errmsg)) ||
		client_status(client) != 1) {

		if (!errmsg) st_sprint(errmsg, "Failed to return a successful state after LIST.");
		client_close(client);
		return false;
	}

	// Test the RETR command.
	else if (client_print(client, "RETR 1\r\n") != 8 || !check_pop_client_read_end(client, NULL, NULL) ||
		client_status(client) != 1) {

		st_sprint(errmsg, "Failed to return a successful state after RETR.");
		client_close(client);
		return false;
	}

	// Test the DELE command.
	else if (client_print(client, "DELE 1\r\n") != 8 || client_read_line(client) <= 0 || client_status(client) != 1 ||
		st_cmp_cs_starts(&(client->line), NULLER("+OK"))) {

		st_sprint(errmsg, "Failed to return a successful state after DELE.");
		client_close(client);
		return false;
	}

	// Test the NOOP command.
	else if (client_print(client, "NOOP\r\n") != 6 || client_read_line(client) <= 0 || client_status(client) != 1 ||
		st_cmp_cs_starts(&(client->line), NULLER("+OK"))) {

		st_sprint(errmsg, "Failed to return a successful state after NOOP.");
		client_close(client);
		return false;
	}

	// Test the TOP command.
	else if (!(top_command = st_aprint_opts(MANAGED_T | CONTIGUOUS | STACK, "TOP %lu 0\r\n", message_num)) ||
		client_print(client, st_char_get(top_command)) != st_length_get(top_command) || client_status(client) != 1 ||
		client_read_line(client) <= 0 || st_cmp_cs_starts(&(client->line), NULLER("+OK"))||
		!check_pop_client_read_end(client, NULL, NULL)) {

		st_sprint(errmsg, "Failed to return a successful state after TOP.");
		client_close(client);
		return false;
	}

	// Test the RSET command.
	else if (client_print(client, "RSET\r\n") != 6 || client_read_line(client) <= 0 || client_status(client) != 1 ||
		st_cmp_cs_eq(&(client->line), NULLER("+OK All messages were reset.\r\n"))) {

		st_sprint(errmsg, "Failed to return a successful state after RSET.");
		client_close(client);
		return false;
	}

	// Test the QUIT command.
	else if (client_print(client, "QUIT 1\r\n") <= 0 || client_read_line(client) <= 0 || client_status(client) != 1 ||
		st_cmp_cs_starts(&(client->line), NULLER("+OK"))) {

		st_sprint(errmsg, "Failed to return a successful state after QUIT.");
		client_close(client);
		return false;
	}

	client_close(client);

	return true;
}
