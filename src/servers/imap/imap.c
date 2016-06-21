
/**
 * @file /magma/servers/imap/imap.c
 *
 * @brief	Functions used to handle IMAP commands/actions.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/// TODO: Review error messages and update them with the appropriate response code.
/// LOW: When should we check the serial number to see if the local data is stale and needs to be refreshed?

/**
 * @brief	Create a secure connection for an IMAP session.
 *
 * @note	RFC 2595 / section 3.1 specifies that STARTTLS is only available in a non-authenticated state.
 *
 * @param	con		the connection on top of which the TLS session will be established.
 *
 * @return	This function returns no value.
 */
void imap_starttls(connection_t *con) {

	if (con->imap.session_state != 0) {
		imap_invalid(con);
		return;
	}
	else if (con_secure(con) == 1) {
		con_print(con, "%.*s BAD This session is already using TLS.\r\n", st_length_get(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}
	else if (con_secure(con) == -1) {
		con_print(con, "%.*s NO This server is not configured to support TLS.\r\n", st_length_get(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}

	// Tell the user that we are ready to start the negotiation.
	con_print(con, "%.*s OK Ready to start TLS negotiation.\r\n", st_length_get(con->imap.tag), st_char_get(con->imap.tag));

	if (!(con->network.tls = ssl_alloc(con->server, con->network.sockd, M_SSL_BIO_NOCLOSE))) {
		con_print(con, "%.*s NO TLS Connection attempt failed.\r\n", st_length_get(con->imap.tag), st_char_get(con->imap.tag));
		log_pedantic("The TLS connection attempt failed.");
		return;
	}

	// Clear the input buffer. A shorthand session reset.
	stats_increment_by_name("imap.connections.secure");
	st_length_set(con->network.buffer, 0);
	con->network.line = pl_null();
	con->network.status = 1;

	return;
}

/**
 * @brief	Respond to an invalid IMAP command from a client.
 *
 * @param	con		a pointer to the client connection that issued the bad command.
 *
 * @return	This function returns no value.
 */
void imap_invalid(connection_t *con) {

	con->protocol.violations++;
	usleep(con->server->violations.delay);
	con_print(con, "%.*s BAD Command not recognized.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));

	return;
}

/**
 * @brief	Terminate an IMAP session gracefully with a "BYE" message and destroy the underlying connection.
 *
 * @param	con		the IMAP connection to be terminated.
 *
 * @return	This function returns no value.
 */
void imap_logout(connection_t *con) {

	if (con_status(con) == 2) {
		con_write_bl(con, "* BYE Unexpected connection shutdown detected. Goodbye.\r\n", 57);
	}
	else if (con_status(con) >= 0) {
		con_print(con, "* BYE Goodbye.\r\n%.*s OK Completed.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}
	else {
		con_write_bl(con, "* BYE Network connection failure.\r\n", 35);
	}

	con_destroy(con);

	return;
}

/**
 * @brief	Attempt to perform a user login on an IMAP client connection.
 *
 * @param	con		a pointer to the connection object of the remote session.
 *
 * @return	This function returns no value.
 */
void imap_login(connection_t *con) {

	int_t state = 1;
	auth_t *auth = NULL;

	// The LOGIN command is only valid in the non-authenticated state.
	if (con->imap.session_state != 0) {
		con_print(con, "%.*s BAD This session has already been authenticated. Please logout and connect again to change users.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}

	// Input validation. Requires two non-NULL string arguments.
	if (ar_length_get(con->imap.arguments) != 2 || imap_get_type_ar(con->imap.arguments, 0) == IMAP_ARGUMENT_TYPE_ARRAY ||
		!imap_get_st_ar(con->imap.arguments, 0) ||	imap_get_type_ar(con->imap.arguments, 1) == IMAP_ARGUMENT_TYPE_ARRAY ||
		!imap_get_st_ar(con->imap.arguments, 1)) {
		con_print(con, "%.*s BAD The login command requires two string arguments.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}

	// Convert the strings into a full fledged authentication object.
	if ((state = auth_login(imap_get_st_ar(con->imap.arguments, 0), imap_get_st_ar(con->imap.arguments, 1), &auth))) {

		// The AUTHENTICATIONFAILED response code is provided by RFC 5530 which states: "Authentication failed for some reason on which the server is
		// unwilling to elaborate. Typically, this includes 'unknown user' and 'bad password'."
		if (state > 0) {
			con_print(con,  "%.*s NO [AUTHENTICATIONFAILED] The username and password combination is invalid.\r\n",
				st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		}
		else {
			con_print(con, "%.*s NO [ALERT] Internal server error. Please try again in a few minutes.\r\n",
				st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		}
		return;
	}

	// Check if the account is locked.
	if (auth->status.locked) {

		// The CONTACTADMIN response code is provided by RFC 5530 which states: "The user should contact the system administrator or support."
		if (auth->status.locked == 1) {
			con_print(con, "%.*s NO [CONTACTADMIN] This account has been administratively locked.\r\n",
				st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		}
		/// HIGH: Inactivity locks shouldn't prevent a user from logging into the system.
		else if (auth->status.locked == 2) {
			con_print(con, "%.*s NO [CONTACTADMIN] This account has been locked for inactivity.\r\n",
				st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		}
		else if (auth->status.locked == 3) {
			con_print(con, "%.*s NO [CONTACTADMIN] This account has been locked on suspicion of abuse.\r\n",
				st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		}
		else if (auth->status.locked == 4) {
			con_print(con, "%.*s NO [CONTACTADMIN] This account has been locked at the request of the user.\r\n",
				st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		}
		else {
			con_print(con, "%.*s NO [CONTACTADMIN] This account has been locked.\r\n",
				st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		}

		auth_free(auth);
		return;
	}

	// Pull the user info out.
	if ((state = meta_get(auth->usernum, auth->username, auth->keys.master, auth->tokens.verification, META_PROTOCOL_IMAP,
		META_GET_MESSAGES | META_GET_KEYS | META_GET_FOLDERS, &(con->imap.user)))) {

		// The UNAVAILABLE response code is provided by RFC 5530 which states: "Temporary failure because a subsystem is down."
		if (state < 0) {
			con_print(con, "%.*s NO [UNAVAILABLE] This server is unable to access your mailbox. Please try again later.\r\n",
				st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		}
		else {
			con_print(con, "%.*s NO [AUTHENTICATIONFAILED] The username and password combination is invalid.\r\n",
				st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		}

		auth_free(auth);
		return;
	}
	else if (st_populated(con->imap.username)) {
		st_free(con->imap.username);
	}

	// Store the username and usernum as part of the session.
	con->imap.username = st_dupe(auth->username);
	con->imap.usernum = auth->usernum;
	auth_free(auth);

	// Check whether this account is using the secure storage feature and/or is configured to require transport layer security,
	// and if so, check whether the transport layer is encrypted.
	if (((con->imap.user->flags & META_USER_ENCRYPT_DATA) || (con->imap.user->flags & META_USER_TLS)) && con_secure(con) != 1) {

		meta_inx_remove(con->imap.usernum, META_PROTOCOL_IMAP);

		// The PRIVACYREQUIRED response code is provided by RFC 5530 which states: "The operation is not permitted due to a lack of privacy. If
		// Transport Layer Security (TLS) is not in use, the client could try STARTTLS and then repeat the operation."
		con_print(con, "%.*s NO [PRIVACYREQUIRED] This account requires an encrypted network connection to login. Your current connection is " \
			"vulnerable to villainous voyeurs. Reconnect using transport layer security, aka SSL/TLS, to access this account.\r\n",
			st_length_int(con->imap.tag), st_char_get(con->imap.tag));

		con->imap.user = NULL;
		con->imap.usernum = 0;
		return;
	}

	// Store the checkpoints.
	meta_user_rlock(con->imap.user);
	con->imap.messages_checkpoint = con->imap.user->serials.messages;
	con->imap.folders_checkpoint = con->imap.user->serials.folders;
	con->imap.user_checkpoint = con->imap.user->serials.user;
	meta_user_unlock(con->imap.user);

	// Debug logging.
	log_pedantic("User %.*s logged in from %s via IMAP. {poprefs = %lu, imaprefs = %lu, messages = %lu, folders = %lu}",
		st_length_int(con->imap.username), st_char_get(con->imap.username), st_char_get(con_addr_presentation(con, MANAGEDBUF(1024))),
		con->imap.user->refs.pop, con->imap.user->refs.imap, con->imap.user->messages ? inx_count(con->imap.user->messages) : 0, con->imap.user->folders ? inx_count(con->imap.user->folders) : 0);

	// Update the log.
	if (con->imap.user->refs.imap == 1) {

		// HIGH: We used to assume if the bypass flag was set, its a webmail session. But right now were lacking a trusted connection interface.
		//if (con->imap.bypass == 1) {
		//	meta_data_update_log(con->imap.user, META_PROT_WEB);
		//}
		//else {
			meta_data_update_log(con->imap.user, META_PROTOCOL_IMAP);
		//}
	}

	// Update session state.
	con->imap.session_state = 1;

	// Tell the client everything worked.
	con_print(con, "%.*s OK Password accepted.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	return;
}

void imap_noop(connection_t *con) {
	if (con->imap.session_state == 1 && con->imap.selected != 0 && con->imap.user && imap_session_update(con) == 1) {
		con_print(con, "* %lu EXISTS\r\n* %lu RECENT\r\n%.*s OK NOOP Completed.\r\n", con->imap.messages_total, con->imap.messages_recent,
			st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}
	else {
		con_print(con, "%.*s OK NOOP Completed.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}
	return;
}

void imap_check(connection_t *con) {

	if (con->imap.session_state == 1 && con->imap.selected != 0 && con->imap.user && imap_session_update(con) >= 0) {
		con_print(con, "* %lu EXISTS\r\n* %lu RECENT\r\n%.*s OK CHECK Completed.\r\n", con->imap.messages_total,
			con->imap.messages_recent, st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}
	else if (con->imap.session_state == 1) {
		con_print(con, "%.*s OK CHECK Completed.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}
	else {
		con_print(con, "%.*s BAD The CHECK command is not available until you are authenticated.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}
	return;
}

void imap_list(connection_t *con) {

	inx_t *list;
	stringer_t *name;
	inx_cursor_t *cursor;
	meta_folder_t *active;

	// Check for the right state.
	if (con->imap.session_state != 1) {
		con_print(con, "%.*s BAD The list command is not available until you are authenticated.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}

	// Input validation. Requires two string arguments, which can both be NULL.
	if (ar_length_get(con->imap.arguments) != 2 || imap_get_type_ar(con->imap.arguments, 0) == IMAP_ARGUMENT_TYPE_ARRAY || imap_get_type_ar(con->imap.arguments, 1) == IMAP_ARGUMENT_TYPE_ARRAY) {
		con_print(con, "%.*s BAD The list command requires two string arguments.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}

	// To handle the special mailbox case.
	if (imap_get_st_ar(con->imap.arguments, 1) == NULL) {
		con_print(con, "* LIST (\\Noselect) \".\" \"\"\r\n%.*s OK LIST Complete.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}

	meta_user_rlock(con->imap.user);

	//********************************************//

	//Here. Now force the lister to use the new interface.

	if ((cursor = inx_cursor_alloc(con->imap.user->message_folders))) {

		stringer_t *nm;
		message_folder_t *cur;
		while ((cur = inx_cursor_value_next(cursor))) {
			if ((nm = magma_folder_name(con->imap.user->message_folders, cur))) {
				st_free(nm);
			}
			else {
				log_pedantic("Folder name error.");
			}

		}

		inx_cursor_free(cursor);
	}
	//*************************************************//

	// Because the list index is a shallow copy we need to ensure the original memory buffers aren't freed by another thread.
	if ((list = imap_narrow_folders(con->imap.user->folders, imap_get_st_ar(con->imap.arguments, 0), imap_get_st_ar(con->imap.arguments, 1))) != NULL) {

		if ((cursor = inx_cursor_alloc(list))) {

			// Some buggy clients require that the Inbox always come first.
			while ((active = inx_cursor_value_next(cursor))) {
				if (active->parent == 0 && !st_cmp_ci_eq(NULLER(active->name), PLACER("Inbox", 5))) {
					con_print(con, "* LIST (\\Noinferiors) \".\" \"%s\"\r\n", active->name);
				}
			}

			inx_cursor_reset(cursor);

			// On the second pass print all the folders except the Inbox. Because some folders have special characters we need to
			// generate an encoded/sanitized version of the folder name to print.
			while ((active = inx_cursor_value_next(cursor))) {
				if ((active->parent != 0 || st_cmp_ci_eq(NULLER(active->name), PLACER("Inbox", 5))) &&
					(name = imap_folder_name_escaped(con->imap.user->folders, active))) {
						con_print(con, "* LIST () \".\" %.*s\r\n", st_length_int(name), st_char_get(name));
						st_free(name);
				}
			}

			inx_cursor_free(cursor);
		}

		inx_free(list);
	}

	meta_user_unlock(con->imap.user);

	// Let the user know everything worked.
	con_print(con, "%.*s OK LIST Complete.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	return;
}

void imap_lsub(connection_t *con) {

	inx_t *list;
	stringer_t *name;
	inx_cursor_t *cursor;
	meta_folder_t *active;

	// Check for the right state.
	if (con->imap.session_state != 1) {
		con_print(con, "%.*s BAD The LSUB command is not available until you are authenticated.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}

	// Input validation. Requires two string arguments, which can both be NULL.
	if (ar_length_get(con->imap.arguments) != 2 || imap_get_type_ar(con->imap.arguments, 0) == IMAP_ARGUMENT_TYPE_ARRAY || imap_get_type_ar(con->imap.arguments, 1) == IMAP_ARGUMENT_TYPE_ARRAY) {
		con_print(con, "%.*s BAD The LSUB command requires two string arguments.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}

	// To handle the special mailbox case.
	if (imap_get_st_ar(con->imap.arguments, 1) == NULL) {
		con_print(con, "* LSUB (\\Noselect) \".\" \"\"\r\n%.*s OK LSUB Complete.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}

	meta_user_rlock(con->imap.user);

	// Because the list index is a shallow copy we need to ensure the original memory buffers aren't freed by another thread.
	if ((list = imap_narrow_folders(con->imap.user->folders, imap_get_st_ar(con->imap.arguments, 0), imap_get_st_ar(con->imap.arguments, 1))) != NULL) {

		if ((cursor = inx_cursor_alloc(list))) {

			// Some buggy clients require that the Inbox always come first.
			while ((active = inx_cursor_value_next(cursor))) {
				if (active->parent == 0 && !st_cmp_ci_eq(NULLER(active->name), PLACER("Inbox", 5))) {
					con_print(con, "* LSUB (\\Noinferiors) \".\" \"%s\"\r\n", active->name);
				}
			}

			inx_cursor_reset(cursor);

			// On the second pass print all the folders except the Inbox. Because some folders have special characters we need to
			// generate an encoded/sanitized version of the folder name to print.
			while ((active = inx_cursor_value_next(cursor))) {
				if ((active->parent != 0 || st_cmp_ci_eq(NULLER(active->name), PLACER("Inbox", 5))) &&
					(name = imap_folder_name_escaped(con->imap.user->folders, active))) {
						con_print(con, "* LSUB () \".\" %.*s\r\n", st_length_int(name), st_char_get(name));
						st_free(name);
				}
			}

			inx_cursor_free(cursor);
		}

		inx_free(list);
	}

	meta_user_unlock(con->imap.user);

	// Let the user know everything worked.
	con_print(con, "%.*s OK LSUB Complete.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));

	return;
}

/**
 * @brief	Create a new IMAP folder in response to the IMAP "CREATE" command.
 *
 * @see		imap_folder_create()
 *
 * @param	con		the connection across which the folder creation request was made.
 *
 * @return	This function returns no value.
 */
void imap_create(connection_t *con) {

	int_t state;

	// Check for the right state.
	if (con->imap.session_state != 1) {
		con_print(con, "%.*s BAD The create command is not available until you are authenticated.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}

	// Input validation. Requires one string argument, which cannot be NULL.
	if (ar_length_get(con->imap.arguments) != 1 || imap_get_type_ar(con->imap.arguments, 0) == IMAP_ARGUMENT_TYPE_ARRAY) {
		con_print(con, "%.*s BAD The create command requires one string arguments.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}

	meta_user_wlock(con->imap.user);

	// Create the folder.
	state = imap_folder_create(con->imap.user->usernum, con->imap.user->folders, imap_get_st_ar(con->imap.arguments, 0));

	// If the serial number indicates no outside changes we can increment it without forcing a refresh.
	if (con->imap.user->serials.folders == serial_get(OBJECT_FOLDERS, con->imap.user->usernum)) {
		con->imap.folders_checkpoint = con->imap.user->serials.folders = serial_increment(OBJECT_FOLDERS, con->imap.user->usernum);
	}
	// The context is already due for a refresh, but we increment the serial to let the rest of the cluster know about the change.
	else {
		serial_increment(OBJECT_FOLDERS, con->imap.user->usernum);
	}

	meta_user_unlock(con->imap.user);

	// Let the user know what happened.
	if (state == 1) {
		con_print(con, "%.*s OK CREATE Complete.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}
	else if (state == -1) {
		con_print(con, "%.*s NO [CANNOT] CREATE Failed. The specified folder name is invalid.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}
	else if (state == -2) {
		con_print(con, "%.*s NO [LIMIT] CREATE Failed. A folder can have a maximum of %i levels.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag), IMAP_FOLDER_RECURSION_LMIIT);
	}
	else if (state == -3) {
		con_print(con, "%.*s NO [CANNOT] CREATE Failed. Your Inbox is not allowed to have sub folders.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}
	else if (state == -4) {
		con_print(con, "%.*s NO [LIMIT] CREATE Failed. Individual folder names must be %i characters or less.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag), FOLDER_LENGTH_LIMIT);
	}
	else if (state == -5) {
		con_print(con, "%.*s NO [ALREADYEXISTS] CREATE Failed. The folder name provided already exists.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}
	else {
		con_print(con, "%.*s NO [UNAVAILABLE] CREATE Failed. An internal server error occurred while trying to create the new folder. Please try again later.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}

	return;
}

/**
 * @brief	Handle the IMAP "DELETE" command and delete the specified IMAP folder.
 *
 * @see		imap_folder_remove()
 *
 * @param	con		a pointer to the connection object of the IMAP session generating the delete request.
 *
 * @return	This function returns no value.
 */
void imap_delete(connection_t *con) {

	int_t state;

	// Check for the right state.
	if (con->imap.session_state != 1) {
		con_print(con, "%.*s BAD The delete command is not available until you are authenticated.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}

	// Input validation. Requires one string argument, which cannot be NULL.
	if (ar_length_get(con->imap.arguments) != 1 || imap_get_type_ar(con->imap.arguments, 0) == IMAP_ARGUMENT_TYPE_ARRAY) {
		con_print(con, "%.*s BAD The delete command requires one string arguments.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}

	meta_user_wlock(con->imap.user);

	// Delete the folder.
	state = imap_folder_remove(con->imap.user->usernum, con->imap.user->folders, con->imap.user->messages, imap_get_st_ar(con->imap.arguments, 0));

	// If the serial number indicates no outside changes we can increment it without forcing a refresh.
	if (con->imap.user->serials.folders == serial_get(OBJECT_FOLDERS, con->imap.user->usernum)) {
		con->imap.folders_checkpoint = con->imap.user->serials.folders = serial_increment(OBJECT_FOLDERS, con->imap.user->usernum);
	}
	// The context is already due for a refresh, but we increment the serial to let the rest of the cluster know about the change.
	else {
		serial_increment(OBJECT_FOLDERS, con->imap.user->usernum);
	}

	meta_user_unlock(con->imap.user);

	// Let the user know what happened.
	if (state == 1) {
		con_print(con, "%.*s OK DELETE Complete.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}
	else if (state == -1) {
		con_print(con, "%.*s NO [CANNOT] DELETE Failed. The folder provided for deletion is invalid.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}
	else if (state == -2) {
		con_print(con, "%.*s NO [CANNOT] DELETE Failed. You are not allowed to delete your Inbox.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}
	else if (state == -3) {
		con_print(con, "%.*s NO [NONEXISTENT] DELETE Failed. The folder you are trying to delete does not exist.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}
	else {
		con_print(con, "%.*s NO [UNAVAILABLE] DELETE Failed. An internal server error occurred while trying to delete the folder. Please try again later.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}

	return;
}

void imap_rename(connection_t *con) {

	int_t state;

	// Check for the right state.
	if (con->imap.session_state != 1) {

		con_print(con, "%.*s BAD The rename command is not available until you are authenticated.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}

	// Input validation. Requires one string argument, which cannot be NULL.
	if (ar_length_get(con->imap.arguments) != 2|| imap_get_type_ar(con->imap.arguments, 0) == IMAP_ARGUMENT_TYPE_ARRAY || imap_get_type_ar(con->imap.arguments, 1) == IMAP_ARGUMENT_TYPE_ARRAY) {

		con_print(con, "%.*s BAD The rename command requires two string arguments.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}

	meta_user_wlock(con->imap.user);

	// Rename the folder.
	state = imap_folder_rename(con->imap.user->usernum, con->imap.user->folders, imap_get_st_ar(con->imap.arguments, 0), imap_get_st_ar(con->imap.arguments, 1));

	// If the serial number indicates no outside changes we can increment it without forcing a refresh.
	if (con->imap.user->serials.folders == serial_get(OBJECT_FOLDERS, con->imap.user->usernum)) {
		con->imap.folders_checkpoint = con->imap.user->serials.folders = serial_increment(OBJECT_FOLDERS, con->imap.user->usernum);
	}
	// The context is already due for a refresh, but we increment the serial to let the rest of the cluster know about the change.
	else {
		serial_increment(OBJECT_FOLDERS, con->imap.user->usernum);
	}

	meta_user_unlock(con->imap.user);

	// Let the user know what happened.
	if (state == 1) {
		con_print(con, "%.*s OK RENAME Complete.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}
	else if (state == -1) {
		con_print(con, "%.*s NO RENAME Failed. One of the folder names provided is invalid.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}
	else if (state == -2) {
		con_print(con, "%.*s NO RENAME Failed. You are not allowed to rename your Inbox.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}
	else if (state == -3) {
		con_print(con, "%.*s NO RENAME Failed. The folder you are trying to rename does not exist.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}
	else if (state == -4) {
		con_print(con, "%.*s NO RENAME Failed. A folder can have a maximum of %i levels.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag), IMAP_FOLDER_RECURSION_LMIIT);
	}
	else if (state == -5) {
		con_print(con, "%.*s NO RENAME Failed. The target name already exists.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}
	else if (state == -6) {
		con_print(con, "%.*s NO RENAME Failed. Individual folder names must be %i characters or less.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag), FOLDER_LENGTH_LIMIT);
	}
	else if (state == -7) {
		con_print(con, "%.*s NO RENAME Failed. A folder cannot be renamed such that it becomes a child of itself.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}
	else {
		con_print(con, "%.*s NO RENAME Failed. An internal server error occurred while trying to rename the folder. Please try again later.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}

	return;
}

void imap_status(connection_t *con) {

	int_t state;
	size_t number;
	chr_t buffer[128];
	imap_arguments_t *values;

	stringer_t *output = NULL;
	imap_folder_status_t status;

	// Check for the right state.
	if (con->imap.session_state != 1) {
		con_print(con, "%.*s BAD The status command is not available until you are authenticated.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}

	// Input validation. Requires two arguments, which cannot be NULL.
	if (ar_length_get(con->imap.arguments) != 2 || imap_get_type_ar(con->imap.arguments, 0) == IMAP_ARGUMENT_TYPE_ARRAY || imap_get_type_ar(con->imap.arguments, 1) != IMAP_ARGUMENT_TYPE_ARRAY) {
		con_print(con, "%.*s BAD The status command requires a folder name and a parenthetical list of status items as arguments.\r\n",
			st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}

	// Get the folder status.
	meta_user_rlock(con->imap.user);
	state = imap_folder_status(con->imap.user->folders, con->imap.user->messages, imap_get_st_ar(con->imap.arguments, 0), &status);
	meta_user_unlock(con->imap.user);

	// Figure out what to output.
	if (state == 1) {

		values = imap_get_ar_ar(con->imap.arguments, 1);
		number = ar_length_get(values);

		for (size_t i = 0; i < number; i++) {

			// Figure out what the client wants to output.
			if (!st_cmp_ci_eq(imap_get_st_ar(values, i), PLACER("MESSAGES", 8))) {
				snprintf(buffer, 128, "%sMESSAGES %lu", (output == NULL ? "" : " "), status.messages);
				output = st_append_opts(1024, output, NULLER(buffer));
			}
			else if (!st_cmp_ci_eq(imap_get_st_ar(values, i), PLACER("RECENT", 6))) {
				snprintf(buffer, 128, "%sRECENT %lu", (output == NULL ? "" : " "), status.recent);
				output = st_append_opts(1024, output, NULLER(buffer));
			}
			else if (!st_cmp_ci_eq(imap_get_st_ar(values, i), PLACER("UNSEEN", 6))) {
				snprintf(buffer, 128, "%sUNSEEN %lu", (output == NULL ? "" : " "), status.unseen);
				output = st_append_opts(1024, output, NULLER(buffer));
			}
			else if (!st_cmp_ci_eq(imap_get_st_ar(values, i), PLACER("UIDNEXT", 7))) {
				snprintf(buffer, 128, "%sUIDNEXT %lu", (output == NULL ? "" : " "), status.uidnext);
				output = st_append_opts(1024, output, NULLER(buffer));
			}
			else if (!st_cmp_ci_eq(imap_get_st_ar(values, i), PLACER("UIDVALIDITY", 11))) {
				snprintf(buffer, 128, "%sUIDVALIDITY %lu", (output == NULL ? "" : " "), status.foldernum);
				output = st_append_opts(1024, output, NULLER(buffer));
			}
			// Unrecognized item requested.
			else {
				con_print(con, "%.*s BAD Invalid data item requested via the status command.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
				if (output) st_free(output);
				return;
			}
		}
	}

	if (state == 1 && output) {
		con_print(con, "* STATUS %.*s (%.*s)\r\n%.*s OK STATUS Complete.\r\n", st_length_get(imap_get_st_ar(con->imap.arguments, 0)), st_char_get(imap_get_st_ar(con->imap.arguments, 0)),
			st_length_get(output), st_char_get(output), st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}
	else if (state == 1){
		con_print(con, "%.*s NO STATUS Failed. An internal server error occurred while trying to retrieve the folder status. Please try again later.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}
	else if (state == -1) {
		con_print(con, "%.*s NO STATUS Failed. The folder name provided is invalid.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}
	else if (state == -2) {
		con_print(con, "%.*s NO STATUS Failed. The requested folder does not exist.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}
	else {
		con_print(con, "%.*s NO STATUS Failed. An internal server error occurred while trying to retrieve the folder status. Please try again later.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}

	if (output) st_free(output);
	return;
}

void imap_subscribe(connection_t *con) {

	int_t state = 1;
	meta_folder_t *folder;

	// Check for the right state.
	if (con->imap.session_state != 1) {
		con_print(con, "%.*s BAD The SUBSCRIBE command is not available until you are authenticated.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}

	// Input validation. Requires one string argument, which cannot be NULL.
	if (ar_length_get(con->imap.arguments) != 1 || imap_get_type_ar(con->imap.arguments, 0) == IMAP_ARGUMENT_TYPE_ARRAY) {
		con_print(con, "%.*s BAD The SUBSCRIBE command requires one string arguments.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}

	// Create the folder.
	meta_user_wlock(con->imap.user);

	if (!(folder = meta_folders_by_name(con->imap.user->folders, imap_get_st_ar(con->imap.arguments, 0)))) {
		state = imap_folder_create(con->imap.user->usernum, con->imap.user->folders, imap_get_st_ar(con->imap.arguments, 0));
	}

	meta_user_unlock(con->imap.user);

	// Let the user know what happened.
	if (state == 1) {
		con_print(con, "%.*s OK SUBSCRIBE Complete.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}
	else if (state == -1) {
		con_print(con, "%.*s NO SUBSCRIBE Failed. The specified folder name is invalid.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}
	else if (state == -2) {
		con_print(con, "%.*s NO SUBSCRIBE Failed. A folder can have a maximum of %i levels.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag), IMAP_FOLDER_RECURSION_LMIIT);
	}
	else if (state == -3) {
		con_print(con, "%.*s NO SUBSCRIBE Failed. Your Inbox is not allowed to have sub folders.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}
	else if (state == -4) {
		con_print(con, "%.*s NO SUBSCRIBE Failed. Individual folder names must be %i characters or less.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag), FOLDER_LENGTH_LIMIT);
	}
	else {
		con_print(con, "%.*s NO SUBSCRIBE Failed. An internal server error occurred while trying to create the new folder. Please try again later.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}

	return;
}

void imap_unsubscribe(connection_t *con) {
	if (con->imap.session_state != 1) {
		con_print(con, "%.*s BAD The UNSUBSCRIBE command is not available until you are authenticated.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}
	else {
		con_print(con, "%.*s OK UNSUBSCRIBE complete.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}
	return;
}

void imap_examine(connection_t *con) {

	int_t state;
	chr_t buffer[128];
	inx_cursor_t *cursor;
	meta_message_t *active;
	imap_folder_status_t status;

	// Check for the right state.
	if (con->imap.session_state != 1) {
		con_print(con, "%.*s BAD The examine command is not available until you are authenticated.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}

	// Input validation. Requires one string argument, which cannot be NULL.
	if (ar_length_get(con->imap.arguments) != 1 || imap_get_type_ar(con->imap.arguments, 0) == IMAP_ARGUMENT_TYPE_ARRAY) {
		con_print(con, "%.*s BAD The examine command requires a string argument.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}

	// If a folder was previously selected, clear the recent flag before closing the mailbox.
	if (con->imap.selected != 0 && con->imap.read_only == 0) {
		meta_user_wlock(con->imap.user);
		if ((cursor = inx_cursor_alloc(con->imap.user->messages))) {
			while ((active = inx_cursor_value_next(cursor))) {
				if (active->foldernum == con->imap.selected && (active->status & MAIL_STATUS_RECENT) == MAIL_STATUS_RECENT) {
					active->status = (active->status | MAIL_STATUS_RECENT) ^ MAIL_STATUS_RECENT;
				}
			}
		inx_cursor_free(cursor);
		}
		meta_user_unlock(con->imap.user);
	}
	con->imap.read_only = con->imap.selected = con->imap.messages_total = con->imap.messages_recent = 0;

	// Get the folder status.
	meta_user_rlock(con->imap.user);
	state = imap_folder_status(con->imap.user->folders, con->imap.user->messages, imap_get_st_ar(con->imap.arguments, 0), &status);
	meta_user_unlock(con->imap.user);

	if (state == 1) {

		if (status.first != 0) {
			sprintf(buffer, "* OK [UNSEEN %lu]\r\n", status.first);
		}

		// Some clients expect the flags line to come first.
		con_print(con, "* FLAGS (\\Answered \\Flagged \\Deleted \\Seen \\Draft \\Recent)\r\n" \
			"* OK [PERMANENTFLAGS (\\Answered \\Flagged \\Deleted \\Seen \\Draft)]\r\n" \
			"* %lu EXISTS\r\n* %lu RECENT\r\n%s* OK [UIDVALIDITY %lu]\r\n* OK [UIDNEXT %lu]\r\n" \
			"%.*s OK EXAMINE [READ-ONLY] Complete.\r\n",
			status.messages, status.recent, (status.first != 0 ? buffer : ""), status.foldernum, status.uidnext,
			st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		con->imap.messages_total = status.messages;
		con->imap.messages_recent = status.recent;
		con->imap.selected = status.foldernum;
		con->imap.read_only = 1;
	}
	else if (state == -1) {
		con_print(con, "%.*s NO EXAMINE Failed. The folder name provided is invalid.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}
	else if (state == -2) {
		con_print(con, "%.*s NO EXAMINE Failed. The requested folder does not exist.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}
	else {
		con_print(con, "%.*s NO EXAMINE Failed. An internal server error occurred while trying to retrieve the folder status. Please try again later.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}

	return;
}

void imap_select(connection_t *con) {

	int_t state;
	chr_t buffer[128];
	inx_cursor_t *cursor;
	meta_message_t *active;
	imap_folder_status_t status;

	// Check for the right state.
	if (con->imap.session_state != 1) {

		con_print(con, "%.*s BAD The select command is not available until you are authenticated.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}

	// Input validation. Requires one string argument, which cannot be NULL.
	if (ar_length_get(con->imap.arguments) != 1 || imap_get_type_ar(con->imap.arguments, 0) == IMAP_ARGUMENT_TYPE_ARRAY) {

		con_print(con, "%.*s BAD The select command requires a string argument.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}

	// If a folder was previously selected, clear the recent flag before closing the mailbox.
	if (con->imap.selected != 0 && con->imap.read_only == 0) {
		meta_user_wlock(con->imap.user);
		if ((cursor = inx_cursor_alloc(con->imap.user->messages))) {
			while ((active = inx_cursor_value_next(cursor))) {
				if (active->foldernum == con->imap.selected && (active->status & MAIL_STATUS_RECENT) == MAIL_STATUS_RECENT) {
					active->status = (active->status | MAIL_STATUS_RECENT) ^ MAIL_STATUS_RECENT;
				}
			}
			inx_cursor_free(cursor);
		}
		meta_user_unlock(con->imap.user);
	}

	con->imap.read_only = con->imap.selected = con->imap.messages_total = con->imap.messages_recent = 0;

	// Get the folder status.
	meta_user_wlock(con->imap.user);
	if ((state = imap_folder_status(con->imap.user->folders, con->imap.user->messages, imap_get_st_ar(con->imap.arguments, 0), &status)) == 1) {

		// Now that this folder has been opened, remove the recent flag in the database.
		meta_data_flags_remove(con->imap.user->messages, con->imap.user->usernum, status.foldernum, MAIL_STATUS_RECENT);

	}
	meta_user_unlock(con->imap.user);

	if (state == 1) {

		if (status.first != 0) {
			sprintf(buffer, "* OK [UNSEEN %lu]\r\n", status.first);
		}

		// Some clients expect the flags line to come first.
		con_print(con, "* FLAGS (\\Answered \\Flagged \\Deleted \\Seen \\Draft \\Recent)\r\n" \
			"* OK [PERMANENTFLAGS (\\Answered \\Flagged \\Deleted \\Seen \\Draft)]\r\n" \
			"* %lu EXISTS\r\n* %lu RECENT\r\n%s* OK [UIDVALIDITY %lu]\r\n* OK [UIDNEXT %lu]\r\n" \
			"%.*s OK SELECT [READ-WRITE] Complete.\r\n",
			status.messages, status.recent, (status.first != 0 ? buffer : ""), status.foldernum, status.uidnext,
			st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		con->imap.messages_total = status.messages;
		con->imap.messages_recent = status.recent;
		con->imap.selected = status.foldernum;
		con->imap.read_only = 0;
	}
	else if (state == -1) {
		con_print(con, "%.*s NO SELECT Failed. The folder name provided is invalid.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}
	else if (state == -2) {
		con_print(con, "%.*s NO SELECT Failed. The requested folder does not exist.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}
	else {
		con_print(con, "%.*s NO SELECT Failed. An internal server error occurred while trying to retrieve the folder status. Please try again later.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}

	return;
}

void imap_store(connection_t *con) {

	int_t action;
	uint32_t flags;
	chr_t buffer[128];
	inx_cursor_t *cursor;
	meta_message_t *active;
	inx_t *messages, *duplicate;
	uint64_t recent = 0, exists = 0;

	// Check for the right state.
	if (con->imap.session_state != 1) {
		con_print(con, "%.*s BAD The STORE command is not available until you are authenticated.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}
	else if (con->imap.selected == 0) {
		con_print(con, "%.*s BAD The STORE command is not available until you have selected a folder.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}
	else if (con->imap.read_only == 1) {
		con_print(con, "%.*s BAD The STORE is not available in read only mode.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}

	// Input validation. Requires three arguments.
	else if (ar_length_get(con->imap.arguments) != 3|| imap_get_type_ar(con->imap.arguments, 0) == IMAP_ARGUMENT_TYPE_ARRAY || imap_get_type_ar(con->imap.arguments, 1) == IMAP_ARGUMENT_TYPE_ARRAY) {
		con_print(con, "%.*s BAD The store command requires three arguments.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}

	// Determine whether we are replacing, removing, or adding.
	else if ((action = imap_flag_action(imap_get_st_ar(con->imap.arguments, 1))) == 0) {
		con_print(con, "%.*s BAD An invalid data item parameter was passed to the store command.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}

	// Parse the list of flags.
	else if ((flags = imap_flag_parse(imap_get_ptr(con->imap.arguments, 2), imap_get_type_ar(con->imap.arguments, 2))) == 0) {
		con_print(con, "%.*s BAD Unable to parse the list of flags provided to the store command.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}

	// The client is not allowed to set the recent flag.
	else if ((flags & MAIL_STATUS_RECENT) == MAIL_STATUS_RECENT) {
		con_print(con, "%.*s BAD The recent message flag may only be set by the server.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}

	// Make sure its a valid sequence number.
	else if (imap_valid_sequence(imap_get_st_ar(con->imap.arguments, 0)) != 1) {
		con_print(con, "%.*s BAD An invalid sequence was provided to the store command.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}

	meta_user_wlock(con->imap.user);

	if (!con->imap.user || !con->imap.user->messages) {
		meta_user_unlock(con->imap.user);
		con_print(con, "%.*s OK Store complete.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}

	// Narrow by the sequence range provided.
	else if (!(messages = imap_narrow_messages(con->imap.user->messages, con->imap.selected, imap_get_st_ar(con->imap.arguments, 0), con->imap.uid))) {
		meta_user_unlock(con->imap.user);
		con_print(con, "%.*s OK Store complete.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}

	/// LOW: Shouldn't we be checking for stale status info so the update doesn't make decisions based on incorrect status data? On the other
	/// hand the actual IMAP logic is passed all the way through to the DB so even if the server ends up with incorrect status information, the database
	/// should remain accurate.
	// Perform the flag update. We use the shallow index copy so the updates are reflected in the central/shared session data.
	imap_update_flags(con->imap.user, messages, con->imap.selected, action, flags);

	// If the serial number indicates no outside changes we can increment it without forcing a refresh.
	if (con->imap.user->serials.messages == serial_get(OBJECT_MESSAGES, con->imap.user->usernum)) {
		con->imap.messages_checkpoint = con->imap.user->serials.messages = serial_increment(OBJECT_MESSAGES, con->imap.user->usernum);
	}

	// The context is already due for a refresh, but we increment the serial to let the rest of the cluster know about the change.
	else {
		serial_increment(OBJECT_MESSAGES, con->imap.user->usernum);
	}

	// Now that the updates are done we can create a deep copy of the meta data so we don't need to hold onto the session lock
	// while the status information is streamed out to the network.
	if ((duplicate = imap_duplicate_messages(messages))) {
		inx_free(messages);
	}

	meta_user_unlock(con->imap.user);

	// Loop through and output each message.
	if ((action & IMAP_FLAG_SILENT) != IMAP_FLAG_SILENT && (cursor = inx_cursor_alloc(duplicate))) {

			while ((active = inx_cursor_value_next(cursor))) {

				if (con->imap.uid) {
					snprintf(buffer, 128, " UID %lu", active->messagenum);
				} else {
					buffer[0] = '\0';
				}

				con_print(con, "* %lu FETCH (FLAGS (%s%s%s%s%s%s%s%s%s%s%s)%s)\r\n", active->sequencenum,
					(active->status & MAIL_STATUS_ANSWERED) != 0 ? "\\Answered" : "",
					(active->status & MAIL_STATUS_ANSWERED) != 0 && (active->status & MAIL_STATUS_FLAGGED) != 0 ? " " : "",
					(active->status & MAIL_STATUS_FLAGGED) != 0 ? "\\Flagged" : "",
					(active->status & (MAIL_STATUS_ANSWERED | MAIL_STATUS_FLAGGED)) != 0 && (active->status & MAIL_STATUS_DELETED) != 0 ? " " : "",
					(active->status & MAIL_STATUS_DELETED) != 0 ? "\\Deleted" : "",
					(active->status & (MAIL_STATUS_ANSWERED | MAIL_STATUS_FLAGGED | MAIL_STATUS_DELETED)) != 0 && (active->status & MAIL_STATUS_SEEN) != 0 ? " " : "",
					(active->status & MAIL_STATUS_SEEN) != 0 ? "\\Seen" : "",
					(active->status & (MAIL_STATUS_ANSWERED | MAIL_STATUS_FLAGGED | MAIL_STATUS_DELETED | MAIL_STATUS_SEEN)) != 0 && (active->status & MAIL_STATUS_DRAFT) != 0 ? " " : "",
					(active->status & MAIL_STATUS_DRAFT) != 0 ? "\\Draft" : "",
					(active->status & (MAIL_STATUS_ANSWERED | MAIL_STATUS_FLAGGED | MAIL_STATUS_DELETED | MAIL_STATUS_SEEN | MAIL_STATUS_DRAFT)) != 0 && (active->status & MAIL_STATUS_RECENT) != 0 ? " " : "",
					(active->status & MAIL_STATUS_RECENT) != 0 ? "\\Recent" : "", buffer);
			}

			inx_cursor_free(cursor);
	}

	// Cleanup
	inx_free(duplicate);

	// The relevant folder status changed.
	if (imap_session_update(con) == 1) {
		con_print(con, "* %lu EXISTS\r\n* %lu RECENT\r\n%.*s OK Store complete.\r\n", exists, recent, st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}
	else {
		con_print(con, "%.*s OK Store complete.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}

	return;
}

void imap_close(connection_t *con) {

	inx_cursor_t *cursor;
	meta_message_t *active;
	int_t recent = 0, deleted = 0;

	// Check for the right state.
	if (con->imap.session_state != 1) {
		con_print(con, "%.*s BAD The close command is not available until you are authenticated.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}
	else if (con->imap.selected == 0) {
		con_print(con, "%.*s BAD The close command is not available until you have selected a folder.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}
	// If this is a read only folder, don't delete, or update the message status.
	else if (con->imap.read_only == 1) {
		con_print(con, "%.*s OK Close complete.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		con->imap.read_only = con->imap.selected = con->imap.messages_total = con->imap.messages_recent = 0;
		return;
	}

	// Input validation. Requires no arguments.
	else if (con->imap.arguments != NULL && ar_length_get(con->imap.arguments) != 0) {
		con_print(con, "%.*s BAD The close command does not take any arguments.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}

	// Scan the mailbox, and see if any messages need to be deleted or made non-recent.
	meta_user_rlock(con->imap.user);
	if ((cursor = inx_cursor_alloc(con->imap.user->messages))) {
		while ((active = inx_cursor_value_next(cursor)) && (recent == 0 || deleted == 0)) {
			if (deleted == 0 && active->foldernum == con->imap.selected && (active->status & MAIL_STATUS_DELETED) == MAIL_STATUS_DELETED) {
				deleted = 1;
			}
			else if (recent == 0 && active->foldernum == con->imap.selected && (active->status & MAIL_STATUS_RECENT) == MAIL_STATUS_RECENT) {
				recent = 1;
			}
		}
		inx_cursor_free(cursor);
	}
	meta_user_unlock(con->imap.user);

	// If messages are deleted, get a global lock.
	if (deleted == 1) {

		meta_user_wlock(con->imap.user);

		// Don't expunge while POP session is active. We don't make this check until we have a write lock, just to prevent any
		// POP sessions from starting between when we startup and when we finish.
		if (con->imap.user->refs.pop > 0) {
			meta_user_unlock(con->imap.user);
			con_print(con, "%.*s NO The mailbox is locked by a POP session. Please try again later.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
			con->imap.read_only = con->imap.selected = con->imap.messages_total = con->imap.messages_recent = 0;
			return;
		}

		// Lock the account.
		if (user_lock(con->imap.user->usernum) != 1) {
			meta_user_unlock(con->imap.user);
			con_print(con, "%.*s NO The close command could not lock the user account. Please try again later.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
			con->imap.read_only = con->imap.selected = con->imap.messages_total = con->imap.messages_recent = 0;
			return;
		}

		// When you close the folder, delete all the messages.
		if ((cursor = inx_cursor_alloc(con->imap.user->messages))) {
			while ((active = inx_cursor_value_next(cursor))) {
				if (active->foldernum == con->imap.selected && (active->status & MAIL_STATUS_DELETED) == MAIL_STATUS_DELETED) {
					imap_message_expunge(con, active);
				}
			}
			inx_cursor_free(cursor);
		}

		// Update all of the sequences at once.
		meta_messages_update_sequences(con->imap.user->folders, con->imap.user->messages);

		// If the serial number indicates no outside changes we can increment it without forcing a refresh.
		if (con->imap.user->serials.messages == serial_get(OBJECT_MESSAGES, con->imap.user->usernum)) {
			con->imap.messages_checkpoint = con->imap.user->serials.messages = serial_increment(OBJECT_MESSAGES, con->imap.user->usernum);
		}
		// The context is already due for a refresh, but we increment the serial to let the rest of the cluster know about the change.
		else {
			serial_increment(OBJECT_MESSAGES, con->imap.user->usernum);
		}

		user_unlock(con->imap.user->usernum);
		meta_user_unlock(con->imap.user);
	}

	// If a folder was previously selected, clear the recent flag before closing the mailbox.
	if (recent == 1) {
		meta_user_wlock(con->imap.user);
		if ((cursor = inx_cursor_alloc(con->imap.user->messages))) {
			while ((active = inx_cursor_value_next(cursor))) {
				if (active->foldernum == con->imap.selected && (active->status & MAIL_STATUS_RECENT) == MAIL_STATUS_RECENT) {
					active->status = (active->status | MAIL_STATUS_RECENT) ^ MAIL_STATUS_RECENT;
				}
			}
			inx_cursor_free(cursor);
		}
		meta_user_unlock(con->imap.user);
	}

	con->imap.read_only = con->imap.selected = con->imap.messages_total = con->imap.messages_recent = 0;
	con_print(con, "%.*s OK CLOSE complete.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	return;
}

void imap_expunge(connection_t *con) {

	int_t deleted = 0;
	inx_cursor_t *cursor;
	meta_message_t *active;
	uint64_t sequencenum, expunged = 0;

	// Check for the right state.
	if (con->imap.session_state != 1) {
		con_print(con, "%.*s BAD The EXPUNGE command is not available until you are authenticated.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}
	else if (con->imap.selected == 0) {
		con_print(con, "%.*s BAD The EXPUNGE command is not available until you have selected a folder.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}
	else if (con->imap.read_only == 1) {
		con_print(con, "%.*s BAD The EXPUNGE command is not available while in read only mode.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}

	// Input validation. Requires no arguments.
	if (con->imap.arguments != NULL && ar_length_get(con->imap.arguments) != 0) {
		con_print(con, "%.*s BAD The EXPUNGE command does not take any arguments.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}

	// Scan the mailbox, and see if any messages need to be deleted.
	meta_user_rlock(con->imap.user);
	if ((cursor = inx_cursor_alloc(con->imap.user->messages))) {
		while ((active = inx_cursor_value_next(cursor)) && deleted == 0) {
			if (active->foldernum == con->imap.selected && (active->status & MAIL_STATUS_DELETED) == MAIL_STATUS_DELETED) {
				deleted = 1;
			}
		}
		inx_cursor_free(cursor);
	}
	meta_user_unlock(con->imap.user);

	// If messages are deleted, get a global lock.
	if (deleted == 1) {

		meta_user_wlock(con->imap.user);

		// Don't expunge while POP session is active. We don't make this check until we have a write lock, just to prevent any
		// POP sessions from starting between when we startup and when we finish.
		if (con->imap.user->refs.pop > 0) {
			meta_user_unlock(con->imap.user);
			con_print(con, "%.*s NO The mailbox is locked by a POP session. Please try again later.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
			con->imap.read_only = con->imap.selected = con->imap.messages_total = con->imap.messages_recent = 0;
			return;
		}

		// Lock the account.
		if (user_lock(con->imap.user->usernum) != 1) {
			meta_user_unlock(con->imap.user);
			con_print(con, "%.*s NO The EXPUNGE command could not lock the user account. Try again later.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
			return;
		}

		// Loop through and perform the deletes.
		if ((cursor = inx_cursor_alloc(con->imap.user->messages))) {
			while ((active = inx_cursor_value_next(cursor))) {
				if (active->foldernum == con->imap.selected && (active->status & MAIL_STATUS_DELETED) == MAIL_STATUS_DELETED) {
					sequencenum = active->sequencenum;
					if (imap_message_expunge(con, active) != 0) {
						con_print(con, "* %lu EXPUNGE\r\n", sequencenum - expunged++);
					}
				}
			}
			inx_cursor_free(cursor);
		}

		// Update all of the sequences at once.
		meta_messages_update_sequences(con->imap.user->folders, con->imap.user->messages);

		// If the serial number indicates no outside changes we can increment it without forcing a refresh.
		if (con->imap.user->serials.messages == serial_get(OBJECT_MESSAGES, con->imap.user->usernum)) {
			con->imap.messages_checkpoint = con->imap.user->serials.messages = serial_increment(OBJECT_MESSAGES, con->imap.user->usernum);
		}
		// The context is already due for a refresh, but we increment the serial to let the rest of the cluster know about the change.
		else {
			serial_increment(OBJECT_MESSAGES, con->imap.user->usernum);
		}

		user_unlock(con->imap.user->usernum);
		meta_user_unlock(con->imap.user);

		// Output the new status.
		imap_session_update(con);

		// Relay the new folder status to the client.
		con_print(con, "* %lu EXISTS\r\n* %lu RECENT\r\n", con->imap.messages_total, con->imap.messages_recent);
	}

	con_print(con, "%.*s OK Expunge complete.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	return;
}

void imap_copy(connection_t *con) {

	inx_t *messages;
	inx_cursor_t *cursor;
	meta_folder_t *folder;
	meta_message_t *active;
	size_t nodes, count = 0;
	stringer_t *source_range = NULL, *dest_range = NULL;
	uint64_t *source_uids = NULL, *dest_uids = NULL, outnum = 0, recent = 0, exists = 0;;

	// Check for the right state.
	if (con->imap.session_state != 1) {
		con_print(con, "%.*s BAD The COPY command is not available until you are authenticated.\r\n", st_length_int(con->imap.tag),
			st_char_get(con->imap.tag));
		return;
	}
	else if (con->imap.selected == 0) {
		con_print(con, "%.*s BAD The COPY command is not available until you have selected a folder.\r\n", st_length_int(con->imap.tag),
			st_char_get(con->imap.tag));
		return;
	}

	// Input validation. Requires two string arguments.
	else if (ar_length_get(con->imap.arguments) != 2 || imap_get_type_ar(con->imap.arguments, 0) == IMAP_ARGUMENT_TYPE_ARRAY ||
		imap_get_type_ar(con->imap.arguments, 1) == IMAP_ARGUMENT_TYPE_ARRAY) {
		con_print(con, "%.*s BAD The COPY command requires two string arguments.\r\n", st_length_int(con->imap.tag),
			st_char_get(con->imap.tag));
		return;
	}

	meta_user_wlock(con->imap.user);

	// Pull the folder structure.
	if ((folder = meta_folders_by_name(con->imap.user->folders, imap_get_st_ar(con->imap.arguments, 1))) == NULL) {
		meta_user_unlock(con->imap.user);
		con_print(con, "%.*s NO [TRYCREATE] Unable to find the requested target folder.\r\n", st_length_int(con->imap.tag),
			st_char_get(con->imap.tag));
		return;
	}

	// Make sure were not copying to the same folder.
	else if (folder->foldernum == con->imap.selected) {
		meta_user_unlock(con->imap.user);
		con_print(con, "%.*s NO The target folder must be different than the selected folder.\r\n", st_length_int(con->imap.tag),
			st_char_get(con->imap.tag));
		return;
	}

	// Make sure its a valid sequence number.
	else if (imap_valid_sequence(imap_get_st_ar(con->imap.arguments, 0)) != 1) {
		meta_user_unlock(con->imap.user);
		con_print(con, "%.*s NO An invalid sequence was provided to the store command.\r\n", st_length_int(con->imap.tag),
			st_char_get(con->imap.tag));
		return;
	}

	// Narrow by the sequence range provided.
	// Due to bugs in several clients, invalid sequences may be submitted. Return an okay if the sequence isn't found so the client doesn't hang.
	else if (con->imap.user->messages == NULL || (messages = imap_narrow_messages(con->imap.user->messages, con->imap.selected, imap_get_st_ar(con->imap.arguments, 0), con->imap.uid)) == NULL) {
		meta_user_unlock(con->imap.user);
		con_print(con, "%.*s OK No messages were found matching the range provided.\r\n", st_length_int(con->imap.tag),
			st_char_get(con->imap.tag));
		return;
	}

	// Figure out how many nodes.
	else if ((nodes = inx_count(messages)) == 0) {
		meta_user_unlock(con->imap.user);
		con_print(con, "%.*s OK No messages were found matching the range provided.\r\n", st_length_int(con->imap.tag),
			st_char_get(con->imap.tag));
		inx_free(messages);
		return;
	}

	// Setup the UID buffers.
	else if ((source_uids = mm_alloc(nodes * sizeof(uint64_t))) == NULL) {
		meta_user_unlock(con->imap.user);
		con_print(con, "%.*s NO Internal server error. Please try again later.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		inx_free(messages);
		return;
	}
	else if ((dest_uids = mm_alloc(nodes * sizeof(uint64_t))) == NULL) {
		meta_user_unlock(con->imap.user);
		con_print(con, "%.*s NO Internal server error. Please try again later.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		inx_free(messages);
		mm_free(source_uids);
		return;
	}

	// Lock the account.
	if (user_lock(con->imap.user->usernum) != 1) {
		log_pedantic("Could not lock the user account %lu.", con->imap.user->usernum);
		meta_user_unlock(con->imap.user);
		con_print(con, "%.*s NO The COPY command could not lock the user account. Try again later.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		inx_free(messages);
		mm_free(source_uids);
		mm_free(dest_uids);
		return;
	}

	// Loop through and output each message.
	if ((cursor = inx_cursor_alloc(messages))) {
		while ((active = inx_cursor_value_next(cursor))) {
			if (imap_message_copier(con, active, folder->foldernum, &outnum) != 1) {
				con_print(con, "%.*s NO Copy failed on message number %lu.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag), active->sequencenum);
				meta_user_unlock(con->imap.user);
				user_unlock(con->imap.user->usernum);
				inx_free(messages);
				mm_free(source_uids);
				mm_free(dest_uids);
				return;
			}
			else {
				*(source_uids + count) = active->messagenum;
				*(dest_uids + count) = outnum;
				count++;
			}
		}
		inx_cursor_free(cursor);
	}

	// Build the new status.
	if ((cursor = inx_cursor_alloc(con->imap.user->messages))) {
		while ((active = inx_cursor_value_next(cursor))) {
			if (active->foldernum == con->imap.selected && (active->status & MAIL_STATUS_RECENT) == MAIL_STATUS_RECENT) {
				recent++;
				exists++;
			}
			else if (active->foldernum == con->imap.selected) {
				exists++;
			}
		}
		inx_cursor_free(cursor);
	}

	con_print(con, "* %lu EXISTS\r\n* %lu RECENT\r\n", exists, recent);

	// Store the current serial number and folder status.
	con->imap.messages_checkpoint = con->imap.user->serials.messages;
	con->imap.messages_recent = recent;
	con->imap.messages_total = exists;

	// Get the ranges.
	if ((source_range = imap_range_build(count, source_uids)) == NULL || (dest_range = imap_range_build(count, dest_uids)) == NULL) {
		con_print(con, "%.*s OK Copy complete.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}
	else {
		con_print(con, "%.*s OK [COPYUID %lu %.*s %.*s] Copy complete.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag), folder->foldernum,
			st_length_get(source_range), st_char_get(source_range), st_length_get(dest_range), st_char_get(dest_range));
	}

	// Cleanup
	user_unlock(con->imap.user->usernum);
	meta_user_unlock(con->imap.user);
	inx_free(messages);
	mm_free(source_uids);
	mm_free(dest_uids);
	if (source_range != NULL) {
		mm_free(source_range);
	}
	if (dest_range != NULL) {
		mm_free(dest_range);
	}
	return;
}

void imap_append(connection_t *con) {

	meta_folder_t *folder;
	inx_cursor_t *cursor;
	meta_message_t *active;
	uint32_t flags = MAIL_STATUS_EMPTY;
	uint64_t recent = 0, exists = 0, outnum = 0;

	if (con->imap.session_state != 1) {
		con_print(con, "%.*s BAD The APPEND command is not available until you are authenticated.\r\n", st_length_int(con->imap.tag),
			st_char_get(con->imap.tag));
		return;
	}

	// Input validation. Requires at least two arguments, and maximum of four. The first argument must be a string, and the last argument must be a literal.
	if (ar_length_get(con->imap.arguments) < 2 || ar_length_get(con->imap.arguments) > 4 || imap_get_type_ar(con->imap.arguments, 0) == IMAP_ARGUMENT_TYPE_ARRAY ||
		imap_get_type_ar(con->imap.arguments, ar_length_get(con->imap.arguments) - 1) != IMAP_ARGUMENT_TYPE_LITERAL) {

		con_print(con, "%.*s NO The APPEND command must have between two and four arguments. The first argument must be a string "
			"and the last argument must be a literal.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		log_pedantic("Invalid APPEND parameters.");
		return;
	}

	// Over quota.
	/// BUG: If the user is over their quota check whether rollout is enabled and if so. If enabled make room for the appended message using the rollout logic.
	if ((con->imap.user->flags & META_USER_OVERQUOTA) == META_USER_OVERQUOTA) {
		con_print(con, "%.*s NO This user account is over its quota. Try deleting older messages to free up space.\r\n", st_length_int(con->imap.tag),
			st_char_get(con->imap.tag));
		return;
	}

	// Parse the list of flags.
	if (ar_length_get(con->imap.arguments) > 2) {
		flags = imap_flag_parse(imap_get_ptr(con->imap.arguments, 1), imap_get_type_ar(con->imap.arguments, 1));
	}

	meta_user_wlock(con->imap.user);

	// Pull the folder structure.
	if ((folder = meta_folders_by_name(con->imap.user->folders, imap_get_st_ar(con->imap.arguments, 0))) == NULL) {
		meta_user_unlock(con->imap.user);
		con_print(con, "%.*s NO [TRYCREATE] Unable to find the requested target folder.\r\n", st_length_int(con->imap.tag),
			st_char_get(con->imap.tag));
		return;
	}

	if (imap_append_message(con, folder, flags, imap_get_st_ar(con->imap.arguments, ar_length_get(con->imap.arguments) - 1), &outnum) != 1) {
		meta_user_unlock(con->imap.user);
		con_print(con, "%.*s NO Unable to APPEND the message provided. Please try again later.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}

	// The message appended to the current folder, so we need to output the current status.
	if (con->imap.selected == folder->foldernum) {

		if ((cursor = inx_cursor_alloc(con->imap.user->messages))) {
			while ((active = inx_cursor_value_next(cursor))) {
				if (active->foldernum == con->imap.selected && (active->status & MAIL_STATUS_RECENT) == MAIL_STATUS_RECENT) {
					recent++;
					exists++;
				}
				else if (active->foldernum == con->imap.selected) {
					exists++;
				}
			}
			inx_cursor_free(cursor);
		}

		con_print(con, "* %lu EXISTS\r\n* %lu RECENT\r\n", exists, recent);

		// Store the current serial number and folder status.
		con->imap.messages_checkpoint = con->imap.user->serials.messages;
		con->imap.messages_recent = recent;
		con->imap.messages_total = exists;
	}

	// Stash the folder number so we can release the session lock before making the final network print.
	recent = folder->foldernum;
	meta_user_unlock(con->imap.user);

	con_print(con, "%.*s OK [APPENDUID %lu %lu] Append complete.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag), recent, outnum);
	return;
}

void imap_fetch(connection_t *con) {

	int_t space = 0;
	inx_cursor_t *cursor;
	meta_message_t *active;
	inx_t *messages, *duplicate;
	imap_fetch_dataitems_t *items;
	imap_fetch_response_t *response, *iterate;

	// Check for the right state.
	if (con->imap.session_state != 1) {
		con_print(con, "%.*s BAD The fetch command is not available until you are authenticated.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}
	else if (con->imap.selected == 0) {
		con_print(con, "%.*s BAD The FETCH command is not available until you have selected a folder.\r\n", st_length_int(con->imap.tag), 	st_char_get(con->imap.tag));
		return;
	}

	// Input validation. Requires two arguments.
	if (ar_length_get(con->imap.arguments) < 2 || imap_get_type_ar(con->imap.arguments, 0) == IMAP_ARGUMENT_TYPE_ARRAY) {
		con_print(con, "%.*s BAD The fetch command requires two arguments.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}

	// Make sure its a valid sequence number.
	if (imap_valid_sequence(imap_get_st_ar(con->imap.arguments, 0)) != 1) {
		con_print(con, "%.*s BAD An invalid sequence was provided to the fetch command.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}

	// Parse the data items this command should be displaying.
	if ((items = imap_parse_dataitems(con->imap.arguments)) == NULL) {
		con_print(con, "%.*s BAD An invalid list of items was requested.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}

	// If were going to be updating flags, get a write lock.
	if (con->imap.read_only == 0 && (items->normal != NULL || items->rfc822 == 1 || items->rfc822_header == 1 || items->rfc822_text == 1)) {
		meta_user_wlock(con->imap.user);
	}
	else {
		meta_user_rlock(con->imap.user);
	}

	// Narrow by the sequence range provided.
	if (con->imap.user->messages == NULL || (messages = imap_narrow_messages(con->imap.user->messages, con->imap.selected, imap_get_st_ar(con->imap.arguments, 0), con->imap.uid)) == NULL) {
		meta_user_unlock(con->imap.user);
		con_print(con, "%.*s OK Fetch complete. No messages were found matching the range provided.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		imap_fetch_free_items(items);
		return;
	}

	// If RFC822, RFC822.TEXT or any BODY[] items are requested, add the seen flag.
	if (con->imap.read_only == 0 && (items->normal != NULL || items->rfc822 == 1 || items->rfc822_text == 1)) {
		meta_data_flags_add(messages, con->imap.user->usernum, con->imap.selected, MAIL_STATUS_SEEN);
		if ((cursor = inx_cursor_alloc(messages))) {
			while ((active = inx_cursor_value_next(cursor))) {
				if ((active->status & MAIL_STATUS_SEEN) != MAIL_STATUS_SEEN) {
					active->status = (active->status | MAIL_STATUS_SEEN);
					active->updated = 1;
				}
			}

			inx_cursor_reset(cursor);
			while ((active = inx_cursor_value_next(cursor))) {
				active->updated = 0;
			}

			inx_cursor_free(cursor);
		}

		// If the serial number indicates no outside changes we can increment it without forcing a refresh.
		if (con->imap.user->serials.messages == serial_get(OBJECT_MESSAGES, con->imap.user->usernum)) {
			con->imap.messages_checkpoint = con->imap.user->serials.messages = serial_increment(OBJECT_MESSAGES, con->imap.user->usernum);
		}
		// The context is already due for a refresh, but we increment the serial to let the rest of the cluster know about the change.
		else {
			serial_increment(OBJECT_MESSAGES, con->imap.user->usernum);
		}
	}

	// Create a deep copy so we can unlock the mailbox during the fetch.
	duplicate = imap_duplicate_messages(messages);
	inx_free(messages);
	meta_user_unlock(con->imap.user);

	// Loop through and output each message.
	if ((cursor = inx_cursor_alloc(duplicate))) {
		while (status() && con_status(con) >= 0 && (active = inx_cursor_value_next(cursor))) {

			// Fetch the data.
			iterate = response = imap_fetch_message(con, active, items);
			space = 0;

			// Output the response.
			con_print(con, "* %lu FETCH (", active->sequencenum);

			// Output the UID first.
			while (iterate != NULL) {

				if (!st_cmp_cs_eq(iterate->key, PLACER("UID", 3))) {
					con_write_st(con, iterate->key);
					con_write_bl(con, " ", 1);
					con_write_st(con, iterate->value);
					iterate = NULL;
					space = 1;
				}
				else {
					iterate = (imap_fetch_response_t *)iterate->next;
				}
			}

			// Now output the size, if its present.
			iterate = response;
			while (iterate != NULL) {
				if (!st_cmp_cs_eq(iterate->key, PLACER("RFC822.SIZE", 11))) {
					if (space == 1) {
						con_write_bl(con, " ", 1);
					}
					else {
						space = 1;
					}
					con_write_st(con, iterate->key);
					con_write_bl(con, " ", 1);
					con_write_st(con, iterate->value);
					iterate = NULL;
				}
				else {
					iterate = (imap_fetch_response_t *)iterate->next;
				}
			}

			// Output the rest of the items.
			iterate = response;
			while (iterate != NULL) {
				if (st_cmp_cs_eq(iterate->key, PLACER("UID", 3)) && st_cmp_cs_eq(iterate->key, PLACER("RFC822.SIZE", 11))) {
					if (space == 1) {
						con_write_bl(con, " ", 1);
					}
					else {
						space = 1;
					}
					con_write_st(con, iterate->key);
					con_write_bl(con, " ", 1);
					con_write_st(con, iterate->value);
				}
				iterate = (imap_fetch_response_t *)iterate->next;
			}

			// Free the response and advance.
			imap_fetch_response_free(response);
			con_write_bl(con, ")\r\n", 3);
		}

		inx_cursor_free(cursor);
	}

	con_print(con, "%.*s OK Fetch complete.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	imap_fetch_free_items(items);
	inx_free(duplicate);

	return;
}

void imap_search(connection_t *con) {

	inx_t *messages;
	inx_cursor_t *cursor;
	meta_message_t *active;
	stringer_t *output = NULL, *buffer = MANAGEDBUF(128);

	// Check for the right state.
	if (con->imap.session_state != 1) {
		con_print(con, "%.*s BAD The SEARCH command is not available until you are authenticated.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}
	else if (con->imap.selected == 0) {
		con_print(con, "%.*s BAD The SEARCH command is not available until you have selected a folder.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}

	// Input validation. Requires two arguments.
	if (ar_length_get(con->imap.arguments) == 0) {
		con_print(con, "%.*s BAD The SEARCH command requires at least one argument.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}

	// Perform the search. The internal search functions will lock the session as necessary.
	messages = imap_search_messages(con);

	if ((output = st_aprint_opts(MANAGED_T | HEAP | JOINTED, "* SEARCH")) && (cursor = inx_cursor_alloc(messages))) {

		while (status() && con_status(con) >= 0 && (active = inx_cursor_value_next(cursor))) {

			st_sprint(buffer, " %lu", con->imap.uid ? active->messagenum : active->sequencenum);
			st_append_opts(1024, output, buffer);

			if (st_length_get(output) > 16536) {
				con_write_st(con, output);
				st_length_set(output, 0);
			}
		}
	}

	if (output && st_length_get(output)) {
		con_write_st(con, output);
	}

	con_print(con, "\r\n%.*s OK Search complete.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));

	if (output) st_free(output);
	inx_free(messages);
	return;
}

void imap_idle(connection_t *con) {

	if (con->imap.session_state != 1) {
		con_print(con, "%.*s BAD The IDLE command is not available until you are authenticated.\r\n", st_length_int(con->imap.tag),
			st_char_get(con->imap.tag));
	}
	else {
		con_print(con, "%.*s NO IDLE Failed. The IDLE command is not currently supported.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
	}
	return;
}

/***
 * The ID command is described by RFC 2971 and allows clients to submit information about themselves and servers to supply similar information.
 * According to section 3.3: "Field strings MUST NOT be longer than 30 octets. Value strings MUST NOT be longer than 1024 octets. Implementations "
 * MUST NOT send more than 30 field-value pairs."
 */
void imap_id(connection_t *con) {

	int_t limit;
	size_t count = 0;
	stringer_t *client = NULL, *current, *cleaned = MANAGEDBUF(256);
	imap_arguments_t *pairs = NULL;

	// We use the pair pointer to abstract away whether the values were supplied inside an array.
	if (con->imap.arguments
		&& ((imap_get_type_ar(con->imap.arguments, 0) == IMAP_ARGUMENT_TYPE_ARRAY && (pairs = imap_get_ar_ar(con->imap.arguments, 0))) ||
		(imap_get_type_ar(con->imap.arguments, 0) != IMAP_ARGUMENT_TYPE_ARRAY && (pairs = con->imap.arguments))) &&
		(count = ar_length_get(pairs))) {

		for (size_t i = 0; i < count; i++) {

			// If the argument was a string, try to clean it up before serializing the data into a single line suitable for the log.
			if (imap_get_type_ar(pairs, i) != IMAP_ARGUMENT_TYPE_ARRAY && (current = imap_get_st_ar(pairs, i)) && st_length_get(client) < 8192) {

				// Name fields are limited to 30 characters which conforms to the RFC imposed limit, and to keep things readable
				// we trim data fields to 128 characters.
				limit = i % 2 ? 30 : 128;

				if (st_sprint(cleaned, "%s\"%.*s%s\"", client ? " " : "", st_length_int(current) > limit ? limit : st_length_int(current),
					st_length_int(current) > limit ? "..." : "", st_char_get(current)) > 0) {
					client = st_append_opts(8192, client, cleaned);
				}
			}
		}

		if (client) {
			log_info("[IMAP ID %s] %s (%.*s)", st_char_get(time_print_local(MANAGEDBUF(128), "%Y-%m-%d %T %Z", time(NULL))),
				st_char_get(con_addr_presentation(con, MANAGEDBUF(128))), st_length_int(client), st_char_get(client));
			st_free(client);
		}
	}

	con_print(con, "* ID (\"name\" \"Magma\" \"version\" \"%s\" \"build\" \"%s\" \"vendor\" \"Lavabit\" \"support-url\" \"https://lavabit.com/\")\r\n"
		"%.*s OK Completed.\r\n", build_version(), build_stamp(), st_length_int(con->imap.tag), st_char_get(con->imap.tag));

	return;
}

/**
 * @brief	Display the capability string for the IMAP server.
 *
 * @note	This string always needs to stay in sync with the banner greeting.
 *
 * @return	This function returns no value.
 */
void imap_capability(connection_t *con) {

	if (con->imap.arguments) {
		con_print(con, "%.*s BAD The capability command does not accept any arguments.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		return;
	}

	// STARTTLS should only appear if the server instance has been configured with an TLS certificate. The connection must also be pre-authentication and unencrypted.
	con_print(con, "* CAPABILITY IMAP4 IMAP4rev1%sLITERAL+ ID\r\n%.*s OK Completed.\r\n", con_secure(con) == 0 && con->imap.session_state == 0 ?
		" STARTTLS " : " ",	st_length_int(con->imap.tag), st_char_get(con->imap.tag));

	return;
}

/**
 * @brief	The main imap entry point for all inbound client connections, as dispatched by the generic protocol handler (display
 * 			banner greeting).
 *
 * @param	con		a pointer to the connection object of the newly connected client.
 *
 * @return	This function returns no value.
 */
void imap_init(connection_t *con) {

	con_reverse_enqueue(con);

	// Introduce ourselves. Note the string below needs to stay in sync with the capability command.
	con_print(con, "* OK [CAPABILITY IMAP4 IMAP4rev1%sLITERAL+ ID]%s%.*s%sMagma IMAP server v%s is ready.\r\n",
		con_secure(con) == 0 ? " STARTTLS " : " ", st_length_get(con->server->domain) ? " " : "", st_length_int(con->server->domain),
		st_char_get(con->server->domain), st_length_get(con->server->domain) ? " " : "", build_version());

	imap_requeue(con);

	return;
}
