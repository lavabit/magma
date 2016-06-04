
/**
 * @file /magma/servers/smtp/smtp.c
 *
 * @brief	Functions used to handle SMTP commands/actions.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/// TODO: Review error messages and update them with the appropriate response code.

/**
 * @brief	Initialize a TLS session for an unauthenticated SMTP session.
 * @param	con		the connection of the SMTP endpoint requesting the transport layer security upgrade.
 * @return	This function returns no value.
 */
void smtp_starttls(connection_t *con) {

	// Check for an existing SSL connection.
	if (con_secure(con) == 1) {
		con_write_bl(con, "454 Session is already encrypted.\r\n", 35);
		return;
	}
	// Check whether we support the STARTTLS command.
	else if (con_secure(con) == -1) {
		con_write_bl(con, "554 This server has not been configured to support STARTTLS.\r\n", 62);
		return;
	}

	con_write_bl(con, "220 READY\r\n", 11);

	if (!(con->network.ssl = ssl_alloc(con->server, con->network.sockd, M_SSL_BIO_NOCLOSE))) {
		con_write_bl(con, "454 STARTTLS FAILED\r\n", 21);
		log_pedantic("The SSL connection attempt failed.");
		return;
	}

	stats_increment_by_name("smtp.connections.secure");
	st_length_set(con->network.buffer, 0);
	con->network.line = pl_null();
	con->network.status = 1;
	smtp_session_reset(con);

	return;
}

/**
 * @brief	Specify the identity of a message's sender, in response to an SMTP MAIL FROM command.
 * @see		smtp_parse_mail_from_path()
 * @note	This command must be preceded by a HELO command and successful authentication.
 * 			Any prior email address specified by a MAIL FROM command will be overwritten.
 * 			If the SIZE parameter was specified with the MAIL FROM command, its value will be compared to the maximum value specified in
 *	 			the smtp.message_length_limit configuration option.
 * @param	con		the SMTP client connection issuing the command.
 * @return	This function returns no value.
 */
void smtp_mail_from(connection_t *con) {

	credential_t *cred;
	// If they try to send this command without saying hello.
	if (!con->smtp.helo && !con->smtp.authenticated) {
		con_write_bl(con, "503 MAIL FROM REJECTED - PLEASE PROVIDE A HELO OR EHLO AND TRY AGAIN\r\n", 70);
		return;
	}

	// If they try to send MAIL FROM twice, trigger a session reset.
	if (con->smtp.mailfrom)	{
		smtp_session_reset(con);
	}

	// Attempt to pull the path off the line.
	if (!(con->smtp.mailfrom = smtp_parse_mail_from_path(con))) {
		con_write_bl(con, "553 MAIL FROM ERROR - INVALID SENDER SYNTAX\r\n", 45);
		return;
	}

	// Make sure the suggested size isn't over the system wide limit.
	if (con->smtp.suggested_length > magma.smtp.message_length_limit) {
		st_free(con->smtp.mailfrom);
		con->smtp.mailfrom = NULL;
		con->smtp.suggested_length = 0;
		con_write_bl(con, "552 MAIL FROM ERROR - SIZE EXCEEDS SYSTEM LIMIT\r\n", 49);
		return;
	}

	if (!(cred = credential_alloc_mail(con->smtp.mailfrom))) {
		st_free(con->smtp.mailfrom);
		con->smtp.mailfrom = NULL;
		con->smtp.suggested_length = 0;
		con_write_bl(con, "451 INTERNAL SERVER ERROR - PLEASE TRY AGAIN LATER\r\n", 52);
		return;
	}

	// Require authentication if user is not authenticated and MAIL FROM domain is hosted on this server
	if(!con->smtp.authenticated && domain_mailboxes(cred->auth.domain) >= 0) {
		credential_free(cred);
		st_free(con->smtp.mailfrom);
		con->smtp.mailfrom = NULL;
		con->smtp.suggested_length = 0;
		con_write_bl(con, "530 AUTHENTICATION REQUIRED\r\n", 29);
		return;
	}

	credential_free(cred);

	// Spit back the all clear.
	con_write_bl(con, "250 MAIL FROM COMPLETE\r\n", 24);

	return;
}

/**
 * @brief	Process an SMTP EHLO command.
 * @see		smtp_parse_helo_domain()
 * @note	Any prior domain specified by a HELO/EHLO command will be overwritten.
 * @param	con		the SMTP client connection issuing the command.
 * @return	This function returns no value.
 */
void smtp_ehlo(connection_t *con) {

	stringer_t *helo;

	if (!(helo = smtp_parse_helo_domain(con))) {
		con_write_bl(con, "501 EHLO SYNTAX ERROR - MISSING REQUIRED DOMAIN PARAMETER\r\n", 59);
		return;
	}

	// Free any previously provided values.
	st_cleanup(con->smtp.helo);
	con->smtp.helo = helo;
	con->smtp.esmtp = true;

	// If the user is connected via SSL already, or there is no SSL context, omit the STARTTLS parameter.
	con_print(con, "250-%.*s\r\n250-8BITMIME\r\n%s250-PIPELINING\r\n250-SIZE %lu\r\n250-AUTH LOGIN PLAIN\r\n250-AUTH=LOGIN PLAIN\r\n250 EHLO COMPLETE\r\n",
		st_length_int(con->server->domain), st_char_get(con->server->domain), (con_secure(con) != 0 ? "" : "250-STARTTLS\r\n"),
		magma.smtp.message_length_limit);

	return;
}

/**
 * @brief	Process an SMTP HELO command.
 * @see		smtp_parse_helo_domain()
 * @note	Any prior domain specified by a HELO/EHLO command will be overwritten.
 * @param	con		the SMTP client connection issuing the command.
 * @return	This function returns no value.
 */
void smtp_helo(connection_t *con) {

	stringer_t *helo;

	if (!(helo = smtp_parse_helo_domain(con))) {
		con_write_bl(con, "501 HELO SYNTAX ERROR - MISSING REQUIRED DOMAIN PARAMETER\r\n", 59);
		return;
	}

	// Free any previously provided values.
	st_cleanup(con->smtp.helo);
	con->smtp.helo = helo;
	con->smtp.esmtp = false;

	// Spit back the standard SMTP greeting..

	con_print(con, "250 %.*s\r\n", st_length_int(con->server->domain), st_char_get(con->server->domain));
	return;
}

/**
 * @brief	Perform an SMTP NOOP (no-operation) command.
 * @note	This command does essentially nothing and is mostly a way to keep connections alive without timing out due to inactivity.
 * @return	This function returns no value.
 */
void smtp_noop(connection_t *con) {

	con_write_bl(con, "250 NOOP COMPLETE\r\n", 19);
	return;
}

/**
 * @brief	A stub function for an SMTP command that has not been implemented.
 * @note	Executing a disabled command while result in a small delay and the protocol violation counter being incremented.
 * @return	This function returns no value.
 */
void smtp_disabled(connection_t *con) {

	con->protocol.violations++;
	usleep(con->server->violations.delay);
	con_print(con, "502 %.*s DISABLED\r\n", (int)con->command->length, con->command->string);

	return;
}

/**
 * @brief	A function that is executed when an invalid SMTP command is executed.
 * @return	This function returns no value.
 */
void smtp_invalid(connection_t *con) {

	con->protocol.violations++;
	usleep(con->server->violations.delay);
	con_write_bl(con, "500 INVALID COMMAND\r\n", 21);

	return;
}

/**
 * @brief	Gracefully terminate an SMTP session, especially in response to an SMTP QUIT command.
 * @note	The standards specify that the receiver MUST send an OK reply, and then close the transmission channel.
 * @param	the SMTP client connection to be terminated.
 * @return	This function returns no value.
 */
void smtp_quit(connection_t *con) {

	if (con_status(con) == 2) {
		con_write_bl(con, "451 Unexpected connection shutdown detected. Goodbye.\r\n", 55);
	}
	else if (con_status(con) >= 0) {
		con_write_bl(con, "221 BYE\r\n", 9);
	}
	else {
		con_write_bl(con, "421 Network connection failure.\r\n", 33);
	}

	con_destroy(con);

	return;
}

/**
 * @brief	Reset the SMTP session, in response to an SMTP RSET command.
 * @note	This command clears any sender, recipient, and mail data, along with all buffers and state tables.
 * 			The connection structure is reset to the same it was in immediately after the HELO/EHLO command.
 * @param	con		the SMTP client connection issuing the command.
 * @return	This function returns no value.
 */
void smtp_rset(connection_t *con) {

	smtp_session_reset(con);
	con_write_bl(con, "250 RSET COMPLETE\r\n", 19);

	return;
}


/// BUG: The SMTP server is not handling AUTH requests in accordance with
///		RFC 2554 <http://tools.ietf.org/html/rfc2554> and RFC 4954
///		<http://tools.ietf.org/html/rfc4954>, according to a user.
///
///		In this particular case it's erroring out when users present an AUTH
///		PLAIN request without the optional initial response. This could affect other
///		protocols as well.
///
///		According to the RFC, a client can send either the following:
///		1. "AUTH mechanism" (e.g. "AUTH PLAIN"), which the server acknowledges
///		with a "334 " (note the space). The client then sends the base64-encoded
///		authentication response on a separate line.
///
///		2. "AUTH mechanism [initial response]" (e.g. "AUTH PLAIN
///		dGVzdAB0ZXN0ADEyMzQ="), where user transmits the auth mechanism and the
///		base64 authentication response in a single line, so as to minimize the
///		back-and-forth traffic.
void smtp_auth_plain(connection_t *con) {

	credential_t *cred;
	int_t state, cred_res;
	salt_state_t salt_res;
	smtp_outbound_prefs_t *outbound;
	stringer_t *decoded = NULL, *argument = NULL, *salt = NULL;
	placer_t username = { .opts = PLACER_T | JOINTED | STACK | FOREIGNDATA}, password = { .opts = PLACER_T | JOINTED | STACK | FOREIGNDATA},
		authorize_id = { .opts = PLACER_T | JOINTED | STACK | FOREIGNDATA };

	// If the user is already authenticated.
	if (con->smtp.authenticated) {
		con_write_bl(con, "503 ALREADY AUTHENTICATED\r\n", 27);
		return;
	}

	// If AUTH PLAIN is sent without any parameters write the continue code and wait for a line of input data.
	if (!st_cmp_ci_eq(&(con->network.line), PLACER("AUTH PLAIN\r\n", 12)) || !st_cmp_ci_eq(&(con->network.line), PLACER("AUTH PLAIN\n", 11))) {
		if (con_write_bl(con, "334 \r\n", 6) != -1 && con_read_line(con, true) >= 0) {
			argument = smtp_parse_auth(&(con->network.line));
		}

	}

	// Otherwise the authentication data was passed along with the AUTH PLAIN commands, so we simply setup a placer to point_t at it.
	else if (pl_length_get(con->network.line) > 10) {
		argument = smtp_parse_auth(PLACER(pl_char_get(con->network.line) + 10, pl_length_get(con->network.line) - 10));
	}

	// Validate that an argument was extracted using the above logic.
	if (!argument) {
		con_write_bl(con, "501 INVALID AUTH SYNTAX\r\n", 25);
		return;
	}

	// Decode the base64 string into its various components.
	decoded = base64_decode(argument, NULL);
	st_free(argument);

	// Make sure we were able to decode something.
	if (!decoded || !st_length_get(decoded)) {
		st_cleanup(decoded);
		con_write_bl(con, "501 INVALID AUTH SYNTAX\r\n", 25);
		return;
	}

	// Fetch the different components.
	if (tok_get_st(decoded, '\0', 0, &authorize_id) || tok_get_st(decoded, '\0', 1, &username) || tok_get_st(decoded, '\0', 2, &password) != 1) {
		st_free(decoded);
		con_write_bl(con, "501 INVALID AUTH SYNTAX\r\n", 25);
		return;
	}

	// If for some reason the username field is NULL, but the authorize-id isn't, try that instead. We really don't need this.
	if (st_empty(&username) && !st_empty(&authorize_id)) {
		mm_copy(&username, &authorize_id, sizeof(placer_t));
	}

	// Error check.
	if (st_empty(&username) || st_empty(&password)) {
		st_free(decoded);
		con_write_bl(con, "501 INVALID AUTH SYNTAX\r\n", 25);
		return;
	}

	/// BUG: The code should be able to differentiate between invalid usernames which trigger a NULL return and allocation (or similar)
	/// errors which are temporary. We approximate this functionality by not rejecting "@domain.com" as a username via the credentials
	/// function, but instead check for leading at symbols explicitly below.
	// Create the credential context.

	cred = credential_alloc_auth(&username);
	st_free(decoded);

	if(!cred) {
		con_write_bl(con, "423 INTERNAL SERVER ERROR - PLEASE TRY AGAIN LATER\r\n", 52);
		return;
	}

	salt_res = credential_salt_fetch(cred->auth.username, &salt);

	if(salt_res == USER_SALT) {
		cred_res = credential_calc_auth(cred, &password, salt);
		st_free(salt);
	}
	else if(salt_res == USER_NO_SALT) {
		cred_res = credential_calc_auth(cred, &password, NULL);
	}
	else {
		cred_res = 0;
	}

	if(!cred_res) {
		credential_free(cred);
		con_write_bl(con, "423 INTERNAL SERVER ERROR - PLEASE TRY AGAIN LATER\r\n", 52);
		return;
	}

	// Check to make sure the username doesn't start with an at symbol.
	if (!st_cmp_cs_starts(cred->auth.username, PLACER("@", 1))) {
		credential_free(cred);
		con_write_bl(con, "535 AUTHENTICATION FAILURE - INVALID USERNAME AND PASSWORD COMBINATION\r\n", 72);
		return;
	}

	// Authorize the user, and securely delete the keys.
	state = smtp_fetch_authorization(cred, &outbound);
	credential_free(cred);

	// Tell the user what happened.
	if (state == 1) {
		smtp_add_outbound(con, outbound);
		smtp_session_reset(con);
		con->smtp.max_length = outbound->send_size_limit;
		con->smtp.authenticated = true;
		con_write_bl(con, "235 AUTH PLAIN SUCCESSFUL - AUTHENTICATED\r\n", 43);
	}
	else if (state == 0) {
		con_write_bl(con, "535 AUTHENTICATION FAILURE - INVALID USERNAME AND PASSWORD COMBINATION\r\n", 72);
	}
	else if (state == -2) {
		con_write_bl(con, "535 AUTHENTICATION FAILURE - THIS ACCOUNT HAS BEEN ADMINISTRATIVELY LOCKED\r\n", 76);
	}
	else if (state == -3) {
		con_write_bl(con, "535 AUTHENTICATION FAILURE - THIS ACCOUNT HAS BEEN LOCKED ON SUSPICION OF ABUSE POLICY VIOLATIONS\r\n", 99);
	}
	else if (state == -4) {
		con_write_bl(con, "535 AUTHENTICATION FAILURE - THIS ACCOUNT HAS BEEN LOCKED AT THE REQUEST OF THE USER\r\n", 86);
	}
	else {
		con_write_bl(con, "423 INTERNAL SERVER ERROR - PLEASE TRY AGAIN LATER\r\n", 52);
	}

	return;
}

void smtp_auth_login(connection_t *con) {

	credential_t *cred;
	int_t state, cred_res;
	salt_state_t salt_res;
	smtp_outbound_prefs_t *outbound;
	stringer_t *username = NULL, *password = NULL, *argument = NULL, *salt = NULL;

	// If the user is already authenticated.
	if (con->smtp.authenticated == true) {
		con_write_bl(con, "503 ALREADY AUTHENTICATED\r\n", 27);
		return;
	}

	// If AUTH LOGIN is sent without any parameters spit out 'Username:' in base64.
	if (!st_cmp_ci_eq(&(con->network.line), PLACER("AUTH LOGIN\r\n", 12)) || !st_cmp_ci_eq(&(con->network.line), PLACER("AUTH LOGIN\n", 11))) {
		if (con_write_bl(con, "334 VXNlcm5hbWU6\r\n", 18) != -1 && con_read_line(con, true) >= 0) {
			argument = smtp_parse_auth(&(con->network.line));
		}
	}

	// Otherwise the authentication data was passed along with the AUTH PLAIN commands, so we simply setup a placer to point_t at it.
	else if (pl_length_get(con->network.line) > 10) {
		argument = smtp_parse_auth(PLACER(pl_char_get(con->network.line) + 10, pl_length_get(con->network.line) - 10));
	}

	// Validate that an argument was extracted using the above logic.
	if (!argument || !(username =	base64_decode(argument, NULL)) || st_empty(username)) {
		st_cleanup(argument);
		st_cleanup(username);
		con_write_bl(con, "501 INVALID AUTH SYNTAX\r\n", 25);
		return;
	}

	st_free(argument);
	argument = NULL;

	// Now spit out 'Password:' in base64.
	if (con_write_bl(con, "334 UGFzc3dvcmQ6\r\n", 18) != -1 && con_read_line(con, true) >= 0) {
		argument = smtp_parse_auth(&(con->network.line));
	}

	// Validate that an argument was extracted using the above logic.
	if (!argument || !(password = base64_decode(argument, NULL)) || st_empty(password)) {
		st_cleanup(argument);
		st_cleanup(password);
		st_free(username);
		con_write_bl(con, "501 INVALID AUTH SYNTAX\r\n", 25);
		return;
	}

	st_free(argument);
	argument = NULL;

	/// BUG: The code should be able to differentiate between invalid usernames which trigger a NULL return and allocation (or similar)
	/// errors which are temporary. We approximate this functionality by not rejecting "@domain.com" as a username via the credentials
	/// function, but instead check for leading at symbols explicitly below.
	// Create the credential context.
	if(!(cred = credential_alloc_auth(username))) {
		con_write_bl(con, "423 INTERNAL SERVER ERROR - PLEASE TRY AGAIN LATER\r\n", 52);
		return;
	}

	salt_res = credential_salt_fetch(cred->auth.username, &salt);

	if(salt_res == USER_SALT) {
		cred_res = credential_calc_auth(cred, password, salt);
		st_free(salt);
	}
	else if(salt_res == USER_NO_SALT) {
		cred_res = credential_calc_auth(cred, password, NULL);
	}
	else {
		cred_res = 0;
	}

	st_free(username);
	st_free(password);

	if(!cred_res) {
		credential_free(cred);
		con_write_bl(con, "423 INTERNAL SERVER ERROR - PLEASE TRY AGAIN LATER\r\n", 52);
		return;
	}

	// Check to make sure the username doesn't start with an at symbol.
	if (!st_cmp_cs_starts(cred->auth.username, PLACER("@", 1))) {
		credential_free(cred);
		con_write_bl(con, "535 AUTHENTICATION FAILURE - INVALID USERNAME AND PASSWORD COMBINATION\r\n", 72);
		return;
	}

	// Authorize the user, and securely delete the keys.
	state = smtp_fetch_authorization(cred, &outbound);
	credential_free(cred);

	// Tell the user what happened.
	if (state == 1) {
		smtp_add_outbound(con, outbound);
		smtp_session_reset(con);
		con->smtp.max_length = outbound->send_size_limit;
		con->smtp.authenticated = true;
		con_write_bl(con, "235 AUTH LOGIN SUCCESSFUL - AUTHENTICATED\r\n", 43);
	}
	else if (state == 0) {
		con_write_bl(con, "535 AUTHENTICATION FAILURE - INVALID USERNAME AND PASSWORD COMBINATION\r\n", 72);
	}
	else if (state == -2) {
		con_write_bl(con, "535 AUTHENTICATION FAILURE - THIS ACCOUNT HAS BEEN ADMINISTRATIVELY LOCKED\r\n", 76);
	}
	else if (state == -3) {
		con_write_bl(con, "535 AUTHENTICATION FAILURE - THIS ACCOUNT HAS BEEN LOCKED ON SUSPICION OF ABUSE POLICY VIOLATIONS\r\n", 99);
	}
	else if (state == -4) {
		con_write_bl(con, "535 AUTHENTICATION FAILURE - THIS ACCOUNT HAS BEEN LOCKED AT THE REQUEST OF THE USER\r\n", 86);
	}
	else {
		con_write_bl(con, "423 INTERNAL SERVER ERROR - PLEASE TRY AGAIN LATER\r\n", 52);
	}

	return;
}

void smtp_rcpt_to(connection_t *con) {

	int_t state;
	credential_t *cred;
	stringer_t *address;
	smtp_inbound_prefs_t *result;

	// If they try to send this command without saying hello.
	if (!(con->smtp.helo) && con->smtp.authenticated == false) {
		con_write_bl(con, "503 RCPT TO REJECTED - PLEASE PROVIDE A HELO OR EHLO AND TRY AGAIN\r\n", 68);
		return;
	}
	else if (!(con->smtp.mailfrom)) {
		con_write_bl(con, "503 RCPT TO REJECTED - PLEASE PROVIDE A MAIL FROM TRY AGAIN\r\n", 61);
		return;
	}

	// Check to make sure we are not over the recipient limit.
	if (con->smtp.num_recipients >= magma.smtp.recipient_limit) {
		con_write_bl(con, "541 RCPT TO REJECTED - PER MESSAGE RECIPIENT LIMIT REACHED\r\n", 50);
		return;
	}

	// Cleanup the input data, if it is blank, reject.
	if (!(address = smtp_parse_rcpt_to(con))) {
		con_write_bl(con, "551 RCPT TO REJECTED - INVALID ADDRESS\r\n", 40);
		return;
	}

	// If the session is authorized, we are going to be relaying this message.
	if (con->smtp.authenticated == true) {

		// Are we only allowing this user to send messages via a secure method?
		if (con->smtp.out_prefs->ssl == 1 && con_secure(con) != 1) {
			con_write_bl(con, "530 TRANSPORT LAYER SECURITY REQUIRED - THIS ACCOUNT CAN ONLY BE ACCESSED USING A SECURE NETWORK CONNECTION, PLEASE ENABLE " \
				"SSL OR TLS AND TRY AGAIN\r\n", 149);
			st_free(address);
			return;
		}

		// Make sure the proposed size is good.
		if (con->smtp.suggested_length > con->smtp.out_prefs->send_size_limit) {
			con_print(con, "552 OUTBOUND SIZE LIMIT EXCEEDED - THIS ACCOUNT MAY ONLY SEND MESSAGES UP TO %zu BYTES IN LENGTH\r\n",
				con->smtp.out_prefs->send_size_limit);
			st_free(address);
			return;
		}

		// Has this user exceeded their sending quota.
		if (con->smtp.out_prefs->sent_today + con->smtp.num_recipients >= con->smtp.out_prefs->daily_send_limit) {
			con_print(con, "451 OUTBOUND MAIL QUOTA EXCEEDED - THIS ACCOUNT MAY ONLY SEND %u %s IN A TWENTY-FOUR HOUR PERIOD, PLEASE TRY AGAIN AT A LATER TIME\r\n",
				con->smtp.out_prefs->daily_send_limit, (con->smtp.out_prefs->daily_send_limit != 1) ? "MESSAGES" : "MESSAGE" );
			st_free(address);
			return;
		}

		// Figure out where to store the address.
		if (!smtp_add_recipient(con, address)) {
			con_write_bl(con, "451 INTERNAL SERVER ERROR - PLEASE TRY AGAIN LATER\r\n", 52);
			st_free(address);
			return;
		}

		con_write_bl(con, "250 RCPT TO ACCEPTED\r\n", 22);
		st_free(address);
		return;
	}

	/// BUG: The code should be able to differentiate between invalid addresses which trigger a NULL return and allocation (or similar)
	/// errors which are temporary.
	if (!(cred = credential_alloc_mail(address))) {
		con_write_bl(con, "451 INTERNAL SERVER ERROR - PLEASE TRY AGAIN LATER\r\n", 52);
		st_free(address);
		return;
	}

	// Hit the mailboxes table, and see if this is a legitimate address.
	state = smtp_fetch_inbound(cred, address, &result);

	// If the account is locked.
	if (state == -2) {
		con_print(con, "550 ACCOUNT LOCKED - THE ACCOUNT <%.*s> HAS BEEN ADMINISTRATIVELY LOCKED\r\n", st_length_int(address),
			st_char_get(lower_st(address)));
		credential_free(cred);
		st_free(address);
		return;
	}
	// If the account is inactive.
	else if (state == -3) {
		con_print(con, "550 ACCOUNT LOCKED - THE ACCOUNT <%.*s> HAS BEEN LOCKED FOR INACTIVITY\r\n", st_length_int(address),
			st_char_get(lower_st(address)));
		credential_free(cred);
		st_free(address);
		return;
	}
	// The account has been locked for abuse.
	else if (state == -4) {
		con_print(con, "550 ACCOUNT LOCKED - THE ACCOUNT <%.*s> HAS BEEN LOCKED FOR ABUSE POLICY VIOLATIONS\r\n", st_length_int(address),
			st_char_get(lower_st(address)));
		credential_free(cred);
		st_free(address);
		return;
	}
	// The user has locked the account.
	else if (state == -5) {
		con_print(con, "550 ACCOUNT LOCKED - THE ACCOUNT <%.*s> HAS BEEN LOCKED AT THE REQUEST OF THE OWNER\r\n", st_length_int(address),
			st_char_get(lower_st(address)));
		credential_free(cred);
		st_free(address);
		return;
	}
	// The user has locked the account.
	else if (state == -6) {
		con_print(con, "551 RELAY ACCESS DENIED - THE DOMAIN <%.*s> IS NOT HOSTED LOCALLY AND RELAY ACCESS REQUIRES AUTHENTICATION\r\n",
			st_length_int(cred->mail.domain), st_char_get(lower_st(cred->mail.domain)));
		credential_free(cred);
		st_free(address);
		return;
	}
	// If the domain is local but user wasn't found.
	else if (state == 0) {
		con_print(con, "554 INVALID RECIPIENT - THE EMAIL ADDRESS <%.*s> DOES NOT MATCH AN ACCOUNT ON THIS SYSTEM\r\n",
			st_length_int(address), st_char_get(lower_st(address)));
		credential_free(cred);
		st_free(address);
		return;
	}
	// Catch database or any other error here.
	else if (state < 0 || result == NULL) {
		con_write_bl(con, "451 INTERNAL SERVER ERROR - PLEASE TRY AGAIN LATER\r\n", 52);
		credential_free(cred);
		st_free(address);
		return;
	}

	credential_free(cred);
	st_free(address);

	// Check for duplicate user numbers.
	if (smtp_check_duplicate_recipient(con, result->usernum)) {
		con->smtp.num_recipients++;
		con_write_bl(con, "250 RCPT TO ACCEPTED\r\n", 22);
		smtp_free_inbound(result);
		return;
	}

	// Make sure the user isn't over their storage quota. If the message is from the admin or contact email address,
	// accept it regardless of whether the account is already over its quota.
	// LOW: This logic seemed to be broken. It was "fixed" but not tested. Make sure quota is only bypassed IF the bypass flag is set.
	if (result->overquota == 1 && !result->rollout && !result->forwarded) {

		if (!(con->smtp.bypass && ((magma.admin.contact && !st_cmp_ci_eq(con->smtp.mailfrom, magma.admin.contact)) ||
			(magma.admin.abuse && !st_cmp_ci_eq(con->smtp.mailfrom, magma.admin.abuse))))) {
			con_print(con, "552 MAILBOX FULL - THE ACCOUNT <%.*s> HAS EXCEEDED THEIR STORAGE QUOTA\r\n", st_length_int(result->rcptto),
				st_char_get(lower_st(result->rcptto)));
			smtp_free_inbound(result);
			return;
		}

	}

	// Check to see if the proposed size is over the established max.
	if (con->smtp.suggested_length > result->recv_size_limit) {
		con_print(con, "552 MESSAGE EXCEEDS SIZE LIMIT - THE ACCOUNT <%.*s> IS ONLY AUTHORIZED TO ACCEPT MESSAGES UP TO %u BYTES\r\n", 	st_length_int(result->rcptto), st_char_get(lower_st(result->rcptto)),
			result->recv_size_limit);
		smtp_free_inbound(result);
		return;
	}

	// Check the connecting server against several RBL databases.
	if (result->rbl == 1) {

		// Perform the check.
		if (con->smtp.checked.rbl == 0) {
			con->smtp.checked.rbl = smtp_check_rbl(con);
		}

		// If the user has elected to reject these messages.
		if (con->smtp.checked.rbl == -2 && result->rblaction == SMTP_ACTION_REJECT) {

			address = con_addr_presentation(con, MANAGEDBUF(INET6_ADDRSTRLEN));

			con_print(con, "550 MESSAGE BLOCKED - THE ADDRESS [%.*s] HAS BEEN LISTED ON A REALTIME BLACKLIST AND THE ACCOUNT <%.*s> IS CONFIGURED TO " \
				"REJECT MESSAGES FROM BLACKLISTED ADDRESSES\r\n",	st_length_int(address), st_char_get(address),	st_length_int(result->rcptto),
				st_char_get(lower_st(result->rcptto)));

			smtp_free_inbound(result);
			return;
		}
	}

	if (result->greylist == 1) {

		// Check the greylist and see if this IP has tried to e-mail this user before.
		// This function will only return zero if no rows are found matching, but will add an entry for next time.
		if (smtp_check_greylist(con, result) == -2) {
			con_print(con, "451 MAILBOX UNAVAILABLE - PLEASE TRY AGAIN IN %u %s\r\n", result->greytime, result->greytime != 1 ? "MINUTES" : "MINUTE");
			smtp_free_inbound(result);
			return;
		}

	}

	// Now we check to make sure the user hasn't received too many messages today. Basic mail-bomb defense.
	state = smtp_check_receive_quota(con, result);

	if (state == 1 || state == 2) {

		// Use the first message if the user is over their total, and the second if the IP subnet total is over.
		if (state == 1) {
			con_print(con, "451 MAILBOX UNAVAILABLE - THE ACCOUNT <%.*s> IS CONFIGURED NOT TO ACCEPT MORE THAN %u %s MESSAGES IN A TWENTY-FOUR " \
				"HOUR PERIOD - PLEASE TRY AGAIN LATER\r\n", st_length_int(result->rcptto), st_char_get(lower_st(result->rcptto)), result->daily_recv_limit,
				(result->daily_recv_limit != 1) ? "MESSAGES" : "MESSAGE");
		}
		else {
			con_print(con, "451 MAILBOX UNAVAILABLE - THE ACCOUNT <%.*s> IS CONFIGURED NOT TO ACCEPT MORE THAN %u %s MESSAGES FROM ANY SINGLE SUBNET IN " \
				"A TWENTY-FOUR HOUR PERIOD - PLEASE TRY AGAIN LATER\r\n", st_length_int(result->rcptto), st_char_get(lower_st(result->rcptto)), result->daily_recv_limit_ip,
				(result->daily_recv_limit_ip != 1) ? "MESSAGES" : "MESSAGE");
		}

		smtp_free_inbound(result);
		return;
	}


	// If this user is enforcing SPF.
	if (result->spf == 1) {
		// Perform the SPF check.
		if (con->smtp.checked.spf == 0) {
			con->smtp.checked.spf = spf_check(con_addr(con, MEMORYBUF(sizeof(ip_t))), con->smtp.helo, con->smtp.mailfrom);
		}

		/// BUG: Detect messages 'from' a local user/domain and tell them to authenticate first.
		if (con->smtp.checked.spf == -2 && result->spfaction == SMTP_ACTION_REJECT) {

			address = con_addr_presentation(con, MANAGEDBUF(INET6_ADDRSTRLEN));

			con_print(con, "550 MESSAGE BLOCKED - THE MESSAGE IS BEING REJECTED BECAUSE THE ADDRESS [%.*s] IS NOT AUTHORIZED TO SEND "
				"MESSAGES FROM <%.*s> - PLEASE CORRECT THE ISSUE AND TRY AGAIN\r\n", st_length_int(address), st_char_get(address),
				st_length_int(result->rcptto), st_char_get(lower_st(result->rcptto)));
			smtp_free_inbound(result);
			return;
		}
	}

	// Is this the user with the largest receive size is what?
	if (result->recv_size_limit > con->smtp.max_length) {
		con->smtp.max_length = result->recv_size_limit;
	}

	// Were good, so let the user know.
	smtp_add_inbound(con, result);
	con_write_bl(con, "250 RCPT TO ACCEPTED\r\n", 22);
	return;
}

void smtp_data_finish(connection_t *con, size_t read, int_t checker) {

	chr_t *stream;
	int_t increment;

	// In case we exit early.
	stream = st_data_get(con->network.buffer);

	// If there is no data in the buffer.
	if (read == 0) {
		read = con_read(con);
	}

	while (checker != 4 && status() && read > 0) {

		stream = st_data_get(con->network.buffer);

		for (increment = 0; increment < read && checker != 4; increment++) {
			if (checker == 0 && *stream == '\n') {
				checker++;
			}
			else if (checker == 1 && *stream == '.') {
				checker++;
			}
			else if (checker == 2 && *stream == '\n') {
				checker += 2;
			}
			else if (checker == 2 && *stream == '\r') {
				checker++;
			}
			else if (checker == 3 && *stream == '\n') {
				checker++;
			}
			else if (*stream == '\n') {
				checker = 1;
			}
			else if (checker != 0) {
				checker = 0;
			}
			stream++;
		}

		if (checker != 4) {
			read = con_read(con);
		}
	}

	// So that read line will get anything left in buffer.
	if ((stream - st_char_get(con->network.buffer)) < read) {
		st_data_set(&(con->network.line), st_data_get(con->network.buffer));
		st_length_set(&(con->network.line), stream - st_char_get(con->network.buffer));
	}

	return;
}

int_t smtp_data_read(connection_t *con, stringer_t **message) {

	chr_t *stream, *buffer;
	stringer_t *result, *holder;
	int_t read = 0, increment;
	size_t used = 0, size = 128 * 1024;
	int_t header = 1, checker = 1, carriage = 0;

	// In case we end early.
	*message = NULL;
	stream = st_data_get(con->network.buffer);

	// Expand the stringer in 128 KB chunks.
	if (!(result = st_alloc_opts(MAPPED_T | JOINTED | HEAP, size))) {
		smtp_data_finish(con, 0, checker);
		return -1;
	}

	// Setup the pointer into the stringer.
	buffer = st_char_get(result);
	read = con_read(con);

	while (checker != 4 && status() && read > 0) {

		// Setup the stream.
		stream = st_data_get(con->network.buffer);

		// Size check.
		if (read + used > con->smtp.max_length) {
			log_pedantic("Message exceeded size limit of %zu bytes. Reading till the end, and then returning an error.", con->smtp.max_length);
			smtp_data_finish(con, read, checker);
			st_free(result);
			return -2;
		}

		// Read in the new data.
		for (increment = 0; checker != 4 && increment < read; increment++) {

			// Logic for detecting header mode.
			if (header != 3) {
				if (header == 0 && *stream == '\n') {
					header++;
				}
				else if (header == 1 && *stream == '\n') {
					header += 2;
				}
				else if (header == 1 && *stream == '\r') {
					header++;
				}
				else if (header == 2 && *stream == '\n') {
					header++;
				}
				else if (header != 0) {
					header = 0;
				}
			}

			// Logic for detecting the end of a message.
			if (checker == 0 && *stream == '\n') {
				checker++;
			}
			else if (checker == 1 && *stream == '.') {
				checker++;
			}
			else if (checker == 2 && *stream == '\n') {
				checker += 2;
			}
			else if (checker == 2 && *stream == '\r') {
				checker++;
			}
			else if (checker == 3 && *stream == '\n') {
				checker++;
			}
			else if (*stream == '\n') {
				checker = 1;
			}
			else if (checker != 0) {
				checker = 0;
			}

			// Make sure every line ends with a carriage return, then line break.
			if (*stream == '\n' && carriage == 0) {
				*buffer++ = '\r';
				used++;
			}
			else if (*stream == '\r') {
				carriage = 1;
			}
			else if (carriage != 0) {
				carriage = 0;
			}

			// In header mode, we only read in ASCII (0x00 to 0x7F) characters.
			if (header != 3 && *stream >= 0) {
				*buffer++ = *stream;
				used++;
			}
			// Otherwise we read in anything.
			else if (header == 3) {
				*buffer++ = *stream;
				used++;
			}

			// Make sure we have enough room in the buffer.
			if (used + 32 > size) {
				if ((holder = st_realloc(result, size + (128 * 1024))) == NULL) {
					log_pedantic("Attempted to allocate a buffer of %zu bytes to hold an incoming message, and failed. Returning an error to the client.", size + (128 * 1024));
					smtp_data_finish(con, read, checker);
					st_free(result);
					return -1;
				}

				// Setup the pointers again.
				size += 128 * 1024;
				result = holder;
				buffer = st_char_get(result) + used;
			}

			stream++;
		}

		if (checker != 4) {
			read = con_read(con);
		}
	}

	// The server is shutting down or the client disconnected.
	if (status() != 1) {
		st_free(result);
		return -3;
	}
	else if (read <= 0) {
		st_free(result);
		return -4;
	}

	// So that read line will get anything left in buffer.
	if ((stream - st_char_get(con->network.buffer)) < read) {
		st_data_set(&(con->network.line), st_data_get(con->network.buffer));
		st_length_set(&(con->network.line), stream - st_char_get(con->network.buffer));
	}

	// Setup the output.
	st_length_set(result, used);
	*message = result;

	return 1;
}

void smtp_data_outbound(connection_t *con) {

	int_t state;
	stringer_t *holder;

	// Check the outbound blocker list.
	if (pattern_check(con->smtp.message->text) == -2) {
		con_write_bl(con, "550 DATA BLOCKED - THIS MESSAGE IS BEING BLOCKED ON SUSPICION OF BEING JUNK MAIL\r\n", 82);
		smtp_session_reset(con);
		return;
	}

	// Check the transmit quota one more time before we actually send the message.
	else if (smtp_check_transmit_quota(con->smtp.out_prefs->usernum, con->smtp.num_recipients, con->smtp.out_prefs) == 1) {
		con_print(con, "451 DATA BLOCKED - THIS USER ACCOUNT IS ONLY ALLOWED TO SEND %u %s IN A TWENTY-FOUR HOUR PERIOD - PLEASE TRY AGAIN LATER\r\n",
			con->smtp.out_prefs->daily_send_limit, (con->smtp.out_prefs->daily_send_limit != 1) ? "MESSAGES" : "MESSAGE");
		smtp_session_reset(con);
		return;
	}

	// We need to extract just the e-mail address from the message header.
	else if (!(holder = mail_extract_address(&(con->smtp.message->from)))) {
		con_write_bl(con, "550 DATA FAILED - UNABLE TO LOCATE THE \"FROM\" ADDRESS IN THE MESSAGE - PLEASE CHECK YOUR EMAIL CLIENT SETTINGS AND TRY AGAIN\r\n", 126);
		smtp_session_reset(con);
		return;
	}


	// Now check the address in the header.
	else if ((state = smtp_check_authorized_from(con->smtp.out_prefs->usernum, holder)) == 0) {
		con_print(con, "550 DATA BLOCKED - THIS USER ACCOUNT IS NOT AUTHORIZED TO SEND MESSAGES WITH THE ADDRESS <%.*s> - PLEASE CHECK YOUR " \
			"EMAIL CLIENT SETTINGS AND TRY AGAIN\r\n", st_length_get(holder), st_char_get(holder));
		smtp_session_reset(con);
		st_free(holder);
		return;
	}
	else if (state < 0) {
		con_print(con, "451 DATA FAILED - AN ERROR OCCURRED WHILE CHECKING WHETHER THIS ACCOUNT IS AUTHORIZED TO SEND MESSAGES USING <%.*s> - " \
			"PLEASE TRY AGAIN LATER\r\n", st_length_get(holder), st_char_get(holder));
		smtp_session_reset(con);
		st_free(holder);
		return;
	}

	// Check the mail from. If its a designated null sender, use the holder.
	if (!st_cmp_ci_eq(con->smtp.mailfrom, PLACER("<>", 2))) {
		st_free(con->smtp.mailfrom);
		con->smtp.mailfrom = st_dupe(holder);
	}
	else if ((state = smtp_check_authorized_from(con->smtp.out_prefs->usernum, con->smtp.mailfrom)) == 0) {
		con_print(con, "550 DATA BLOCKED - THIS USER ACCOUNT IS NOT AUTHORIZED TO SEND MESSAGES WITH THE ADDRESS <%.*s> - PLEASE CHECK YOUR " \
			"EMAIL CLIENT SETTINGS AND TRY AGAIN\r\n", st_length_get(con->smtp.mailfrom), st_char_get(con->smtp.mailfrom));
		smtp_session_reset(con);
		st_free(holder);
		return;
	}
	else if (state < 0) {
		con_print(con, "451 DATA FAILED - AN ERROR OCCURRED WHILE CHECKING WHETHER THIS ACCOUNT IS AUTHORIZED TO SEND MESSAGES USING <%.*s> - " \
			"PLEASE TRY AGAIN LATER\r\n", st_length_get(con->smtp.mailfrom), st_char_get(con->smtp.mailfrom));
		smtp_session_reset(con);
		st_free(holder);
		return;
	}

	st_free(holder);
	holder = NULL;

	// Make sure this message does not contain a virus.
	if ((state = virus_check(con->smtp.message->text)) == -2) {
		con_print(con, "550 DATA BLOCKED - THIS MESSAGE IS BEING BLOCKED BECAUSE IT APPEARS TO CONTAIN A COMPUTER VIRUS OR INTERNET WORM\r\n", 114);
		smtp_session_reset(con);
		return;
	}

	state = smtp_relay_message(con, &holder);
	if (state < 0 && holder != NULL) {
		con_write_st(con, holder);
		st_free(holder);
	}
	else if (state > 0) {
		con_write_st(con, holder);
		smtp_update_transmission_stats(con);
		st_free(holder);
	}
	else {
		con_write_bl(con, "451 DATA FAILED - UNABLE TO RELAY OUTBOUND MESSAGES AT THIS TIME - PLEASE TRY AGAIN LATER\n\n", 91);
	}

	smtp_session_reset(con);

	return;
}

void smtp_data_inbound(connection_t *con) {

	smtp_inbound_prefs_t *current;
	uint32_t perm_errors = 0, temp_errors = 0, delivered = 0, bounces = 0;

	current = con->smtp.in_prefs;
	while (current != NULL) {

		// Process the message.
		current->outcome = smtp_accept_message(con, current);

		// Track the outcomes.
		if (current->outcome == SMTP_OUTCOME_PERM_FAILURE) {
			perm_errors++;
		}
		else if (current->outcome == SMTP_OUTCOME_TEMP_SERVER || current->outcome == SMTP_OUTCOME_TEMP_LOCKED || current->outcome == SMTP_OUTCOME_TEMP_OVERQUOTA) {
			temp_errors++;
		}
		else if (current->outcome != SMTP_OUTCOME_SUCESS) {
			bounces++;
		}
		else {
			delivered++;
		}

		current = (smtp_inbound_prefs_t *) current->next;
	}

	// Tell the connected server what the outcome was.
	if (perm_errors != 0 && !temp_errors && !delivered && !bounces) {
		con_print(con, "551 DATA FAILED - UNABLE TO DELIVER THE MESSAGE TO %s\r\n", con->smtp.num_recipients > 1 ?	"ANY OF THE RECIPIENTS" :	"THE RECIPIENT");
	}
	else if (temp_errors != 0 && !delivered && !bounces) {
		con_print(con, "451 DATA FAILED - ENCOUNTERED A TEMPORARY ERROR WITH %s\r\n", con->smtp.num_recipients > 1 ? "ALL OF THE RECIPIENTS" : "THE RECIPIENT");
	}
	else {
		con_write_bl(con, "250 MESSAGE ACCEPTED\r\n", 22);
		if (temp_errors || perm_errors || bounces) {
			smtp_bounce(con);
		}
	}

	smtp_session_reset(con);

	return;

}

void smtp_data(connection_t *con) {

	int_t state;
	stringer_t *text;
	smtp_message_t *message;

	// Make sure outsiders say HELO.
	// If the remote host tries to send data before sending a MAIL FROM and RCPT TO, return a protocol error.
	if (con->smtp.helo == NULL && con->smtp.authenticated == false) {
		con_write_bl(con, "503 DATA REJECTED - PLEASE PROVIDE A HELO OR EHLO AND TRY AGAIN\r\n", 65);
		smtp_requeue(con);
		return;
	}
	else if (con->smtp.mailfrom == NULL) {
		con_write_bl(con, "503 DATA REJECTED - PLEASE PROVIDE A MAIL FROM AND TRY AGAIN\r\n", 62);
		smtp_requeue(con);
		return;
	}
	else if ((con->smtp.authenticated == false && con->smtp.in_prefs == NULL) || (con->smtp.authenticated == true && con->smtp.out_prefs->recipients == NULL)) {
		con_write_bl(con, "503 DATA REJECTED - PLEASE PROVIDE A RCPT AND TRY AGAIN\r\n", 57);
		smtp_requeue(con);
		return;
	}

	// Tell the user we are ready to receive.
	con_write_bl(con, "354 Enter mail, end with \".\" on a line by itself.\r\n", 51);

	if ((state = smtp_data_read(con, &text)) == -1) {
		con_write_bl(con, "451 DATA FAILED - MEMORY ALLOCATION FAILED - PLEASE TRY AGAIN LATER\r\n", 69);
		smtp_requeue(con);
		return;
	}
	/// LOW: Why are -2 and -3 identical?
	else if (state == -2 && con->smtp.authenticated == true) {
		con_print(con, "552 DATA FAILED - OUTBOUND SIZE LIMIT EXCEEDED - THIS ACCOUNT MAY ONLY SEND MESSAGES UP TO %zu BYTES IN LENGTH\r\n", con->smtp.max_length);
		smtp_requeue(con);
		return;
	}
	else if (state == -2) {
		con_print(con, "552 DATA FAILED - INBOUND SIZE LIMIT EXCEEDED - THE MAILBOXES INDICATED MAY ONLY RECIEVE MESSAGES UP TO %zu BYTES IN LENGTH\r\n", con->smtp.max_length);
		smtp_requeue(con);
		return;
	}
	else if (state == -3) {
		con_write_bl(con, "451 DATA FAILED - THE SERVER IS SHUTTING DOWN FOR MAINTENANCE - PLEASE TRY AGAIN LATER\r\n", 88);
		smtp_quit(con);
		return;
	}
	else if (state == -4) {
		con_write_bl(con, "421 DATA FAILED - THE CONNECTION TIMED OUT WHILE WAITING FOR DATA - GOOD BYE\r\n", 78);
		smtp_quit(con);
		return;
	}
	else if (state < 0) {
		con_write_bl(con, "451 DATA FAILED - INTERNAL SERVER ERROR - PLEASE TRY AGAIN LATER\n\n", 66);
		smtp_requeue(con);
		return;
	}

	// Count the number of Received lines.
	if (mail_count_received(text) > magma.smtp.relay_limit) {
		con_write_bl(con, "550 DATA FAILED - THE MESSAGE HAS TOO MANY RECEIVED HEADER LINES AND IS BEING REJECTED BECAUSE IT APPEARS TO BE CAUGHT IN A " \
			"FORWARD LOOP\r\n", 138);
		smtp_requeue(con);
		st_free(text);
		return;
	}

	// Setup the message structure and cleanup the message data.
	if (mail_message_cleanup(&text) != 1) {
		con_write_bl(con, "451 DATA FAILED - INTERNAL SERVER ERROR - PLEASE TRY AGAIN LATER\n\n", 66);
		smtp_requeue(con);
		st_free(text);
		return;
	}

	// Create the message structure.
	if (!(message = mail_create_message(text))) {
		con_write_bl(con, "451 DATA FAILED - INTERNAL SERVER ERROR - PLEASE TRY AGAIN LATER\n\n", 66);
		smtp_requeue(con);
		st_free(text);
		return;
	}

	// Add all of the required headers.
	if (!mail_add_required_headers(con, message)) {
		con_write_bl(con, "451 DATA FAILED - INTERNAL SERVER ERROR - PLEASE TRY AGAIN LATER\n\n", 66);
		mail_destroy_message(message);
		smtp_requeue(con);
		return;
	}

	// Add the message context to the session.
	con->smtp.message = message;

	if (con->smtp.authenticated == true) {
		requeue(&smtp_data_outbound, &smtp_requeue, con);
	}
	else {
		requeue(&smtp_data_inbound, &smtp_requeue, con);
	}

	return;
}

/**
 * @brief	The start of the protocol handler for the SMTP server.
 * @param	con		the new inbound SMTP client connection.
 * @return	This function returns no value.
 */
void smtp_init(connection_t *con) {

	// Does this connection come from a trusted IP?
	if (smtp_bypass_check(con)) {
		con->smtp.bypass = 1;
	}

	// Queue a reverse lookup.
	con_reverse_enqueue(con);

	// Print_t the greeting and queue for a new command.
	con_print(con, "220 %.*s ESMTP Magma\r\n", st_length_int(con->server->domain), st_char_get(con->server->domain));
	smtp_requeue(con);

	return;
}

void submission_init(connection_t *con) {

	con->smtp.submission = true;
	smtp_init(con);

	return;
}
