
/**
 * @file /magma/core/compare/search.c
 *
 * @brief	String search functions.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Search one managed string for an occurrence of another in a case-sensitive manner, and save its location.
 * @param	haystack	the managed string to be searched.
 * @param	needle		the managed string to be found.
 * @param	location	if not NULL, a pointer to store the index of needle if found, or 0 on no match.
 * @return	true if the string is found or false otherwise.
 */
bool_t st_search_cs(stringer_t *haystack, stringer_t *needle, size_t *location) {

	uchr_t *h, *n;
	bool_t result = false;
	size_t i, j, hlen, nlen;

	if (st_empty_out(haystack, &h, &hlen) || st_empty_out(needle, &n, &nlen)) {
		log_pedantic("Passed an empty string.");
		return false;
	} else if (nlen > hlen) {
		return false;
	}

	// If a location was provided for storing the position of needle, store reset it to zero in case needle isn't found.
	if (location) {
		*location = 0;
	}

	// The needle will never be found if it's longer than the haystack.
	if (nlen > hlen) {
		return false;
	}

	for (i = 0; i <= hlen - nlen && !result; i++) {

		for (j = 0; j < nlen && (*(h + j) == *(n + j)); j++);

		// In theory, j won't ever equal nlen if a non-matching character is found.
		if (j == nlen && location) {
			result = true;
			*location = i;
		}
		else if (j == nlen) {
			result = true;
		}
		else {
			h++;
		}

	}

	return result;
}

/**
 * @brief	Search one managed string for an occurrence of another in a case-insensitive manner, and save its location.
 * @param	haystack	the managed string to be searched.
 * @param	needle		the managed string to be found.
 * @param	location	if not NULL, a pointer to store the index of needle if found, or 0 on no match.
 * @return	true if the string is found or false otherwise.
 */
bool_t st_search_ci(stringer_t *haystack, stringer_t *needle, size_t *location) {

	uchr_t *h, *n;
	bool_t result = false;
	size_t i, j, hlen, nlen;

	if (st_empty_out(haystack, &h, &hlen) || st_empty_out(needle, &n, &nlen)) {
		log_pedantic("Passed an empty string.");
		return false;
	}

	// If a location was provided for storing the position of needle, store reset it to zero in case needle isn't found.
	if (location) {
		*location = 0;
	}

	// The needle will never be found if it's longer than the haystack.
	if (nlen > hlen) {
		return false;
	}

	for (i = 0; i <= hlen - nlen && !result; i++) {

		for (j = 0; j < nlen && (lower_chr(*(h + j)) == lower_chr(*(n + j))); j++);

		// In theory, j won't ever equal nlen if a non-matching character is found.
		if (j == nlen && location) {
			result = true;
			*location = i;
		}
		else if (j == nlen) {
			result = true;
		}
		else {
			h++;
		}

	}

	return result;
}

/**
 * @brief	Search for a character inside of a managed string, and save its location.
 * @param	haystack	the managed string to be searched.
 * @param	needle		the character to be found in the string.
 * @param	location	if not NULL, a pointer to store the index of needle if found, or 0 on no match.
 * @return	true if the specified character was found or false otherwise.
 */
bool_t st_search_chr(stringer_t *haystack, chr_t needle, size_t *location) {

	uchr_t *h;
	size_t hlen;

	if (st_empty_out(haystack, &h, &hlen)) {
		log_pedantic("Passed an empty string.");
		return false;
	}

	// If a location was provided for storing the position of needle, store reset it to zero in case needle isn't found.
	if (location) {
		*location = 0;
	}

	for (size_t i = 0; i < hlen; i++) {

		if (h[i] == needle) {

			if (location) {
				*location = i;
			}

			return true;
		}

	}

	return false;
}
