
/**
 * @file /magma/servers/imap/parse_address.c
 *
 * @brief Functions used to handle IMAP commands/actions.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/// LOW: Do we need an imap_parse_address_group() function?

void imap_parse_address_put(stringer_t *buffer, chr_t c) {

	if (buffer == NULL) {
		return;
	}

	if (st_length_get(buffer) >= st_avail_get(buffer)) {
		return;
	}

	*(st_char_get(buffer) + st_length_get(buffer)) = c;
	st_length_set(buffer, st_length_get(buffer) + 1);
	return;
}

stringer_t * imap_parse_address_part(placer_t input) {

	chr_t *stream;
	int_t mode = 0;
	placer_t box, dom;
	stringer_t *address = NULL, *name = NULL, *output = NULL;
	size_t zero_len = 0, one_len = 0, two_len = 0, length;

	length = pl_length_get(input);
	stream = pl_data_get(input);

	while (length > 0) {

		if (mode == 0 && *stream == '\"') {
			mode = 1;
		}
		else if (mode == 0 && *stream == '<') {
			mode = 2;
		}

		else if (mode == 1 && *stream == '\"') {
			mode = 0;
		}
		else if (mode == 2 && *stream == '>') {
			mode = 0;
		}

		else if (mode == 0 && *stream != ' ') {
			zero_len++;
		}
		else if (mode == 1) {
			one_len++;
		}
		else if (mode == 2) {
			two_len++;
		}

		length--;
		stream++;
	}

	/*if (two_len != 0) {
		if ((address = st_alloc(two_len)) == NULL) {
			return NULL;
		}
	}
	if (one_len != 0) {
		if ((name = st_alloc(one_len + zero_len)) == NULL) {
			return NULL;
		}
	}
	if (zero_len != 0 && two_len == 0 && one_len == 0) {
		if ((address = st_alloc(zero_len)) == NULL) {
			return NULL;
		}
	}*/

	if ((address = st_alloc(pl_length_get(input))) == NULL) {
		return NULL;
	}
	else if ((name = st_alloc(pl_length_get(input))) == NULL) {
		if (address) st_free(address);
		return NULL;

	}

	mode = 0;
	length = pl_length_get(input);
	stream = pl_data_get(input);

	while (length > 0) {

		if (mode == 0 && *stream == '\"') {
			mode = 1;
		}
		else if (mode == 0 && *stream == '<') {
			mode = 2;
		}

		else if (mode == 1 && *stream == '\"') {
			mode = 0;
		}
		else if (mode == 2 && *stream == '>') {
			mode = 0;
		}

		// No quoted part, no <> part.
		else if ((mode == 0 && one_len == 0 && two_len == 0) ||
			// If there is a quoted part, but no <>, use the quoted part for name, and the <> for address, and ignore everything outside that.
			(mode == 0 && one_len != 0 && two_len == 0) ||
			// Or the data is inside the <>.
			mode == 2) {

			// Email addresses shouldn't have spaces.
			if (*stream != ' ')
				imap_parse_address_put(address, *stream);
		}
		else if (mode == 0 || mode == 1) {
			// Keeps any leading spaces from being added.
			if (st_length_get(name) != 0 || (st_length_get(name) == 0 && *stream != ' '))
				imap_parse_address_put(name, *stream);
		}

		length--;
		stream++;
	}

	// A simple trimmer.
	while (st_length_get(name) != 0 && *(st_char_get(name) + st_length_get(name) - 1) == ' ') {
		st_length_set(name, st_length_get(name) - 1);
	}

	// In situations where a name couldn't be parsed, reset the pointer to NULL so that "NIL" is returned.
	if (st_length_get(name) == 0) {
		st_free(name);
		name = NULL;
	}

	tok_get_st(address, '@', 0, &box);
	tok_get_st(address, '@', 1, &dom);
	output = imap_build_array("snss", name, NULL, &box, &dom);
	if (name != NULL) {
		st_free(name);
	}
	if (address != NULL) {
		st_free(address);
	}

	return output;
}

placer_t imap_parse_address_breaker(stringer_t *address, uint32_t part) {

	size_t length;
	int_t status = 0;
	uint32_t position = 0;
	placer_t output = pl_null();
	chr_t *holder, *start = NULL, *end = NULL;

	if (address == NULL) {
		return pl_null();
	}

	length = st_length_get(address);
	holder = st_char_get(address);

	while (length > 0 && end == NULL) {

		if (position == part) {
			start = holder;
			position++;
		}

		// Inside quotes, nothing counts.
		if (status == 0 && *holder == '\"') {
			status = 1;
		}
		else if (status == 1 && *holder == '\"') {
			status = 0;
		}
		// Outside quotes, and we hit the break character.
		else if (status == 0 && position > part && *holder == ',') {
			end = holder;
		}
		else if (status == 0 && position < part && *holder == ',') {
			position++;
		}

		length--;
		holder++;
	}

	// If we hit the end.
	if (end == NULL) {
		end = holder;
	}

	if (start != NULL && end != NULL) {
		output = pl_init(start, end - start);
	}

	return output;
}

stringer_t * imap_parse_address(stringer_t *address) {

	placer_t token;
	uint32_t part = 0;
	stringer_t *holder, *value, *output = NULL;

	if (address == NULL) {
		return NULL;
	}

	while (!pl_empty((token = imap_parse_address_breaker(address, part++)))) {

		holder = imap_parse_address_part(token);

		if (output != NULL) {
			if ((value = st_merge("sns", output, " ", holder)) != NULL) {
				st_free(holder);
				st_free(output);
				output = value;
			}
			else {
				st_free(holder);
			}
		}
		else {
			output = holder;
		}
	}

	if (output != NULL && (value = imap_build_array("a", output)) != NULL) {
		st_free(output);
		output = value;
	}

	return output;
}
