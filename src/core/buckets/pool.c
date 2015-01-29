
/**
 * @file /magma/core/buckets/pool.c
 *
 * @brief	A collection of functions used to create, maintain and safely utilize collections of object pointers that are accessed by multiple threads.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Free an object pool.
 * @warning	This function will not free the underlying objects contained by the pool!
 * @param	pool	the pool to be released from memory.
 * @return	This function returns no value.
 */
void pool_free(pool_t *pool) {

	if (!pool)
		return;

	mutex_destroy(&(pool->lock));
	sem_destroy(&(pool->available));
	mm_free(pool);
	return;
}

/**
 * @brief	Allocate a new object pool.
 * @param	count		the number of items the pool can hold.
 * @param	timeout		a timeout for pool requests in seconds (specify 0 for infinite wait).
 * @return	NULL on failure,
 */
pool_t * pool_alloc(uint32_t count, uint32_t timeout) {

	pool_t *pool;
	size_t pool_size = sizeof(pool_t) + (sizeof(status_t) * count) + (sizeof(void *) * count);

	if (count > MAGMA_CORE_POOL_OBJECTS_LIMIT) {
		log_info("%u exceeds the maximum number of pool objects allowed.", count);
		return NULL;
	}

	if (timeout > MAGMA_CORE_POOL_TIMEOUT_LIMIT) {
		log_info("%u exceeds the timeout maximum allowable timeout for a pool.", timeout);
		return NULL;
	}

	// Allocate enough memory for the pool structure, plus the boolean list and object array.
	if (!(pool = mm_alloc(pool_size))) {
		log_info("Unable to allocate %zu bytes for a pool structure.", pool_size);
		return NULL;
	}

	// Initialize.
	pool->count = count;
	pool->timeout = timeout;

	pool->status = (status_t *)((char *)pool + sizeof(pool_t));
	pool->objects = (void *)((char *)pool + sizeof(pool_t) + ((sizeof(status_t) * count)));

	if (sem_init(&(pool->available), 0, count)) {
		log_info("Unable to initialize the pool semaphore.");
		mm_free(pool);
		return NULL;
	}

	if (mutex_init(&(pool->lock), NULL)) {
		log_info("Unable to initialize the pool mutex.");
		sem_destroy(&(pool->available));
		mm_free(pool);
		return NULL;
	}

	return pool;
}

/**
 * @brief	Return the total number of items allocated inside a pool.
 * @note	Some of the item slots may be unused, but remain reserved.
 * @param	pool	the pool to be queried.
 * @return	0 on failure, or the allocation count of the pool on success.
 */
uint32_t pool_get_count(pool_t *pool) {
	if (!pool)
		return 0;
	return pool->count;
}

/**
 * @brief	Return the total number of items actively available in a pool.
 * @param	pool	the pool to be queried.
 * @return	0 on failure, or the total number of items actively available in the specified pool.
 */
uint32_t pool_get_available(pool_t *pool) {
	int available;
	if (!pool || !sem_getvalue(&(pool->available), &available))
		return 0;
	return available;
}

/**
 * @brief	Get the timeout value for a pool.
 * @note	A timeout of zero will cause threads to wait forever.
 * @param pool	a pointer to the pool to be examined.
 * @return	the timeout value of the specified pool, in seconds.
 */
uint32_t pool_get_timeout(pool_t *pool) {
	if (!pool)
		return 0;
	return pool->timeout;
}

/**
 * @brief	Get the number of failed requests for a pool.
 * @note	Most of the time, a failure will correspond to a timeout, but can also be triggered by other error conditions.
 * @param pool	a pointer to the pool to be examined.
 * @return	the number of failed requests made on the specified pool.
 */
uint64_t pool_get_failures(pool_t *pool) {

	uint64_t result;
	if (!pool)
		return 0;

	mutex_lock(&(pool->lock));
	result = pool->failures;
	mutex_unlock(&(pool->lock));

	return result;
}

/**
 * @brief	Get the status flag for an item in a pool.
 * @note	A value of PL_AVAILABLE indicates the object is available for use,; PL_RESERVED indicates the object is in use by a worker thread.
 * @param	pool	the pool containing the specified item.
 * @param	item	the identifier of the item to be queried.
 * @return 	PL_ERROR on failure; otherwise, the status of the specified item.
 */
status_t pool_get_status(pool_t *pool, uint32_t item) {

	if (!pool) {
		log_pedantic("A NULL pointer was passed in.");
		return PL_ERROR;
	}

	log_check(*(pool->status + item) != PL_AVAILABLE && *(pool->status + item) != PL_RESERVED);

	return *(pool->status + item);
}

/**
 * @brief	Set the status flag for an item in a pool.
 * @note	A value of PL_AVAILABLE indicates the object is available for use,; PL_RESERVED indicates the object is in use by a worker thread.
 * @param	pool 	the pool containing the specified item.
 * @param	item	the identifier of the item to be adjusted.
 * @param	status	the new status for the item.
 * @return	PL_ERROR on failure; otherwise, the new status value of the specified item.
 */
status_t pool_set_status(pool_t *pool, uint32_t item, status_t status) {

	if (!pool) {
		log_pedantic("A NULL pointer was passed in.");
		return PL_ERROR;
	}

	log_check(status != PL_AVAILABLE && status != PL_RESERVED);

	return *(pool->status + item) = status;
}

/**
 * @brief	Return the first available object in a pool.
 * @note	If no object can be returned immediately, wait for the pool's configured timeout value, in seconds, for
 * 			an object to become available. If the timeout is zero, wait indefinitely.
 * @param	item	A pointer to a number that will store the zero-based indexed of the first available item in the pool.
 * @return	PL_RESERVED on success or PL_ERROR if an object couldn't be reserved.
 */
status_t pool_pull(pool_t *pool, uint32_t *item) {

	status_t result = PL_ERROR;
	struct timespec timeout;

	if (!pool || !item)
		return PL_ERROR;

	if (pool->timeout != 0) {

		if (clock_gettime(CLOCK_REALTIME, &timeout))
			return PL_ERROR;

		timeout.tv_sec += pool->timeout;

		if (sem_timedwait(&(pool->available), &timeout)) {
			mutex_lock(&(pool->lock));
			pool->failures++;
			mutex_unlock(&(pool->lock));
			return PL_ERROR;
		}

	} else {
		sem_wait(&(pool->available));
	}

	mutex_lock(&(pool->lock));
	for (uint32_t i = 0; result == PL_ERROR && i < pool->count; i++) {
		if (pool_get_status(pool, i) == PL_AVAILABLE) {
			result = pool_set_status(pool, i, PL_RESERVED);
			*item = i;
		}
	}

	if (result == PL_ERROR)
		pool->failures++;

	mutex_unlock(&(pool->lock));

	return result;
}

/**
 * @brief	Return an object to a pool and set its status to PL_AVAILABLE.
 * @param	pool 	the pool tracking the returned item.
 * @param	item	the identifier of the item being returned.
 * @return	This function returns no value.
 */
void pool_release(pool_t *pool, uint32_t item) {
	if (!pool)
		return;
	mutex_lock(&(pool->lock));
	pool_set_status(pool, item, PL_AVAILABLE);
	mutex_unlock(&(pool->lock));
	sem_post(&(pool->available));
}

/**
 * @brief	Return a specified object from a pool.
 * @param	pool	the pool containing the object.
 * @param	item	the identifier of the object to query.
 * @return	NULL on failure, or the object corresponding to the specified item number.
 */
void * pool_get_obj(pool_t *pool, uint32_t item) {

	if (!pool) {
		return NULL;
	}

#ifdef MAGMA_PEDANTIC
	mutex_lock(&(pool->lock));
	if (item >= pool_get_count(pool)) {
		mutex_unlock(&(pool->lock));
		log_pedantic("The item number provided (%u) is outside the valid range.", item);
		return NULL;
	}
	mutex_unlock(&(pool->lock));
#endif

	return *(pool->objects + item);
}

/**
 * @brief	Set the object pointer for a given item in a pool.
 * @param	pool	the pool containing the specified item.
 * @param	item	the item id to be adjusted.
 * @param	object	the new value of the object corresponding to the specified item.
 * @return	NULL on failure, or a pointer to the object that was set.
 */
void * pool_set_obj(pool_t *pool, uint32_t item, void *object) {

	if (!pool) {
		return NULL;
	}

#ifdef MAGMA_PEDANTIC
	mutex_lock(&(pool->lock));
	if (item >= pool_get_count(pool)) {
		mutex_unlock(&(pool->lock));
		log_pedantic("The item number provided (%u) is outside the valid range.", item);
		return NULL;
	}
	mutex_unlock(&(pool->lock));
#endif

	return *(pool->objects + item) = object;
}

/**
 * @brief	Wait to acquire the value of an object in a pool, and return the original value while updating it with a new value.
 * @param	pool	a pointer to the pool to be modified.
 * @param	item	the zero-based index of the object in the pool to be updated.
 * @param	object	the new value of the object corresponding to the specified item.
 * @return	a pointer to the original pool object's value, or NULL on failure.
 */
void * pool_swap_obj(pool_t *pool, uint32_t item, void *object) {

	bool_t loop = true;
	void *current = NULL;
	struct timespec delay;

	if (!pool)
		return NULL;

	// 10000000 nanoseconds should be equivalent to 0.01 seconds.
	delay.tv_sec = 0;
	delay.tv_nsec = 10000000;

	do {
		mutex_lock(&(pool->lock));
		if (pool_get_status(pool, item) == PL_AVAILABLE) {
			current = pool_get_obj(pool, item);
			pool_set_obj(pool, item, object);
			loop = false;
		}
		mutex_unlock(&(pool->lock));

		/// LOW: Currently the function loops until the requested object is available. A superior implementation would hook into the release function and detect when
		/// the desired object is available and perform the swap at that point.
		if (loop) {
			nanosleep(&delay, NULL);
		}

	} while (loop);

	return current;
}
