
/**
 * @file /magma/core/strings/length.c
 *
 * @brief	Fnctions used to find stringer lengths.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Return the length of the data in a managed string.
 * @param	s	the input managed string.
 * @return	0 on failure, or the length of the managed string's data in bytes.
 */
size_t st_length_get(stringer_t *s) {

	char *data;
	size_t result = 0;
	uint32_t opts; ;

	if (!s || !(opts = *((uint32_t *)s))) {
		return 0;
	}

#ifdef MAGMA_PEDANTIC
	if (!st_valid_opts(opts)) {
		log_pedantic("Invalid string options. { opt = %u = %s }", opts, st_info_opts(opts, MEMORYBUF(128), 128));
		return 0;
	}
#endif

	switch (opts & (CONSTANT_T | NULLER_T | BLOCK_T | PLACER_T | MANAGED_T | MAPPED_T)) {
		case (CONSTANT_T):
			data = ((constant_t *)s)->data;
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
			break;
	}

	return result;
}

/**
 * Return the length of a managed string as an integer.
 * @param	s	the managed string as type stringer_t *.
 * @return	the string length as an integer, capped at INT_MAX.
 * */
int_t st_length_int(stringer_t *s) {

	size_t len;

	if (!s || !*((uint32_t *)s)) {
		return 0;
	}

#ifdef MAGMA_PEDANTIC
	if (!st_valid_opts(*((uint32_t *)s))) {
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

/**
 * @brief	Set the data length for a managed string that supports data length tracking.
 * @param	s	the input managed string.
 * @param	len	the new value of the managed string's data length.
 * @return	0 on error, or the new data length on success.
 */
size_t st_length_set(stringer_t *s, size_t len) {

	uint32_t opts;

	if (!s || !(opts = *((uint32_t *)s))) {
		return 0;
	}

#ifdef MAGMA_PEDANTIC
	if (!st_valid_tracked(opts)) {
		log_pedantic("Invalid string options. { opt = %u = %s }", opts, st_info_opts(opts, MEMORYBUF(128), 128));
		return 0;
	}
#endif

	switch (opts & (PLACER_T | MANAGED_T | MAPPED_T)) {
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
			break;
	}

	return len;
}

/**
 * @brief	Return the total data buffer size allocated for a managed string.
 * @param	s	the input managed string.
 * @return	0 on failure, or the total buffer size in bytes on success.
 */
size_t st_avail_get(stringer_t *s) {

	size_t result = 0;
	uint32_t opts;

	if (!s || !(opts = *((uint32_t *)s))) {
		return 0;
	}

#ifdef MAGMA_PEDANTIC
	if (!st_valid_opts(opts)) {
		log_pedantic("Invalid string options. { opt = %u = %s }", opts, st_info_opts(opts, MEMORYBUF(128), 128));
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
			result = st_length_get(s);
			break;
	}

	return result;
}

/**
 * @brief	Set the total data buffer size allocated for a managed string.
 * @param	s		the input managed string.
 * @param	avail	the new buffer size for the managed string.
 * @return	0 on failure, or the managed string's new buffer size in bytes on success.
 */
size_t st_avail_set(stringer_t *s, size_t avail) {

	uint32_t opts;

	if (!s || !(opts = *((uint32_t *)s))) {
		return 0;
	}

#ifdef MAGMA_PEDANTIC
	if (!st_valid_avail(opts)) {
		log_pedantic("Invalid string options. { opt = %u = %s }", opts, st_info_opts(opts, MEMORYBUF(128), 128));
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
			break;
	}

	return avail;
}
