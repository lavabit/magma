
/**
 * @file /magma/objects/mail/signatures.c
 *
 * @brief	Functions used to insert signatures into mail messages.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Build a spam text signature for insertion into a message body.
 * @param	server				the server object of the web server where the teacher application is hosted.
 * @param	type				MESSAGE_TYPE_HTML to generate an html-based signature, or any other value for plain text.
 * @param	content_encoding	if MESSAGE_ENCODING_QUOTED_PRINTABLE, set the encoding type to qp.
 * @param	signum				the spam signature number referenced by the teacher url.
 * @param	sigkey				the spam signature key for client verification in the teacher app.
 * @param	disposition			if 0, the message disposition is "innocent"; otherwise, the disposition specifies "spam".
 * @return	NULL on failure, or a newly allocated managed string containing the desired mail signature on success.
 */
stringer_t * mail_build_signature(server_t *server, int_t content_type, int_t content_encoding, uint64_t signum, uint64_t sigkey, int_t disposition) {

	size_t length;
	stringer_t *holder;
	stringer_t *result = NULL;
	stringer_t *spam_text = NULL;
	static const chr_t *line = "\r\n____________________________________________________________________________________\r\n";

	// Build the spam signature.
	if (signum && sigkey) {
		// This is the format for spam signatures.
		if (content_type == MESSAGE_TYPE_HTML) {
			holder = st_merge("nnnnnsnsn", "\r\n<div style='display: block; text-transform: none; text-indent: 0px; letter-spacing: normal; "
					"line-height: normal; text-align: left; white-space: normal; height: auto; visibility: visible; border-bottom: 1px solid black; border-collapse: collapse; "
					"width: 50em; font-size: 12px; color: black; font-family: sans-serif; padding: 0px 0px 4px 0px; clear: both; font-weight: normal;"
					"text-decoration: none; background: white; ", "margin: 20px 2px 10px 2px; border-top: 1px solid black;",
					"'>\r\n\tUse the link below to report this message as ", !disposition ? "spam" : "innocent",
					".<br />\r\n\t<a style='font-size: 12px; font-family: sans-serif; color: blue; background: white; text-decoration: underline; font-weight: normal;' href='https://",
					server->domain, "/apps/teacher?sig=%lu&amp;key=%lu'>https://", server->domain, "/apps/teacher?sig=%lu&amp;key=%lu</a>\r\n</div>\r\n");
		}
		else {
			holder = st_merge("nnsn", "Use the link below to report this message as ", !disposition ? "spam.\r\nhttps://" : "innocent.\r\nhttps://",
				server->domain, "/apps/teacher?sig=%lu&amp;key=%lu");
		}

		if (!holder) {
			log_pedantic("Unable to build the spam signature.");
			return NULL;
		}

		// How big is the signature with the numbers inserted.
		length = st_length_get(holder) + uint64_digits(signum) + uint64_digits(sigkey) + uint64_digits(signum) + uint64_digits(sigkey);

		if (!(spam_text = st_alloc(length))) {
			log_pedantic("Unable to build the spam signature.");
			st_free(holder);
			return NULL;
		}

		// Print the spam signature text.
		if (content_type == MESSAGE_TYPE_HTML) {
			length = st_sprint(spam_text, st_char_get(holder), signum, sigkey, signum, sigkey);
		}
		else {
			length = st_sprint(spam_text, st_char_get(holder), signum, sigkey);
		}

		st_free(holder);

		if (!length) {
			log_pedantic("Unable to build the spam signature.");
			st_free(spam_text);
			return NULL;
		}

	}

	// Error check.
	if (!spam_text) {
		log_pedantic("Could not build the signature.");
		return NULL;
	}

	// Build the output signature.
	if (content_type == MESSAGE_TYPE_HTML) {
		result = spam_text;
		spam_text = NULL;
	}
	else {
		result = st_merge("nnsn", line, (spam_text == NULL) ? "" : line, spam_text, line);
	}

	if (!result) {
		log_pedantic("An error occurred while attempting to merge the signature.");
	}

	// Cleanup.
	st_cleanup(spam_text);

	// Check for quoted printable encodings.
	if (content_encoding == MESSAGE_ENCODING_QUOTED_PRINTABLE) {
		holder = qp_encode(result);

		if (holder && result) {
			st_free(result);
			result = holder;
		}

	}

	return result;
}

/**
 * @brief	Get the value of the Content-Type header in a mail message header.
 * @note	Possible return values include MESSAGE_TYPE_MULTI_ALTERNATIVE, MESSAGE_TYPE_MULTI_RELATED, MESSAGE_TYPE_MULTI_MIXED,
 * 			MESSAGE_TYPE_HTML, MESSAGE_TYPE_MULTI_UNKOWN, and MESSAGE_TYPE_PLAIN.
 * @param	header	a managed string containing the mail message header to be parsed.
 * @return	the MESSAGE_TYPE code of the mail content type, or MESSAGE_TYPE_PLAIN by default.
 */
int_t mail_discover_type(stringer_t *header) {

	chr_t *stream;
	size_t remaining;
	stringer_t *content;
	int_t result = MESSAGE_TYPE_PLAIN;

	// Get the content encoding line from the header.
	if (!(content = mail_header_fetch_all(header, PLACER("Content-Type", 12)))) {
		return MESSAGE_TYPE_PLAIN;
	}

	remaining = st_length_get(content) - 13;
	stream = st_char_get(content) + 13;

	// Advance past any garbage.
	while (remaining && (*stream == '\n' || *stream == '\r' || *stream == ' ' || *stream == '\t')) {
		stream++;
		remaining--;
	}

	if (remaining >= 21 && !mm_cmp_ci_eq(stream, "multipart/alternative", 21)) {
		result = MESSAGE_TYPE_MULTI_ALTERNATIVE;
	}
	else if (remaining >= 17 && !mm_cmp_ci_eq(stream, "multipart/related", 17)) {
		result = MESSAGE_TYPE_MULTI_RELATED;
	}
	else if (remaining >= 15 && !mm_cmp_ci_eq(stream, "multipart/mixed", 15)) {
		result = MESSAGE_TYPE_MULTI_MIXED;
	}
	else if (remaining >= 9 && !mm_cmp_ci_eq(stream, "text/html", 9)) {
		result = MESSAGE_TYPE_HTML;
	}
	else if (remaining >= 9 && !mm_cmp_ci_eq(stream, "text/plain", 9)) {
		result = MESSAGE_TYPE_PLAIN;
	}
	else if (remaining >= 15 && !mm_cmp_ci_eq(stream, "multipart", 9)) {
		result = MESSAGE_TYPE_MULTI_UNKOWN;
	}

	st_free(content);

	return result;
}

/**
 * @brief	Get the value of the Content-Transfer-Encoding header in a mail message header.
 * @note	Possible return values include 	MESSAGE_ENCODING_QUOTED_PRINTABL, MESSAGE_ENCODING_BASE64, MESSAGE_ENCODING_8BIT, and MESSAGE_ENCODING_7BIT.
 * @param	header	a managed string containing the mail message header to be parsed.
 * @return	the MESSAGE_ENCODING code of the mail transfer encoding type, or MESSAGE_ENCODING_7BIT by default.
 */
int_t mail_discover_encoding(stringer_t *header) {

	chr_t *stream;
	size_t remaining;
	stringer_t *content;
	int_t result = MESSAGE_ENCODING_UNKNOWN;

	// Get the content encoding line from the header.
	if (!(content = mail_header_fetch_all(header, PLACER("Content-Transfer-Encoding", 25)))) {
		return MESSAGE_ENCODING_7BIT;
	}

	remaining = st_length_get(content) - 26;
	stream = st_char_get(content) + 26;

	// Advance past any garbage.
	while (remaining && (*stream == '\n' || *stream == '\r' || *stream == ' ' || *stream == '\t')) {
		stream++;
		remaining--;
	}

	if (remaining >= 16 && !mm_cmp_ci_eq(stream, "quoted-printable", 16)) {
		result = MESSAGE_ENCODING_QUOTED_PRINTABLE;
	}
	else if (remaining >= 6 && !mm_cmp_ci_eq(stream, "base64", 6)) {
		result = MESSAGE_ENCODING_BASE64;
	}
	else if (remaining >= 4 && !mm_cmp_ci_eq(stream, "8bit", 4)) {
		result = MESSAGE_ENCODING_8BIT;
	}
	else if (remaining >= 4 && !mm_cmp_ci_eq(stream, "7bit", 4)) {
		result = MESSAGE_ENCODING_7BIT;
	}

	st_free(content);

	return result;
}

/**
 * @brief	Extract an html tag from a data buffer, searching backwards.
 * @warning	Remember that this function takes the end and not the start of a buffer!
 * @note	The extracted tag contains the enclosing brackets and only printable characters (no whitespace).
 * @param	stream	the end of a data buffer containing the html data to be parsed.
 * @param	length	the size, in bytes, of the data buffer to be parsed.
 * @return	NULL on failure, or a managed string containing the printable contents of the nearest html tag on success.
 */
stringer_t * mail_extract_tag(chr_t *stream, size_t length) {

	chr_t *holder;
	size_t amount = 0;
	stringer_t *tag;

	if (*stream != '>') {
		return NULL;
	}

	// How long is this tag.
	while (length && *stream != '<') {
		length--;
		stream--;
		amount++;
	}

	// Did we hit the end of the stream or did we hit the beginning of the tag.
	if (!length) {
		return NULL;
	}
	else {
		amount++;
	}

	// Allocate a buffer.
	if (!(tag = st_alloc(amount))) {
		log_pedantic("Unable to allocate a stringer of %zu bytes to hold the HTML tag.", amount);
		return NULL;
	}

	// Get setup.
	length = 0;
	holder = st_char_get(tag);

	// QUESTION: Without folding whitespace?? Without any whitespace!
	// This returns the tag without folding whitespace.
	while (amount) {

		if (*stream <= '~' && *stream >= '!') {
			*holder = *stream;
			stream++;
			holder++;
			length++;
		}
		else {
			stream++;
		}

		amount--;
	}

	// This should never happen.
	if (!length) {
		st_free(tag);
		return NULL;
	}

	st_length_set(tag, length);

	return tag;
}

/**
 * @brief	Find the position in a mail message where a custom message can be inserted.
 * @note	The part parameter is expected to be a placer pointing into the contents of message.
 * 			If the encoding type is not html, the insertion point is determined to be at the end of the part.
 * 			If the encoding type is html, the following rules are followed:
 * 			1. Scanning backwards from the end of the message, skip trailing whitespace get the next html tag.
 * 			2. If that tag is NOT </html> or </body>, insert the siignature AFTER it.
 *			3. If that tag IS </html> or </body>, insert the signature right before it closes.
 * @param	message		a managed string containing the mail message body to be parsed.
 * @param	part		a managed string (placer) containing the part of the message where the custom message should be inserted.
 * @param	type		the encoding type of the message (MESSAGE_TYPE_HTML or other).
 * @return	the zero-based index of the position in the specified message where the custom message can be inserted.
 */
// QUESTION: Why are we using managed strings here? It makes no sense, especially since we have expectations as to where they point.
// QUESTION: And why even have message? It seems like we could make do with just having part.
size_t mail_discover_insertion_point(stringer_t *message, stringer_t *part, int_t type) {

	chr_t *stream;
	size_t length;
	stringer_t *tag;

	// If the message is not HTML, return the end of the part as the insertion point.
	if (type != MESSAGE_TYPE_HTML) {
		return st_char_get(part) - st_char_get(message) + st_length_get(part);
	}

	// Get setup.
	length = st_length_get(part);
	stream = st_data_get(part) + length - 1;

	while (length) {

		// Reverse until we find a character that is not whitespace.
		while (length && (*stream == ' ' || *stream == '\r' || *stream == '\n' || *stream == '\t')) {
			length--;
			stream--;
		}

		if (!(tag = mail_extract_tag(stream, length))) {
			return (st_char_get(part) - st_char_get(message)) + length;
		}

		// What tag is it?
		if (st_cmp_ci_eq(tag, PLACER("</body>", 7)) != 0 && st_cmp_ci_eq(tag, PLACER("</html>", 7)) != 0) {
			st_free(tag);
			return (st_char_get(part) - st_char_get(message)) + length;
		}

		st_free(tag);

		// Advance past this tag.
		while (length && *stream != '<') {
			length--;
			stream--;
		}

		if (length && *stream == '<') {
			length--;
			stream--;
		}

	}

	return (st_char_get(part) - st_char_get(message)) + length;
}

/**
 * @brief	Insert a spam signature training link into a base64-encoded message part.
 * @see		mail_discover_insertion_point()
 * @note	This is similar to mail_insert_chunk_text() except the part has to be decoded and then re-encoded after the training link is inserted.
 * @param	server		the server object of the web server where the teacher application is hosted.
 * @param	message		a managed string containing the base64-encoded message body to be parsed.
 * @param	part		a managed string (placer) containing the part of the message where the signature should be inserted.
 * @param	signum		the spam signature number referenced by the teacher url.
 * @param	sigkey		the spam signature key for client verification in the teacher app.
 * @param	disposition	if 0, the message disposition is "innocent"; otherwise, the disposition specifies "spam".
 * @param	type		the encoding type of the message (MESSAGE_TYPE_HTML or other).
 * @param	encoding	if MESSAGE_ENCODING_QUOTED_PRINTABLE, set the encoding type to qp.
 * @return	NULL on failure or a managed string containing the base64-encoded message with the inserted signature training link on success.
 */
stringer_t * mail_insert_chunk_base64(server_t *server, stringer_t *message, stringer_t *part, uint64_t signum, uint64_t sigkey, int_t disposition, int_t type, int_t encoding) {

	chr_t *end;
	chr_t *start;
	chr_t *stream;
	size_t length;
	stringer_t *posttext;
	stringer_t *portion;
	int_t headpart = 0;
	stringer_t *result;
	stringer_t *decoded;
	stringer_t *encoded;
	stringer_t *signature;

	// If this is the first time through, and were looking at the message header, start the headpart count at 1.
	if (st_char_get(message) == st_data_get(part)) {
		headpart = 1;
	}

	// Discover the length of the header.
	length = st_length_get(part);
	stream = st_data_get(part);

	while (length != 0 && headpart != 3) {

		// Logic for detecting the end of the header.
		if (headpart == 0 && *stream == '\n') {
			headpart++;
		}
		else if (headpart == 1 && *stream == '\n') {
			headpart += 2;
		}
		else if (headpart == 1 && *stream == '\r') {
			headpart++;
		}
		else if (headpart == 2 && *stream == '\n') {
			headpart++;
		}
		else if (headpart != 0) {
			headpart = 0;
		}
		stream++;
		length--;
	}

	// Now skip any remaining garbage.
	while (length != 0 && (*stream == '\n' || *stream == '\r' || *stream == ' ' || *stream == '\t')) {
		stream++;
		length--;
	}

	// This is the insertion starting point.
	start = stream;

	// Figure out how long the base64 amount is.
	while (length != 0 && ((*stream >= 'A' && *stream <= 'Z') || (*stream >= 'a' && *stream <= 'z') || (*stream >= '0' && *stream <= '9') || *stream == '+' || *stream == '/' || *stream == '\n' || *stream == '\r' || *stream == '\t' || *stream == ' ')) {
		stream++;
		length--;
	}

	// Record the ending point.
	end = stream;

	// If its an empty base64 chunk, just encode a signature and insert.
	if (start == end) {

		if (!(signature = mail_build_signature(server, type, encoding, signum, sigkey, disposition))) {
			return NULL;
		}

		encoded = base64_encode(signature,NULL);
		st_free(signature);

		portion = PLACER(st_char_get(message), start - (chr_t *)st_char_get(message));
		posttext = PLACER(end, st_length_get(message) - (end - (chr_t *)st_char_get(message)));
		result = st_merge("sss", portion, encoded, posttext);
		st_cleanup(encoded);
		return result;
	}

	// Decode.
	if (!(decoded = base64_decode(PLACER(start, end - start),NULL))) {
		log_pedantic("Nothing was returned from the base64 decoder.");
		return NULL;
	}

	// Find the insertion point.
	portion = PLACER(st_char_get(decoded), st_length_get(decoded));
	length = mail_discover_insertion_point(decoded, portion, type);

	// Get our place holders setup.
	portion = PLACER(st_char_get(decoded), length);
	posttext = PLACER(st_char_get(decoded) + length, st_length_get(decoded) - length);

	// Get the signature.
	if (!(signature = mail_build_signature(server, type, encoding, signum, sigkey, disposition))) {
		st_free(decoded);
		return NULL;
	}

	// Merge and clean.
	result = st_merge("sss", portion, signature, posttext);
	st_free(decoded);
	st_free(signature);

	if (!result) {
		log_pedantic("Unable to merge the strings together.");
		return NULL;
	}

	// Make it base64 again.
	encoded = base64_encode(result,NULL);
	st_free(result);

	// Now rejoin the message again.
	portion = PLACER(st_char_get(message), start - (chr_t *)st_char_get(message));
	posttext = PLACER(end, st_length_get(message) - (end - (chr_t *)st_char_get(message)));
	result = st_merge("sss", portion, encoded, posttext);

	st_free(encoded);

	if (!result) {
		log_pedantic("Unable to merge the strings together.");
	}

	return result;
}

/**
 * @brief	Insert a spam signature training link into a plain text message part.
 * @see		mail_discover_insertion_point()
 * @param	server		the server object of the web server where the teacher application is hosted.
 * @param	message		a managed string containing the message body to be parsed.
 * @param	part		a managed string (placer) containing the part of the message where the signature should be inserted.
 * @param	signum		the spam signature number referenced by the teacher url.
 * @param	sigkey		the spam signature key for client verification in the teacher app.
 * @param	disposition	if 0, the message disposition is "innocent"; otherwise, the disposition specifies "spam".
 * @param	type		the encoding type of the message (MESSAGE_TYPE_HTML or other).
 * @param	encoding	if MESSAGE_ENCODING_QUOTED_PRINTABLE, set the encoding type to qp.
 * @return	NULL on failure or a managed string containing the message with the inserted signature training link on success.
 */
stringer_t * mail_insert_chunk_text(server_t *server, stringer_t *message, stringer_t *part, uint64_t signum, uint64_t sigkey, int_t disposition, int_t type, int_t encoding) {

	size_t point;
	stringer_t *pretext;
	stringer_t *posttext;
	stringer_t *result;
	stringer_t *signature;

	if (st_empty(part)) {
		return NULL;
	}

	// Start at the end of the chunk and search for where to insert the signature.
	if (!(signature = mail_build_signature(server, type, encoding, signum, sigkey, disposition))) {
		return NULL;
	}

	// Figure out the insertion point_t and create the new message.
	point = mail_discover_insertion_point(message, part, type);
	pretext = PLACER(st_char_get(message), point);
	posttext = PLACER(st_char_get(message) + point, st_length_get(message) - point);
	result = st_merge("sss", pretext, signature, posttext);

	st_free(signature);

	if (!result) {
		log_pedantic("Unable to merge the strings together.");
	}

	return result;
}

/**
 * @brief	Get a specified chunk (mime part) of a multipart mime message.
 * @param	message		a managed string containing the mime message to be parsed.
 * @param	boundary	a managed string containing the boundary used to split the multipart mime message.
 * @param	chunk		the one-index based chunk to be retrieved from the multipart message
 * @return	NULL on failure or a placer containing the specified chunk on success.
 */
bool_t mail_get_chunk(placer_t *result, stringer_t *message, stringer_t *boundary, int_t chunk) {

	int_t found = 0;
	size_t start = 0, length = 0, input = 0;

	while (chunk != 0) {

		// So on repeats we don't have to start all over again.
		if (length != 0) {
			start += length - 1;
		}

		found = 0;

		while (found == 0) {

			// Get the start of the MIME message part.
			if (!st_search_cs(PLACER(st_char_get(message) + start, st_length_get(message) - start), boundary, &input)) {
				log_pedantic("The boundary doesn't appear to be part of this message.");
				return false;
			}

			// Skip the boundary before searching again.
			start += input + st_length_get(boundary);

			// This will detect the section ending.
			if (st_length_get(message) - start >= 2 && mm_cmp_cs_eq(st_char_get(message) + start, "--", 2) == 1) {
				return false;
			}
			// Some broken implementations use similar boundaries. This should detect those.
			else if (st_length_get(message) - start > 0 && (*(st_char_get(message) + start) < '!' || *(st_char_get(message) + start) > '~')) {
				found = 1;
			}
		}

		found = 0;

		while (found == 0) {

			// Get the end.
			if (!st_search_cs(PLACER(st_char_get(message) + start, st_length_get(message) - start), boundary, &length)) {
				length = st_length_get(message) - start;
				found = 1;
			}
			else if (st_length_get(message) - start - length > 0 && (*(st_char_get(message) + start) < '!' || *(st_char_get(message) + start) > '~')) {
				found = 1;
			}

		}

		chunk--;
	}

	// Setup a placer with the chunk.
	pl_replace(result, st_char_get(message) + start, length);

	return true;
}

/**
 * @brief	Get the boundary string from a message header.
 * @note	This function works by parsing the Content-Type header if it exists, falling back to the entire header otherwise.
 * 			The returned boundary string includes a trailing "--".
 * @param	header	a managed string containing the message header.
 * @return	NULL on failure or a managed strirng containing the boundary string on success.
 */
stringer_t * mail_get_boundary(stringer_t *header) {

	chr_t *stream;
	int_t quote = 0;
	size_t length, bounder;
	stringer_t *boundary, *content, *holder, *haystack;

	// Get the content type line from the header.
	if ((content = mail_header_fetch_all(header, PLACER("Content-Type", 12)))) {
		haystack = PLACER(st_char_get(content), st_length_get(content));
	}
	// If there is no content line, search the entire header.
	else {
		haystack = header;
	}

	// Find the boundary.
	if (!st_search_ci(PLACER(st_data_get(haystack), st_length_get(haystack)), PLACER("boundary", 8), &bounder)) {
		log_pedantic("We couldn't find the MIME boundary.");
		st_cleanup(content);
		return NULL;
	}

	// Get setup.
	length = st_length_get(haystack) - bounder;
	stream = st_data_get(haystack) + bounder + 8;

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
		log_pedantic("We couldn't copy the boundary into a buffer.");
		st_cleanup(content);
		return NULL;
	}

	// Release the stringer, if it was used.
	st_cleanup(content);

	return boundary;
}

/**
 * @brief	Insert a spam signature training link into a specified part of a mime message.
 * @warning	This function may fail to work as expected and still return a value of 1.
 * @note	If the mime type is MESSAGE_TYPE_UNKNOWN (binary file), the function returns immediately.
 * 			If the type is MESSAGE_TYPE_HTML or MESSAGE_TYPE_PLAIN, the training link is inserted normally.
 * 			For MESSAGE_TYPE_MULTI_RELATED, MESSAGE_TYPE_MULTI_MIXED, MESSAGE_TYPE_MULTI_UNKOWN the link is inserted in the first mime part.
 * 			For MESSAGE_TYPE_MULTI_ALTERNATIVE, the link is inserted into each part, for up to 8 times.
 * @param	server			the server object of the web server where the teacher application is hosted.
 * @param	message			the mail message object of the message to be modified.
 * @param	part			a placer pointing to the specified part of the message to be modified.
 * @param	signum			the spam signature number referenced by the teacher url.
 * @param	sigkey			the spam signature key for client verification in the teacher app.
 * @param	disposition		if 0, the message disposition is "innocent"; otherwise, the disposition specifies "spam".
 * @param	recursion		an incremented recursion level tracker for calling this function, to prevent an overflow from occurring.
 * @return	0 on recursion failure, or 1 otherwise.
 */
int_t mail_modify_part(server_t *server, mail_message_t *message, stringer_t *part, uint64_t signum, uint64_t sigkey, int_t disposition, int_t recursion) {

	int_t type;
	int_t encoding;
	chr_t *stream;
	size_t length;
	stringer_t *header;
	placer_t chunk;
	bool_t chunk_success = true;
	int_t headpart = 0;
	size_t increment;
	stringer_t *boundary;
	stringer_t *replacement;

	if (recursion > MAIL_SIGNATURES_RECURSION_LIMIT) {
		log_pedantic("Recursion limit hit.");
		return 0;
	}

	// If this is the first time through, and were looking at the message header, start the headpart count at 1.
	if (st_char_get(message->text) == st_data_get(part)) {
		headpart = 1;
	}

	// Discover the length of the header.
	length = st_length_get(part);
	stream = st_data_get(part);

	// QUESTION: Can mail_header_end() be used?
	for (increment = 0; increment < length && headpart != 3; increment++) {

		// Logic for detecting the end of the header.
		if (headpart == 0 && *stream == '\n') {
			headpart++;
		}
		else if (headpart == 1 && *stream == '\n') {
			headpart += 2;
		}
		else if (headpart == 1 && *stream == '\r') {
			headpart++;
		}
		else if (headpart == 2 && *stream == '\n') {
			headpart++;
		}
		else if (headpart != 0) {
			headpart = 0;
		}

		stream++;
	}

	// Setup a placer for the entire header.
	header = PLACER(st_data_get(part), increment);

	// Figure out the content type.
	type = mail_discover_type(header);

	// Figure out the content encoding.
	encoding = mail_discover_encoding(header);

	// Odds are this is a binary attachment so stay away.
	if (type == MESSAGE_TYPE_UNKNOWN) {
		return 1;
	}

	// If this is a plain text or HTML message, just insert the chunk.
	if (type == MESSAGE_TYPE_HTML || type == MESSAGE_TYPE_PLAIN) {
		// Use a special function for base64 encoded messages.
		if (encoding != MESSAGE_ENCODING_BASE64) {
			replacement = mail_insert_chunk_text(server, message->text, part, signum, sigkey, disposition, type, encoding);
		}
		else {
			replacement = mail_insert_chunk_base64(server, message->text, part, signum, sigkey, disposition, type, encoding);
		}

		if (replacement) {
			st_free(message->text);
			message->text = replacement;
		}
		//else {
		//	log_error("We found a message we cound't insert a signature into.\n%.*s", st_length_get(message->text), st_char_get(message->text));
		//	log_error("part = %*.s", pl_length_get(part), st_data_get(part));
		//}
		return 1;
	}

	// If it's a multipart message.
	if (!(boundary = mail_get_boundary(header))) {
		log_pedantic("Found a multipart related message without a proper boundary.\n");
		return 1;
	}

	// If its a related message, we just insert into the first chunk.
	if (type == MESSAGE_TYPE_MULTI_RELATED || type == MESSAGE_TYPE_MULTI_MIXED || type == MESSAGE_TYPE_MULTI_UNKOWN) {
		mail_get_chunk(&chunk, message->text, boundary, 1);
		mail_modify_part(server, message, &chunk, signum, sigkey, disposition, recursion + 1);
	}
	// If its an alternative message we have to insert into each chunk.
	else if (type == MESSAGE_TYPE_MULTI_ALTERNATIVE) {
		length = 1;
		mail_get_chunk(&chunk, message->text, boundary, length);

		while (chunk_success && length < 8) {
			length++;
			mail_modify_part(server, message, &chunk, signum, sigkey, disposition, recursion + 1);
			chunk_success = mail_get_chunk(&chunk, message->text, boundary, length);
		}

	}

	// Cleanup.
	st_free(boundary);

	return 1;
}

/**
 * @brief	Insert a spam signature training link into a mail message.
 * @see		mail_modify_part()
 * @param	message			the mail message object of the message to be modified.
 * @param	server			the server object of the web server where the teacher application is hosted.
 * @param	signum			the spam signature number referenced by the teacher url.
 * @param	sigkey			the spam signature key for client verification in the teacher app.
 * @param	disposition		if 0, the message disposition is "innocent"; otherwise, the disposition specifies "spam".
 * @return	This function returns no value.
 */
void mail_signature_add(mail_message_t *message, server_t *server, uint64_t signum, uint64_t sigkey, int_t disposition) {

	stringer_t *part;

	part = PLACER(st_char_get(message->text), st_length_get(message->text));

	if ((mail_modify_part(server, message, part, signum, sigkey, disposition, 0)) == 0) {
		log_pedantic("------ MESSAGE ---------\n%.*s------------------", st_length_int(message->text), st_char_get(message->text));
	}

	return;
}
