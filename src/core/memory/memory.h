
/**
 * @file /magma/core/memory/memory.h
 *
 * @brief The functions used to allocate and manipulate blocks of memory off the heap and inside the secured address space.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_CORE_MEMORY_H
#define MAGMA_CORE_MEMORY_H

/// align.c
size_t align(size_t alignment, size_t len);

// Bit Manipulation
uint_t bits_count(uint64_t value);

// Secure Memory Blocks
void mm_sec_stop(void);
bool_t mm_sec_start(void);
void mm_sec_free(void *block);
bool_t mm_sec_secured(void *block);
void * mm_sec_alloc(size_t len);
void * mm_sec_realloc(void *orig, size_t len);
bool_t mm_sec_stats(size_t *total, size_t *bytes, size_t *items) __attribute__ ((nonnull (1, 2, 3)));

/// memory.c
void *   mm_alloc(size_t len);
void     mm_cleanup(void *block);
void *   mm_copy(void *dst, const void *src, size_t len);
void *   mm_dupe(void *block, size_t len);
bool_t   mm_empty(void *block, size_t len);
void     mm_free(void *block);
void *   mm_move(void *dst, void *src, size_t len);
void *   mm_set(void *block, int_t set, size_t len);
void *   mm_wipe(void *block, size_t len);

// Allocation requests are aligned to 12 bytes, which is also the length of the secured_t.
#define MM_SEC_REQUEST_ALIGNMENT 12

// The page size should be at least one kilobyte.
#define MM_SEC_PAGE_ALIGNMENT_MIN 1024

// The minimum secure memory block length.
#define MM_SEC_POOL_LENGTH_MIN 4096

// Usage: void *buffer = MEMORYBUF(length);
#define MEMORYBUF(l) (void *)&((chr_t []){ [ 0 ... l ] = 0 })

#endif
