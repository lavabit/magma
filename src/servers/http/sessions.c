
/**
 * @file /magma/servers/http/sessions.c
 *
 * @brief	HTTP session handlers.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Reset a client http connection to its original, uninitialized state.
 * @param	con		the connection object to have its http session state reset.
 * @return	This function returns no value.
 */
void http_session_reset(connection_t *con) {

	st_cleanup(con->http.host);
	con->http.host = NULL;

	st_cleanup(con->http.agent);
	con->http.agent = NULL;

	st_cleanup(con->http.cookie);
	con->http.cookie = NULL;

	st_cleanup(con->http.location);
	con->http.location = NULL;

	st_cleanup(con->http.body);
	con->http.body = NULL;

	inx_cleanup(con->http.pairs);
	con->http.pairs = NULL;

	inx_cleanup(con->http.headers);
	con->http.headers = NULL;

	// Handle the web application context.
	if (con->http.merged == HTTP_PORTAL) {

		if (con->http.portal.request) {
			json_decref_d(con->http.portal.request);
		}

		con->http.portal.id = 0;
		con->http.portal.params = NULL;
		con->http.portal.request = NULL;
	}

	// Release the session reference.
	if (con->http.session) {
		sess_release(con->http.session);
		con->http.session = NULL;
	}

	// Check the mode.
	if (con->http.response.connection == HTTP_CONNECTION_CLOSE) {
		con->http.mode = HTTP_CLOSE;
	}
	else {
		con->http.mode = HTTP_READY;
	}

	con->http.port = 0;
	con->http.merged = HTTP_MERGED;
	con->http.method = HTTP_METHOD_NONE;
	con->http.response.connection = HTTP_CONNECTION_NEUTRAL;

	return;
}

/**
 * @brief	Destroy an http client connection.
 * @param	con		the connection object to have its http session destroyed.
 * @return	This function returns no value.
 */
void http_session_destroy(connection_t *con) {

	http_session_reset(con);
	return;
}
