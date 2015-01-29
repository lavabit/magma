
/**
 * @file /magma/core/parsers/line.c
 *
 * @brief	Functions used to extract lines from within a larger block of data.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Get a placer pointing to the specified line ('\n' delimited) of content in a data buffer.
 * @param	block	a pointer to the block of data to be scanned.
 * @param	length	the length, in bytes, of the specified data buffer.
 * @param	number	the zero-based index of the line to be retrieved from the buffer.
 * @return	a null placer on failure, or a placer pointing to the specified line on success.
 */
placer_t line_pl_bl(char *block, size_t length, uint64_t number) {

	char *start;

	// We can't search NULL pointers or empty blocks.
	if (mm_empty(block, length)) {
		log_pedantic("Passed an invalid parameter.");
		return pl_null();
	}

	// Keep advancing till we reach the requested line, or the end of the block.
	while (number && length--) {

		if (*block++ == '\n') {
			number--;
		}

	}

	// If we hit the end of the string before finding the requested line, return NULL.
	if (!length || number) {
		return pl_null();
	}

	// Advance through until we reach the next new-line character.
	start = block;

	while (length-- && *block++ != '\n');

	if (*(block-1) != '\n') {
		return pl_null();
	}

	return pl_init(start, block - start);
}

/**
 * @brief	Get a placer pointing to the specified line ('\n' delimited) of a null-terminated string.
 * @see		line_pl_bl()
 * @param	string	a pointer to a null-terminated string to be scanned.
 * @param	number	the zero-based index of the line to be retrieved from the string.
 * @return	a null placer on failure, or a placer pointing to the specified line on success.
 */
placer_t line_pl_ns(char *string, uint64_t number) {

	return line_pl_bl(string, ns_length_get(string), number);
}

/**
 * @brief	Get a placer pointing to the specified line ('\n' delimited) of a managed string.
 * @param	string	a pointer to the managed string to be scanned.
 * @param	number	the zero-based index of the line to be retrieved from the managed string.
 * @return	a null placer on failure, or a placer pointing to the specified line on success.
 */
placer_t line_pl_st(stringer_t *string, uint64_t number) {

	return line_pl_bl(st_char_get(string), st_length_get(string), number);
}

/**
 * @brief	Get a placer pointing to the specified line ('\n' delimited) of another placer.
 * @param	string	a pointer to the placer to be scanned.
 * @param	number	the zero-based index of the line to be retrieved from the placer.
 * @return	a null placer on failure, or a placer pointing to the specified line on success.
 */
placer_t line_pl_pl(placer_t string, uint64_t number) {
	return line_pl_bl(pl_char_get(string), pl_length_get(string), number);
}
