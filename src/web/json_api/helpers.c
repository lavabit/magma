
/**
 * @file /magma/web/json_api/helpers.c
 *
 * @brief The the JSON API interface functions.
 */

#include "magma.h"

unsigned long jansson_flags(void) {
	if (magma.web.portal.indent) {
		return JSON_PRESERVE_ORDER | JSON_INDENT(4);
	}
	return JSON_PRESERVE_ORDER | JSON_COMPACT;
}

/**
 * @brief	Return a json-rpc error response to the remote client.
 * @param con	a pointer to the connection object across which the response will be sent.
 * @param http_code	the http response code to be sent to the remote client in the response header.
 * @param error_code	the numerical error code to be encoded in the json-rpc error message.
 * @param message	a descriptive error string to be encoded in the json-rpc error message.
 * @return	This function returns no value.
 */
void api_error(connection_t *con, int_t http_code, int_t error_code, chr_t *message) {
	api_response(con, http_code, "{s:s, s:{s:i, s:s}, s:I}", "jsonrpc", "2.0", "error", "code", error_code, "message", message, "id", con->http.portal.id);
	return;
}

/**
 * @brief	Generate a json-rpc 2.0 response to a portal request.
 * @see	json_vpack_ex()
 * @note	This function indents the json response if specified in the configuration, and also automatically decreases the reference count
 * 		of any json object that was packed for the reply.
 * @param con	a pointer to the connection object across which the portal response will be sent.
 * @param format	a pointer to a format string specifying the construction of the json-rpc response.
 * @param va_list	a variable arguments style list of parameters to be passed to the json packing function.
 * @return	This function returns no value.
 */
void api_response(connection_t *con, int_t http_code, chr_t *format, ...) {
	va_list args;
	json_error_t jansson_err;
	json_t *object	= NULL;
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
	http_response_header(con, http_code, PLACER("application/json; charset=utf-8", 31), ns_length_get(response));

	con_write_st(con, NULLER(response));

	ns_free(response);
	json_decref_d(object);
	va_end(args);
	return;

cleanup_object: json_decref_d(object);
error: va_end(args);
	return;
}

