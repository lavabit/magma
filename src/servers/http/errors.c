
/**
 * @file /magma/servers/http/errors.c
 *
 * @brief	Error page templates.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Redirect the browser to a new location with an HTTP 301 response.
 * @note	SSL downgrades are not possible; in order for an ssl upgrade, magma.web.ssl_redirect must first be set.
 * @param	con			the client's http connection handle.
 * @param	location	the url to which the client will be redirected.
 * @param	tls			if 1, direct to https:// else direct to http:// address.
 * @return	This function returns no value.
 */
void http_print_301(connection_t *con, chr_t *location, int_t tls) {

	stringer_t *redirect = NULL;
	bool_t do_transform = false;
	stringer_t *site;
	chr_t *protocol, numbuf[16];

	// If we're trying to upgrade the security of the connection and the operation isn't supported, throw an error.
	if (tls && (con_secure(con) <= 0) && !magma.web.tls_redirect) {
		log_pedantic("Failed to upgrade 301 redirection: magma.web.tls_redirect was not specified.");
		http_print_403(con);
		return;
	}
	else if (tls != (con_secure(con) > 0)) {
		do_transform = true;
	}

	protocol = (tls == 1) ? "https://" : "http://";

	if (do_transform && tls) {
		site = st_dupe(magma.web.tls_redirect);
	}
	else {

		// If the user tries to downgrade security, don't alow them to do so.
		if (!tls && do_transform) {
			log_pedantic("HTTP redirect from TLS to insecure transport is not currently supported.");
			tls = 1;
		}

		// If we're using a non-standard port, the url needs to reflect that.
		if ((tls && con->http.port != 443) || ((tls <= 0) && con->http.port != 80)) {
			snprintf(numbuf, sizeof(numbuf), ":%u", con->http.port);
			site = st_merge("sn", con->http.host, numbuf);
		}
		else {
			site = st_dupe(con->http.host);
		}

	}

	if (!site || (!(redirect = st_merge("nsn", protocol, site, location)))) {
		st_cleanup(site);
		http_print_500(con);
		return;
	}

	/// LOW: This function should probably move to the response file and be updated to use the cookie/xss helper functions.
	con_print(con, "HTTP/1.1 301 Moved Permanently\r\nLocation: %.*s\r\nContent-Type: text/plain\r\nContent-Length: 28\r\n\r\nContent moved permanently.\r\n",
		st_length_int(redirect), st_char_get(redirect));
	con->http.mode = HTTP_COMPLETE;
	st_free(site);
	st_free(redirect);
	return;

}

/**
 * @brief	Return an HTTP 400 bad client request response to the client.
 * @param	con		the client's http connection handle.
 * @return	This function returns no value.
 */
void http_print_400(connection_t *con) {

	con->http.response.connection = HTTP_CONNECTION_CLOSE;
	http_response_header(con, 400, PLACER("text/plain", 10), 21);
	con_write_st(con, PLACER("Bad client request.\r\n", 21));

	return;
}

/**
 * @brief	Return an HTTP 403 access denied response to the client.
 * @param	con		the client's http connection handle.
 * @return	This function returns no value.
 */
void http_print_403(connection_t *con) {

	con->http.response.connection = HTTP_CONNECTION_CLOSE;
	http_response_header(con, 403, PLACER("text/plain", 10), 16);
	con_write_st(con, PLACER("Access denied.\r\n", 16));

	return;
}

/**
 * @brief	Return an HTTP 404 location not found response to the client.
 * @param	con	the client's http connection handle.
 * @return	This function returns no value.
 */
void http_print_404(connection_t *con) {

	con->http.response.connection = HTTP_CONNECTION_CLOSE;
	http_response_header(con, 404, PLACER("text/plain", 10), 21);
	con_write_st(con, PLACER("Location not found.\r\n", 21));

	return;
}

/**
 * @brief	Return an HTTP 405 method not allowed response to the client.
 * @param	con	the client's http connection handle.
 * @return	This function returns no value.
 */
void http_print_405(connection_t *con) {

	con->http.response.connection = HTTP_CONNECTION_CLOSE;
	http_response_header(con, 405, PLACER("text/plain", 10), 21);
	con_write_st(con, PLACER("Method not allowed.\r\n", 21));

	return;
}

/**
 * @brief	Return an HTTP 500 internal server error response to the client.
 * @param	con	the client's http connection handle.
 * @return	This function returns no value.
 */
void http_print_500(connection_t *con) {

	// If we've encountered an internal issue, we probably want to log it.
	log_options(M_LOG_CRITICAL | M_LOG_STACK_TRACE, "HTTP server logged a 500 internal service error.");
	debug_hook();

	con->http.response.connection = HTTP_CONNECTION_CLOSE;
	http_response_header(con, 500, PLACER("text/plain", 10), 24);
	con_write_st(con, PLACER("Internal server error.\r\n", 24));

	return;
}

/**
 * @brief	Return an HTTP 500 internal server error response to the client, with additional logging information.
 * @param	con		the client's http connection handle.
 * @param	logmsg	a pointer to a null-terminated string with an message describing the cause of the http 500 error.
 * @return	This function returns no value.
 */
void http_print_500_log(connection_t *con, chr_t *logmsg) {

	log_options(M_LOG_CRITICAL, "%s", logmsg);
	con->http.response.connection = HTTP_CONNECTION_CLOSE;
	http_response_header(con, 500, PLACER("text/plain", 10), 24);
	con_write_st(con, PLACER("Internal server error.\r\n", 24));

	return;
}

/**
 * @brief	Return an HTTP 501 method not implemented response to the client.
 * @param	con	the client's http connection handle.
 * @return	This function returns no value.
 */
void http_print_501(connection_t *con) {

	con->http.response.connection = HTTP_CONNECTION_CLOSE;
	http_response_header(con, 501, PLACER("text/plain", 10), 25);
	con_write_st(con, PLACER("Method not implemented.\r\n", 25));

	return;
}
