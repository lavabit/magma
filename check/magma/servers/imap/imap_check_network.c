
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
	uint32_t login_line_len = ns_length_get(tag) + ns_length_get(user) + ns_length_get(pass) + 10;

	// Construct the login command
	if (!(login_line = st_merge("nsnsns", tag, NULLER(" LOGIN "), user, NULLER(" "), pass, NULLER("\r\n")))) {

		st_sprint(errmsg, "Failed to construct the login command.");
		return false;
	}
	// Test the LOGIN command.
	else if (client_print(client, st_char_get(login_line)) != login_line_len || !check_imap_client_read_end(client, tag) ||
			client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER(tag))) {

		st_sprint(errmsg, "Failed to return a successful state after LOGIN.");
		return false;
	}

	st_cleanup(login_line);
	return true;
}

/**
 * @brief	Prints the SELECT command to the passed client using the passed parameter.
 *
 * @param	client	The client_t* to print the command to. It should be connected to an IMAP server.
 * @param	folder	A chr_t* holding the name of the folder to select.
 * @param	tag		A chr_t* holding the tag to place at the beginning of the SELECT command.
 * @param	errmsg	A stringer_t* into which the error message will be printed in the even of an error.
 * @return	True if the SELECT command was successful, otherwise false.
 */
bool_t check_imap_client_select(client_t *client, chr_t *folder, chr_t *tag, stringer_t *errmsg) {

	// Test the SELECT command.
	if (client_print(client, "%s SELECT Inbox\r\n", tag) <= 0 || !check_imap_client_read_end(client, tag) ||
		client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER(tag))) {

		st_sprint(errmsg, "Failed to return a successful state after SELECT.");
		return false;
	}

	return true;
}

bool_t check_imap_client_close_logout(client_t *client, stringer_t *errmsg) {

	// Test the CLOSE command.
	if (client_print(client, "A4 CLOSE\r\n") <= 0 || !check_imap_client_read_end(client, "A4") ||
			client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("A4 OK"))) {

		st_sprint(errmsg, "Failed to return a successful state after CLOSE.");
		return false;
	}

	// Test the LOGOUT command.
	else if (client_print(client, "A5 LOGOUT\r\n") <= 0 || !check_imap_client_read_end(client, "A5") ||
			client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("A5 OK"))) {

		st_sprint(errmsg, "Failed to return a successful state after LOGOUT.");
		return false;
	}

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
	else if (!check_imap_client_login(client, "princess", "password", "A0", errmsg)) {
		return false;
	}
	// Test the SELECT command.
	else if (!check_imap_client_select(client, "Inbox", "A1", errmsg)) {
		return false;
	}
	// Test SEARCH ALL
	else if (client_print(client, "A2 SEARCH ALL\r\n") <= 0 || !check_imap_client_read_end(client, "A2") ||
			client_status(client) != 1 || st_cmp_cs_eq(&(client->line), NULLER("A2 OK Search complete.\r\n"))) {

		st_sprint(errmsg, "Failed to return a successful status after SEARCH ALL.");
		return false;
	}
	// Test SEARCH ANSWERED
	else if (client_print(client, "A3 SEARCH ANSWERED\r\n") <= 0 || !check_imap_client_read_end(client, "A3") ||
			client_status(client) != 1 || st_cmp_cs_eq(&(client->line), NULLER("A3 OK Search complete.\r\n"))) {

		st_sprint(errmsg, "Failed to return a successful status after SEARCH ANSWERED.");
		return false;
	}
	// Test SEARCH BCC <string>
	else if (client_print(client, "A4 SEARCH BCC\r\n") <= 0 || !check_imap_client_read_end(client, "A4") ||
			client_status(client) != 1 || st_cmp_cs_eq(&(client->line), NULLER("A4 OK Search complete.\r\n"))) {

		st_sprint(errmsg, "Failed to return a successful status after SEARCH BCC.");
		return false;
	}
	// Test SEARCH BEFORE <date>
	// MID: replace the hard coded date with a dynamic one based on the current date.
	else if (client_print(client, "A5 SEARCH BEFORE 01-Apr-2017\r\n") <= 0 || !check_imap_client_read_end(client, "A5") ||
			client_status(client) != 1 || st_cmp_cs_eq(&(client->line), NULLER("A5 OK Search complete.\r\n"))) {

		st_sprint(errmsg, "Failed to return a successful status after SEARCH BEFORE.");
		return false;
	}
	// Test SEARCH BODY <string>
	else if (client_print(client, "A6 SEARCH BODY Hello\r\n") <= 0 || !check_imap_client_read_end(client, "A6") ||
			client_status(client) != 1 || st_cmp_cs_eq(&(client->line), NULLER("A6 OK Search complete.\r\n"))) {

		st_sprint(errmsg, "Failed to return a successful status after SEARCH BODY.");
		return false;
	}
	// Test SEARCH CC <string>
	else if (client_print(client, "A7 SEARCH CC\r\n") <= 0 || !check_imap_client_read_end(client, "A7") ||
			client_status(client) != 1 || st_cmp_cs_eq(&(client->line), NULLER("A7 OK Search complete.\r\n"))) {

		st_sprint(errmsg, "Failed to return a successful status after SEARCH CC.");
		return false;
	}
	// Test SEARCH DELETED
	else if (client_print(client, "A8 SEARCH DELETED\r\n") <= 0 || !check_imap_client_read_end(client, "A8") ||
			client_status(client) != 1 || st_cmp_cs_eq(&(client->line), NULLER("A8 OK Search complete.\r\n"))) {

		st_sprint(errmsg, "Failed to return a successful status after SEARCH DELETED.");
		return false;
	}
	// Test SEARCH DRAFT
	else if (client_print(client, "A9 SEARCH DRAFT\r\n") <= 0 || !check_imap_client_read_end(client, "A9") ||
			client_status(client) != 1 || st_cmp_cs_eq(&(client->line), NULLER("A9 OK Search complete.\r\n"))) {

		st_sprint(errmsg, "Failed to return a successful status after SEARCH DRAFT.");
		return false;
	}
	// Test SEARCH FLAGGED
	else if (client_print(client, "A10 SEARCH FLAGGED\r\n") <= 0 || !check_imap_client_read_end(client, "A10") ||
			client_status(client) != 1 || st_cmp_cs_eq(&(client->line), NULLER("A10 OK Search complete.\r\n"))) {

		st_sprint(errmsg, "Failed to return a successful status after SEARCH FLAGGED.");
		return false;
	}
	// Test SEARCH FROM <string>
	else if (client_print(client, "A11 SEARCH FROM ladar@lavabit.com\r\n") <= 0 || !check_imap_client_read_end(client, "A11") ||
			client_status(client) != 1 || st_cmp_cs_eq(&(client->line), NULLER("A11 OK Search complete.\r\n"))) {

		st_sprint(errmsg, "Failed to return a successful status after SEARCH FROM.");
		return false;
	}
	// Test SEARCH HEADER <field> <string>
	else if (client_print(client, "A12 SEARCH HEADER lavabit\r\n") <= 0 || !check_imap_client_read_end(client, "A12") ||
			client_status(client) != 1 || st_cmp_cs_eq(&(client->line), NULLER("A12 OK Search complete.\r\n"))) {

		st_sprint(errmsg, "Failed to return a successful status after SEARCH HEADER.");
		return false;
	}
	// Test SEARCH KEYWORD <flag>
	else if (client_print(client, "A13 SEARCH KEYWORD Seen\r\n") <= 0 || !check_imap_client_read_end(client, "A13") ||
			client_status(client) != 1 || st_cmp_cs_eq(&(client->line), NULLER("A13 OK Search complete.\r\n"))) {

		st_sprint(errmsg, "Failed to return a successful status after SEARCH KEYWORD.");
		return false;
	}
	// Test SEARCH LARGER <n>
	else if (client_print(client, "A14 SEARCH LARGER 1024\r\n") <= 0 || !check_imap_client_read_end(client, "A14") ||
			client_status(client) != 1 || st_cmp_cs_eq(&(client->line), NULLER("A14 OK Search complete.\r\n"))) {

		st_sprint(errmsg, "Failed to return a successful status after SEARCH LARGER.");
		return false;
	}
	// Test SEARCH NEW
	else if (client_print(client, "A15 SEARCH NEW\r\n") <= 0 || !check_imap_client_read_end(client, "A15") ||
			client_status(client) != 1 || st_cmp_cs_eq(&(client->line), NULLER("A15 OK Search complete.\r\n"))) {

		st_sprint(errmsg, "Failed to return a successful status after SEARCH NEW.");
		return false;
	}
	// Test SEARCH NOT <search-key>
	else if (client_print(client, "A16 SEARCH NOT Seen\r\n") <= 0 || !check_imap_client_read_end(client, "A16") ||
			client_status(client) != 1 || st_cmp_cs_eq(&(client->line), NULLER("A16 OK Search complete.\r\n"))) {

		st_sprint(errmsg, "Failed to return a successful status after SEARCH NOT.");
		return false;
	}
	// Test SEARCH OLD
	else if (client_print(client, "A17 SEARCH OLD\r\n") <= 0 || !check_imap_client_read_end(client, "A17") ||
			client_status(client) != 1 || st_cmp_cs_eq(&(client->line), NULLER("A17 OK Search complete.\r\n"))) {

		st_sprint(errmsg, "Failed to return a successful status after SEARCH OLD.");
		return false;
	}
	// Test SEARCH ON <date>
	// MID: The hard coded date needs to be changed to a dynamic one based on the current date.
	else if (client_print(client, "A18 SEARCH ON 23-Mar-2017\r\n") <= 0 || !check_imap_client_read_end(client, "A18") ||
			client_status(client) != 1 || st_cmp_cs_eq(&(client->line), NULLER("A18 OK Search complete.\r\n"))) {

		st_sprint(errmsg, "Failed to return a successful status after SEARCH ON.");
		return false;
	}
	// Test SEARCH OR <search-key> <search-key>
	else if (client_print(client, "A19 SEARCH OR Seen Flagged\r\n") <= 0 || !check_imap_client_read_end(client, "A19") ||
			client_status(client) != 1 || st_cmp_cs_eq(&(client->line), NULLER("A19 OK Search complete.\r\n"))) {

		st_sprint(errmsg, "Failed to return a successful status after SEARCH OR.");
		return false;
	}
	// Test SEARCH RECENT
	else if (client_print(client, "A20 SEARCH RECENT\r\n") <= 0 || !check_imap_client_read_end(client, "A20") ||
			client_status(client) != 1 || st_cmp_cs_eq(&(client->line), NULLER("A20 OK Search complete.\r\n"))) {

		st_sprint(errmsg, "Failed to return a successful status after SEARCH RECENT.");
		return false;
	}
	// Test SEARCH SEEN
	else if (client_print(client, "A21 SEARCH SEEN\r\n") <= 0 || !check_imap_client_read_end(client, "A21") ||
			client_status(client) != 1 || st_cmp_cs_eq(&(client->line), NULLER("A21 OK Search complete.\r\n"))) {

		st_sprint(errmsg, "Failed to return a successful status after SEARCH SEEN.");
		return false;
	}
	// Test SEARCH SENTBEFORE <date>
	else if (client_print(client, "A22 SEARCH SENTBEFORE 23-Mar-2017\r\n") <= 0 || !check_imap_client_read_end(client, "A22") ||
			client_status(client) != 1 || st_cmp_cs_eq(&(client->line), NULLER("A22 OK Search complete.\r\n"))) {

		st_sprint(errmsg, "Failed to return a successful status after SEARCH SENTBEFORE.");
		return false;
	}
	// Test SEARCH SENTON <date>
	else if (client_print(client, "A23 SEARCH 23-Mar-2017\r\n") <= 0 || !check_imap_client_read_end(client, "A23") ||
			client_status(client) != 1 || st_cmp_cs_eq(&(client->line), NULLER("A23 OK Search complete.\r\n"))) {

		st_sprint(errmsg, "Failed to return a successful status after SEARCH SENTON.");
		return false;
	}
	// Test SEARCH SENTSINCE <date>
	else if (client_print(client, "A24 SEARCH SENTSINCE 01-Jan-2017\r\n") <= 0 || !check_imap_client_read_end(client, "A24") ||
			client_status(client) != 1 || st_cmp_cs_eq(&(client->line), NULLER("A24 OK Search complete.\r\n"))) {

		st_sprint(errmsg, "Failed to return a successful status after SEARCH SENTSINCE.");
		return false;
	}
	// Test SEARCH SINCE <date>
	else if (client_print(client, "A25 SEARCH SINCE 01-Jan-2017\r\n") <= 0 || !check_imap_client_read_end(client, "A25") ||
			client_status(client) != 1 || st_cmp_cs_eq(&(client->line), NULLER("A25 OK Search complete.\r\n"))) {

		st_sprint(errmsg, "Failed to return a successful status after SEARCH SINCE.");
		return false;
	}
	// Test SEARCH SMALLER <n>
	else if (client_print(client, "A26 SEARCH SMALLER 30960\r\n") <= 0 || !check_imap_client_read_end(client, "A26") ||
			client_status(client) != 1 || st_cmp_cs_eq(&(client->line), NULLER("A26 OK Search complete.\r\n"))) {

		st_sprint(errmsg, "Failed to return a successful status after SEARCH SMALLER.");
		return false;
	}
	// Test SEARCH SUBJECT <string>
	else if (client_print(client, "A27 SEARCH SUBJECT lavabit\r\n") <= 0 || !check_imap_client_read_end(client, "A27") ||
			client_status(client) != 1 || st_cmp_cs_eq(&(client->line), NULLER("A27 OK Search complete.\r\n"))) {

		st_sprint(errmsg, "Failed to return a successful status after SEARCH SUBJECT.");
		return false;
	}
	// Test SEARCH TEXT <string>
	else if (client_print(client, "A28 SEARCH TEXT lavabit\r\n") <= 0 || !check_imap_client_read_end(client, "A28") ||
			client_status(client) != 1 || st_cmp_cs_eq(&(client->line), NULLER("A28 OK Search complete.\r\n"))) {

		st_sprint(errmsg, "Failed to return a successful status after SEARCH TEXT.");
		return false;
	}
	// Test SEARCH TO <string>
	else if (client_print(client, "A29 SEARCH TO ladar@lavabit.com\r\n") <= 0 || !check_imap_client_read_end(client, "A29") ||
			client_status(client) != 1 || st_cmp_cs_eq(&(client->line), NULLER("A29 OK Search complete.\r\n"))) {

		st_sprint(errmsg, "Failed to return a successful status after SEARCH TO.");
		return false;
	}
	// Test SEARCH UID <sequence set>
	else if (client_print(client, "A30 SEARCH UID 1\r\n") <= 0 || !check_imap_client_read_end(client, "A30") ||
			client_status(client) != 1 || st_cmp_cs_eq(&(client->line), NULLER("A30 OK Search complete.\r\n"))) {

		st_sprint(errmsg, "Failed to return a successful status after SEARCH UID.");
		return false;
	}
	// Test SEARCH UNANSWERED
	else if (client_print(client, "A31 SEARCH UNANSWERED\r\n") <= 0 || !check_imap_client_read_end(client, "A31") ||
			client_status(client) != 1 || st_cmp_cs_eq(&(client->line), NULLER("A31 OK Search complete.\r\n"))) {

		st_sprint(errmsg, "Failed to return a successful status after SEARCH UNANSWERED.");
		return false;
	}
	// Test SEARCH UNDELETED
	else if (client_print(client, "A32 SEARCH UNDELETED\r\n") <= 0 || !check_imap_client_read_end(client, "A32") ||
			client_status(client) != 1 || st_cmp_cs_eq(&(client->line), NULLER("A32 OK Search complete.\r\n"))) {

		st_sprint(errmsg, "Failed to return a successful status after SEARCH UNDELETED.");
		return false;
	}
	// Test SEARCH UNDRAFT
	else if (client_print(client, "A33 SEARCH UNDRAFT\r\n") <= 0 || !check_imap_client_read_end(client, "A33") ||
			client_status(client) != 1 || st_cmp_cs_eq(&(client->line), NULLER("A33 OK Search complete.\r\n"))) {

		st_sprint(errmsg, "Failed to return a successful status after SEARCH UNDRAFT.");
		return false;
	}
	// Test SEARCH UNFLAGGED
	else if (client_print(client, "A34 SEARCH UNFLAGGED\r\n") <= 0 || !check_imap_client_read_end(client, "A34") ||
			client_status(client) != 1 || st_cmp_cs_eq(&(client->line), NULLER("A34 OK Search complete.\r\n"))) {

		st_sprint(errmsg, "Failed to return a successful status after SEARCH UNFLAGGED.");
		return false;
	}
	// Test SEARCH UNKEYWORD <flag>
	else if (client_print(client, "A35 SEARCH UNKEYWORD Seen\r\n") <= 0 || !check_imap_client_read_end(client, "A35") ||
			client_status(client) != 1 || st_cmp_cs_eq(&(client->line), NULLER("A35 OK Search complete.\r\n"))) {

		st_sprint(errmsg, "Failed to return a successful status after SEARCH UNKEYWORD.");
		return false;
	}
	// Test SEARCH UNSEEN
	else if (client_print(client, "A36 SEARCH UNSEEN\r\n") <= 0 || !check_imap_client_read_end(client, "A36") ||
			client_status(client) != 1 || st_cmp_cs_eq(&(client->line), NULLER("A36 OK Search complete.\r\n"))) {

		st_sprint(errmsg, "Failed to return a successful status after SEARCH UNSEEN.");
		return false;
	}

	return true;
}

bool_t check_imap_network_fetch_sthread(stringer_t *errmsg, uint32_t port, bool_t secure) {

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
	else if (!check_imap_client_login(client, "princess", "password", "A0", errmsg)) {
		return false;
	}
	// Test the SELECT command.
	else if (!check_imap_client_select(client, "Inbox", "A1", errmsg)) {
		return false;
	}
	// Test FETCH 1 (BODY.PEEK[HEADER]) and make sure the message is not marked as seen
	else if (client_print(client, "A2 FETCH 1 (BODY.PEEK[HEADER])\r\n") <= 0 || !check_imap_client_read_end(client, "A2") ||
			client_status(client) != 1 || st_cmp_cs_eq(&(client->line), NULLER("A2 OK Search complete.\r\n"))) {

		st_sprint(errmsg, "Failed to return a successful status after FETCH 1 (BODY.PEEK[HEADER]).");
		return false;
	}
	// Test FETCH 1 (BODY [HEADER])

	// Test FETCH 1 (BODY[])

	// Test FETCH 1 (FLAGS BODY[HEADER.FIELDS (DATE FROM SUBJECT)])

	// Test FETCH 1:* FLAGS

	// Test FETCH 1:* INTERNALDATE

	return true;
}

