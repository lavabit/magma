
/**
 * @file /magma/core/compare/ends.c
 *
 * @brief	Functions used to compare the ends of strings with other strings.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Perform a case-sensitive check to see if one string ends in another, comparing backwards.
 * @note	The value of the result depends on a backwards character comparison pattern, not a forward one.
 * @param	s		the managed string to have its ending characters examined.
 * @param	ends	the managed string to be compared against the end of s.
 * @return	-1 if s < ends, 1 if ends < s or 0 if s ends with the content of ends.
 */
int_t st_cmp_cs_ends(stringer_t *s, stringer_t *ends) {

	bool_t se, ende;
	int_t result = 0;
	uchr_t *sptr, *endptr;
	size_t slen, endlen, check;

	// Setup.
	se = st_empty_out(s, &sptr, &slen);
	ende = st_empty_out(ends, &endptr, &endlen);

	// Empty string checks.
	if (se && ende) return 0;
	else if (se) return -1;
	else if (ende) return 1;

	// Were comparing from the end of the buffers, so adjust the pointers accordingly.
	sptr += (slen - 1);
	endptr += (endlen - 1);

	// Calculate how many bytes to compare.
	check = (slen <= endlen ? slen : endlen);

	// Break on the first non matching byte.
	for (size_t i = 0; result == 0 && i < check; i++) {
		if (*sptr < *endptr) result = -1;
		else if (*sptr > *endptr) result = 1;
		sptr--;
		endptr--;
	}

	// If the string length is equal/greater and result is still set to 0, we have a match.
	if (result == 0 && slen < endlen) result =  -1;
	return result;
}

/**
 * @brief	Perform a case-insensitive check to see if one string ends in another, comparing backwards.
 * @note	The value of the result depends on a backwards character comparison pattern, not a forward one.
 * @param	s		the managed string to have its ending characters examined.
 * @param	ends	the managed string to be compared against the end of s.
 * @return	-1 if s < ends, 1 if ends < s or 0 if s ends with the content of ends.
 */
int_t st_cmp_ci_ends(stringer_t *s, stringer_t *ends) {

	bool_t se, ende;
	int_t result = 0;
	uchr_t *sptr, *endptr;
	size_t slen, endlen, check;

	// Setup.
	se = st_empty_out(s, &sptr, &slen);
	ende = st_empty_out(ends, &endptr, &endlen);

	// Empty string checks.
	if (se && ende) return 0;
	else if (se) return -1;
	else if (ende) return 1;

	// Were comparing from the end of the buffers, so adjust the pointers accordingly.
	sptr += (slen - 1);
	endptr += (endlen - 1);

	// Calculate how many bytes to compare.
	check = (slen <= endlen ? slen : endlen);

	// Break on the first non matching byte.
	for (size_t i = 0; result == 0 && i < check; i++) {
		if (lower_chr(*sptr) < lower_chr(*endptr)) result = -1;
		else if (lower_chr(*sptr) > lower_chr(*endptr)) result = 1;
		sptr--;
		endptr--;
	}

	// If the string length is equal/greater and result is still set to 0, we have a match.
	if (result == 0 && slen < endlen) result =  -1;
	return result;
}
