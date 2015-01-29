
/**
 * @file /magma/core/parsers/special/bracket.c
 *
 * @brief	Functions for extracting a value inside a pair of brackets.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Get a pointer to a block of data between a pair of brackets.
 * @note	The data is of format "[_data_]"
 * @param	block	a buffer containing the bracketed data.
 * @param	length	the length, in bytes, of the data buffer to be parsed.
 * @return	a null placer on failure, or a placer pointing to the data between the brackets on success.
 */
placer_t bracket_extract_pl(void *block, size_t length) {

	char *start, *data;

	if (!block || !length) {
		log_pedantic("A NULL parameter was passed in.");
		return pl_null();
	}
	else if (*((char *)block) != '[') {
		log_pedantic("The provided string doesn't start with an open bracket.");
		return pl_null();
	}

	start = data = block + 1;
	while (length-- && *data != ']') data++;

	if (start == block) return pl_null();
	if (*data != ']') return pl_null();

	return pl_init(start, data - start);
}
