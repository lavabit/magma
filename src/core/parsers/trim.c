
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

static bool_t is_trim(char c) {
	return c == ' ' || c == '\n' || c == '\r' || c == '\t' || c == '\v';
}

// Removes any starting/ending whitespace from a stringer. Since the trimmed string cannot ever become longer, it is returned inside the existing buffer.
void st_trim(stringer_t *string) {

	chr_t *start, *end;

	start = st_char_get(string);
	end = start + st_length_get(string);

	while (start != end && is_trim(*start)) {
			start++;
		}

	while (end != start && is_trim(end[-1])) {
			end--;
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

	chr_t *start, *end;

	start = pl_char_get(place);
	end = start + pl_length_get(place);

	while (start != end && is_trim(*start)) {
			start++;
		}
	while (end != start && is_trim(end[-1])) {
			end--;
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

	chr_t *start, *end;

	start = pl_char_get(place);
	end = start + pl_length_get(place);

	while (start != end && is_trim(*start)) {
			start++;
		}

	if (start == end)
		return pl_null();

	return pl_init(start, end - start);
}

placer_t pl_trim_end(placer_t place) {

	chr_t *start, *end;

	start = pl_char_get(place);
	end = start + pl_length_get(place);

	while (end != start && is_trim(end[-1])) {
			end--;
		}

	if (start == end)
		return pl_null();

	return pl_init(start, end - start);
}
