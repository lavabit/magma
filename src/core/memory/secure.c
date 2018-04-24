
/**
 * @file /magma/core/memory/secure.c
 *
 * @brief	Functions for allocating secure memory. Secure buffers should always be used to hold sensitive information.
 */

#include "magma.h"



enum {
	MM_SEC_CHUNK_AVAILABLE = 0,
	MM_SEC_CHUNK_ALLOCATED = 1
};

typedef struct __attribute__ ((packed)) {
	uint32_t flags;
	size_t length;
} secured_t;

static struct {

	struct {
		void *data_true;
		void *data;
		size_t length;
		size_t length_true;
		pthread_mutex_t lock;
	} slab;

	struct {
		size_t items;
		size_t bytes;
	} allocated;

	bool_t enabled;

} secure = {

	.slab = {
	.data = NULL,
	.length = 0,
	.lock = PTHREAD_MUTEX_INITIALIZER
	},

	.allocated = {
		.items = 0,
		.bytes = 0
	},

	.enabled = false
};

/**
 * @brief	Get the collected secure memory statistics for the caller.
 * @param	total	a pointer to a size_t variable that will store the secure memory region length, in bytes.
 * @param	bytes	a pointer to a size_t variable that will store the number of secure bytes allocated by magma.
 * @param	items	a pointer to a size_t variable that will store the number of secure memory allocations requested by magma.
 * @return	true on success or false on failure.
 */
bool_t mm_sec_stats(size_t *total, size_t *bytes, size_t *items) {

	if (!secure.enabled || !secure.slab.data || !total || !bytes || !items) {
		return false;
	}

	mutex_lock(&secure.slab.lock);
	*total = secure.slab.length;
	*bytes = secure.allocated.bytes;
	*items = secure.allocated.items;
	mutex_unlock(&secure.slab.lock);

	return true;
}

/**
 * @brief	Determine whether the data pointer falls within the secure memory block.
 * @param	block	the data pointer to be tested.
 * @return	true if block points to secure data; false if not, or if block is invalid or secure memory is disabled.
 */
bool_t mm_sec_secured(void *block) {

	size_t input, slab;

	if (!block || !secure.enabled || !secure.slab.data) {
		return false;
	}

	// Cast the provided pointer and the start of the secure memory slab into arithmetic operations on numbers are universal while arithmetic operations on pointers
	// are vary across platforms.
	input = (size_t)block;
	slab = (size_t)secure.slab.data;

	// If the value of input falls within the range of our secure memory slab we let our customer know by returning a value of true.
	return input >= slab && input < (slab + secure.slab.length) ? true : false;
}

/**
 * @brief	Get the next chunk of secure memory.
 * @param	chunk	the input secure chunk.
 * @return	a pointer to the next chunk of secure memory, or NULL on failure or if the end of the slab is reached.
 */
secured_t * mm_sec_chunk_next(secured_t *chunk) {

	secured_t *next;

	next = (secured_t *)((chr_t *)chunk + sizeof(secured_t) + chunk->length);
	if (!mm_sec_secured(next)) next = NULL;

	return next;
}

/**
 * @brief	Get the previous chunk of secure memory.
 * @param	chunk	the input secure chunk.
 * @return	a pointer to the previous chunk of secure memory, or NULL on failure or if the beginning of the slab is reached.
 */
secured_t * mm_sec_chunk_prev(secured_t *chunk) {

	secured_t *prev, *next;

	if (chunk == secure.slab.data) {
		prev = NULL;
	}
	else {
		prev = (secured_t *)secure.slab.data;
		while ((next = mm_sec_chunk_next(prev)) && next != chunk) prev = next;
	}

	return prev;
}

/**
 * @brief  If an adjacent region is available, merge them together.
 */
void mm_sec_chunk_merge(secured_t *chunk) {

	secured_t *prev, *next;

	prev = mm_sec_chunk_prev(chunk);
	next = mm_sec_chunk_next(chunk);

	if (prev && (!(prev->flags & MM_SEC_CHUNK_ALLOCATED))) {
		prev->length += sizeof(secured_t) + chunk->length;
		chunk = prev;
	}

	if (next && (!(next->flags & MM_SEC_CHUNK_ALLOCATED))) {
		chunk->length += sizeof(secured_t) + next->length;
	}

	return;
}

/**
 * @brief  Locates a properly sized chunk of memory and reserves it.
 */
secured_t * mm_sec_chunk_new(secured_t *block, size_t size) {

	bool_t loop = true;
	secured_t *chunk, *split;

	chunk = block;
	while(loop && mm_sec_secured(chunk)) {

		if (!(chunk->flags & MM_SEC_CHUNK_ALLOCATED) && chunk->length >= size) {

			// Flag the chunk as allocated.
			chunk->flags |= MM_SEC_CHUNK_ALLOCATED;

			// If splitting the chunk would yield any bytes beyond the overhead.
			if ((chunk->length - size) > sizeof(secured_t)) {
				split = (secured_t *)(((chr_t *)chunk) + sizeof(secured_t) + size);
				split->length = chunk->length - size - sizeof(secured_t);
				split->flags = 0;
				chunk->length = size;
				mm_sec_chunk_merge(split);
			}

			loop = false;
		}
		else {
			chunk = mm_sec_chunk_next(chunk);
		}
	}

	// If we end up reaching the end of the secure address space, return NULL.
	if (!mm_sec_secured(chunk))	chunk = NULL;

	return chunk;
}

/**
 * @brief	Free a secure memory block and perform a multi-pass wipe of its contents.
 * @return	This function returns no value.
 */
void mm_sec_free(void *block) {

	size_t len;
	secured_t *chunk;

#ifdef MAGMA_PEDANTIC
	if (!mm_sec_secured(block)) {
		log_options(M_LOG_PEDANTIC | M_LOG_STACK_TRACE, "The secure memory system was asked to free an non-secure address.");
	}
#endif

	if (block && secure.enabled && mm_sec_secured(block)) {
		chunk = (secured_t *)((chr_t *)block - sizeof(secured_t));
		len = chunk->length;

		// Wipe the data segment three times to ensure sensitive information isn't leaked.
		mm_set(block, 255, len);
		mm_set(block, 128, len);
		mm_set(block, 0, len);

		mutex_lock(&secure.slab.lock);

		secure.allocated.items--;
		secure.allocated.bytes -= len;

		chunk->flags &= ~MM_SEC_CHUNK_ALLOCATED;
		mm_sec_chunk_merge(chunk);

		mutex_unlock(&secure.slab.lock);
	}

	return;
}

/**
 * @brief	Performed a checked memory free.
 * @see		mm_sec_free
 * @param	block	the block of memory to be freed.
 * @return	This function returns no value.
 */
void mm_sec_cleanup(void *block) {

	if (block) {
		mm_sec_free(block);
	}

	return;
}

/**
 * @brief	Allocate a chunk of memory from the secure memory slab
 * @see		mm_sec_chunk_new()
 * @param	len		the length, in bytes, of the secure memory chunk to be allocated.
 * @return	NULL on failure, or a pointer to the freshly allocated chunk of secure memory on success.
 */
void * mm_sec_alloc(size_t len) {

	secured_t *chunk;
	void *result = NULL;

	if (!secure.enabled || !secure.slab.data || !len) {
		return NULL;
	}

	// Align allocations to a length of 12 bytes, which is the size of our secured_t structure.
	len = align(16, len);

	mutex_lock(&secure.slab.lock);

	if ((chunk = mm_sec_chunk_new(secure.slab.data, len))) {
		secure.allocated.items++;
		secure.allocated.bytes += len;
	}

	mutex_unlock(&secure.slab.lock);

	if (chunk) {
		result = ((chr_t *)chunk + sizeof(secured_t));
		mm_wipe(result, len);
	}

#ifdef MAGMA_PEDANTIC
	else {
		log_options(M_LOG_PEDANTIC | M_LOG_STACK_TRACE, "Secure memory allocation failed. {len = %zu}", len);
		size_t total, bytes, items;
		mm_sec_stats(&total, &bytes, &items);
		log_pedantic("secmem usage: %lu/%lu bytes in %lu chunks\n",	bytes, total, items);

		//log_pedantic("Secure memory allocation failed. {len = %zu}", len);

	}
#endif

	return result;
}

/**
 * @brief	Allocates a larger block of secure memory if requested. Depends on allocation/free routines to lock the required mutex. If a new block
 *		is allocated, the original data is copied and then the block is freed. In the event of an error, the original block is preserved and NULL is returned.
 */
void * mm_sec_realloc(void *orig, size_t len) {

	size_t olen;
	void *result;
	secured_t *chunk;

	if (!secure.enabled || !secure.slab.data || !orig || !len) {
#ifdef MAGMA_PEDANTIC
		if (!len) log_pedantic("Secure reallocation request is for a zero length block! {len = %zu}", len);
#endif
		return NULL;
	}

	chunk = (secured_t *)((chr_t *)orig - sizeof(secured_t));
	olen = chunk->length;

	// Requests that would shrink the chunk by less than 256 bytes probably aren't worth the overhead to process.
	if (len <= olen && (olen - len) >= 256) {
		result = orig;
	}
	else if ((result = mm_sec_alloc(len))) {
		mm_copy(result, orig, len < olen ? len : olen);
		mm_sec_free(orig);
	}

	return result;
}

/**
 * @brief	Deallocate and perform a multi-stage secure wipe of the secure memory region.
 * @return	This function returns no value.
 */
void mm_sec_stop(void) {

	if (secure.enabled && secure.slab.data) {

		mm_set(secure.slab.data, 255, secure.slab.length);
		mm_set(secure.slab.data, 128, secure.slab.length);
		mm_set(secure.slab.data, 64, secure.slab.length);
		mm_set(secure.slab.data, 32, secure.slab.length);
		mm_set(secure.slab.data, 0, secure.slab.length);

		munlock(secure.slab.data, secure.slab.length);

		//BUG: Need to add in calculations
		munmap(secure.slab.data_true, secure.slab.length_true);

		secure.slab.data = secure.slab.data_true = NULL;
		secure.slab.length = secure.slab.length_true = 0;

	}

	return;
}

/**
 * @brief	If enabled, allocate and initialize the secure memory slab.
 * @note	This function will mmap a page-aligned secure memory slab (defaults to 32768 bytes long), mlock() it into memory, and zero-wipe it.
 * 			Guard pages with empty permissions are created on the boundaries of the slab to prevent memory bungling.
 * @return	true if the secure memory slab has been initialized, or false if the process fails.
 */
bool_t mm_sec_start(void) {

	uchr_t *bndptr;
	size_t alignment;
	secured_t *chunk;
#ifdef MAGMA_H
	if (!(secure.enabled = magma.secure.memory.enable)) {
		log_pedantic("Secure memory management disabled.");
		return true;
	}
#endif


	// Ensure the page length is positive.
#ifdef MAGMA_H
	if ((alignment = magma.page_length) <= 0) {
#else
	if ((alignment = CORE_PAGE_LENGTH) <= 0) {
#endif
		log_pedantic("Invalid page size.");
		return false;
	}
	// If the page length is smaller than MM_SEC_PAGE_ALIGNMENT_MIN bytes, replace it with an aligned value of at least MM_SEC_PAGE_ALIGNMENT_MIN.
	else if (alignment < MM_SEC_PAGE_ALIGNMENT_MIN) {
		 alignment = (MM_SEC_PAGE_ALIGNMENT_MIN + alignment - 1) & ~(alignment - 1);
	}

	// Ensure the default length for secure memory slabs is greater than zero and is aligned by the page table size.
#ifdef MAGMA_H
	if ((secure.slab.length = (magma.secure.memory.length + alignment - 1) & ~(alignment - 1)) < MM_SEC_POOL_LENGTH_MIN) {
#else
		if ((secure.slab.length = (CORE_SECURE_MEMORY_LENGTH + alignment - 1) & ~(alignment - 1)) < MM_SEC_POOL_LENGTH_MIN) {
#endif
		log_pedantic("The secure memory pool size is too small. { length = %zu / min = %i }", secure.slab.length, MM_SEC_POOL_LENGTH_MIN);
		return false;
	}

	// Allocate secured boundary pages around the secure slab to prevent against memory underflows and overflows.
#ifdef MAGMA_H
	secure.slab.length_true = secure.slab.length + (magma.page_length * 2);
#else
	secure.slab.length_true = secure.slab.length + (CORE_PAGE_LENGTH * 2);
#endif
	// Request an anonymous memory mapping that is aligned according to the system page size. Were asking the kernel to lock returned block into memory.
	if ((secure.slab.data_true = mmap64(NULL, secure.slab.length_true, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_LOCKED, -1, 0)) == MAP_FAILED) {
		log_pedantic("Unable to memory map an anonymous file. { error = %s }", strerror_r(errno, MEMORYBUF(1024), 1024));
		return false;
	}

	bndptr = secure.slab.data_true;
#ifdef MAGMA_H
	if (mprotect(bndptr, magma.page_length, PROT_NONE)) {
#else
		if (mprotect(bndptr, CORE_PAGE_LENGTH, PROT_NONE)) {
#endif
		log_pedantic("Unable to set protections on lower secure memory boundary chunk.");
		return false;
	}
#ifdef MAGMA_H
	bndptr += magma.page_length;
#else
	bndptr += CORE_PAGE_LENGTH;
#endif
	secure.slab.data = bndptr;
	bndptr += secure.slab.length;
#ifdef MAGMA_H
	if (mprotect(bndptr, magma.page_length, PROT_NONE)) {
#else
		if (mprotect(bndptr, CORE_PAGE_LENGTH, PROT_NONE)) {
#endif
		log_pedantic("Unable to set protections on upper secure memory boundary chunk.");
		return false;
	}

	// We also request the address range assigned be locked into memory using the mlock call.
	if (mlock(secure.slab.data, secure.slab.length)) {
		log_pedantic("Unable to lock the address space reserved for sensitive data in memory.");
		// BUG: Need to add in calculations
		munmap(secure.slab.data_true, secure.slab.length_true);
		secure.slab.data = secure.slab.data_true = NULL;
		return false;
	}

	mm_wipe(secure.slab.data, secure.slab.length);

	chunk = (secured_t *)secure.slab.data;
	chunk->length = secure.slab.length - sizeof(secured_t);
	chunk->flags = 0;

	return true;
}
