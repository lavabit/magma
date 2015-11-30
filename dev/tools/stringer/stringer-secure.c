/**
 * @file /stringer/stringer-secure.c
 *
 * @brief Functions used for allocating secure blocks of memory suitable for holding sensitive information.
 *
 * $Author: Ladar Levison $
 * $Date: 2010/12/12 23:59:04 $
 * $Revision: a46af76362b976a414117a5bfee9dfc2e9fd38b2 $
 *
 */

#include "stringer.h"

typedef struct {
	uint32_t flags;
	size_t length;
} __attribute__ ((packed)) secured_t;

enum {
	MM_SEC_CHUNK_AVAILABLE = 0,
	MM_SEC_CHUNK_ACTIVE = 1
};

// Supply a default secured memory length of 64 kilobytes; which should fall below the locked memory limit for unprivileged user accounts.
void *mm_sec_slab = NULL;
size_t mm_sec_slab_length = 65536;
pthread_mutex_t mm_sec_lock =	PTHREAD_MUTEX_INITIALIZER;

bool_t mm_sec_enable = true;
uint64_t mm_sec_bytes_allocated = 0, mm_sec_chunks_allocated = 0;

void mm_sec_dump_stats() {

	log_pedantic("secmem usage: %lu/%lu bytes in %lu chunks\n",
			mm_sec_bytes_allocated, (unsigned long)mm_sec_slab_length, mm_sec_chunks_allocated);
	return;
}

/* Update the statistics.  */
void stats_update(size_t add, size_t sub) {
	if (add) {
		mm_sec_bytes_allocated += add;
		mm_sec_chunks_allocated++;
	}
	if (sub) {
		mm_sec_bytes_allocated -= sub;
		mm_sec_chunks_allocated--;
	}
}

// Returns true if the pointer address is inside the secure memory block.
bool_t mm_sec_secured(void *ptr) {

	size_t input, slab_start;

	if (!ptr || !mm_sec_enable || !mm_sec_slab) {
		return false;
	}

	// Cast the provided pointer and the start of the secure memory slab into arithmetic operations on numbers are universal while arithmetic operations on pointers
	// are vary across platforms.
	input = (size_t)ptr;
	slab_start = (size_t)mm_sec_slab;

	// If the value of input falls within the range of our secure memory slab we let our customer know by returning a value of true.
	return input >= slab_start && input < slab_start + mm_sec_slab_length ? true : false;
}

// Returns the next chunk, or NULL if the end of the slab is reached.
secured_t * mm_sec_chunk_next(secured_t *chunk) {

	secured_t *chunk_next;

	chunk_next = (secured_t *)((chr_t *)chunk + sizeof(secured_t) + chunk->length);

	if (!mm_sec_secured(chunk_next))
		chunk_next = NULL;

	return chunk_next;
}

// Returns the previous chunk, or NULL if the start of the slab is reached.
secured_t * mm_sec_chunk_prev(secured_t *chunk) {

	secured_t *chunk_prev, *chunk_next;

	if (chunk == mm_sec_slab) {
		chunk_prev = NULL;
	}
	else {
		chunk_prev = (secured_t *)mm_sec_slab;
		while (1) {
			chunk_next = mm_sec_chunk_next(chunk_prev);
			if (chunk_next == chunk)
				break;
			else
				chunk_prev = chunk_next;
		}
	}

	return chunk_prev;
}

// If an adjacent region is available, merge them together.
void mm_sec_chunk_merge(secured_t *chunk) {

	secured_t *chunk_prev, *chunk_next;

	chunk_prev = mm_sec_chunk_prev(chunk);
	chunk_next = mm_sec_chunk_next(chunk);

	if (chunk_prev && (!(chunk_prev->flags & MM_SEC_CHUNK_ACTIVE))) {
		chunk_prev->length += sizeof(secured_t) + chunk->length;
		chunk = chunk_prev;
	}

	if (chunk_next && (!(chunk_next->flags & MM_SEC_CHUNK_ACTIVE))) {
		chunk->length += sizeof(secured_t) + chunk_next->length;
	}

	return;
}

// Locates a properly sized chunk of memory and reserves it.
secured_t * mm_sec_chunk_new(secured_t *block, size_t size) {

	secured_t *chunk, *chunk_split;

	for (chunk = block; mm_sec_secured(chunk); chunk = mm_sec_chunk_next(chunk)) {
		if (!(chunk->flags & MM_SEC_CHUNK_ACTIVE) && chunk->length >= size) {
			/* Found a free block.  */
			chunk->flags |= MM_SEC_CHUNK_ACTIVE;

			if (chunk->length - size > sizeof(secured_t)) {
				/* Split block.  */

				chunk_split = (secured_t *)(((chr_t *)chunk) + sizeof(secured_t) + size);
				chunk_split->length = chunk->length - size - sizeof(secured_t);
				chunk_split->flags = 0;

				chunk->length = size;

				mm_sec_chunk_merge(chunk_split);
			}

			break;
		}
	}

	if (!mm_sec_secured(chunk))
		chunk = NULL;

	return chunk;
}

void mm_sec_free(void *chunk) {

	size_t size;
	secured_t *mem;

	if (!chunk) {
		return;
	}

	mem = (secured_t *)((chr_t *)chunk - sizeof(secured_t));
	size = mem->length;

	mm_set(chunk, 255, size);
	mm_set(chunk, 128, size);
	mm_set(chunk, 0, size);

	pthread_mutex_lock(&mm_sec_lock);
	stats_update(0, size);
	mem->flags &= ~MM_SEC_CHUNK_ACTIVE;
	mm_sec_chunk_merge(mem);
	pthread_mutex_unlock(&mm_sec_lock);
	return;
}

void * mm_sec_alloc(size_t length) {

	secured_t *mem;

	if (!mm_sec_enable || !mm_sec_slab || !length) {
		return NULL;
	}

	// Align allocations on a 32 byte boundaries.
	length = (length + 32 - 1) & ~(32 - 1);

	pthread_mutex_lock(&mm_sec_lock);
	mem = mm_sec_chunk_new((secured_t *)mm_sec_slab, length);
	pthread_mutex_unlock(&mm_sec_lock);

	if (mem) {
		mm_set(((chr_t *)mem + sizeof(secured_t)), 0, length);
		stats_update(length, 0);
	}

	return mem ? ((chr_t *)mem + sizeof(secured_t)) : NULL;
}

// Allocates a larger block of secure memory if requested. Depends on allocation/free routines to lock the required mutex. If a new block
// is allocated, the original data is copied and then the block is freed. In the event of an error, the original block is preserved and NULL is returned.
void * mm_sec_realloc(void *orig, size_t len) {

	size_t olen;
	void *result;
	secured_t *mem;

	mem = (secured_t *)((chr_t *)orig - sizeof(secured_t));
	olen = mem->length;

	// Requests that would shrink the chunk by less than 1,024 bytes probably aren't worth the overhead to process.
	if (len <= olen && (olen - len) >= 1024) {
		result = orig;
	}

	else if ((result = mm_sec_alloc(len))) {
			mm_copy(result, orig, len <= olen ? len : olen);
			mm_sec_free(orig);
	}

	return result;
}

void mm_sec_stop(void) {

	if (!mm_sec_enable || !mm_sec_slab) {
		return;
	}

	mm_set(mm_sec_slab, 255, mm_sec_slab_length);
	mm_set(mm_sec_slab, 128, mm_sec_slab_length);
	mm_set(mm_sec_slab, 64, mm_sec_slab_length);
	mm_set(mm_sec_slab, 32, mm_sec_slab_length);
	mm_set(mm_sec_slab, 0, mm_sec_slab_length);

	munlock(mm_sec_slab, mm_sec_slab_length);
	munmap(mm_sec_slab, mm_sec_slab_length);
	mm_sec_slab = NULL;

	return;
}

bool_t mm_sec_start(void) {

	uid_t uid;
	secured_t *mem;
	size_t pagesize;
	chr_t buf[1024];

	if (!mm_sec_enable) {
		log_pedantic("Secure memory management disabled.");
		return true;
	}

	if ((pagesize = getpagesize()) <= 0) {
		log_pedantic("Invalid page size.");
		return false;
	}

	// Ensure the default length for secure memory slabs is greater than zero and is aligned by the page table size.
	else if (!(mm_sec_slab_length = (mm_sec_slab_length + pagesize - 1) & ~(pagesize - 1))) {
		log_pedantic("Invalid size supplied for the secure memory block.");
		return false;
	}

	else if ((mm_sec_slab = mmap64(NULL, mm_sec_slab_length, PROT_READ | PROT_WRITE, MAP_PRIVATE | MAP_ANONYMOUS | MAP_LOCKED, -1, 0)) == MAP_FAILED) {
		log_pedantic("Unable to memory map an anonymous file. {%s}", strerror_r(errno, buf, 1024));
		return false;
	}

	else if (mlock(mm_sec_slab, mm_sec_slab_length)) {
		log_pedantic("Unable to lock the address space reserved for sensitive data in memory.");
		munmap(mm_sec_slab, mm_sec_slab_length);
		mm_sec_slab = NULL;
		return false;
	}

	else if (((uid = getuid()) && !geteuid()) && ((setuid(uid) || getuid() != geteuid() || !setuid(0)))) {
		log_pedantic("Failed to reset effective permissions. {%s}", strerror_r(errno, buf, 1024));
		munmap(mm_sec_slab, mm_sec_slab_length);
		mm_sec_slab = NULL;
		return false;
	}

	mm_set(mm_sec_slab, 0, mm_sec_slab_length);

	mem = (secured_t *)mm_sec_slab;
	mem->length = mm_sec_slab_length;
	mem->flags = 0;

	return true;
}
