/**
 * @file /stringer/memory.c
 *
 * @brief The functions used to handle Magma memory buffers.
 *
 * $Author: $
 * $Date: $
 * $Revision: $
 *
 */

#include "stringer.h"

/**
 *
 * @param target the target block
 * @param source the source of the copy
 * @param length the amount of data to copy
 * copies source buffer into the target, the buffers should not overlap, if the buffers
 * overlap, use bl_move instead
 */
void * mm_copy(void *dst, void *src, size_t len) {
	return memcpy(dst, src, len);
}

/**
 *
 * @param target the target block
 * @param source the source of the copy
 * @param length the amount of data to copy
 * copies source buffer into the target, the buffers may overlap
 */
void * mm_move(void *dst, void *src, size_t len) {
	return memmove(dst, src, len);
}

bool_t mm_empty(void *block, size_t len) {
	if(!block || !len) return true;
	return false;
}

/**
 * blanks the block of memory
 */
void mm_set(void *block, uchr_t set, size_t len) {
	volatile size_t vlen = (volatile size_t)len;
	volatile uchr_t *vaddr = (volatile uchr_t *)block;
	while (vlen) {
		*vaddr = set;
		vaddr++;
		vlen--;
	}
	return;
}

/**
 * wipes the block of memory
 */
void mm_wipe(void *block, size_t len) {

#ifdef MAGMA_PEDANTIC
	if (!block) log_pedantic("Attempting to wipe a NULL block pointer.");
#endif

	if (block && len) mm_set(block, 0, len);
	return;
}

/**
 * Frees the block of memory.
 *
 * @param block A pointer to the block that will be freed.
 */
void mm_free(void *block) {

#ifdef MAGMA_PEDANTIC
	if (!block) log_pedantic("Attempted to free a NULL pointer.");
	else if (mm_sec_secured(block)) log_pedantic("Attempting to free a block of memory inside the secure address range.");
#endif

	if (block) free(block);
	return;
}

/**
 * @param size amount of memory to allocate
 * @return a valid pointer on success, or NULL on error
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
