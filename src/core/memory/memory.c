
/**
 * @file /magma/core/memory/memory.c
 *
 * @brief	The functions used to handle Magma memory buffers.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Performed a checked memory free.
 * @see		mm_cleanup
 * @param	block	the block of memory to be freed.
 * @return	This function returns no value.
 */
void mm_cleanup(void *block) {

	if (block) {
		mm_free(block);
	}

	return;
}


/**
 * @brief	Determine whether the memory buffer and/or its length encompass an empty block.
 * @param	block	a pointer to the block of memory to be assessed.
 * @param	len		the length, in bytes, of the memory block.
 * @return	false if block is NULL or len is 0; true otherwise.
 */
bool_t mm_empty(void *block, size_t len) {

	if(!block || !len) {
		return true;
	}

	return false;
}

/**
 * @brief	Copy data from one buffer to another.
 * @note	For overlapping data regions, bl_move() must be used.
 * @warning	This function performs an aligned copy so data directly after the src buffer may end up inside the dst buffer.
 * @param	dst	the destination buffer of the copy operation.
 * @param	src	the source buffer of the copy operation.
 * @param	len	the length, in bytes, of the data to be copied.
 * @return	a pointer to the destination buffer.
 */
void * mm_copy(void *dst, const void *src, size_t len) {

	return memcpy(dst, src, len);
}

/**
 * @brief	Copy the contents of one buffer into another one, allowing for overlapping data.
 * @see		memmove()
 * @param	dst		a pointer to the destination buffer to receive the data to be copied.
 * @param	src		a pointer to the source buffer supplying the data to be copied.
 * @param	len		the length, in bytes, of the data buffer to be copied.
 * @return	a pointer to the specified destination buffer.
 */
void * mm_move(void *dst, void *src, size_t len) {

	return memmove(dst, src, len);
}

/**
 * @brief	Sets a block of memory to a specified value.
 * @note Uses the 'optimize (0)' and 'noinline' function attributes to prevent compiler optimization from removing logic it might consider unnecessary.
 * @param	block	the block of memory to be set.
 * @param	set		the byte value to be written to block.
 * @param	len		the number of times to write the value of the byte repeatedly to block.
 * @return	a pointer to the block of memory passed to the function.
 * @see http://gcc.gnu.org/onlinedocs/gcc-4.4.4/gcc/Function-Attributes.html
 *
 */
// QUESTION: Why isn't "set" of type unsigned char?
void * mm_set(void *block, int_t set, size_t len) {

	volatile char *ptr = block;

	asm ("");

	while (len--) {
		*ptr++ = set;
	}

	asm ("");

	return block;
}

/**
 * @brief	Zero out a block of memory.
 * @note Uses the 'optimize (0)' and 'noinline' function attributes to prevent compiler optimization from removing logic it might consider unnecessary.
 * @param	block	the block of memory to be zeroed.
 * @param	len		the number of zero bytes to write to memory.
 * @see	http://gcc.gnu.org/onlinedocs/gcc-4.4.4/gcc/Function-Attributes.html
 */
void * mm_wipe(void *block, size_t len) {

#ifdef MAGMA_PEDANTIC
	if (!block) {
		log_pedantic("Attempting to wipe a NULL block pointer.");
	}
#endif

	if (block && len) {
		mm_set(block, 0, len);
	}

	return block;
}

/**
 * @brief	Free a block of memory.
 * @note	block can point to either a secure or insecure memory block.
 * @param	block	a pointer to the block to be freed.
 * @return	This function does not return any value.
 */
// QUESTION: the note of this function really makes no sense when compared to the pedantic block below.
void mm_free(void *block) {

#ifdef MAGMA_PEDANTIC
	if (!block) {
		log_pedantic("Attempted to free a NULL pointer.");
	}
	else if (mm_sec_secured(block)) {
		log_pedantic("Attempting to free a block of memory inside the secure address range.");
	}
#endif

	if (block) {
		free(block);
	}

	return;
}

/**
 * @brief	Duplicate a block of memory.
 * @param	block	a pointer to the block of memory to be duplicated.
 * @param	len		the length, in bytes, of the buffer to be duplicated.
 * @return	a freshly allocated buffer containing a copy of the input data.
 */
void * mm_dupe(void *block, size_t len) {

	void *result = NULL;

#ifdef MAGMA_PEDANTIC
	if (!block) {
		log_pedantic("Attempting to dupe a NULL block pointer.");
	}
	else if (!len) {
		log_pedantic("Attempting to dupe a zero length memory block.");
	}
#endif

	if (block && len && (result = mm_alloc(len))) {
		mm_copy(result, block, len);
	}

	return result;
}

/**
 *
 * @brief	Allocate a chunk of memory with the system allocator and zero-wipe it.
 * @note	Uses the 'malloc' function attribute to indicate any non-NULL return value is not an alias for any other valid pointer.
 * @note	The buffer length must be non-zero.
 * @see		http://gcc.gnu.org/onlinedocs/gcc-4.4.4/gcc/Function-Attributes.html
 * @param	len	the amount of memory to allocate.
 * @return	a valid pointer to the allocated memory on success, or NULL on error.
 */
void * mm_alloc(size_t len) {

	void *result;

	if (!len) {
		log_pedantic("Attempted to allocate a zero length string.");
		return NULL;
	} else if ((result = malloc(len))) {
		mm_set(result, 0, len);
	} else {
		log_pedantic("Unable to allocate a block of %zu bytes.", len);
	}

	return result;
}
