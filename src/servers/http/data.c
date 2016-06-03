
/**
 * @file /magma/servers/http/data.c
 *
 * @brief	Assorted functions for handling HTTP data.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Free an http data object.
 * @param	data	the http data object to be freed.
 * @return	This function returns no value.
 */
void http_data_free(http_data_t *data) {

	if (data) {
		st_cleanup(data->name);
		st_cleanup(data->value);
		mm_free(data);
	}

	return;
}

/**
 * @brief	Get a name/value pair associated with an http connection, by name.
 * @note	The name/value pairs searched can be supplied by a client through http request headers, or through GET or POST data.
 * @param	con		the connection object to be queried.
 * @param	source	the source of the client supplied value pair: HTTP_DATA_GET, HTTP_DATA_POST or HTTP_DATA_ANY.
 * @param	name	the name associated with the name/value pair to be retrieved.
 * @return	NULL on failure, or a pointer to the http name/value data pair requested on success.
 */
http_data_t * http_data_get(connection_t *con, HTTP_DATA source, chr_t *name) {

	inx_cursor_t *cursor;
	http_data_t *data, *result = NULL;


	/// TODO: We could just use the index find function.
	if (con->http.pairs && (cursor = inx_cursor_alloc(con->http.pairs))) {
		while (!result && (data = inx_cursor_value_next(cursor))) {
			if ((data->source == source || source == HTTP_DATA_ANY) && !st_cmp_ci_eq(data->name, NULLER(name))) {
				result = data;
			}
		}
		inx_cursor_free(cursor);
	}

	if (con->http.headers && (cursor = inx_cursor_alloc(con->http.headers))) {
		while (!result && (data = inx_cursor_value_next(cursor))) {
			if ((data->source == source || source == HTTP_DATA_ANY) && !st_cmp_ci_eq(data->name, NULLER(name))) {
				result = data;
			}
		}
		inx_cursor_free(cursor);
	}

	return result;
}

/**
 * @brief	Decode an escaped URI component into its original data.
 * @note	Since the un-escaped string will always be at least as small as the encoded value, the input managed string is transformed in place.
 * @param	string	a managed string containing the escaped URI data to be decoded.
 * @return	This function returns no value.
 */
void http_data_value_decode(stringer_t *string) {

	size_t length;
	chr_t *reader, *writer;
	byte_t this_byte;

	// Setup the pointers, and lengths.
	length = st_length_get(string);
	reader = writer = st_char_get(string);

	// Iterate through. When we encounter a percent sign, assume the following two characters are the value in hex.
	while (length > 0) {
		// If we don't have enough data for a valid escape code, or don't have one otherwise, then don't parse it.
		if ((*reader == '%') && (length >= 3) && (this_byte = hex_decode_chr(*(reader + 1), *(reader + 2)))) {
			*writer++ = this_byte;
			reader += 3;
			length -= 3;
		}
		else if (*reader == '+') {
			*writer++ = ' ';
			reader++;
			length--;
		}
		else {
			*writer++ = *reader++;
			length--;
		}
	}

	*writer = '\0';
	st_length_set(string, writer - st_char_get(string));

	return;
}

/**
 * @brief	Parse a string containing a name/value pair, and place it in the specified connection's pairs holder.
 * @note	Each name/value pair is assumed to be delimited with a '=' character, and each half is URI-decoded before storage.
 * @param	con		a pointer to the connection object of the http client submitting the user data to be parsed.
 * @param	source	an HTTP_DATA value specifying the source of the name/value pair (can be HTTP_DATA_HEADER, HTTP_DATA_GET, or HTTP_DATA_POST).
 * @param	pair	a placer pointing to a string containing the name/value pair to be parsed.
 * @return	0 on general or parsing failure, or 1 if the specified input buffer was successfully parsed and stored.
 */
int_t http_data_value_parse(connection_t *con, HTTP_DATA source, placer_t pair) {

	placer_t fragment;
	http_data_t *data;
	stringer_t *name, *value = NULL;
	multi_t key = { .type = M_TYPE_STRINGER, .val.st = NULL };

	if (pl_empty(pair)) {
		return 0;
	}

	if (tok_get_pl(pair, '=', 0, &fragment) >= 0 && (name = st_dupe_opts(MANAGED_T | HEAP | CONTIGUOUS, &fragment))) {
		http_data_value_decode(name);
	}
	else {
		return 0;
	}

	// Sometimes the value can be null.
	if (tok_get_pl(pair, '=', 1, &fragment) >= 0 && (value = st_dupe_opts(MANAGED_T | HEAP | CONTIGUOUS, &fragment))) {
		http_data_value_decode(value);
	}

	if ((data = mm_alloc(sizeof(http_data_t))) == NULL) {
		if (value) st_free(value);
		st_free(name);
		return 0;
	}

	data->name = name;
	key.val.st = name;
	data->value = value;
	data->source = source;

	// Print the post values before they are stored.
	if (magma.log.http) {
		log_pedantic("%.*s - %.*s", st_length_int(name), st_char_get(name), st_length_int(value), st_char_get(value));
	}

	if (inx_insert(con->http.pairs, key, data) != 1) {
		http_data_free(data);
		return 0;
	}

	return 1;
}

/**
 * @brief	Parse a data buffer into a http header name/value pair.
 * @param	con		the connection object of the http client to be read.
 * @return	NULL on failure, or a pointer to an http header name/value pair on success.
 */
http_data_t * http_data_header_parse_line(chr_t *buf, size_t len) {

	chr_t *start, *stream = buf;
	http_data_t *result;
	size_t position = 0;
	stringer_t *name = NULL, *value = NULL;

	if (!buf || !len) {
		return NULL;
	}

	// Check for the end of the header break.
	if ((*stream == '\r' && *(stream + 1) == '\n') || *stream == '\n') {
		return NULL;
	}

	while (position != len && *stream != '\r' && *stream != '\n' && *stream != ':') {
		stream++;
		position++;
	}

	// Save the name.
	if (position == 0 || *stream != ':' || (name = st_import(buf, position)) == NULL) {
		return NULL;
	}

	// Skip the junk.
	while (position != len && *stream != '\r' && *stream != '\n' && (*stream == ':' || *stream == ' ')) {
		stream++;
		position++;
	}

	// Store the start position.
	start = stream;
	while (position != len && *stream != '\r' && *stream != '\n') {
		stream++;
		position++;
	}

	// Error check and store.
	if (stream == start || (value = st_import(start, stream - start)) == NULL) {
		st_free(name);
		return NULL;
	}

	if ((result = mm_alloc(sizeof(http_data_t))) == NULL) {
		st_free(value);
		st_free(name);
		return NULL;
	}

	result->name = name;
	result->value = value;
	result->source = HTTP_DATA_HEADER;

	return result;
}

/**
 * @brief	Parse the current line of input from an http client connection into a http header name/value pair.
 * @param	con		the connection object of the http client to be read.
 * @return	NULL on failure, or a pointer to an http header name/value pair on success.
 */
http_data_t * http_data_header_parse(connection_t *con) {

/*	chr_t *stream, *start;
	http_data_t *result;
	size_t position = 0, len;
	stringer_t *name = NULL, *value = NULL; */

	return (http_data_header_parse_line(st_char_get(&(con->network.line)), st_length_get(&(con->network.line))));

	/*if (st_empty_out(&(con->network.line), (uchr_t **)&stream, &len)) {
		return NULL;
	}

	// Check for the end of the header break.
	if ((*stream == '\r' && *(stream + 1) == '\n') || *stream == '\n') {
		return NULL;
	}

	while (position != len && *stream != '\r' && *stream != '\n' && *stream != ':') {
		stream++;
		position++;
	}

	// Save the name.
	if (position == 0 || *stream != ':' || (name = st_import(st_char_get(&(con->network.line)), position)) == NULL) {
		return NULL;
	}

	// Skip the junk.
	while (position != len && *stream != '\r' && *stream != '\n' && (*stream == ':' || *stream == ' ')) {
		stream++;
		position++;
	}

	// Store the start position.
	start = stream;
	while (position != len && *stream != '\r' && *stream != '\n') {
		stream++;
		position++;
	}

	// Error check and store.
	if (stream == start || (value = st_import(start, stream - start)) == NULL) {
		st_free(name);
		return NULL;
	}

	if ((result = mm_alloc(sizeof(http_data_t))) == NULL) {
		st_free(value);
		st_free(name);
		return NULL;
	}

	result->name = name;
	result->value = value;
	result->source = HTTP_DATA_HEADER;

	return result;*/
}
