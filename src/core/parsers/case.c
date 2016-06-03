
/**
 * @file /magma/core/parsers/case.c
 *
 * @brief	A collection of functions used for manipulating the capitalization of characters.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Return the uppercase representation of a character.
 * @param	c	the character to be transformed.
 * @return	the input character, in uppercase.
 */
uchr_t upper_chr(uchr_t c) {
	if (c >= 'a' && c <= 'z') c -= 32;
	return c;
}

/**
 * @brief	Return the lowercase representation of a character.
 * @param	c	the character to be transformed.
 * @return	the input character, in lowercase.
 */
uchr_t lower_chr(uchr_t c) {
	if (c >= 'A' && c <= 'Z') c += 32;
	return c;
}

/**
 * @brief	Transform a managed string (in-place) into uppercase.
 * @param	s	the managed string to be modified.
 * @return	NULL on error, or a pointer to the input managed string, in uppercase.
 */
stringer_t * upper_st(stringer_t *s) {

	size_t len;
	uchr_t *ptr;

	if (st_empty_out(s, &ptr, &len)) {
		log_pedantic("Passed in a NULL pointer or zero length string.");
		return NULL;
	}

	for (size_t i = 0; i < len; i++) {
		if (*ptr >= 'a' && *ptr <= 'z') *ptr -= 32;
		ptr++;
	}

	return s;
}

/**
 * @brief	Transform a managed string (in-place) into lowercase.
 * @param	s	the managed string to be modified.
 * @return	NULL on error, or a pointer to the input managed string, in lowercase.
 */
stringer_t * lower_st(stringer_t *s) {

	size_t len;
	uchr_t *ptr;

	if (st_empty_out(s, &ptr, &len)) {
		log_pedantic("Passed in a NULL pointer or zero length string.");
		return NULL;
	}

	for (size_t i = 0; i < len; i++) {
		if (*ptr >= 'A' && *ptr <= 'Z') *ptr += 32;
		ptr++;
	}

	return s;
}
