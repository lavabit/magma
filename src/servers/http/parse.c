
/**
 * @file /magma/servers/http/parse.c
 *
 * @brief	Functions used to parse an HTTP request.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Get the origin of a resource from a url.
 * @note	Usually we'll be give the value of the Origin header field to work with so we'll end up returning the entire string,
 * 			but if we had to fall back and use the Referrer instead this should strip off the path portion.
 * @param	s		a managed string containing the url to be parsed.
 * @param	output	a pointer to a placer that will be set to point to the origin string inside the user-supplied url.
 * @return	0 on success or -1 on failure.
 */
int_t http_parse_origin(stringer_t *s, placer_t *output) {

	uchr_t *p;
	size_t len;
	int_t result = 1;

	if (st_empty_out(s, &p, &len) || !output) {
		return -1;
	}

	// Skip past the protocol portion.
	if (!st_cmp_ci_starts(s, PLACER("http://", 7))) {
		p += 7;
		len -= 7;
	}
	else if (!st_cmp_ci_starts(s, PLACER("https://", 8))) {
		p += 8;
		len -= 8;
	}
	else {
		return -1;
	}

	// Make sure we have at least one legitimate character following the protocol specification.
	if (!len || !*p) {
		return -1;
	}

	while (len && result) {
		if (*p && *p != '/') {
			len--;
			p++;
		}
		else {
			*output = pl_init(st_data_get(s), p - st_uchar_get(s));
			result = 0;
			// QUESTION: Shouldn't we break out of the loop here?
		}
	}

	// We hit the end of the string.
	if (result) {
		*output = pl_init(st_data_get(s), p - st_uchar_get(s));
		result = 0;
	}

	return result;
}

/**
 * @brief	Parse all the GET and POST parameters present in an http client request, and store them with the connection.
 * @note	All valid field parameters will be stored in the connection's http.pairs object.
 * @param	con		a pointer to the connection object generating the http requesting being processed.
 * @return	This function returns no value.
 */
void http_parse_pairs(connection_t *con) {

	placer_t pair, data;
	size_t count, position;

	// Grab and parse the POST data.
	if (!con->http.pairs && (con->http.pairs = inx_alloc(M_INX_LINKED, &http_data_free)) == NULL) {
		con->http.mode = HTTP_ERROR_500;
		return;
	}

	// TODO: This body parsing is terrible and needs to be corrected.
	// If present, parse any posted data.
	else if (con->http.body) {
		count = tok_get_count_st(con->http.body, '&');

		for (size_t i = 0; i < count && tok_get_st(con->http.body, '&', i, &pair) >= 0; i++) {
			http_data_value_parse(con, HTTP_DATA_POST, pair);

		}
	}

	// Then check for any data pairs in the request location.
	if (st_search_cs(con->http.location, PLACER("?", 1), &position)) {

		data = pl_init(st_char_get(con->http.location) + position + 1, st_length_get(con->http.location) - position - 1);
		count = tok_get_count_st(&data, '&');
		for (size_t i = 0; i < count && tok_get_st(&data, '&', i, &pair) >= 0; i++) {
			http_data_value_parse(con, HTTP_DATA_GET, pair);
		}
	}

	return;
}

/**
 * @brief	Attempt to retrieve a connected user's associated session, by searching the cookie, the POST "session" variable, and the URL.
 * @param	con				a pointer to the connection object to be queried for a session id.
 * @param	application	 	a managed string containing the application associated with the connection's pending request.
 * @param	path			a managed string containing the path associated with the connection's pending request.
 * @return	This function returns no value.
 */
void http_parse_context(connection_t *con, stringer_t *application, stringer_t *path) {

	json_error_t err;
	placer_t pair, name, value;
	json_t *object = NULL, *val;
	int_t result;

	// Look for the session token as a cookie.
	if (con->http.cookie) {

		// Doesn't handle clients that submit more than one cookie, but its a start.
/*		if ((tok_get_st(con->http.cookie, ';', 0, &pair) < 0) || (tok_get_st(&pair, '=', 0, &name) < 0) || (tok_get_st(&pair, '=', 1, &value))) {
			return;
		}*/
        tok_get_st(con->http.cookie, ';', 0, &pair);
        tok_get_st(&pair, '=', 0, &name);
        tok_get_st(&pair, '=', 1, &value);

		/// TODO: Develop better logic for handling cookies. Namely add the ability to focibly trigger the Set-Cookie entity and if necessary
		/// delete the existing cookie.

		// If the cookie didn't leave us with an active session we remove it from the request context so that if a session does get created
		// the new token will be sent along with the response and overwrite this invalid cookie.
		result = sess_get(con, &name, path, &value);

	}

	// If the request was posted, see if the data is a valid JSON object, and whether it contains a session reference.
	if (!con->http.session && con->http.method == HTTP_METHOD_POST && (object = json_loads_d(st_char_get(con->http.body), 0, &err)) &&
		(val = json_object_get_d(object, "session")) && json_is_string(val)) {
		result = sess_get(con, application, path, NULLER((chr_t *)json_string_value_d(val)));
	}

	// If that fails, see if the session token was passed via the URL.
	if (!con->http.session && !st_cmp_ci_starts(con->http.location, path) && st_length_get(con->http.location) > st_length_get(path)) {
		pair = pl_init(st_char_get(con->http.location) + st_length_get(path), st_length_get(con->http.location) - st_length_get(path));
		if (tok_get_pl(pair, '/', 1, &value) < 0) {
			result = sess_get(con, application, path, &pair);
		}
		else {
			result = sess_get(con, application, path, &value);
		}
	}

	if (object) {
		json_decref_d(object);
	}

	return;
}

/**
 * @brief	Parse and process the current line of input from an http client connection as an http request header, storing data in the connection's http.headers member.
 * @note	If no more http headers can be read, control is returned by setting the connection http.mode to HTTP_RESPOND.
 * 			Special actions are taken to store the "Host", "User-Agent", "Cookie", and "Connection" headers.
 * @param	con		the connection object of the http client to be read, and to store the results of the operation.
 * @return	This function returns no value.
 */
void http_parse_header(connection_t *con) {

	http_data_t *data;
	size_t position = 0;
	placer_t pl;
	multi_t key = {
		.type = M_TYPE_STRINGER, .val.st = NULL
	};

	// Make sure we have a linked list available to store the header.
	if (!con->http.headers && !((con->http.headers = inx_alloc(M_INX_LINKED, &http_data_free)))) {
		con->http.mode = HTTP_ERROR_500;
		return;
	}

	if (!(data = http_data_header_parse(con))) {
		con->http.mode = HTTP_RESPOND;
		return;
	}

	// Print_t the header name/values as they are stored.
	if (magma.log.http) {
		log_info("%.*s - %.*s", st_length_int(data->name), st_char_get(data->name), st_length_int(data->value), st_char_get(data->value));
	}

	// Store the Host and User-Agent strings using dedicated session variables.
	if (!con->http.host && !st_cmp_ci_eq(data->name, PLACER("Host", 4))) {
		if (st_search_cs(data->value, PLACER(":", 1), &position)) {

			// Record the port value so we can make intelligent decisions about whether to include the domain/host with cookies. If we have trouble
			// extracting a legitimate value store -1 so the cookie handler still knows not to use the domain. If the port is set to 0, 80, or 443
			// the cookie handler will send along the domain attribute.
			if (position == st_length_get(data->value) || int32_conv_bl(st_char_get(data->value) + position + 1,
				st_length_get(data->value) - position - 1, &(con->http.port)) != true || !con->http.port) {
				con->http.port = -1;
			}

			// Then trim the string before adding it to the context.
			st_length_set(data->value, position);
		}

		con->http.host = st_dupe_opts(MANAGED_T | HEAP | CONTIGUOUS, data->value);
		lower_st(con->http.host);
		http_data_free(data);
	}

	// Store the user agent.
	else if (!con->http.agent && !st_cmp_ci_eq(data->name, PLACER("User-Agent", 10))) {
		con->http.agent = st_dupe_opts(MANAGED_T | HEAP | CONTIGUOUS, data->value);
		http_data_free(data);
	}

	// Detect the presence of a cookie tag and save it for parsing later.
	else if (!con->http.cookie && !st_cmp_ci_eq(data->name, PLACER("Cookie", 6))) {
		con->http.cookie = st_dupe_opts(MANAGED_T | HEAP | CONTIGUOUS, data->value);
		http_data_free(data);
	}

	/// LOW: Should we bother to throw an error if con->http.connection isn't HTTP_CONNECTION_NEUTRAL?
	// Should we close the connection? RFC 2068 dictates the "Keep-Alive" header should be ignore if it isn't accompanied by
	// the corresponding "Connection" token.
	else if (!st_cmp_ci_eq(data->name, PLACER("Connection", 10)) && !st_cmp_ci_eq(data->value, PLACER("keep-alive", 10))) {
		con->http.response.connection = HTTP_CONNECTION_KEEPALIVE;
		http_data_free(data);
	}
	else if (!st_cmp_ci_eq(data->name, PLACER("Connection", 10)) && !st_cmp_ci_eq(data->value, PLACER("close", 5))) {
		con->http.response.connection = HTTP_CONNECTION_CLOSE;
		http_data_free(data);
	}
	else if (!st_cmp_ci_eq(data->name, PLACER("Expect",6))) {
		pl = get_header_value_noopt(data->value);

		// If the client sends this, we should send back a quick acknowledgement.
		if (!st_cmp_ci_eq(&pl, PLACER("100-continue",12))) {
			con_print(con, "HTTP/1.1 100 Continue\r\n\r\n");
		}

	}

	else if (!(key.val.st = data->name) || inx_insert(con->http.headers, key, data) != 1) {
		http_data_free(data);
	}

	return;
}

/**
 * @brief	Parse an http request and determine the request method and location.
 * @note	This function returns no value but sets the internal method, location, and state of the underlying connection object.
 * @param	con		the client connection making an http request.
 * @return	This function returns no value.
 */
void http_parse_method(connection_t *con) {

	placer_t location;

	// Detect the method type.
	if (!st_cmp_ci_starts(&(con->network.line), PLACER("GET", 3)))
		con->http.method = HTTP_METHOD_GET;
	else if (!st_cmp_ci_starts(&(con->network.line), PLACER("POST", 4)))
		con->http.method = HTTP_METHOD_POST;
	else if (!st_cmp_ci_starts(&(con->network.line), PLACER("PUT", 3)))
		con->http.method = HTTP_METHOD_PUT;
	else if (!st_cmp_ci_starts(&(con->network.line), PLACER("DELETE", 6)))
		con->http.method = HTTP_METHOD_DELETE;
	else if (!st_cmp_ci_starts(&(con->network.line), PLACER("HEAD", 4)))
		con->http.method = HTTP_METHOD_HEAD;
	else if (!st_cmp_ci_starts(&(con->network.line), PLACER("TRACE", 5)))
		con->http.method = HTTP_METHOD_TRACE;
	else if (!st_cmp_ci_starts(&(con->network.line), PLACER("OPTIONS", 7)))
		con->http.method = HTTP_METHOD_OPTIONS;
	else if (!st_cmp_ci_starts(&(con->network.line), PLACER("CONNECT", 7)))
		con->http.method = HTTP_METHOD_CONNECT;
	else {
		con->http.method = HTTP_METHOD_UNSUPPORTED;
	}

	// Get the location.
	if (tok_get_count_st(&(con->network.line), ' ') >= 2 && tok_get_pl(con->network.line, ' ', 1, &location) >= 0) {
		con->http.location = st_dupe_opts(MANAGED_T | HEAP | CONTIGUOUS, &location);
	}

	if (magma.log.http && con->http.location) {
		log_info("Location - %.*s", st_length_int(con->http.location), st_char_get(con->http.location));
	}

	con->http.mode = HTTP_PARSE_HEADER;
	return;
}

/**
 * @brief	Get the simple value of an http header, with any optional parameters stripped away.
 * @note	Only the value after the ":" in a header field is passed to this function (in other words, the header name is omitted).
 * @param	vstring		a managed string containing the single http header line value to be parsed.
 * @return	a placer pointing to the option-free header value of the specified input line.
 */
placer_t get_header_value_noopt(stringer_t *vstring) {

	placer_t val;

	if (!tok_get_count_st(vstring, ';')) {
			return (pl_init(st_char_get(vstring), st_length_get(vstring)));
	}

	// This should never happen but...
	if (tok_get_st(vstring, ';', 0, &val) < 0) {
		return (pl_init(st_char_get(vstring), st_length_get(vstring)));
	}

	return val;
}

/**
 * @brief	Get the value of a named optional parameter from an http header value.
 * @note	Only the value after the ":" in a header field is passed to this function (in other words, the header name is omitted).
 * @param	vstring		a managed string containing the single http header line value to be parsed.
 * @param	optname		a managed string with the name of the optional parameter to be found.
 * @return	a placer pointing to the named optional parameter of the specified http header value.
 */
placer_t get_header_opt(stringer_t *vstring, stringer_t *optname) {

	placer_t val, vname, vval;
	uint64_t tindex = 1;
	int rtok;

	if (!tok_get_count_st(vstring, ';')) {
		return pl_null();
	}

	while ((rtok = tok_get_st(vstring, ';', tindex, &val)) >= 0) {
		val = pl_trim_start(val);

		// Can't be the last token... should be one more.
		if (!tok_get_st(&val, '=', 0, &vname)) {
			// But this should be the last token because there should only be two.
			if (!st_cmp_cs_eq(optname, &vname) && tok_get_st(&val, '=', 1, &vval)) {
				return vval;
			}

		}

		if (rtok)
			break;

		tindex++;
	}

	return pl_null();
}

/**
 * @brief	Get the boundary delimiter for a request by a connection specifying a Content-Type of multipart/form-data.
 * @param	con		a pointer to the connection object of the client making the http request.
 * @param	output	a pointer to the address of a managed string that will receive a copy of the value of the boundary string on success.
 * @return	true on success or false on failure.
 */
bool_t multipart_get_boundary(connection_t *con, placer_t *output) {

	http_data_t *content_type;
	placer_t ctypestr, ctypeval;

	if (!(content_type = http_data_get(con, HTTP_DATA_HEADER, "Content-Type"))) {
		return false;
	}

	ctypestr = get_header_value_noopt(content_type->value);

	if (pl_empty(ctypeval = get_header_opt(content_type->value, NULLER("boundary")))) {
		return false;
	}

	*output = ctypeval;

	return true;
}
