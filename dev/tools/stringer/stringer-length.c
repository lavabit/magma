/**
 * @file /stringer/stringer-length.c
 *
 * @brief The functions used to find stringer lens.
 *
 * $Author: Ladar Levison $
 * $Date: 2010/12/12 23:59:04 $
 * $Revision: a46af76362b976a414117a5bfee9dfc2e9fd38b2 $
 *
 */

#include "stringer.h"

size_t st_length_get(stringer_t *s) {

	char *data;
	size_t result = 0;
	uint32_t opts = *((uint32_t *)s);

#ifdef MAGMA_PEDANTIC
	if (!st_opts_valid(opts)) {
		log_pedantic("Invalid string options. {opt = %u}", opts);
		return 0;
	}
#endif

	switch (opts & (CONSTANT_T | NULLER_T | BLOCK_T | PLACER_T | MANAGED_T | MAPPED_T)) {
		case (CONSTANT_T):
			data = ((char *)s + sizeof(stringer_t));
			while (*data++) {
				result++;
			}
			break;
		case (NULLER_T):
			data = ((nuller_t *)s)->data;
			while (*data++) {
				result++;
			}
			break;
		case (BLOCK_T):
			result = ((block_t *)s)->length;
			break;
		case (PLACER_T):
			result = ((placer_t *)s)->length;
			break;
		case (MANAGED_T):
			result = ((managed_t *)s)->length;
			break;
		case (MAPPED_T):
			result = ((mapped_t *)s)->length;
			break;
		default:
			log_pedantic("Invalid string type.");
	}

	return result;
}

int st_length_int(stringer_t *s) {

	size_t len;

#ifdef MAGMA_PEDANTIC
	if (!st_opts_valid(*((uint32_t *)s))) {
		log_pedantic("Invalid string options. {opt = %u}", *((uint32_t *)s));
		return 0;
	}
#endif

	if ((len = st_length_get(s)) > INT_MAX) {
		log_pedantic("Requested length is greater than INT_MAX. {nuller.length = %zu}", len);
		return INT_MAX;
	}

	return (int)len;
}

size_t st_length_set(stringer_t *s, size_t len) {

	uint32_t opts = *((uint32_t *)s);

#ifdef MAGMA_PEDANTIC
	if (!st_opts_tracked(opts)) {
		log_pedantic("Invalid string options. {opt = %u}", opts);
		return 0;
	}
#endif

	switch (opts & (BLOCK_T | PLACER_T | MANAGED_T | MAPPED_T)) {
		case (BLOCK_T):
			((block_t *)s)->length = len;
			break;
		case (PLACER_T):
			((placer_t *)s)->length = len;
			break;
		case (MANAGED_T):
			if (len > ((managed_t *)s)->avail) {
				log_pedantic("The provided length is greater than the size of the buffer allocated. {avail = %lu / len = %lu}", ((managed_t *)s)->avail, len);
				len = ((managed_t *)s)->avail;
			}
			((managed_t *)s)->length = len;
			break;
		case (MAPPED_T):
			if (len > ((mapped_t *)s)->avail) {
				log_pedantic("The provided length is greater than the size of the buffer allocated. {avail = %lu / len = %lu}", ((mapped_t *)s)->avail, len);
				len = ((mapped_t *)s)->avail;
			}
			((mapped_t *)s)->length = len;
			break;
		default:
			log_pedantic("Invalid string type.");
			len = 0;
	}

	return len;
}

size_t st_avail_get(stringer_t *s) {

	size_t result = 0;
	uint32_t opts = *((uint32_t *)s);

#ifdef MAGMA_PEDANTIC
	if (!st_opts_avail(opts)) {
		log_pedantic("Invalid string options. {opt = %u}", opts);
		return 0;
	}
#endif

	switch (opts & (MANAGED_T | MAPPED_T)) {
		case (MANAGED_T):
			result = ((managed_t *)s)->avail;
			break;
		case (MAPPED_T):
			result = ((mapped_t *)s)->avail;
			break;
		default:
			log_pedantic("Available space is not tracked by the provided type.");
			result = st_length_get(s);
	}

	return result;
}

size_t st_avail_set(stringer_t *s, size_t avail) {

	uint32_t opts = *((uint32_t *)s);

#ifdef MAGMA_PEDANTIC
	if (!st_opts_avail(opts)) {
		log_pedantic("Invalid string options. {opt = %u}", opts);
		return 0;
	}
#endif

	switch (opts & (MANAGED_T | MAPPED_T)) {
		case (MANAGED_T):
			((managed_t *)s)->avail = avail;
			break;
		case (MAPPED_T):
			((mapped_t *)s)->avail = avail;
			break;
		default:
			log_pedantic("Available space is not tracked by the provided type.");
			avail = 0;
	}

	return avail;
}
