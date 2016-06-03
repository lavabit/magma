
/**
 * @file /magma/core/compare/equal.c
 *
 * @brief	Functions to check for string equality.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"


/**
 * @brief	Perform a case-sensitive comparison of two memory blocks.
 * @param	a	a pointer to the first buffer in memory to be compared.
 * @param	b	a pointer to the second buffer in memory to be compared.
 * @param	len	the length, in bytes, of the comparison that is to take place.
 * @return	-1 if a < b, 1 if b < a, or 0 if the two memory blocks are equal.
 */
int_t mm_cmp_cs_eq(void *a, void *b, size_t len) {
	bool_t ae, be;
	int_t result = 0;
	uchr_t *aptr = a, *bptr = b;

	// Setup
	ae = mm_empty(a, len);
	be = mm_empty(b, len);

	// Empty string checks.
	if (ae && be)
		return 0;
	else if (ae)
		return -1;
	else if (be) return 1;

	// Break on the first non matching byte.
	for (size_t i = 0; result == 0 && i < len; i++) {
		if (*aptr < *bptr) result = -1;
		else if (*aptr > *bptr) result = 1;
		aptr++;
		bptr++;
	}

	return result;
}


/**
 * @brief	Perform a case-insensitive comparison of two memory blocks.
 * @param	a	a pointer to the first buffer in memory to be compared.
 * @param	b	a pointer to the second buffer in memory to be compared.
 * @param	len	the length, in bytes, of the comparison that is to take place.
 * @return	-1 if a < b, 1 if b < a, or 0 if the two memory blocks are equal.
 */
int_t mm_cmp_ci_eq(void *a, void *b, size_t len) {
	bool_t ae, be;
	int_t result = 0;
	uchr_t *aptr = a, *bptr = b;

	// Setup
	ae = mm_empty(a, len);
	be = mm_empty(b, len);

	// Empty string checks.
	if (ae && be)
		return 0;
	else if (ae)
		return -1;
	else if (be) return 1;

	// Break on the first non matching byte.
	for (size_t i = 0; result == 0 && i < len; i++) {
		if (lower_chr(*aptr) < lower_chr(*bptr)) result = -1;
		else if (lower_chr(*aptr) > lower_chr(*bptr)) result = 1;
		aptr++;
		bptr++;
	}

	return result;
}

/**
 * @brief	Perform a case-sensitive comparison of two managed strings.
 * @param	a	the first managed string to be compared.
 * @param	b	the second managed string to be compared.
 * @return	-1 if a < b, 1 if b < a, or 0 if the two memory blocks are equal.
 */
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
		else if (*aptr > *bptr) result = 1;
		aptr++;
		bptr++;
	}

	// If the strings match, then the longer string is greater
	if (result == 0 && alen < blen) result =  -1;
	else if (result == 0 && alen > blen) result = 1;
	return result;
}

/**
 * @brief	Perform a case-insensitive comparison of two managed strings.
 * @param	a	the first managed string to be compared.
 * @param	b	the second managed string to be compared.
 * @return	-1 if a < b, 1 if b < a, or 0 if the two memory blocks are equal.
 */
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
		if (lower_chr(*aptr) < lower_chr(*bptr)) result = -1;
		else if (lower_chr(*aptr) > lower_chr(*bptr)) result = 1;
		aptr++;
		bptr++;
	}

	// If the strings match, then the longer string is greater
	if (result == 0 && alen < blen) result =  -1;
	else if (result == 0 && alen > blen) result = 1;
	return result;
}

