
/**
 * @file /stringer/nuller.c
 *
 * @brief Functions for handling NULL terminated strings.
 *
 * $Author: $
 * $Date: $
 * $Revision: a46af76362b976a414117a5bfee9dfc2e9fd38b2 $
 *
 */

#include "stringer.h"

size_t ns_length_get(chr_t *s) {
	size_t len = 0;
	while (*s++) len++;
	return len;
}

bool_t ns_empty(chr_t *s) {
	if (!s || !ns_length_get(s)) return true;
	return false;
}

bool_t ns_empty_out(chr_t *s, chr_t **ptr, size_t *len) {
	if (!(*ptr = s) || !(*len = ns_length_get(s))) return true;
	return false;
}

/**
 *
 * Used to pass the length of a nuller into a function that requires an int, and not a size_t. If the length
 * is greater than what an int can hold, the value is adjusted to INT_MAX.
 *
 * @param string The string whose length is returned.
 * @return The length of the placer, or INT_MAX
 */
int ns_length_int(chr_t *s) {

	size_t len = ns_length_get(s);

	if (len > INT_MAX) {
		log_pedantic("Requested length is greater than INT_MAX. {nuller.length = %zu}", len);
		return INT_MAX;
	}

	return len;
}

chr_t * ns_alloc(size_t len) {

	chr_t *result;

	// No zero length strings.
	if (len == 0) {
		return NULL;
	}

	// Do the allocation. Include room for two sizer_ts plus a terminating NULL. If memory was allocated clear the buffer.
	if ((result = mm_alloc(len + 1)) != NULL) {
		ns_wipe(result, len);
	}

	// If no memory was allocated, discover that here.
	else {
		log_pedantic("Could not allocate %zu bytes.", len + 1);
	}

	return result;
}

void ns_free(chr_t *s) {

#ifdef MAGMA_PEDANTIC
	if (s == NULL)
		log_pedantic("Attempted to free a NULL pointer.");
#endif

	if (s) mm_free(s);
	return;
}

chr_t * ns_dupe(chr_t *s) {

	chr_t *result;
	size_t length;


	if (s == NULL || ((length = ns_length_get(s)) == 0))
		return NULL;

	if ((result = mm_alloc(length + 1)) == NULL)
		return NULL;

	mm_copy(result, s, length);

	return result;
}

chr_t * ns_import(void *block, size_t len) {

	chr_t *result;

	if ((result = mm_alloc(len + 1)) == NULL)
		return NULL;

	mm_copy(result, block, len);

	return result;
}

void ns_wipe(chr_t *s, size_t len) {

#ifdef MAGMA_PEDANTIC
	if (!s) log_pedantic("Attempting to wipe a NULL string pointer.");
#endif

	if (s && len) mm_set(s, 0, len);
	return;
}
