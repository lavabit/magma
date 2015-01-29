
/**
 * @file /magma/core/buckets/buckets.h
 *
 * @brief	The function declarations and types for the thread-safe object pool interface.
 *
 * $$Author$$
 * $$Date$$
 * $$Revision$$
 *
 */

#ifndef MAGMA_CORE_BUCKETS_H
#define MAGMA_CORE_BUCKETS_H

/**
 *  The maximum number of objects allowed inside a pool.
 */
#define MAGMA_CORE_POOL_OBJECTS_LIMIT 4096

/**
 *  The maximum number of seconds a pool can be configured to wait in seconds.
 */
#define MAGMA_CORE_POOL_TIMEOUT_LIMIT 86400

// Defines for the array type.
#define ARRAY_MAX_ELEMENTS 16384
#define ARRAY_TYPE_EMPTY 0
#define ARRAY_TYPE_ARRAY 1
#define ARRAY_TYPE_STRINGER 2
#define ARRAY_TYPE_SIZER 3
#define ARRAY_TYPE_NULLER 4
#define ARRAY_TYPE_PLACER 5
#define ARRAY_TYPE_POINTER 6

// TODO: Someday arrays should go away and we should use the inx interfaces instead.
typedef char array_t;

typedef enum {
	PL_ERROR = -1,
	PL_AVAILABLE = 0,
	PL_RESERVED = 1
} M_POOL_STATUS;

typedef M_POOL_STATUS status_t;

typedef struct {
	void *data;
	struct stacker_node_t *next;
} stacker_node_t;

typedef struct {
	uint64_t items;
	pthread_mutex_t mutex;
	stacker_node_t *list, *last;
	void (*free_function)(void *data);
} stacker_t;

typedef struct {
	uint32_t count; /* Number of objects allocated. */
	uint32_t timeout; /* How long to wait for an object before timing out. Zero is forever. */
	uint64_t failures; /* Tracks the number of times a thread was forced to return empty handed. */
	sem_t available; /* Semaphore holding the number of objects currently available. */
	pthread_mutex_t lock; /* Mutex for locking coordinating updates between threads. */
	status_t *status; /* Array of booleans to indicate object availability. */
	void **objects; /* Array of objects. */
} pool_t;

// Pool interface
void pool_free(pool_t *pool);
uint32_t pool_get_count(pool_t *pool);
uint32_t pool_get_timeout(pool_t *pool);
uint64_t pool_get_failures(pool_t *pool);
uint32_t pool_get_available(pool_t *pool);
pool_t * pool_alloc(uint32_t count, uint32_t timeout);

// Status interface
status_t pool_get_status(pool_t *pool, uint32_t item);
status_t pool_set_status(pool_t *pool, uint32_t item, status_t status);

// Object interface
void pool_release(pool_t *pool, uint32_t item);
void * pool_get_obj(pool_t *pool, uint32_t item);
status_t pool_pull(pool_t *pool, uint32_t *item);
void * pool_swap_obj(pool_t *pool, uint32_t item, void *object);
void * pool_set_obj(pool_t *pool, uint32_t item, void *object);

/// arrays.c
array_t *     ar_alloc(size_t size);
int_t         ar_append(array_t **array, uint32_t type, void *item);
size_t        ar_avail_get(array_t *array);
array_t *     ar_dupe(array_t *array);
array_t *     ar_field_ar(array_t *array, size_t element);
chr_t *       ar_field_ns(array_t *array, size_t element);
placer_t *    ar_field_pl(array_t *array, size_t element);
void *        ar_field_ptr(array_t *array, size_t element);
stringer_t *  ar_field_st(array_t *array, size_t element);
uint32_t      ar_field_type(array_t *array, size_t element);
void          ar_free(array_t *array);
size_t        ar_length_get(array_t *array);
void          ar_length_set(array_t *array, size_t used);

/// stacked.c
int_t stacker_push(stacker_t *stack, void *data);
stacker_t * stacker_alloc(void *free_function);
unsigned long stacker_nodes(stacker_t *stack);
void * stacker_pop(stacker_t *stack);
void stacker_free(stacker_t *stack);


#endif
