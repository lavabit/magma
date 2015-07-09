#include "magma.h"

typedef struct {
	char *string;
	size_t length;
	void (*callback)(connection_t *con);
} api_lookup_t;

static
api_lookup_t
api_methods[] = {
	{
		.string = "auth" ,
		.length = 4,
		.callback = &api_endpoint_auth
	},
};

static
int_t
api_method_compare(
	void const *compare,
	void const *command)
{
	api_lookup_t *cmd = (api_lookup_t *)command;
	api_lookup_t *cmp = (api_lookup_t *)compare;

	return st_cmp_ci_eq(
		PLACER(cmp->string, cmp->length),
		PLACER(cmd->string, cmd->length));
}

static
bool_t
is_localhost(connection_t *con) {
	return con_addr_word(con, 0) == 0x0100007f;
}

static
bool_t
is_ssl(connection_t *con) {
	return con_secure(con) == 1;
}

static
void
internal_error(connection_t *con) {
	api_error(
		con,
		HTTP_ERROR_500,
		JSON_RPC_2_ERROR_SERVER_INTERNAL,
		"Internal server error.");
}

static
void
bad_request_error(connection_t *con) {
	api_error(
		con,
		HTTP_ERROR_400,
		JSON_RPC_2_ERROR_PARSE_MALFORMED,
		"Internal server error.");
}

static
void
not_found_error(connection_t *con) {
	api_error(
		con,
		HTTP_ERROR_400,
		JSON_RPC_2_ERROR_SERVER_METHOD_UNAVAIL,
		"Method not found.");
}

void json_api_dispatch(connection_t *con) {
	json_error_t jansson_err;
	json_t *method;
	json_t *id;
	api_lookup_t *found_method;
	api_lookup_t method_lookup = {
		.callback = NULL
	};

	if (
		//magma.web.portal.safeguard &&
		!is_ssl(con) &&
		!is_localhost(con))
	{
		log_pedantic("Insecure request denied");
		con->http.mode = HTTP_ERROR_400;
		goto out;
	}

	// Make sure this connection's session was marked as clean (HTTP_MERGED).
	// TODO - "merged" does not convey much useful information here
	if (con->http.merged != HTTP_MERGED) {
		log_error(
			"Invalid merged web application context type. "
			"Was the Portal endpoint processor called twice for the same request? "
			"{ merged = %i }",
			con->http.merged);
		internal_error(con);
		goto out;
	}

	// Inform http_session_reset that it needs to clean up the
	// portal-specific fields
	con->http.merged = HTTP_PORTAL;

	// Try extracting the session from either a cookie, or the location.
	http_parse_context(
		con,
		PLACER("magma", 5),
		PLACER("api", 3));

	// If the connection doesn't have a session context already, create a new one.
	if (!con->http.session) {
		con->http.session = sess_create(
			con,
			PLACER("api", 3),
			PLACER("magma", 5));

		if (con->http.session == NULL) {
			log_error("Session creation attempt failed");
			internal_error(con);
			goto out;
		}
	}

	if (!con->http.body) {
		log_pedantic("Empty JSON RPC body");
		bad_request_error(con);
		goto out;
	}

	// Parse the JSON request.
	con->http.portal.request = json_loads_d(
		st_char_get(con->http.body),
		0, // jansson does not use this argument yet and expects 0
		&jansson_err);

	if (con->http.portal.request == NULL) {
		log_pedantic(
			"API request JSON parse error. "
			"{ line = %d / text = %s }",
			jansson_err.line,
			jansson_err.text);
		bad_request_error(con);
		goto out;
	}

	// Ensure a method name was provided.
	method = json_object_get_d(con->http.portal.request, "method");
	if (method == NULL || !json_is_string(method)) {
		log_pedantic("API request does not contain a valid method");
		bad_request_error(con);
		goto out;
	}

	// And a request ID.
	id = json_object_get_d(con->http.portal.request, "id");
	if (id == NULL || !json_is_string(id) && !json_is_integer(id)) {
		log_pedantic("API request does not contain a valid id");
		bad_request_error(con);
		goto out;
	}

	if (json_is_string(id)) {
		if (
			!uint64_conv_ns(
				(chr_t *)json_string_value_d(id),
				&(con->http.portal.id)))
		{
			log_pedantic("API request id string conversion error");
			bad_request_error(con);
			goto out;
		}
	}
	else {
		con->http.portal.id = json_integer_value_d(id);
	}

	// And finally store the parameters object (if present).
	con->http.portal.params = json_object_get_d(
		con->http.portal.request,
		"params");

	// Generate the method lookup structure.
	method_lookup.string = (chr_t *)json_string_value_d(method);
	method_lookup.length = ns_length_get((chr_t *)json_string_value_d(method));

	// If a method callback is found, execute it.
	found_method = bsearch(
		&method_lookup,
		api_methods,
		sizeof(api_methods) / sizeof(api_lookup_t),
		sizeof(api_lookup_t),
		&api_method_compare);

	if (found_method == NULL) {
		log_pedantic(
			"API method not found { method = %s }",
			method_lookup.string);
		not_found_error(con);
		goto out;
	}

	if (found_method->callback == NULL) {
		log_error(
			"API method callback missing { method = %s }",
			method_lookup.string);
		internal_error(con);
		goto out;
	}

	found_method->callback(con);

out:
	return;
}
