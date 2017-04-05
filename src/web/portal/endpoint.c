
/**
 * @file /magma/web/portal/endpoint.c
 *
 * @brief	The control logic for the Portal JSON endpoint.
 */

#include "magma.h"
#include "methods.h"

/// TODO: We use session locks to synchronize access to the user context. Should we also be using mailbox cluster locks? Cluster level locking
/// might be appropriate for operations that delete data likes folders.remove and messages.remove.

// TODO: make sure that all 500 internal errors decrement json reference counts before they return
// TODO: All json reference counts need to be checked for leaks. Do we need to release before we print a portal response, etc?

/**
 * @brief	Sort the web portal JSON command table to be ready for binary searches.
 * @return	This function returns no value.
 */
void portal_endpoint_sort(void) {

	qsort(portal_methods, sizeof(portal_methods) / sizeof(portal_methods[0]), sizeof(command_t), &portal_endpoint_compare);
	return;
}

/**
 * @brief	Internal bsearch() comparison function for dispatching the proper portal handler.
 * @param	compare		a pointer to a command_t object with a portal method name for string comparison.
 * @param	command		a pointer to a command_t object with a portal method name for string comparison.
 *
 *
 *
 * @return
 */
int_t portal_endpoint_compare(const void *compare, const void *command) {

	int_t result;
	command_t *cmd = (command_t *)command, *cmp = (command_t *)compare;

	/*if (!cmp->function) {
		// QUESTION: Why did we use this old comparison? It fails for messages.tag/messages.tags ...
		result = st_cmp_ci_starts(PLACER(cmp->string, cmp->length), PLACER(cmd->string, cmd->length));
	}
	else { */
		result = st_cmp_ci_eq(PLACER(cmp->string, cmp->length), PLACER(cmd->string, cmd->length));
	//}

	return result;
}

/**
 * @brief	Verify that a session underlying a portal request has been authenticated.
 * @note	If the session is not authenticated, an error message will be sent to the client, so the caller only needs to return from its
 * 			function if this function returns false.
 * @param	con			a pointer to the connection object across which the portal request was made.
 * @param	err_mask	an optional error bitmask to be combined with any portal authentication error code.
 * @param	method		a pointer to a null-terminated string that will be used in an optional error logging message if it is not NULL.
 * @param	has_params	if true, return an error if the authenticated portal session supplied no method parameters;
 * 						if false, return an error if the authenticated portal supplies params and they're not empty.
 * @param	nparams		if non-zero and has_params is also set, return an error if the portal session's parameters count does not match this value.
 * @return
 */
bool_t portal_validate_request (connection_t *con, int err_mask, chr_t *method, bool_t has_params, size_t nparams) {

	// The client must have an authenticated session to make any portal request.
	if (!con->http.session || con->http.session->state != SESSION_STATE_AUTHENTICATED) {

		if (method) {
			log_pedantic("User requested portal method \"%s\" without an authenticated session { sess = %lx, state = %lx }",
				method, (unsigned long)con->http.session, (unsigned long)(con->http.session ? (unsigned long)con->http.session->state : (unsigned long)con->http.session));
		} else {
			log_pedantic("User requested portal method without an authenticated session. { sess = %lx, state = %lx }",
				(unsigned long)con->http.session, (unsigned long)(con->http.session ? (unsigned long)con->http.session->state : (unsigned long)con->http.session));
		}

		portal_endpoint_error(con, 403, PORTAL_ENDPOINT_ERROR_MODE | err_mask, "This method requires authentication first.");
		return false;
	}

	// If the portal method isn't supposed to have any parameters, make sure there aren't any.
	else if (!has_params && con->http.portal.params && json_object_size_d(con->http.portal.params)) {

		if (method) {
			log_pedantic("User supplied parameters to portal method which accepts none { method = %s, user = %.*s }",
				method, (int)st_length_get(con->http.session->user->username), st_char_get(con->http.session->user->username));
		}

		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return false;
	}
	else if (has_params && (!con->http.portal.params || !json_is_object(con->http.portal.params))) {

		if (method) {
			log_pedantic("User supplied no parameters to portal method which required them { method = %s, user = %.*s}",
				method, (int)st_length_get(con->http.session->user->username), st_char_get(con->http.session->user->username));
		}

		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return false;
	} else if (has_params && nparams && (json_object_size_d(con->http.portal.params) != nparams)) {

		if (method) {
			log_pedantic("User supplied an incorrect number of parameters to portal method { method = %s, required = %u, sent = %u, user = %.*s }",
				method, (unsigned int)nparams, (unsigned int)json_object_size_d(con->http.portal.params), (int)st_length_get(con->http.session->user->username),
				st_char_get(con->http.session->user->username));
		}

		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return false;
	}

	return true;
}

/**
 * @brief	Return a json-rpc error response to the remote client.
 * @param	con			a pointer to the connection object across which the response will be sent.
 * @param	http_code	the http response code to be sent to the remote client in the response header.
 * @param	error_code	the numerical error code to be encoded in the json-rpc error message.
 * @param	message		a descriptive error string to be encoded in the json-rpc error message.
 * @return	This function returns no value.
 */
void portal_endpoint_error(connection_t *con, int_t http_code, int_t error_code, chr_t *message) {

	json_error_t err;
	json_t *object = NULL;
	chr_t *response = NULL;

	// If we've encountered an internal issue, we probably want to log it.
	if (http_code == HTTP_ERROR_500) {
		log_options(M_LOG_CRITICAL | M_LOG_STACK_TRACE, "Portal encountered an internal error: {msg = %s}", message);
	}

	if ((object = json_pack_ex_d(&err, JSON_ENSURE_ASCII, "{s:s, s:{s:i, s:s}, s:I}", "jsonrpc", "2.0", "error", "code", error_code, "message",
		message, "id", con->http.portal.id)) && (response = json_dumps_d(object, JSON_PRESERVE_ORDER | (magma.web.portal.indent ? JSON_INDENT(4) : JSON_COMPACT)))) {
		if (magma.web.portal.indent) response = ns_append(response, "\r\n");
		http_response_header(con, http_code, PLACER("application/json; charset=utf-8", 31), ns_length_get(response));
		con_write_st(con, NULLER(response));
	}
	else {
		if (object) log_pedantic("Unable to generate a proper JSON error response. { response = NULL / error = %s }", err.text);
		else log_pedantic("Unable to generate a proper JSON error response. { object = NULL / error = %s }", err.text);
		con->http.mode = HTTP_ERROR_500;
	}

	if (object) json_decref_d (object);
	if (response) ns_free(response);
	return;
}

/**
 * @brief	Generate a json-rpc 2.0 response to a portal request.
 * @see		json_vpack_ex()
 * @note	This function indents the json response if specified in the configuration, and also automatically decreases the reference
 * 			count of any json object that was packed for the reply.
 * @param	con		a pointer to the connection object across which the portal response will be sent.
 * @param	format	a pointer to a format string specifying the construction of the json-rpc response.
 * @param	...		a variable arguments style list of parameters to be passed to the json packing function.
 * @return	This function returns no value.
 */
void portal_endpoint_response(connection_t *con, chr_t *format, ...) {

	va_list args;
	json_error_t err;
	json_t *object = NULL;
	chr_t *response = NULL;

	va_start(args, format);

	if ((object = json_vpack_ex_d(&err, 0, format, args)) && (response = json_dumps_d(object, JSON_PRESERVE_ORDER | (magma.web.portal.indent ? JSON_INDENT(4) : JSON_COMPACT)))) {
		if (magma.web.portal.indent) response = ns_append(response, "\r\n");
		http_response_header(con, 200, PLACER("application/json; charset=utf-8", 31), ns_length_get(response));
		con_write_st(con, NULLER(response));
	}
	else {
		if (object) log_pedantic("Unable to generate a proper JSON response. { response = NULL / error = %s }", err.text);
		else log_pedantic("Unable to generate a proper JSON response. { object = NULL / error = %s }", err.text);
		con->http.mode = HTTP_ERROR_500;
	}

	if (object) json_decref_d (object);
	if (response) ns_free(response);
	va_end(args);

	return;
}

/**
 * @brief	Obtain credentials and log a user into portal session in response to an "auth" json-rpc portal request.
 * @param	con		a pointer to the connection object of the user attempting to log in.
 * @return	This function returns no value.
 */
void portal_endpoint_auth(connection_t *con) {

	int_t state;
	json_error_t err;
	uint64_t fails = 0;
	auth_t *auth = NULL;
	meta_user_t *user = NULL;
	chr_t *username = NULL, *password = NULL;
	stringer_t *subnet = NULL, *key = NULL;

	// Check the session state.
	if (!con->http.session || con->http.session->state != SESSION_STATE_NEUTRAL) {
		portal_endpoint_error(con, 403, PORTAL_ENDPOINT_ERROR_MODE | PORTAL_ENDPOINT_ERROR_AUTH, "The auth method is unavailable after a successful login.");
		return;
	}

	// Store the subnet for tracking login failures. Make the buffer big enough to hold an IPv6 subnet string.
	subnet = con_addr_subnet(con, MANAGEDBUF(256));

	// Generate the invalid login tracker.
	key = st_quick(MANAGEDBUF(384), "magma.logins.invalid.%lu.%.*s", time_datestamp(), st_length_int(subnet), st_char_get(subnet));

	// For now we hard code the maximum number of failed logins.
	if (st_populated(key) && cache_increment(key, 0, 0, 86400) >= 16) {
		portal_endpoint_error(con, 403, PORTAL_ENDPOINT_ERROR_MODE | PORTAL_ENDPOINT_ERROR_AUTH, "The maximum number of failed login attempts has been reached. Please try again later.");
		con->protocol.violations++;
		return;
	}

	// Validate the request format and extract the submitted values.
	else if ((json_unpack_ex_d(con->http.portal.params, &err, JSON_STRICT, "{s:s, s:s}", "username", &username, "password", &password))) {
		log_pedantic("Received invalid portal auth request parameters { user = %s, errmsg = %s }", st_char_get(con->http.session->user->username), err.text);
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	}

/*	// Pull the user info out.
	if ((state = meta_get(auth->usernum, auth->username, auth->keys.master, auth->tokens.verification, META_PROTOCOL_IMAP,
		META_GET_MESSAGES | META_GET_FOLDERS, &(con->imap.user)))) {

		// The UNAVAILABLE response code is provided by RFC 5530 which states: "Temporary failure because a subsystem is down."
		if (state < 0) {
			con_print(con, "%.*s NO [UNAVAILABLE] This server is unable to access your mailbox. Please try again later.\r\n",
				st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		}
		else {
			con_print(con,  "%.*s NO [AUTHENTICATIONFAILED] The username and password combination is invalid.\r\n",
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

*/

	// Convert the strings into a full fledged authentication object.
	if ((state = auth_login(NULLER(username), NULLER(password), &auth))) {

		if (state < 0) {
			portal_endpoint_error(con, 200, PORTAL_ENDPOINT_ERROR_AUTH, "This server is unable to access your mailbox. Please try again later.");
		}
		else {
			portal_endpoint_response(con, "{s:s, s:{s:s, s:s}, s:I}", "jsonrpc", "2.0", "result", "auth", "failed", "message",
				"The username and password provided are incorrect, please try again.", "id", con->http.portal.id);
		}

		log_info("Failed login attempt. { ip = %s / username = %s / protocol = HTTP }", st_char_get(con_addr_presentation(con, MANAGEDBUF(256))),
			username);

		// If we have a valid key, we increment the failed login counter.
		if (st_populated(key) && (fails = cache_increment(key, 1, 1, 86400)) >= 16) {
			log_info("Subnet banned. { subnet = %s / fails = %lu }", st_char_get(con_addr_subnet(con, MANAGEDBUF(256))), fails);
		}

		con->protocol.violations++;
		return;
	}

	// Locks
	if (auth->status.locked) {

		if (auth->status.locked == 1) {
			portal_endpoint_response(con, "{s:s, s:{s:s, s:s}, s:I}", "jsonrpc", "2.0", "result", "auth", "locked", "admin", "message",
				"This account has been administratively locked.", "id", con->http.portal.id);
		}
		else if (auth->status.locked == 2) {
			portal_endpoint_response(con, "{s:s, s:{s:s, s:s}, s:I}", "jsonrpc", "2.0", "result", "auth", "locked", "inactivity", "message",
				"This account has been locked for inactivity.", "id", con->http.portal.id);
		}
		else if (auth->status.locked == 3) {
			portal_endpoint_response(con, "{s:s, s:{s:s, s:s}, s:I}", "jsonrpc", "2.0", "result", "auth", "locked", "abuse", "message",
				"This account has been locked on suspicion of abuse.", "id", con->http.portal.id);
		}
		else if (auth->status.locked == 4) {
			portal_endpoint_response(con, "{s:s, s:{s:s, s:s}, s:I}", "jsonrpc", "2.0", "result", "auth", "locked", "user", "message",
				"This account has been locked at the request of the user.", "id", con->http.portal.id);
		}
		else if (auth->status.locked != 0) {
			portal_endpoint_response(con, "{s:s, s:{s:s, s:s}, s:I}", "jsonrpc", "2.0", "result", "auth", "locked", "generic", "message",
				"This account has been locked.", "id", con->http.portal.id);
		}

		auth_free(auth);
		return;
	}

	// Pull the user info out.
	if ((state =  meta_get(auth->usernum, auth->username, auth->keys.master, auth->tokens.verification, META_PROTOCOL_WEB,
		META_GET_MESSAGES | META_GET_KEYS | META_GET_ALIASES | META_GET_FOLDERS | META_GET_CONTACTS, &(user)))) {

		if (state < 0) {
			portal_endpoint_error(con, 200, PORTAL_ENDPOINT_ERROR_AUTH, "This server is unable to access your mailbox. Please try again later.");
		}
		else {
			portal_endpoint_response(con, "{s:s, s:{s:s, s:s}, s:I}", "jsonrpc", "2.0", "result", "auth", "failed", "message",
				"The username and password provided are incorrect, please try again.", "id", con->http.portal.id);
		}

		auth_free(auth);
		return;
	}

	// Store the username and usernum as part of the session.
	auth_free(auth);

	// Transport Layer Security Required
	if (((user->flags & META_USER_ENCRYPT_DATA) || (user->flags & META_USER_TLS)) && con_secure(con) != 1 && !con_localhost(con)) {

		meta_inx_remove(user->usernum, META_PROTOCOL_WEB);
		portal_endpoint_response(con, "{s:s, s:{s:s, s:s}, s:I}", "jsonrpc", "2.0","result", "auth", "failed", "message",
			"The provided user account requires all connections be secured using encryption.", "id", con->http.portal.id);
		return;
	}

	// The session is authenticated!
	con->http.session->state = SESSION_STATE_AUTHENTICATED;
	con->http.response.cookie = HTTP_COOKIE_SET;
	con->http.session->user = user;

	portal_endpoint_response(con, "{s:s, s:{s:s, s:s}, s:I}", "jsonrpc", "2.0", "result", "auth", "success", "session",
		st_char_get(con->http.session->warden.token), "id", con->http.portal.id);

	return;
}

/**
 * @brief	Log a user out of a portal session in response to a "logout" json-rpc portal request.
 * @param	con		a pointer to the connection object of the requesting user.
 * @return	This function returns no value.
 */
void portal_endpoint_logout(connection_t *con) {

	// Check the session state. Method has no parameters.
	if (!portal_validate_request (con, PORTAL_ENDPOINT_ERROR_LOGOUT, "logout", false, 0)) {
		return;
	}

	con->http.session->state = SESSION_STATE_TERMINATED;
	con->http.response.cookie = HTTP_COOKIE_DELETE;

	portal_endpoint_response(con, "{s:s, s:{s:s}, s:I}", "jsonrpc", "2.0", "result", "logout", "success", "id", con->http.portal.id);
	meta_user_ref_dec(con->http.session->user, META_PROTOCOL_WEB);
	con->http.session->user = NULL;

	return;
}

/**
 * @brief	Get the list of a user's alert messages in response to an "alert.list" json-rpc portal request.
 * @param	con		a pointer to the connection object of the requesting user.
 * @return	This function returns no value.
 */
void portal_endpoint_alert_list(connection_t *con) {

	inx_t *alerts;
	json_error_t err;
	json_t *list, *entry;
	inx_cursor_t *cursor;
	meta_alert_t *active;

	// Check the session state. Method has no parameters.
	if (!portal_validate_request (con, PORTAL_ENDPOINT_ERROR_ALERT_LIST, "alert.list", false, 0)) {
		return;
	}

	if (!(list = json_array_d())) {
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}

	if ((alerts = meta_data_fetch_alerts(con->http.session->user->usernum))) {

		if ((cursor = inx_cursor_alloc(alerts))) {
			while ((active = inx_cursor_value_next(cursor))) {
				if (!(entry = json_pack_ex_d(&err, JSON_ENSURE_ASCII, "{s:I, s:s, s:s, s:I}", "alertID", active->alertnum, "type", st_char_get(active->type), "message",
						st_char_get(active->message), "date", active->created))) {
					log_pedantic("Alert packing attempt failed. { error = %s }", err.text);
				}
				else if (json_array_append_new_d(list, entry)) {
					log_pedantic("The alert object could not be appended to the result list. { error = %s }", err.text);
					json_decref_d(entry);
				}
			}

			inx_cursor_free(cursor);
		}

		inx_free(alerts);
	}

	portal_endpoint_response(con, "{s:s, s:o, s:I}", "jsonrpc", "2.0", "result", list, "id", con->http.portal.id);

	return;
}

/**
 * @brief	Process user alert message acknowledgements in response to an "alert.acknowledge" json-rpc portal request.
 * @param	con		a pointer to the connection object of the requesting user.
 * @return	This function returns no value.
 */
void portal_endpoint_alert_acknowledge(connection_t *con) {

	json_error_t err;
	json_t *alerts;
	size_t count;
	uint32_t transaction;

	// Check the session state. Method has only one parameter, which may optionally be empty.
	if (!portal_validate_request (con, PORTAL_ENDPOINT_ERROR_ALERT_ACKNOWLEDGE, "alert.acknowledge", true, 0)) {
		return;
	}
	// Params are optional. There may be alert acknowledgements.
	else if (json_unpack_ex_d(con->http.portal.params, &err, JSON_STRICT, "{s:o}", "alertIDs", &alerts)) {
		log_pedantic("Received invalid portal alert acknowledge request parameters { user = %.*s, errmsg = %s }",
			(int)st_length_get(con->http.session->user->username), st_char_get(con->http.session->user->username), err.text);
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	} else if (!json_is_array(alerts)) {
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	}

	// Shortcut. If an empty array was provided we can just return the success message.
	if (!(count = json_array_size_d(alerts))) {
		portal_endpoint_response(con, "{s:s, s:{s:s}, s:I}", "jsonrpc", "2.0", "result", "alert.acknowledge", "success", "id", con->http.portal.id);
		return;
	}

	// Before starting a SQL transaction check that all of the array values are positive integer values.
	for (size_t i = 0; i < count; i++) {
		if (!json_is_integer(json_array_get_d(alerts, i)) || json_integer_value_d(json_array_get_d(alerts, i)) < 0) {
			portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
			return;
		}
	}

	// Start a SQL transaction.
	if ((transaction = tran_start()) < 0) {
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}

	// Update each of the alerts. If any of the updates fail rollback and return a failure message.
	for (size_t i = 0; i < count; i++) {
		if (!meta_data_acknowledge_alert((uint64_t)json_integer_value_d(json_array_get_d(alerts, i)),
			con->http.session->user->usernum, transaction)) {
			tran_rollback(transaction);
			portal_endpoint_response(con, "{s:s, s:{s:s, s:s}, s:I}", "jsonrpc", "2.0", "result", "alert.acknowledge", "failed", "message",
				"Alert acknowledgement failed.", "id", con->http.portal.id);
			return;
		}
	}

	// Commit the transaction.
	if (tran_commit(transaction)) {
		portal_endpoint_response(con, "{s:s, s:{s:s, s:s}, s:I}", "jsonrpc", "2.0", "result", "alert.acknowledge", "failed", "message",
			"Alert acknowledgement failed.", "id", con->http.portal.id);
		return;
	}

	portal_endpoint_response(con, "{s:s, s:{s:s}, s:I}", "jsonrpc", "2.0", "result", "alert.acknowledge", "success", "id", con->http.portal.id);
	return;
}

/**
 * @brief	Return the list of a user account's aliases in response to an "aliases" json-rpc portal request.
 * @param	con		a pointer to the connection object of the requesting user.
 * @return	This function returns no value.
 */
void portal_endpoint_aliases(connection_t *con) {

	json_error_t err;
	json_t *list, *entry;
	inx_cursor_t *cursor;
	meta_alias_t *active;

	// Check the session state. Method has no parameters.
	if (!portal_validate_request (con, PORTAL_ENDPOINT_ERROR_ALIASES, "aliases", false, 0)) {
		return;
	}

	if (!(list = json_array_d())) {
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}

	if ((cursor = inx_cursor_alloc(con->http.session->user->aliases)))	{

		while ((active = inx_cursor_value_next(cursor))) {

			// If the current alias is the selected address, include the default flag in the output. Otherwise use the short version.
			if ((active->selected == true && !(entry = json_pack_ex_d(&err, JSON_ENSURE_ASCII, "{s:s, s:s, s:b}", "email",
				st_char_get(active->address),	"name", st_char_get(active->display), "def", json_true_d()))) ||
				(active->selected != true && !(entry = json_pack_ex_d(&err, JSON_ENSURE_ASCII, "{s:s, s:s}",  "email", st_char_get(active->address),	"name", st_char_get(active->display))))) {
				log_pedantic("Mailbox alias packing attempt failed. { error = %s }", err.text);
				portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
				return;
			}
			else if (json_array_append_new_d(list, entry)) {
				log_pedantic("The mailbox alias object could not be appended to the result list. { error = %s }", err.text);
				json_decref_d(entry);
				portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
				return;
			}

		}

		inx_cursor_free(cursor);
	}

	portal_endpoint_response(con, "{s:s, s:o, s:I}", "jsonrpc", "2.0", "result", list, "id", con->http.portal.id);
	return;
}

/**
 * @brief	Return whether or not a user is using cookies, in response to a "cookies" json-rpc portal request.
 * @param	con		a pointer to the connection object of the requesting user.
 * @return	This function returns no value.
 */
void portal_endpoint_cookies(connection_t *con) {

	// Check the session state. Method has no parameters.
	if (!portal_validate_request (con, PORTAL_ENDPOINT_ERROR_COOKIES, "cookies", false, 0)) {
		return;
	}

	if (con->http.cookie && !st_cmp_ci_starts(con->http.cookie, PLACER("portal=", 7))) {
		portal_endpoint_response(con, "{s:s, s:{s:s}, s:I}", "jsonrpc", "2.0", "result", "cookies", "enabled", "id", con->http.portal.id);
	}
	else {
		portal_endpoint_response(con, "{s:s, s:{s:s}, s:I}", "jsonrpc", "2.0", "result", "cookies", "disabled", "id", con->http.portal.id);
	}

	return;
}

/**
 * @brief	Create a new folder in response to a "folders.add" json-rpc portal request.
 * @note	Currently only contexts for new mail folders and contacts folders are supported.
 * @param	con		a pointer to the connection object of the requesting user.
 * @return	This function returns no value.
 */
void portal_endpoint_folders_add(connection_t *con) {

	size_t count;
	int_t context;
	json_error_t err;
	json_int_t parent = 0;
	stringer_t *sname;
	chr_t *name, *method = NULL;

	// Check the session state. Method has a variable number of parameters.
	if (!portal_validate_request (con, PORTAL_ENDPOINT_ERROR_FOLDERS_ADD, "folders.add", true, 0)) {
		return;
	}
	// Validate the request format and extract the submitted values.
	if ((count = json_object_size_d(con->http.portal.params)) && (count > 3 || count < 2)) {
		log_pedantic("Received invalid portal folder add request parameters { user = %.*s, count = %u }",
			(int)st_length_get(con->http.session->user->username), st_char_get(con->http.session->user->username), (unsigned int) count);
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	} else if ((count == 2 && json_unpack_ex_d(con->http.portal.params, &err, JSON_STRICT, "{s:s, s:s}", "context", &method, "name", &name)) ||
		(count == 3 && json_unpack_ex_d(con->http.portal.params, &err, JSON_STRICT, "{s:s, s:s, s:I}", "context", &method, "name", &name, "parentID", &parent))) {
		log_pedantic("Received invalid portal folder add request parameters { user = %.*s, errmsg = %s }",
			(int)st_length_get(con->http.session->user->username), st_char_get(con->http.session->user->username), err.text);
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	}

	// Parse the context.
	if ((context = portal_parse_context(NULLER(method))) == PORTAL_ENDPOINT_CONTEXT_INVALID) {
		log_pedantic("Received invalid folder add request context { user = %s }", st_char_get(con->http.session->user->username));
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_INVALID_KEYWORD | PORTAL_ENDPOINT_ERROR_FOLDERS_ADD, "Unrecognized context.");
		return;
	}

	if (!(sname = st_import(name,ns_length_get(name)))) {
		log_pedantic("Allocation of added folder name failed.");
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}
	else if (context == PORTAL_ENDPOINT_CONTEXT_MAIL) {
		portal_folder_mail_add(con, sname, parent);
	}
	else if (context == PORTAL_ENDPOINT_CONTEXT_CONTACTS) {
		portal_folder_contacts_add(con, sname, parent);
	}
	else {
		log_pedantic("Received bad folder add request context { user = %s, context = %s }", st_char_get(con->http.session->user->username), method);
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
	}

	st_free(sname);
	//portal_endpoint_response(con, "{s:s, s:{s:s}, s:I}", "jsonrpc", "2.0", "result", "folder.rename", "success", "id", con->http.portal.id);
	return;
}

/**
 * @brief	Return a list of a user's folders in response to a "folders.list" json-rpc portal request.
 * @note	Currently only contexts for new mail folders and contacts folders are supported.
 * @param	con		a pointer to the connection object of the requesting user.
 * @return	This function returns no value.
 */
void portal_endpoint_folders_list(connection_t *con) {

	json_error_t err;
	json_t *list, *entry;
	inx_cursor_t *cursor;
	meta_folder_t *active;
	magma_folder_t *folder;
	chr_t *context = "mail";

	// Check the session state. Method has 1 parameter.
	if (!portal_validate_request (con, PORTAL_ENDPOINT_ERROR_FOLDERS_LIST, "folders.list", true, 1)) {
		return;
	}
	// Validate the request format and extract the submitted values.
	else if (json_unpack_ex_d(con->http.portal.params, &err, JSON_STRICT, "{s:s}", "context", &context)) {
		log_pedantic("Received invalid portal folders list request parameters { user = %.*s, errmsg = %s }",
			(int)st_length_get(con->http.session->user->username), st_char_get(con->http.session->user->username), err.text);
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	}

	// One of the comparison attempts will return a zero and prevent the error if the context is a legal value. Note that we initialize the
	// context string to "mail" to allow requests that have no context parameter..
	if (st_cmp_ci_eq(NULLER(context), PLACER("mail", 4)) && st_cmp_ci_eq(NULLER(context), PLACER("contacts", 8)) &&
		st_cmp_ci_eq(NULLER(context), PLACER("settings", 8)) && st_cmp_ci_eq(NULLER(context), PLACER("help", 4))) {
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	}

	if (!(list = json_array_d())) {
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}

	if ((!st_cmp_ci_eq(NULLER(context), PLACER("mail", 4)) && (cursor = inx_cursor_alloc(con->http.session->user->folders))))	{

		while ((active = inx_cursor_value_next(cursor))) {

			/// LOW: User created folder names  _could_ have a NULL byte. How should we handle that?
			if ((active->parent == 0 && !(entry = json_pack_ex_d(&err, JSON_ENSURE_ASCII, "{s:s, s:I, s:s}", "context", "mail", "folderID", active->foldernum, "name", active->name))) ||
				(active->parent != 0 && !(entry = json_pack_ex_d(&err, JSON_ENSURE_ASCII, "{s:s, s:I, s:I, s:s}", "context", "mail", "folderID", active->foldernum, "parentID", active->parent, "name", active->name)))) {
				log_pedantic("Folder packing attempt failed. { error = %s }", err.text);
			}
			else if (json_array_append_new_d(list, entry)) {
				log_pedantic("The folder object could not be appended to the result list. { error = %s }", err.text);
				json_decref_d(entry);
			}

		}

		inx_cursor_free(cursor);
	}
	else if ((!st_cmp_ci_eq(NULLER(context), PLACER("contacts", 8)) && (cursor = inx_cursor_alloc(con->http.session->user->contacts))))	{

		while ((folder = inx_cursor_value_next(cursor))) {
			/// LOW: User created folder names  _could_ have a NULL byte. How should we handle that?
			if ((folder->parent == 0 && !(entry = json_pack_ex_d(&err, JSON_ENSURE_ASCII, "{s:s, s:I, s:s}", "context", "contacts", "folderID", folder->foldernum, "name", st_char_get(folder->name)))) ||
				(folder->parent != 0 && !(entry = json_pack_ex_d(&err, JSON_ENSURE_ASCII, "{s:s, s:I, s:I, s:s}", "context", "contacts", "folderID", folder->foldernum, "parentID", folder->parent, "name", st_char_get(folder->name))))) {
				log_pedantic("Folder packing attempt failed. { error = %s }", err.text);
			}
			else if (json_array_append_new_d(list, entry)) {
				log_pedantic("The folder object could not be appended to the result list. { error = %s }", err.text);
				json_decref_d(entry);
			}

		}

		inx_cursor_free(cursor);
	}
	else {
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_INVALID_KEYWORD | PORTAL_ENDPOINT_ERROR_FOLDERS_LIST, "Context not supported.");
		return;
	}

	portal_endpoint_response(con, "{s:s, s:o, s:I}", "jsonrpc", "2.0", "result", list, "id", con->http.portal.id);
	return;
}

/**
 * @brief	Remove a user's folder in response to a json-rpc "folders.remove" portal request.
 * @param	con		a pointer to the connection object of the requesting user.
 * @return	This function returns no value.
 */
void portal_endpoint_folders_remove(connection_t *con) {

	int_t context;
	json_error_t err;
	chr_t *method = NULL;
	json_int_t foldernum = 0;

	// Check the session state. Method has 2 parameters.
	if (!portal_validate_request (con, PORTAL_ENDPOINT_ERROR_FOLDERS_REMOVE, "folders.remove", true, 2)) {
		return;
	}
	// Validate the request format and extract the submitted values.
	else if ((json_unpack_ex_d(con->http.portal.params, &err, JSON_STRICT, "{s:s, s:I}", "context", &method, "folderID", &foldernum))) {
		log_pedantic("Received invalid portal folders remove request parameters { user = %.*s, errmsg = %s }",
			(int)st_length_get(con->http.session->user->username), st_char_get(con->http.session->user->username), err.text);
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	}

	// Parse the context.
	if ((context = portal_parse_context(NULLER(method))) == PORTAL_ENDPOINT_CONTEXT_INVALID) {
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_INVALID_KEYWORD | PORTAL_ENDPOINT_ERROR_FOLDERS_REMOVE, "Unrecognized context.");
	}
	else if (context == PORTAL_ENDPOINT_CONTEXT_MAIL) {
		portal_folder_mail_remove(con, foldernum);
	}
	else if (context == PORTAL_ENDPOINT_CONTEXT_CONTACTS) {
		portal_folder_contacts_remove(con, foldernum);
	}
	else {
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
	}

	return;
}

/**
 * @brief	Rename a user's folder in response to a json-rpc "folders.rename" portal request.
 * @param	con		a pointer to the connection object of the requesting user.
 * @return	This function returns no value.
 */
void portal_endpoint_folders_rename(connection_t *con) {

	json_error_t err;
	uint64_t foldernum;
	int_t state, context;
	meta_folder_t *active_m = NULL;
	magma_folder_t *active_c = NULL;
	chr_t *rename = NULL, *method = NULL;
	stringer_t *original = NULL, *srename = NULL;

	// Check the session state. Method has 3 parameters.
	if (!portal_validate_request (con, PORTAL_ENDPOINT_ERROR_FOLDERS_RENAME, "folders.rename", true, 3)) {
		return;
	}
	// Validate the request format and extract the submitted values.
	else if ((json_unpack_ex_d(con->http.portal.params, &err, JSON_STRICT, "{s:s, s:I, s:s}", "context", &method, "folderID", &foldernum, "name", &rename))) {
		log_pedantic("Received invalid portal folders rename request parameters { user = %.*s, errmsg = %s }",
				(int)st_length_get(con->http.session->user->username), st_char_get(con->http.session->user->username), err.text);
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	}

	// Parse the context.
	if ((context = portal_parse_context(NULLER(method))) == PORTAL_ENDPOINT_CONTEXT_INVALID) {
		log_pedantic("Received invalid folder rename request context { user = %s }", st_char_get(con->http.session->user->username));
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_INVALID_KEYWORD | PORTAL_ENDPOINT_ERROR_FOLDERS_RENAME, "Unrecognized context.");
		return;
	}

	if ((context != PORTAL_ENDPOINT_CONTEXT_MAIL) && (context != PORTAL_ENDPOINT_CONTEXT_CONTACTS)) {
		log_pedantic("Received bad folder rename request context { user = %s, context = %s }", st_char_get(con->http.session->user->username), method);
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	}
	else if (context == PORTAL_ENDPOINT_CONTEXT_MAIL) {
		active_m = meta_folders_by_number(con->http.session->user->folders, foldernum);
	} else if (context == PORTAL_ENDPOINT_CONTEXT_CONTACTS) {
		active_c = magma_folder_find_number(con->http.session->user->contacts, foldernum);
	}

	if (((context == PORTAL_ENDPOINT_CONTEXT_MAIL) && !active_m) || ((context == PORTAL_ENDPOINT_CONTEXT_CONTACTS) && !active_c)) {
		log_pedantic("Unable to find user's folder for renaming { user = %s, folder = %zu, context = %s }", st_char_get(con->http.session->user->username), foldernum, method);
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_FOLDERS_RENAME,
			"The folder provided for deletion is invalid.");
		return;
	}

	if (context == PORTAL_ENDPOINT_CONTEXT_MAIL) {
		original = meta_folders_name(con->http.session->user->folders, active_m);
	} else if (context == PORTAL_ENDPOINT_CONTEXT_CONTACTS) {
		original = magma_folder_name(con->http.session->user->contacts, active_c);
	}

	if (!original) {
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}

	if (!(srename = st_import(rename,ns_length_get(rename)))) {
		log_pedantic("Allocation of renamed folder name failed.");
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}

	// Create the folder.
	meta_user_wlock(con->http.session->user);

	if (context == PORTAL_ENDPOINT_CONTEXT_MAIL) {
		state = imap_folder_rename(con->http.session->user->usernum, con->http.session->user->folders, original, srename);
	}
	else if (context == PORTAL_ENDPOINT_CONTEXT_CONTACTS) {
		state = contact_folder_rename(con->http.session->user->usernum, foldernum, srename, con->http.session->user->contacts);
	}

	st_free(srename);
	st_free(original);

	/// HIGH: Use the context and act appropriately!

	// And finally, increment the serial number. If the serial number indicates no outside changes we can increment it without forcing a refresh.
	if (context == PORTAL_ENDPOINT_CONTEXT_MAIL) {

		if (con->http.session->user->serials.folders == serial_get(OBJECT_FOLDERS, con->http.session->user->usernum)) {
			con->http.session->user->serials.folders = serial_increment(OBJECT_FOLDERS, con->http.session->user->usernum);
		}
		// Increment the reference counter and queue a session update.
		else {
			serial_increment(OBJECT_FOLDERS,con->http.session->user->usernum);
			sess_trigger(con->http.session);
		}

	} else if (context == PORTAL_ENDPOINT_CONTEXT_CONTACTS) {

		if (con->http.session->user->serials.contacts == serial_get(OBJECT_CONTACTS, con->http.session->user->usernum)) {
			con->http.session->user->serials.contacts = serial_increment(OBJECT_CONTACTS, con->http.session->user->usernum);
		}
		// Increment the reference counter and queue a session update.
		else {
			serial_increment(OBJECT_CONTACTS,con->http.session->user->usernum);
			sess_trigger(con->http.session);
		}

	}

	meta_user_unlock(con->http.session->user);

	// Let the user know what happened.
	if (state == 1) {
		portal_endpoint_response(con, "{s:s, s:{s:s}, s:I}", "jsonrpc", "2.0", "result", "folder.rename", "success", "id", con->http.portal.id);
	}
	else if (state == -1) {
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_FOLDERS_RENAME,
			"The folder provided for renaming is invalid.");
	}
	else if (state == -2) {
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_CONSTRAINT_VIOLATION | PORTAL_ENDPOINT_ERROR_FOLDERS_RENAME,
			"You are not allowed to rename the Inbox folder.");
	}
	else if (state == -3) {
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_FOLDERS_RENAME,
			"The folder you are trying to rename does not exist.");
	}
	else if (state == -4) {
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_CONSTRAINT_VIOLATION | PORTAL_ENDPOINT_ERROR_FOLDERS_RENAME,
			"The specified folder would exceed the limit on folder levels.");
	}
	else if (state == -5) {
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_CONSTRAINT_VIOLATION | PORTAL_ENDPOINT_ERROR_FOLDERS_RENAME,
			"The target name already exists.");
	}
	else if (state == -6) {
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_CONSTRAINT_VIOLATION | PORTAL_ENDPOINT_ERROR_FOLDERS_RENAME,
			"The specified folder name is too long.");
	}
	else if (state == -7) {
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_CONSTRAINT_VIOLATION | PORTAL_ENDPOINT_ERROR_FOLDERS_RENAME,
			"A folder cannot be turned into a child of itself.");
	}
	else {
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
	}

	return;
}

void portal_endpoint_folders_tags(connection_t *con) {

	inx_t *stats;
	int_t context;
	json_error_t err;
	uint64_t folder;
	inx_cursor_t *cursor;
	json_t *list, *entry;
	meta_stats_tag_t *active;
	chr_t *method = "mail";
	//multi_t multi = { .type = M_TYPE_UINT64, .val.u64 = 0 };

	// Check the session state. Method has 2 parameters.
	if (!portal_validate_request (con, PORTAL_ENDPOINT_ERROR_FOLDERS_TAGS, "folders.tags", true, 2)) {
		return;
	}
	// Validate the request format and extract the submitted values.
	else if (json_unpack_ex_d(con->http.portal.params, &err, JSON_STRICT, "{s:s, s:I}", "context", &method, "folderID", &folder)) {
		log_pedantic("Received invalid portal folders tags request parameters { user = %.*s, errmsg = %s }",
			(int)st_length_get(con->http.session->user->username), st_char_get(con->http.session->user->username), err.text);
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	}

	// This endpoint only applies to mail folders at the moment.
	else if ((context = portal_parse_context(NULLER(method))) != PORTAL_ENDPOINT_CONTEXT_MAIL) {
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	}

	// Lock the user while we scan the folder.
	meta_user_rlock(con->http.session->user);

	// Invalid folder number.
	if (!folder || !(meta_folders_by_number(con->http.session->user->folders, folder))) {
		meta_user_unlock(con->http.session->user);
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_FOLDERS_TAGS, "The requested folder number is invalid.");
		return;
	}
	else if (!(list = json_object_d())) {
		meta_user_unlock(con->http.session->user);
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}

	/// HIGH: Right now we only have indexes with the mail folders so other context types generate an empty array result.
	if (context == PORTAL_ENDPOINT_CONTEXT_MAIL && (stats = meta_folders_stats_tags(con->http.session->user->messages, folder))) {
		if ((cursor = inx_cursor_alloc(stats))) {
			while ((active = inx_cursor_value_next(cursor))) {
				if (!(entry = json_integer_d(active->count))) {
					log_pedantic("Tag entry count serialization failed.");
				}
				else if (json_object_set_new_d(list, st_char_get(active->tag), entry)) {
					log_pedantic("Tag statistic entry serialization failed.");
					json_decref_d(entry);
				}
			}
			inx_cursor_free(cursor);
		}
		inx_free(stats);
	}

	meta_user_unlock(con->http.session->user);
	portal_endpoint_response(con, "{s:s, s:o, s:I}", "jsonrpc", "2.0", "result", list, "id", con->http.portal.id);

	return;
}

/**
 * @brief	Retrieve a list of the user's messages in response to a json-rpc "messages.list" portal request.
 * @param	con		a pointer to the connection object of the requesting user.
 * @return	This function returns no value.
 */
void portal_endpoint_messages_list(connection_t *con) {

	json_error_t err;;
	json_t *tags, *list, *entry;
	inx_cursor_t *cursor;
	meta_message_t *active;
	uint64_t foldernum, count;
	stringer_t *header, *fields[8];

	// Check the session state. Method has 1 parameter.
	if (!portal_validate_request (con, PORTAL_ENDPOINT_ERROR_MESSAGES_LIST, "messages.list", true, 1)) {
		return;
	}
	// Validate the request format and extract the submitted values.
	else if (json_unpack_ex_d(con->http.portal.params, &err, JSON_STRICT, "{s:I}", "folderID", &foldernum)) {
		log_pedantic("Received invalid portal messages list request parameters { user = %.*s, errmsg = %s }",
			(int)st_length_get(con->http.session->user->username), st_char_get(con->http.session->user->username), err.text);
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	}

	if (!(list = json_array_d())) {
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}

	if ((cursor = inx_cursor_alloc(con->http.session->user->messages)))	{

		while ((active = inx_cursor_value_next(cursor))) {

			if (active->foldernum == foldernum && (header = mail_load_header(active, con->http.session->user))) {

				fields[0] = mail_header_fetch_cleaned(header, PLACER("From", 4));
				fields[1] = mail_header_fetch_cleaned(header, PLACER("To", 2));

				/// LOW: Add the ability to track the recipient email address for a message, even if its not provided in the To field.
				fields[2] = mail_header_fetch_cleaned(header, PLACER("To", 2));

				fields[3] = mail_header_fetch_cleaned(header, PLACER("Reply-To", 8));
				fields[4] = mail_header_fetch_cleaned(header, PLACER("Return-Path", 11));
				fields[5] = mail_header_fetch_cleaned(header, PLACER("Subject", 7));
				fields[6] = mail_header_fetch_cleaned(header, PLACER("Date", 4));

				/// LOW: Add snippet support.
				fields[7] = st_import("...", 3);

				// Tags
				if ((tags = json_array_d()) && active->tags && (count = ar_length_get(active->tags))) {

					for (uint64_t i = 0; i < count; i++) {
						json_array_append_new_d(tags, json_string_d(st_char_get(ar_field_st(active->tags, i))));
					}

				}

				if (!(entry = json_pack_ex_d(&err, JSON_ENSURE_ASCII, "{s:I, s:o, s:o, s:S, s:S, s:S, s:S, s:S, s:S, s:I, s:I, s:S, s:I}", "messageID",
					active->messagenum, "flags", portal_message_flags_array(active), "tags", tags, "from", st_char_get(fields[0]), "to", st_char_get(fields[1]),
					"addressedTo", st_char_get(fields[2]), "replyTo", st_char_get(fields[3]), "returnPath", st_char_get(fields[4]), "subject",
					st_char_get(fields[5]), "utc", active->created, "arrivalUtc", active->created, "snippet", st_char_get(fields[7]), "bytes",
					active->size))) {
					log_pedantic("Message packing attempt failed. { error = %s }", err.text);
				}
				else if (json_array_append_new_d(list, entry)) {
					log_pedantic("The message object could not be appended to the result list. { error = %s }", err.text);
					json_decref_d(entry);
				}

				// Release the header fields.
				for (int_t i = 0; i <= 7; i++) {
					st_cleanup(fields[i]);
				}

				// Release header string.
				st_free(header);
			}
		}

		inx_cursor_free(cursor);
	}

	portal_endpoint_response(con, "{s:s, s:o, s:I}", "jsonrpc", "2.0", "result", list, "id", con->http.portal.id);

	return;
}

void portal_endpoint_messages_copy(connection_t *con) {

	json_error_t err;
	bool_t commit = true;
	meta_message_t *active;
	json_t *list, *entry, *messages;
	uint64_t src_folder, dst_folder, count, copy;
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = 0 };

	// Check the session state. Method has 3 parameters.
	if (!portal_validate_request (con, PORTAL_ENDPOINT_ERROR_MESSAGES_COPY, "messages.copy", true, 3)) {
		return;
	}
	// Validate the request format and extract the submitted values.
	else if (json_unpack_ex_d(con->http.portal.params, &err, JSON_STRICT, "{s:I, s:I, s:o}", "sourceFolderID", &src_folder, "targetFolderID",
		&dst_folder, "messageIDs", &messages)) {
		log_pedantic("Received invalid portal messages copy request parameters { user = %.*s, errmsg = %s }",
			(int)st_length_get(con->http.session->user->username), st_char_get(con->http.session->user->username), err.text);
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	} else if (!json_is_array(messages) || !(count = json_array_size_d(messages))) {
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	}

	meta_user_wlock(con->http.session->user);

	// Confirm the source and target folder IDs are legit and that the source and target folder IDs are not identical.
	if (!src_folder || !dst_folder || src_folder == dst_folder || !(meta_folders_by_number(con->http.session->user->folders, src_folder)) ||
		!(meta_folders_by_number(con->http.session->user->folders, dst_folder))) {
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_MESSAGES_COPY, "Invalid folder reference.");
	}
	else if (!(list = json_array_d())) {
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
	}
	else {

		for (uint64_t i = 0; i < count && commit; i++) {

			// Confirm the message ID is a number, that the message exists and that the message is in the source folder.
			if (!json_is_integer(json_array_get_d(messages, i)) || !(key.val.u64 = json_integer_value_d(json_array_get_d(messages, i))) ||
				!(active = inx_find(con->http.session->user->messages, key)) || active->foldernum != src_folder) {
				portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_MESSAGES_COPY, "Invalid message reference.");
				commit = false;
			}
			else if (!meta_messages_copier(con->http.session->user, active, dst_folder, &copy, false, META_LOCKED)) {
				portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
				commit = false;
			}
			else if (!(entry = json_pack_ex_d(&err, JSON_ENSURE_ASCII, "{s:I, s:I}", "sourceMessageID", active->messagenum, "targetMessageID", copy))) {
				portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
				log_pedantic("Message packing attempt failed. { error = %s }", err.text);
				commit = false;
			}
			else if (json_array_append_new_d(list, entry)) {
				log_pedantic("The message object could not be appended to the result list. { error = %s }", err.text);
				json_decref_d(entry);
				commit = false;
			}

		}

		// If any messages are copied to a different folder we'll need to update the sequence numbers to reflect the new status.
		meta_messages_update_sequences(con->http.session->user->folders, con->http.session->user->messages);

		if (commit) {

			// And finally, increment the serial number. If the serial number indicates no outside changes we can increment it without forcing a refresh.
			if (con->http.session->user->serials.messages == serial_get(OBJECT_MESSAGES, con->http.session->user->usernum)) {
				con->http.session->user->serials.messages = serial_increment(OBJECT_MESSAGES, con->http.session->user->usernum);
			}
			// Increment the reference counter and queue a session update.
			else {
				serial_increment(OBJECT_MESSAGES,con->http.session->user->usernum);
				sess_trigger(con->http.session);
			}
			portal_endpoint_response(con, "{s:s, s:o, s:I}", "jsonrpc", "2.0", "result", list, "id", con->http.portal.id);
		}
		/// HIGH: If a multiple message copy fails in the middle, go back and remove any messages that may have been copied before the error.
		else {
			json_decref_d(list);
		}
	}

	meta_user_unlock(con->http.session->user);

	return;
}

void portal_endpoint_messages_flag(connection_t *con) {

	uint32_t bits;
	json_error_t err;
	int_t action, ret;
	inx_t *list = NULL;
	chr_t *method = NULL;
	bool_t commit = true;
	inx_cursor_t *cursor;
	uint64_t folder, count;
	meta_message_t *active = NULL;
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = 0 };
	json_t *flags = NULL, *messages, *collection = NULL, *entry = NULL;

	// Check the session state. Method can have 3 or 4 parameters.
	if (!portal_validate_request (con, PORTAL_ENDPOINT_ERROR_MESSAGES_FLAG, "messages.flag", true, 0)) {
		return;
	}
	// Validate the request format and extract the submitted values.
	else if (json_object_size_d(con->http.portal.params) == 4  && json_unpack_ex_d(con->http.portal.params, &err, JSON_STRICT,
			"{s:s, s:o, s:o, s:I}", "action", &method, "flags",	&flags, "messageIDs", &messages, "folderID", &folder)) {
		log_pedantic("Received invalid portal messages flag request parameters { user = %.*s, errmsg = %s }",
			(int)st_length_get(con->http.session->user->username), st_char_get(con->http.session->user->username), err.text);
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	} else if (json_object_size_d(con->http.portal.params) == 3  && json_unpack_ex_d(con->http.portal.params, &err, JSON_STRICT,
			"{s:s, s:o, s:I}", "action", &method, "messageIDs", &messages, "folderID", &folder)) {
		log_pedantic("Received invalid portal messages flag request parameters { user = %.*s, errmsg = %s }",
			(int)st_length_get(con->http.session->user->username), st_char_get(con->http.session->user->username), err.text);
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	} else if (!json_is_array(messages) || !(count = json_array_size_d(messages))) {
		log_pedantic("Received portal messages flag request with b ad messageIDs parameter  { user = %s }", st_char_get(con->http.session->user->username));
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	}

	// Parse the action.
	if (!st_cmp_ci_eq(NULLER(method), PLACER("add", 3))) {
		action = PORTAL_ENDPOINT_ACTION_ADD;
	}
	else if (!st_cmp_ci_eq(NULLER(method), PLACER("remove", 6))) {
		action = PORTAL_ENDPOINT_ACTION_REMOVE;
	}
	else if (!st_cmp_ci_eq(NULLER(method), PLACER("replace", 7))) {
		action = PORTAL_ENDPOINT_ACTION_REPLACE;
	}
	else if (!st_cmp_ci_eq(NULLER(method), PLACER("list", 4))) {
		action = PORTAL_ENDPOINT_ACTION_LIST;
	}
	else {
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_INVALID_KEYWORD | PORTAL_ENDPOINT_ERROR_MESSAGES_FLAG, "Unrecognized action.");
		return;
	}

	// Parse the array of flags.
	if ((action == PORTAL_ENDPOINT_ACTION_LIST && flags) || (action != PORTAL_ENDPOINT_ACTION_LIST && !flags)) {
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	}
	else if (flags && (ret = portal_parse_flags(flags, &bits)) == -2) {
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_INVALID_KEYWORD | PORTAL_ENDPOINT_ERROR_MESSAGES_FLAG, "Unrecognized message flag.");
		return;
	}
	else if (flags && ret == -1) {
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}
	else if (flags && ret == 0 && (action == PORTAL_ENDPOINT_ACTION_ADD || action == PORTAL_ENDPOINT_ACTION_REMOVE)) {
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_ILLEGAL_COMBINATION | PORTAL_ENDPOINT_ERROR_MESSAGES_FLAG, "The requested action does not accept an empty array of flags.");
		return;
	}
	// Make sure the user isn't trying to manipulate one of the internal system flags.
	else if (flags && (bits & MAIL_STATUS_SYSTEM_FLAGS)) {
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_SYSTEM_FLAG | PORTAL_ENDPOINT_ERROR_MESSAGES_FLAG, "The flags submitted included one or more inaccessible system flags.");
		return;
	}

	if (action == PORTAL_ENDPOINT_ACTION_LIST) {
		meta_user_rlock(con->http.session->user);
	}
	else {
		meta_user_wlock(con->http.session->user);
	}

	// Confirm the source and target folder IDs are legit and that the source and target folder IDs are not identical.
	if (!folder || !(meta_folders_by_number(con->http.session->user->folders, folder))) {
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_MESSAGES_FLAG, "Invalid folder reference.");
	}

	else if (!(list = inx_alloc(M_INX_LINKED, NULL))) {
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
	}
	else {

		for (uint64_t i = 0; i < count && commit; i++) {

			// Confirm the message ID is a number, that the message exists and that the message is in the source folder.
			if (!json_is_integer(json_array_get_d(messages, i)) || !(key.val.u64 = json_integer_value_d(json_array_get_d(messages, i))) ||
				!(active = inx_find(con->http.session->user->messages, key)) || active->foldernum != folder) {
				portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_MESSAGES_FLAG, "Invalid message reference.");
				commit = false;
			}
			else if (inx_insert(list, key, active) != true) {
				portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
				commit = false;
			}
		}

		if (action == PORTAL_ENDPOINT_ACTION_LIST && !(collection = json_array_d())) {
			portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
			commit = false;
		}

		if (commit) {

			// Update the database first.
			switch (action) {
			case (PORTAL_ENDPOINT_ACTION_ADD):
				meta_data_flags_add(list, con->http.session->user->usernum, folder, bits);
				break;
			case (PORTAL_ENDPOINT_ACTION_REMOVE):
				meta_data_flags_remove(list, con->http.session->user->usernum, folder, bits);
				break;
			case (PORTAL_ENDPOINT_ACTION_REPLACE):
				meta_data_flags_replace(list, con->http.session->user->usernum, folder, bits);
				break;
			}

			// Now update the messages structure.
			if ((cursor = inx_cursor_alloc(list))) {
				while ((active = inx_cursor_value_next(cursor))) {
					switch (action) {
					case (PORTAL_ENDPOINT_ACTION_ADD):
						active->status = active->status | bits;
						break;
					case (PORTAL_ENDPOINT_ACTION_REMOVE):
						active->status = (active->status | bits) ^ bits;
						break;
					case (PORTAL_ENDPOINT_ACTION_REPLACE):
						active->status = ((active->status | MAIL_STATUS_USER_FLAGS) ^ MAIL_STATUS_USER_FLAGS) | bits;
						break;
					case (PORTAL_ENDPOINT_ACTION_LIST):
						if (!(entry = json_pack_ex_d(&err, JSON_ENSURE_ASCII, "{s:I, s:o}", "messageID", active->messagenum, "flags", portal_message_flags_array(active)))) {
							log_pedantic("Message flag packing attempt failed. { error = %s }", err.text);
						}
						else if (json_array_append_new_d(collection, entry)) {
							log_pedantic("The message object could not be appended to the result list. { error = %s }", err.text);
							json_decref_d(entry);
						}
						break;
					}
				}
				inx_cursor_free(cursor);
			}

			// And finally, increment the serial number. If the serial number indicates no outside changes we can increment it without forcing a refresh.
			if (con->http.session->user->serials.messages == serial_get(OBJECT_MESSAGES, con->http.session->user->usernum)) {
				con->http.session->user->serials.messages = serial_increment(OBJECT_MESSAGES, con->http.session->user->usernum);
			}
			// Increment the reference counter and queue a session update.
			else {
				serial_increment(OBJECT_MESSAGES, con->http.session->user->usernum);
				sess_trigger(con->http.session);
			}

			if (action == PORTAL_ENDPOINT_ACTION_LIST) {


				/// HIGH: This function needs restructuring pretty badly. We are setting collection to NULL so we can add a cleanup call below. But if the function works as intended
				/// the reference is stolen by the response. but if an error occurs and the we don't return a response the collection ends up as a memory leak without the cleanup call below.
				portal_endpoint_response(con, "{s:s, s:o, s:I}", "jsonrpc", "2.0", "result", collection, "id", con->http.portal.id);
				collection = NULL;

			}
			else {
				portal_endpoint_response(con, "{s:s, s:{s:s}, s:I}", "jsonrpc", "2.0", "result", "messages.flag", "success", "id", con->http.portal.id);
			}

		}

	}

	/// TODO: Were holding onto the user lock while waiting on the response to be sent over the wire; and this function could probably use a rethink.
	meta_user_unlock(con->http.session->user);

	if (list) {
		inx_free(list);
	}

	/// HIGH: See the sister comment a few lines back!
	if (collection) {
		json_decref_d(collection);
	}


	return;
}

void portal_endpoint_messages_move(connection_t *con) {

	int_t result;
	json_error_t err;
	bool_t commit = true;
	meta_message_t *active;
	json_t *messages;
	uint64_t src_folder, dst_folder, count;
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = 0 };

	// Check the session state. Method has 3 parameters.
	if (!portal_validate_request (con, PORTAL_ENDPOINT_ERROR_MESSAGES_MOVE, "messages.move", true, 3)) {
		return;
	}
	// Validate the request format and extract the submitted values.
	else if (json_unpack_ex_d(con->http.portal.params, &err, JSON_STRICT, "{s:I, s:I, s:o}", "sourceFolderID", &src_folder, "targetFolderID",
		&dst_folder, "messageIDs", &messages)) {
		log_pedantic("Received invalid portal messages move request parameters { user = %.*s, errmsg = %s }",
			(int)st_length_get(con->http.session->user->username), st_char_get(con->http.session->user->username), err.text);
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	}
	else if (!json_is_array(messages) || !(count = json_array_size_d(messages)) || !src_folder || !dst_folder) {
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	}
	else if (src_folder == dst_folder) {
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_ILLEGAL_COMBINATION | PORTAL_ENDPOINT_ERROR_MESSAGES_MOVE, "The source and destination folders must be different.");
		meta_user_unlock(con->http.session->user);
		return;
	}

	meta_user_wlock(con->http.session->user);

	// Confirm the source and target folder IDs are legit and that the source and target folder IDs are not identical.
	if (!(meta_folders_by_number(con->http.session->user->folders, src_folder)) || !(meta_folders_by_number(con->http.session->user->folders, dst_folder))) {
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_MESSAGES_MOVE, "Invalid folder reference.");
	}
	else {

		for (uint64_t i = 0; i < count && commit; i++) {

			// Confirm the message ID is a number, that the message exists and that the message is in the source folder.
			if (!json_is_integer(json_array_get_d(messages, i)) || !(key.val.u64 = json_integer_value_d(json_array_get_d(messages, i))) ||
				!(active = inx_find(con->http.session->user->messages, key)) || active->foldernum != src_folder) {
				portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_MESSAGES_MOVE, "Invalid message reference.");
				commit = false;
			}
			else if ((result = meta_messages_mover(con->http.session->user, active, dst_folder, false, false, META_LOCKED)) != 1) {

				if (!result) {
					portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_MESSAGES_MOVE,
						"Invalid message reference.");
				}
				else {
					portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
				}

				commit = false;
			}

		}

		// If any messages are moved to a different folder we'll need to update the sequence numbers to reflect the new status.
		meta_messages_update_sequences(con->http.session->user->folders, con->http.session->user->messages);

		if (commit) {

			// And finally, increment the serial number. If the serial number indicates no outside changes we can increment it without forcing a refresh.
			if (con->http.session->user->serials.messages == serial_get(OBJECT_MESSAGES, con->http.session->user->usernum)) {
				con->http.session->user->serials.messages = serial_increment(OBJECT_MESSAGES, con->http.session->user->usernum);
			}
			// Increment the reference counter and queue a session update.
			else {
				serial_increment(OBJECT_MESSAGES,con->http.session->user->usernum);
				sess_trigger(con->http.session);
			}

			portal_endpoint_response(con, "{s:s, s:{s:s}, s:I}", "jsonrpc", "2.0", "result", "messages.move", "success", "id", con->http.portal.id);
		}
		/// HIGH: If a multiple message copy fails in the middle, go back and remove any messages that may have been copied before the error.
		// else {
		// Insert reversal logic here.
		// }
	}

	meta_user_unlock(con->http.session->user);

	return;
}

/**
 * @brief	Remove a user's mail message in response to a "messages.reomve" json-rpc portal request.
 * @param	con		a pointer to the connection object across which the json-rpc response will be sent.
 * @return	This function returns no value.
 */
void portal_endpoint_messages_remove(connection_t *con) {

	json_error_t err;
	bool_t commit = true;
	meta_message_t *active;
	json_t *messages;
	uint64_t folder, count;
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = 0 };

	// Check the session state. Method has 2 parameters.
	if (!portal_validate_request (con, PORTAL_ENDPOINT_ERROR_MESSAGES_REMOVE, "messages.remove", true, 2)) {
		return;
	}
	// Validate the request format and extract the submitted values.
	else if (json_unpack_ex_d(con->http.portal.params, &err, JSON_STRICT, "{s:I, s:o}", "folderID", &folder, "messageIDs", &messages)) {
		log_pedantic("Received invalid portal messages remove request parameters { user = %.*s, errmsg = %s }",
			(int)st_length_get(con->http.session->user->username), st_char_get(con->http.session->user->username), err.text);
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	} else if (!json_is_array(messages) || !(count = json_array_size_d(messages))) {
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	}

	meta_user_wlock(con->http.session->user);

	// Confirm the source and target folder IDs are legit and that the source and target folder IDs are not identical.
	if (!folder || !(meta_folders_by_number(con->http.session->user->folders, folder))) {
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_MESSAGES_REMOVE, "Invalid folder reference.");
	}
	else {

		for (uint64_t i = 0; i < count && commit; i++) {

			// Confirm the message ID is a number, that the message exists and that the message is in the source folder.
			if (!json_is_integer(json_array_get_d(messages, i)) || !(key.val.u64 = json_integer_value_d(json_array_get_d(messages, i))) ||
				!(active = inx_find(con->http.session->user->messages, key)) || active->foldernum != folder) {
				portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_MESSAGES_REMOVE, "Invalid message reference.");
				commit = false;
			}
			else if (!mail_remove_message(con->http.session->user->usernum, active->messagenum, active->size, active->server)) {
				portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
				commit = false;
			} else {
				// Remove the message from the mailbox context.
				inx_delete(con->http.session->user->messages, key);
			}

		}

		// If any messages are moved to a different folder we'll need to update the sequence numbers to reflect the new status.
		meta_messages_update_sequences(con->http.session->user->folders, con->http.session->user->messages);

		if (commit) {

			// And finally, increment the serial number. If the serial number indicates no outside changes we can increment it without forcing a refresh.
			if (con->http.session->user->serials.messages == serial_get(OBJECT_MESSAGES, con->http.session->user->usernum)) {
				con->http.session->user->serials.messages = serial_increment(OBJECT_MESSAGES, con->http.session->user->usernum);
			}
			// Increment the reference counter and queue a session update.
			else {
				serial_increment(OBJECT_MESSAGES,con->http.session->user->usernum);
				sess_trigger(con->http.session);
			}

			portal_endpoint_response(con, "{s:s, s:{s:s}, s:I}", "jsonrpc", "2.0", "result", "messages.remove", "success", "id", con->http.portal.id);
		}
		//	TODO: How should message failure be handled? There's basically three options:
		//	1: Try to delete all messages, and notify the user if there were any failure(s).
		//	2: Attempt to delete all messages, and abort immediately if and when a failure is encountered.
		//	3: Attempt to delete all messages, and if any failures are encountered, roll back the entire batch operation.
		//	At the moment we have chosen #2, but this could change in the future.
		//  QUESTION: What is the best response below?
		else {
			portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		}

	}

	meta_user_unlock(con->http.session->user);

	return;
}

/**
 * @brief	Get all of the message tags used by a specified user in response to a "messages.tags" json-rpc portal request.
 * @param	con		a pointer to the connection object across which the json-rpc response will be sent.
 * @return	This function returns no value.
 */
void portal_endpoint_messages_tags(connection_t *con) {

	inx_t *list;
	stringer_t *active;
	inx_cursor_t *cursor;
	json_t *tags;

	// TODO: This is definitely going to need to support caching in the future.

	// Check the session state. Method has no parameters.
	if (!portal_validate_request (con, PORTAL_ENDPOINT_ERROR_MESSAGES_TAGS, "messages.tags", false, 0)) {
		return;
	}

	if (!(tags  = json_array_d())) {
		log_error("Unable to allocate space for json array in messages.tags portal response.");
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}

	if (!(list = meta_data_fetch_all_tags(con->http.session->user->usernum))) {
		log_pedantic("Failed to fetch user message tags.");
		json_decref_d(tags);
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}

	if ((cursor = inx_cursor_alloc(list))) {

		while ((active = inx_cursor_value_next(cursor))) {
			json_array_append_new_d(tags, json_string_d(st_char_get(active)));
		}

		inx_cursor_free(cursor);

	} else {
		json_decref_d(tags);
		inx_free(list);
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}

	inx_free(list);
	portal_endpoint_response(con, "{s:s, s:o, s:I}", "jsonrpc", "2.0", "result", tags, "id", con->http.portal.id);
}

/**
 * @brief	Perform a tag operation on a user's message, in response to a "messages.tag" json-rpc portal request.
 * @param	con		a pointer to the connection object across which the json-rpc response will be sent.
 * @return	This function returns no value.
 */
void portal_endpoint_messages_tag(connection_t *con) {

	int_t action;
	json_error_t err;
	inx_t *list = NULL;
	chr_t *method = NULL;
	bool_t commit = true;
	inx_cursor_t *cursor = NULL;
	meta_message_t *active = NULL;
	uint64_t folder = 0, mess_count = 0, tag_count = 0;
	json_t *tags = NULL, *messages, *collection, *entry;
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = 0 };

	// Check the session state. Method has 3 or 4 parameters.
	if (!portal_validate_request (con, PORTAL_ENDPOINT_ERROR_MESSAGES_TAG, "messages.tag", true, 0)) {
		return;
	}
	// Validate the request format and extract the submitted values.
	else if (json_object_size_d(con->http.portal.params) == 4 && json_unpack_ex_d(con->http.portal.params, &err, JSON_STRICT,
		"{s:s, s:o, s:o, s:I}", "action", &method, "tags", &tags, "messageIDs", &messages, "folderID", &folder)) {
		log_pedantic("Received invalid portal messages tag request parameters { user = %.*s, errmsg = %s }",
			(int)st_length_get(con->http.session->user->username), st_char_get(con->http.session->user->username), err.text);
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	}
	else if (json_object_size_d(con->http.portal.params) == 3 && json_unpack_ex_d(con->http.portal.params, &err, JSON_STRICT,
		"{s:s, s:o, s:I}", "action", &method, "messageIDs", &messages, "folderID", &folder)) {
		log_pedantic("Received invalid portal messages tag request parameters { user = %.*s, errmsg = %s }",
			(int)st_length_get(con->http.session->user->username), st_char_get(con->http.session->user->username), err.text);
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	} else if (!json_is_array(messages) || !(mess_count = json_array_size_d(messages))) {
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	}


	// Parse the action.
	if (!st_cmp_ci_eq(NULLER(method), PLACER("add", 3))) {
		action = PORTAL_ENDPOINT_ACTION_ADD;
	}
	else if (!st_cmp_ci_eq(NULLER(method), PLACER("remove", 6))) {
		action = PORTAL_ENDPOINT_ACTION_REMOVE;
	}
	else if (!st_cmp_ci_eq(NULLER(method), PLACER("replace", 7))) {
		action = PORTAL_ENDPOINT_ACTION_REPLACE;
	}
	else if (!st_cmp_ci_eq(NULLER(method), PLACER("list", 4))) {
		action = PORTAL_ENDPOINT_ACTION_LIST;
	}
	else {
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_INVALID_KEYWORD | PORTAL_ENDPOINT_ERROR_MESSAGES_TAG, "Unrecognized action.");
		return;
	}

	// Ensure the tags array isn't empty for add/remove operations.
	if ((!tags && action != PORTAL_ENDPOINT_ACTION_LIST) || (tags && action == PORTAL_ENDPOINT_ACTION_LIST)) {
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	}
	else if (tags && !(tag_count = json_array_size_d(tags)) && (action == PORTAL_ENDPOINT_ACTION_ADD || action == PORTAL_ENDPOINT_ACTION_REMOVE)) {
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_ILLEGAL_COMBINATION | PORTAL_ENDPOINT_ERROR_MESSAGES_TAG, "The requested action does not accept an empty array of tags.");
		return;
	}

	if (action == PORTAL_ENDPOINT_ACTION_LIST) {
		meta_user_rlock(con->http.session->user);
	}
	else {
		meta_user_wlock(con->http.session->user);
	}

	// Confirm the source and target folder IDs are legit and that the source and target folder IDs are not identical.
	if (!folder || !(meta_folders_by_number(con->http.session->user->folders, folder))) {
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_MESSAGES_TAG, "Invalid folder reference.");
	}

	else if (!(list = inx_alloc(M_INX_LINKED, NULL))) {
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
	}
	else {

		for (uint64_t i = 0; i < mess_count && commit; i++) {

			// Confirm the message ID is a number, that the message exists and that the message is in the source folder.
			if (!json_is_integer(json_array_get_d(messages, i)) || !(key.val.u64 = json_integer_value_d(json_array_get_d(messages, i)))
				|| !(active = inx_find(con->http.session->user->messages, key)) || active->foldernum != folder) {
				portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_MESSAGES_TAG, "Invalid message reference.");
				commit = false;
			}
			else if (inx_insert(list, key, active) != true) {
				portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
				commit = false;
			}

		}

		if (commit && action != PORTAL_ENDPOINT_ACTION_LIST && (cursor = inx_cursor_alloc(list))) {

			/// LOW: We don't need to add the flag to every message. Just the ones that don't already have the flag.
			if (action == PORTAL_ENDPOINT_ACTION_ADD || (action == PORTAL_ENDPOINT_ACTION_REPLACE && tag_count)) {
				meta_data_flags_add(list, con->http.session->user->usernum, folder, MAIL_STATUS_TAGGED);
			}
			/// LOW: Like the line, were not checking whether the message even needs to have the flag removed. Were also not handling remove requests that result in a message having no tags.
			// If were replacing the tags, and the replacement set of flags is empty, we can remove the flag.
			else if (action == PORTAL_ENDPOINT_ACTION_REPLACE && !tag_count) {
				meta_data_flags_remove(list, con->http.session->user->usernum, folder, MAIL_STATUS_TAGGED);
			}

			while ((active = inx_cursor_value_next(cursor))) {

				/// If this is going supposed to be a replace operation, we truncate all of the message tags so we can insert the replacements below.
				if (action == PORTAL_ENDPOINT_ACTION_REPLACE) {
					meta_data_truncate_tags(active);
				}

				for (size_t i = 0; i < tag_count && commit; i++) {

					if (!json_is_string(json_array_get_d(tags, i))) {
						portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
						commit = false;
					}
					else if ((action == PORTAL_ENDPOINT_ACTION_ADD || action == PORTAL_ENDPOINT_ACTION_REPLACE) && meta_data_insert_tag(active,	NULLER((chr_t *)json_string_value_d(json_array_get_d(tags, i))))) {
						portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_MESSAGES_TAG, "Invalid tag reference.");
						commit = false;
					}
					else if (action == PORTAL_ENDPOINT_ACTION_REMOVE && meta_data_delete_tag(active, NULLER((chr_t *)json_string_value_d(json_array_get_d(tags, i))))) {
						portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_MESSAGES_TAG, "Invalid tag reference.");
						commit = false;
					}
				}

				/// TODO: This is ugly. Were rebuilding the tags array from the database on each iteration because its easier than trying to figure out which slot needs to be removed.
				if (active->tags) {
					ar_free(active->tags);
					active->tags = NULL;
				}

				meta_data_fetch_message_tags(active);
			}

			inx_cursor_free(cursor);

			/// TODO: If the update is triggered before the tagged flag is added to the database the refresh might not pull the data for who recently got tagged! We need to need to look
			/// for this type of sequence bug elsewhere too.
			meta_user_serial_check(con->http.session->user, OBJECT_MESSAGES);


			// Commit should only be true if there were no errors. Which means we also still need to output a success confirmation.
			if (commit) {
				portal_endpoint_response(con, "{s:s, s:{s:s}, s:I}", "jsonrpc", "2.0", "result", "messages.tag", "success", "id", con->http.portal.id);
			}
		}
		else if (commit && action == PORTAL_ENDPOINT_ACTION_LIST && (collection = json_array_d())) {

			if ((cursor = inx_cursor_alloc(list))) {
				while ((active = inx_cursor_value_next(cursor))) {

					if ((tags = json_array_d()) && active->tags && (mess_count = ar_length_get(active->tags))) {
						for (uint64_t i = 0; i < mess_count; i++) {
							json_array_append_new_d(tags, json_string_d(st_char_get(ar_field_st(active->tags, i))));
						}
					}

					if (!(entry = json_pack_ex_d(&err, JSON_ENSURE_ASCII, "{s:I, s:o}", "messageID", active->messagenum, "tags", tags))) {
						log_pedantic("Message flag packing attempt failed. { error = %s }", err.text);
					}
					else if (json_array_append_new_d(collection, entry)) {
						log_pedantic("The message object could not be appended to the result list. { error = %s }", err.text);
						json_decref_d(entry);
					}
				}
				inx_cursor_free(cursor);
			}

			portal_endpoint_response(con, "{s:s, s:o, s:I}", "jsonrpc", "2.0", "result", collection, "id", con->http.portal.id);
		}
	}

	/// TODO: Were holding onto the user lock while were waiting on the response to be sent over the wire; and this function could probably use a rethink.
	meta_user_unlock(con->http.session->user);

	if (list) {
		inx_free(list);
	}

	return;
}

/***
 *
 * @link http://www.whatwg.org/specs/web-apps/current-work/multipage/the-iframe-element.html#attr-iframe-sandbox
 * @warning Setting both the allow-scripts and allow-same-origin keywords together when the embedded page has the same origin as the page containing the
 * 	iframe allows the embedded page to simply remove the sandbox attribute. Sandboxing hostile content is of minimal help if an attacker can convince the user
 * 	to just visit the hostile content directly, rather than in the iframe. To limit the damage that can be caused by hostile HTML content, it should be served
 * 	using the text/html-sandboxed MIME type.
 */
void portal_endpoint_messages_load(connection_t *con) {

	int_t ret;
	json_error_t err;
	uint32_t sections = 0;
	meta_message_t *active;
	uint64_t message;
	mail_message_t *data = NULL;
	json_t *array, *result, *value;
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = 0	};

	// Check the session state. Method has 2 parameters.
	if (!portal_validate_request (con, PORTAL_ENDPOINT_ERROR_MESSAGES_LOAD, "messages.load", true, 2)) {
		return;
	}
	// Validate the request format and extract the submitted values.
	else if (json_unpack_ex_d(con->http.portal.params, &err, JSON_STRICT, "{s:I, s:o}", "messageID", &message, "section", &array)) {
		//json_unpack_ex_d(con->http.portal.params, &err, JSON_STRICT, "{s:I, s:I, s:o}", "messageID", &message, "folderID", &folder, "sections", &array)) {
		log_pedantic("Received invalid portal mnessages load request parameters { user = %.*s, errmsg = %s }",
			(int)st_length_get(con->http.session->user->username), st_char_get(con->http.session->user->username), err.text);
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	}

	// See what sections were requested.
	else if ((ret = portal_parse_sections(array, &sections)) != 1) {
		switch (ret) {
			case (0):
				portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_ILLEGAL_COMBINATION | PORTAL_ENDPOINT_ERROR_MESSAGES_LOAD, "The sections array must not be empty.");
				break;
			case (-1):
				portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
				break;
			case (-2):
				portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_INVALID_KEYWORD | PORTAL_ENDPOINT_ERROR_MESSAGES_LOAD, "Unrecognized section requested.");
				break;
			default:
				portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
				break;
		}
		return;
	}
	// Find the message, and then validate the folder number.
	//else if (!(key.val.u64 = message) || !(active = inx_find(con->http.session->user->messages, key)) || active->foldernum != folder) {
	//else if (!(key.val.u64 = message) || !(active = inx_find(con->http.session->user->messages, key))) {
	if (!(key.val.u64 = message) || !(active = inx_find(con->http.session->user->messages, key))) {
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_MESSAGES_LOAD, "The requested message could not be located.");
		return;
	}
	else if ((sections & (PORTAL_ENDPOINT_MESSAGE_HEADER | PORTAL_ENDPOINT_MESSAGE_BODY | PORTAL_ENDPOINT_MESSAGE_ATTACHMENTS)) != 0 &&
		(!(data = mail_load_message(active, con->http.session->user, con->server, 0)) || mail_mime_update(data) != 1))  {
			if (data) {
				mail_destroy(data);
			}
			portal_endpoint_error(con, 500, PORTAL_ENDPOINT_ERROR_READ | PORTAL_ENDPOINT_ERROR_MESSAGES_LOAD, "An error occurred while trying to load the requested message data.");
			return;
	}
	else if ((result = json_object_d())) {
		if ((sections & PORTAL_ENDPOINT_MESSAGE_META) && (value = portal_message_meta(active))) {
			json_object_set_new_d(result, "meta", value);
		}
		if ((sections & PORTAL_ENDPOINT_MESSAGE_SOURCE) && (value = portal_message_source(active))) {
			json_object_set_new_d(result, "source", value);
		}
		if ((sections & PORTAL_ENDPOINT_MESSAGE_SECURITY) && (value = portal_message_security(active))) {
			json_object_set_new_d(result, "security", value);
		}
		if ((sections & PORTAL_ENDPOINT_MESSAGE_SERVER) && (value = portal_message_server(active))) {
			json_object_set_new_d(result, "server", value);
		}
		if ((sections & PORTAL_ENDPOINT_MESSAGE_HEADER) && (value = portal_message_header(active, data))) {
			json_object_set_new_d(result, "header", value);
		}
		if ((sections & PORTAL_ENDPOINT_MESSAGE_BODY) && (value = portal_message_body(active, data))) {
			json_object_set_new_d(result, "body", value);
		}
		if ((sections & PORTAL_ENDPOINT_MESSAGE_ATTACHMENTS) && (value = portal_message_attachments(active, data))) {
			json_object_set_new_d(result, "attachments", value);
		}
		if ((sections & PORTAL_ENDPOINT_MESSAGE_INFO) && (value = portal_message_info(active))) {
			json_object_set_new_d(result, "info", value);
		}
	}

	// If we had to load the message off the disk, cleanup here.
	if (data) {
		mail_destroy(data);
	}

	// If we end up with a NULL, or the result object ends up without any sections we assume there was an internal server error.
	if (!result || !json_object_size_d(result)) {
		if (result) json_decref_d(result);
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}

	portal_endpoint_response(con, "{s:s, s:o, s:I}", "jsonrpc", "2.0", "result", result, "id", con->http.portal.id);

	return;
}

/**
 * @brief	Compose a new message in response to a "messages.compose" json-rpc portal request.
 * @param	con		a pointer to the connection object across which the json-rpc response will be sent.
 * @return	This function returns no value.
 */
void portal_endpoint_messages_compose(connection_t *con) {

	json_t *object;
	json_error_t err;
	composition_t *comp;
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = 0 };

	// Check the session state. Method has no parameters.
	if (!portal_validate_request (con, PORTAL_ENDPOINT_ERROR_MESSAGES_COMPOSE, "messages.compose", false, 0)) {
		return;
	}

	if (!(object = json_object_d())) {
		log_error("Unable to allocate a JSON object for settings request.");
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}

	if (!(comp = mm_alloc(sizeof(composition_t)))) {
		log_error("Unable to allocate memory for new user message composition.");
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}

	// Get a unique composition ID.
	mutex_lock(&(con->http.session->lock));
	key.val.u64 = ++con->http.session->composed;

	// Better safe than sorry? Make sure the unique id isn't already taken.
	while (inx_find(con->http.session->compositions, key)) {
		key.val.u64++;
	}

	comp->comp_id = key.val.u64;

	if ((!(comp->attachments = inx_alloc (M_INX_LINKED, &sess_release_attachment))) || !inx_insert(con->http.session->compositions, key, comp)) {
		log_error("Unable to process new user message composition.");
		sess_release_composition(comp);
		mutex_unlock(&(con->http.session->lock));
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}

	mutex_unlock(&(con->http.session->lock));

	if (!(object = json_pack_ex_d(&err, JSON_ENSURE_ASCII, "{s:I}", "composeID", comp->comp_id))) {
		log_error("Unable to compose new user mail. {%s}", err.text);
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}

	portal_endpoint_response(con, "{s:s, s:o, s:I}", "jsonrpc", "2.0", "result", object, "id", con->http.portal.id);
	return;
}

/**
 * @brief	Add an attachment to a message being composed in response to an "attachments.add" json-rpc portal request.
 * @param	con		a pointer to the connection object across which the json-rpc response will be sent.
 * @return	This function returns no value.
 */
void portal_endpoint_attachments_add(connection_t *con) {

	json_error_t err;
	json_t *object;
	composition_t *comp;
	attachment_t *attachment;
	chr_t *filename;
	uint64_t cid;
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = 0 };

	// Check the session state. Method has 2 parameters.
	if (!portal_validate_request (con, PORTAL_ENDPOINT_ERROR_ATTACHMENTS_ADD, "attachments.add", true, 2)) {
		return;
	}
	// Validate the request format and extract the submitted values.
	else if (json_unpack_ex_d(con->http.portal.params, &err, JSON_STRICT, "{s:I, s:s}", "composeID", &cid, "filename", &filename)) {
		log_pedantic("Received invalid portal attachments add request parameters { user = %.*s, errmsg = %s }",
			(int)st_length_get(con->http.session->user->username), st_char_get(con->http.session->user->username), err.text);
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	}

	// Does the specified composition actually exist?
	mutex_lock(&(con->http.session->lock));
	key.val.u64 = cid;

	if (!(comp = inx_find(con->http.session->compositions, key))) {
		mutex_unlock(&(con->http.session->lock));
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_ATTACHMENTS_ADD, "The specified composition ID was not found.");
		return;
	}

	// Now add a new attachment to the composition that was just found.
	key.val.u64 = ++comp->attached;

	// Better safe than sorry? Make sure the unique attachment id isn't already taken.
	while (inx_find(comp->attachments, key)) {
		key.val.u64++;
	}

	if (!(attachment = mm_alloc(sizeof(attachment_t)))) {
		log_error("Unable to allocate space for new message attachment.");
		mutex_unlock(&(con->http.session->lock));
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}

	attachment->attach_id = key.val.u64;

	if ((!(attachment->filename = st_import(filename, ns_length_get(filename)))) || !inx_insert(comp->attachments, key, attachment)) {
		log_error("Unable to process new user message attachment.");
		mutex_unlock(&(con->http.session->lock));
		sess_release_attachment(attachment);
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}

	mutex_unlock(&(con->http.session->lock));

	if (!(object = json_object_d())) {
		log_error("Unable to allocate a JSON object for attachments add request.");
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	} else if (!(object = json_pack_ex_d(&err, JSON_ENSURE_ASCII, "{s:I}", "attachmentID", attachment->attach_id))) {
		log_error("Unable to pack attachments add response. {%s}", err.text);
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}

	portal_endpoint_response(con, "{s:s, s:o, s:I}", "jsonrpc", "2.0", "result", object, "id", con->http.portal.id);
	return;
}

/**
 * @brief	Remove an attachment from a message being composed in response to an "attachments.remove" json-rpc portal request.
 * @param	con		a pointer to the connection object across which the json-rpc response will be sent.
 * @return	This function returns no value.
 */
void portal_endpoint_attachments_remove(connection_t *con) {

	json_error_t err;
	composition_t *comp;
	attachment_t *attachment;
	uint64_t cid, aid;
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = 0 };

	// Check the session state. Method has 2 parameters.
	if (!portal_validate_request (con, PORTAL_ENDPOINT_ERROR_ATTACHMENTS_REMOVE, "attachments.remove", true, 2)) {
		return;
	}
	// Validate the request format and extract the submitted values.
	else if (json_unpack_ex_d(con->http.portal.params, &err, JSON_STRICT, "{s:I, s:I}", "composeID", &cid, "attachmentID", &aid)) {
		log_pedantic("Received invalid portal attachments remove request parameters { user = %.*s, errmsg = %s }",
			(int)st_length_get(con->http.session->user->username), st_char_get(con->http.session->user->username), err.text);
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	}

	// First make sure the composition is valid.
	mutex_lock(&(con->http.session->lock));
	key.val.u64 = cid;

	if (!(comp = inx_find(con->http.session->compositions, key))) {
		mutex_unlock(&(con->http.session->lock));
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_ATTACHMENTS_REMOVE, "The specified composition ID was not found.");
		return;
	}

	// Then make sure the specified attachment exists.
	key.val.u64 = aid;

	if (!(attachment = inx_find(comp->attachments, key))) {
		mutex_unlock(&(con->http.session->lock));
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_ATTACHMENTS_REMOVE, "The specified attachment ID was not found.");
		return;
	}

	if (!inx_delete(comp->attachments, key)) {
		log_error("Unable to remove attachment from composition.");
		mutex_unlock(&(con->http.session->lock));
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}

	mutex_unlock(&(con->http.session->lock));
	portal_endpoint_response(con, "{s:s, s:b, s:I}", "jsonrpc", "2.0", "result", true, "id", con->http.portal.id);
	return;
}

/**
 * @brief	Send a composed message in response to a "messages.send" json-rpc portal request.
 * @param	con		a pointer to the connection object across which the json-rpc response will be sent.
 * @return	This function returns no value.
 */
void portal_endpoint_messages_send(connection_t *con) {

	json_error_t err;
	json_t *attachments, *to, *cc, *bcc, *body;
	composition_t *comp;
	inx_t *tos, *ccs, *bccs;
	uint64_t compose_id;
	stringer_t *newbody;
	chr_t *from, *subject, *priority, *body_plain, *body_html, *errmsg = "OK";
	size_t nto, ncc, nbcc, nrecipients;
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = 0 };

	// Check the session state. Method has 9 parameters.
	if (!portal_validate_request (con, PORTAL_ENDPOINT_ERROR_MESSAGES_SEND, "messages.send", true, 9)) {
		return;
	}
	// Validate the request format and extract the submitted values.
	if (json_unpack_ex_d(con->http.portal.params, &err, JSON_STRICT, "{s:I, s:s, s:o, s:o, s:o, s:s, s:s, s:o, s:o}", "composeID", &compose_id, "from", &from,
		"to", &to, "cc", &cc, "bcc", &bcc, "subject", &subject, "priority", &priority, "attachments", &attachments, "body", &body)) {
		log_pedantic("Received invalid portal messages send request parameters { user = %.*s, errmsg = %s }",
			(int)st_length_get(con->http.session->user->username), st_char_get(con->http.session->user->username), err.text);
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	}

	// The body parameter must be an object containing two pieces of data: the plain text and html bodies of the message.
	else if (!json_is_object(body) || (json_object_size_d(body) != 2) ||
			json_unpack_ex_d(body, &err, JSON_STRICT, "{s:s, s:s}", "text", &body_plain, "html", &body_html)) {
		log_pedantic("Received invalid body parameters for portal message send request.");
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	}

	key.val.u64 = compose_id;

	// Make sure the specified composition id is valid.
	if (!(comp = inx_find(con->http.session->compositions, key))) {
		log_pedantic("User attempted to send message with invalid composition ID.");
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_MESSAGES_SEND, "Invalid send request. Please compose a new message.");
		return;
	}

	// TODO: These all return 400 errors, but they could also be caused by 500 internal errors.
	// Transform the To:, CC:, and BCC: fields from json arrays into inx holders containing managed strings.
	if (!(tos = portal_parse_json_str_array(to, &nto))) {
		log_pedantic("Portal request contained invalid To: field");
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_ILLEGAL_COMBINATION | PORTAL_ENDPOINT_ERROR_MESSAGES_SEND, "Invalid To: field specified in message send request.");
		return;
	} else if (!(ccs = portal_parse_json_str_array(cc, &ncc))) {
		log_pedantic("Portal request contained invalid CC: field");
		inx_free(tos);
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_ILLEGAL_COMBINATION | PORTAL_ENDPOINT_ERROR_MESSAGES_SEND, "Invalid CC: field specified in message send request.");
		return;
	} else if (!(bccs = portal_parse_json_str_array(bcc, &nbcc))) {
		log_pedantic("Portal request contained invalid BCC: field");
		inx_free(tos);
		inx_free(ccs);
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_ILLEGAL_COMBINATION | PORTAL_ENDPOINT_ERROR_MESSAGES_SEND, "Invalid BCC: field specified in message send request.");
		return;
	}

	nrecipients = nto + ncc + nbcc;

	// Perform some basic security checks and validation on the email.
	if (!portal_outbound_checks(con->http.session->user->usernum, con->http.session->user->username, con->http.session->user->verification, NULLER(from), nrecipients, NULLER(body_plain), NULLER(body_html), &errmsg)) {
		log_pedantic("User failed outbound checks for sending mail through portal: {%s}", errmsg);
		inx_free(tos);
		inx_free(ccs);
		inx_free(bccs);
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_MESSAGES_SEND, "Your email could not be sent.");
		return;
	}

	// Create one giant blob that will be passed to the SMTP DATA command.
	if (!(newbody = portal_smtp_create_data(comp->attachments, NULLER(from), tos, ccs, bccs, NULLER(subject), NULLER(body_plain), NULLER(body_html)))) {
		inx_free(tos);
		inx_free(ccs);
		inx_free(bccs);
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}

	// Relay our message.
	if (!portal_smtp_relay_message(NULLER(from), tos, newbody, st_length_get(newbody), &errmsg)) {
		log_pedantic("User was unable to send email through portal: {%s}", errmsg);
		st_free(newbody);
		inx_free(tos);
		inx_free(ccs);
		inx_free(bccs);
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_MESSAGES_SEND, "Your email could not be sent.");
		return;
	}

	inx_free(tos);
	inx_free(ccs);
	inx_free(bccs);
	st_free(newbody);

	// We sent the message successfully... so blow away the composition.
	mutex_lock(&(con->http.session->lock));
	key.val.u64 = compose_id;

	if (!inx_delete(con->http.session->compositions, key)) {
		log_error("Unable to delete sent composition.");
	}

	mutex_unlock(&(con->http.session->lock));

	portal_endpoint_response(con, "{s:s, s:s, s:I}", "jsonrpc", "2.0", "result", errmsg, "id", con->http.portal.id);

	return;
}

/**
 * @note	If the source and target folder are equal, a copy will be made with a different name.
 */
void portal_endpoint_contacts_copy(connection_t *con) {

	json_error_t err;
	inx_cursor_t *cursor;
	contact_detail_t *detail;
	contact_t *contact, *copy;
	stringer_t *name, *modified = NULL;
	contact_folder_t *folder, *target;
	uint64_t src_folder, dst_folder, contactnum;
	multi_t multi = { .type = M_TYPE_UINT64, .val.u64 = 0 };

	// Check the session state. Method has 3 parameters.
	if (!portal_validate_request (con, PORTAL_ENDPOINT_ERROR_CONTACTS_COPY, "contacts.copy", true, 3)) {
		return;
	}
	// Validate the request format and extract the submitted values.
	else if (json_unpack_ex_d(con->http.portal.params, &err, JSON_STRICT, "{s:I, s:I, s:I}", "contactID", &contactnum, "sourceFolderID",
		&src_folder, "targetFolderID", &dst_folder)) {
		log_pedantic("Received invalid portal contacts copy request parameters { user = %.*s, errmsg = %s }",
			(int)st_length_get(con->http.session->user->username), st_char_get(con->http.session->user->username), err.text);
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	} else if (!src_folder || !dst_folder) {
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	}

	meta_user_rlock(con->http.session->user);

	if (!(folder = magma_folder_find_number(con->http.session->user->contacts, src_folder))) {
		meta_user_unlock(con->http.session->user);
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_CONTACTS_COPY, "The source folder number is invalid.");
		return;
	} else if (!(target = magma_folder_find_number(con->http.session->user->contacts, dst_folder))) {
		meta_user_unlock(con->http.session->user);
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_CONTACTS_COPY, "The target folder number is invalid.");
		return;
	}
	else if (!(contact = contact_find_number(folder, contactnum))) {
		meta_user_unlock(con->http.session->user);
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_CONTACTS_COPY, "The requested contact number is invalid.");
		return;
	}

	// We munge the name if the source and target are the same.
	if (folder->foldernum == target->foldernum) {

		if (!(modified = st_merge("ss", PLACER("Copy of ", 8), contact->name))) {
			log_error("Could not create named copy of user contact entry.");
			meta_user_unlock(con->http.session->user);
			portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
			return;
		}

		name = modified;
	}
	else {
		name = contact->name;
	}

	// We store the contact number using the multi variable here so that we can free the contact if an error occurs and still return the correct contact number.
	if (!(copy = contact_create(con->http.session->user->usernum, target->foldernum, name)) || !(multi.val.u64 = copy->contactnum)) {
		log_pedantic("Could not create duplicate contact entry.");
		meta_user_unlock(con->http.session->user);

		// The only way we can reach this point with contact holding a pointer value is if the second boolean expression fails: contactnum == 0.
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_CONSTRAINT_VIOLATION | PORTAL_ENDPOINT_ERROR_CONTACTS_COPY, "The contact name is invalid.");
		if (copy) contact_free(copy);
		st_cleanup(modified);
		return;
	}

	// Once the record has been created we look through and duplicate all of the contact details.
	else if ((cursor = inx_cursor_alloc(contact->details))) {

		while ((detail = inx_cursor_value_next(cursor))) {
			contact_edit(copy, con->http.session->user->usernum, target->foldernum, detail->key, detail->value);
		}

		inx_cursor_free(cursor);
	}

	meta_user_unlock(con->http.session->user);
	meta_user_wlock(con->http.session->user);

	if (copy && (!(target = magma_folder_find_number(con->http.session->user->contacts, dst_folder)) || !inx_insert(target->records, multi, copy))) {
		contact_free(copy);
	}
	else {
		sess_serial_check(con->http.session, OBJECT_CONTACTS);
	}

	meta_user_unlock(con->http.session->user);
	portal_endpoint_response(con, "{s:s, s:{s:I}, s:I}", "jsonrpc", "2.0", "result", "contactID", multi.val.u64, "id", con->http.portal.id);

	// If we had to modified the name, make sure that buffer is freed.
	st_cleanup(modified);

	return;
}

// TODO: Eventually update this to allow multiple entry parameters?
/**
 * @brief	Move a user's contact entry to another contacts folder in response to a "contacts.move" json-rpc portal request.
 * @param	con		a pointer to the connection object across which the json-rpc response will be sent.
 * @return	This function returns no value.
 */
void portal_endpoint_contacts_move(connection_t *con) {

	int_t status = 0;
	json_error_t err;
	contact_t *contact;
	contact_folder_t *folder, *target;
	uint64_t src_folder, dst_folder, contactnum;

	// Check the session state. Method has 3 parameters.
	if (!portal_validate_request (con, PORTAL_ENDPOINT_ERROR_CONTACTS_MOVE, "contacts.move", true, 3)) {
		return;
	}
	// Validate the request format and extract the submitted values.
	else if (json_unpack_ex_d(con->http.portal.params, &err, JSON_STRICT, "{s:I, s:I, s:I}", "contactID", &contactnum,
		"sourceFolderID", &src_folder, "targetFolderID", &dst_folder)) {
		log_pedantic("Received invalid portal contacts move request parameters { user = %.*s, errmsg = %s }",
			(int)st_length_get(con->http.session->user->username), st_char_get(con->http.session->user->username), err.text);
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	} else if (!src_folder || !dst_folder) {
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	}
	else if (src_folder == dst_folder) {
		meta_user_unlock(con->http.session->user);
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_ILLEGAL_COMBINATION | PORTAL_ENDPOINT_ERROR_CONTACTS_MOVE, "The source and destination contacts folders must be different.");
		return;
	}

	meta_user_wlock(con->http.session->user);

	if (!(folder = magma_folder_find_number(con->http.session->user->contacts, src_folder)) || !(target = magma_folder_find_number(con->http.session->user->contacts, dst_folder))) {
		meta_user_unlock(con->http.session->user);
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_CONTACTS_MOVE, "The requested folder number is invalid.");
		return;
	}
	else if (!(contact = contact_find_number(folder, contactnum))) {
		meta_user_unlock(con->http.session->user);
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_CONTACTS_MOVE, "The requested contact number is invalid.");
		return;
	}

	// Delete the contact from the database and remove it from the user's context.
	else if ((status = contact_move(folder, target, contact, con->http.session->user->usernum)) == 1) {
		sess_serial_check(con->http.session, OBJECT_CONTACTS);
	}

	meta_user_unlock(con->http.session->user);

	if (status != 1) {
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}

	portal_endpoint_response(con, "{s:s, s:{s:s}, s:I}", "jsonrpc", "2.0", "result", "contacts.move", "success", "id", con->http.portal.id);
	return;
}

/**
 * @brief	Remove a user's contact entry in response to a "contacts.remove" json-rpc portal request.
 * @param	con		a pointer to the connection object across which the json-rpc response will be sent.
 * @return	This function returns no value.
 */
void portal_endpoint_contacts_remove(connection_t *con) {

	int_t status = 0;
	json_error_t err;
	contact_t *contact;
	contact_folder_t *folder;
	uint64_t foldernum, contactnum;
	multi_t multi = { .type = M_TYPE_UINT64, .val.u64 = 0 };

	// Check the session state. Method has 2 parameters.
	if (!portal_validate_request (con, PORTAL_ENDPOINT_ERROR_CONTACTS_REMOVE, "contacts.remove", true, 2)) {
		return;
	}
	// Validate the request format and extract the submitted values.
	else if ((json_unpack_ex_d(con->http.portal.params, &err, JSON_STRICT, "{s:I, s:I}", "folderID", &foldernum, "contactID", &contactnum))) {
		log_pedantic("Received invalid portal contacts remove request parameters { user = %.*s, errmsg = %s }",
			(int)st_length_get(con->http.session->user->username), st_char_get(con->http.session->user->username), err.text);
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	}

	meta_user_wlock(con->http.session->user);

	if (!(folder = magma_folder_find_number(con->http.session->user->contacts, foldernum))) {
		meta_user_unlock(con->http.session->user);
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_CONTACTS_REMOVE, "The requested folder number is invalid.");
		return;
	}
	else if (!(contact = contact_find_number(folder, contactnum))) {
		meta_user_unlock(con->http.session->user);
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_CONTACTS_REMOVE, "The requested contact number is invalid.");
		return;
	}

	// Delete the contact from the database and remove it from the user's context.
	else if ((status = contact_delete(contact->contactnum, con->http.session->user->usernum, foldernum)) >= 0 && (multi.val.u64 = contact->contactnum) && inx_delete(folder->records, multi)) {
		sess_serial_check(con->http.session, OBJECT_CONTACTS);
	}

	meta_user_unlock(con->http.session->user);

	if (status < 0) {
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}

	portal_endpoint_response(con, "{s:s, s:{s:s}, s:I}", "jsonrpc", "2.0", "result", "contacts.remove", "success", "id", con->http.portal.id);
	return;
}

/**
 * @brief	List a user's contacts in response to a "contacts.list" json-rpc portal request.
 * @param	con		a pointer to the connection object across which the json-rpc response will be sent.
 * @return	This function returns no value.
 */
void portal_endpoint_contacts_list(connection_t *con) {

	json_error_t err;
	uint64_t foldernum;
	contact_t *contact;
	inx_cursor_t *cursor, *dcursor;
	json_t *list, *entry;
	contact_folder_t *active;
	contact_detail_t *detail;
	//stringer_t *detail_email = NULL, *detail_company = NULL;
	//stringer_t *s_email = PLACER("email",5), *s_company = PLACER("company",7);

	// Check the session state. Method has 1 parameter.
	if (!portal_validate_request (con, PORTAL_ENDPOINT_ERROR_CONTACTS_LIST, "contacts.list", true, 1)) {
		return;
	}
	// Validate the request format and extract the submitted values.
	else if ((json_unpack_ex_d(con->http.portal.params, &err, JSON_STRICT, "{s:I}", "folderID", &foldernum))) {
		log_pedantic("Received invalid portal contacts list request parameters { user = %.*s, errmsg = %s }",
			(int)st_length_get(con->http.session->user->username), st_char_get(con->http.session->user->username), err.text);
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	}
	// Find the specified folder and prepare json folder list.
	else if (!(active = magma_folder_find_number(con->http.session->user->contacts, foldernum))) {
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_CONTACTS_LIST,	"The requested folder number is invalid.");
		return;
	}
	else if (!(list = json_array_d())) {
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}

	// Return a list of the contact's detail key-value pairs.
    else if ((cursor = inx_cursor_alloc(active->records)))  {

            while ((contact = inx_cursor_value_next(cursor))) {
                    if (!(entry = json_pack_ex_d(&err, JSON_ENSURE_ASCII, "{s:I, s:s}", "contactID", contact->contactnum, "name", st_char_get(contact->name)))) {
                            log_pedantic("Contact entry packing attempt failed. { error = %s }", err.text);
                    }
                    else if (json_array_append_new_d(list, entry)) {
                            log_pedantic("The message object could not be appended to the result list. { error = %s }", err.text);
                            json_decref_d(entry);
                    }

        			if ((dcursor = inx_cursor_alloc (contact->details))) {

        				while ((detail = inx_cursor_value_next(dcursor))) {
        					json_object_set_new_d(entry, st_char_get(detail->key), json_string_d(st_char_get(detail->value)));

        				}

        				inx_cursor_free(dcursor);
        			}

            }

            inx_cursor_free(cursor);
    }

    portal_endpoint_response(con, "{s:s, s:o, s:I}", "jsonrpc", "2.0", "result", list, "id", con->http.portal.id);
    return;
}

/**
 * @brief	Add a contact in response to a "contacts.add" json-rpc portal request.
 * @param	con		a pointer to the connection object across which the json-rpc response will be sent.
 * @return	This function returns no value.
 */
void portal_endpoint_contacts_add(connection_t *con) {

	void *iter;
	int_t status = 0;
	json_error_t err;
	uint64_t foldernum;
	contact_t *contact, *active;
	chr_t *key, *content, *errmsg;
	contact_folder_t *folder;
	json_t *pairs, *name, *value;
	multi_t multi = { .type = M_TYPE_UINT64, .val.u64 = 0 };
	inx_cursor_t *cursor;

	// Check the session state. Method has 2 parameters.
	if (!portal_validate_request (con, PORTAL_ENDPOINT_ERROR_CONTACTS_ADD, "contacts.add", true, 2)) {
		return;
	}
	// Validate the request format and extract the submitted values.
	else if (json_unpack_ex_d(con->http.portal.params, &err, JSON_STRICT, "{s:I, s:o}", "folderID", &foldernum, "contact", &pairs)) {
		log_pedantic("Received invalid portal contacts add request parameters { user = %.*s, errmsg = %s }",
			(int)st_length_get(con->http.session->user->username), st_char_get(con->http.session->user->username), err.text);
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	} else if (!json_is_object(pairs)) {
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	}

	meta_user_rlock(con->http.session->user);

	if (!(folder = magma_folder_find_number(con->http.session->user->contacts, foldernum))) {
		meta_user_unlock(con->http.session->user);
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_CONTACTS_ADD, "The requested folder number is invalid.");
		return;
	}

	meta_user_unlock(con->http.session->user);

	// Ensure a name was provided.
	if (!(name = json_object_get_d(pairs, "name")) || !json_is_string(name)) {
		log_pedantic("Invalid JSON request submitted to the Portal Camelface. { method != string / type = %s }", json_type_string_d(name));
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	}
	// Make sure that the name was valid.
	else if (!contact_validate_name(NULLER((chr_t *)json_string_value_d(name)), &errmsg)) {
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_CONSTRAINT_VIOLATION | PORTAL_ENDPOINT_ERROR_CONTACTS_ADD, errmsg);
		return;
	}

	// Also make sure that the contact doesn't already exist.
	if ((cursor = inx_cursor_alloc(folder->records))) {

		while ((active = inx_cursor_value_next(cursor))) {

			if (!st_cmp_cs_eq(active->name,NULLER((chr_t *)json_string_value_d(name)))) {
				inx_cursor_free(cursor);
				portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_CONSTRAINT_VIOLATION | PORTAL_ENDPOINT_ERROR_CONTACTS_ADD, "Could not add duplicate contact to database.");
				return;
			}
		}

		inx_cursor_free(cursor);
	}

	// We store the contact number using the multi variable here so that we can free the contact if an error occurs and still return the correct contact number.
	if (!(contact = contact_create(con->http.session->user->usernum, folder->foldernum, NULLER((chr_t *)json_string_value_d(name)))) || !(multi.val.u64 = contact->contactnum)) {

		// The only way we can reach this point with contact holding a pointer value is if the second boolean expression fails: contactnum == 0.
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_CONSTRAINT_VIOLATION | PORTAL_ENDPOINT_ERROR_CONTACTS_ADD, "Contact entry could not be added to database.");
		if (contact) contact_free(contact);
		return;
	}

	iter = json_object_iter_d(pairs);

	/// HIGH: We aren't checking/validating the contact details! And we should probably do our own duplication checks to avoid placing unnecessary load on the database.
	while (iter && status >= 0) {

		if ((key = (chr_t *)json_object_iter_key_d(iter)) && (value = json_object_iter_value_d(iter)) && json_is_string(value) &&
    	(content = (chr_t *)json_string_value_d(value)) && st_cmp_ci_eq(NULLER(key), PLACER("name", 4))) {

			if (!contact_validate_detail(NULLER(key), NULLER(content), &errmsg)) {
				portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_CONSTRAINT_VIOLATION | PORTAL_ENDPOINT_ERROR_CONTACTS_ADD, errmsg);
				contact_free(contact);
				return;
			}

			status =  contact_edit(contact, con->http.session->user->usernum, foldernum, NULLER(key), NULLER(content));
		}

		iter = json_object_iter_next_d(pairs, iter);
	}

	if (status < 0) {
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		contact_free(contact);
		return;
	}

	meta_user_wlock(con->http.session->user);

	// Since we released the read lock above were playing it safe and looking for the folder context again just to be sure.
	if (!(folder = magma_folder_find_number(con->http.session->user->contacts, foldernum)) || !inx_insert(folder->records, multi, contact)) {
		contact_free(contact);
	}
	else {
		sess_serial_check(con->http.session, OBJECT_CONTACTS);
	}

	meta_user_unlock(con->http.session->user);

	portal_endpoint_response(con, "{s:s, s:{s:I}, s:I}", "jsonrpc", "2.0", "result", "contactID", multi.val.u64, "id", con->http.portal.id);
	return;
}

void portal_endpoint_contacts_edit(connection_t *con) {

	void *iter;
	int_t status = 0;
	json_error_t err;
	contact_t *contact;
	chr_t *key, *content;
	json_t *pairs, *value;
	contact_folder_t *folder;
	uint64_t foldernum, contactnum;

	// Check the session state. Method has 3 parameters.
	if (!portal_validate_request (con, PORTAL_ENDPOINT_ERROR_CONTACTS_EDIT, "contacts.edit", true, 3)) {
		return;
	}
	// Validate the request format and extract the submitted values.
	else if (json_unpack_ex_d(con->http.portal.params, &err, JSON_STRICT, "{s:I, s:I, s:o}", "folderID", &foldernum, "contactID", &contactnum, "contact", &pairs)) {
		log_pedantic("Received invalid portal contacts edit request parameters { user = %.*s, errmsg = %s }",
			(int)st_length_get(con->http.session->user->username), st_char_get(con->http.session->user->username), err.text);
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	}
	else if (!json_is_object(pairs)) {
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	}
	else if (!json_object_size_d(pairs)) {
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_ILLEGAL_COMBINATION | PORTAL_ENDPOINT_ERROR_CONTACTS_EDIT, "This method does not accept an empty collection of contact details.");
		return;
	}

	meta_user_rlock(con->http.session->user);

	if (!(folder = magma_folder_find_number(con->http.session->user->contacts, foldernum))) {
		meta_user_unlock(con->http.session->user);
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_CONTACTS_EDIT, "The requested folder number is invalid.");
		return;
	}
	else if (!(contact = contact_find_number(folder, contactnum))) {
		meta_user_unlock(con->http.session->user);
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_CONTACTS_EDIT, "The requested contact number is invalid.");
		return;
	}

	meta_user_unlock(con->http.session->user);

	meta_user_wlock(con->http.session->user);
	iter = json_object_iter_d(pairs);

	/// HIGH: We aren't checking/validating the contact details! And we should probably do our own duplication checks to avoid placing unnecessary load on the database.
	while (iter && status >= 0) {
    if ((key = (chr_t *)json_object_iter_key_d(iter)) && (value = json_object_iter_value_d(iter)) && json_is_string(value) &&	(content = (chr_t *)json_string_value_d(value))) {
    	status = contact_edit(contact, con->http.session->user->usernum, foldernum, NULLER(key), NULLER(content));
		}

    iter = json_object_iter_next_d(pairs, iter);
	}

	sess_serial_check(con->http.session, OBJECT_CONTACTS);
	meta_user_unlock(con->http.session->user);

	if (status < 0) {
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}

	portal_endpoint_response(con, "{s:s, s:{s:s}, s:I}", "jsonrpc", "2.0", "result", "contacts.edit", "success", "id", con->http.portal.id);
	return;
}

/**
 * @brief	Retrieve all of a user's config options and return them to the remote client in response to a portal "contacts.load" json-rpc request.
 * @note	Requires a user to be authenticated; failure will be communicated to the client via a json response.
 * @param	con		a pointer to a connection object over which the json response will be sent.
 * @return	This function returns no value.
 */
void portal_endpoint_config_edit(connection_t *con) {

	void *iter;
	json_t *value;
	int_t status = 0;
	chr_t *key, *content;
	user_config_t *collection;

	// Check the session state. Method takes a variable number of parameters.
	if (!portal_validate_request (con, PORTAL_ENDPOINT_ERROR_CONFIG_EDIT, "config.edit", true, 0)) {
		return;
	}
/*	else if (!json_object_size_d(con->http.portal.params)) {
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_ILLEGAL_COMBINATION | PORTAL_ENDPOINT_ERROR_CONFIG_EDIT, "This method does not accept an empty collection of config entries.");
		return;
	} */

	// Create the request iterator and load the current config collection.
	if (!(iter = json_object_iter_d(con->http.portal.params)) || !(collection = user_config_create(con->http.session->user->usernum))) {
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}

	/// HIGH: We aren't checking/validating the config entries! And we should probably do our own duplication checks to avoid placing unnecessary load on the database.
	while (iter && status >= 0) {

		if ((key = (chr_t *)json_object_iter_key_d(iter)) && (value = json_object_iter_value_d(iter))) {

			if (json_is_string(value) && (content = (chr_t *)json_string_value_d(value))) {
				status = user_config_edit(collection, NULLER(key), NULLER(content));
			}
			// A special case for a null value, which means it should be deleted - but won't be detected by Jansson as a string.
			else if (json_is_null(value)) {
				status = user_config_edit(collection, NULLER(key), NULL);
			}

		}

		iter = json_object_iter_next_d(con->http.portal.params, iter);
	}

	// Were all done with the collection so we can release it.
	user_config_free(collection);

	if (status < 0) {
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}

	portal_endpoint_response(con, "{s:s, s:{s:s}, s:I}", "jsonrpc", "2.0", "result", "config.edit", "success", "id", con->http.portal.id);
	return;
}

/**
 * @brief	Retrieve all of a user's config options and return them to the remote client in response to a portal "contacts.load" json-rpc request.
 * @note	Requires a user to be authenticated; failure will be communicated to the client via a json response.
 * @param	con		a pointer to a connection object over which the json response will be sent.
 * @return	This function returns no value.
 */
void portal_endpoint_config_load(connection_t *con) {

	json_t *object;
	user_config_t *collection;

	// Check the session state. Method takes no parameters.
	if (!portal_validate_request (con, PORTAL_ENDPOINT_ERROR_CONFIG_LOAD, "config.load", false, 0)) {
		return;
	}

	// Create the request iterator and load the current config collection.
	else if (!(collection = user_config_create(con->http.session->user->usernum))) {
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}

	// Pack the config as a json object.
	else if (!(object = portal_config_collection(collection))) {
		user_config_free(collection);
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}

	// Were all done with the collection so we can release it.
	user_config_free(collection);

	portal_endpoint_response(con, "{s:s, s:o, s:I}", "jsonrpc", "2.0", "result", object, "id", con->http.portal.id);
	return;
}

/**
 * @brief	Return information for a portal "contacts.load" json-rpc request.
 * @note	This function returns the all the stored details about a specified contact entry.
 * @param	con		a pointer to the connection object across which the json-rpc response will be sent.
 * @return	This function returns no value.
 */
void portal_endpoint_contacts_load(connection_t *con) {

	json_t *entry;
	json_error_t err;
	contact_t *contact;
	contact_folder_t *active;
	uint64_t foldernum, contactnum;
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = 0 };

	// Check the session state. Method takes 2 parameters.
	if (!portal_validate_request (con, PORTAL_ENDPOINT_ERROR_CONTACTS_LOAD, "contacts.load", true, 2)) {
		return;
	}
	// Validate the request format and extract the submitted values.
	else if ((json_unpack_ex_d(con->http.portal.params, &err, JSON_STRICT, "{s:I, s:I}", "contactID", &contactnum, "folderID", &foldernum))) {
		log_pedantic("Received invalid portal contacts load request parameters { user = %.*s, errmsg = %s }",
			(int)st_length_get(con->http.session->user->username), st_char_get(con->http.session->user->username), err.text);
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	}

	else if (!(active = magma_folder_find_number(con->http.session->user->contacts, foldernum))) {
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_CONTACTS_LOAD, "The requested folder number is invalid.");
		return;
	}

	else if (!(key.val.u64 = contactnum) || !(contact = inx_find(active->records, key))) {
		portal_endpoint_error(con, 400, PORTAL_ENDPOINT_ERROR_REFERENCE | PORTAL_ENDPOINT_ERROR_CONTACTS_LOAD, "The requested contact number is invalid.");
		return;
	}

	else if (!(entry = portal_contact_details(contact))) {
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}

	portal_endpoint_response(con, "{s:s, s:o, s:I}", "jsonrpc", "2.0", "result", entry, "id", con->http.portal.id);
	return;
}

/**   ^ finished   /   work area   v     **/




/**   ^ work area   /   stubs   v    **/


/**
 * @brief	Return advertising information for a portal "ad" json-rpc request.
 * @note	This function is not implemented and may be removed entirely in the future.
 * @param	con		a pointer to the connection object across which the json-rpc response will be sent.
 * @return	This function returns no value.
 */
void portal_endpoint_ad(connection_t *con) {

	json_t *object;
	json_error_t err;
	chr_t *ad =	"data:image/png;base64,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";

	// Check the session state.
	if (!portal_validate_request(con, PORTAL_ENDPOINT_ERROR_AD, "ad", true, 1)) {
		return;
	}

	if (!(object = json_object_d())) {
		log_pedantic("Unable to allocate a JSON object for ad request.");
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}

	if (!(object = json_pack_ex_d(&err, JSON_ENSURE_ASCII, "{s:{s:s, s:s, s:{s:s, s:s} } }", "ad", "href",
		"/", "title", "Project Mascot", "img", "src", ad, "alt", "Project Mascot"))) {
		log_pedantic("Unable to construct advertisement response. {%s}", err.text);
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}

	portal_endpoint_response(con, "{s:s, s:o, s:I}", "jsonrpc", "2.0", "result", object, "id", con->http.portal.id);
	return;
}

void portal_endpoint_search(connection_t *con) {

	stringer_t *response;

	if ((response = st_aprint("{\"jsonrpc\": \"2.0\", \"error\": {\"code\": %i, \"message\": \"%s%s%s\"}, \"id\": %lu}\r\n",
		PORTAL_ENDPOINT_ERROR_SEARCH, "Method (", __FUNCTION__, ") is under construction.", con->http.portal.id))) {
		http_response_header(con, 200, PLACER("application/json; charset=utf-8", 31), st_length_get(response));
		con_write_st(con, response);
		st_free(response);
	}
	else {
		con->http.mode = HTTP_ERROR_500;
	}

	return;
}

/**
 * @note	This function is not implemented and may be removed entirely in the future.
 * @param	con		a pointer to the connection object across which the json-rpc response will be sent.
 * @return	This function returns no value.
 */
void portal_endpoint_scrape_add(connection_t *con) {

	// Check the session state.
	if (!portal_validate_request (con, PORTAL_ENDPOINT_ERROR_AD, "scrape.add", false, 0)) {
		return;
	}

	portal_endpoint_error(con, 404, JSON_RPC_2_ERROR_SERVER_METHOD_UNAVAIL, "Method is not implemented.");
	return;
}

/**
 * @note	This function is not implemented and may be removed entirely in the future.
 * @param	con		a pointer to the connection object across which the json-rpc response will be sent.
 * @return	This function returns no value.
 */
void portal_endpoint_scrape(connection_t *con) {

	// Check the session state.
	if (!portal_validate_request (con, PORTAL_ENDPOINT_ERROR_AD, "scape", false, 0)) {
		return;
	}

	portal_endpoint_error(con, 404, JSON_RPC_2_ERROR_SERVER_METHOD_UNAVAIL, "Method is not implemented.");
	return;
}

void portal_endpoint_attachments_progress(connection_t *con) {

	stringer_t *response;

	if ((response = st_aprint("{\"jsonrpc\": \"2.0\", \"error\": {\"code\": %i, \"message\": \"%s%s%s\"}, \"id\": %lu}\r\n",
		PORTAL_ENDPOINT_ERROR_ATTACHMENTS_PROGRESS, "Method (", __FUNCTION__, ") is under construction.", con->http.portal.id))) {
		http_response_header(con, 200, PLACER("application/json; charset=utf-8", 31), st_length_get(response));
		con_write_st(con, response);
		st_free(response);
	}
	else {
		con->http.mode = HTTP_ERROR_500;
	}

	return;
}

/**
 * @brief	Return information for a portal "settings.identity" json-rpc request.
 * @note	This function returns the full name, first name, last name, and website of the requested user.
 * @param	con		a pointer to the connection object across which the json-rpc response will be sent.
 * @return	This function returns no value.
 */
void portal_settings_identity(connection_t *con) {

	json_t *object;
	json_error_t err;

	// Check the session state. Method takes no parameters.
	if (!portal_validate_request (con, PORTAL_ENDPOINT_ERROR_SETTINGS_IDENTITY, "settings.identity", false, 0)) {
		return;
	}

	if (!(object = json_object_d())) {
		log_pedantic("Unable to allocate a JSON object for settings request.");
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}

	if (!(object = json_pack_ex_d(&err, JSON_ENSURE_ASCII, "{s:s, s:s, s:s, s:s}", "name", st_char_get(con->http.session->user->username), "first", "Firstname", "last", "Last", "website", "lavabit.com"))) {
		log_pedantic("Unable to display user settings. {%s}", err.text);
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}

	portal_endpoint_response(con, "{s:s, s:o, s:I}", "jsonrpc", "2.0", "result", object, "id", con->http.portal.id);

	return;
}

/**
 * @brief	Return information for a portal "meta" json-rpc request.
 * @note	This function returns various pieces of information about the user such as their plan type, quota, etc.
 * @param	con		a pointer to the connection object across which the json-rpc response will be sent.
 * @return	This function returns no value.
 */
void portal_meta(connection_t *con) {

	json_t *object;
	json_error_t err;
	chr_t ipbuf[64];

	// Check the session state. Method takes no parameters.
	if (!portal_validate_request (con, PORTAL_ENDPOINT_ERROR_META, "meta", false, 0)) {
		return;
	}

	if (!(object = json_object_d())) {
		log_pedantic("Unable to allocate a JSON object for meta request.");
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}

	snprintf(ipbuf, sizeof(ipbuf), "%s", st_char_get(con_addr_presentation(con, MANAGEDBUF(64))));
	//ip_locator_country_name();

	if (!(object = json_pack_ex_d(&err, JSON_ENSURE_ASCII, "{s:i, s:s, s:s, s:s, s:i, s:s, s:s, s:s, s:b, s:b, s:b}", "userID", con->http.session->user->usernum,
		"clientIP", ipbuf, "location", "Dallas, TX", "timezone", "Central", "reputation", 100, "plan", "Basic", "version", MAGMA_PORTAL_VERSION, "quota", "45%",
		"javascript", 1, "stylesheets", 1, "connection:", 1))) {
		log_pedantic("Unable to display user settings. {%s}", err.text);
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}

	portal_endpoint_response(con, "{s:s, s:o, s:I}", "jsonrpc", "2.0", "result", object, "id", con->http.portal.id);
	return;
}

/**
 * @brief	Change a user's password in response to a portal "settings.changepass" json-rpc request.
 * @param	con		a pointer to the connection object across which the json-rpc response will be sent.
 * @return	This function returns no value.
 */
void portal_settings_changepass(connection_t *con) {

	json_t *object;
	json_error_t err;
	chr_t *oldpass, *newpass;

	// Check the session state. Method takes 2 parameters.
	if (!portal_validate_request (con, PORTAL_ENDPOINT_ERROR_SETTINGS_CHANGEPASS, "settings.changepass", true, 2)) {
		return;
	}
	// Validate the request format and extract the submitted values.
	else if ((json_unpack_ex_d(con->http.portal.params, &err, JSON_STRICT, "{s:s, s:s}", "oldpass", &oldpass, "newpass", &newpass))) {
		log_pedantic("Received invalid portal changepass request parameters { user = %.*s, errmsg = %s }",
			(int)st_length_get(con->http.session->user->username), st_char_get(con->http.session->user->username), err.text);
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	}

	if (!(object = json_object_d())) {
		log_pedantic("Unable to allocate a JSON object for meta request.");
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}

	if (!(object = json_pack_ex_d(&err, JSON_ENSURE_ASCII, "{s:i, s:s, s:s, s:s, s:i, s:s, s:s, s:s, s:b, s:b, s:b}", "userID", con->http.session->user->usernum,
		"clientIP", "127.0.0.1", "location", "Dallas, TX", "timezone", "Central", "reputation", 100, "plan", "Basic", "version", MAGMA_PORTAL_VERSION, "quota", "45%",
		"javascript", 1, "stylesheets", 1, "connection:", 1))) {
		log_pedantic("Unable to display user settings. {%s}", err.text);
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}

	portal_endpoint_response(con, "{s:s, s:o, s:I}", "jsonrpc", "2.0", "result", object, "id", con->http.portal.id);

	return;
}

/**
 * @brief	The entry point for camel requests sent to the portal.
 * @param	con		the connection object corresponding to the web client making the request.
 * @return	This function returns no value.
 */
void portal_endpoint(connection_t *con) {

	json_error_t err;
	json_t *method, *id;
	void (*function)(connection_t *con);
	command_t *command, name = {
		.function = NULL
	};

	// Is HTTPS required to access to the portal from anywhere but the localhost.
	if (magma.web.portal.safeguard && con_secure(con) != 1 && !con_localhost(con)) {
		http_print_301(con, "/portal", 1);
		return;
	}

	// Make sure the merged context type is empty before setting it to the portal type code.
	if (con->http.merged != HTTP_MERGED) {
		log_pedantic("Invalid merged web application context type. Was the Portal endpoint processor called twice for the same request? { merged = %i }", con->http.merged);
		con->http.mode = HTTP_ERROR_500;
		return;
	}
	else {
		con->http.merged = HTTP_PORTAL;
	}

	// Try extracting the session from either a cookie, or the location.
	http_parse_context(con, PLACER("portal", 6), PLACER("/portal/camel/", 13));

	// If the connection doesn't have a session context already, create a new one.
	if (!con->http.session && !(con->http.session = sess_create(con, PLACER("/portal/camel", 13), PLACER("portal", 6)))) {
		log_pedantic("Session creation attempt failed.");
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		return;
	}

	// The JSON RPC specification says we should accept an empty POST and return any queued status messages in the response.
	// Until we need that functionality we can simply return an empty response.
	if (!con->http.body) {
		http_response_header(con, 200, PLACER("application/json; charset=utf-8", 31), 2);
		return;
	}

	// Parse the JSON request.
	if (!(con->http.portal.request = json_loads_d(st_char_get(con->http.body), 0, &err))) {
		log_pedantic("Invalid JSON request submitted to the Portal Camelface. { line = %d / text = %s }", err.line, err.text);
		portal_endpoint_error(con, 500, JSON_RPC_2_ERROR_PARSE_MALFORMED, "Parse error.");
		return;
	}

	// Ensure a method name was provided.
	else if (!(method = json_object_get_d(con->http.portal.request, "method")) || !json_is_string(method)) {
		log_pedantic("Invalid JSON request submitted to the Portal Camelface. { method != string / type = %s }", json_type_string_d(method));
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_REQUEST, "Invalid request.");
		return;
	}

	// And a request ID.
	else if (!(id = json_object_get_d(con->http.portal.request, "id")) || (!json_is_string(id) && !json_is_integer(id)) ||
		(json_is_string(id) && uint64_conv_ns((chr_t *)json_string_value_d(id), &(con->http.portal.id)) != 1) ||
		(json_is_integer(id) && !(con->http.portal.id = json_integer_value_d(id)))) {
		log_pedantic("Invalid JSON request submitted to the Portal Camelface. { id != string / type = %s }", json_type_string_d(id));
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_REQUEST, "Invalid request.");
		return;
	}

	// And finally store the parameters object (if present).
	con->http.portal.params = json_object_get_d(con->http.portal.request, "params");

	// Generate the method request request structure.
	name.string = (chr_t *)json_string_value_d(method);
	name.length = ns_length_get((chr_t *)json_string_value_d(method));

	// If a method handler is found, execute it.
	if ((command = bsearch(&name, portal_methods, sizeof(portal_methods) / sizeof(portal_methods[0]), sizeof(command_t), portal_endpoint_compare)) &&
		(function = command->function)) {
		function(con);
	}
	else {
		log_pedantic("Unrecognized method name submitted to the Portal Camelface. { method = %s }", name.string);
		portal_endpoint_error(con, 404, JSON_RPC_2_ERROR_SERVER_METHOD_UNAVAIL, "Method not found.");
	}

	return;
}

/**
 * @brief	Get a user's attachment to a message composition uploaded via multipart form data.
 * @note
 * @param	path	a pointer to a managed string containing the full /attach path specified in the client's http POST request.
 */
attachment_t * portal_get_upload_attachment(connection_t *con) {

	composition_t *comp;
	attachment_t *attachment;
	placer_t p_id;
	uint64_t ntokens, cnum, anum;
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = 0 };

	// There should be at least (and really, only) 5 frontslashes in this path: /portal/camel/attachment/composition-id/attachment-id
	ntokens = tok_get_count_st(con->http.location, '/');

	if (ntokens < 5) {
		log_pedantic("Portal upload path didn't specify attachment ID.");
		return NULL;
	}

	// Parse out the intended composition ID. It will come right after the /attach/ part of the path.
	if (tok_get_st(con->http.location, '/', 4, &p_id)) {
	    log_pedantic("Error parsing composition id from portal upload path.");
	    return NULL;
	}

	if (!uint64_conv_st(&p_id, &cnum)) {
		log_pedantic("Invalid composition specified in portal upload request path.");
		return NULL;
	}

	// Next is the attachment ID.
	if (tok_get_st(con->http.location, '/', 5, &p_id) < 0) {
		log_pedantic("Error parsing attachment id from portal upload path.");
		return NULL;
	}

	if (!uint64_conv_st(&p_id, &anum)) {
		log_pedantic("Invalid attachment specified in portal upload request path.");
		return NULL;
	}

	// Now make sure the specified composition + attachment actually exist.
	mutex_lock(&(con->http.session->lock));
	key.val.u64 = cnum;

	if (!(comp = inx_find(con->http.session->compositions, key))) {
		mutex_unlock(&(con->http.session->lock));
		log_pedantic("Portal upload request specified invalid composition id.");
		return NULL;
	}

	// Then make sure the specified attachment exists.
	key.val.u64 = anum;

	if (!(attachment = inx_find(comp->attachments, key))) {
		mutex_unlock(&(con->http.session->lock));
		log_pedantic("Portal upload request specified invalid attachment id.");
		return NULL;
	}

	mutex_unlock(&(con->http.session->lock));

	// If filedata isn't NULL, it means that the attachment has already been uploaded and received.
	if (attachment->filedata) {
		log_pedantic("Portal upload attachment was already uploaded.");
		return NULL;
	}

	return attachment;
}

/**
 * @brief	Process uploaded attachments for messages composed in conjunction with the portal interface.
 */
void portal_upload(connection_t *con) {

	inx_cursor_t *cursor;
	attachment_t *attachment;
	http_data_t *data, *cdisposition, *ctype, *ctransfer;
	placer_t boundary, token, ffilename;
	uint64_t ntokens;
	char *ptr;

	if (con->http.method != HTTP_METHOD_POST) {
		log_pedantic("Portal upload request did not use POST method.");
		con->http.mode = HTTP_ERROR_403;
		return;
	}

	// A bit of housekeeping that is normally done in portal_endpoint() ...
	if (magma.web.portal.safeguard && con_secure(con) != 1 && !con_localhost(con)) {
		http_print_301(con, con->http.location, 1);
		return;
	}

	// Make sure the merged context type is empty before setting it to the portal type code.
	if (con->http.merged != HTTP_MERGED) {
		log_pedantic("Invalid merged web application context type. Was the Portal endpoint processor called twice for the same request? { merged = %i }", con->http.merged);
		con->http.mode = HTTP_ERROR_500;
		return;
	}
	else {
		con->http.merged = HTTP_PORTAL;
	}

	// Try extracting the session from either a cookie, or the location.
	http_parse_context(con, PLACER("portal", 6), PLACER("/portal/camel/", 13));

	if (!con->http.session) {
		log_pedantic("Portal upload request was made without a valid session.");
		con->http.mode = HTTP_ERROR_401;
		return;
	}

	if (!(attachment = portal_get_upload_attachment(con))) {
		log_pedantic("Portal upload request specified invalid attachment info.");
		con->http.mode = HTTP_ERROR_403;
		return;
	}

    if (con->http.headers && (cursor = inx_cursor_alloc(con->http.headers))) {
        	while ((data = inx_cursor_value_next(cursor)))

        	inx_cursor_free(cursor);
      }

    if (!multipart_get_boundary(con, &boundary)) {
    	log_pedantic("Portal upload request supplied unreadable Content-Type parameters.");
    	con->http.mode = HTTP_ERROR_405;
    	return;
    }

    ntokens = str_tok_get_count_bl(st_char_get(con->http.body), st_length_get(con->http.body), st_char_get(&boundary), st_length_get(&boundary));

    // We are going to need at least two boundary chunks: at the beginning and at the end.
    // The first boundary string should be at the very beginning, leaving the first token blank; same goes for the 3rd token after the boundary string at the end.
    // So, really all we care about is the one in the middle (index 1).
    if (ntokens < 2) {
    	log_pedantic("Error occurred parsing portal upload request boundary fields.");
    	con->http.mode = HTTP_ERROR_405;
    	return;
    }

    if (str_tok_get_bl(st_char_get(con->http.body), st_length_get(con->http.body), st_char_get(&boundary), st_length_get(&boundary), 1, &token)) {
    	log_pedantic("Error occurred retrieving contents of multipart data in portal upload request.");
    	con->http.mode = HTTP_ERROR_405;
    	return;
    }

    // We may need to trim hyphens from the end.
    if (!pl_skip_characters (&token, "-", 1)) {
    	log_pedantic("Error occurred retrieving contents of multipart data in portal upload request.");
    	con->http.mode = HTTP_ERROR_405;
    	return;
    }

    if (!pl_skip_characters (&token, "\r\n", 2)) {
    	log_pedantic("Error occurred retrieving contents of multipart data in portal upload request.");
    	con->http.mode = HTTP_ERROR_405;
    	return;
    }

    // Inside the form data we expect at least a Content-Disposition header followed by a Content-Type header.
    if (!(cdisposition = http_data_header_parse_line(pl_char_get(token), pl_length_get(token))) &&
    		(st_cmp_ci_eq(cdisposition->name, NULLER("Content-Disposition")))) {
    	log_pedantic("Portal upload request multipart data contained no Content-Disposition field.");
    	con->http.mode = HTTP_ERROR_405;
    	return;
    }

    // Skip past end of Content-Disposition line, by skipping TO the newline characters and then skipping OVER them.
    if (!pl_skip_to_characters(&token, "\r\n", 2) || !pl_skip_characters(&token, "\r\n", 2)) {
    	log_pedantic("Portal upload request multipart data contained no Content-Disposition field.");
    	http_data_free(cdisposition);
    	con->http.mode = HTTP_ERROR_405;
    	return;
    }

    //TODO: Content-Type is actually optional? (but defaults to text/plain)...

    // Now we are at the Content-Type.
    if (!(ctype = http_data_header_parse_line(pl_char_get(token), pl_length_get(token))) ||
    		(st_cmp_ci_eq(ctype->name, NULLER("Content-Type")))) {
    	log_pedantic("Portal upload request multipart data contained no Content-Type field.");
    	http_data_free(cdisposition);
    	con->http.mode = HTTP_ERROR_405;
    	return;
    }

    // And skip past the Content-Type line as well..
     if (!pl_skip_to_characters(&token, "\r\n", 2) || !pl_skip_characters(&token, "\r\n", 2)) {
     	log_pedantic("Portal upload request multipart data contained no Content-Type field.");
     	http_data_free(cdisposition);
     	http_data_free(ctype);
     	con->http.mode = HTTP_ERROR_405;
     	return;
     }

     // There might be a Content-Transfer-Encoding line as well.
     if ((ctransfer = http_data_header_parse_line(pl_char_get(token), pl_length_get(token))) &&
    		 (st_cmp_ci_eq(ctransfer->name, NULLER("Content-Transfer-Encoding")))) {

    	 // See if we can skip past this line.
         if (!pl_skip_to_characters(&token, "\r\n", 2) || !pl_skip_characters(&token, "\r\n", 2)) {
         	log_pedantic("Portal upload request multipart data contained invalid Content-Transfer-Encoding field.");
         	http_data_free(cdisposition);
         	http_data_free(ctype);
         	http_data_free(ctransfer);
         	con->http.mode = HTTP_ERROR_405;
         	return;
         }

     }

     // There still may be a trailing \r\n and hyphens.
     if (!pl_shrink_before_characters (&token, "-", 1)) {
    	 log_pedantic("Portal upload request multipart data contained bad boundary ending.");
    	 con->http.mode = HTTP_ERROR_405;
     }

     if ((pl_length_get(token) >= 2) && (ptr = pl_char_get(token)) && (ptr[pl_length_get(token) - 1] == '\n') && (ptr[pl_length_get(token) - 2] == '\r')) {
    		 token.length -= 2;
     }

     if (pl_empty(ffilename = get_header_opt(cdisposition->value, NULLER("filename")))) {
    	 log_pedantic("Portal upload request missed filename in Content-Disposition value.");
    	 http_data_free(cdisposition);
    	 http_data_free(ctype);
    	 http_data_free(ctransfer);
    	 con->http.mode = HTTP_ERROR_405;
     }

     http_data_free(cdisposition);
     http_data_free(ctype);
     http_data_free(ctransfer);


     if (!(attachment->filedata = st_import(pl_char_get(token), pl_length_get(token)))) {
    	 log_error("Could not allocate space for uploaded user attachment.");
    	 con->http.mode = HTTP_ERROR_500;
     }

	con->http.mode = HTTP_ERROR_404;

	return;
}




/**
 * @brief	A portal debug function that will be disabled and/or deleted completely in production.
 * @note	The debug output is displayed LOCALLY in magmad's console.
 */
void portal_debug(connection_t *con) {

	return;
}
