/**
 * @file /stringer/stringer-compare.c
 *
 * @brief A collection of functions used handle common bit manipulation tasks.
 *
 * $Author: Ladar Levison $
 * $Date: 2010/12/12 23:59:04 $
 * $Revision: a46af76362b976a414117a5bfee9dfc2e9fd38b2 $
 *
 */

#include "stringer.h"

int_t st_cmp_cs_eq(stringer_t *a, stringer_t *b) {

	bool_t ae, be;
	int_t result = 0;
	uchr_t *aptr, *bptr;
	size_t alen, blen, check;

	// Setup.
	ae = st_empty_out(a, &aptr, &alen);
	be = st_empty_out(b, &bptr, &blen);

	// Empty string checks.
	if (ae && be) return 0;
	else if (ae) return -1;
	else if (be) return 1;

	// Calculate how many bytes to compare.
	check = (alen <= blen ? alen : blen);

	// Break on the first non matching byte.
	for (size_t i = 0; result == 0 && i < check; i++) {
		if (*aptr < *bptr) result = -1;
		else if (*bptr > *aptr) result = 1;
		aptr++;
		bptr++;
	}

	// If the strings match, then the longer string is greater
	if (result == 0 && alen < blen) result =  -1;
	else if (result == 0 && alen > blen) result = 1;
	return result;
}

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
		else if (*startptr > *sptr) result = 1;
		sptr++;
		startptr++;
	}

	// If the string length is equal/greater and result is still set to 0, we have a match.
	if (result == 0 && slen < startlen) result =  -1;
	return result;
}

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
	sptr += slen;
	endptr += endlen;

	// Calculate how many bytes to compare.
	check = (slen <= endlen ? slen : endlen);

	// Break on the first non matching byte.
	for (size_t i = 0; result == 0 && i < check; i++) {
		if (*sptr < *endptr) result = -1;
		else if (*endptr > *sptr) result = 1;
		sptr--;
		endptr--;
	}

	// If the string length is equal/greater and result is still set to 0, we have a match.
	if (result == 0 && slen < endlen) result =  -1;
	return result;
}

int_t st_cmp_ci_eq(stringer_t *a, stringer_t *b) {

	bool_t ae, be;
	int_t result = 0;
	uchr_t *aptr, *bptr;
	size_t alen, blen, check;

	// Setup.
	ae = st_empty_out(a, &aptr, &alen);
	be = st_empty_out(b, &bptr, &blen);

	// Empty string checks.
	if (ae && be) return 0;
	else if (ae) return -1;
	else if (be) return 1;

	// Calculate how many bytes to compare.
	check = (alen <= blen ? alen : blen);

	// Break on the first non matching byte.
	for (size_t i = 0; result == 0 && i < check; i++) {
		if (c_lowercase(*aptr) < c_lowercase(*bptr)) result = -1;
		else if (c_lowercase(*bptr) > c_lowercase(*aptr)) result = 1;
		aptr++;
		bptr++;
	}

	// If the strings match, then the longer string is greater
	if (result == 0 && alen < blen) result =  -1;
	else if (result == 0 && alen > blen) result = 1;
	return result;
}

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
		if (c_lowercase(*sptr) < c_lowercase(*startptr)) result = -1;
		else if (c_lowercase(*startptr) > c_lowercase(*sptr)) result = 1;
		sptr++;
		startptr++;
	}

	// If the string length is equal/greater and result is still set to 0, we have a match.
	if (result == 0 && slen < startlen) result =  -1;
	return result;
}

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
	sptr += slen;
	endptr += endlen;

	// Calculate how many bytes to compare.
	check = (slen <= endlen ? slen : endlen);

	// Break on the first non matching byte.
	for (size_t i = 0; result == 0 && i < check; i++) {
		if (c_lowercase(*sptr) < c_lowercase(*endptr)) result = -1;
		else if (c_lowercase(*endptr) > c_lowercase(*sptr)) result = 1;
		sptr--;
		endptr--;
	}

	// If the string length is equal/greater and result is still set to 0, we have a match.
	if (result == 0 && slen < endlen) result =  -1;
	return result;
}

