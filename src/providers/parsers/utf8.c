
/**
 * @file /magma/providers/parsers/utf.c
 *
 * @brief	The interface for UTF-8 parsing.
 * 		CURRENTLY ONLY SUPPORTS ASCII.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Return the shared library version string for libutf8proc.
 * @return	a pointer to a character string containing the libutf8proc version information.
 */
chr_t * lib_version_utf8proc(void) {
	return (chr_t *)utf8proc_release_d();
}

/**
 * @brief	Initialize the UTF8 library and dynamically bind to the required symbols.
 * @return	true on success or false on failure.
 */
bool_t lib_load_utf8proc(void) {

	symbol_t utf8proc[] = {
		M_BIND(utf8proc_release), M_BIND(utf8proc_iterate), M_BIND(utf8proc_errmsg), M_BIND(utf8proc_category),
		M_BIND(utf8proc_category_string), M_BIND(utf8proc_get_property)
	};

	if (lib_symbols(sizeof(utf8proc) / sizeof(symbol_t), utf8proc) != 1) {
		return false;
	}

	return true;
}

/**
 * @brief	Exchange an error code for a string explaining the nature of the UTF8 error.
 * @param	error_code	the numeric error returned by the internal UTF8 mapping logic.
 * @return	A string constant with the error message. If the code goes unrecognized, then a generic message is returned.
 */
const chr_t * utf8_error_string(ssize_t error_code) {
	return utf8proc_errmsg_d(error_code);
}

/**
 * @brief	Determine how many valid Unicode characters the string has.
 * @param	s	the managed string to be tested.
 * @return	The number of codepoints found if the string is valid, otherwise 0.
 */
size_t utf8_length_st(stringer_t *s) {

	uint8_t *ptr;
	ssize_t bytes;
	int32_t codepoint;
	size_t len, result = 0;
	const utf8proc_property_t *properties;

	if (st_empty_out(s, &ptr, &len)) {
		log_pedantic("Passed in a NULL pointer or zero length string.");
		return 0;
	}

	while (len) {

		// Iterate through the buffer and decompose each character.
		if ((bytes = utf8proc_iterate_d(ptr, len, &codepoint)) <= 0) {
			log_pedantic("Invalid UTF8 byte sequence encountered. { hex = %08X, error = %s }",
				((uint8_t *)&codepoint)[0], utf8_error_string(bytes));
			return 0;
		}

		// We only increment the length counter if the codepoint isn't marked as ignorable.
		if (!((properties = utf8proc_get_property_d(codepoint))->ignorable)) {
			result++;
		}

		len -= bytes;
		ptr += bytes;
	}

	return result;
}

/**
 * @brief	Determine whether a managed string is a properly formatted UTF8 string.
 * @param	s	the managed string to be tested.
 * @return	Valid strings return true, while invalid strings return false.
 */
bool_t utf8_valid_st(stringer_t *s) {

	size_t len;
	uint8_t *ptr;
	ssize_t bytes;
	int32_t codepoint;

	if (st_empty_out(s, &ptr, &len)) {
		log_pedantic("Passed in a NULL pointer or zero length string.");
		return false;
	}

	while (len) {
		if ((bytes = utf8proc_iterate_d(ptr, len, &codepoint)) < 0) {
			log_pedantic("Invalid UTF8 byte sequence encountered. { codepoint = %02X, %02X, %02X, %02X }", ((uint8_t *)&codepoint)[0],
				((uint8_t *)&codepoint)[1], ((uint8_t *)&codepoint)[2], ((uint8_t *)&codepoint)[3]);
			return false;
		}

		len -= bytes;
		ptr += bytes;
	}

	return true;
}
