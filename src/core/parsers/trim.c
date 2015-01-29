
/**
 * @file /magma/core/parsers/trim.c
 *
 * @brief	Functions used to trim whitespace from strings.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

#define CHARS_TO_TRIM ' ', '\n', '\r', '\t', '\v'

// Removes any starting/ending whitespace from a stringer. Since only a subtraction is possible, the trimmed string is returned inside the existing buffer.
void st_trim(stringer_t *string) {

	int_t trim = 1;
	size_t trim_chars_len;
	chr_t *start, *end, trim_chars[] = { CHARS_TO_TRIM };

	start = st_char_get(string);
	trim_chars_len = sizeof(trim_chars);
	end = st_char_get(string) + st_length_get(string);

	while (trim == 1 && start != end) {

		trim = 0;
		for (size_t i = 0; trim == 0 && i < trim_chars_len; i++) {
			if (*start == trim_chars[i])
				trim = 1;
		}

		if (trim) {
			start++;
		}
	}

	trim = 1;
	while (trim == 1 && start != end) {

		trim = 0;
		for (size_t i = 0; trim == 0 && i < trim_chars_len; i++) {
			if (*(end - 1) == trim_chars[i])
				trim = 1;
		}

		if (trim) {
			end--;
		}
	}

	if (start == end) {
		st_length_set(string, 0);
	}
	else if (end - start != st_length_get(string)) {
		mm_move(st_char_get(string), start, end - start);
		st_length_set(string, end - start);
		mm_wipe(st_char_get(string) + st_length_get(string), st_avail_get(string) - st_length_get(string));
	}

	return;
}

placer_t pl_trim(placer_t place) {

	bool_t trim = true;
	size_t trim_chars_len;
	chr_t *start, *end, trim_chars[] = { CHARS_TO_TRIM };

	start = pl_char_get(place);
	trim_chars_len = sizeof(trim_chars);
	end = pl_char_get(place) + pl_length_get(place);

	while (trim == true && start != end) {

		trim = false;
		for (int_t i = 0; trim == false && i < trim_chars_len; i++) {
			if (*start == trim_chars[i])
				trim = true;
		}

		if (trim) {
			start++;
		}
	}

	trim = true;
	while (trim == true && start != end) {

		trim = false;
		for (int_t i = 0; trim == false && i < trim_chars_len; i++) {
			if (*(end - 1) == trim_chars[i])
				trim = true;
		}

		if (trim) {
			end--;
		}
	}

	if (start == end)
		return pl_null();

	return pl_init(start, end - start);
}

/**
 * @brief	Trim the leading whitespace from a placer.
 * @param	place	a placer containing the string to have its leading whitespace trimmed.
 * @return	a placer pointing to the trimmed value inside the originally specified input string.
 */
placer_t pl_trim_start(placer_t place) {

	bool_t trim = true;
	size_t trim_chars_len;
	chr_t *start, *end, trim_chars[] = { CHARS_TO_TRIM };

	start = pl_char_get(place);
	trim_chars_len = sizeof(trim_chars);
	end = pl_char_get(place) + pl_length_get(place);

	while (trim == true && start != end) {

		trim = false;
		for (int_t i = 0; trim == false && i < trim_chars_len; i++) {
			if (*start == trim_chars[i])
				trim = true;
		}

		if (trim) {
			start++;
		}
	}

	if (start == end)
		return pl_null();

	return pl_init(start, end - start);
}

placer_t pl_trim_end(placer_t place) {

	bool_t trim = true;
	size_t trim_chars_len;
	chr_t *start, *end, trim_chars[] = { CHARS_TO_TRIM };

	start = pl_char_get(place);
	trim_chars_len = sizeof(trim_chars);
	end = pl_char_get(place) + pl_length_get(place);

	while (trim == true && start != end) {

		trim = false;
		for (int_t i = 0; trim == false && i < trim_chars_len; i++) {
			if (*(end - 1) == trim_chars[i])
				trim = true;
		}

		if (trim) {
			end--;
		}
	}

	if (start == end)
		return pl_null();

	return pl_init(start, end - start);
}
