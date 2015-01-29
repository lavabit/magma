
/**
 * @file /magma/core/compare/starts.c
 *
 * @brief	Functions used to compare the starts of strings with other strings.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Perform a case-sensitive check to see if one string starts with another.
 * @param	s		the managed string to have its beginning characters examined.
 * @param	ends	the managed string to be compared against the beginning of s.
 * @return	-1 if s < starts, 1 if starts < s or 0 if s begins with starts.
 */
int_t st_cmp_cs_starts(stringer_t *s, stringer_t *starts) {

	bool_t se, starte;
	int_t result = 0;
	uchr_t *sptr, *startptr;
	size_t slen, startlen, check;

	// Setup.
	se = st_empty_out(s, &sptr, &slen);
	starte = st_empty_out(starts, &startptr, &startlen);

	// Empty string checks.
	if (se && starte) return 0;
	else if (se) return -1;
	else if (starte) return 1;

	// Calculate how many bytes to compare.
	check = (slen <= startlen ? slen : startlen);

	// Break on the first non matching byte.
	for (size_t i = 0; result == 0 && i < check; i++) {
		if (*sptr < *startptr) result = -1;
		else if (*sptr > *startptr) result = 1;
		sptr++;
		startptr++;
	}

	// If the string length is equal/greater and result is still set to 0, we have a match.
	if (result == 0 && slen < startlen) result =  -1;
	return result;
}

/**
 * @brief	Perform a case-insensitive check to see if one string starts with another.
 * @param	s		the managed string to have its beginning characters examined.
 * @param	starts	the managed string to be compared against the beginning of s.
 * @return	-1 if s < starts, 1 if starts < s or 0 if s begins with starts.
 */
int_t st_cmp_ci_starts(stringer_t *s, stringer_t *starts) {

	bool_t se, starte;
	int_t result = 0;
	uchr_t *sptr, *startptr;
	size_t slen, startlen, check;

	// Setup.
	se = st_empty_out(s, &sptr, &slen);
	starte = st_empty_out(starts, &startptr, &startlen);

	// Empty string checks.
	if (se && starte) return 0;
	else if (se) return -1;
	else if (starte) return 1;

	// Calculate how many bytes to compare.
	check = (slen <= startlen ? slen : startlen);

	// Break on the first non matching byte.
	for (size_t i = 0; result == 0 && i < check; i++) {
		if (lower_chr(*sptr) < lower_chr(*startptr)) result = -1;
		else if (lower_chr(*sptr) > lower_chr(*startptr)) result = 1;
		sptr++;
		startptr++;
	}

	// If the string length is equal/greater and result is still set to 0, we have a match.
	if (result == 0 && slen < startlen) result =  -1;
	return result;
}
