
/**
 * @file /magma/core/strings/data.c
 *
 * @brief	Functions used to inspect the data of managed strings.
 *
 * $Author$
 * $Author$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Determine whether the specified managed string is empty or not.
 * @param	s 	the input managed string.
 * @result	true if string is NULL or uninitialized or empty; false otherwise.
 */
bool_t st_empty(stringer_t *s) {

	if (!s || !*((uint32_t *)s) || !st_data_get(s) || !st_length_get(s)) {
		return true;
	}

	return false;
}

/**
 * @brief	Determine whether the specified managed string is empty or not, and store its underlying data and data length.
 * @param	s		the input managed string.
 * @param	ptr		a pointer to a null-terminated string that will receive the address of the managed string's underlying data.
 * @param	len		a pointer to store the size of the managed string's underlying data buffer.
 * @return	true if string is NULL or uninitialized or empty; false otherwise.
 */
bool_t st_empty_out(stringer_t *s, uchr_t **ptr, size_t *len) {

	if (!s || *((uint32_t *)s) == 0 || !(*ptr = st_data_get(s)) || !(*len = st_length_get(s))) {
		return true;
	}

	return false;
}


/**
 *@brief	Set the underlying data of a jointed managed string.
 *@param	s		the managed string to be adjusted.
 *@param	data	the data buffer to be attached to the input managed string.
 *@note		The underlying data of s will be released, unless it contains foreign data.
 *@return	This function does not return a value.
 */
void st_data_set(stringer_t *s, void *data) {

	uint32_t opts;
	void (*release)(void *buffer);

	if (!s || !(opts = *((uint32_t *)s)) || !(release = opts & SECURE ? &mm_sec_free : &mm_free)) {
		return;
	}

#ifdef MAGMA_PEDANTIC
	if (!st_valid_jointed(opts)) {
		log_pedantic("Invalid string options. { opt = %u = %s }", opts, st_info_opts(opts, MEMORYBUF(128), 128));
		return;
	}
#endif

	if (data && (opts & SECURE) && !mm_sec_secured(data)) {
		log_pedantic("Assigning an insecure memory address to a secure string is impossible.");
		return;
	}

	if (data && !(opts & JOINTED)) {
		log_pedantic("Setting the data pointer on a non-jointed string is impossible.");
		return;
	}

	// QUESTION: Is it possible for s->data to point to a NULL value?

	switch (opts & (NULLER_T | BLOCK_T | PLACER_T | MANAGED_T | MAPPED_T | JOINTED)) {
		case (NULLER_T | JOINTED):
			if (!(opts & FOREIGNDATA)) release(((nuller_t *)s)->data);
			((nuller_t *)s)->data = data;
			break;
		case (BLOCK_T | JOINTED):
			if (!(opts & FOREIGNDATA)) release(((block_t *)s)->data);
			((block_t *)s)->data = data;
			break;
		case (MANAGED_T | JOINTED):
			if (!(opts & FOREIGNDATA)) release(((managed_t *)s)->data);
			((managed_t *)s)->data = data;
			break;
		case (PLACER_T | JOINTED):
			if (!(opts & FOREIGNDATA)) {
				release(((placer_t *)s)->data);
			}
			((placer_t *)s)->data = data;
			break;
	}

	return;
}

/**
 * @brief	Retrieve the data associated with a managed string.
 * @param	s	the input managed string.
 * @return	NULL on failure or for an improperly constructed string; otherwise, a pointer to the string's data.
 */
void * st_data_get(stringer_t *s) {

	uint32_t opts;
	void *result = NULL;

	if (!s || !(opts = *((uint32_t *)s))) {
		return NULL;
	}

#ifdef MAGMA_PEDANTIC
	else if (!st_valid_opts(opts)) {
		debug_hook();
		log_pedantic("Invalid string options. { opt = %u = %s }", opts, st_info_opts(opts, MEMORYBUF(128), 128));
		return NULL;
	}
#endif

	switch (opts & (CONSTANT_T | NULLER_T | BLOCK_T | PLACER_T | MANAGED_T | MAPPED_T)) {

		case (CONSTANT_T):
			result = ((constant_t *)s)->data;
			break;
		case (NULLER_T):
			result = ((nuller_t *)s)->data;
			break;
		case (BLOCK_T):
			result = ((block_t *)s)->data;
			break;
		case (PLACER_T):
			result = ((placer_t *)s)->data;
			break;
		case (MANAGED_T):
			result = ((managed_t *)s)->data;
			break;
		case (MAPPED_T):
			result = ((mapped_t *)s)->data;
			break;
	}

	return result;
}

/**
 * @brief	Retrieve a character pointer to a managed string's data buffer.
 * @see		st_data_get()
 * @param	s	the input managed string.
 * @return	NULL on failure or for an improperly constructed string; otherwise, a pointer to the string's data.
 */
chr_t * st_char_get(stringer_t *s) {

	return (chr_t *)st_data_get(s);
}

/**
 * @brief	Retrieve an unsigned character pointer to a managed string's data buffer.
 * @see		st_data_get()
 * @param	s	the input managed string.
 * @return	NULL on failure or for an improperly constructed string; otherwise, a pointer to the string's data.
 */
uchr_t * st_uchar_get(stringer_t *s) {

	return (uchr_t *)st_data_get(s);
}

/**
 * @brief	Wipe all of a managed string's allocated memory and if applicable, reset its length.
 * @param	s	the managed string to be wiped.
 * @return	This function returns no value.
 */
void st_wipe(stringer_t *s) {

	uint32_t opts;

	if (!s || !(opts = *((uint32_t *)s))) {
		return;
	}

#ifdef MAGMA_PEDANTIC
	if (!st_valid_opts(opts)) {
		log_pedantic("Invalid string options. { opt = %u = %s }", opts, st_info_opts(opts, MEMORYBUF(128), 128));
		return;
	}
#endif

	mm_wipe(st_data_get(s), st_avail_get(s));

	if (st_valid_tracked(opts)) {
		st_length_set(s, 0);
	}

	return;
}
