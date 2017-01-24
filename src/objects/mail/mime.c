
/**
 * @file /magma/objects/mail/mime.c
 *
 * @brief	Functions used to parse and manipulate MIME messages.
 */

#include "magma.h"

media_type_t media_types[] = {
		{ "",       true,  "application/octet-stream"},
		{ ".aif",   true,  "audio/aiff" },
		{ ".aiff",  true,  "audio/aiff" },
		{ ".au",    true,  "audio/basic" },
		{ ".avi",   true,  "video/avi" },
		{ ".bmp",   true,  "image/bmp" },
		{ ".bz",    true,  "application/x-bzip" },
		{ ".bz2",   true,  "application/x-bzip2" },
		{ ".c",     false, "text/x-c" },
		{ ".class", true,  "application/java" },
		{ ".conf",  false, "text/plain" },
		{ ".cpp",   false, "text/x-c" },
		{ ".crt",   true,  "application/x-x509-ca-cert" },
		{ ".css",   false, "text/css" },
		{ ".csv",   false, "text/csv" },
		{ ".doc",   true,  "application/msword" },
		{ ".dot",   true,  "application/msword" },
		{ ".flv",   true,  "video/x-flv" },
		{ ".gif",   true,  "image/gif" },
		{ ".gz",    true,  "application/gzip" },
		{ ".h",     false, "text/x-h" },
		{ ".htm",   false, "text/html" },
		{ ".html",  false, "text/html" },
		{ ".ico",   true,  "image/x-icon" },
		{ ".inf",   false, "application/inf" },
		{ ".java",  false, "text/x-java-source" },
		{ ".jfif",  true,  "image/jpeg" },
		{ ".jpe",   true,  "image/jpeg" },
		{ ".jpeg",  true,  "image/jpeg" },
		{ ".jpg",   true,  "image/jpeg" },
		{ ".js",    false, "application/javascript" },
		{ ".json",  false, "application/json" },
		{ ".lst",   true,  "text/plain" },
		{ ".lzh",   true,  "application/x-lzh" },
		{ ".lzs",   true,  "application/x-lzh" },
		{ ".m1v",   true,  "video/mpeg" },
		{ ".m2v",   true,  "video/mpeg" },
		{ ".m4v",   true,  "video/mpeg" },
		{ ".mid",   true,  "audio/midi" },
		{ ".midi",  true,  "audio/midi" },
		{ ".mov",   true,  "video/quicktime" },
		{ ".mp3",   true,  "audio/mpeg3" },
		{ ".mpeg",  true,  "video/mpeg" },
		{ ".mpg",   true,  "video/mpeg" },
		{ ".ogg",   true,  "application/ogg" },
		{ ".pdf",   true,  "application/pdf" },
		{ ".pl",    false, "text-script.perl" },
		{ ".png",   true,  "image/png" },
		{ ".pps",   true,  "application/mspowerpoint" },
		{ ".ppt",   true,  "application/powerpoint" },
		{ ".ps",    true,  "application/postscript" },
		{ ".py",    false, "text/x-script.python" },
		{ ".qt",    true,  "video/quicktime" },
		{ ".ra",    true,  "audio/x-realaudio" },
		{ ".rss",   true,  "application/rss+xml" },
		{ ".rtf",   false, "text/rtf" },
		{ ".s",     false, "text/x-asm" },
		{ ".sgm",   false, "text/x-sgml" },
		{ ".smgl",  false, "text/x-sgml" },
		{ ".sh",    false, "application/x-sh" },
		{ ".shtml", false, "text/html" },
		{ ".swf",   true,  "application/x-shockwave-flash" },
		{ ".tar",   true,  "application/x-tar" },
		{ ".tcl",   false, "application-xtcl" },
		{ ".text",  false, "text/plain" },
		{ ".tgz",   true,  "application/x-compressed" },
		{ ".tif",   true,  "image/tiff" },
		{ ".tiff",  true,  "image/tiff" },
		{ ".txt",   false, "text/plain" },
		{ ".uu",    false, "text/x-uuencode" },
		{ ".uue",   false, "text/x-uuencode" },
		{ ".wav",   true,  "audio/wav" },
		{ ".wmv",   true,  "video/x-ms-wmv" },
		{ ".wp5",   true,  "application/wordperfect" },
		{ ".wp6",   true,  "application/wordperfect" },
		{ ".xbm",   true,  "image/xbm" },
		{ ".xl",    true,  "application/excel" },
		{ ".xls",   true,  "application/excel" },
		{ ".xml",   false, "text/xml" },
		{ ".z",     true,  "application/x-compressed" },
		{ ".zip",   true,  "application/zip" },
};


/**
 * @brief	Get the media type for a given file extension.
 * @note	If no direct match is found for the content, "application/octet-stream" will be returned.
 * @param	extension	a pointer to a null-terminated string containing the file extension to be looked up, starting with a period.
 * @return	a pointer to a media type object corresponding to the media type of the specified file extension.
 */
media_type_t * mail_mime_get_media_type (chr_t *extension) {

	size_t cmplen = ns_length_get(extension) + 1;

	for (size_t i = 1; i < sizeof(media_types) / sizeof(media_type_t); i++) {

		if (!mm_cmp_cs_eq(media_types[i].extension, extension, cmplen)) {
			return (&(media_types[i]));
		}

	}

	return (&(media_types[0]));
}

/**
 * @brief	Get the value of the Content-Type header from a mime header.
 * @param	header	a placer pointing to the mime header to be parsed.
 * @return	a managed string containing the content type value of the header, with "text" as the default.
 */
stringer_t * mail_mime_type_group(placer_t header) {

	chr_t *stream;
	stringer_t *line, *result;
	size_t remaining, characters = 0;

	if (!(line = mail_header_fetch_cleaned(&header, PLACER("Content-Type", 12)))) {
		return st_import("text", 4);
	}

	stream = st_char_get(line);
	remaining = st_length_get(line);

	// Advance past any garbage.
	while (remaining != 0 && (*stream == '\n' || *stream == '\r' || *stream == ' ' || *stream == '\t' || *stream == '"')) {
		stream++;
		remaining--;
	}

	// Count how many characters are in the group. Based on RFC4288, section 4.2.
	while (remaining != 0 && ((*stream >= 'a' && *stream <= 'z') || (*stream >= 'A' && *stream <= 'Z') || (*stream >= '0' && *stream <= '9') ||
		*stream == '!' || *stream == '#' || *stream == '$' || *stream == '&' || *stream == '.' || *stream == '+' || *stream == '-' ||
		*stream == '^' || *stream == '_')) {
		stream++;
		remaining--;
		characters++;
	}

	// Make sure we got something back. Use a default value of text.
	if (!characters) {
		st_free(line);
		return st_import("text", 4);
	}

	result = st_import(stream - characters, characters);
	st_free(line);

	return result;
}

/**
 * @brief	Get the subtype of the Content-Type header value from a mime header.
 * @note	For example in the case of a Content-Type of 'text/plain', "plain" would be the subtype.
 * @param	header	a placer pointing to the mime header to be parsed.
 * @return	a managed string containing the content subtype of the header, with "plain" as the default.
 */
stringer_t * mail_mime_type_sub(placer_t header) {

	chr_t *stream;
	stringer_t *line, *result;
	size_t remaining, characters = 0;

	if (!(line = mail_header_fetch_cleaned(&header, PLACER("Content-Type", 12)))) {
		return st_import("plain", 5);
	}

	stream = st_char_get(line);
	remaining = st_length_get(line);

	// Advance past any garbage.
	while (remaining && (*stream == '\n' || *stream == '\r' || *stream == ' ' || *stream == '\t' || *stream == '"')) {
		stream++;
		remaining--;
	}

	// Count how many characters are in the group. Based on RFC4288, section 4.2.
	while (remaining && ((*stream >= 'a' && *stream <= 'z') || (*stream >= 'A' && *stream <= 'Z') || (*stream >= '0' && *stream <= '9') ||
		*stream == '!' || *stream == '#' || *stream == '$' || *stream == '&' || *stream == '.' || *stream == '+' || *stream == '-' ||
		*stream == '^' || *stream == '_')) {
		stream++;
		remaining--;
	}

	// Make sure we got something back. Use a default value of plain.
	if (!remaining || *stream != '/') {
		st_free(line);
		return st_import("plain", 5);
	}

	// Advance past the separator.
	stream++;
	remaining--;

	// Count how many characters are in the subtype. Based on RFC4288, section 4.2.
	while (remaining != 0 && ((*stream >= 'a' && *stream <= 'z') || (*stream >= 'A' && *stream <= 'Z') || (*stream >= '0' && *stream <= '9') ||
		*stream == '!' || *stream == '#' || *stream == '$' || *stream == '&' || *stream == '.' || *stream == '+' || *stream == '-' ||
		*stream == '^' || *stream == '_')) {
		stream++;
		remaining--;
		characters++;
	}

	// Make sure we got something back. Use a default value of text.
	if (!characters) {
		st_free(line);
		return st_import("plain", 5);
	}

	result = st_import(stream - characters, characters);
	st_free(line);

	return result;
}

/**
 * @brief	Get the content encoding type from a mime header.
 * @note	This function parses the Content-Transfer-Encoding field of the header, returning "7bit" by default.
 * @param	header	 a placer pointing to the mime header to be parsed.
 * @return	a managed string containing the content encoding type value of the header, with "7bit" as default.
 */
stringer_t * mail_mime_content_encoding(placer_t header) {

	chr_t *stream;
	stringer_t *holder, *result = NULL;
	size_t remaining, characters = 0;

	if (!(holder = mail_header_fetch_cleaned(&header, PLACER("Content-Transfer-Encoding", 25)))) {
		return st_import("7bit", 4);
	}

	remaining = st_length_get(holder);
	stream = st_char_get(holder);

	// Advance past any garbage.
	while (remaining != 0 && (*stream == '\n' || *stream == '\r' || *stream == ' ' || *stream == '\t')) {
		stream++;
		remaining--;
	}

	// Count how many characters are in the group. Based on RFC2045, section 6.1.
	while (remaining != 0 && ((*stream >= 'a' && *stream <= 'z') || (*stream >= 'A' && *stream <= 'Z') || (*stream >= '0' && *stream <= '9') ||
		*stream == '-')) {
		stream++;
		remaining--;
		characters++;
	}

	// Make sure we got something back. Use a default value of 7bit.
	if (characters != 0) {
		result = st_import(stream - characters, characters);
	}
	else {
		result = st_import("7bit", 4);
	}

	st_free(holder);

	return result;
}

/**
 * @brief	Get the content id from a mime header.
 * @note	This function parses the Content-Id field of the header, returning "7bit" by default.
 * @param	header	 a placer pointing to the mime header to be parsed.
 * @return	NULL on failure, or a managed string containing the content id value of the header.
 */
stringer_t * mail_mime_content_id(placer_t header) {

	chr_t *stream;
	stringer_t *holder, *result = NULL;
	size_t remaining, characters = 0;

	if (!(holder = mail_header_fetch_cleaned(&header, PLACER("Content-Id", 10)))) {
		return NULL;
	}

	remaining = st_length_get(holder);
	stream = st_char_get(holder);

	// Advance past any garbage.
	while (remaining != 0 && (*stream == '\n' || *stream == '\r' || *stream == ' ' || *stream == '\t')) {
		stream++;
		remaining--;
	}

	// Count how many characters are in the group. Based on RFC2822, section 3.6.4.
	while (remaining != 0 && ((*stream >= 'a' && *stream <= 'z') || (*stream >= 'A' && *stream <= 'Z') || (*stream >= '0' && *stream <= '9') ||
		*stream == '<' || *stream == '@' || *stream == '!' || *stream == '#' || *stream == '$' || *stream == '%' || *stream == '&' || *stream == '\'' ||
		*stream == '*' || *stream == '+' || *stream == '-' || *stream == '/' || *stream == '=' || *stream == '?' || *stream == '^' || *stream == '_' ||
		*stream == '`' || *stream == '{' || *stream == '|' || *stream == '}' || *stream == '~' || *stream == '[' || *stream == ']' || *stream == '.' || *stream == '>')) {
		stream++;
		remaining--;
		characters++;
	}

	// Make sure we got something back.
	if (characters != 0) {
		result = st_import(stream - characters, characters);
	}

	st_free(holder);

	return result;
}

/**
 * @brief	Get the key name of a mime header line parameter.
 * @param	parameter	a managed string containing the complete parameter (key/value pair) of the mime header line.
 * @return	NULL on failure or a managed string containing the mime header parameter key name on success.
 */
stringer_t * mail_mime_type_parameters_key(stringer_t *parameter) {

	chr_t *stream;
	size_t length, characters = 0;

	if (!parameter) {
		return NULL;
	}

	length = st_length_get(parameter);
	stream = st_char_get(parameter);

	// Skip the garbage.
	while (length != 0 && *stream == ' ') {
		stream++;
		length--;
	}

	// Find the end of the key.
	while (length > 0 && *stream != '=') {
		characters++;
		length--;
		stream++;
	}

	// Make sure we got something back.
	if (!characters) {
		return NULL;
	}

	return st_import(stream - characters, characters);
}

/**
 * @brief	Get the value of a mime header line parameter.
 * @param	parameter	a managed string containing the complete parameter (key/value pair) of the mime header line.
 * @return	NULL on failure or a managed string containing the mime header parameter value on success.
 */
stringer_t * mail_mime_type_parameters_value(stringer_t *parameter) {

	chr_t *stream;
	size_t length, characters = 0;

	if (!parameter) {
		return NULL;
	}

	length = st_length_get(parameter);
	stream = st_char_get(parameter);

	// Skip the garbage.
	while (length && *stream == ' ') {
		stream++;
		length--;
	}

	// Find the end of the key.
	while (length && *stream != '=') {
		length--;
		stream++;
	}

	if (!length || *stream != '=') {
		return NULL;
	}

	// Skip any remaining garbage.
	while (length && (*stream == ' ' || *stream == '=' || *stream == '\"')) {
		stream++;
		length--;
	}

	// Find the length of the value.
	while (length && *stream != '"') {
		characters++;
		length--;
		stream++;
	}

	// Make sure we got something back.
	if (!characters) {
		return NULL;
	}

	// Remove the trailing space. The header cleanup function checks for consecutive spaces.
	if (*stream == ' ') {
		stream--;
		characters--;
	}

	// Make sure we got something back.
	if (!characters) {
		return NULL;
	}

	return st_import(stream - characters, characters);
}

/**
 * @brief	Get an array of the key/value pairs of parameters passed to the value of the mime Content-Type header.
 * @note	The parameters of the Content-Type header value will be examined, and each found parameter will result in the addition of
 * 			TWO managed strings to the returned array: the first containing the parameter key name, and the second with the parameter value.
 * @param	header	a placer pointing to the mime header to be parsed.
 * @return	NULL on failure, or on success, an array of managed strings structured as the key name followed by the value of each parameter
 * 			passed in the Content-Type header.
 */
array_t * mail_mime_type_parameters(placer_t header) {

	array_t *output;
	stringer_t *key, *holder;
	placer_t parameter;
	unsigned increment, tokens;

	if (!(holder = mail_header_fetch_cleaned(&header, PLACER("Content-Type", 12)))) {
		return NULL;
	}

	if ((tokens = tok_get_count_st(holder, ';')) <= 1) {
		st_free(holder);
		return NULL;
	}

	// Allocate an array.
	if (!(output = ar_alloc((tokens - 1) * 2))) {
		st_free(holder);
		return NULL;
	}

	for (increment = 1; increment < tokens; increment++) {
		tok_get_st(holder, ';', increment, &parameter);

		if ((key = mail_mime_type_parameters_key(&parameter))) {
			upper_st(key);
			ar_append(&output, ARRAY_TYPE_STRINGER, key);
			ar_append(&output, ARRAY_TYPE_STRINGER, mail_mime_type_parameters_value(&parameter));
		}

	}

	st_free(holder);

	if (!ar_length_get(output)) {
		ar_free(output);
		return NULL;
	}

	return output;
}

/**
 * @brief	Get a placer pointing to a mime header in a mime part.
 * @param	part	a managed string containing the mime part text to be parsed.
 * @return	a placer pointing to the mime header at the start of the specified mime part.
 */
placer_t mail_mime_header(stringer_t *part) {

	chr_t *stream;
	size_t length;
	int_t header = 0;

	length = st_length_get(part);
	stream = st_data_get(part);

	for (size_t i = 0; i < length && header != 3; i++) {

		if (header == 0 && *stream == '\n') {
			header++;
		}
		else if (header == 1 && *stream == '\n') {
			header += 2;
		}
		else if (header == 1 && *stream == '\r') {
			header++;
		}
		else if (header == 2 && *stream == '\n') {
			header++;
		}
		else if (header != 0) {
			header = 0;
		}

		stream++;
	}

	return pl_init(st_data_get(part), stream - st_char_get(part));
}

/**
 * @brief	Get the content type from a MIME header.
 * @note	If no content type is specified in the header via Content-Type, "text/plain" is assumed.
 * @param	header	a placer containing the MIME header to be examined.
 * @return	the MIME content type specified by the header, or MESSAGE_TYPE_UNKNOWN on failure.
 */
int_t mail_mime_type(placer_t header) {

	chr_t *stream;
	int_t result;
	size_t remaining;
	stringer_t *holder;

	if ((holder = mail_header_fetch_cleaned(&header, PLACER("Content-Type", 12))) == NULL) {
		return MESSAGE_TYPE_PLAIN;
	}

	remaining = st_length_get(holder);
	stream = st_char_get(holder);

	// Advance past any garbage.
	while (remaining != 0 && (*stream == '\n' || *stream == '\r' || *stream == ' ' || *stream == '\t' || *stream == '"')) {
		stream++;
		remaining--;
	}

	if (remaining >= 21 && mm_cmp_ci_eq(stream, "multipart/alternative", 21) == 0) {
		result = MESSAGE_TYPE_MULTI_ALTERNATIVE;
	}
	else if (remaining >= 17 && mm_cmp_ci_eq(stream, "multipart/related", 17) == 0) {
		result = MESSAGE_TYPE_MULTI_RELATED;
	}
	else if (remaining >= 15 && mm_cmp_ci_eq(stream, "multipart/rfc822", 16) == 0) {
		result = MESSAGE_TYPE_MULTI_RFC822;
	}
	else if (remaining >= 15 && mm_cmp_ci_eq(stream, "multipart/mixed", 15) == 0) {
		result = MESSAGE_TYPE_MULTI_MIXED;
	}
	else if (remaining >= 9 && mm_cmp_ci_eq(stream, "text/html", 9) == 0) {
		result = MESSAGE_TYPE_HTML;
	}
	else if (remaining >= 9 && mm_cmp_ci_eq(stream, "text/plain", 9) == 0) {
		result = MESSAGE_TYPE_PLAIN;
	}
	else if (remaining >= 15 && mm_cmp_ci_eq(stream, "multipart", 9) == 0) {
		result = MESSAGE_TYPE_MULTI_UNKOWN;
	}
	else {
		result = MESSAGE_TYPE_UNKNOWN;
	}

	st_free(holder);

	return result;
}

/**
 * @brief	Get the encoding type from a MIME header.
 * @note	If no encoding type is specified in the header via Content-Transfer-Encoding, 7bit encoding is assumed.
 * @param	header	a placer containing the MIME header to be examined.
 * @return	the MIME encoding type specified by the header, or MESSAGE_ENCODING_UNKNOWN on failure.
 */
int_t mail_mime_encoding(placer_t header) {

	chr_t *stream;
	int_t result;
	size_t remaining;
	stringer_t *holder;

	if ((holder = mail_header_fetch_cleaned(&header, PLACER("Content-Transfer-Encoding", 25))) == NULL) {
		return MESSAGE_ENCODING_7BIT;
	}

	remaining = st_length_get(holder);
	stream = st_char_get(holder);

	// Advance past any garbage.
	while (remaining != 0 && (*stream == '\n' || *stream == '\r' || *stream == ' ' || *stream == '\t')) {
		stream++;
		remaining--;
	}

	if (remaining >= 16 && mm_cmp_ci_eq(stream, "quoted-printable", 16) == 0) {
		result = MESSAGE_ENCODING_QUOTED_PRINTABLE;
	}
	else if (remaining >= 6 && mm_cmp_ci_eq(stream, "base64", 6) == 0) {
		result = MESSAGE_ENCODING_BASE64;
	}
	else if (remaining >= 4 && mm_cmp_ci_eq(stream, "8bit", 4) == 0) {
		result = MESSAGE_ENCODING_8BIT;
	}
	else if (remaining >= 4 && mm_cmp_ci_eq(stream, "7bit", 4) == 0) {
		result = MESSAGE_ENCODING_7BIT;
	}
	else {
		result = MESSAGE_ENCODING_UNKNOWN;
	}

	st_free(holder);

	return result;
}

/**
 * @brief	Get the boundary from a MIME header.
 * @note	This function first scans the value of Content-Type for the boundary, and then the rest of the MIME header.
 * @param	header	a placer pointing to the MIME header to be parsed.
 * @return	NULL on failure, or a pointer to a managed string containing the MIME boundary string on success.
 */
stringer_t * mail_mime_boundary(placer_t header) {

	chr_t *stream;
	int_t quote = 0;
	size_t length, bounder;
	stringer_t *holder, *haystack, *boundary, *content;

	// Get the content type line from the header.
	if ((content = mail_header_fetch_all(&header, PLACER("Content-Type", 12)))) {
		haystack = PLACER(st_char_get(content), st_length_get(content));
	}
	// If there is no content line, search the entire header.
	else {
		haystack = &header;
	}

	// Find the boundary.
	if (!st_search_ci(haystack, PLACER("boundary", 8), &bounder)) {
		log_pedantic("We couldn't find the MIME boundary.");
		st_cleanup(content);
		return NULL;
	}

	// Get setup.
	length = st_length_get(haystack) - bounder;
	stream = st_char_get(haystack) + bounder + 8;

	// Skip the garbage.
	while (length != 0 && quote == 0 && (*stream == '"' || *stream == ' ' || *stream == '\r' || *stream == '\n' || *stream == '\t' || *stream == '=')) {

		if (*stream == '"') {
			quote = 1;
		}

		stream++;
		length--;
	}

	// How long is the boundary.
	bounder = 0;

	while (length != 0 && *stream != '"' && *stream != '\r' && *stream != '\n' && *stream != '\t' && *stream != ';' && (quote == 1 || *stream != ' ')) {
		stream++;
		bounder++;
		length--;
	}

	// Is there something to extract.
	if (!length) {
		log_pedantic("We couldn't find the MIME boundary.");
		st_cleanup(content);
		return NULL;
	}

	// Setup a placer with the boundary.
	holder = PLACER(stream - bounder, bounder);

	// Create the boundary search string.
	if (!(boundary = st_merge("ns", "--", holder))) {
		log_error("Failed to allocate space for boundary string.");
		st_cleanup(content);
		return NULL;
	}

	// Release the stringer, if it was used.
	st_cleanup(content);

	return boundary;
}

/**
 * @brief	Count the number of instances of a boundary string inside a MIME body.
 * @note	The search is terminated if "--" is found right after the boundary string.
 * @param	body		a placer containing the body text to be parsed.
 * @param	boundary	a pointer to a managed string containing the boundary string for the MIME content.
 * @return	0 on failure, or the number of times the boundary string was located in the MIME body on success.
 */
uint32_t mail_mime_count(placer_t body, stringer_t *boundary) {

	uint32_t result = 0;
	chr_t *stream, *bounddata;
	size_t increment = 0, length, boundlen;

	if (pl_empty(body) || st_empty(boundary)) {
		return 0;
	}

	// Figure out the lengths.
	if (!(length = st_length_get(&body))) {
		log_pedantic("Cannot count boundary marker in zero-length MIME body..");
		return 0;
	}
	else if (!(boundlen = st_length_get(boundary))) {
		log_pedantic("Cannot count zero-length MIME boundary.");
		return 0;
	}

	// Setup.
	stream = st_char_get(&body);
	bounddata = st_char_get(boundary);

	// Find the start of the first part.
	while (increment + boundlen <= length) {

		if (mm_cmp_cs_eq(stream, bounddata, boundlen) == 0) {
			stream += boundlen + 1;
			increment += boundlen + 1;

			// Two dashes indicate the end of this mime sections.
			if (increment + 1 <= length && mm_cmp_cs_eq(stream, "--", 2) == 0) {
				increment = length + 1;
			}
			else {
				result++;
			}
		}
		else {
			stream++;
			increment++;
		}
	}

	return result;
}

/**
 * @brief	Get a placer pointing to the specified child inside a MIME body.
 * @param	body		a placer containing the body text to be parsed.
 * @param	boundary	a pointer to a managed string containing the boundary string to split the MIME content.
 * @param	child		the zero-based index of the MIME child to be located in the body text.
 * @return	pl_null() on failure, or a placer containing the specified MIME child on success.
 */
placer_t mail_mime_child(placer_t body, stringer_t *boundary, uint32_t child) {

	uint32_t result = 0;
	chr_t *start, *stream, *bounddata;
	size_t increment = 0, length, boundlen;

	if (pl_empty(body) || st_empty(boundary)) {
		return pl_null();
	}

	// Figure out the lengths.
	if (!(length = st_length_get(&body))) {
		log_pedantic("Cannot parse children from zero-length MIME body..");
		return pl_null();
	}
	else if (!(boundlen = st_length_get(boundary))) {
		log_pedantic("Cannot parse children from MIME body with zero-length boundary.");
		return pl_null();;
	}

	// Setup.
	stream = st_char_get(&body);
	bounddata = st_char_get(boundary);

	// Find the start of the first part.
	while (increment + boundlen <= length && result < child) {

		if (mm_cmp_cs_eq(stream, bounddata, boundlen) == 0 && (increment + boundlen == length || *(stream + boundlen) < '!' || *(stream + boundlen) > '~')) {
			stream += boundlen;
			increment += boundlen;

			// Two dashes indicate the end of this mime sections.
			if (increment < length && mm_cmp_cs_eq(stream, "--", 2) == 0) {
				increment = length + 1;
			}
			else {
				result++;
			}

		}
		else {
			stream++;
			increment++;
		}

	}

	// The requested child wasn't found.
	if (increment + boundlen >= length) {
		return pl_null();
	}

	// This will skip a line break after the boundary marker.
	if (length - increment > 0 && *stream == '\r') {
		stream++;
		increment++;
	}

	if (length - increment > 0 && *stream == '\n') {
		stream++;
		increment++;
	}

	// Store the start position.
	start = stream;

	// Find the end.
	while (increment < length) {

		if (increment + boundlen < length && mm_cmp_cs_eq(stream, bounddata, boundlen) == 0) {
			increment = length;
		}
		else {
			stream++;
			increment++;
		}
	}

	// Make sure we advanced.
	if (stream == start) {
		return pl_null();
	}

	return pl_init(start, stream - start);
}

/**
 * @brief	Split a mime body into an array of children by a boundary string.
 * @param	body		a placer containing the body text to be parsed.
 * @param	boundary	a pointer to a managed string containing the boundary string to split the mime content.
 * @return	NULL on failure, or a pointer to an array of mime children on success.
 */
array_t * mail_mime_split(placer_t body, stringer_t *boundary) {

	array_t *result;
	uint32_t parts;
	stringer_t *item;

	// Figure out how many children this body part has.
	if (!(parts = mail_mime_count(body, boundary))) {
		return NULL;
	}

	// Allocate an array to hold all of the children.
	if (!(result = ar_alloc(parts))) {
		log_pedantic("Could not allocate an array of %i elements for the MIME parts.", parts);
		return NULL;
	}

	// Build an array that contains all of the children.
	for (uint32_t i = 1; i <= parts; i++) {

		if ((item = st_alloc_opts(PLACER_T | JOINTED | HEAP | FOREIGNDATA, 0))) {

			// Get the part and clean it up.
			*((placer_t *)item) = pl_set(*((placer_t *)item), mail_mime_child(body, boundary, i));

			/// TODO: This is ugly. Because the array gets a placer pointer we need to free it when done. But that means differentiating between
			/// these placers and what were usually passed which will likely stack allocated placers. We could probably just change it to a stringer
			/// now that its going to st_free(), but that would mean lots of updates all over the place.
			if (st_empty(item) || ar_append(&result, ARRAY_TYPE_STRINGER, item) != 1) {
				st_free(item);
			}

		}
	}

	return result;
}

/**
 * @brief	Free a mail mime object and its underlying data, and recursively free its children parts.
 * @param	mime	a pointer to the mail mime object to be freed.
 * @return	This function returns no value.
 */
void mail_mime_free(mail_mime_t *mime) {

	size_t increment, elements;

	if (!mime) {
		return;
	}

	if (mime->children) {
		elements = ar_length_get(mime->children);

		for (increment = 0; increment < elements; increment++) {
			mail_mime_free((mail_mime_t *)ar_field_ptr(mime->children, increment));
		}

		ar_free(mime->children);
	}

	st_cleanup(mime->boundary);
	mm_free(mime);

	return;
}

/**
 * @brief	Parse a block of data into a mail mime object.
 * @note	By parsing the specified mime part, this function fills in the content type and encoding of the resulting mail mime object.
 * 			If the message is multipart, the boundary string is determined and then used to split the body into children;
 * 			then each child part is passed to mail_mime_part() to be parsed likewise, recursively.
 * @param	part		a managed string containing the mime part data to be parsed.
 * @param	recursion	an incremented recursion level tracker for calling this function, to prevent an overflow from occurring.
 * @return	NULL on failure or a pointer to a newly allocated and updated mail mime object parsed from the part data on success.
 */
mail_mime_t * mail_mime_part(stringer_t *part, uint32_t recursion) {

	array_t *holder;
	size_t elements, increment;
	mail_mime_t *result, *subpart;

	// Recursion limiter.
	if (recursion >= MAIL_MIME_RECURSION_LIMIT) {
		log_pedantic("Recursion limit hit.");
		return NULL;
	}

	if (st_empty(part)) {
		log_pedantic("Passed an empty placer_t.");
		return NULL;
	}

	if (!(result = mm_alloc(sizeof(mail_mime_t)))) {
		log_pedantic("Could not allocate %zu bytes for the MIME structure.", sizeof(mail_mime_t));
		return NULL;
	}

	// Store the entire part, and figure out the length of the header.
	result->entire = pl_init(st_data_get(part), st_length_get(part));
	result->header = mail_mime_header(part);

	// Check to make sure the header doesn't take up the entire part.
	if (st_length_get(&(result->header)) != st_length_get(part)) {
		result->body = pl_init(st_char_get(part) + st_length_get(&(result->header)), st_length_get(part) - st_length_get(&(result->header)));
	}

	// Determine the content type.
	result->type = mail_mime_type(result->header);
	result->encoding = mail_mime_encoding(result->header);

	// If were dealing with a multipart message, get the boundary.
	if ((result->type == MESSAGE_TYPE_MULTI_ALTERNATIVE || result->type == MESSAGE_TYPE_MULTI_MIXED || result->type == MESSAGE_TYPE_MULTI_RELATED ||
		result->type == MESSAGE_TYPE_MULTI_RFC822 || result->type == MESSAGE_TYPE_MULTI_UNKOWN) && (result->boundary = mail_mime_boundary(result->header))) {

		// Get an array of message parts.
		if ((holder = mail_mime_split(result->body, result->boundary)) && (elements = ar_length_get(holder))) {

			if ((result->children = ar_alloc(elements))) {

				for (increment = 0; increment < elements; increment++) {

					if ((subpart = mail_mime_part(ar_field_st(holder, increment), recursion + 1))) {

						if (ar_append(&(result->children), ARRAY_TYPE_POINTER, subpart) != 1) {
							mail_mime_free(subpart);
						}

					}

				}

			}

		}

		if (holder) {
			ar_free(holder);
		}

	}

	return result;
}

/**
 * @brief	Re-parse a mail message's data as a mime part, freeing any existing mime part(s) that may have already existed.
 * @param	message		the mail message object containing the message data to be parsed.
 * @return	This function always returns 1.
 */
// QUESTION: Should this function always return 1??
int_t mail_mime_update(mail_message_t *message) {

	placer_t part;

	if (message->mime) {
		mail_mime_free(message->mime);
	}

	part = pl_init(st_char_get(message->text), st_length_get(message->text));
	message->mime = mail_mime_part(&part, 1);

	return 1;
}

/**
 * @brief	Generate a MIME boundary string that is unique to a collection of content.
 * @param	parts	a pointer to an array of managed strings containing the MIME children data to be separated by the boundary.
 * @return	NULL on failure, or a pointer to a managed string containing the generated boundary on success.
 */
stringer_t * mail_mime_generate_boundary (array_t *parts) {

	stringer_t *result, *cmp;
	chr_t *ptr;
	size_t blen = 16;
	int_t rnd;

	if (!parts) {
		log_pedantic("Cannot generate mime boundary with empty input data.");
		return NULL;
	}

	if (!(result = st_alloc(blen))) {
		log_error("Unable to allocate space for boundary string.");
		return NULL;
	}

	// Keep generating boundary strings until one of them is unique... we don't expect collisions to happen that often.
	while (1) {
		srand(rand_get_uint64());
		ptr = st_char_get (result);

		// Generate blen's worth of random bytes, each being either a random digit or lowercase letter.
		for (size_t i = 0; i < blen; i++) {
			rnd = rand();

			if (rnd % 2) {
				*ptr++ = '0' + (rnd % 10);
			} else {
				*ptr ++ = 'a' + (rnd % 26);
			}

		}

		st_length_set(result, blen);

		// Now make sure it's not found in any of the parts.
		for (size_t i = 0; i < ar_length_get(parts); i++) {

			if (!(cmp = (stringer_t *) ar_field_ptr(parts, i))) {
				log_pedantic("Could not generate mime boundary for null content.");
				st_free(result);
				return NULL;
			}

			if (st_search_ci(cmp, result, NULL)) {
				continue;
			}

		}

		// If we made it this far, the boundary string was not found in any of the supplied content.
		break;
	}

	return result;
}

/**
 * @brief	Encode a MIME part for a provided block of data (file attachment) with the specified filename.
 * @note	This function will look up the media type based on the supplied filename, and use that media type as a determination
 * 			of whether the content is to be encoded as either quoted-printable or base64.
 * @param	data		a pointer to a managed string containing the body of the data to be encoded.
 * @param	filename	a pointer to a managed string containing the filename of the attachment for which the data was provided.
 * @param	boundary	a pointer to a managed string containing the boundary that will be used to separate the individual MIME parts.
 * @return	NULL on failure, or a pointer to a managed string containing the file attachment encoded as a MIME part on success.
 */
stringer_t * mail_mime_encode_part(stringer_t *data, stringer_t *filename, stringer_t *boundary) {

	stringer_t *result, *encoded;
	media_type_t *mtype;
	chr_t *fstart, *extptr = NULL, *ctype;
	size_t flen;

	if (!data) {
		return NULL;
	}

	// First get the extension of the filename so we can look up the media type.
	if (!st_empty_out(filename, (uchr_t **)&fstart, &flen)) {
		extptr = fstart + flen + 1;

		while (extptr >= fstart) {

			if (*extptr == '.') {
				break;
			}

			extptr--;
		}

		if (extptr < fstart) {
			extptr = NULL;
		}

	}

	mtype = mail_mime_get_media_type (extptr);

	if (mtype->bin) {
		encoded = base64_encode(data, NULL);
		ctype = "base64";
	} else {
		encoded = qp_encode(data);
		ctype = "quoted-printable";
	}

	if (!encoded) {
		log_pedantic("Unable to encode MIME part data.");
		return NULL;
	}

	// What we return is: boundary/CRLF, Content-Type/CRLF, Content-Transfer-Encoding/CRLF, Content-Disposition/CRLF, data/CRLF
	if (!(result = st_merge("nsnnnnnnnsns", "--------------", boundary, "\r\n", "Content-Type: ", mtype->name, ";\r\n", "Content-Transfer-Encoding: ", ctype,
			"\r\nContent-Disposition: attachment; filename=\"", filename, "\"\r\n\r\n", encoded))) {
		log_pedantic("Unable to generate MIME part data.");
		return NULL;
	}

	st_free(encoded);

	return result;
}

/**
 * @brief	Get smtp envelope data for an outbound message sent by a webmail client.
 * @param	from		a pointer to a managed string containing the sender's address.
 * @param	tos			a pointer to an inx holder containing a collection of managed strings with the email addresses specified in the To: header.
 * @param	ccs			a pointer to an inx holder containing a collection of managed strings with the email addresses specified in the CC: header.
 * @param	bccs		a pointer to an inx holder containing a collection of managed strings with the email addresses specified in the BCC: header.
 * @param	subject		a pointer to a managed string containing the text of the subject line.
 * @param	boundary	a pointer to a managed string containing a boundary string to be used in multipart messages.
 * @param	attached	if true, the envelope is to be created for a mail with attachments (type "multipart/mixed");
 * 						if false, only a single part will be sent for the main email body.
 * @return	NULL on failure or a pointer to a managed string containing the smtp envelope data that will be supplied to an smtp
 * 			relay server at the beginning of the DATA command.
 */
stringer_t * mail_mime_get_smtp_envelope(stringer_t *from, inx_t *tos, inx_t *ccs, inx_t *bccs, stringer_t *subject, stringer_t *boundary, bool_t attached) {

	stringer_t *result, *firsthead = NULL;
	stringer_t *str_to, *str_cc, *str_bcc;
	struct tm ltime;
	static const chr_t *date_format = "Date: %a, %d %b %Y %H:%M:%S %z";
	chr_t date_buffer[1024];
	time_t utime;

	if (!from || !tos || !ccs | !bccs || !subject) {
		log_pedantic("Could not build smtp envelope with incomplete information.");
		return NULL;
	}

	// Serialize the To, CC, and BCC data into separate strings.
	if (!(str_to = portal_smtp_merge_headers(tos, NULLER("To: "), NULLER("\r\n")))) {
		log_error("Could not construct To: header for smtp envelope.");
		return NULL;
	} else if (!(str_cc = portal_smtp_merge_headers(ccs, NULLER("CC: "), NULLER("\r\n")))) {
		log_error("Could not construct CC: header for smtp envelope.");
		st_free(str_to);
		return NULL;
	} else if (!(str_bcc = portal_smtp_merge_headers(bccs, NULLER("BCC: "), NULLER("\r\n")))) {
		log_error("Could not construct BCC: header for smtp envelope.");
		st_free(str_to);
		st_free(str_cc);
		return NULL;
	}

	// Add the current date/time to the outbound message.
	if (((utime = time(&utime)) == -1) || (localtime_r(&utime, &ltime) == NULL) || (strftime(date_buffer, sizeof(date_buffer), date_format, &ltime) <= 0)) {
		log_error("Could not build smtp envelope without current time.");
		st_free(str_to);
		st_free(str_cc);
		st_free(str_bcc);
		return NULL;
	}

	// If there are attachments, we are going to make a multipart message.
	if (attached) {

		if (!(firsthead = st_merge("nsn", "Content-Type: multipart/mixed; boundary=\"------------", boundary, "\"\r\n\r\nThis is a multi-part message in MIME format.\r\n"))) {
			log_error("Could not build multipart header for smtp envelope.");
			st_free(str_to);
			st_free(str_cc);
			st_free(str_bcc);
			return NULL;
		}

	}

	/*if (!(result = st_merge("nnsnsnsns", date_buffer, "\r\nFrom: ", from, "\r\nUser-Agent: lavaweb 1.0\r\nMIME-Version: 1.0\r\nTo: ",
			to, "\r\nSubject: ", subject, "\r\n", firsthead))) { */
	if (!(result = st_merge("nnsnsssnsns", date_buffer, "\r\nFrom: ", from, "\r\nUser-Agent: lavaweb 1.0\r\nMIME-Version: 1.0\r\n",
				str_to, str_cc, str_bcc, "Subject: ", subject, "\r\n", firsthead))) {
		log_error("Unable to allocate space for smtp envelope.");
		st_free(str_to);
		st_free(str_cc);
		st_free(str_bcc);
		st_cleanup(firsthead);
		return NULL;
	}

	st_free(str_to);
	st_free(str_cc);
	st_free(str_bcc);
	st_cleanup(firsthead);

	return result;
}
