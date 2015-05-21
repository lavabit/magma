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
 * @brief	Check that data is a valid UTF string.
 * @param	utf_string		Data stringer to be checked.
 * @return	true if good, false if not.
 * NOTE: FIXME TODO Place-holder only supporting a subset of UTF-8 (ASCII)
 */
bool_t utf_is_good(stringer_t *utf_string) {

	void *data;

	if(!utf_string) {
		log_pedantic("A NULL pointer was passed in");
		return false;
	}

	if(!(data = st_data_get(utf_string))) {
		log_pedantic("Unallocated stringer data.");
		return false;
	}

	for(size_t i = 0; i < st_length_get(utf_string); ++i) {

		if(!isascii(data[i])) {
			return false;
		}
		
	}

	return true;
}

/**
 * @brief	Check size of UTF-8 data.
 * @param	utf_string	UTF-8 data in a stringer.
 * @return	Size of utf_string in UTF characters, 0 on failure.
 * NOTE: FIXME TODO Place-holder only supporting a subset of UTF-8 (ASCII)
*/
size_t utf_length_get(stringer_t *utf_string) {

	if(!utf_string) {
		log_pedantic("A NULL pointer was passed in.");
		return 0;
	}

	if(!utf_is_good(utf_string)) {
		log_pedantic("Invalid UTF-8 string.");
	}

	return st_length_get(utf_string);
}
