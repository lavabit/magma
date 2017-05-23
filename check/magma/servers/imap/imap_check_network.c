
/**
 * @file /magma/check/magma/servers/imap/imap_check_helpers.c
 *
 * @brief Functions used to test IMAP connections over a network connection.
 *
 */

#include "magma_check.h"

/**
 * @brief 	Calls client_read_line on a client until it finds a line matching "<tag> OK".
 *
 * @param client 	The client to read from (which should be connected to an IMAP server).
 * @param token 	The unique token that identifies the current imap command dialogue.
 *
 * @return 	Returns true if client_read_line was successful until the last line was found.
 * 			specified in num and there was no error. Otherwise returns false.
 */
bool_t check_imap_client_read_end(client_t *client, chr_t *tag) {

	bool_t outcome = false;
	stringer_t *last_line = st_merge("ss", NULLER(tag), NULLER(" OK"));

	while (!outcome && client_read_line(client) > 0) {
		if (!st_cmp_cs_starts(&client->line, last_line)) outcome = true;
	}

	st_cleanup(last_line);
	return outcome;
}

/**
 * @brief 	Prints the LOGIN command to the passed client using the passed credentials.
 *
 * @param	client	The client_t* to print the commands to. It should be connected to an IMAP server.
 * @param	user	A chr_t* holding the username to use in the LOGIN command.
 * @param	pass	A chr_t* holding the password to use in the LOGIN command.
 * @param	tag		A chr_t* holding the tag to place at the beginning of the LOGIN command.
 * @param	errmsg	A stringer_t* into which the error message will be printed in the even of an error.
 * @return	True if the LOGIN command was successful, otherwise false.
 */
bool_t check_imap_client_login(client_t *client, chr_t *user, chr_t *pass, chr_t *tag, stringer_t *errmsg) {

	stringer_t *login_line = NULL;

	// Test the LOGIN command.
	if (client_print(client, "%s LOGIN %s %s\r\n", tag, user, pass) <= 0 ||
		!check_imap_client_read_end(client, tag) || client_status(client) != 1 ||
		st_cmp_cs_starts(&(client->line), NULLER(tag))) {

		st_sprint(errmsg, "Failed to return a successful state after LOGIN.");
		return false;
	}

	st_cleanup(login_line);
	return true;
}

/**
 * @brief	Prints the SELECT command to the passed client using the passed parameters.
 *
 * @param	client	The client_t* to print the command to. It should be connected to an IMAP server.
 * @param	folder	A chr_t* holding the name of the folder to select.
 * @param	tag		A chr_t* holding the tag to place at the beginning of the SELECT command.
 * @param	errmsg	A stringer_t* into which the error message will be printed in the even of an error.
 * @return	True if the SELECT command was successful, otherwise false.
 */
bool_t check_imap_client_select(client_t *client, chr_t *folder, chr_t *tag, stringer_t *errmsg) {

	// Test the SELECT command.
	if (client_print(client, "%s SELECT Inbox\r\n", tag) != (ns_length_get(tag) + 15) ||
		!check_imap_client_read_end(client, tag) || client_status(client) != 1 ||
		st_cmp_cs_starts(&(client->line), NULLER(tag))) {

		st_sprint(errmsg, "Failed to return a successful state after SELECT.");
		return false;
	}

	return true;
}

/**
 * @brief	Prints the CLOSE and LOGOUT commands to the passed client using the passed tag.
 *
 * @param	client	The client_t* to print the commands to. It should be connected to an IMAP server.
 * @param	tag_num	A uint32_t holding the sequence tag number from the last command.
 * @param	errmsg	A stringer_t* into which the error message will be printed in the even of an error.
 * @return	True if the commands were successful, otherwise false.
 */
bool_t check_imap_client_close_logout(client_t *client, uint32_t tag_num, stringer_t *errmsg) {

	stringer_t *tag = NULL, *command = NULL, *success = NULL;

	tag_num += 1;

	// Construct the tag, close_command, and success stringers for CLOSE.
	if (!(tag = st_alloc(1024)) || (st_sprint(tag, "A%u", tag_num) != uint32_digits(tag_num)+1) ||
		!(command = st_merge("sn", tag, " CLOSE\r\n")) || !(success = st_merge("sn", tag, " OK"))) {

		st_sprint(errmsg, "Failed to construct tag, command, or success strings for CLOSE.");
		st_cleanup(tag, command, success);
		return false;
	}
	// Test the CLOSE command.
	else if (client_print(client, st_char_get(command)) != st_length_get(command) ||
		!check_imap_client_read_end(client, st_char_get(tag)) || client_status(client) != 1 ||
		st_cmp_cs_starts(&(client->line), success)) {

		st_sprint(errmsg, "Failed to return a successful state after CLOSE.");
		st_cleanup(tag, command, success);
		return false;
	}

	tag_num += 1;

	st_free(tag);
	st_free(command);
	st_free(success);

	// Construct the tag, close_command, and success stringers for LOGOUT.
	if (!(tag = st_alloc(1024)) || (st_sprint(tag, "A%u", tag_num) != uint32_digits(tag_num)+1) ||
		!(command = st_merge("sn", tag, " LOGOUT\r\n")) || !(success = st_merge("sn", tag, " OK"))) {

		st_sprint(errmsg, "Failed to construct tag, command, or success strings for LOGOUT.");
		st_cleanup(tag, command, success);
		return false;
	}
	// Test the LOGOUT command.
	else if (client_print(client, st_char_get(command)) != st_length_get(command) ||
		!check_imap_client_read_end(client, st_char_get(tag)) || client_status(client) != 1 ||
		st_cmp_cs_starts(&(client->line), success)) {

		st_sprint(errmsg, "Failed to return a successful state after LOGOUT.");
		st_cleanup(tag, command, success);
		return false;
	}

	st_cleanup(tag, command, success);

	return true;
}

bool_t check_imap_network_basic_sthread(stringer_t *errmsg, uint32_t port, bool_t secure) {

	client_t *client = NULL;

	// Check the initial response.
	if (!(client = client_connect("localhost", port)) || (secure && (client_secure(client) == -1)) ||
		!net_set_timeout(client->sockd, 20, 20) || client_read_line(client) <= 0 || (client->status != 1) ||
		st_cmp_cs_starts(&(client->line), NULLER("* OK"))) {

		st_sprint(errmsg, "Failed to connect with the IMAP server.");
		client_close(client);
		return false;
	}

	// Test the LOGIN command.
	else if (client_print(client, "A1 LOGIN princess password\r\n") <= 0 || !check_imap_client_read_end(client, "A1") ||
			client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("A1 OK"))) {

		st_sprint(errmsg, "Failed to return a successful state after LOGIN.");
		client_close(client);
		return false;
	}

	// Test the SELECT command.
	else if (client_print(client, "A2 SELECT Inbox\r\n") <= 0 || !check_imap_client_read_end(client, "A2") ||
			client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("A2 OK"))) {

		st_sprint(errmsg, "Failed to return a successful state after SELECT.");
		client_close(client);
		return false;
	}

	// Test the FETCH command.
	else if (client_print(client, "A3 FETCH 1 RFC822\r\n") <= 0 || !check_imap_client_read_end(client, "A3") ||
			client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("A3 OK"))) {

		st_sprint(errmsg, "Failed to return a successful state after FETCH.");
		client_close(client);
		return false;
	}

	// Test the CLOSE command.
	else if (client_print(client, "A4 CLOSE\r\n") <= 0 || !check_imap_client_read_end(client, "A4") ||
			client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("A4 OK"))) {

		st_sprint(errmsg, "Failed to return a successful state after CLOSE.");
		client_close(client);
		return false;
	}

	/// HIGH: Test other IMAP commands, like LIST, CREATE, TAG, APPEND.

	// Test the LOGOUT command.
	else if (client_print(client, "A5 LOGOUT\r\n") <= 0 || !check_imap_client_read_end(client, "A5") ||
			client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("A5 OK"))) {

		st_sprint(errmsg, "Failed to return a successful state after LOGOUT.");
		client_close(client);
		return false;
	}

	client_close(client);

	return true;
}

bool_t check_imap_network_search_sthread(stringer_t *errmsg, uint32_t port, bool_t secure) {

	uint32_t tag_num = 0;
	client_t *client = NULL;
	stringer_t *tag = NULL, *success = NULL;
	chr_t *commands[] = {
		"SEARCH 1 ALL\r\n",
		"SEARCH 1 ANSWERED\r\n",
		"SEARCH 1 BCC\r\n",
		"SEARCH 1 BEFORE 01-Apr-2017\r\n",
		"SEARCH 1 BODY Hello\r\n",
		"SEARCH 1 CC\r\n",
		"SEARCH 1 DELETED\r\n",
		"SEARCH 1 FLAGGED\r\n",
		"SEARCH 1 FROM ladar@lavabit.com\r\n",
		"SEARCH 1 HEADER lavabit\r\n",
		"SEARCH 1 KEYWORD Seen\r\n",
		"SEARCH 1 LARGER 1024\r\n",
		"SEARCH 1 NEW\r\n",
		"SEARCH 1 NOT Seen\r\n",
		"SEARCH 1 OLD\r\n",
		"SEARCH 1 ON 23-Mar-2017\r\n",
		"SEARCH 1 OR Seen Flagged\r\n",
		"SEARCH 1 RECENT\r\n",
		"SEARCH 1 SEEN\r\n",
		"SEARCH 1 SENTBEFORE 23-Mar-2017\r\n",
		"SEARCH 1 SENTON 23-Mar-2017\r\n",
		"SEARCH 1 SENTSINCE 01-Jan-2017\r\n",
		"SEARCH 1 SINCE 01-Jan-2017\r\n",
		"SEARCH 1 SMALLER 30960\r\n",
		"SEARCH 1 SUBJECT lavabit\r\n",
		"SEARCH 1 TEXT lavabit\r\n",
		"SEARCH 1 TO ladar@lavabit.com\r\n",
		"SEARCH 1 UID 1\r\n",
		"SEARCH 1 UNANSWERED\r\n",
		"SEARCH 1 UNDELETED\r\n",
		"SEARCH 1 UNDRAFT\r\n",
		"SEARCH 1 UNFLAGGED\r\n",
		"SEARCH 1 UNKEYWORD Seen\r\n",
		"SEARCH 1 UNSEEN\r\n"
	};

	// Check the initial response.
	if (!(client = client_connect("localhost", port)) || (secure && (client_secure(client) == -1)) ||
		!net_set_timeout(client->sockd, 20, 20) || client_read_line(client) <= 0 || (client->status != 1) ||
		st_cmp_cs_starts(&(client->line), NULLER("* OK"))) {
		st_sprint(errmsg, "Failed to connect with the IMAP server.");
		client_close(client);
		return false;
	}
	// Test the LOGIN command.
	else if (!check_imap_client_login(client, "princess", "password", "A0", errmsg)) {
		client_close(client);
		return false;
	}
	// Test the SELECT command.
	else if (!check_imap_client_select(client, "Inbox", "A1", errmsg)) {
		client_close(client);
		return false;
	}

	// Test each of the SEARCH commands.
	for (uint32_t i = 0; i < sizeof(commands)/sizeof(chr_t*); i++) {

		tag_num = i + 2;

		if (!(tag = st_alloc(uint32_digits(tag_num) + 2)) || (st_sprint(tag, "A%u", tag_num) != uint32_digits(tag_num) + 1) ||
			!(success = st_merge("sn", tag, " OK Search complete.\r\n"))) {

			st_sprint(errmsg, "Failed to construct the tag or success strings. { i = %d }", i);
			st_cleanup(tag, success);
			client_close(client);
			return false;
		}
		else if (client_print(client, "%s %s\r\n", st_char_get(tag), commands[i]) <= 0 ||
			!check_imap_client_read_end(client, st_char_get(tag)) || client_status(client) != 1 ||
			st_cmp_cs_eq(&(client->line), success)) {

			st_sprint(errmsg, "Failed to return a successful status. { command = \"%s\" }", commands[i]);
			st_cleanup(tag, success);
			client_close(client);
			return false;
		}

		st_free(success);
		st_free(tag);
	}

	// Test the CLOSE and LOGOUT commands;
	if (!check_imap_client_close_logout(client, tag_num+1, errmsg)) {

		client_close(client);
		return false;
	}

	client_close(client);

	return true;
}

bool_t check_imap_network_fetch_sthread(stringer_t *errmsg, uint32_t port, bool_t secure) {

	uint32_t tag_num = 0;
	client_t *client = NULL;
	stringer_t *tag = NULL, *success = NULL;
	chr_t *commands[] = {
		"FETCH 1:* FLAGS\r\n",
		"FETCH 1 (BODY [])\r\n",
		"FETCH 1:* INTERNALDATE\r\n"
		"FETCH 1 (BODY [HEADER])\r\n",
		"FETCH 1 (BODY.PEEK[HEADER])\r\n",
		"FETCH 1 (FLAGS BODY[HEADER.FIELDS (DATE FROM SUBJECT)])\r\n"
	};

	// Check the initial response.
	if (!(client = client_connect("localhost", port)) || (secure && (client_secure(client) == -1)) ||
		!net_set_timeout(client->sockd, 20, 20) || client_read_line(client) <= 0 || (client->status != 1) ||
		st_cmp_cs_starts(&(client->line), NULLER("* OK"))) {

		st_sprint(errmsg, "Failed to connect with the IMAP server.");
		client_close(client);
		return false;
	}
	// Test the LOGIN command.
	else if (!check_imap_client_login(client, "princess", "password", "A0", errmsg)) {
		client_close(client);
		return false;
	}
	// Test the SELECT command.
	else if (!check_imap_client_select(client, "Inbox", "A1", errmsg)) {
		client_close(client);
		return false;
	}
	// Test each of the commands.
	for (uint32_t i = 0; i < sizeof(commands)/sizeof(chr_t*); i++) {

		tag_num = i + 2;

		if (!(tag = st_alloc(uint32_digits(tag_num) + 2)) || (st_sprint(tag, "A%u", tag_num) != uint32_digits(tag_num) + 1) ||
			!(success = st_merge("sn", tag, " OK"))) {

			st_sprint(errmsg, "Failed to construct the tag or success strings. { i = %d }", i);
			st_cleanup(tag, success);
			client_close(client);
			return false;
		}
		else if (client_print(client, "%s %s\r\n", st_char_get(tag), commands[i]) <= 0 ||
			!check_imap_client_read_end(client, st_char_get(tag)) || client_status(client) != 1 ||
			st_cmp_cs_starts(&(client->line), success)) {

			st_sprint(errmsg, "Failed to return a successful status. { command = \"%s\" }", commands[i]);
			st_cleanup(tag, success);
			client_close(client);
			return false;
		}

		st_cleanup(tag, success);
	}
	// Test the CLOSE and LOGOUT commands;
	if (!check_imap_client_close_logout(client, tag_num+1, errmsg)) {
		client_close(client);
		return false;
	}

	client_close(client);

	return true;
}

bool_t check_imap_network_starttls_ad_sthread(stringer_t *errmsg, uint32_t tcp_port, uint32_t tls_port) {

	size_t location = 0;
	client_t *client = NULL;

	// Check the initial response.
	if (!(client = client_connect("localhost", tcp_port)) || !net_set_timeout(client->sockd, 20, 20) ||
		client_read_line(client) <= 0 || client->status != 1 || st_cmp_cs_starts(&(client->line), NULLER("* OK"))) {

		st_sprint(errmsg, "Failed to connect with the IMAP server over TCP.");
		client_close(client);
		return false;
	}
	// Check for STARTTLS in the capabilities when connected over TCP.
	else if (client_write(client, PLACER("A0 CAPABILITY\r\n", 15)) != 15 || client_read_line(client) <= 0 ||
		!st_search_cs(&(client->line), PLACER("STARTTLS", 8), &location)) {

		st_sprint(errmsg, "Failed to find STARTTLS advertised in the IMAP CAPABILITY response over TCP.");
		client_close(client);
		return false;
	}
	// Initiate a TLS handshake and secure the connection.
	else if (client_write(client, PLACER("A1 STARTTLS\r\n", 13)) != 13 || client_read_line(client) <= 0 ||
		client_secure(client)) {

		st_sprint(errmsg, "Failed to completed TLS handshake and secure the connection when connected to the TCP port.");
		client_close(client);
		return false;
	}
	// Check for STARTTLS in the capabilities when connected over TLS.
	else if (client_write(client, PLACER("A0 CAPABILITY\r\n", 15)) != 15 || client_read_line(client) <= 0 ||
		st_search_cs(&(client->line), PLACER("STARTTLS", 8), &location)) {

		st_sprint(errmsg, "IMAP advertised STARTTLS after completing a TLS handshake on the TCP port.");
		client_close(client);
		return false;
	}
	// Close the client.
	else if (!check_imap_client_close_logout(client, 1, errmsg)) {
		client_close(client);
		return false;
	}

	client_close(client);
	client = NULL;

	// Reconnect the client over TLS.
	if (!(client = client_connect("localhost", tls_port)) || client_secure(client)) {

		st_sprint(errmsg, "Failed to connect securely with the IMAP server over TLS.");
		client_close(client);
		return false;
	}
	// Check for the absense of STARTTLS in the capabilities when connected over TLS.
	else if (client_write(client, PLACER("A0 CAPABILITY\r\n", 15)) != 15 || client_read_line(client) <= 0 ||
		st_search_cs(&(client->line), PLACER("STARTTLS", 8), &location)) {

		st_sprint(errmsg, "IMAP advertised STARTTLS when already connected securely on the TLS port.");
		client_close(client);
		return false;
	}
	// Close the client.
	else if (!check_imap_client_close_logout(client, 1, errmsg)) {
		client_close(client);
		return false;
	}

	client_close(client);
	return true;
}
