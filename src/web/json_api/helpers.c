#include "magma.h"

static
unsigned long
jansson_flags() {
	if (magma.web.portal.indent) {
		return JSON_PRESERVE_ORDER | JSON_INDENT(4);
	} else {
		return JSON_PRESERVE_ORDER | JSON_COMPACT;
	}
}

void
api_error(
	connection_t *con,
	int_t http_code,
	int_t error_code,
	chr_t *message)
{
	api_response(
		con,
		http_code,
		"{s:s, s:{s:i, s:s}, s:I}",
		"jsonrpc", "2.0",
		"error",
			"code", error_code,
			"message", message,
		"id", con->http.portal.id);

	return;
}

void
api_response(
	connection_t *con,
	int_t http_code,
	chr_t *format,
	...)
{
	va_list args;
	json_error_t jansson_err;
	json_t *object = NULL;
	chr_t *response = NULL;

	va_start(args, format);

	object = json_vpack_ex_d(&jansson_err, 0, format, args);
	if (!object) {
		log_pedantic(
			"Unable to generate a JSON response. "
			"{ object = NULL / error = %s }",
			jansson_err.text);
		con->http.mode = HTTP_ERROR_500;
		goto error;
	}

	response = json_dumps_d(object, jansson_flags());

	if (!response) {
		log_pedantic(
			"Unable to generate a JSON response. "
			"{ response = NULL }");
		con->http.mode = HTTP_ERROR_500;
		goto cleanup_object;
	}

	if (magma.web.portal.indent) {
		response = ns_append(response, "\r\n");
	}

	http_response_header(
		con,
		http_code,
		PLACER("application/json; charset=utf-8", 31),
		ns_length_get(response));

	con_write_st(con, NULLER(response));

	ns_free(response);
	json_decref_d (object);
	va_end(args);
	return;

cleanup_object:
	json_decref_d(object);
error:
	va_end(args);
	return;
}
