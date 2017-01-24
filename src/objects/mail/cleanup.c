
/**
 * @file /magma/objects/mail/cleanup.c
 *
 * @brief	Functions for cleaning up mail messages.
 */

#include "magma.h"

/**
 * @brief	Destroy a mail header.
 * @param	header	a managed string containing the mail header to be freed.
 * @return	This function returns nov alue.
 */
void mail_destroy_header(stringer_t *header) {

	st_cleanup(header);
	return;
}

/**
 * @brief	Clean up the body of a message read in via smtp, enforcing magma.smtp.wrap_line_length.
 * @note	This function fixes broken line separators by making sure each \r is followed by \n and vice versa.
 * 			All Return-Path: header lines are also removed.
 * 			New lines are begun whenever the current length of any line reaches the configuration value set in magma.smtp.wrap_line_length.
 * 			The trailing dot at the end of the smtp DATA command is also stripped.
 * @note	If the original message ends with \r, it will have \n appended to it.
 * @param	message		a pointer to a managed string that contains the message input, and will also store the cleaned output on success.
 * @return	true on success or false on failure.
 */
bool_t mail_message_cleanup(stringer_t **message) {

	chr_t *new, *orig;
	stringer_t *output;
	size_t increment, added, length, used = 0;
	int_t next = 1, skip = 0, line = 0, header = 1;

	if (!message) {
		log_pedantic("Invalid message passed in.");
		return false;
	}

	// The first iteration is so we can calculate how much storage we'll need for the new message.
	orig = st_char_get(*message);
	added = length = st_length_get(*message);

	// Check for leading breaks.
	if (*orig == '\n') {
		added++;
	}

	for (increment = 0; increment < length; increment++) {

		// Detect improper carriage returns
		if (*orig == '\r' && (length - increment) > 1 && *(orig + 1) != '\n') {
			added++;
		}
		else if (*orig == '\n' && increment != 0 && *(orig - 1) != '\r') {
			added++;
		}

		// For maximum line lengths.
		if (*orig == '\r' || *orig == '\n') {
			line = 0;
		}
		else {
			line++;
		}

		if (line == magma.smtp.wrap_line_length) {
			added += 3;
			line = 0;
		}

		orig++;
	}

	// Trailing carriage returns.
	if (*orig == '\r') {
		added++;
	}

	// Allocate a new stringer for the cleaned up message.
	if (!(output = st_alloc_opts(MAPPED_T | JOINTED | HEAP,  added))) {
		log_pedantic("Unable to allocate %zu bytes for a new stringer.", added);
		return false;
	}

	// Now go through and copy.
	line = 0;
	new = st_char_get(output);
	orig = st_char_get(*message);

	// Check for leading line breaks.
	if (*orig == '\n') {
		*new++ = '\r';
		used++;
	}

	for (increment = 0; increment < length; increment++) {

		if (header != 3) {

			// Logic for detecting the end of the header.
			if (header == 0 && *orig == '\n') {
				header++;
			}
			else if (header == 1 && *orig == '\n') {
				header += 2;
			}
			else if (header == 1 && *orig == '\r') {
				header++;
			}
			else if (header == 2 && *orig == '\n') {
				header++;
			}
			else if (header != 0) {
				header = 0;
			}

			// Look for the return path and dotstuffs.
			if (next == 1 && *orig != '\n') {

				if (length - increment >= 12 && mm_cmp_ci_eq(orig, "Return-Path:", 12) == 0) {
					skip = 1;
				}
				else if (length - increment >= 2 && *orig == '.' && (*(orig + 1) == '\r' || *(orig + 1) == '\n')) {
					skip = 1;
				}
				else if (length - increment >= 2 && *orig == '.' && *(orig + 1) == '.') {
					skip = 2;
				}
				else if (skip != 0) {
					skip = 0;
				}

				next = 0;
			}

			// So we check the start of the next line for the return path.
			if (next == 0 && (*orig == '\n' || *orig == '\r')) {
				next = 1;
			}
		}
		else {
			// Now were just looking for dotstuffs.
			if (next == 1 && *orig != '\n') {

				if (length - increment >= 2 && *orig == '.' && (*(orig + 1) == '\r' || *(orig + 1) == '\n')) {
					skip = 1;
				}
				else if (length - increment >= 2 && *orig == '.' && *(orig + 1) == '.') {
					skip = 2;
				}
				else if (skip != 0) {
					skip = 0;
				}

				next = 0;
			}

			// So we check the start of the next line for the return path.
			if (next == 0 && (*orig == '\n' || *orig == '\r')) {
				next = 1;
			}

		}

		// Copy into the new buffer, replacing invalid sequences as we go, and skipping the rejected lines.
		if (skip == 0) {

			if (*orig == '\r' && (length - increment) > 1 && *(orig + 1) != '\n') {
				*new++ = '\r';
				*new++ = '\n';
				used += 2;
			}
			else if (*orig == '\n' && increment != 0 && *(orig - 1) != '\r') {
				*new++ = '\r';
				*new++ = '\n';
				used += 2;
			}
			else {
				*new++ = *orig;
				used++;
			}

			// For maximum line lengths.
			if (*orig == '\r' || *orig == '\n') {
				line = 0;
			}
			else {
				line++;
			}

			/// LOW: Splitting lines, particularly headers causes more problems then it fixes, so this code is currently
			/// 	disabled. At some point we should survey other mail systems and see how they handle long lines.
			/*
			// Detect when we've reached the maximum line length and insert a new line. If we are still iterating over
			// the header, split lines include a leading tab to indicate a continuation of the current field.
			if (header == 3 && line == magma.smtp.wrap_line_length) {
				*new++ = '\r';
				*new++ = '\n';
				used += 2;
				line = 0;
			}
			else if ((header != 3) && (line == magma.smtp.wrap_line_length)) {
				*new++ = '\r';
				*new++ = '\n';
				*new++ = '\t';
				used += 3;
				line = 0;
			}*/

		}
		else if (skip == 2) {
			skip = 0;
		}

		orig++;
	}

	// If the buffer ends with a carriage return add a line break.
	if (*orig == '\r') {
		*new++ = '\n';
		used++;
	}

	// Setup the output.
	st_length_set(output, used);
	st_free(*message);
	*message = output;

	return true;
}
