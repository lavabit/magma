/**
 * @file /stringer/stringer-data.c
 *
 * @brief The functions used to manipulate stringers.
 *
 * $Author: Ladar Levison $
 * $Date: 2010/12/12 23:59:04 $
 * $Revision: a46af76362b976a414117a5bfee9dfc2e9fd38b2 $
 *
 */

#include "stringer.h"

bool_t st_empty(stringer_t *s) {
	if (!s || !st_data_get(s) || !st_length_get(s)) return true;
	return false;
}

bool_t st_empty_out(stringer_t *s, uchr_t **ptr, size_t *len) {
	if (!s || !(*ptr = st_data_get(s)) || !(*len = st_length_get(s))) return true;
	return false;
}

void st_data_set(stringer_t *s, void *data) {

	uint32_t opts = *((uint32_t *)s);
	void (*release)(void *buffer) = opts & SECURE ? &mm_sec_free : &mm_free;

#ifdef MAGMA_PEDANTIC
	if (!st_opts_jointed(opts)) {
		log_pedantic("Invalid string options. {opt = %u}", opts);
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

	switch (opts & (NULLER_T | BLOCK_T | PLACER_T | MANAGED_T | MAPPED_T | JOINTED)) {
		case (NULLER_T | JOINTED):
			release(((nuller_t *)s)->data);
			((nuller_t *)s)->data = data;
			break;
		case (BLOCK_T | JOINTED):
			release(((block_t *)s)->data);
			((block_t *)s)->data = data;
			break;
		case (MANAGED_T | JOINTED):
			release(((managed_t *)s)->data);
			((managed_t *)s)->data = data;
			break;
		case (PLACER_T | JOINTED):
			((placer_t *)s)->data = data;
			break;
	}

	return;
}

void * st_data_get(stringer_t *s) {

	void *result = NULL;
	uint32_t opts = *((uint32_t *)s);

#ifdef MAGMA_PEDANTIC
	if (!st_opts_valid(opts)) {
		log_pedantic("Invalid string options. {opt = %u}", opts);
		return NULL;
	}
#endif

	switch (opts & (CONSTANT_T | NULLER_T | BLOCK_T | PLACER_T | MANAGED_T | MAPPED_T)) {

		case (CONSTANT_T):
			result = ((char *)s + sizeof(stringer_t));
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

chr_t * st_char_get(stringer_t *s) {

#ifdef MAGMA_PEDANTIC
	if (!st_opts_valid(*((uint32_t *)s))) {
		log_pedantic("Invalid string options. {opt = %u}", *((uint32_t *)s));
		return NULL;
	}
#endif

	return (chr_t *)st_data_get(s);
}
