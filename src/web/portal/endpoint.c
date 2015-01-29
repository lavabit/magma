
/**
 * @file /magma/web/portal/endpoint.c
 *
 * @brief	The control logic for the Portal JSON endpoint.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
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
 *
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
	meta_user_t *user;
	credential_t *cred;
	chr_t *username, *password;

	// Check the session state.
	if (!con->http.session || con->http.session->state != SESSION_STATE_NEUTRAL) {
		portal_endpoint_error(con, 403, PORTAL_ENDPOINT_ERROR_MODE | PORTAL_ENDPOINT_ERROR_AUTH, "The auth method is unavailable after a successful login.");
		return;
	}

	// Validate the request format and extract the submitted values.
	else if ((json_unpack_ex_d(con->http.portal.params, &err, JSON_STRICT, "{s:s, s:s}", "username", &username, "password", &password))) {
		log_pedantic("Received invalid portal auth request parameters { user = %s, errmsg = %s }", st_char_get(con->http.session->user->username), err.text);
		portal_endpoint_error(con, 400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	}

	// Convert the strings into a full fledged credential context.
	else if (!(cred = credential_alloc_auth(NULLER(username), NULLER(password)))) {
		portal_endpoint_error(con, 200, PORTAL_ENDPOINT_ERROR_AUTH, "Internal server error. Please try again in a few minutes.");
		return;
	}

	// Try getting the session out of the global cache.
	state = meta_get(cred->auth.username, cred->auth.domain, cred->auth.password, cred->auth.key, META_PROT_WEB, META_GET_MESSAGES | META_GET_FOLDERS | META_GET_CONTACTS, &(user));
	con->http.session->warden.cred = cred;

	// Not found, or invalid password.
	// QUESTION: Is state == 0 really an error condition?
	if (state == 0) {
		portal_endpoint_response(con, "{s:s, s:{s:s, s:s}, s:I}", "jsonrpc", "2.0", "result", "auth", "failed", "message",
			"The username and password provided are incorrect, please try again.", "id", con->http.portal.id);
		return;
	}

	// If we get past here we assume the user variable is a valid pointer.
	else if (state < 0 || user == NULL) {
		portal_endpoint_error(con, 200, PORTAL_ENDPOINT_ERROR_AUTH, "This server is unable to access your mailbox. Please try again later.");
		return;
	}

	// Transport Layer Security Required
	if ((user->flags & META_USER_SSL) == META_USER_SSL && con_secure(con) != 1) {
		portal_endpoint_response(con, "{s:s, s:{s:s, s:s}, s:I}", "jsonrpc", "2.0","result", "auth", "failed", "message",
			"The provided user account requires all connections be secured using encryption.", "id", con->http.portal.id);
	}

	// Locks
	else if (user->lock_status == 1) {
		portal_endpoint_response(con, "{s:s, s:{s:s, s:s}, s:I}", "jsonrpc", "2.0", "result", "auth", "locked", "admin", "message",
			"This account has been administratively locked.", "id", con->http.portal.id);
	}
	else if (user->lock_status == 2) {
		portal_endpoint_response(con, "{s:s, s:{s:s, s:s}, s:I}", "jsonrpc", "2.0", "result", "auth", "locked", "inactivity", "message",
			"This account has been locked for inactivity.", "id", con->http.portal.id);
	}
	else if (user->lock_status == 3) {
		portal_endpoint_response(con, "{s:s, s:{s:s, s:s}, s:I}", "jsonrpc", "2.0", "result", "auth", "locked", "abuse", "message",
			"This account has been locked on suspicion of abuse.", "id", con->http.portal.id);
	}
	else if (user->lock_status == 4) {
		portal_endpoint_response(con, "{s:s, s:{s:s, s:s}, s:I}", "jsonrpc", "2.0", "result", "auth", "locked", "user", "message",
			"This account has been locked at the request of the user.", "id", con->http.portal.id);
	}
	else if (user->lock_status != 0) {
		portal_endpoint_response(con, "{s:s, s:{s:s, s:s}, s:I}", "jsonrpc", "2.0", "result", "auth", "locked", "generic", "message",
			"This account has been locked.", "id", con->http.portal.id);
	}
	else {
		con->http.session->state = SESSION_STATE_AUTHENTICATED;
		con->http.response.cookie = HTTP_COOKIE_SET;
		portal_endpoint_response(con, "{s:s, s:{s:s, s:s}, s:I}", "jsonrpc", "2.0", "result", "auth", "success", "session",
			st_char_get(con->http.session->warden.token), "id", con->http.portal.id);
		meta_user_ref_add(user, META_PROT_WEB);
		con->http.session->user = user;
	}

	if (user && user->username) meta_remove(user->username, META_PROT_WEB);
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
	meta_user_ref_dec(con->http.session->user, META_PROT_WEB);
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

	int_t state, context;
	json_error_t err;
	chr_t *rename = NULL, *method;
	uint64_t foldernum;
	magma_folder_t *active_c;
	meta_folder_t *active_m;
	stringer_t *original = NULL, *srename;

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

	inx_t *list;
	chr_t *method;
	uint32_t bits;
	json_error_t err;
	int_t action, ret;
	bool_t commit = true;
	inx_cursor_t *cursor;
	meta_message_t *active;
	uint64_t folder, count;
	json_t *flags = NULL, *messages, *collection = NULL, *entry;
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = 0 };


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
	inx_t *list;
	chr_t *method;
	json_error_t err;
	bool_t commit = true;
	meta_message_t *active;
	inx_cursor_t *cursor;
	uint64_t folder, mess_count, tag_count;
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
	if (!portal_outbound_checks(con->http.session->warden.cred, con->http.session->user->usernum, NULLER(from), nrecipients, NULLER(body_plain), NULLER(body_html), &errmsg)) {
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

	// Check the session state.
	if (!portal_validate_request (con, PORTAL_ENDPOINT_ERROR_AD, "ad", false, 0)) {
		return;
	}

	portal_endpoint_error(con, 404, JSON_RPC_2_ERROR_SERVER_METHOD_UNAVAIL, "Method is not implemented.");
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
	// TODO: This check needs to be cleaned up.
	if (magma.web.portal.safeguard && con_secure(con) != 1 && con_addr_word(con, 0) != 0x0100007f) {
		// QUESTION: Redirect here, or to /portal/camel?
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
 *
 *
 *
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
 *
 *
 */
void portal_upload(connection_t *con) {

	inx_cursor_t *cursor;
	attachment_t *attachment;
	http_data_t *data, *cdisposition, *ctype, *ctransfer;
	placer_t boundary, token, fname, ffilename;
	uint64_t ntokens;
	char *ptr;


	if (con->http.method != HTTP_METHOD_POST) {
		log_pedantic("Portal upload request did not use POST method.");
		con->http.mode = HTTP_ERROR_403;
		return;
	}

	// A bit of housekeeping that is normally done in portal_endpoint() ...
	// TODO: This check needs to be cleaned up.
	if (magma.web.portal.safeguard && con_secure(con) != 1 && con_addr_word(con, 0) != 0x0100007f) {
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

     fname = pl_null();

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
