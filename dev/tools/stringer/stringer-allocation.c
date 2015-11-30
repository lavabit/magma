/**
 * @file /stringer/stringer-allocation.c

 *
 * @brief The functions used to manipulate stringers.
 *
 * $Author: Ladar Levison $
 * $Date: 2010/12/12 23:59:04 $
 * $Revision: a46af76362b976a414117a5bfee9dfc2e9fd38b2 $
 *
 */

#include "stringer.h"

void st_free(stringer_t *s) {

	uint32_t opts = *((uint32_t *)s);
	void (*release)(void *buffer) = opts & SECURE ? &mm_sec_free : &mm_free;

#ifdef MAGMA_PEDANTIC
	if (!st_opts_free(opts)) {
		log_pedantic("Invalid string options. {opt = %u}", opts);
		return;
	}
#endif

	switch (opts & (NULLER_T | PLACER_T | BLOCK_T | MANAGED_T | MAPPED_T | CONTIGUOUS | JOINTED)) {
		case (PLACER_T | JOINTED):
			release(s);
			break;
		case (NULLER_T | JOINTED):
			release(((nuller_t *)s)->data);
			release(s);
			break;
		case (NULLER_T| CONTIGUOUS):
			release(s);
			break;
		case (BLOCK_T | JOINTED):
			release(((block_t *)s)->data);
			release(s);
			break;
		case (BLOCK_T | CONTIGUOUS):
			release(s);
			break;
		case (MANAGED_T | JOINTED):
			release(((managed_t *)s)->data);
			release(s);
			break;
		case (MANAGED_T | CONTIGUOUS):
			release(s);
			break;
		case (MAPPED_T | JOINTED):
			munmap(((mapped_t *)s)->data, ((mapped_t *)s)->length + 1);
			close(((mapped_t *)s)->handle);
			release(s);
			break;
		default:
			log_pedantic("Invalid string options.");
	}
	return;
}

/**
 * Assumes that a properly sized placer has been allocated off the stack and needs to be initialized and its data/length variables set.
 *
 * @param s
 * @param data
 * @param length
 * @return
 */
stringer_t * st_placer_init(stringer_t *s, void *data, size_t len) {

	void *result;

	((placer_t *)s)->length = len;
	((placer_t *)s)->data = result = data;
	((placer_t *)s)->opts = (PLACER_T | JOINTED | STACK);

	return result;
}

stringer_t * st_placer_set(stringer_t *s, void *data, size_t len) {

	void *result = NULL;
	uint32_t opts = *((uint32_t *)s);

#ifdef MAGMA_PEDANTIC
	if (!st_opts_placer(opts)) {
		log_pedantic("Invalid string options. {opt = %u}", opts);
		return NULL;
	}
#endif

	if (opts & PLACER_T) {
		((placer_t *)s)->length = len;
		((placer_t *)s)->data = result = data;
	}

	return result;
}

stringer_t * st_merge(uint32_t opts, chr_t *format, ...) {

	va_list list;
	void *current;
	stringer_t *result;
	chr_t *cursor, *out;
	size_t length = 0, remaining;

#ifdef MAGMA_PEDANTIC
	if (!st_opts_destination(opts)) {
		log_pedantic("Invalid string options. {opt = %u}", opts);
		return NULL;
	}
#endif

	// Iterate through and calculate the combined length of the input strings.
	cursor = format;
	va_start(list, format);

	for (cursor = format; *cursor; cursor++) {
		if (*cursor == 's') {
			current = va_arg(list, stringer_t *);
			length += st_length_get(current);
		}
		else if (*cursor == 'n') {
			current = va_arg(list, char *);
			length += ns_length_get(current);
		}
	}

	va_end(list);

	// Allocate a buffer to hold the output.
	if (!(remaining = length) || !(result = st_alloc(opts, length)) || !(out = st_data_get(result))) {
		if (result) st_free(result);
		return NULL;
	}

	// Iterate through a second time and copy the bytes into the output buffer.
	cursor = format;
	va_start(list, format);

	for (cursor = format; *cursor; cursor++) {
		if (*cursor == 's') {

			// Fetch a stringer and determine its length.
			current = va_arg(list, stringer_t *);
			length = st_length_get(current);

			// The length returned should never exceed the space remaining.
			length = length > remaining ? remaining : length;

			// Copy the bytes into the output buffer.
			mm_copy(out, st_data_get(current), length);

			// Update the counters.
			out += length;
			remaining -= length;
		}
		else if (*cursor == 'n') {

			// Fetch a null terminated string and determine its length.
			current = va_arg(list, char *);
			length = ns_length_get(current);

			// The length returned should never exceed the space remaining.
			length = length > remaining ? remaining : length;

			// Copy the bytes into the output buffer.
			mm_copy(out, current, length);

			// Update the counters.
			out += length;
			remaining -= length;
		}
	}

	va_end(list);

	if (opts & (BLOCK_T | MANAGED_T | MAPPED_T)) {
		st_length_set(result, out - st_char_get(result));
	}

	return result;
}

stringer_t * st_print(uint32_t opts, chr_t *format, ...) {

	void *out;
	va_list list;
	int_t length;
	stringer_t *result;

#ifdef MAGMA_PEDANTIC
	if (!st_opts_destination(opts)) {
		log_pedantic("Invalid string options. {opt = %u}", opts);
		return NULL;
	}
#endif

	// Calculate the length.
	va_start(list, format);
	length = vsnprintf(NULL, 0, format, list);
	va_end(list);

	// Allocate a properly sized buffer.
	if (!length || !(result = st_alloc(opts, length + 1)) || !(out = st_data_get(result))) {
		if (result) st_free(result);
		return NULL;
	}

	// Print the provided format into the newly allocated buffer.
	va_start(list, format);
	if (vsnprintf(out, length + 1, format, list) != length) {
		log_pedantic("The print operation did not generate the amount of data we expected.");
		st_free(result);
		result = NULL;
	}
	va_end(list);

	if (opts & (BLOCK_T | MANAGED_T | MAPPED_T)) {
		st_length_set(result, length);
	}

	return result;
}

stringer_t * st_import(void *s, size_t len) {

	stringer_t *result;

	if (!(result = st_alloc(MANAGED_T | CONTIGUOUS | HEAP, len))) {
		return NULL;
	}

	mm_copy(st_data_get(result), s, len);
	st_length_set(result, len);

	return result;
}

stringer_t * st_dupe(stringer_t *s) {

	void *data;
	stringer_t *result = NULL;
	size_t length = 0, avail = 0;
	uint32_t opts = *((uint32_t *)s);

#ifdef MAGMA_PEDANTIC
	if (!st_opts_destination(opts)) {
		log_pedantic("Invalid string options. {opt = %u}", opts);
		return NULL;
	}
#endif

	switch (opts & (PLACER_T | NULLER_T | BLOCK_T | MANAGED_T | MAPPED_T)) {

		case (PLACER_T):

			data = st_data_get(s);
			length = st_length_get(s);

			if ((result = st_alloc(opts, 0))) {
				((placer_t *)result)->opts = opts;
				((placer_t *)result)->data = data;
				((placer_t *)result)->length = length;
			}
			break;

		case (NULLER_T):

			data = st_data_get(s);
			length = st_length_get(s);

			if ((result = st_alloc(opts, length))) {
				mm_copy(st_data_get(result), data, length);
			}
			break;

		case (BLOCK_T):

			data = st_data_get(s);
			length = st_length_get(s);
			if ((result = st_alloc(opts, length))) {
				mm_copy(st_data_get(result), data, length);
				st_length_set(result, length);
			}
			break;

		case (MANAGED_T):

			data = st_data_get(s);
			avail = st_avail_get(s);
			length = st_length_get(s);

			if ((result = st_alloc(opts, avail))) {
				mm_copy(st_data_get(result), data, length);
				st_length_set(result, length);
			}
			break;

		case (MAPPED_T):

			data = st_data_get(s);
			avail = st_avail_get(s);
			length = st_length_get(s);

			if ((result = st_alloc(opts, avail))) {
				mm_copy(st_data_get(result), data, length);
				st_length_set(result, length);
			}
			break;

		default:
			log_pedantic("Invalid string options. {opt = %u}", opts);
	}

	return result;
}

stringer_t * st_alloc(uint32_t opts, size_t len) {

	int handle;
	void *joint;
	stringer_t *result = NULL;
	char pattern[128] = "/tmp/mapped.XXXXXX";
	void (*release)(void *buffer) = opts & SECURE ? &mm_sec_free : &mm_free;
	void * (*allocate)(size_t len) = opts & SECURE ? &mm_sec_alloc : &mm_alloc;

#ifdef MAGMA_PEDANTIC
	if (!st_opts_valid(opts)) {
		log_pedantic("Invalid string options. {opt = %u}", opts);
		return NULL;
	}
#endif

	switch (opts & (NULLER_T | PLACER_T | BLOCK_T | MANAGED_T | MAPPED_T | CONTIGUOUS | JOINTED)) {

		case (PLACER_T | JOINTED):

			// Allow the allocation of placer's but the size parameter is reasonably useless.
			if ((result = allocate(sizeof(placer_t)))) {
				((placer_t *)result)->opts = opts;
			}
			break;

		case (NULLER_T | JOINTED):

			if ((result = allocate(sizeof(nuller_t))) && (joint = allocate(len + 1))) {
				((nuller_t *)result)->opts = opts;
				((nuller_t *)result)->data = joint;
			} else if (result) {
				release(result);
				result = NULL;
			}
			break;

		case (NULLER_T | CONTIGUOUS):

			if ((result = allocate(sizeof(nuller_t) + len + 1))) {
				((nuller_t *)result)->opts = opts;
				((nuller_t *)result)->data = ((char *)result + sizeof(nuller_t));
			}
			break;

		case (BLOCK_T | JOINTED):

			if ((result = allocate(sizeof(block_t))) && (joint = allocate(len + 1))) {
				((block_t *)result)->opts = opts;
				((block_t *)result)->length = len;
				((block_t *)result)->data = joint;
			} else if (result) {
				release(result);
				result = NULL;
			}
			break;

		case (BLOCK_T | CONTIGUOUS):

			if ((result = allocate(sizeof(block_t) + len + 1))) {
				((block_t *)result)->opts = opts;
				((block_t *)result)->length = len;
				((block_t *)result)->data = ((char *)result + sizeof(block_t));
			}
			break;

		case (MANAGED_T | JOINTED):

			if ((result = allocate(sizeof(managed_t))) && (joint	= allocate(len + 1))) {
				((managed_t *)result)->opts = opts;
				((managed_t *)result)->avail = len;
				((managed_t *)result)->data = joint;
			} else if (result) {
				release(result);
				result = NULL;
			}
			break;

		case (MANAGED_T | CONTIGUOUS):

			if ((result = (allocate(sizeof(managed_t) + len + 1)))) {
				((managed_t *)result)->opts = opts;
				((managed_t *)result)->avail = len;
				((managed_t *)result)->data = ((char *)result + sizeof(managed_t));
			}
			break;

		case (MAPPED_T | JOINTED):

			// Ensure the allocated size is always a multiple of the memory page size. Then truncate the resulting file to specified length before memory mapping.
			len = len ? ((int)len + getpagesize() - 1) & ~(getpagesize() - 1) : getpagesize();

			if (len && (handle = mkstemp(pattern)) != -1 && ftruncate64(handle, len) == 0 && (result = allocate(sizeof(mapped_t))) &&
					(joint = mmap64(NULL, len,	PROT_WRITE | PROT_READ, opts & SECURE ? MAP_PRIVATE | MAP_LOCKED : MAP_PRIVATE, handle, 0)) != MAP_FAILED) {
				mm_set(joint, 0, len);
				((mapped_t *)result)->opts = opts;
				((mapped_t *)result)->avail = len - 1;
				((mapped_t *)result)->data = joint;
				((mapped_t *)result)->handle = handle;
			} else {
				if (handle != -1)	close(handle);
				if (result) {
					release(result);
					result = NULL;
				}
			}
			break;

		default:
			log_pedantic("Invalid string options. {opt = %u}", opts);

	}

	return result;
}

stringer_t * st_realloc(stringer_t *s, size_t len) {

	void *joint;
	char message[1024];
	size_t original, avail;
	stringer_t *result = NULL;
	uint32_t opts = *((uint32_t *)s);
	void (*release)(void *buffer) = opts & SECURE ? &mm_sec_free : &mm_free;
	void * (*allocate)(size_t len) = opts & SECURE ? &mm_sec_alloc : &mm_alloc;

#ifdef MAGMA_PEDANTIC
	if (!st_opts_valid(opts)) {
		log_pedantic("Invalid string options. {opt = %u}", opts);
		return NULL;
	}
#endif

	// Store the original string length.
	original = st_length_get(s);

	switch (opts & (NULLER_T | BLOCK_T | MANAGED_T | MAPPED_T | CONTIGUOUS | JOINTED)) {

		case (NULLER_T | JOINTED):

			if ((joint = allocate(len + 1))) {
				mm_copy(joint, ((nuller_t *)s)->data, (original < len ? original : len));
				release(((nuller_t *)s)->data);
				((nuller_t *)s)->data = joint;
				result = s;
			}
			break;

		case (NULLER_T | CONTIGUOUS):

			if ((result = allocate(sizeof(nuller_t) + len + 1))) {
				mm_copy(((char *)result + sizeof(nuller_t)), ((nuller_t *)s)->data, (original < len ? original : len));
				((nuller_t *)result)->opts = opts;
				((nuller_t *)result)->data = ((char *)result + sizeof(nuller_t));
			}
			break;

		case (BLOCK_T | JOINTED):

			if ((joint = allocate(len + 1))) {
				mm_copy(joint, ((block_t *)s)->data, (original < len ? original : len));
				release(((block_t *)s)->data);
				((block_t *)s)->length = len;
				((block_t *)s)->data = joint;
				result = s;
			}
			break;

		case (BLOCK_T | CONTIGUOUS):

			if ((result = allocate(sizeof(block_t) + len + 1))) {
				mm_copy(((char *)result + sizeof(block_t)), ((block_t *)s)->data, (original < len ? original : len));
				((block_t *)result)->opts = opts;
				((block_t *)result)->length = len;
				((block_t *)result)->data = ((char *)result + sizeof(block_t));
			}
			break;

		case (MANAGED_T | JOINTED):

			if ((joint = allocate(len + 1))) {
				mm_copy(joint, ((managed_t *)s)->data, (original < len ? original : len));
				release(((managed_t *)s)->data);
				((managed_t *)s)->length = (original < len ? original : len);
				((managed_t *)s)->avail = len;
				((managed_t *)s)->data = joint;
				result = s;
			}
			break;

		case (MANAGED_T | CONTIGUOUS):

			if ((result = allocate(sizeof(managed_t) + len + 1))) {
				mm_copy(((char *)result + sizeof(managed_t)), ((managed_t *)s)->data, (original < len ? original : len));
				((managed_t *)result)->opts = opts;
				((managed_t *)result)->length = (original < len ? original : len);
				((managed_t *)result)->avail = len;
				((managed_t *)result)->data = ((char *)result + sizeof(managed_t));
			}
			break;

		// Strings using mmap's must always be jointed.
		case (MAPPED_T | JOINTED):

			avail = ((mapped_t *)s)->avail + 1;
			len = len ? ((int)len + getpagesize() - 1) & ~(getpagesize() - 1) : getpagesize();

			// If we end up shrinking the available memory then we'll need to update the length variable to reflect that.
			if (avail >= len && (joint = mremap(((mapped_t *)s)->data, avail, len, MREMAP_MAYMOVE)) != MAP_FAILED) {
				((mapped_t *)s)->length = len - 1;
				((mapped_t *)s)->avail = len - 1;
				((mapped_t *)s)->data = joint;
				*((char *)joint + len - 1) = 0;
				result = s;
			}

			// If we increase the amount of the available space, the length parameter can remain unchanged since any existing data should be preserved.
			else if (avail < len && ftruncate64(((mapped_t *)s)->handle, len) == 0 && (joint = mremap(((mapped_t *)s)->data, avail, len, MREMAP_MAYMOVE)) != MAP_FAILED) {
				((mapped_t *)s)->opts = opts;
				((mapped_t *)s)->avail = len - 1;
				((mapped_t *)s)->data = joint;
				result = s;
			}

			// An error occurred.
			else {
				log_pedantic("An error occurred while resizing a memory mapped buffer. {%s}", strerror_r(errno, message, 1024));
			}
			break;

		default:
			log_pedantic("Invalid string options. {opt = %u}", opts);
	}

	return result;
}
