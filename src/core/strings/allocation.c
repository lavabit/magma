
/**
 * @file /magma/core/strings/allocation.c
 *
 * @brief	Functions used to allocate stringers.
 *
 * $Author$
 * $Date$
 * $Revision:$
 *
 */

#include "magma.h"

/**
 * @brief	Free a managed string.
 * @see		st_valid_free(), st_valid_opts()
 * @note	Any managed string to be freed should conform with the validity checks enforced by st_valid_free()/st_valid_opts()
 * 			*NO* managed string should be passed to st_free() if it was allocated on the stack, or contains foreign data.
 * 			The following logic is carried out depending on the allocation options of the string to be freed:
 * 			Jointed placer:		Free header, if secure or on heap
 * 								Free underlying data if it is not foreign
 * 			Jointed nuller:		Free header and underlying data
 * 			Jointed block:		Free header and underlying data
 * 			Jointed managed:	Free header and underlying data
 * 			Jointed mapped:		Free the header and (munmap) underlying data
 * 			Contiguous nuller:	Free header (this includes the underlying data because they are already merged)
 * 			Contiguous block:	Free header (this includes the underlying data because they are already merged)
 * 			Contiguous managed:	Free header (this includes the underlying data because they are already merged)
 *
 * @param	s	the managed string to be freed.
 * @return	This function returns no value.
 */
void st_free(stringer_t *s) {

	uint32_t opts = *((uint32_t *)s);
	void (*release)(void *buffer) = opts & SECURE ? &mm_sec_free : &mm_free;

#ifdef MAGMA_PEDANTIC
	if (!st_valid_free(opts)) {
		debug_hook();
		log_pedantic("Invalid string options. { opt = %u = %s }", opts, st_info_opts(opts, MEMORYBUF(128), 128));
		return;
	}
#endif

	/// HIGH: Finish this logic out. Stack allocations are skipped, unless they are jointed. In that case the data is freed
	/// unless it carries a foreigner flag. Note its possible for a jointed string to have a NULL data reference. We should be
	/// picking up on that too. Contiguous heap buffers are just destroyed.

	/// Do we need to differentiate between stack structures, and heap data blocks needing to be freed, or not?

	switch (opts & (NULLER_T | PLACER_T | BLOCK_T | MANAGED_T | MAPPED_T | CONTIGUOUS | JOINTED)) {
		case (PLACER_T | JOINTED):
			if (!(opts & FOREIGNDATA)) release(((placer_t *)s)->data);
			if (opts & (HEAP | SECURE)) release(s);
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
			break;
	}

	return;
}

/**
 * @brief	A checked cleanup function which can be used free a variable number managed strings.
 * @see		st_free()
 *
 * @param	va_list	a list of managed strings to be freed.
 *
 * @result	This function returns no value.
 */
void st_cleanup_variadic(ssize_t len, ...) {

	va_list list;
	stringer_t *s = NULL;

	va_start(list, len);

	for (ssize_t i = 0; i < len; i++) {
		s = va_arg(list, stringer_t *);
		if (s) st_free(s);
	}
	va_end(list);

	return;
}

/**
 * @brief	Concatenate a variable number of user-supplied multi-type strings.
 * @param	opts	the options value of the resulting managed string that will be allocated for the caller.
 * @param	format	a format string consisting of the characters 's' for specifying that the next variable argument
 * 					will be a managed string or 'n' if it is a null-terminated string.
 * @param	...		a variable argument list containing the strings to be concatenated to one another.
 * @return	a newly allocated string containing the concatenations of all specified managed and null-terminated strings.
 */
stringer_t * st_merge_opts(uint32_t opts, chr_t *format, ...) {

	va_list list;
	void *current;
	stringer_t *result = NULL;
	chr_t *cursor, *out;
	size_t length = 0, remaining;

#ifdef MAGMA_PEDANTIC
	if (!st_valid_destination(opts)) {
		log_pedantic("Invalid string options. { opt = %u = %s }", opts, st_info_opts(opts, MEMORYBUF(128), 128));
		return NULL;
	}
#endif

	// Iterate through and calculate the combined length of the input strings.
	cursor = format;
	va_start(list, format);
	for (cursor = format; *cursor; cursor++) {
		if (*cursor == 's') {
			current = va_arg(list, stringer_t *);
			if (!st_empty(current)) {
				length += st_length_get(current);
			}
		}
		else if (*cursor == 'n') {
			current = va_arg(list, chr_t *);
			if (!ns_empty(current)) {
				length += ns_length_get(current);
			}
		}
	}

	va_end(list);

	// Allocate a buffer to hold the output.
	if (!(remaining = length) || !(result = st_alloc_opts(opts, length)) || !(out = st_data_get(result))) {
		st_cleanup(result);
		return NULL;
	}

	// Iterate through a second time and copy the bytes into the output buffer.
	cursor = format;
	va_start(list, format);

	for (cursor = format; *cursor; cursor++) {
		if (*cursor == 's') {

			// Fetch a stringer and determine its length.
			current = va_arg(list, stringer_t *);
			if (!st_empty(current)) {
				length = st_length_get(current);

				// The length returned should never exceed the space remaining.
				length = length > remaining ? remaining : length;

				// Copy the bytes into the output buffer.
				mm_copy(out, st_data_get(current), length);

				// Update the counters.
				out += length;
				remaining -= length;
			}
		}
		else if (*cursor == 'n') {

			// Fetch a null terminated string and determine its length.
			current = va_arg(list, char *);
			if (!ns_empty(current)) {
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
	}

	va_end(list);

	if (st_valid_tracked(opts)) {
		st_length_set(result, out - st_char_get(result));
	}

	return result;
}

/**
 * @brief	Create a new (contiguous managed) managed string on the heap to hold a copy of the specified data buffer.
 * @param	s	the address of the buffer to be duplicated.
 * @param	len	the length, in bytes, of the copied buffer.
 * @result	NULL on failure, or a pointer to the newly allocated managed string on success.
 */
stringer_t * st_import(const void *s, size_t len) {

	stringer_t *result;

	if (!(result = st_alloc_opts(MANAGED_T | CONTIGUOUS | HEAP, len))) {
		return NULL;
	}

	mm_copy(st_data_get(result), s, len);
	st_length_set(result, len);

	return result;
}

/**
 * @brief	Copy data into a a managed string.
 * @param	s	the managed string to store the copied contents of the data.
 * @param	buf	a pointer to the buffer containing the data to be copied.
 * @param	len	the length of the data to be copied.
 * @result	NULL on failure, or a pointer to the managed string on success.
 */
stringer_t * st_copy_in(stringer_t *s, void *buf, size_t len) {

	void *dstbuf;

	if (!s || !buf || !len) {
		log_pedantic("Managed string copy operation contained NULL input.");
		return NULL;
	} else if (st_avail_get(s) < len) {
		log_pedantic("Managed string was not big enough to store copied data.");
		return NULL;
	} else if (!(dstbuf =  st_data_get(s))) {
		log_pedantic("Could not retrieve managed string data buffer for copy operation.");
		return NULL;
	}

	mm_copy(dstbuf,buf,len);

	if (!st_length_set(s,len)) {
		log_pedantic("Error setting length of copied string.");
		return NULL;
	}

	return s;
}

/**
 * @brief	Create a duplicate copy of a managed string with a specified set of allocation options.
 * @see		st_valid_opts()
 * @note	Both the source and destination managed strings must have option values that pass st_valid_opts().
 * 			The following procedure occurs when creating the duplicate managed string:
 * 			If the destination is a placer, 0 bytes are allocated for the duplicate's underlying data.
 * 			If the destination is a constant, nuller, or block, its underlying data buffer will be of equal size to the source's data length.
 *			If the destination is managed or mapped, and so is the source, its underlying data buffer will be of equal size to the source's available size;
 *				but if the source is neither, the underlying data buffer of the destination managed or mapped string will equal the size of the source's data length.
 *			A deep copy will be made of the source data to the destination if the destination is NOT a placer.
 *
 * @param	opts	the allocation options mask to be used for the newly created managed string.
 * @param	s		the target managed string to be duplicated.
 * @return	NULL on failure or a pointer to a copy of the duplicated managed string on success.
 */
stringer_t * st_dupe_opts(uint32_t opts, stringer_t *s) {

	void *src_data;
	stringer_t *result = NULL;
	size_t dst_length = 0, src_length = 0;
	uint32_t src_opts = *((uint32_t *)s);

#ifdef MAGMA_PEDANTIC
	if (!st_valid_opts(opts) || !st_valid_opts(src_opts)) {
		log_pedantic("Invalid string options. {opt = %u}", st_valid_opts(opts) ? src_opts : opts);
		return NULL;
	}
#endif

	// Determine how big the string buffer should be.
	switch (opts & (CONSTANT_T | PLACER_T | NULLER_T | BLOCK_T | MANAGED_T | MAPPED_T)) {

		case (PLACER_T):
			dst_length = 0;
			break;

		case (CONSTANT_T):
		case (NULLER_T):
		case (BLOCK_T):
			dst_length = st_length_get(s);
			break;

		case (MANAGED_T):
		case (MAPPED_T):
			if (src_opts & (MANAGED_T | MAPPED_T)) {
				dst_length = st_avail_get(s);
			}
			else {
				dst_length = st_length_get(s);
			}
			break;

		default:
			log_pedantic("Invalid string options. { opt = %u = %s }", opts, st_info_opts(opts, MEMORYBUF(128), 128));
			break;
	}

	// Allocate an appropriate buffer.
	if (!(result = st_alloc_opts(opts, dst_length))) {
		return NULL;
	}

	src_data = st_data_get(s);
	src_length = st_length_get(s);

	// Placer's just store the pointer.
	if (opts & PLACER_T) {
		st_data_set(result, src_data);
	}
	else {
		mm_copy(st_data_get(result), src_data, src_length);
	}

	// If the length of the data segment is tracked explicitly, set it here. Note that block's presume the length of the buffer is the
	// the length of the data, and null strings don't keep track of their length explicitly.
	if (st_valid_tracked(opts)) {
		st_length_set(result, src_length);
	}

	return result;
}

/**
 * @brief	Duplicate a managed string.
 * @see		st_dupe_opts()
 * @note	The allocation options of the duplicated string will be the same as that of the source string.
 * @param	s	the managed string to be duplicated.
 * @return	NULL on failure, or a copy of the input managed string on success.
 */
stringer_t * st_dupe(stringer_t *s) {

	uint32_t opts = *((uint32_t *)s);

	return st_dupe_opts(opts, s);
}

/**
 * @brief	Append one managed string to another, with aligned memory boundaries.
 * @param	align	an alignment value to be used for managed string (re)allocation. This only works for powers of 2.
 * @param	s		the managed string to be extended, which will be allocated for the caller if passed as NULL.
 * @param	append	the managed string to be appended to s.
 * @result	NULL on failure, or a pointer to the appended result on success.
 */
stringer_t * st_append_opts(size_t align, stringer_t *s, stringer_t *append) {

	size_t alen, slen = 0;

	// We can only append to mapped strings or jointed managed strings.
	if (s && !st_valid_append(*((uint32_t *)s))) {
		log_pedantic("Invalid string options. {opt = %u}", *((uint32_t *)s));
		return NULL;
	}

	if (!append || !(alen = st_length_get(append))) {
		log_pedantic("The append string appears to be empty.");
		return s;
	}

	// Allocate a new string if the existing string pointer is NULL.
	if (!s) {
		s = st_alloc_opts(MANAGED_T | HEAP | JOINTED, (alen + align - 1) & ~(align - 1));
	}
	// Otherwise check the amount of available space in the buffer and if necessary allocate more.
	else if (st_avail_get(s) - (slen = st_length_get(s)) < alen) {
		s = st_realloc(s, (slen + alen + align - 1) & ~(align - 1));
	}

	if (s) {
		mm_copy(st_data_get(s) + slen, st_data_get(append), alen);
		st_length_set(s, slen + alen);
	}

	return s;
}

/**
 * @brief	Allocate a managed string with a specified options mask.
 * @see		st_valid_options()
 * @note	All requested allocation masks should conform to the validation imposed by st_valid_opts().
 * 			The supported types are: placer, nuller, block, managed, and mapped.
 * 			The following logic is applied to requested string allocation options:
 * 			1. Any allocation options specified for strings to be allocated on the stack are IGNORED.
 * 			2. All jointed strings allocate memory for the header and data separately and then link them EXCEPT:
 * 				Jointed placers only allocate space for a header.
 * 				Jointed mapped strings allocate the data with an aligned mmap() operation.
 * 			3. Contiguous strings allocate space for the header and data together in a single contiguous block.
 * 			4. After allocation, the length field is set for block strings.
 * 			5. After allocation, the available field is set for managed strings.
 * 			6. Placers can only be jointed; mapped strings can only be contiguous.
 *
 * @param	opts	the allocation options mask to be used by the newly allocated string.
 * @param	len		the length, in bytes, of the managed string to be allocated.
 * @return	NULL on failure or a pointer to the newly allocated managed string on success.
 */
stringer_t * st_alloc_opts(uint32_t opts, size_t len) {

	int handle;
	void *joint;
	stringer_t *result = NULL;
	void (*release)(void *buffer) = opts & SECURE ? &mm_sec_free : &mm_free;
	void * (*allocate)(size_t len) = opts & SECURE ? &mm_sec_alloc : &mm_alloc;

	// The logic below allocates memory off the heap, so if were passed options calling for the stack we silently replace it with instructions to use the heap.
	opts = (opts & STACK ? (opts ^ STACK) | HEAP : opts);

#ifdef MAGMA_PEDANTIC
	if (!st_valid_opts(opts)) {
		log_pedantic("Invalid string options. { opt = %u = %s }", opts, st_info_opts(opts, MEMORYBUF(128), 128));
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

			if ((result = allocate(sizeof(managed_t))) && (joint = allocate(len + 1))) {
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

			// Ensure the allocated size is always a multiple of the memory page size.
			len = align(magma.page_length, len ? len + 1 : 0);

			//Then truncate the file to ensure it matches the memory map size.
			if (len && (handle = spool_mktemp(MAGMA_SPOOL_DATA, "mapped")) != -1 && ftruncate64(handle, len) == 0 && (result = allocate(sizeof(mapped_t))) &&
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
			log_pedantic("Invalid string options. { opt = %u = %s }", opts, st_info_opts(opts, MEMORYBUF(128), 128));
			break;
	}

	return result;
}

/**
 * @brief	Reallocate a managed string to a specified size.
 * @note	The caller can request a new size that is either smaller (truncated) or larger than the original string.
 * 			Only these types of strings can be reallocated: nuller, block, managed, and mapped.
 * 			The following logic is applied to string reallocation requests:
 * 			For jointed strings, a new data buffer is allocated of the requested length, the original contents copied in, the data field of the
 * 				new string is set, and the original data buffer freed. The stringer header returned to the caller resides at the same address as the original.
 * 			For contiguous strings, a new data buffer is allocated for both the requested length and the new stringer header, and the data of both parts
 * 				are copied from the original string to the resulting one.
 * 			For block strings, the length field of the resulting string is set to the requested length.
 * 			For managed strings, the length field of the resulting string is set to the original length, but the available field is adjusted accordingly.
 * 			For mapped strings, an aligned mremap() call is made, and both the length and available fields are set to the new length.
 *
 * @param	s	the managed string to have its size reallocated.
 * @param	len	the new size, in bytes, of the resulting managed string.
 * @return	NULL on failure or a pointer to the newly reallocated managed string on success.
 */
// TODO: The semantics of st_realloc() are not what would typically be expected from a realloc() style function, since both
//       the original and the new string need to be st_freed()'d if the managed string is continuous. This needs to be fleshed out.
stringer_t * st_realloc(stringer_t *s, size_t len) {

	void *joint;
	char message[1024];
	size_t original, avail;
	stringer_t *result = NULL;
	uint32_t opts = *((uint32_t *)s);
	void (*release)(void *buffer) = opts & SECURE ? &mm_sec_free : &mm_free;
	void * (*allocate)(size_t len) = opts & SECURE ? &mm_sec_alloc : &mm_alloc;

#ifdef MAGMA_PEDANTIC
	if (!st_valid_opts(opts)) {
		log_pedantic("Invalid string options. { opt = %u = %s }", opts, st_info_opts(opts, MEMORYBUF(128), 128));
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
			len = len ? ((int)len + magma.page_length - 1) & ~(magma.page_length - 1) : magma.page_length;

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


			else {
				// An error occurred. If the errno is set to EAGAIN and it's secure memory, the most likely problem is that the requested amount of memory exceeds
				// the amount of locked memory available under this user account.
				if ((((mapped_t *)s)->opts & SECURE) && errno == EAGAIN && (system_limit_cur(RLIMIT_MEMLOCK) < len ||
						system_limit_cur(RLIMIT_MEMLOCK) < (len + magma.secure.memory.length))) {
					log_pedantic("Unable to resize the secure memory mapped buffer, the requested size exceeds the system limit for locked pages. " \
							"{limit = %lu / requested = %zu}", system_limit_cur(RLIMIT_MEMLOCK), len);
				}
				else {
					log_pedantic("An error occurred while resizing a memory mapped buffer. {%s}", strerror_r(errno, message, 1024));
				}
			}
			break;

		default:
			log_pedantic("Invalid string options. { opt = %u = %s }", opts, st_info_opts(opts, MEMORYBUF(128), 128));
			break;
	}

	return result;
}

/**
 * @brief	Return a suitable managed string to store data of a specified length.
 * @note	If a NULL output string is specified, one will be allocated for the caller.
 * @param	output	the input managed string (can be NULL).
 * @param	len		the size, in bytes, of the requested data buffer.
 * @result	NULL on failure or if input was not large enough  or a pointer to a validated output managed string.
 */
stringer_t * st_output(stringer_t *output, size_t len) {

	uint32_t opts = 0;
	stringer_t *result = NULL;

	if (output && !st_valid_destination((opts = *((uint32_t *)output)))) {
		log_pedantic("An output string was supplied but it does not represent a buffer capable of holding a result.");
		return NULL;
	}

	// Make sure the output buffer is large enough or if output was passed in as NULL we'll attempt the allocation of our own buffer.
	if ((result = output) && st_avail_get(output) < len) {
		log_pedantic("The output buffer supplied is not large enough to hold the result. {avail = %zu / required = %zu}", st_avail_get(output), len);
		return NULL;
	}
	else if (!output && !(result = st_alloc(len))) {
		log_pedantic("The memory allocation of the output buffer failed. {requested = %zu}", len);
		return NULL;
	}

	return result;
}

/**
 * @brief	Create a null-terminated string out of a block of memory and return it as a newly allocated nuller.
 * @param	input	a pointer to the data block to be copied.
 * @param	len		the size, in bytes, of the data block to be copied before being terminated with a null byte.
 * @return	NULL on failure, or a pointer to the new null-terminated string on success.
 */
stringer_t * st_nullify(chr_t *input, size_t len) {
	stringer_t *result;

	if (!input || !len || (!(result = st_alloc_opts(MANAGED_T | CONTIGUOUS | HEAP, len+1)))) {
		return NULL;
	}

	mm_copy(st_data_get(result), input, len);
	*((char *) st_data_get(result) + len) = 0;
	st_length_set(result, len);

	return result;
}
