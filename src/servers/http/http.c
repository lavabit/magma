
/**
 * @file /magma/servers/http/http.c
 *
 * @brief	Functions used to handle HTTP commands and actions.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Close a connection corresponding to an http session.
 * @return	This function returns no value.
 */
void http_close(connection_t *con) {

	con_destroy(con);
	return;
}

/**
 * @brief	The main http server requeue entry point state machine for processing client data.
 * @return	This function returns no value.
 */
void http_requeue(connection_t *con) {

	if (!status() || con_status(con) < 0 || con->http.mode == HTTP_CLOSE || con->protocol.violations > con->server->violations.cutoff) {
		enqueue(&http_close, con);
	}
	else if (con->http.mode == HTTP_RESPOND) {
		requeue(&http_response, &http_requeue, con);
	}
	else if (con->http.mode == HTTP_READ_BODY) {
		requeue(&http_body, &http_requeue, con);
	}
	else if (con->http.mode == HTTP_PARSE_PAIRS) {
		requeue(&http_parse_pairs, &http_requeue, con);
	}
	else if (con->http.mode == HTTP_COMPLETE) {
		requeue(&http_session_reset, &http_process, con);
	}
	else if (con->http.mode == HTTP_ERROR_501) {
		requeue(&http_print_501, &http_close, con);
	}
	else if (con->http.mode == HTTP_ERROR_500) {
		requeue(&http_print_500, &http_close, con);
	}
	else if (con->http.mode == HTTP_ERROR_405) {
		requeue(&http_print_405, &http_close, con);
	}
	else if (con->http.mode == HTTP_ERROR_404) {
		requeue(&http_print_404, &http_close, con);
	}
	else if (con->http.mode == HTTP_ERROR_403) {
		requeue(&http_print_403, &http_close, con);
	}
	else if (con->http.mode == HTTP_ERROR_400) {
		requeue(&http_print_400, &http_close, con);
	}
	// HTTP_PARSE_HEADER and HTTP_READY should trigger the process function.
	else {
		enqueue(&http_process, con);
	}

	return;
}

/**
 * @brief	Get the body of the http request by reading the value of the Content-Length header.
 * @note	Any request errors will be handled directly without returns. This function sets the value of the connection's http.body member.
 * @param	con		a pointer to the connection object of the remote http client.
 * @return	This function returns no value.
 */
void http_body(connection_t *con) {

	http_data_t *data;
	size_t read, length;

	// Get the content length.
	if (!(data = http_data_get(con, HTTP_DATA_HEADER, "Content-Length")) || size_conv_bl(st_data_get(data->value),
		st_length_get(data->value), &length) != 1) {
		con->http.mode = HTTP_ERROR_400;
		return;
	}

	// If the length is zero we assign an empty string to the connection body so the next call into the responder realizes the body data has been read.
	else if (!length) {
		con->http.body = st_alloc_opts(MANAGED_T | HEAP | CONTIGUOUS, 0);
		con->http.mode = HTTP_RESPOND;
		return;
	}

	// If the length indicates we should be expecting more than a megabyte of data, manually allocate a memory mapped string. Were assuming the
	// kernel will be more likely to store the data on disk if memory comes in short supply and that operations on this buffer are not likely to
	// suffer from the performance penalty. If it appears the data will be less than 1 megabyte or an error occurs we can just use a jointed
	// memory string and grow it by 32 KB each time.
	if (!con->http.body && length > 1048576) {
		con->http.body = st_alloc_opts(MAPPED_T | HEAP | JOINTED, length);
	}

	// There should be more data for us to read.
	if (length && (!con->http.body || st_length_get(con->http.body) < length) && (read = con_read(con)) > 0) {
		con->http.body = st_append_opts(32768, con->http.body, con->network.buffer);
	}

	// When were done reading the body reset the mode to respond and the requeue function will route accordingly.
	if (!length || (con->http.body && st_length_get(con->http.body) >= length)) {
		con->http.mode = HTTP_RESPOND;
	}

	// HIGH: It seems like this allows the entire body not to be read in.
	// Print the post values before they are stored.
	if ((con->http.body && st_length_get(con->http.body) >= length) && magma.log.http) {
		log_pedantic("%.*s", st_length_int(con->http.body), st_char_get(con->http.body));
	}

	return;
}

/**
 * @brief	Process data sent by an http client.
 * @note	This is performed in two stages: first read the http method with http_parse_method(), then transfer control to http_parse_header().
 * 			If a failure occurs reading a line of data, or too many protocol violations occur, http_close() is called to drop the connection.
 * @param	con		the connection object from which to read data.
 * @return	This function returns no value.
 */
void http_process(connection_t *con) {

	int_t state;

	if (((state = con_read_line(con, false)) == -1) || (state == -2)) {
		enqueue(&http_close, con);
		return;
	}
	else if (pl_empty(con->network.line) && ((con->protocol.spins++) + con->protocol.violations) > con->server->violations.cutoff) {
		enqueue(&http_close, con);
		return;
	}
	else if (pl_empty(con->network.line)) {
		enqueue(&http_process, con);
		return;
	}

	// Get the method and the location.
	if (con->http.mode == HTTP_READY) {
		requeue(&http_parse_method, &http_requeue, con);
	}
	// Parse the request header. If the end of the header is reached, the mode will be updated and the requeue function will call the
	// respond function.
	else if (con->http.mode == HTTP_PARSE_HEADER) {
		requeue(&http_parse_header, &http_requeue, con);
	}

	// If we end up here its time to find another pot of coffee.
	else {
		log_pedantic("Asked to process an HTTP connection in an unrecognized state. { mode = %i }", con->http.mode);
		enqueue(&http_close, con);
	}

	return;
}

/**
 * @brief	Handle a new http client connection.
 * @param	con		a pointer to the http client connection that was just accepted.
 * @return	This function returns no value.
 */
void http_init(connection_t *con) {

	con_reverse_enqueue(con);
	http_process(con);

	return;
}
