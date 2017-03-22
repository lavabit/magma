
/**
 * @file /magma/check/magma/servers/smtp/smtp_check_helpers.c
 *
 * @brief Functions used to test SMTP connections over a network connection.
 *
 */

#include "magma_check.h"

/**
 * @brief 	Calls client_read_line on a client until the last line is found
 * @param 	client The client_t* to read from (which should be connected to an SMTP server).
 * @return 	Returns true if client_read_line was successful until the last line was found.
 * 			Otherwise returns false.
 */
bool_t check_smtp_client_read_end(client_t *client) {

	while (client_read_line(client) > 0) {
		if (pl_char_get(client->line)[3] == ' ') return true;
	}
	return false;
}

/**
 * @brief 	Submits the MAIL TO, RCPT FROM, and DATA commands to the passed client using the passed params
 * @param 	client The client to write the commands to. It should be connected to an SMTP server and have
 * 			had HELO/EHLO issued previously.
 * @baram 	from A chr_t* holding the value to be inserted in the MAIL FROM command
 * @param 	to A chr_t* holding the value to be inserted in the RCPT TO command
 * @param 	errmsg A stringer_t* that will have the error message printed to it in the event of an error
 * @return 	True if no errors, false otherwise
 */
bool_t check_smtp_client_mail_rcpt_data(client_t *client, chr_t *from, chr_t *to, stringer_t *errmsg) {

	chr_t *line_from = "MAIL FROM: <%s>\r\n", *line_to = "RCPT TO: <%s>\r\n";
	size_t size_from = ns_length_get(line_from) + ns_length_get(from) -2, size_to = ns_length_get(line_to) + ns_length_get(to) -2;

	// Issue MAIL command.
	if (client_print(client, line_from, from) != size_from || !check_smtp_client_read_end(client) ||
		client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("250"))) {
		st_sprint(errmsg, "Failed to return successful status after MAIL.");
		return false;
	}

	// Issue RCPT command.
	else if (client_print(client, line_to, to) != size_to || !check_smtp_client_read_end(client) ||
		client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("250"))) {
		st_sprint(errmsg, "Failed to return successful status after RCPT.");
		return false;
	}

	// Issue DATA command.
	else if (client_print(client, "DATA\r\n") != 6 || !check_smtp_client_read_end(client) ||
		client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("354"))) {
		st_sprint(errmsg, "Failed to return a proceed status code after DATA.");
		return false;
	}

	return true;
}

/**
 * @brief 	Submits the AUTH PLAIN command to the passed client using the passed parameters
 * @param 	client A client_t* connected to an SMTP server that has had the HELO/EHLO command
 * 			already submitted
 * @param 	auth The user authentication string
 * @param 	errmsg A stringer_t* that will have the error message printed to it in the event of
 * 			and error
 * @return 	True if no errors, false otherwise
 */
bool_t check_smtp_client_auth_plain(client_t *client, chr_t *auth, stringer_t *errmsg) {

	chr_t *line_auth = "AUTH PLAIN %s\r\n";

	if (client_print(client, line_auth, auth) != (ns_length_get(line_auth) + ns_length_get(auth)) -2 ||
		!check_smtp_client_read_end(client) || client_status(client) != 1 ||
		st_cmp_cs_starts(&(client->line), NULLER("235"))) {

		st_sprint(errmsg, "Failed to return a successful status after submitting credentials");
		return false;
	}

	return true;
}

/**
 * @brief 	Submits the AUTH LOGIN command to the passed client using the passed parameters
 * @param 	client A client_t* connected to an SMTP server that has had the HELO/EHLO command
 * 			already submitted
 * @param 	user A chr_t* containing the username of the user
 * @param 	pass A chr_t* containing the password of the user
 * @param 	errmsg A stringer_t* that will have the error message printed to it in the event of
 * 			and error
 * @return 	True if no errors, false otherwise
 */
bool_t check_smtp_client_auth_login(client_t *client, chr_t *user, chr_t *pass, stringer_t *errmsg) {

	if (client_print(client, "AUTH LOGIN\r\n") != 12 || !check_smtp_client_read_end(client) ||
		client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("334"))) {

		st_sprint(errmsg, "Failed to return a proceed status code after AUTH LOGIN.");
		return false;
	}
	else if (client_print(client, "%s\r\n", user) != ns_length_get(user) + 2 || !check_smtp_client_read_end(client) ||
		client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("334"))) {

		st_sprint(errmsg, "Failed to return a proceed status code after submitting username.");
		return false;
	}
	else if (client_print(client, "%s\r\n", pass) != ns_length_get(pass) + 2 || !check_smtp_client_read_end(client) ||
		client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("235"))) {

		st_sprint(errmsg, "Failed to return a successful status after submitting credentials.");
		return false;
	}

	return true;
}

/**
 * @brief 	Submits the QUIT command to the passed client and checks for errors
 * @param 	client The client_t* to submit the command to, it should be connected
 * 			to an smtp server
 * @param 	errmsg A stringer_t* that will have the error message printed to it in
 * 			the event of an error
 * @return 	True if no errors, false otherwise
 */
bool_t check_smtp_client_quit(client_t *client, stringer_t *errmsg) {

	// Test the QUIT command.
	if (client_print(client, "QUIT\r\n") != 6 || client_read_line(client) <= 0 ||
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

	return true;
}

bool_t check_smtp_network_basic_sthread(stringer_t *errmsg, uint32_t port, bool_t secure) {

	size_t location = 0;
	client_t *client = NULL;

	// Test the connect banner.
	if (!(client = client_connect("localhost", port)) || (secure && (client_secure(client) == -1)) ||
		!net_set_timeout(client->sockd, 20, 20) || client_read_line(client) <= 0 || client_status(client) != 1 ||
		st_cmp_cs_starts(&(client->line), NULLER("220")) || !st_search_cs(&(client->line), NULLER(" ESMTP "), &location)) {

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
	else if (client_print(client, "EHLO localhost\r\n") != 16 || !check_smtp_client_read_end(client) ||
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

bool_t check_smtp_network_auth_sthread(stringer_t *errmsg, uint32_t port, bool_t login) {

	size_t location = 0;
	client_t *client = NULL;

	// Connect the client.
	if (!(client = client_connect("localhost", port)) || client_read_line(client) <= 0 ||
		client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("220")) ||
		!st_search_cs(&(client->line), NULLER(" ESMTP "), &location)) {

		st_sprint(errmsg, "Failed to connect with the SMTP server.");
		client_close(client);
		return false;
	}
	// Issue EHLO.
	else if (client_print(client, "EHLO localhost\r\n") != 16 || !check_smtp_client_read_end(client) ||
		client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("250"))) {

		st_sprint(errmsg, "Failed to return successful status after EHLO.");
		client_close(client);
		return false;
	}
	// Issue AUTH with incorrect credentials.
	else if ((login ? check_smtp_client_auth_login(client, "bWFnbWE=", "aW52YWxpZHBhc3N3b3Jk", errmsg)
			: check_smtp_client_auth_plain(client, "bWFnbWEAbWFnbWEAaW52YWxpZHBhc3N3b3Jk", errmsg))) {

		client_close(client);
		return false;
	}
	// Issue AUTH with correct credentials.
	else if (!(login ? check_smtp_client_auth_login(client, "bWFnbWE=", "cGFzc3dvcmQ=", errmsg)
			: check_smtp_client_auth_plain(client, "bWFnbWEAbWFnbWEAcGFzc3dvcmQ=", errmsg))) {

		client_close(client);
		return false;
	}
	// Try sending mail from an unauthenticated account (ladar@lavabit.com).
	else if ((errmsg = NULL) || !check_smtp_client_mail_rcpt_data(client, "ladar@lavabit.com", "princess@example.com", errmsg) ||
		client_print(client, ".\r\n") != 3 || !check_smtp_client_read_end(client) || client_status(client) != 1 ||
		st_cmp_cs_starts(&(client->line), NULLER("550"))) {

		if (!errmsg) st_sprint(errmsg, "Failed to return an error status after sending from an unauthenticated account.");
		client_close(client);
		return false;
	}
	// Try sending mail from the authenticated account (magma@lavabit.com).
	else if (!check_smtp_client_mail_rcpt_data(client, "magma@lavabit.com", "princess@example.com", errmsg) ||
		client_print(client, ".\r\n") != 3 || !check_smtp_client_read_end(client) || client_status(client) != 1 ||
		st_cmp_cs_starts(&(client->line), NULLER("250"))) {

		if (!errmsg) st_sprint(errmsg, "Failed to return successful status after sending from an authenticated account.");
		client_close(client);
		return false;
	}
	// Submit QUIT and cleanup.
	else if (!check_smtp_client_quit(client, errmsg)) {
		return false;
	}

	return true;
}
