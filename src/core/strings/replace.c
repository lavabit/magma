
/**
 * @file /magma/core/strings/replace.c
 *
 * @brief	Functions used for string replacement.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/*
 * @brief	Replace all instances of a substring inside another string.
 * @param	target			a pointer to the address of a managed string containing the haystack string, which will be overwritten with the
 * 			address of the transformed string on success.
 * @param	pattern			a managed string containing the search pattern.
 * @param	replacement		a managed string containing the data that will replace all found instances of the search pattern.
 * @return	-1 on error, or the number of times the pattern string was found in the input string.
 */
 // LOW: Function could use some serious cleanup; NEED TO TEST THIS IN UNIT TESTS
int_t st_replace(stringer_t **target, stringer_t *pattern, stringer_t *replacement) {

	stringer_t *output;
	uchr_t *tptr, *optr, *rptr, *pptr;
	size_t hits = 0, tlen, plen, rlen, olen;

	// replacement can be blank but it can't be null
	if (!target || st_empty_out(*target, &tptr, &tlen) || st_empty_out(pattern, &pptr, &plen) ||
			(st_empty_out(replacement, &rptr, &rlen) && !st_char_get(replacement))) {
		log_pedantic("Sanity check failed. Passed a NULL pointer.");
		return -1;
	}

	// Check to make sure the target is big enough to hold the pattern.
	if (tlen < plen) {
//		log_pedantic("The target isn't long enough to contain the pattern.");
		return 0;
	}

	// Increment through the entire target and find out how many times the pattern is present.
	for (size_t i = 0; i <= (tlen - plen); i++) {
		if (!st_cmp_cs_starts(PLACER(tptr++, tlen - i), pattern)) {
			hits++;
			i += plen - 1;
			tptr += plen - 1;
		}
	}

	//  Did we get any hits? Or if the output length would be zero, return.
	// QUESTION: Should 2nd part of conditional ever be necessary? tlen - (plen * hits) can't be less than zero,
	//           and hits has to be positive. So the expression will always evaluate positive.
	// QUESTION: Shouldn't we allow the output length to be zero?
	/*if (!hits || !(olen = tlen - (plen * hits) + (rlen * hits))) {
		return hits;
	}*/

	if (!hits) {
		return 0;
	}

	olen = tlen - (plen * hits) + (rlen * hits);

	// If our new string is now empty we truncate the original target and return it.
	if (!olen) {
		*((char *)(st_data_get(*target))) = 0;
		st_length_set(*target, 0);
		return hits;
	}

	// Allocate a new stringer.
	if (!(output = st_alloc(olen))) {
		log_pedantic("Could not allocate %zu bytes for the new string.", olen);
		// QUESTION: -3?
		return -3;
	}

	// Setup.
	tptr = st_data_get(*target);
	optr = st_data_get(output);

	// Increment through the entire target and copy the bytes.
	for (size_t i = 0; i <= tlen; i++) {
		if (i <= (tlen - plen) && !st_cmp_cs_starts(PLACER(tptr, tlen - i), pattern)) {
			i += plen - 1;
			tptr += plen;
			for (size_t j = 0; j < rlen; j++) {
				*optr++ = *(rptr + j);
			}
		}
		else {
			*optr++ = *tptr++;
		}
	}

	if (st_valid_free(*((uint32_t *)(*target)))) {
		st_free(*target);
	}

	st_length_set(output, olen);
	*target = output;
	return hits;
}

/**
 * @brief	Replace all instances of one character in a managed string with another.
 * @param	target		the input string which will be transformed by the character replacement.
 * @param	pattern		the character to be searched and replaced in the target string.
 * @param	replacement	the character to be substituted for the pattern character in the target string.
 * @return	a pointer to the target managed string.
 */
stringer_t * st_swap(stringer_t *target, uchr_t pattern, uchr_t replacement) {

	size_t tlen;
	uchr_t *tptr;

	if (st_empty_out(target, &tptr, &tlen)) {
		log_pedantic("Sanity check failed. Passed a NULL pointer.");
		return target;
	}

	log_check(pattern == replacement);

	// Increment through and replace the pattern.
	for (size_t i = 0; i < tlen; i++) {
		if (*tptr == pattern) *tptr = replacement;
		tptr++;
	}

	return target;
}
