
/**
 * @file /magma/servers/http/response.c
 *
 * @brief	Functions for fulfilling responses to http client requests.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Get a descriptive string for a numerical http status code.
 * @param	status	the value of the http status code to be looked up.
 * @return	NULL on failure, or a pointer to a null-terminated string containing a description of the specified http status code on success.
 */
chr_t * http_response_status(int_t status) {

	chr_t *result = NULL;

	switch (status) {
		case (100):
			result = "Continue";
			break;
		case (101):
			result = "Switching Protocols";
			break;
		case (102):
			result = "Processing";
			break;
		case (200):
			result = "OK";
			break;
		case (201):
			result = "Created";
			break;
		case (202):
			result = "Accepted";
			break;
		case (203):
			result = "Non-Authoritative Information";
			break;
		case (204):
			result = "No Content";
			break;
		case (205):
			result = "Reset Content";
			break;
		case (206):
			result = "Partial Content";
			break;
		case (207):
			result = "Multi-Status";
			break;
		case (208):
			result = "Already Reported";
			break;
		case (226):
			result = "IM Used";
			break;
		case (300):
			result = "Multiple Choices";
			break;
		case (301):
			result = "Moved Permanently";
			break;
		case (302):
			result = "Found";
			break;
		case (303):
			result = "See Other";
			break;
		case (304):
			result = "Not Modified";
			break;
		case (305):
			result = "Use Proxy";
			break;
		case (306):
			result = "Reserved";
			break;
		case (307):
			result = "Temporary Redirect";
			break;
		case (400):
			result = "Bad Request";
			break;
		case (401):
			result = "Unauthorized";
			break;
		case (402):
			result = "Payment Required";
			break;
		case (403):
			result = "Forbidden";
			break;
		case (404):
			result = "Not Found";
			break;
		case (405):
			result = "Method Not Allowed";
			break;
		case (406):
			result = "Not Acceptable";
			break;
		case (407):
			result = "Proxy Authentication Required";
			break;
		case (408):
			result = "Request Timeout";
			break;
		case (409):
			result = "Conflict";
			break;
		case (410):
			result = "Gone";
			break;
		case (411):
			result = "Length Required";
			break;
		case (412):
			result = "Precondition Failed";
			break;
		case (413):
			result = "Request Entity Too Large";
			break;
		case (414):
			result = "Request-URI Too Long";
			break;
		case (415):
			result = "Unsupported Media Type";
			break;
		case (416):
			result = "Requested Range Not Satisfiable";
			break;
		case (417):
			result = "Expectation Failed";
			break;
		case (422):
			result = "Unprocessable Entity";
			break;
		case (423):
			result = "Locked";
			break;
		case (424):
			result = "Failed Dependency";
			break;
		case (425):
			result = "Reserved for WebDAV advanced";
			break;
		case (426):
			result = "Upgrade Required";
			break;
		case (500):
			result = "Internal Server Error";
			break;
		case (501):
			result = "Not Implemented";
			break;
		case (502):
			result = "Bad Gateway";
			break;
		case (503):
			result = "Service Unavailable";
			break;
		case (504):
			result = "Gateway Timeout";
			break;
		case (505):
			result = "HTTP Version Not Supported";
			break;
		case (506):
			result = "Variant Also Negotiates";
			break;
		case (507):
			result = "Insufficient Storage";
			break;
		case (508):
			result = "Loop Detected";
			break;
		case (510):
			result = "Not Extended";
			break;
		default:
			log_pedantic("Invalid HTTP status code! { status = %i }", status);
			break;
	}

	return result;
}

stringer_t * http_response_cookie(connection_t *con) {

	stringer_t *cookie = NULL;

	// If there is a session context, but there was no cookie supplied with the request, we create a Set-Cookie header to send back with
	// the response. For security, we only send '"secure" cookies out if the connection is secure.
	if (con->http.response.cookie == HTTP_COOKIE_SET && con->http.session && (!con->http.session->request.secure || con_secure(con) == 1)) {
		cookie = st_aprint_opts(MANAGED_T | JOINTED | HEAP, "Set-Cookie: %.*s=%.*s", st_length_int(con->http.session->request.application),
			st_char_get(con->http.session->request.application), st_length_int(con->http.session->warden.token),
			st_char_get(con->http.session->warden.token));
	}

	// Send along the Set-Cookie and include a Max-Age of zero so the browser knows its time to erase the cookie.
	else if (con->http.response.cookie == HTTP_COOKIE_DELETE && con->http.session && (!con->http.session->request.secure || con_secure(con) == 1)) {
		cookie = st_aprint_opts(MANAGED_T | JOINTED | HEAP, "Set-Cookie: %.*s=", st_length_int(con->http.session->request.application),
			st_char_get(con->http.session->request.application));
	}

	// If the host domain is available we can add it for extract security, but only if the client is connected to the default port for
	// HTTP or HTTPS. Otherwise its impossible to set the domain attribute without tiggering origin failures. A 0 implies the port
	// wasn't provided, while 80 and 443 are the protocol defaults and should be stripped off automagicaly for origin comparisons.
	if (cookie && con->http.session->request.host && (con->http.port == 0 || con->http.port == 80 || con->http.port == 443)) {
		cookie = st_append(cookie, st_quick(MANAGEDBUF(1024), "; domain=%.*s", st_length_int(con->http.session->request.host),
			st_char_get(con->http.session->request.host)));
	}

	// If a path restriction is available.
	if (cookie && con->http.session->request.path) {
		cookie = st_append(cookie, st_quick(MANAGEDBUF(1024), "; path=%.*s", st_length_int(con->http.session->request.path),
			st_char_get(con->http.session->request.path)));
	}

	// If the session indicates it we should supply Secure flag.
	if (cookie && con->http.session->request.secure) {
		cookie = st_append(cookie, PLACER("; secure", 8));
	}

	// If the HttpOnly flag is true it should still get submitted by browsers, but can't be read/modified/stolen via scripts.
	if (cookie && con->http.session->request.httponly) {
		cookie = st_append(cookie, PLACER("; httponly", 10));
	}

	if (cookie && con->http.response.cookie == HTTP_COOKIE_DELETE) {
		cookie = st_append(cookie, st_quick(MANAGEDBUF(256), "; expires=%s; max-age:0", st_char_get(time_print_gmt(MANAGEDBUF(128), "a, %d-%b-%Y %T %Z",
			time(NULL) - 31556926))));
	}

	if (cookie) {
		cookie = st_append(cookie, PLACER("\r\n", 2));
	}

	return cookie;

}

stringer_t * http_response_allow_cross(connection_t *con) {

	placer_t origin;
	http_data_t *field;
	stringer_t *allow = NULL;

	// Look for an Origin entity in the request headers. If its missing try and use the referrer. If both are missing, or using an
	// invalid format, fall back to the "*" wildcard. Although its important to note that wildcards will not work when using the
	// XmlHttpRequest Javascript method.
	if (((field = http_data_get(con, HTTP_DATA_HEADER, "Origin")) || (field = http_data_get(con, HTTP_DATA_HEADER, "Referer"))) && !http_parse_origin(field->value, &origin)) {
		allow = st_append(allow, st_quick(MANAGEDBUF(512), "Access-Control-Allow-Origin: %.*s\r\n", st_length_int(&origin), st_char_get(&origin)));
	}
	else {
		allow = st_append(allow, PLACER("Access-Control-Allow-Origin: *\r\n", 32));
	}

	// Browsers will reject any credentialed cross domain response that does not have the Access-Control-Allow-Credentials: true header.
	allow = st_append(allow, PLACER("Access-Control-Allow-Credentials: true\r\nAccess-Control-Max-Age: 86400\r\n", 71));

	// The client wants permission for a specific HTTP method.
	if ((field = http_data_get(con, HTTP_DATA_HEADER, "Access-Control-Request-Method"))	&& (!st_cmp_ci_eq(field->value, PLACER("OPTIONS", 7)) ||
		!st_cmp_ci_eq(field->value, PLACER("POST", 4))	|| !st_cmp_ci_eq(field->value, PLACER("GET", 3)))) {
		allow = st_append(allow, st_quick(MANAGEDBUF(512), "Access-Control-Allow-Methods: %.*s\r\n", st_length_int(field->value),	st_char_get(upper_st(field->value))));
	}

	// Allow access to cookies and any other headers the client may have requested.
	if ((field = http_data_get(con, HTTP_DATA_HEADER, "Access-Control-Request-Headers"))) {
		allow = st_append(allow, st_quick(MANAGEDBUF(1024), "Access-Control-Allow-Headers: %.*s, Cookie, Set-Cookie\r\n", st_length_int(field->value),
				st_char_get(field->value)));
	}

	return allow;
}

/**
 * @brief	Get an appropriate value for the Connection header of an http response.
 * @note	If magma.http.close was set in the configuration options, the connection will be closed immediately.
 * @param	con		a pointer to the connection object of the outgoing response.
 * @param	force	either HTTP_CONNECTION_CLOSE, HTTP_CONNECTION_NEUTRAL, or HTTP_CONNECTION_KEEPALIVE.
 * 					HTTP_CONNECTION_CLOSE will result in the connection being terminated immediately, HTTP_CONNECTION_KEEPALIVE will
 * 					result in a "Connection: keep-alive" response, and HTTP_CONNECTION_NEUTRAL will not result in any output.
 * @return	a pointer to a managed string containing the http Connection header field that should be used for the response.
 */
stringer_t * http_response_connection(connection_t *con, int_t force) {

	stringer_t *connection = NULL;

	// If the HTTP close config flag has been enabled we send the connection close tag and modify the mode to flag the
	// connection for closing once the response has been sent.
	if (magma.http.close || force == HTTP_CONNECTION_CLOSE) {
		connection = st_dupe_opts(MANAGED_T | HEAP | CONTIGUOUS, PLACER("Connection: close\r\n", 19));
		con->http.mode = HTTP_CLOSE;
	}

	// If the connection flag is HTTP_CONNECTION_NEUTRAL (0), don't output anything. If its HTTP_CONNECTION_KEEPALIVE (1) indicate keep-alive,
	// and if its HTTP_CONNECTION_CLOSE (-1) set the value to close.
	else if (con->http.response.connection == HTTP_CONNECTION_CLOSE || con->http.mode == HTTP_CLOSE) {
		connection = st_dupe_opts(MANAGED_T | HEAP | CONTIGUOUS, PLACER("Connection: close\r\n", 19));
	}
	else if (con->http.response.connection == HTTP_CONNECTION_KEEPALIVE) {
		connection = st_dupe_opts(MANAGED_T | HEAP | CONTIGUOUS, PLACER("Connection: keep-alive\r\n", 24));
	}

	return connection;
}

/**
 * @brief	Return a response to an http OPTIONS request.
 * @param	con		the client connection which made the OPTIONS request.
 * @return	This function returns no value.
 */
void http_response_options(connection_t *con) {

	stringer_t *allow = NULL, *connection = NULL;

	// We depend upon this buffer being empty so it gets skipped when the connection label wasn't provbided.
	mm_wipe(&connection, sizeof(connection));

	// Indicate the request has been processed so the requeue method will reset the context and enqueue HTTP processor.
	if (con->http.mode == HTTP_RESPOND) {
		con->http.mode = HTTP_COMPLETE;
	}

	if (magma.http.allow_cross_domain) {
		allow = http_response_allow_cross(con);
	}

	connection = http_response_connection(con, HTTP_CONNECTION_NEUTRAL);

	/// LOW: I couldn't find a definitive answer about whether the OPTION response should always return a Content-Type of text/plain
	/// or use the Content-Type associated with the location provided in the request.
	con_print(con, "HTTP/1.1 200 OK\r\n" \
		"Date: %s\r\n" \
		"Allow: GET, POST, OPTIONS\r\n" \
		"Server: Magma/%s Build/%s (Vendor/Lavabit) (Support-Url/lavabit.com)\r\n" \
		"%.*s" \
		"%.*s" \
		"Content-Type: text/plain\r\n" \
		"Content-Length: 0\r\n" \
		"\r\n",
		st_char_get(time_print_gmt(MANAGEDBUF(128), "%a, %d %b %Y %T %Z", time(NULL))),
		build_version(), build_stamp(),
		(allow ? st_length_int(allow) : 0),	(allow ? st_char_get(allow) : NULL),
		(connection ? st_length_int(connection) : 0),	(connection ? st_char_get(connection) : NULL));

	st_cleanup(allow);
	st_cleanup(connection);
	return;
}

/**
 * @brief	Send a full set of http response headers to the remote client.
 * @note	If the mode is HTTP_RESPOND it will be changed to HTTP_COMPLETE, to tell the http requeue function to reset the context and enqueue request processor.
 * @param	con		a pointer to the connection object across which the response will be sent.
 * @param	status	an integer containing the http status code for the response.
 * @param	type	a managed string containing the value of the Content-Type header.
 * @param	len		the value of the Content-Length header.
 * @return	This function returns no value.
 */
void http_response_header(connection_t *con, int_t status, stringer_t *type, size_t len) {

	stringer_t *cookie = NULL, *allow = NULL, *connection = NULL;

	// We depend upon this buffer being empty so it gets skipped when the connection label wasn't provided.
	// QUESTION: I don't get this ^
	mm_wipe(&connection, sizeof(connection));

	// Indicate the request has been processed so the requeue method will reset the context and enqueue HTTP processor.
	if (con->http.mode == HTTP_RESPOND) {
		con->http.mode = HTTP_COMPLETE;
	}

	if (magma.http.allow_cross_domain) {
		allow = http_response_allow_cross(con);
	}

	cookie = http_response_cookie(con);
	connection = http_response_connection(con, HTTP_CONNECTION_NEUTRAL);

	con_print(con, "HTTP/1.1 %i %s\r\n" \
		"Date: %s\r\n" \
		"%.*s" \
		"%.*s" \
		"Cache-Control: no-cache\r\n" \
		"Pragma: no-cache\r\n" \
		"Content-Type: %.*s\r\n" \
		"Content-Length: %zu\r\n" \
		"%.*s" \
		"\r\n",
		status, http_response_status(status),
		st_char_get(time_print_gmt(MANAGEDBUF(128), "%a, %d %b %Y %T %Z", time(NULL))),
		(allow ? st_length_int(allow) : 0),	(allow ? st_char_get(allow) : NULL),
		(cookie ? st_length_int(cookie) : 0), (cookie ? st_char_get(cookie) : NULL),
		st_length_int(type), st_char_get(type),
		len,
		(connection ? st_length_int(connection) : 0), (connection ? st_char_get(connection) : NULL));

	st_cleanup(allow);
	st_cleanup(cookie);
	st_cleanup(connection);

	return;
}

/**
 * @brief	Make a response to an http client request.
 * @note	The following http methods aren't supported: PUT, DELETE, HEAD, TRACE, and CONNECT.
 * 			The http server will first attempt to retrieve the requested url as a static page; otherwise the following special locations
 * 			are supported: /portal, /portal/camel, /register, /contact, /report_abuse, /teacher, and /statistics.
 *
 *
 *
 * @return	This function returns no value.
 */
void http_response(connection_t *con) {

	http_content_t *content;

	// Method access recognized but were denying access to it.
	if (con->http.method == HTTP_METHOD_PUT || con->http.method == HTTP_METHOD_DELETE || con->http.method == HTTP_METHOD_HEAD ||
		con->http.method == HTTP_METHOD_TRACE || con->http.method == HTTP_METHOD_CONNECT) {
		con->http.mode = HTTP_ERROR_405;
	}

	// Its a POST request so we need to read the body data.
	else if (con->http.method == HTTP_METHOD_POST && !con->http.body && !con->http.pairs) {
		con->http.mode = HTTP_READ_BODY;
	}

	// Its an OPTIONS request so were sending information the client can use to make future requests.
	else if (con->http.method == HTTP_METHOD_OPTIONS) {
		http_response_options(con);
	}

	// Method not implemented.
	else if (con->http.method == HTTP_METHOD_UNSUPPORTED) {
		con->http.mode = HTTP_ERROR_501;
	}

	// We check this list first so that static resources take precedence. This allows for static content to be served using dynamic application paths.
	else if ((content = http_get_static(con->http.location))) {
		http_response_header(con, 200, content->type, st_length_get(content->resource));
		con_write_st(con, content->resource);
	}
	// A special case: upload through the portal.
	else if (!st_cmp_cs_starts(con->http.location, NULLER("/portal/camel/attach/"))) {
		http_parse_pairs(con);
		portal_upload(con);
	}
	// Online portal. First check whether its a JSON request.
	else if (!st_cmp_ci_starts(con->http.location, PLACER("/portal/camel", 13))) {
		portal_endpoint(con);
	}
	else if (!st_cmp_ci_starts(con->http.location, PLACER("/portal", 7))) {
		portal_process(con);
	}
	else if (!st_cmp_ci_starts(con->http.location, PLACER("/json", 4))) {
		json_api_dispatch(con);
	}

	// The registration engine.
	//else if (!st_cmp_cs_eq(con->http.location, PLACER("/register", 9))) {
	else if (!st_cmp_cs_starts(con->http.location, PLACER("/register", 9))) {

		if (!magma.web.registration) {
			con->http.mode = HTTP_ERROR_403;
		} else {
			http_parse_pairs(con);
			register_process(con);
		}

	}
	// The contact form.
	else if (!st_cmp_cs_eq(con->http.location, PLACER("/contact", 8))) {

		if (!magma.admin.contact) {
			con->http.mode = HTTP_ERROR_403;
		} else {
			http_parse_pairs(con);
			contact_process(con, "Contact");
		}

	}
	// The report abuse form.
	else if (!st_cmp_cs_eq(con->http.location, PLACER("/report_abuse", 13))) {

		if (!magma.admin.abuse) {
			con->http.mode = HTTP_ERROR_403;
		} else {
			http_parse_pairs(con);
			contact_process(con, "Abuse");
		}

	}
	// The statistical filter needs teaching.
	else if (!st_cmp_cs_starts(con->http.location, PLACER("/teacher", 8))) {
		http_parse_pairs(con);
		teacher_process(con);
	}
	// The statistics page.
	else if (!st_cmp_cs_eq(con->http.location, PLACER("/statistics", 11))) {

		if (!magma.web.statistics) {
			con->http.mode = HTTP_ERROR_403;
		} else {
			statistics_process(con);
		}

	}
	// We didn't find the dynamic, or static page requested.
	else {
		con->http.mode = HTTP_ERROR_404;
	}

	// Fail safe! If the mode is still set to HTTP_REQUEST then something went horribly wrong! Its likely this also means nothing was sent to the
	// client to indicate the problem. The bottom line is that we need to force an error state to ensure we don't get trapped in an endless loop.
	if (con->http.mode == HTTP_RESPOND) {
		con->http.mode = HTTP_ERROR_500;
	}

	return;
}
