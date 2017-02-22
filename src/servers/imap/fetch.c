
/**
 * @file /magma/servers/imap/fetch.c
 *
 * @brief	Functions used to handle IMAP commands/actions.
 */

#include "magma.h"

// Checks for a valid sequence range.
int_t imap_valid_sequence(stringer_t *range) {

	size_t length;
	chr_t *stream;

	if (st_empty_out(range, (uchr_t **)&stream, &length)) {
		return 0;
	}

	while (length > 0 && *stream && ((*stream >= '0' && *stream <= '9') || *stream == ':' || *stream == ',' || *stream == '*')) {
		stream++;
		length--;
	}

	if (length) {
		return 0;
	}

	return 1;
}

void imap_fetch_free_items(imap_fetch_dataitems_t *items) {

	mm_cleanup(items->normal);
	mm_cleanup(items->normal_partial);
	mm_cleanup(items->peek);
	mm_cleanup(items->peek_partial);
	mm_free(items);

	return;
}

// This function is used with the fetch command to find out what dataitems need to be output.
imap_fetch_dataitems_t * imap_parse_dataitems(imap_arguments_t *arguments) {

	int_t type;
	stringer_t *item = NULL;
	imap_arguments_t *array = NULL;
	imap_fetch_dataitems_t *output;
	size_t number, increment;

	if (!arguments) {
		log_error("Sanity check failed, passed a NULL parameter.");
		return NULL;
	}

	if (!(output = mm_alloc(sizeof(imap_fetch_dataitems_t)))) {
		log_error("Unable to allocate %zu bytes for the data items structure. Returning NULL.", sizeof(imap_fetch_dataitems_t));
		return NULL;
	}

	// If its a array, find the length.
	if ((type = imap_get_type_ar(arguments, 1)) == IMAP_ARGUMENT_TYPE_ARRAY) {
		array = imap_get_ar_ar(arguments, 1);
		number = ar_length_get(array);
		increment = 0;
	}
	else {
		array = arguments;
		number = ar_length_get(array);
		increment = 1;
	}

	// Loop through and process each one.
	while (increment < number) {

		// Were parsing an array, so get the item pointer each time.
		item = imap_get_st_ar(array, increment);

		// Advance to the argument.
		increment++;

		// If for some reason the pointer is NULL, free the structure and return.
		if (!item) {
			imap_fetch_free_items(output);
			return NULL;
		}

		// Figure out what flags to flip.
		if (!st_cmp_ci_eq(item, PLACER("BODYSTRUCTURE", 13))) {
			output->bodystructure = 1;
		}
		else if (!st_cmp_ci_eq(item, PLACER("UID", 3))) {
			output->uid = 1;
		}
		else if (!st_cmp_ci_eq(item, PLACER("FLAGS", 5))) {
			output->flags = 1;
		}
		else if (!st_cmp_ci_eq(item, PLACER("INTERNALDATE", 12))) {
			output->internaldate = 1;
		}
		else if (!st_cmp_ci_eq(item, PLACER("ENVELOPE", 8))) {
			output->envelope = 1;
		}
		else if (!st_cmp_ci_eq(item, PLACER("RFC822.HEADER", 13))) {
			output->rfc822_header = 1;
		}
		else if (!st_cmp_ci_eq(item, PLACER("RFC822.SIZE", 11))) {
			output->rfc822_size = 1;
		}
		else if (!st_cmp_ci_eq(item, PLACER("RFC822.TEXT", 11))) {
			output->rfc822_text = 1;
		}
		else if (!st_cmp_ci_eq(item, PLACER("RFC822", 6))) {
			output->rfc822 = 1;
		}
		else if (!st_cmp_ci_eq(item, PLACER("ALL", 3))) {
			output->flags = 1;
			output->internaldate = 1;
			output->rfc822_size = 1;
			output->envelope = 1;
		}
		else if (!st_cmp_ci_eq(item, PLACER("FAST", 4))) {
			output->flags = 1;
			output->internaldate = 1;
			output->rfc822_size = 1;
		}
		else if (!st_cmp_ci_eq(item, PLACER("FULL", 4))) {
			output->flags = 1;
			output->internaldate = 1;
			output->rfc822_size = 1;
			output->envelope = 1;
			output->body = 1;
		}
		// This item requires an array as an arguemnt.
		else if (!st_cmp_ci_eq(item, PLACER("BODY.PEEK", 9)) && increment < number && imap_get_type_ar(array, increment)
			== IMAP_ARGUMENT_TYPE_ARRAY) {

			// Store the array.
			ar_append(&(output->peek), ARRAY_TYPE_ARRAY, imap_get_ar_ar(array, increment++));

			// If the next item is a stringer, and appears to be a partial indicator, add it to the array, otherwise NULL.
			if (increment < number && imap_get_type_ar(array, increment) != IMAP_ARGUMENT_TYPE_ARRAY && (item = imap_get_st_ar(array, increment))
				!= NULL && !st_cmp_cs_starts(item, PLACER("<", 1))) {
				ar_append(&(output->peek_partial), ARRAY_TYPE_POINTER, item);
				increment++;
			}
			else {
				ar_append(&(output->peek_partial), ARRAY_TYPE_POINTER, NULL);
			}
		}
		else if (!st_cmp_ci_eq(item, PLACER("BODY", 4))) {
			// If the next arugment is an array, then store that pointer.
			if (increment < number && imap_get_type_ar(array, increment) == IMAP_ARGUMENT_TYPE_ARRAY) {

				// Store the array.
				ar_append(&(output->normal), ARRAY_TYPE_ARRAY, imap_get_ar_ar(array, increment++));

				// If the next item is a stringer, and appears to be a partial indicator, add it to the array, otherwise NULL.
				if (increment < number && imap_get_type_ar(array, increment) != IMAP_ARGUMENT_TYPE_ARRAY && (item
					= imap_get_st_ar(array, increment)) != NULL && !st_cmp_cs_starts(item, PLACER("<", 1))) {
					ar_append(&(output->normal_partial), ARRAY_TYPE_POINTER, item);
					increment++;
				}
				else {
					ar_append(&(output->normal_partial), ARRAY_TYPE_POINTER, NULL);
				}
			}
			else {
				output->body = 1;
			}
		}
		// The command wasn't found or properly formed.
		else {
			imap_fetch_free_items(output);
			return NULL;
		}

	}

	return output;
}

stringer_t * imap_fetch_envelope(stringer_t *header) {

	placer_t header_pl = pl_null();
	stringer_t *holder[10], *parsed[6], *output = NULL;

	if (!header) {
		return NULL;
	}
	else {
		header_pl = pl_init(st_char_get(header), st_length_get(header));
	}

	holder[0] = mail_header_fetch_cleaned(&header_pl, PLACER("Date", 4));
	holder[1] = mail_header_fetch_cleaned(&header_pl, PLACER("Subject", 7));
	holder[2] = mail_header_fetch_cleaned(&header_pl, PLACER("From", 4));
	holder[3] = mail_header_fetch_cleaned(&header_pl, PLACER("Sender", 6));
	holder[4] = mail_header_fetch_cleaned(&header_pl, PLACER("Reply-To", 8));
	holder[5] = mail_header_fetch_cleaned(&header_pl, PLACER("To", 2));
	holder[6] = mail_header_fetch_cleaned(&header_pl, PLACER("Cc", 2));
	holder[7] = mail_header_fetch_cleaned(&header_pl, PLACER("Bcc", 3));
	holder[8] = mail_header_fetch_cleaned(&header_pl, PLACER("In-Reply-To", 11));
	holder[9] = mail_header_fetch_cleaned(&header_pl, PLACER("Message-ID", 10));

	parsed[0] = imap_parse_address(holder[2]);
	parsed[1] = imap_parse_address(holder[3]);
	parsed[2] = imap_parse_address(holder[4]);
	parsed[3] = imap_parse_address(holder[5]);
	parsed[4] = imap_parse_address(holder[6]);
	parsed[5] = imap_parse_address(holder[7]);

	output = imap_build_array("ssaaaaaass", holder[0], holder[1], parsed[0], parsed[1] == NULL ? parsed[0] : parsed[1],
		parsed[2] == NULL ? parsed[0] : parsed[2], parsed[3], parsed[4], parsed[5], holder[8], holder[9]);

	// Cleanup
	for (int_t i = 0; i < 10; i++) {
		st_cleanup(holder[i]);
	}

	for (int_t i = 0; i < 6; i++) {
		st_cleanup(parsed[i]);
	}

	return output;
}

stringer_t * imap_fetch_bodystructure(mail_mime_t *mime) {

	array_t *items;
	uint64_t lines = 0;
	size_t increment = 0, length;
	chr_t *stream, buffer[32];
	stringer_t *output = NULL, *current, *value, *literal, *holder[8];

	if (!mime) {
		return NULL;
	}

	if (!mime->children) {

		// Content type group.
		if ((holder[0] = mail_mime_type_group(mime->header))) {
			upper_st(holder[0]);
		}
		// Content sub type.
		if ((holder[1] = mail_mime_type_sub(mime->header))) {
			upper_st(holder[1]);
		}

		// Content type parameters.
		if (!(items = mail_mime_type_parameters(mime->header))) {
			holder[2] = st_import("(\"CHARSET\" \"US-ASCII\")", ns_length_get("(\"CHARSET\" \"US-ASCII\")"));
		}
		else {
			// Build the parameterized list.
			holder[2] = NULL;
			length = ar_length_get(items);

			while (increment < length) {
				current = ar_field_st(items, increment);
				// If the current item is a literal.

				if (current && (literal = imap_build_array_isliteral(pl_init(st_char_get(current), st_length_get(current))))) {

					if ((value = st_merge("sns", holder[2], holder[2] ? " " : "", literal))) {
						st_cleanup(holder[2]);
						holder[2] = value;
					}

					st_free(literal);
				}
				// If the current item is a quoted string.
				else if (current) {

					if ((value = st_merge("snnsn", holder[2], holder[2] != NULL ? " " : "", "\"", current, "\""))) {
						st_cleanup(holder[2]);
						holder[2] = value;
					}

				}
				// If the current item is nil.
				else {

					if ((value = st_merge("snn", holder[2], holder[2] != NULL ? " " : "", "NIL"))) {
						st_cleanup(holder[2]);
						holder[2] = value;
					}

				}

				increment++;
			}
			// Add the enclosing params.
			if (holder[2] && (value = st_merge("nsn", "(", holder[2], ")"))) {
				st_free(holder[2]);
				holder[2] = value;
			}

			ar_free(items);
		}

		// Content id.
		holder[3] = mail_mime_content_id(mime->header);
		// Content description.
		holder[4] = mail_header_fetch_cleaned(&(mime->header), PLACER("Content-Description", 19));
		// Content encoding.
		if ((holder[5] = mail_mime_content_encoding(mime->header))) {
			upper_st(holder[5]);
		}

		// Body size, octets.
		snprintf(buffer, 32, "%zu", st_length_get(&(mime->body)));
		holder[6] = st_import(buffer, ns_length_get(buffer));

		// For text blocks output the number of lines, otherwise send back a NIL.
		if (holder[0] != NULL && !st_cmp_cs_eq(holder[0], PLACER("TEXT", 4))) {

			stream = st_data_get(&(mime->body));
			length = st_length_get(&(mime->body));

			while (length-- > 0) {

				if (*stream++ == '\n') {
					lines++;
				}

			}

			if (lines == 0) {
				lines = 1;
			}

			snprintf(buffer, 32, "%lu", lines);
			holder[7] = st_import(buffer, ns_length_get(buffer));
		}
		else {
			holder[7] = st_import("NIL", 3);
		}

		output = imap_build_array("ssasssaa", holder[0], holder[1], holder[2], holder[3], holder[4], holder[5], holder[6], holder[7]);
		length = 0;

		while (length <= 7) {
			st_cleanup(holder[length]);
			length++;
		}

	}
	else {
		length = ar_length_get(mime->children);

		while (increment < length) {
			current = imap_fetch_bodystructure(ar_field_ptr(mime->children, increment));

			if (current) {

				if ((value = st_merge("ss", output, current))) {
					st_cleanup(output);
					output = value;
				}

				st_free(current);
			}

			increment++;
		}
		// What type of multipart.
		if ((literal = mail_mime_type_sub(mime->header))) {
			upper_st(literal);
		}

		// Build the final output.
		if ((value = imap_build_array("as", output, literal))) {
			st_cleanup(output);
			output = value;
		}

		st_cleanup(literal);
	}

	return output;
}

stringer_t * imap_fetch_body_header(placer_t header, imap_arguments_t *array, int_t not) {

	placer_t line;
	int_t match = 0;
	size_t position = 0;
	uint32_t increment, number;
	stringer_t *output = NULL, *key;

	number = ar_length_get(array);

	// Go line by line.
	while (!pl_empty((line = mail_header_pop(&header, &position)))) {

		// Reset the match indicator.
		match = 0;

		// Compare each line to array of fields.
		for (increment = 0; increment < number; increment++) {

			// Only compare if its a string argument.
			if (imap_get_type_ar(array, increment) != IMAP_ARGUMENT_TYPE_ARRAY && (key = imap_get_st_ar(array, increment))) {

				// Depending on our not variable, either include/exclude matches.
				if (!mm_cmp_ci_eq(st_char_get(key), pl_data_get(line), st_length_get(key)) && *(pl_char_get(line) + st_length_get(key)) == ':') {

					match = 1;

					// We don't need any more comparisons.
					increment = number;
				}

			}

		}

		if (not == match && (key = st_merge("ss", output, &line))) {
			st_cleanup(output);
			output = key;
		}

	}

	// Always return at least one line, or the trailing line.
	if ((!output || not == 1) && (key = st_merge("sn", output, "\r\n"))) {
		st_cleanup(output);
		output = key;
	}

	return output;
}

stringer_t * imap_fetch_body_mime(placer_t header) {

	int_t increment;
	stringer_t *output, *lines[4];

	// Pull the MIME header fields.
	lines[0] = mail_header_fetch_all(&header, PLACER("Content-ID", 10));
	lines[1] = mail_header_fetch_all(&header, PLACER("Content-Type", 12));
	lines[2] = mail_header_fetch_all(&header, PLACER("Content-Transfer-Encoding", 25));
	lines[3] = mail_header_fetch_all(&header, PLACER("Content-Disposition", 19));

	// If all are NULL, return NULL, and the program will output NIL.
	if (!lines[0] && !lines[1] && !lines[2] && !lines[3]) {
		return NULL;
	}

	// Build the string.
	output = st_merge("ssss", lines[0], lines[1], lines[2], lines[3]);

	// Cleanup
	for (increment = 0; increment < 4; increment++) {
		st_cleanup(lines[increment]);
	}

	return output;
}

stringer_t * imap_fetch_body_tag(stringer_t *tag, array_t *items) {

	uint32_t number = 0;
	stringer_t *value, *array = NULL, *field = NULL, *output = NULL;

	// Size
	if (items) {
		number = ar_length_get(items);
	}

	// Does this body have an array.
	if (number) {

		if (!(array = st_import("(", 1))) {
			return NULL;
		}

		// Loop through and add each item.
		for (uint32_t i = 0; i < number; i++) {
			field = imap_get_st_ar(items, i);

			if (field && (value = st_merge("sns", array, i != 0 ? " " : "", field))) {
				st_free(array);
				array = value;
			}

		}

		if ((value = st_merge("sn", array, ")"))) {
			st_free(array);
			array = value;
		}

	}

	// If we have a tag, uppercase it.
	if (tag) {
		upper_st(tag);
	}

	// Build the string.
	output = st_merge("nsnsn", "BODY[", tag, (array == NULL || tag == NULL) ? "" : " ", array, "]");
	st_cleanup(array);

	return output;
}

// Takes a dotted string, and returns just that portion of it.
placer_t imap_fetch_body_portion(stringer_t *part) {

	chr_t *stream;
	int_t dots = 0;
	size_t length = 0, remaining;

	if (st_empty_out(part, (uchr_t **)&stream, &remaining)) {
		return pl_null();
	}

	while (remaining-- > 0 && ((*stream >= '0' && *stream <= '9') || *stream == '.')) {

		// Make sure we don't get multiple dots.
		if (*stream == '.') {
			dots++;
		}
		else if (dots != 0) {
			dots = 0;
		}

		// If we get multiple dots, return zero.
		if (dots == 2) {
			return pl_null();
		}

		length++;
		stream++;
	}

	// Don't include trailing dots.
	if (dots != 0 && length > 0) {
		length--;
	}

	// If there aren't any characters, return zero.
	if (length == 0) {
		return pl_null();
	}

	return pl_init(st_char_get(part), length);
}

// Takes a body part, and returns the mime structure for that part.
mail_mime_t * imap_fetch_body_part(mail_message_t *message, placer_t portion) {

	placer_t token;
	mail_mime_t *current;
	uint32_t segment, number;

	current = message->mime;
	number = tok_get_count_st(&portion, '.');

	// This is a non MIME message.
	if (!current || !current->children) {

		// Get the number.
		tok_get_st(&portion, '.', 0, &token);

		// Get the numeric portion.
		if (!uint32_conv_st(&token, &segment)) {
			return NULL;
		}

		// Were asking for the whole message.
		if (segment == 1) {
			return current;
		}
		// Were asking for a child element, and there are none.
		else {
			return NULL;
		}
	}

	for (uint32_t i = 0; i < number; i++) {

		// Get each section of the number.
		tok_get_st(&portion, '.', i, &token);

		// Protection against requests for non-existent children.
		if (!current || !current->children) {
			return NULL;
		}

		// Get the numeric portion.
		if (uint32_conv_st(&token, &segment) == true) {
			current = ar_field_ptr(current->children, segment - 1);
		}
		else {
			return current;
		}

	}

	return current;
}

// Takes a partial modifier, and parses it.
int_t imap_fetch_parse_partial(stringer_t *partial, size_t *start, size_t *length) {

	int_t number;
	chr_t *stream;
	size_t remaining, numlen;

	if (partial == NULL || start == NULL || length == NULL) {
		return 0;
	}

	// In case we exit early.
	*start = 0;
	*length = 0;

	// Get setup.
	stream = st_char_get(partial);
	remaining = st_length_get(partial);

	// Advance past the garbage.
	while (remaining > 0 && (*stream == ' ' || *stream == '<')) {
		stream++;
		remaining--;
	}

	if (remaining == 0) {
		return 0;
	}

	// How long is the number.
	numlen = 0;
	while (remaining > 0 && *(stream + numlen) >= '0' && *(stream + numlen) <= '9') {
		numlen++;
		remaining--;
	}

	if (numlen == 0) {
		return 0;
	}

	if (int32_conv_bl(stream, numlen, &number) != true) {
		return 0;
	}

	// Store the number.
	*start = (size_t)number;

	// Advance the stream.
	stream += numlen;

	// Advance past the garbage.
	while (remaining > 0 && (*stream == ' ' || *stream == '.' || *stream == '>')) {
		stream++;
		remaining--;
	}

	if (remaining == 0) {
		return 1;
	}

	// How long is the number.
	numlen = 0;
	while (remaining > 0 && *(stream + numlen) >= '0' && *(stream + numlen) <= '9') {
		numlen++;
		remaining--;
	}

	if (int32_conv_bl(stream, numlen, &number) != true) {
		return 1;
	}

	// Store the number.
	*length = (size_t)number;

	return 2;
}

stringer_t * imap_fetch_return_header(connection_t *con, meta_message_t *meta, mail_message_t **message, stringer_t **header,
	imap_fetch_response_t *output) {

	stringer_t *result = NULL;

	// The header has already been loaded.
	if (*header != NULL) {
		result = *header;
	}

	// We have already loaded the full message. So just copy the header portion.
	else if (*message != NULL && (*header = st_import(st_char_get((*message)->text), (*message)->header_length)) != NULL) {
		result = *header;
	}

	// We need to pull the header off the disk.
	else if ((*header = mail_load_header(meta, con->imap.user)) != NULL) {
		result = *header;
	}

	// We couldn't load the header.
	else if (result == NULL) {
		mail_destroy(*message);
		imap_fetch_response_free(output);
	}

	return result;
}

stringer_t * imap_fetch_return_text(connection_t *con, meta_message_t *meta, mail_message_t **message, stringer_t **header,
	imap_fetch_response_t *output) {

	if (*message == NULL && ((*message = mail_load_message(meta, con->imap.user, con->server, 1)) == NULL
		|| (*message)->text == NULL)) {
		mail_destroy(*message);
		mail_destroy_header(*header);
		imap_fetch_response_free(output);
		return NULL;
	}

	return (*message)->text;
}

mail_message_t * imap_fetch_return_message(connection_t *con, meta_message_t *meta, mail_message_t **message, stringer_t **header,
	imap_fetch_response_t *output) {

	if (*message == NULL && ((*message = mail_load_message(meta, con->imap.user, con->server, 1)) == NULL
		|| (*message)->text == NULL || mail_mime_update(*message) == 0)) {
		mail_destroy(*message);
		mail_destroy_header(*header);
		imap_fetch_response_free(output);
		return NULL;
	}

	return *message;
}

mail_mime_t * imap_fetch_return_mime(connection_t *con, meta_message_t *meta, mail_message_t **message, stringer_t **header,
	imap_fetch_response_t *output) {

	if (*message == NULL && ((*message = mail_load_message(meta, con->imap.user, con->server, 1)) == NULL
		|| (*message)->text == NULL)) {
		mail_destroy(*message);
		mail_destroy_header(*header);
		imap_fetch_response_free(output);
		return NULL;
	}
	else if ((*message)->mime == NULL && mail_mime_update(*message) == 0) {
		mail_destroy(*message);
		mail_destroy_header(*header);
		imap_fetch_response_free(output);
		return NULL;
	}

	return (*message)->mime;
}

imap_fetch_response_t * imap_fetch_body(array_t *outer, array_t *partial, connection_t *con, meta_message_t *meta,
	mail_message_t **message, stringer_t **header, imap_fetch_response_t *output) {

	int_t state;
	array_t *inner;
	mail_mime_t *mime;
	uint32_t number;
	chr_t buffer[128], *stream;
	size_t start, length, value_len;
	placer_t value_pl, headpl, portion = pl_null();
	stringer_t *value_st, *item, *complete, *trailer = NULL, *holder, *tag = NULL;

	// Size
	if (outer != NULL) {
		number = ar_length_get(outer);
	}
	else {
		number = 0;
	}

	for (uint32_t i = 0; i < number; i++) {

		// Reset these variables.
		holder = value_st = tag = NULL;
		portion = headpl = value_pl = pl_null();
		mime = NULL;

		// We should always have arrays.
		if (ar_field_type(outer, i) != ARRAY_TYPE_ARRAY) {
			imap_fetch_response_free(output);
			return NULL;
		}

		// The items requested.
		inner = ar_field_ar(outer, i);

		// Empty array. Print_t the entire message.
		if (inner == NULL || ar_length_get(inner) == 0) {
			if ((holder = imap_fetch_return_text(con, meta, message, header, output)) == NULL) {
				return NULL;
			}
			value_pl = pl_init(st_char_get(holder), st_length_get(holder));
			tag = imap_fetch_body_tag(NULL, NULL);
		}
		// What is the first item.
		else {

			// The first argument should be a string.
			if (imap_get_type_ar(inner, 0) == IMAP_ARGUMENT_TYPE_ARRAY) {
				mail_destroy(*message);
				mail_destroy_header(*header);
				imap_fetch_response_free(output);
				return NULL;
			}

			// Setup the default section.
			complete = item = imap_get_st_ar(inner, 0);

			// See if were supposed to be looking at a subsection.
			if (!pl_empty((portion = imap_fetch_body_portion(item)))) {
				if ((*message = imap_fetch_return_message(con, meta, message, header, output)) == NULL) {
					return NULL;
				}
				mime = imap_fetch_body_part(*message, portion);
			}

			// We've extracted a portion, so build a new stringer with the trailing part.
			if (!pl_empty(portion) && pl_length_get(portion) == st_length_get(item)) {
				item = NULL;
			}
			else if (!pl_empty(portion)) {
				stream = st_char_get(item) + pl_length_get(portion);
				length = st_length_get(item) - pl_length_get(portion);

				// Skip a trailing dot.
				if (length > 0 && *stream == '.') {
					length--;
					stream++;
				}

				// If it ends in a dot, just give them the body.
				if (length == 0) {
					item = NULL;
				}
				else {
					item = trailer = st_import(stream, length);
				}
			}

			// The MIME section wasn't found or its not a multipart message and were requesting a subsection.
			if (!pl_empty(portion) && item != NULL && (mime == NULL || (*message != NULL && (*message)->mime == mime))) {
				tag = imap_fetch_body_tag(complete, NULL);
			}
			// If the item is NULL, its because we were asked for the body section.
			else if (item == NULL) {

				// If a portion was requested, and mime is NULL then the return value should be NIL.
				if (mime == NULL && !pl_empty(portion) && (mime = imap_fetch_return_mime(con, meta, message, header, output)) == NULL) {
					return NULL;
				}
				if (mime != NULL) {
					value_pl = pl_init(st_char_get(&(mime->body)), st_length_get(&(mime->body)));
				}

				tag = imap_fetch_body_tag(complete, NULL);
			}
			// Are we looking for header fields.
			else if (!st_cmp_ci_eq(item, PLACER("HEADER.FIELDS.NOT", 17))) {
				if (mime != NULL) {
					headpl = pl_init(st_char_get(&(mime->header)), st_length_get(&(mime->header)));
				}
				else {
					if ((holder = imap_fetch_return_header(con, meta, message, header, output)) == NULL) {
						return NULL;
					}
					headpl = pl_init(st_char_get(holder), st_length_get(holder));
				}

				// LOW: In theory we could accept strings as arguments, but it would take a little more work.
				if (ar_length_get(inner) != 2 || imap_get_type_ar(inner, 1) != IMAP_ARGUMENT_TYPE_ARRAY || (value_st = imap_fetch_body_header(
					headpl, imap_get_ar_ar(inner, 1), 0)) == NULL) {
					imap_fetch_response_free(output);
					mail_destroy(*message);
					mail_destroy_header(*header);
					return NULL;
				}
				tag = imap_fetch_body_tag(complete, imap_get_ar_ar(inner, 1));
			}
			// Get an array of header fields.
			else if (!st_cmp_ci_eq(item, PLACER("HEADER.FIELDS", 13))) {
				if (mime != NULL) {
					headpl = pl_init(st_char_get(&(mime->header)), st_length_get(&(mime->header)));
				}
				else {
					if ((holder = imap_fetch_return_header(con, meta, message, header, output)) == NULL) {
						return NULL;
					}
					headpl = pl_init(st_char_get(holder), st_length_get(holder));
				}

				// LOW: In theory we could accept strings as arguments, but it would take a little more work.
				if (ar_length_get(inner) != 2 || imap_get_type_ar(inner, 1) != IMAP_ARGUMENT_TYPE_ARRAY || (value_st = imap_fetch_body_header(
					headpl, imap_get_ar_ar(inner, 1), 1)) == NULL) {
					imap_fetch_response_free(output);
					mail_destroy(*message);
					mail_destroy_header(*header);
					return NULL;
				}
				tag = imap_fetch_body_tag(complete, imap_get_ar_ar(inner, 1));
			}
			// Get the header.
			else if (!st_cmp_ci_eq(item, PLACER("HEADER", 6))) {
				if (mime != NULL) {
					value_pl = pl_init(st_char_get(&(mime->header)), st_length_get(&(mime->header)));
				}
				else {
					if ((holder = imap_fetch_return_header(con, meta, message, header, output)) == NULL) {
						return NULL;
					}
					value_pl = pl_init(st_char_get(holder), st_length_get(holder));
				}

				tag = imap_fetch_body_tag(complete, NULL);
			}
			// Get the body text.
			else if (!st_cmp_ci_eq(item, PLACER("TEXT", 4))) {
				if (mime != NULL) {
					value_pl = pl_init(st_char_get(&(mime->body)), st_length_get(&(mime->body)));
				}
				else {
					if ((mime = imap_fetch_return_mime(con, meta, message, header, output)) == NULL) {
						return NULL;
					}
					value_pl = pl_init(st_char_get(&(mime->body)), st_length_get(&(mime->body)));
				}
				tag = imap_fetch_body_tag(complete, NULL);
			}
			// Get the MIME fields.
			else if (!st_cmp_ci_eq(item, PLACER("MIME", !!00))) {
				if (mime != NULL) {
					headpl = pl_init(st_char_get(&(mime->header)), st_length_get(&(mime->header)));
				}
				else {
					if ((holder = imap_fetch_return_header(con, meta, message, header, output)) == NULL) {
						return NULL;
					}
					headpl = pl_init(st_char_get(holder), st_length_get(holder));
				}

				value_st = imap_fetch_body_mime(headpl);
				tag = imap_fetch_body_tag(complete, NULL);
			}
			// Otherwise just give them a NIL.
			else {
				tag = imap_fetch_body_tag(complete, NULL);
			}

			// Cleanup
			if (trailer != NULL) {
				st_free(trailer);
				trailer = NULL;
			}
		}

		// Store the value.
		if (tag) {

			// Look for a partial indicator.
			if ((item = imap_get_ptr(partial, i)) != NULL && (state = imap_fetch_parse_partial(item, &start, &length)) != 0) {

				// What are looking at.
				if (!pl_empty(value_pl)) {
					stream = pl_data_get(value_pl);
					value_len = pl_length_get(value_pl);
				}
				else if (value_st != NULL) {
					stream = st_char_get(value_st);
					value_len = st_length_get(value_st);
				}
				else {
					stream = NULL;
					value_len = 0;
				}

				// Build the partial.
				if (start >= value_len) {
					stream = NULL;
					value_len = 0;
				}
				else {
					stream += start;
					value_len -= start;
				}

				// Length modifier.
				if (state == 2 && length < value_len) {
					value_len = length;
				}

				// Build the new tag.
				if (snprintf(buffer, 128, "<%zu>", start) > 0) {
					complete = st_merge("sn", tag, buffer);
				}
				else {
					complete = NULL;
				}

				// Build the value, and add it to the output.
				if (value_len != 0 && snprintf(buffer, 128, "{%zu}\r\n", value_len) > 0) {
					value_pl = pl_init(stream, value_len);
					item = st_merge("ns", buffer, &value_pl);
					output = imap_fetch_response_add(output, !complete ? tag : complete, item);
				}
				else if ((item = st_import("NIL", 3)) != NULL) {
					output = imap_fetch_response_add(output, !complete ? tag : complete, item);
				}

				// Since we built a new tag.
				st_cleanup(complete);
			}
			// Otherwise output the whole thing.
			else if (!pl_empty(value_pl) && snprintf(buffer, 128, "{%zu}\r\n", pl_length_get(value_pl)) > 0 && (item = st_merge("ns", buffer, &value_pl)) != NULL) {
				output = imap_fetch_response_add(output, tag, item);
			}
			else if (value_st != NULL && snprintf(buffer, 128, "{%zu}\r\n", st_length_get(value_st)) > 0 && (item = st_merge("ns", buffer, value_st)) != NULL) {
				output = imap_fetch_response_add(output, tag, item);
			}
			else if ((item = st_import("NIL", 3)) != NULL) {
				output = imap_fetch_response_add(output, tag, item);
			}

			// Cleanup
			st_free(tag);
			st_cleanup(value_st);
		}
	}

	return output;
}

// Will return a stringer with all of the desired results.
imap_fetch_response_t * imap_fetch_message(connection_t *con, meta_message_t *meta, imap_fetch_dataitems_t *items) {

	int_t state;
	time_t ctime;
	struct tm ltime;
	chr_t buffer[128];
	mail_message_t *message = NULL;
	stringer_t *value, *header = NULL;
	imap_fetch_response_t *output = NULL;

	// Process the UID.
	if (con->imap.uid == 1 || items->uid == 1) {
		if (!(value = st_aprint_opts(MANAGED_T | HEAP | CONTIGUOUS, "%lu", meta->messagenum))) {
			return NULL;
		}
		output = imap_fetch_response_add(output, PLACER("UID", 3), value);
	}

	// Process the message flags.
	if (items->flags == 1 || meta->updated == 1) {
		if ((value
			= st_merge("nnnnnnnnnnnnn", "(", (meta->status & MAIL_STATUS_ANSWERED) != 0 ? "\\Answered" : "",
				(meta->status & MAIL_STATUS_ANSWERED) != 0 && (meta->status & MAIL_STATUS_FLAGGED) != 0 ? " " : "",
				(meta->status & MAIL_STATUS_FLAGGED) != 0 ? "\\Flagged" : "",
				(meta->status & (MAIL_STATUS_ANSWERED | MAIL_STATUS_FLAGGED)) != 0 && (meta->status & MAIL_STATUS_DELETED) != 0 ? " " : "",
				(meta->status & MAIL_STATUS_DELETED) != 0 ? "\\Deleted" : "",
				(meta->status & (MAIL_STATUS_ANSWERED | MAIL_STATUS_FLAGGED | MAIL_STATUS_DELETED)) != 0 && (meta->status & MAIL_STATUS_SEEN) != 0 ? " " : "",
				(meta->status & MAIL_STATUS_SEEN) != 0 ? "\\Seen" : "",
				(meta->status & (MAIL_STATUS_ANSWERED | MAIL_STATUS_FLAGGED | MAIL_STATUS_DELETED | MAIL_STATUS_SEEN)) != 0 && (meta->status & MAIL_STATUS_DRAFT) != 0 ? " " : "",
				(meta->status & MAIL_STATUS_DRAFT) != 0 ? "\\Draft" : "",
				(meta->status & (MAIL_STATUS_ANSWERED | MAIL_STATUS_FLAGGED | MAIL_STATUS_DELETED | MAIL_STATUS_SEEN | MAIL_STATUS_DRAFT)) != 0 && (meta->status & MAIL_STATUS_RECENT) != 0 ? " " : "",
				(meta->status & MAIL_STATUS_RECENT) != 0 ? "\\Recent" : "", ")")) == NULL) {
			imap_fetch_response_free(output);
			return NULL;
		}
		output = imap_fetch_response_add(output, PLACER("FLAGS", 5), value);
	}

	// Process the internal date.
	if (items->internaldate == 1) {
		ctime = meta->created;
		if (localtime_r(&ctime, &ltime) == NULL || strftime(buffer, 128, "\"%d-%b-%Y %H:%M:%S %z\"", &ltime) <= 0 ||
			(value = st_import(buffer, ns_length_get(buffer))) == NULL) {
			imap_fetch_response_free(output);
			return NULL;
		}
		output = imap_fetch_response_add(output, PLACER("INTERNALDATE", 12), value);
	}

	// Process the RFC822 text.
	if (items->rfc822_text == 1) {
		if (message == NULL && ((message = mail_load_message(meta, con->imap.user, con->server, 1)) == NULL
			|| mail_mime_update(message) == 0)) {
			mail_destroy(message);
			mail_destroy_header(header);
			imap_fetch_response_free(output);
			return NULL;
		}
		else if (snprintf(buffer, 128, "{%zu}\r\n", st_length_get(&(message->mime->body))) <= 0 || (value = st_merge("ns", buffer, &(message->mime->body))) == NULL) {
			mail_destroy(message);
			mail_destroy_header(header);
			imap_fetch_response_free(output);
			return NULL;
		}
		output = imap_fetch_response_add(output, PLACER("RFC822.TEXT", 11), value);
	}

	// Process the entire RFC822 message.
	if (items->rfc822 == 1) {
		if (message == NULL && ((message = mail_load_message(meta, con->imap.user, con->server, 1)) == NULL
			|| mail_mime_update(message) == 0)) {
			mail_destroy(message);
			mail_destroy_header(header);
			imap_fetch_response_free(output);
			return NULL;
		}
		else if (snprintf(buffer, 128, "{%zu}\r\n", st_length_get(message->text)) <= 0 || (value = st_merge("ns", buffer, message->text))
			== NULL) {
			mail_destroy(message);
			mail_destroy_header(header);
			imap_fetch_response_free(output);
			return NULL;
		}
		output = imap_fetch_response_add(output, PLACER("RFC822", 6), value);
	}

	// Process the body.
	if (items->body == 1) {
		if (message == NULL && ((message = mail_load_message(meta, con->imap.user, con->server, 1)) == NULL
			|| mail_mime_update(message) == 0)) {
			mail_destroy(message);
			mail_destroy_header(header);
			imap_fetch_response_free(output);
			return NULL;
		}
		else if ((value = imap_fetch_bodystructure(message->mime)) == NULL) {
			mail_destroy(message);
			mail_destroy_header(header);
			imap_fetch_response_free(output);
			return NULL;
		}
		output = imap_fetch_response_add(output, PLACER("BODY", 4), value);
	}

	// Process the bodystructure.
	if (items->bodystructure == 1) {
		if (message == NULL && ((message = mail_load_message(meta, con->imap.user, con->server, 1)) == NULL
			|| mail_mime_update(message) == 0)) {
			mail_destroy(message);
			mail_destroy_header(header);
			imap_fetch_response_free(output);
			return NULL;
		}
		else if ((value = imap_fetch_bodystructure(message->mime)) == NULL) {
			mail_destroy(message);
			mail_destroy_header(header);
			imap_fetch_response_free(output);
			return NULL;
		}
		output = imap_fetch_response_add(output, PLACER("BODYSTRUCTURE", 13), value);
	}

	// An array of body items was requested.
	if (items->normal != NULL) {
		if ((output = imap_fetch_body(items->normal, items->normal_partial, con, meta, &message, &header, output)) == NULL) {
			log_pedantic("Unable to fetch the body for message %lu.", meta->messagenum);
			return NULL;
		}
	}

	if (items->peek != NULL) {
		if ((output = imap_fetch_body(items->peek, items->peek_partial, con, meta, &message, &header, output)) == NULL) {
			log_pedantic("Unable to fetch the body for message %lu.", meta->messagenum);
			return NULL;
		}
	}

	// Process the message envelope.
	if (items->envelope == 1) {
		if ((header = imap_fetch_return_header(con, meta, &message, &header, output)) == NULL) {
			return NULL;
		}
		else if ((value = imap_fetch_envelope(header)) == NULL) {
			mail_destroy(message);
			mail_destroy_header(header);
			imap_fetch_response_free(output);
			return NULL;
		}
		output = imap_fetch_response_add(output, PLACER("ENVELOPE", 8), value);
	}

	// Process the RFC822 header.
	if (items->rfc822_header == 1) {
		if ((header = imap_fetch_return_header(con, meta, &message, &header, output)) == NULL) {
			return NULL;
		}
		else if (snprintf(buffer, 128, "{%zu}\r\n", st_length_get(header)) <= 0 || (value = st_merge("ns", buffer, header)) == NULL) {
			mail_destroy(message);
			mail_destroy_header(header);
			imap_fetch_response_free(output);
			return NULL;
		}
		output = imap_fetch_response_add(output, PLACER("RFC822.HEADER", 13), value);
	}

	// Process the RFC822 size.
	if (items->rfc822_size == 1) {

		// If we had to load the message, use the actual size. Otherwise use the size stored in the database.
		if (message != NULL && message->text != NULL) {
			state = snprintf(buffer, 128, "%zu", st_length_get(message->text));
		}
		else {
			state = snprintf(buffer, 128, "%zu", meta->size);
		}

		if (state <= 0 || (value = st_import(buffer, ns_length_get(buffer))) == NULL) {
			mail_destroy(message);
			mail_destroy_header(header);
			imap_fetch_response_free(output);
			return NULL;
		}
		output = imap_fetch_response_add(output, PLACER("RFC822.SIZE", 11), value);
	}

	mail_destroy(message);
	mail_destroy_header(header);

	return output;
}

/**
 * @brief	Create a copy of a collection of messages.
 * @see		meta_message_dupe()
 * @param	messages	a collection of meta message objects to be duplicated.
 * @return	the copied collection of meta messages on success, or NULL on failure.
 */
inx_t * imap_duplicate_messages(inx_t *messages) {

	inx_t *output = NULL;
	inx_cursor_t *cursor;
	meta_message_t *active, *duplicate;
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = 0 };

	// Allocate the linked the list.
	if (!(output = inx_alloc(M_INX_LINKED, &meta_message_free))) {
		return NULL;
	}

	if ((cursor = inx_cursor_alloc(messages))) {

		while ((active = inx_cursor_value_next(cursor))) {

			if (!(duplicate = meta_message_dupe(active))) {
				log_error("Unable to duplicate the message structure.");
				inx_cursor_free(cursor);
				inx_free(output);
				return NULL;
			}
			else if (!(key.val.u64 = active->messagenum) || !inx_append(output, key, duplicate)) {
				log_error("Unable to add the duplicate message to our list.");
				meta_message_free(duplicate);
				inx_cursor_free(cursor);
				inx_free(output);
				return NULL;
			}

		}

		inx_cursor_free(cursor);
	}

	return output;
}

// Returns a copy of the messages. Make sure you rely on the message numbers and not the sequence numbers.
inx_t * imap_narrow_messages(inx_t *messages, uint64_t selected, stringer_t *range, int_t uid) {

	int_t asterisk;
	inx_t *output = NULL;
	inx_cursor_t *cursor;
	uint32_t commas, parts;
	meta_message_t *active;
	placer_t sequence, start_token, end_token;
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = 0 };
	uint64_t start, end, number, highest_uid = 0, highest_seq = 0;

	if (!messages || !range) {
		log_error("Sanity check failed, passed a NULL parameter.");
		return NULL;
	}

	// Allocate the linked list.
	if (!(output = inx_alloc(M_INX_LINKED, NULL))) {
		return NULL;
	}

	// Find the highest message number.
	if ((cursor = inx_cursor_alloc(messages))) {

		while ((active = inx_cursor_value_next(cursor))) {

			if (active->foldernum == selected && active->messagenum > highest_uid) {
				highest_uid = active->messagenum;
			}

			if (active->foldernum == selected && active->sequencenum > highest_seq) {
				highest_seq = active->sequencenum;
			}

		}

		inx_cursor_free(cursor);
	}

	// Count the commas.
	commas = tok_get_count_st(range, ',');

	// Break apart each sequence section.
	for (uint32_t i = 0; i < commas && tok_get_st(range, ',', i, &sequence) >= 0; i++) {

		// Reset the counters.
		start = end = asterisk = 0;
		start_token = end_token = pl_null();

		// How many parts are in this sequence.
		if (!(parts = tok_get_count_st(&sequence, ':'))) {
			log_pedantic("range parsing error = %.*s", st_length_int(range), st_char_get(range));
			inx_cleanup(output);
			return NULL;
		}

		// Try and get the first part of the sequence.
		if (tok_get_st(&sequence, ':', 0, &start_token) < 0) {
			log_pedantic("range parsing error = %.*s", st_length_int(range), st_char_get(range));
			inx_cleanup(output);
			return NULL;
		}

		if (parts > 1) {
			tok_get_st(&sequence, ':', 1, &end_token);
		}

		// Parse the start.
		if (*(pl_char_get(start_token)) == '*') {
			asterisk = 1;
		}
		else {

			if (!uint64_conv_st(&start_token, &start)) {
				log_pedantic("range parsing error = %.*s", st_length_int(range), st_char_get(range));
				inx_cleanup(output);
				return NULL;
			}

		}

		// Parse the end.
		if (!pl_empty(end_token) && *(pl_char_get(end_token)) == '*') {
			asterisk = 1;
		} else if (pl_empty(end_token)) {
			end = start;
		} else {

			if (!uint64_conv_st(&end_token, &end)) {
				log_pedantic("range parsing error = %.*s", st_length_int(range), st_char_get(range));
				inx_cleanup(output);
				return NULL;
			}

		}

		// If necessary, swap the values.
		if (asterisk == 1 && end != 0) {
			start = end;
		}
		else if (asterisk == 0 && start > end) {
			number = start;
			start = end;
			end = number;
		}

		// If either value is higher than any valid message value, and we have an asterisk, include the highest numbered message. Observed behavior.
		if (uid == 0 && asterisk == 1 && start > highest_seq) {
			start = highest_seq;
		}
		else if (uid == 1 && asterisk == 1 && start > highest_uid) {
			start = highest_uid;
		}

		// Loop through and collect the messages.
		number = 0;

		//log_pedantic("start = %lu / end = %lu / asterisk = %i / uid = %i { %.*s }", start, end, asterisk, uid, st_length_int(range), st_char_get(range));

		if ((cursor = inx_cursor_alloc(messages))) {

			while ((active = inx_cursor_value_next(cursor))) {

				if (active->foldernum == selected && ((uid == 0 && ++number >= start && (asterisk == 1 || number <= end)) ||
					(uid == 1 && active->messagenum >= start && (asterisk == 1 || active->messagenum <= end)))) {
					key.val.u64 = active->messagenum;
					inx_append(output, key, active);
				}

			}

			inx_cursor_free(cursor);
		}
	}

	// If no nodes were returned.
	if (!inx_count(output)) {
		inx_free(output);
		return NULL;
	}

	return output;
}
