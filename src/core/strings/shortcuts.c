
/**
 * @file /magma/core/strings/shortcuts.c
 *
 * @brief	A collection of shortcuts used to call various string functions using sensible default values.
 *
 * $Author$
 * $Date$
 * $Revision:$
 *
 */

#include "magma.h"

/**
 * @brief	Return a zero-length placer pointing to NULL data.
 * @return	a zero-length placer pointing to NULL data.
 */
placer_t pl_null(void) {

	return (placer_t){ .opts = PLACER_T | JOINTED | STACK | FOREIGNDATA, .data = NULL, .length = 0 };
}

/**
 * @brief	Return a placer wrapping a data buffer of given size.
 * @param	data	a pointer to the data to be wrapped.
 * @param	len		the length, in bytes, of the data.
 * @return	a placer pointing to the specified data.
 */
placer_t pl_init(void *data, size_t len) {

	return (placer_t){ .opts = PLACER_T | JOINTED | STACK | FOREIGNDATA, .data = data, .length = len };
}

placer_t pl_clone(placer_t place) {
	return (pl_init(place.data, place.length));
}

placer_t pl_set(placer_t place, placer_t set) {

	return (placer_t){ .opts = place.opts, .data = set.data, .length = set.length };
}

/**
 * @brief	Get a pointer to the data referenced by a placer.
 * @param	place	the input placer.
 * @return	NULL on failure or a pointer to the block of data associated with the specified placer on success.
 */
void * pl_data_get(placer_t place) {

	return st_data_get((stringer_t *)&place);
}

/**
 * @brief	Get a character pointer to the data referenced by a placer.
 * @param	place	the input placer.
 * @return	NULL on failure or a a character pointer to the block of data associated with the specified placer on success.
 */
chr_t * pl_char_get(placer_t place) {

	return st_char_get((stringer_t *)&place);
}

/**
 * @brief	Get the length, in bytes, of a placer as an integer.
 * @param	place	the input placer.
 * @return	the size, in bytes, of the specified placer.
 */
int_t pl_length_int(placer_t place) {

	return st_length_int((stringer_t *)&place);
}

/**
 * @brief	Get the length, in bytes, of a placer.
 * @param	place	the input placer.
 * @return	the size, in bytes, of the specified placer.
 */
size_t pl_length_get(placer_t place) {

	return st_length_get((stringer_t *)&place);
}

/**
 * @brief	Determine whether or not the specified placer is empty.
 * @param	place	the input placer.
 * @return	true if the placer is empty or zero-length, or false otherwise.
 */
bool_t pl_empty(placer_t place) {

	return st_empty((stringer_t *)&place);
}

/**
 * @brief	Determine if a placer begins with a specified character.
 * @param	place	the input placer.
 * @param	c		the character to be compared with the first byte of the placer's data.
 * @return	true if the placer begins with the given character or false otherwise.
 */
bool_t pl_starts_with_char(placer_t place, chr_t c) {

	if (pl_empty(place)) {
		return false;
	}

	if (*(pl_char_get(place)) == c) {
		return true;
	}

	return false;
}

/**
 * @brief	Advance the placer one character forward beyond an expected character.
 * @param	place	the input placer.
 * @param	more	if true, the placer must contain more data, and vice versa.
 * @return	true if more was true and the placer contains more data, or if more was false and the placer ended; false otherwise.
 */
bool_t pl_inc(placer_t *place, bool_t more) {

	if (pl_empty(*place)) {
		return false;
	}

	place->length--;
	place->data = (chr_t *)place->data + 1;

	return (more == (place->length > 0));
}
