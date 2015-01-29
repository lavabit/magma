
/**
 * @file /magma/servers/imap/output.c
 *
 * @brief Functions used to handle IMAP commands/actions.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

stringer_t * imap_build_array_isliteral(placer_t data) {

	size_t length;
	chr_t *holder, buffer[32];

	if (pl_empty(data)) {
		return NULL;
	}

	length = pl_length_get(data);
	holder = pl_data_get(data);

	// Look for illegal characters.
	for (size_t i = 0; i < length; i++) {
		if ((*holder >= '*' && *holder <= '~') || *holder == ' ' || *holder == '_'	|| *holder == '!' || (*holder >= '#' && *holder <= '&')) {
			holder++;
		}
		else {
			snprintf(buffer, 32, "{%zu}\r\n", length);
			return st_merge("ns", buffer, &data);

		}
	}

	return NULL;
}

stringer_t * imap_build_array(chr_t *format, ...) {

	va_list args;
	chr_t *nuller;
	placer_t placer;
	chr_t *result_position, *current_position;
	stringer_t *literal, *stringer, *result = NULL;
	size_t number, current_length, i, j, total = 0, length = 0;

	if (format == NULL) {
		log_pedantic("A NULL pointer was passed in.");
		return NULL;
	}

	// How many arguments were passed in.
	if ((number = ns_length_get(format)) == 0) {
		log_pedantic("A according to the format, zero arguments were passed in.");
		return NULL;
	}

	// Iterate through and determine how long this combined string should be.
	va_start(args, format);
	for (i = 0; i < number; i++) {
		if (*(format + i) == 's' || *(format + i) == 'a') {
			stringer = va_arg(args, stringer_t *);
			if (stringer != NULL) {
				length += st_length_get(stringer);
			}
			else {
				length += 3;
			}
		}
		else if (*(format + i) == 'n') {
			nuller = va_arg(args, chr_t *);
			if (nuller != NULL) {
				length += ns_length_get(nuller);
			}
			else {
				length += 3;
			}
		}
		else if (*(format + i) == 'p') {
			placer = va_arg(args, placer_t);
			if (!pl_empty(placer)) {
				length += pl_length_get(placer);
			}
			else {
				length += 3;
			}
		}
		else {
			log_error("Invalid type code passed in.");
			va_end(args);
			return NULL;
		}
	}
	va_end(args);

	// Now add enough adding in case any of the arguments are literals, and for the parens.
	length += (number * 64) + 2;

	// Allocate a buffer for the new string.
	if ((result = st_alloc(length)) == NULL) {
		log_pedantic("Unable to allocate %zu bytes for the combined string.", length);
		return NULL;
	}

	// Setup
	if ((result_position = st_char_get(result)) == NULL) {
		log_pedantic("Unable to retrieve the result pointer.");
		return NULL;
	}

	// Start with a paren.
	*(result_position++) = '(';
	++total;

	// Loop through and build.
	va_start(args, format);
	for (size_t i = 0; i < number; i++) {

		literal = NULL;

		// If this isn't the first value, add a space.
		if (i != 0) {
			*(result_position++) = ' ';
			++total;
		}

		if (*(format + i) == 's' || *(format + i) == 'a') {
			stringer = va_arg(args, stringer_t *);
			if (stringer != NULL) {
				// If its not an array, check whether we need to make this a literal.
				if (*(format + i) == 'a') {
					current_length = st_length_get(stringer);
					current_position = st_char_get(stringer);
				}
				else if ((literal = imap_build_array_isliteral(pl_init(st_char_get(stringer), st_length_get(stringer)))) == NULL) {
					current_length = st_length_get(stringer);
					current_position = st_char_get(stringer);
					*(result_position++) = '\"';
					++total;
				}
				else {
					current_length = st_length_get(literal);
					current_position = st_char_get(literal);
				}

				// Copy
				for (j = 0; j < current_length; j++) {
					*(result_position++) = *(current_position++);
					if (++total > length) {
						log_error("We've exceeded the space in the allocated buffer. This should never happen.");
						if (literal != NULL) {
							st_free(literal);
						}
						st_free(result);
						va_end(args);
						return NULL;
					}
				}
				// Free the literal holder, if it was used. Otherwise close quotes.
				if (literal != NULL) {
					st_free(literal);
				}
				else if (*(format + i) != 'a') {
					*(result_position++) = '\"';
					++total;
				}
			}
			else {
				*(result_position++) = 'N';
				*(result_position++) = 'I';
				*(result_position++) = 'L';
				total += 3;
			}
		}
		else if (*(format + i) == 'n') {
			nuller = va_arg(args, chr_t *);
			if (nuller != NULL) {
				// Check whether we need to make this a literal.
				if ((literal = imap_build_array_isliteral(pl_init(nuller, ns_length_get(nuller)))) == NULL) {
					current_length = ns_length_get(nuller);
					current_position = nuller;
					*(result_position++) = '\"';
					++total;
				}
				else {
					current_length = st_length_get(literal);
					current_position = st_char_get(literal);
				}
				// Copy
				for (j = 0; j < current_length; j++) {
					*(result_position++) = *(current_position++);
					if (++total > length) {
						log_error("We've exceeded the space in the allocated buffer. This should never happen.");
						if (literal != NULL) {
							st_free(literal);
						}
						st_free(result);
						va_end(args);
						return NULL;
					}
				}
				// Free the literal holder, if it was used. Otherwise close quotes.
				if (literal != NULL) {
					st_free(literal);
				}
				else {
					*(result_position++) = '\"';
					++total;
				}
			}
			else {
				*(result_position++) = 'N';
				*(result_position++) = 'I';
				*(result_position++) = 'L';
				total += 3;
			}
		}
		else if (*(format + i) == 'p') {
			placer = va_arg(args, placer_t);
			if (!pl_empty(placer)) {
				// Check whether we need to make this a literal.
				if ((literal = imap_build_array_isliteral(placer)) == NULL) {
					current_length = pl_length_get(placer);
					current_position = pl_data_get(placer);
					*(result_position++) = '\"';
					++total;
				}
				else {
					current_length = st_length_get(literal);
					current_position = st_char_get(literal);
				}
				// Copy
				for (j = 0; j < current_length; j++) {
					*(result_position++) = *(current_position++);
					if (++total > length) {
						log_error("We've exceeded the space in the allocated buffer. This should never happen.");
						if (literal != NULL) {
							st_free(literal);
						}
						st_free(result);
						va_end(args);
						return NULL;
					}
				}
				// Free the literal holder, if it was used. Otherwise close quotes.
				if (literal != NULL) {
					st_free(literal);
				}
				else {
					*(result_position++) = '\"';
					++total;
				}
			}
			else {
				*(result_position++) = 'N';
				*(result_position++) = 'I';
				*(result_position++) = 'L';
				total += 3;
			}
		}
		else {
			log_error("Invalid format. This error should have been caught above.");
			st_free(result);
			va_end(args);
			return NULL;
		}
	}
	va_end(args);

	// Close with a paren.
	*(result_position++) = ')';
	++total;

	st_length_set(result, total);
	return result;

}
