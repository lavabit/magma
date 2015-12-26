
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
 * TODO (Kent): clean this up.  utf_is_good -> utf_is_valid.
 * TODO (Kent): 0 length should be valid.
 * TODO (Kent): does false indiate not-valid?  Or an error condition?  Note: it
 * can't be both!
 */
bool_t utf_is_good(stringer_t *utf_string) {

	void *data;

	if(st_empty(utf_string)) {
		log_pedantic("A NULL pointer was passed in");
		return false;
	}

	if(!(data = st_data_get(utf_string))) {
		log_pedantic("Unallocated stringer data.");
		return false;
	}

	for(size_t i = 0; i < st_length_get(utf_string); ++i) {

		if(!chr_ascii(((chr_t *)data)[i])) {
			return false;
		}
		
	}

	return true;
}

/**
 * @brief	Check size of UTF-8 data.
 * @param	utf_string	UTF-8 data in a stringer.
 * @return	Size of utf_string in UTF characters, 0 on failure.
 * TODO - This routine should return the length of the utf string period,
 *        0 should be a valid length, -1 on error.  A length routine
 *        should not be validating the utf string unless there's a way
 *        to indicate an error.
 *
 * NOTE: FIXME TODO Place-holder only supporting a subset of UTF-8 (ASCII)
 * TODO (KENT): How to report an error?  0 should be a valid length if the string
 * contains no data.  So, 0 can't be an error.  -1 is error?
*/
size_t utf_length_get (stringer_t *utf_string) {

	if(st_empty(utf_string)) {
		log_pedantic("A NULL pointer was passed in.");
		return 0;
	}

	if(!utf_is_good(utf_string)) {
		log_pedantic("Invalid UTF-8 string.");
	}

	return st_length_get(utf_string);
}
