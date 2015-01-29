
/**
 * @file /magma/core/classify/ascii.c
 *
 * @brief	Functions used to classify ASCII characters.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Determine whether a specified character is an ASCII character.
 * @param	c	the character to be verified.
 * @return	true if the character passed the test, or false otherwise.
 */
bool_t chr_ascii(uchr_t c) {

	if (c < 128) {
		return true;
	}

	return false;
}

/**
 * @brief	Determine whether a specified character is a printable character.
 * @param	c	the character to be verified.
 * @return	true if the character passed the test, or false otherwise.
 */
bool_t chr_printable(uchr_t c) {

	if (c >= ' ' && c <= '~') {
		return true;
	}

	return false;
}

/**
 * @brief	Determine whether a specified character is alpha-numeric.
 * @param	c	the character to be verified.
 * @return	true if the character passed the test, or false otherwise.
 */

bool_t chr_alphanumeric(uchr_t c) {

	if ((c >= 'A' && c <= 'Z') ||	(c >= 'a' && c <= 'z') ||	(c >= '0' && c <= '9')) {
		return true;
	}

	return false;
}
/**
 * @brief	Determine whether a specified character is a lowercase character.
 * @param	c	the character to be verified.
 * @return	true if the character passed the test, or false otherwise.
 */
bool_t chr_lower(uchr_t c) {

	if (c >= 'a' && c <= 'z') {
		return true;
	}

	return false;
}

/**
 * @brief	Determine whether a specified character is an uppercase character.
 * @param	c	the character to be verified.
 * @return	true if the character passed the test, or false otherwise.
 */
bool_t chr_upper(uchr_t c) {

	if (c >= 'A' && c <= 'Z') {
		return true;
	}

	return false;
}

/**
 * @brief	Determine whether a specified character is a numeric character.
 * @param	c	the character to be verified.
 * @return	true if the character passed the test, or false otherwise.
 */
bool_t chr_numeric(uchr_t c) {

	if (c >= '0' && c <= '9') {
		return true;
	}

	return false;
}

/**
 * @brief	Determine whether a specified character is a punctuation character.
 * @param	c	the character to be verified.
 * @return	true if the character passed the test, or false otherwise.
 */
bool_t chr_punctuation(uchr_t c) {

	if ((c >= '!' && c <= '/') || (c >= ':' && c <= '@') || (c >= '[' && c <= '`') || (c >= '{' && c <= '~')) {
		return true;
	}

	return false;
}

/**
 * @brief	Determine whether a specified character is a blank character (space or tab).
 * @param	c	the character to be verified.
 * @return	true if the character passed the test, or false otherwise.
 */
bool_t chr_blank(uchr_t c) {

	if (c == ' ' || c == '\t') {
		return true;
	}

	return false;
}

/**
 * @brief	Determine whether a specified character is a whitespace character (blank or \r\n\f\v).
 * @param	c	the character to be verified.
 * @return	true if the character passed the test, or false otherwise.
 */
bool_t chr_whitespace(uchr_t c) {

	if (c == ' ' || c == '\t' || c == '\v' || c == '\f' || c == '\r' || c == '\n') {
		return true;
	}

	return false;
}

/**
 * @brief	Determine whether a character belongs to a custom set of values.
 * @param	c	the character to be verified.
 * @param	chrs	a pointer to a buffer containing the family of valid character values.
 * @param	chrlen	the number of characters in the testing set.
 * @return	true if the character passed the test, or false otherwise.
 *
 */
bool_t chr_is_class(uchr_t c, uchr_t *chrs, size_t chrlen) {

	for (size_t i = 0; i < chrlen; i++) {

		if (c == chrs[i]) {
			return true;
		}

	}

	return false;
}
