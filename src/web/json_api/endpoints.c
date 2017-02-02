
/**
 * @file /magma/web/json_api/endpoints.c
 *
 * @brief The the JSON API interface functions.
 */

#include "magma.h"

static bool_t is_locked(auth_t *auth) {
	return auth->status.locked != 0;
}

static chr_t * lock_error_message(auth_t *auth) {
	chr_t *result;

	switch (auth->status.locked) {
		case 1:
			result = "This account has been administratively locked.";
			break;
		case 2:
			result = "This account has been locked for inactivity.";
			break;
		case 3:
			result = "This account has been locked on suspicion of abuse.";
			break;
		case 4:
			result = "This account has been locked at the request of the user.";
			break;
		default:
			result = "";
			break;
	}

	return result;
}

void api_endpoint_auth(connection_t *con) {

	int_t state;
	auth_t *auth = NULL;
	json_error_t jansson_err;
	meta_user_t *user = NULL;
	chr_t *username = NULL, *password = NULL;
	stringer_t *subnet = NULL, *key = NULL;

	if (json_unpack_ex_d(con->http.portal.params, &jansson_err, JSON_STRICT, "{s:s, s:s}", "username", &username, "password", &password) != 0) {
		log_pedantic("Received invalid portal auth request parameters { user = %.*s, errmsg = %s }",
			st_length_int(con->http.session->user->username), st_char_get(con->http.session->user->username),
			jansson_err.text);

		api_error(con, HTTP_ERROR_400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		return;
	}

	// Store the subnet for tracking login failures. Make the buffer big enough to hold an IPv6 subnet string.
	subnet = con_addr_subnet(con, MANAGEDBUF(256));

	// Generate the invalid login tracker.
	key = st_quick(MANAGEDBUF(384), "magma.logins.invalid.%lu, %*.s", time_datestamp(), st_length_int(subnet), st_char_get(subnet));

	// For now we hard code the maximum number of failed logins.
	if (st_populated(key) && cache_get_u64(key) > 16) {
		api_error(con, HTTP_ERROR_400, PORTAL_ENDPOINT_ERROR_AUTH, "The maximum number of failed login attempts has been reached. Please try again later.");
		return;
	}

	if ((state = auth_login(NULLER(username), NULLER(password), &auth))) {
		if (state < 0) {
			api_error(con, HTTP_ERROR_500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		}
		else {
			api_error(con, HTTP_ERROR_400, PORTAL_ENDPOINT_ERROR_AUTH, "Unable to authenticate with given username and password.");
		}

		// If we have a valid key, we increment the failed login counter.
		if (st_populated(key)) {
			cache_increment(key, 1, 1, 86400);
		}

		return;
	}

	if (is_locked(auth)) {
		api_error(con, HTTP_ERROR_400, PORTAL_ENDPOINT_ERROR_AUTH, lock_error_message(auth));
		auth_free(auth);
		return;
	}

	if ((state = meta_get(auth->usernum, auth->username, auth->keys.master, auth->tokens.verification, META_PROTOCOL_JSON, META_GET_NONE, &user))) {
		if (state < 0) {
			api_error(con, HTTP_ERROR_500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		}
		else {
			api_error(con, HTTP_ERROR_400, PORTAL_ENDPOINT_ERROR_AUTH, "Unable to authenticate with given username and password.");
		}

		auth_free(auth);
		return;
	}

	con->http.session->state = SESSION_STATE_AUTHENTICATED;
	con->http.response.cookie = HTTP_COOKIE_SET;
	con->http.session->user = user;


	// There were two successful responses in the original function. I think this one is an extra.
	//portal_endpoint_response(con, "{s:s, s:{s:s, s:s}, s:I}", "jsonrpc", "2.0", "result", "auth", "success", "session",
	//	st_char_get(con->http.session->warden.token), "id",	con->http.portal.id);

	api_response(con, HTTP_OK, "{s:s, s:I}", "jsonrpc", "2.0", "id", con->http.portal.id);

	return;
}

void api_endpoint_register(connection_t *con) {
	json_error_t jansson_err;
	chr_t *username;
	chr_t *password;
	chr_t *password_verification;

	int64_t transaction;
	uint64_t usernum = 0;

	if (json_unpack_ex_d(con->http.portal.params, &jansson_err, JSON_STRICT, "{s:s, s:s, s:s}", "username", &username, "password", &password, "password_verification",
		&password_verification) != 0) {
		log_pedantic(
			"Received invalid portal auth request parameters "
			"{ user = %s, errmsg = %s }",
			st_char_get(con->http.session->user->username),
			jansson_err.text);

		api_error(con, HTTP_ERROR_400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");

		goto out;
	}

	// Start the transaction.
	transaction = tran_start();
	if (transaction == -1) {
		api_error(con, HTTP_ERROR_500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		goto out;
	}

	// Database insert.
	if (!register_data_insert_user(con, 1, lower_st(NULLER(username)), NULLER(password), transaction, &usernum)) {
		tran_rollback(transaction);
		api_error(con, HTTP_ERROR_500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
		goto out;
	}

	// Were finally done.
	tran_commit(transaction);

	// And finally, increment the abuse counter.
	register_abuse_increment_history(con);

	api_response(con, HTTP_OK, "{s:s, s:{s:s}, s:I}", "jsonrpc", "2.0", "result", "register", "success", "id", con->http.portal.id);

	out: return;
}

void api_endpoint_delete_user(connection_t *con) {
	json_error_t jansson_err;
	chr_t *username;
	int64_t num_deleted;
	MYSQL_BIND parameters[1];

	mm_wipe(parameters, sizeof(parameters));

	if (json_unpack_ex_d(con->http.portal.params, &jansson_err, JSON_STRICT, "{s:s}", "username", &username) != 0) {
		log_pedantic(
			"Received invalid portal auth request parameters "
			"{ user = %s, errmsg = %s }",
			st_char_get(con->http.session->user->username),
			jansson_err.text);

		api_error(con, HTTP_ERROR_400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");
		// A
		goto out;
	}

	// Key
	parameters[0].buffer_type = MYSQL_TYPE_STRING;
	parameters[0].buffer_length = ns_length_get(username);
	parameters[0].buffer = username;

	num_deleted = stmt_exec_affected(stmts.delete_user, parameters);
	if (0 == num_deleted) {
		api_error(con, HTTP_ERROR_422, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "No such user.");
		goto out;
	}
	if (-1 == num_deleted) {
		api_error(con, HTTP_ERROR_500, JSON_RPC_2_ERROR_SERVER_INTERNAL, "Internal server error.");
	}

	api_response(con, HTTP_OK, "{s:s, s:{s:s}, s:I}", "jsonrpc", "2.0", "result", "delete_user", "success", "id", con->http.portal.id);

	out: return;
}

void api_endpoint_change_password(connection_t *con) {
	json_error_t jansson_err;
	chr_t *password;
	chr_t *new_password;
	chr_t *new_password_verification;

	if (json_unpack_ex_d(con->http.portal.params, &jansson_err, JSON_STRICT, "{s:s, s:s, s:s}", "password", &password, "new_password", &new_password, "new_password_verification",
		&new_password_verification) != 0) {
		log_pedantic(
			"Received invalid portal auth request parameters "
			"{ user = %s, errmsg = %s }",
			st_char_get(con->http.session->user->username),
			jansson_err.text);

		api_error(con, HTTP_ERROR_400, JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS, "Invalid method parameters.");

		goto out;
	}

	/// TODO: - wire up here

	out: return;
}
