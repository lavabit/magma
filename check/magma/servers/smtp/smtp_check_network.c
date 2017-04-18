
/**
 * @file /magma/check/magma/servers/smtp/smtp_check_helpers.c
 *
 * @brief Functions used to test SMTP connections over a network connection.
 *
 */

#include "magma_check.h"

/**
 * @brief 	Calls client_read_line on a client until the last line is found
 * @param 	client 	The client_t* to read from (which should be connected to an SMTP server).
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
 * @param 	client 	The client to write the commands to. It should be connected to an SMTP server and have
 * 					had HELO/EHLO issued previously.
 * @baram 	from 	A chr_t* holding the value to be inserted in the MAIL FROM command
 * @param 	to 		A chr_t* holding the value to be inserted in the RCPT TO command
 * @param 	errmsg 	A stringer_t* that will have the error message printed to it in the event of an error
 * @return 	True if no errors, false otherwise
 */
bool_t check_smtp_client_mail_rcpt_data(client_t *client, chr_t *from, chr_t *to, stringer_t *errmsg) {

	chr_t *line_from = "MAIL FROM: <%s>\r\n", *line_to = "RCPT TO: <%s>\r\n";
	size_t size_from = ns_length_get(line_from) + ns_length_get(from) - 2, size_to = ns_length_get(line_to) + ns_length_get(to) - 2;

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
	else if (client_write(client, PLACER("DATA\r\n", 6)) != 6 || !check_smtp_client_read_end(client) ||
		client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("354"))) {
		st_sprint(errmsg, "Failed to return a proceed status code after DATA.");
		return false;
	}

	return true;
}

/**
 * @brief 	Submits the AUTH PLAIN command to the passed client using the passed parameters
 * @param 	client 	A client_t* connected to an SMTP server that has had the HELO/EHLO command
 * 					already submitted
 * @param 	auth 	The user authentication string
 * @param 	errmsg 	A stringer_t* that will have the error message printed to it in the event of
 * 					and error
 * @return 	True if no errors, false otherwise
 */
bool_t check_smtp_client_auth_plain(client_t *client, stringer_t *auth) {

	if (client_print(client, "AUTH PLAIN %.*s\r\n", st_length_int(auth), st_char_get(auth)) != st_length_get(auth) + 13 ||
		!check_smtp_client_read_end(client) || client_status(client) != 1 ||
		st_cmp_cs_starts(&(client->line), NULLER("235"))) {
		return false;
	}

	return true;
}

/**
 * @brief 	Submits the AUTH LOGIN command to the passed client using the passed parameters
 * @param 	client 	A client_t* connected to an SMTP server that has had the HELO/EHLO command
 * 					already submitted
 * @param 	user 	A NULL string containing the username of the user.
 * @param 	pass 	A NULL string containing the password of the user.
 * @return 	true if no errors, false otherwise
 */
bool_t check_smtp_client_auth_login(client_t *client, stringer_t *user, stringer_t *pass) {

	if (client_write(client, PLACER("AUTH LOGIN\r\n", 12)) != 12 || !check_smtp_client_read_end(client) ||
		client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("334"))) {
		return false;
	}
	else if (client_print(client, "%.*s\r\n", st_length_int(user), st_char_get(user)) != st_length_get(user) + 2 ||
		!check_smtp_client_read_end(client) || client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("334"))) {
		return false;
	}
	else if (client_print(client, "%.*s\r\n", st_length_int(pass), st_char_get(pass)) != st_length_get(pass) + 2 ||
		!check_smtp_client_read_end(client) || client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("235"))) {
		return false;
	}

	return true;
}

/**
 * @brief 	Submits the QUIT command to the passed client and checks for errors
 * @param 	client 	The client_t* to submit the command to, it should be connected
 * 					to an smtp server
 * @param 	errmsg 	A stringer_t* that will have the error message printed to it in
 * 					the event of an error
 * @return 	True if no errors, false otherwise
 */
bool_t check_smtp_client_quit(client_t *client, stringer_t *errmsg) {

	// Test the QUIT command.
	if (client_write(client, PLACER("QUIT\r\n", 6)) != 6 || client_read_line(client) <= 0 ||
		client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("221"))) {

		st_sprint(errmsg, "Failed to return successful status following the QUIT command.");
		return false;
	}
	else if (client_read_line(client) > 0) {

		st_sprint(errmsg, "The server failed to close the connection after issuing a QUIT command.");
		return false;
	}

	return true;
}

bool_t check_smtp_network_basic_sthread(stringer_t *errmsg, uint32_t port, bool_t secure) {

	size_t location = 0;
	client_t *client = NULL;
	chr_t *message = "To: magma@lavabit.com\r\nFrom: princess@example.com\r\nSubject: Unit Tests\r\n\r\n"\
		"Aren't unit tests great?\r\n.\r\n";

	// Test the connect banner.
	if (!(client = client_connect("localhost", port)) || (secure && (client_secure(client) == -1)) ||
		!net_set_timeout(client->sockd, 20, 20) || client_read_line(client) <= 0 || client_status(client) != 1 ||
		st_cmp_cs_starts(&(client->line), NULLER("220")) || !st_search_cs(&(client->line), NULLER(" ESMTP "), &location)) {

		st_sprint(errmsg, "Failed to connect with the SMTP server.");
		client_close(client);
		return false;
	}

	// Test the HELO command.
	else if (client_write(client, PLACER("HELO localhost\r\n", 16)) != 16 || client_read_line(client) <= 0 ||
		client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("250"))) {

		st_sprint(errmsg, "Failed to return successful status after HELO.");
		client_close(client);
		return false;
	}

	// Test the EHLO command.
	else if (client_write(client, PLACER("EHLO localhost\r\n", 16)) != 16 || !check_smtp_client_read_end(client) ||
		client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("250"))) {

		st_sprint(errmsg, "Failed to return successful status after EHLO.");
		client_close(client);
		return false;
	}

	// Test the MAIL command.
	else if (client_write(client, PLACER("MAIL FROM: <>\r\n", 15)) != 15 || client_read_line(client) <= 0 ||
		client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("250"))) {

		st_sprint(errmsg, "Failed to return successful status after MAIL.");
		client_close(client);
		return false;
	}

	// Test the RCPT command.
	else if (client_write(client, PLACER("RCPT TO: <princess@example.com>\r\n", 33)) != 33 || client_read_line(client) <= 0 ||
		client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("250"))) {

		st_sprint(errmsg, "Failed to return successful status after RCPT.");
		client_close(client);
		return false;
	}

	// Test the DATA command.
	else if (client_write(client, PLACER("DATA\r\n", 6)) != 6 || client_read_line(client) <= 0 ||
		client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("354"))) {

		st_sprint(errmsg, "Failed to return a proceed status code after DATA.");
		client_close(client);
		return false;
	}

	// Test sending the contents of an email.
	else if (client_write(client, PLACER(message, ns_length_get(message))) != ns_length_get(message) ||
		client_read_line(client) <= 0 || client_status(client) != 1 ||
		st_cmp_cs_starts(&(client->line), NULLER("250"))) {

		st_sprint(errmsg, "Failed to get a successful status code after email submission.");
		client_close(client);
		return false;
	}
	// Submit QUIT and cleanup.
	else if (!check_smtp_client_quit(client, errmsg)) {

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
	else if (client_write(client, PLACER("EHLO localhost\r\n", 16)) != 16 || !check_smtp_client_read_end(client) ||
		client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("250"))) {

		st_sprint(errmsg, "Failed to return successful status after EHLO.");
		client_close(client);
		return false;
	}
	// Issue AUTH with incorrect credentials.
	else if ((login ? check_smtp_client_auth_login(client,  NULLER("bWFnbWE="),  NULLER("aW52YWxpZHBhc3N3b3Jk"))
			: check_smtp_client_auth_plain(client,  NULLER("bWFnbWEAbWFnbWEAaW52YWxpZHBhc3N3b3Jk")))) {
		st_sprint(errmsg, "Invalid credentials appear to have authenticated when they should have failed.");
		client_close(client);
		return false;
	}
	// Issue AUTH with correct credentials.
	else if (!(login ? check_smtp_client_auth_login(client, NULLER("bWFnbWE="),  NULLER("cGFzc3dvcmQ="))
			: check_smtp_client_auth_plain(client,  NULLER("bWFnbWEAbWFnbWEAcGFzc3dvcmQ=")))) {
		st_sprint(errmsg, "Failed to authenticate even though we supplied valid credentials.");
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

		if (st_empty(errmsg)) st_sprint(errmsg, "Failed to return successful status after sending from an authenticated account.");
		client_close(client);
		return false;
	}
	// Submit QUIT and cleanup.
	else if (!check_smtp_client_quit(client, errmsg)) {

		client_close(client);
		return false;
	}

	client_close(client);
	return true;
}

bool_t check_smtp_network_outbound_quota_sthread(stringer_t *errmsg, uint32_t port, bool_t secure) {

	client_t *client = NULL;
	log_enable();

	// Wipe the Transmitting table history for the Magma user.
	if (sql_query(PLACER("DELETE FROM Transmitting WHERE usernum = 1;", 45)) != 0) {

		st_sprint(errmsg, "The SQL query to clear the Magma user's transmission history failed.");
		return false;
	}
	// Set the daily send limit to 1 for the Magma user.
	else if (sql_query(PLACER("UPDATE Dispatch SET daily_send_limit = 1 WHERE usernum = 1;", 59)) != 0) {

		st_sprint(errmsg, "The SQL query to set the daily send limit low for the Magma user failed.");
		return false;
	}
	// Connect to the SMTP server and authenticate.
	else if (!(client = client_connect("localhost", port)) || !net_set_timeout(client->sockd, 20, 20) ||
		(secure && (client_secure(client) == -1)) || client_read_line(client) <= 0 ||
		!check_smtp_client_auth_login(client, PLACER("bWFnbWE=", 8),  PLACER("cGFzc3dvcmQ=", 12))) {

		sql_query(PLACER("UPDATE Dispatch SET daily_send_limit = 256 WHERE usernum = 1;", 61));
		st_sprint(errmsg, "Failed to connect with the SMTP server or AUTH LOGIN failed.");
		client_close(client);
		return false;
	}
	// Send the first message and expect a reply of "250 MESSAGE ACCEPTED".
	else if (!check_smtp_client_mail_rcpt_data(client, "magma@lavabit.com", "ladar@lavabit.com", errmsg) ||
		client_write(client, PLACER("This is a message body.\r\n.\r\n", 28)) != 28 ||
		!check_smtp_client_read_end(client) || st_cmp_cs_starts(&(client->line), PLACER("250", 3)) != 0) {

		sql_query(PLACER("UPDATE Dispatch SET daily_send_limit = 256 WHERE usernum = 1;", 61));
		if (st_empty(errmsg)) st_sprint(errmsg, "Failed sending the first message.");
		client_close(client);
		return false;
	}
	// Send the second message and expect a reply of "451 OUTBOUND MAIL QUOTA EXCEEDED".
	else if (!check_smtp_client_mail_rcpt_data(client, "magma@lavabit.com", "ladar@lavabit.com", errmsg) ||
		client_write(client, PLACER("This is a message body.\r\n.\r\n", 28)) != 28 ||
		!check_smtp_client_read_end(client) || st_cmp_cs_starts(&(client->line), PLACER("451", 3)) != 0) {

		sql_query(PLACER("UPDATE Dispatch SET daily_send_limit = 256 WHERE usernum = 1;", 61));
		if (st_empty(errmsg)) st_sprint(errmsg, "Failed to be limited by quota.");
		client_close(client);
		return false;
	}
	// Reset the daily send limit for the Magma user.
	else if (sql_query(PLACER("UPDATE Dispatch SET daily_send_limit = 256 WHERE usernum = 1;", 61)) != 0) {

		st_sprint(errmsg, "The SQL query to reset the daily send limit for the Magma user failed.");
		client_close(client);
		return false;
	}

	client_close(client);
	return true;
}

bool_t check_smtp_network_starttls_advertisement_sthread(stringer_t *errmsg, uint32_t tcp_port, uint32_t tls_port) {

	client_t *client = NULL;
	bool_t found_starttls_ad = false;

	// Connect the client over TCP.
	if (!(client = client_connect("localhost", tcp_port)) || client_read_line(client) <= 0 ||
		!net_set_timeout(client->sockd, 20, 20)) {

		st_sprint(errmsg, "Failed to connect with the SMTP server over TCP.");
		client_close(client);
		return false;
	}
	// Issue EHLO.
	else if (client_write(client, PLACER("EHLO localhost\r\n", 16)) != 16) {

		st_sprint(errmsg, "Failed to return successful status after TCP EHLO.");
		client_close(client);
		return false;
	}
	// Check for "250-STARTTLS" in the EHLO response over an insecure connection.
	else {
		while (client_read_line(client) > 0 && pl_char_get(client->line)[3] != ' ') {
			if (st_cmp_cs_starts(&(client->line), PLACER("250-STARTTLS", 12))) found_starttls_ad = true;
		}
		if (!found_starttls_ad) {

			st_sprint(errmsg, "Failed to find STARTTLS advertised in TCP EHLO response.");
			client_close(client);
			return false;
		}
	}

	found_starttls_ad = false;
	client_close(client);
	client = NULL;

	// Connect the client over TLS.
	if (!(client = client_connect("localhost", tls_port)) || client_secure(client) != 0) {

		st_sprint(errmsg, "Failed to connect with the SMTP server over TLS.");
		client_close(client);
		return false;
	}
	// Issue EHLO.
	else if (client_write(client, PLACER("EHLO localhost\r\n", 16)) != 16) {

		st_sprint(errmsg, "Failed to return successful status after TLS EHLO.");
		client_close(client);
		return false;
	}
	// Check for "250-STARTTLS" in the EHLO response over an insecure connection.
	else {
		while (client_read_line(client) > 0 && pl_char_get(client->line)[3] != ' ') {
			if (st_cmp_cs_starts(&(client->line), PLACER("250-STARTTLS", 12))) found_starttls_ad = true;
		}
		if (found_starttls_ad) {
			st_sprint(errmsg, "Found an unnecessary STARTTLS advertisement in TLS connection EHLO response.");
			client_close(client);
			return false;
		}
	}

	client_close(client);
	return true;
}
