#include "magma.h"

void api_endpoint_auth(connection_t *con) {
	api_response(
		con,
		HTTP_OK,
		"{s:s, s:{s:s}, s:I}",
		"jsonrpc", "2.0",
		"result",
			"auth", "success",
		"id", con->http.portal.id);
	return;
}
