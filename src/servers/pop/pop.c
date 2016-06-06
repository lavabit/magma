
/**
 * @file /magma/servers/pop/pop.c
 *
 * @brief	Functions used to handle POP commands/actions.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/// TODO: Review error messages and update them with the appropriate response code.

/**
 * @brief	Initialize a TLS session for an unauthenticated POP3 session.
 * @note	RFC 2595 / section 4 dictates that the STLS/STARTTLS command should only be available in the authorization state.
 * @param	con		the connection of the POP3 client requesting the transport layer security upgrade.
 * @return	This function returns no value (all error messages are written directly to the requesting client).
 */
void pop_starttls(connection_t *con) {

	if (con->pop.session_state != 0) {
		pop_invalid(con);
		return;
	}
	else if (con_secure(con) == 1) {
		con_write_bl(con, "-ERR Session is already encrypted.\r\n", 36);
		return;
	}
	else if (con_secure(con) == -1) {
		con_write_bl(con, "-ERR This server has not been configured to support STLS.\r\n", 59);
		return;
	}

	// Tell the user that we are ready to start the negotiation.
	con_write_bl(con, "+OK Ready to start TLS negotiation.\r\n", 37);

	if (!(con->network.ssl = ssl_alloc(con->server, con->network.sockd, M_SSL_BIO_NOCLOSE))) {
		con_write_bl(con, "-ERR STARTTLS FAILED\r\n", 22);
		log_pedantic("The SSL connection attempt failed.");
		return;
	}

	stats_increment_by_name("pop.connections.secure");
	st_length_set(con->network.buffer, 0);
	con->network.line = pl_null();
	con->network.status = 1;
	pop_session_reset(con);

	return;
}

/**
 * @brief	Execute a POP3 no-operation command.
 * @param	con		the POP3 client connection issuing the command.
 * @return	This function returns no value.
 */
void pop_noop(connection_t *con) {

	con_write_bl(con, "+OK\r\n", 5);
	return;
}

/**
 * @brief	A function handler for invalid POP3 commands.
 * @param	con		the POP3 client connection issuing the command.
 * @return	This function returns no value.
 */
void pop_invalid(connection_t *con) {

	con->protocol.violations++;
	usleep(con->server->violations.delay);
	con_write_bl(con, "-ERR Unrecognized command.\r\n", 28);

	return;
}

/**
 * @brief	Reset the user's mailbox, in response to a POP3 RSET command.
 * @see		pop_session_reset()
 * @param	con		the POP3 client connection issuing the command.
 * @return	This function returns no value.
 */
void pop_rset(connection_t *con) {

	int_t ret;

	if (con->pop.session_state != 1) {
		pop_invalid(con);
	}
	else if ((ret = pop_session_reset(con)) == 1) {
		con_write_bl(con, "+OK All messages were reset.\r\n", 30);
	}
	else if (ret == -1) {
		con_write_bl(con, "-ERR Message reset failed.\r\n", 28);
	}
	else {
		con_write_bl(con, "-ERR Session reset complete.\r\n", 30);
	}

	return;
}

/**
 * @brief	Gracefully destroy a POP3 session, whether because of an error or in response to a user QUIT command.
 * @param	con		the POP3 client connection to be shut down.
 * @brief	This function returns no value.
 */
void pop_quit(connection_t *con) {

	if (con_status(con) == 2) {
		con_write_bl(con, "-ERR Unexpected connection shutdown detected. Goodbye.\r\n", 56);
	}
	else if (con_status(con) >= 0) {
		con_write_bl(con, "+OK Goodbye.\r\n", 14);
	}
	else {
		con_write_bl(con, "-ERR Network connection failure.\r\n", 34);
	}

	con_destroy(con);

	return;
}

/**
 * @brief	Accept a username for POP3 authentication.
 * @note	This command is only allowed for sessions which have not yet been authenticated.
 * 			If the username has already been supplied pre-authentication, the old value will be overwritten with the new one.
 * @param	con		the POP3 client connection issuing the command.
 * @brief	This function returns no value.
 */
void pop_user(connection_t *con) {

	stringer_t *username = NULL, *clean = NULL;

	if (con->pop.session_state != 0) {
		pop_invalid(con);
		return;
	}

	// If they didn't pass in a valid username.
	if (!(username = pop_user_parse(con)) || !(clean = credential_address(username))) {
		con_write_bl(con, "-ERR Invalid USER command.\r\n", 28);
		st_cleanup(username);
		return;
	}

	// Check for a previously provided value and free it.
	st_cleanup(con->pop.username);
	st_free(username);

	// Store the value we were given. Until authentication, this will be the fully qualified username.
	con->pop.username = clean;

	// Tell the client everything worked.
	con_write_bl(con, "+OK Username accepted.\r\n", 24);

	return;
}

/**
 * @brief	Accept and verify a password for POP3 authentication.
 * @note	This command is only allowed for sessions which have not yet been authenticated, but which have already supplied a username.
 *			If the username/password combo was validated, the account information is retrieved and checked to see if it is locked.
 *			After successful authentication, this function will prohibit insecure connections for any user configured to use SSL only,
 *			and enforce the existence of only one POP3 session at a time. See RFC 3206 regarding the response code extension.
 *			Finally, the database Log table for this user's POP3 access is updated, and all the user's messages are retrieved.
 * @param	con		the POP3 client connection issuing the command.
 * @return	This function returns no value.
 */
void pop_pass(connection_t *con) {

	int_t state;
	auth_t *auth = NULL;
	stringer_t *password = NULL;

	if (con->pop.session_state != 0) {
		pop_invalid(con);
		return;
	}

	// The user command must come before the PASS command.
	if (st_empty(con->pop.username)) {
		con_write_bl(con, "-ERR You must supply a username first.\r\n", 40);
		return;
	}

	// If they didn't pass in a valid password.
	if (!(password = pop_pass_parse(con))) {
		con_write_bl(con, "-ERR Invalid PASS command.\r\n", 28);
		return;
	}

	// Authenticate the username and password.
	if ((state = auth_login(con->pop.username, password, &auth))) {
		if (state < 0) con_write_bl(con, "-ERR [SYS/TEMP] Internal server error. Please try again later.\r\n", 64);
		else con_write_bl(con, "-ERR [AUTH] The username and password combination is invalid.\r\n", 63);
		st_free(password);
		return;
	}

	// Free the plain text password. If secure memory was enabled, it will be wiped as well.
	st_free(password);

	// Check if the account is locked.
	if (auth->status.locked) {

		if (auth->status.locked == 1) con_write_bl(con, "-ERR [SYS/PERM] This account has been administratively locked.\r\n", 64);
		else if (auth->status.locked == 2) con_write_bl(con, "-ERR [SYS/PERM] This account has been locked for inactivity.\r\n", 62);
		else if (auth->status.locked == 3) con_write_bl(con, "-ERR [SYS/PERM] This account has been locked on suspicion of abuse.\r\n", 69);
		else if (auth->status.locked == 4) con_write_bl(con, "-ERR [SYS/PERM] This account has been locked at the request of the user.\r\n", 74);
		else if (auth->status.locked != 0) con_write_bl(con, "-ERR [SYS/PERM] This account has been locked.\r\n", 47);

		auth_free(auth);
		return;
	}

	// Check whether this account requires transport layer security, and if so, ensure the transport layer is encrypted.
	if (auth->status.tls && con_secure(con) != 1) {
		con_write_bl(con, "-ERR [SYS/PERM] This user account is configured to require that all POP sessions be connected over SSL.\r\n", 105);
		return;
	}

	// Pull the user info out.
	state = new_meta_get(auth, META_PROT_POP, META_GET_MESSAGES, &(con->pop.user));






	// Single session check.
	if (con->pop.user->refs.pop != 1) {
		con->pop.user = NULL;
		meta_inx_remove(con->pop.username, META_PROT_POP);
		con_write_bl(con, "-ERR [IN-USE] This account is being used by another session. Please try again in a few minutes.\r\n", 97);
		return;
	}

	// User logging.
	log_info("User %.*s logged in from %s via POP. { poprefs = %lu, imaprefs = %lu, messages = %lu }",
		st_length_int(con->pop.username), st_char_get(con->pop.username), st_char_get(con_addr_presentation(con, MANAGEDBUF(256))),
		con->pop.user->refs.pop, con->pop.user->refs.imap, con->pop.user->messages ? inx_count(con->pop.user->messages) : 0);

	// Update the log and unlock the session.
	meta_data_update_log(con->pop.user, META_PROT_POP);

	meta_user_wlock(con->pop.user);
	meta_messages_login_update(con->pop.user, META_LOCKED);
	meta_user_unlock(con->pop.user);

	// Update session state.
	con->pop.session_state = 1;

	// Tell the client everything worked.
	con_write_bl(con, "+OK Password accepted.\r\n", 24);

	return;
}

/**
 * @brief	Display the POP3 server capabilities, in response to a POP3 CAPA command.
 * @note	See RFC 2449 POP3 Extension Mechanism for details.
 * @param	con		the POP3 client connection issuing the command.
 * @return	This function returns no value.
 */
void pop_capa(connection_t *con) {

	// If the user is in authorization mode via an insecure channel and there is an SSL context, output the STLS capability.
	if (con->pop.session_state == 0 && con_secure(con) == 0) {
		con_print(con, "+OK Capabilities follow.\r\nTOP\r\nUSER\r\nUIDL\r\nSTLS\r\nPIPELINING\r\nEXPIRE NEVER\r\nLOGIN-DELAY 0\r\n"
			"RESP-CODES\r\nAUTH-RESP-CODE\r\nIMPLEMENTATION magmad %s\r\n.\r\n", build_version());
	}
	// Otherwise don't advertise STLS.
	else {
		con_print(con, "+OK Capabilities follow.\r\nTOP\r\nUSER\r\nUIDL\r\nPIPELINING\r\nEXPIRE NEVER\r\nLOGIN-DELAY 0\r\n"
			"RESP-CODES\r\nAUTH-RESP-CODE\r\nIMPLEMENTATION magmad %s\r\n.\r\n", build_version());
	}

	return;
}

/**
 * @brief	Display a user's message statistics, in response to a POP3 STAT command.
 * @see		pop_total_messages(), pop_total_size()
 * @param	con		the POP3 client connection issuing the command.
 * @return	This function returns no value.
 */
void pop_stat(connection_t *con) {

	uint64_t count, size;

	if (con->pop.session_state != 1) {
		pop_invalid(con);
		return;
	}

	meta_user_rlock(con->pop.user);
	count = pop_total_messages(con->pop.user->messages);
	size = pop_total_size(con->pop.user->messages);
	meta_user_unlock(con->pop.user);

	// Print the information.
	con_print(con, "+OK %llu %llu\r\n", count, size);

	return;
}

/**
 * @brief	Get the sequence number of the last read message, in response to a POP3 LAST command.
 * @see		pop_get_last()
 * @param	con		the POP3 client connection issuing the command.
 * @return	This function returns no value.
 */
void pop_last(connection_t *con) {

	uint64_t number = 0;

	if (con->pop.session_state != 1) {
		pop_invalid(con);
		return;
	}

	meta_user_rlock(con->pop.user);
	number = pop_get_last(con->pop.user->messages);
	meta_user_unlock(con->pop.user);

	// Print the information.
	con_print(con, "+OK %llu\r\n", number);

	return;
}

/**
 * @brief	Get the list of a user's messages, in response to a POP3 LIST command.
 * @see		pop_total_messages(), pop_get_message()
 * @param	con		the POP3 client connection issuing the command.
 * @return	This function returns no value.
 */
void pop_list(connection_t *con) {

	uint64_t number;
	inx_cursor_t *cursor;
	meta_message_t *active;
	bool_t result;

	if (con->pop.session_state != 1) {
		pop_invalid(con);
		return;
	}

	// Get the argument, if any.
	result = pop_num_parse(con, &number, false);

	meta_user_rlock(con->pop.user);

	// Output all of the messages that aren't deleted or appended.
	if (!result) {
		number = 1;
		con_print(con, "+OK %llu messages total.\r\n", pop_total_messages(con->pop.user->messages));

		if (con->pop.user->messages && (cursor = inx_cursor_alloc(con->pop.user->messages))) {

			while ((active = inx_cursor_value_next(cursor))) {

				if ((active->status & (MAIL_STATUS_APPENDED | MAIL_STATUS_HIDDEN)) == 0) {
					con_print(con, "%llu %u\r\n", number++, active->size);
				}
				else if ((active->status & MAIL_STATUS_APPENDED) == 0) {
					number++;
				}

			}

			inx_cursor_free(cursor);
		}

		con_write_bl(con, ".\r\n", 3);
	}
	// Output a specific message.
	else {
		if (!(active = pop_get_message(con->pop.user->messages, number))) {
			con_write_bl(con, "-ERR Message not found.\r\n", 25);
		}
		else if ((active->status & MAIL_STATUS_HIDDEN) == MAIL_STATUS_HIDDEN) {
			con_write_bl(con, "-ERR Message marked for deletion.\r\n", 35);
		}
		else {
			con_print(con, "+OK %llu %u\r\n", number, active->size);
		}
	}

	meta_user_unlock(con->pop.user);

	return;
}

/**
 * @brief	Get a message, in response to a POP3 DELE command.
 * @param	con		the POP3 client connection issuing the command.
 * @return	This function returns no value.
 */
void pop_dele(connection_t *con) {

	uint64_t number;
	meta_message_t *active = NULL;

	if (con->pop.session_state != 1) {
		pop_invalid(con);
		return;
	}

	// A message number is a required argument.
	if (!pop_num_parse(con, &number, true)) {
		con_write_bl(con, "-ERR The delete command requires a numeric argument.\r\n", 54);
		return;
	}

	meta_user_wlock(con->pop.user);

	if (!(active = pop_get_message(con->pop.user->messages, number))) {
		con_write_bl(con, "-ERR Message not found.\r\n", 25);
	}
	else if ((active->status & MAIL_STATUS_HIDDEN) == MAIL_STATUS_HIDDEN) {
		con_write_bl(con, "-ERR Message already deleted.\r\n", 31);
	}
	else {
		active->status += MAIL_STATUS_HIDDEN;
		con_write_bl(con, "+OK Message marked for deletion.\r\n", 34);
	}

	meta_user_unlock(con->pop.user);

	return;
}

/**
 * @brief	Get the UIDL for a message or collection of messages, in response to a POP3 UIDL command.
 * @param	con		the POP3 client connection issuing the command.
 * @return	This function returns no value.
 */
void pop_uidl(connection_t *con) {

	uint64_t number;
	inx_cursor_t *cursor;
	meta_message_t *active;
	bool_t result;

	if (con->pop.session_state != 1) {
		pop_invalid(con);
		return;
	}

	// Get the argument, if any.
	result = pop_num_parse(con, &number, false);

	meta_user_rlock(con->pop.user);

	// Output all of the messages that aren't deleted or appended.
	if (!result) {
		number = 1;
		con_print(con, "+OK %llu messages total.\r\n", pop_total_messages(con->pop.user->messages));

		if (con->pop.user->messages && (cursor = inx_cursor_alloc(con->pop.user->messages))) {

			while ((active = inx_cursor_value_next(cursor))) {

				if ((active->status & (MAIL_STATUS_APPENDED | MAIL_STATUS_HIDDEN)) == 0) {
					con_print(con, "%llu %llu\r\n", number++, active->messagenum);
				} else if ((active->status & MAIL_STATUS_APPENDED) == 0) {
					number++;
				}

			}

			inx_cursor_free(cursor);
		}

		con_write_bl(con, ".\r\n", 3);
	}
	// Output a specific message.
	else {

		if (!(active = pop_get_message(con->pop.user->messages, number))) {
			con_write_bl(con, "-ERR Message not found.\r\n", 25);
		}
		else if ((active->status & MAIL_STATUS_HIDDEN) == MAIL_STATUS_HIDDEN) {
			con_write_bl(con, "-ERR Message marked for deletion.\r\n", 35);
		}
		else {
			con_print(con, "+OK %llu %llu\r\n", number, active->messagenum);
		}

	}

	meta_user_unlock(con->pop.user);

	return;
}

/**
 * @brief	Get the top lines of a message or collection of messages, in response to a POP3 TOP command.
 * @note	This function will fail if a deleted message was specified by the user.
 * @param	con		the POP3 client connection issuing the command.
 * @return	This function returns no value.
 */
void pop_top(connection_t *con) {

	meta_message_t *meta;
	uint64_t lines, number;
	mail_message_t *message;

	if (con->pop.session_state != 1) {
		pop_invalid(con);
		return;
	}

	// A message number is a required argument.
	if (!pop_top_parse(con, &number, &lines)) {
		con_write_bl(con, "-ERR The top command requires two numeric arguments.\r\n", 54);
		return;
	}

	meta_user_rlock(con->pop.user);

	// Get the message.
	if (!(meta = pop_get_message(con->pop.user->messages, number))) {
		meta_user_unlock(con->pop.user);
		con_write_bl(con, "-ERR Message not found.\r\n", 25);
		return;
	}

	// Check for deletion.
	if ((meta->status & MAIL_STATUS_HIDDEN) == MAIL_STATUS_HIDDEN) {
		meta_user_unlock(con->pop.user);
		con_write_bl(con,  "-ERR This message has been marked for deletion.\r\n", 49);
		return;
	}

	// Load the message and spit back the right number of lines.
	if (!(message = mail_load_message_top(meta, con->pop.user, con->server, lines, true))) {
		meta_user_unlock(con->pop.user);
		con_write_bl(con, "-ERR The message you requested could not be loaded into memory. It has either been deleted "
			"by another connection or is corrupted.\r\n", 131);
		return;
	}

	meta_user_unlock(con->pop.user);

	// Dot stuff the message.
	st_replace(&(message->text), PLACER("\n.", 2), PLACER("\n..", 3));

	// Tell the client to prepare for a message. The size is strictly informational.
	con_print(con, "+OK %u characters follow.\r\n", st_length_get(message->text));

	// We use raw socket IO because it is much faster when writing large amounts of data.
	con_write_st(con, message->text);

	// If the message didn't end with a line break, spit two.
	if (*(st_char_get(message->text) + st_length_get(message->text) - 1) == '\n') {
		con_write_bl(con, ".\r\n", 3);
	}
	else {
		con_write_bl(con, "\r\n.\r\n", 5);
	}

	mail_destroy(message);

	return;
}

/**
 * @brief	Retrieve a user's message, in response to a POP3 RETR command.
 * @note	This function will fail if a deleted message was specified by the user.
 * @param	con		the POP3 client connection issuing the command.
 * @return	This function returns no value.
 */
void pop_retr(connection_t *con) {

	uint64_t number;
	meta_message_t *meta;
	mail_message_t *message;

	if (con->pop.session_state != 1) {
		pop_invalid(con);
		return;
	}

	// Which message are we getting.
	if (!pop_num_parse(con, &number, true)) {
		con_write_bl(con, "-ERR The retrieve command requires a numeric argument.\r\n", 56);
		return;
	}

	meta_user_rlock(con->pop.user);

	// Get the message.
	if (!(meta = pop_get_message(con->pop.user->messages, number))) {
		meta_user_unlock(con->pop.user);
		con_write_bl(con, "-ERR Message not found.\r\n", 25);
		return;
	}

	// Check for deletion.
	if ((meta->status & MAIL_STATUS_HIDDEN) == MAIL_STATUS_HIDDEN) {
		meta_user_unlock(con->pop.user);
		con_write_bl(con,  "-ERR This message has been marked for deletion.\r\n", 49);
		return;
	}

	// Load the message and spit back the right number of lines.
	if (!(message = mail_load_message(meta, con->pop.user, con->server, 1))) {
		meta_user_unlock(con->pop.user);
		con_write_bl(con, "-ERR The message you requested could not be loaded into memory. It has either been "
			"deleted by another connection or is corrupted.\r\n", 131);
		return;
	}

	meta_user_unlock(con->pop.user);

	// Dot stuff the message.
	st_replace(&(message->text), PLACER("\n.", 2), PLACER("\n..", 3));

	// Tell the client to prepare for a message. The size is strictly informational.
	con_print(con, "+OK %u characters follow.\r\n", st_length_get(message->text));

	// We use raw socket IO because it is much faster when writing large amounts of data.
	con_write_st(con, message->text);

	// If the message didn't end with a line break, spit two.
	if (*(st_char_get(message->text) + st_length_get(message->text) - 1) == '\n') {
		con_write_bl(con, ".\r\n", 3);
	}
	else {
		con_write_bl(con, "\r\n.\r\n", 5);
	}

	mail_destroy(message);

	return;
}

/**
 * @brief	Initialize a new POP3 connection.
 * @param	con		the newly connected POP3 client connection.
 * @return	This function returns no value.
 */
void pop_init(connection_t *con) {

	con_write_bl(con, "+OK magma\r\n", 11);
	pop_requeue(con);

	return;
}
