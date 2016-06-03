
/**
 * @file /magma/core/strings/nuller.c
 *
 * @brief	Functions for handling null terminated strings.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"


/**
 * @brief	Return the length of a null-terminated string.
 * @param	s	the input as a null-terminatd string.
 * @return	the length in bytes of the string.
 */
size_t ns_length_get(const chr_t *s) {
	size_t len = 0;
	while (*s++) len++;
	return len;
}

/**
 * @brief	Return whether a null-terminated string is empty or not.
 * @param	s	the input as a null-terminated string.
 * @return	true if string is NULL or zero length; false otherwise.
 */
bool_t ns_empty(chr_t *s) {
	if (!s || !ns_length_get(s)) return true;
	return false;
}

/**
 * @brief	Return whether a null-terminated string is empty or not, while storing its address and length.
 * @param	s	the input as a null-terminated string.
 * @param	ptr	a pointer address to receive a copy of the string's location.
 * @param	len	a pointer to a variable to receive the length of the string, in bytes.
 * @return	true if string is NULL or zero length; false otherwise.
 */
bool_t ns_empty_out(chr_t *s, chr_t **ptr, size_t *len) {
	if (!(*ptr = s) || !(*len = ns_length_get(s))) return true;
	return false;
}

/**
 * @brief	Return the length of a null-terminated string as an int, capped at INT_MAX.
 * @param	s	the null-terminated string to be evaluated.
 * @return	the length of the string.
 */
int ns_length_int(chr_t *s) {

	size_t len = ns_length_get(s);

	if (len > INT_MAX) {
		log_pedantic("Requested length is greater than INT_MAX. {nuller.length = %zu}", len);
		return INT_MAX;
	}

	return len;
}

/**
 * @brief	Allocate (and wipe) a buffer for a null-terminated string.
 * @param	len	the length of the buffer to be allocated.
 * @return	NULL on failure or if len was 0; a pointer to the newly allocated memory otherwise.
 */
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

/**
 * @brief	Free a null-terminated string.
 * @param	s	the string to be freed.
 * @return	This function returns no value.
 */
void ns_free(chr_t *s) {

#ifdef MAGMA_PEDANTIC
	if (s == NULL)
		log_pedantic("Attempted to free a NULL pointer.");
#endif

	if (s) mm_free(s);
	return;
}

/**
 * @brief	A checked null-terminated string free front-end function.
 * @see		ns_free()
 * @param	s	the null-terminated string to be freed.
 * @return	This function returns no value.
 */
void ns_cleanup(chr_t *s) {

	if (s) {
		ns_free(s);
	}

	return;
}

/**
 * @brief	Duplicate a null-terminated string.
 * @param	s	the null-terminated string to be duplicated.
 * @return	NULL on failure, or a pointer to a copy of the input string.
 */
chr_t * ns_dupe(chr_t *s) {

	chr_t *result;
	size_t length;

	if (!s || (!(length = ns_length_get(s)))) {
		log_pedantic("Cannot duplicate NULL or zero-length string.");
		return NULL;
	}

	if (!(result = mm_alloc(length + 1))) {
		log_info("Was not able to allocate buffer for duplicate string.");
		return NULL;
	}

	mm_copy(result, s, length);
	return result;
}

/**
 * @brief	Get a block of memory as a null-terminated string.
 * @param	block	a pointer to the buffer containing the data to be copied.
 * @param	len		the length, in bytes, of the data buffer to be copied.
 * @return	NULL on failure, or a pointer to the newly allocated null-terminated string on success.
 */
chr_t * ns_import(void *block, size_t len) {

	chr_t *result;

	if (!(result = mm_alloc(len + 1))) {
		log_pedantic("Allocation of copied memory failed.");
		return NULL;
	}

	mm_copy(result, block, len);
	return result;
}

/**
 * @brief	Append one string to another and return the result.
 * @param	s		a pointer to a leading null-terminated string to to which the other string will be appended.
 * @param	append	a pointer to a null-terminated string to be appended to the leading string.
 * @return	NULL if a memory allocation failure occurred, or a pointer to the resulting string on success.
 */
chr_t * ns_append(chr_t *s, chr_t *append) {

	chr_t *output = NULL;
	size_t alen, slen = 0;

	if (!append || !(alen = ns_length_get(append))) {
		log_pedantic("The append string appears to be empty.");
		return s;
	}

	// Allocate a new string if the existing string pointer is NULL.
	if (!s) {
		s = ns_dupe(append);
	}
	else if (!(slen = ns_length_get(s))) {
		ns_free(s);
		s = ns_dupe(append);
	}
	// Otherwise check the amount of available space in the buffer and if necessary allocate more.
	else if ((output = ns_alloc(slen + alen + 1))) {
		mm_copy(output, s, slen);
		mm_copy(output + slen, append, alen);
		ns_free(s);
		s = output;
	}

	return s;
}

/**
 * @brief	Zero out a null-terminated string.
 * @param	s	the string to be wiped.
 * @param	len	the number of bytes to be wiped at the start of the string.
 * @return	This function returns no parameters.
 */
// QUESTION: Why are we not calculating the string length automatically?
void ns_wipe(chr_t *s, size_t len) {

#ifdef MAGMA_PEDANTIC
	if (!s) log_pedantic("Attempting to wipe a NULL string pointer.");
#endif

	if (s && len) mm_set(s, 0, len);
	return;
}
