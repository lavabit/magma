#include "magma.h"

static
bool_t
authenticate_stub_REPLACE_ME(
	meta_user_t *user,
	chr_t const *username,
	chr_t const *password,
	META_PROT protocol)
{
	return true;
}

static
bool_t
is_locked(meta_user_t *user) {
	return user->lock_status < 5;
}

static
chr_t *
lock_error_message(meta_user_t *user)
{
	chr_t *result;

	switch (user->lock_status) {
		case 0:
			result = "This account has been locked.";
			break;
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
			break;
	}
}

void
api_endpoint_auth(connection_t *con) {
	size_t count;
	json_error_t jansson_err;
	chr_t *username;
	chr_t *password;
	meta_user_t *user;

	if (
		json_unpack_ex_d(
			con->http.portal.params,
			&jansson_err,
			JSON_STRICT,
			"{s:s, s:s}",
			"username", &username,
			"password", &password)
		!= 0)
	{
		log_pedantic(
			"Received invalid portal auth request parameters "
			"{ user = %s, errmsg = %s }",
			st_char_get(con->http.session->user->username),
			jansson_err.text);

		api_error(
			con,
			HTTP_ERROR_400,
			JSON_RPC_2_ERROR_SERVER_METHOD_PARAMS,
			"Invalid method parameters.");

		goto out;
	}

	if (
		!authenticate_stub_REPLACE_ME(
			user,
			username,
			password,
			META_PROT_JSON))
	{
		api_error(
			con,
			HTTP_ERROR_400,
			PORTAL_ENDPOINT_ERROR_AUTH,
			"Unable to authenticate with given username and password.");
		goto cleanup_username_password;
	}

	if (is_locked(user)) {
		api_error(
			con,
			HTTP_ERROR_400,
			PORTAL_ENDPOINT_ERROR_AUTH,
			lock_error_message(user));
		goto cleanup_user;
	}

	con->http.session->state = SESSION_STATE_AUTHENTICATED;
	con->http.response.cookie = HTTP_COOKIE_SET;
	portal_endpoint_response(
		con,
		"{s:s, s:{s:s, s:s}, s:I}",
		"jsonrpc", "2.0",
		"result",
			"auth", "success",
			"session", st_char_get(con->http.session->warden.token),
		"id", con->http.portal.id);

	con->http.session->user = user;

	api_response(
		con,
		HTTP_OK,
		"{s:s, s:I}",
		"jsonrpc", "2.0",
		"id", con->http.portal.id);

cleanup_user:
	meta_remove(user->username, META_PROT_JSON);
cleanup_username_password:
	ns_free(username);
	ns_free(password);
out:
	return;
}
