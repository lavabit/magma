
/**
 * @file /magma/providers/consumers/cache.c
 *
 * @brief	Distributed cache interface.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

pool_t *cache_pool = NULL;


/**
 * @brief	Initialize the memcached library and bind dynamically to the exported functions that are required.
 * @return	true on success or false on failure.
 */
bool_t lib_load_cache(void) {

	symbol_t cache[] = {
		M_BIND(memcached_add), M_BIND(memcached_append), M_BIND(memcached_behavior_set), M_BIND(memcached_cas), M_BIND(memcached_create),
		M_BIND(memcached_decrement), M_BIND(memcached_decrement_with_initial), M_BIND(memcached_delete), M_BIND(memcached_flush),
		M_BIND(memcached_free), M_BIND(memcached_get), M_BIND(memcached_increment),	M_BIND(memcached_increment_with_initial),
		M_BIND(memcached_lib_version), M_BIND(memcached_prepend), M_BIND(memcached_replace), M_BIND(memcached_server_add_with_weight),
		M_BIND(memcached_set), M_BIND(memcached_strerror)
	};

	if (lib_symbols(sizeof(cache) / sizeof(symbol_t), cache) != 1) {
		return false;
	}

	return true;
}

/**
 * @brief	Return the version string of the memcached library.
 * @return	a pointer to a character string containing the memcached library version information.
 */
const char * lib_version_cache(void) {
	return memcached_lib_version_d();
}

/*
 * @brief	Create and initialize a pool of binary client connections to the memcached server(s).
 * @return	false on failure or true on success.
 */
bool_t cache_start(void) {

	memcached_st *object;
	memcached_return_t e;

	// Allocate a pool structure for the cache instances.
	if (!(cache_pool = pool_alloc(magma.iface.cache.pool.connections, magma.iface.cache.pool.timeout))) {
		log_critical("Could not allocate memory for the cache connection pool.");
		return false;
	}

	for (uint32_t i = 0; i < magma.iface.cache.pool.connections; i++) {

		// Create the context.
		if (!(object = memcached_create_d(NULL))) {
			log_critical("Could not initialize the cache connection structure.");
			return false;
		}

		// The increment/decrement key with an initial value functions only work if were using the binary protocol.
		else if ((e = memcached_behavior_set_d(object, MEMCACHED_BEHAVIOR_BINARY_PROTOCOL, 1)) != MEMCACHED_SUCCESS) {
			log_critical("Unable to configure the cache context to use the binary protocols. {%s}",  memcached_strerror_d(object, e));
			memcached_free_d(object);
			return false;
		}
		// If the client can't connect to a particular server, it will be removed from the rotation (for ten minutes by default).
		else if ((e = memcached_behavior_set_d(object, MEMCACHED_BEHAVIOR_RETRY_TIMEOUT, magma.iface.cache.retry)) != MEMCACHED_SUCCESS) {
			log_critical("Unable to configure the cache context to use the binary protocols. {%s}",  memcached_strerror_d(object, e));
			memcached_free_d(object);
			return false;
		}
		// If the TCP socket timeout.
		else if ((e = memcached_behavior_set_d(object, MEMCACHED_BEHAVIOR_SND_TIMEOUT, magma.iface.cache.timeout)) != MEMCACHED_SUCCESS ||
			(e = memcached_behavior_set_d(object, MEMCACHED_BEHAVIOR_RCV_TIMEOUT, magma.iface.cache.timeout)) != MEMCACHED_SUCCESS) {
			log_critical("Unable to configure the socket timeout. {%s}",  memcached_strerror_d(object, e));
			memcached_free_d(object);
			return false;
		}

		// Add the cache servers.
		for (uint32_t j = 0; j < MAGMA_CACHE_INSTANCES; j++) {
			if (magma.iface.cache.host[j] && (e = memcached_server_add_with_weight_d(object, magma.iface.cache.host[j]->name, magma.iface.cache.host[j]->port,
					magma.iface.cache.host[j]->weight)) != MEMCACHED_SUCCESS) {
				log_critical("Unable to add the cache instance to our context. {%s}",  memcached_strerror_d(object, e));
				memcached_free_d(object);
				return false;
			}
		}

		pool_set_obj(cache_pool, i, object);
	}

	return true;
}

/**
 * @brief	Destroy the memcached client pool and all active connections.
 * @return	This function returns no value.
 */
void cache_stop(void) {

	memcached_st *object;

	// Destroy the objects.
	for (uint32_t i = 0; i < magma.iface.cache.pool.connections; i++) {
		if ((object = pool_get_obj(cache_pool, i))) memcached_free_d(object);
	}

	// Destroy the pool.
	pool_free(cache_pool);
	cache_pool = NULL;

	return;
}

/*
 * @brief	Flush (wipe) the contents held by memcached.
 * @return	This function returns no value.
 */
void cache_flush(void) {

	uint32_t pool;
	memcached_return_t e;

	if ((pool_pull(cache_pool, &pool)) != PL_RESERVED) {
		return;
	}
	else if ((e = memcached_flush_d(pool_get_obj(cache_pool, pool), 0)) != MEMCACHED_SUCCESS) {
		log_info("Unable to flush distributed caching system. {%s}",  memcached_strerror_d(pool_get_obj(cache_pool, pool), e));
	}

	pool_release(cache_pool, pool);
	return;
}

/**
 * @brief	Retrieve data from memcached by key.
 * @param	key		a managed string containing a key to be passed to memcached.
 * @return	NULL on failure, or a pointer to a managed string containing the cached data on success.
 */
stringer_t * cache_get(stringer_t *key) {

	void *data;
	size_t length = 0;
	uint32_t flags = 0, pool;
	stringer_t *result;
	memcached_return_t error;

	if ((pool_pull(cache_pool, &pool)) != PL_RESERVED) {
		return NULL;
	}
	else if (!(data = memcached_get_d(pool_get_obj(cache_pool, pool), st_char_get(key), st_length_get(key), &length, &flags, &error))) {

		// Error check.
		if (error != MEMCACHED_NOTFOUND) {
			log_info("An error occurred while trying to fetch the %.*s object. {%s}",	st_length_int(key), st_char_get(key), memcached_strerror_d(pool_get_obj(cache_pool, pool), error));
		}

		pool_release(cache_pool, pool);
		return NULL;
	}
	else if (!length) {
		log_info("A non-NULL pointer was returned, but the length was set to zero.");
		pool_release(cache_pool, pool);
		mm_free(data);
		return NULL;
	}

	pool_release(cache_pool, pool);
	result = st_import(data, length);
	mm_free(data);

	return result;
}

/**
 * @brief	Retrieve a 64 bit value from memcached by key.
 * @param	key		a managed string containing a key to be passed to memcached.
 * @return	NULL on failure, or the 64 bit value cached data on success.
 */
uint64_t cache_get_u64(stringer_t *key) {

	void *data;
	size_t length = 0;
	uint64_t result = 0;
	uint32_t flags = 0, pool;
	memcached_return_t error;

	if ((pool_pull(cache_pool, &pool)) != PL_RESERVED) {
		return 0;
	}
	else if ((data = memcached_get_d(pool_get_obj(cache_pool, pool), st_char_get(key), st_length_get(key), &length, &flags, &error)) == NULL) {

		// Error check.
		if (error != MEMCACHED_NOTFOUND) {
			log_info("An error occurred while trying to fetch the %.*s object. {%s}",	st_length_int(key), st_char_get(key), memcached_strerror_d(pool_get_obj(cache_pool, pool), error));
		}

		pool_release(cache_pool, pool);
		return 0;
	}
	else if (!length) {
		log_info("A non-NULL pointer was returned, but the length was set to zero.");
		pool_release(cache_pool, pool);
		mm_free(data);
		return 0;
	}

	// If the result is the correct number of bytes, store it in teh result.
	if (length == sizeof(uint64_t)) {
		result = *((uint64_t *)data);
	}

	pool_release(cache_pool, pool);
	mm_free(data);

	return result;
}

/**
 * @brief	Set a value in memcached by key.
 * @param	key			a managed string containing a key to be passed to memcached.
 * @param	object		a managed string containing the new data to be associated with the supplied key.
 * @param	expiration	the expiration time of the cached data.
 * @return	0 on failure, or 1 on success.
 */
int_t cache_set(stringer_t *key, stringer_t *object, time_t expiration) {

	uint32_t pool;
	memcached_return_t ret;

	if ((pool_pull(cache_pool, &pool)) != PL_RESERVED) {
		return 0;
	}
	else if ((ret = memcached_set_d(pool_get_obj(cache_pool, pool), st_char_get(key), st_length_get(key), st_char_get(object), st_length_get(object), expiration, 0)) != MEMCACHED_SUCCESS) {
		log_info("Unable to store the %.*s object. {%s}", st_length_int(key), st_char_get(key), memcached_strerror_d(pool_get_obj(cache_pool, pool), ret));
		pool_release(cache_pool, pool);
		return 0;
	}
	pool_release(cache_pool, pool);
	return 1;
}

/**
 * @brief	Set a 64 bit value in memcached by key.
 * @param	key			a managed string containing a key to be passed to memcached.
 * @param	value		the new value to be associated with the supplied key.
 * @param	expiration	the expiration time of the cached data.
 * @return	NULL on failure, or the retrieved unsigned 64 bit cached data on success.
 */
int_t cache_set_u64(stringer_t *key, uint64_t value, time_t expiration) {

	uint32_t pool;
	memcached_return_t ret;

	if ((pool_pull(cache_pool, &pool)) != PL_RESERVED) {
		return 0;
	}
	else if ((ret = memcached_set_d(pool_get_obj(cache_pool, pool), st_char_get(key), st_length_get(key), (void *)&value, sizeof(uint64_t), expiration, 0)) != MEMCACHED_SUCCESS) {
		log_info("Unable to store the %.*s object. {%s}", st_length_int(key), st_char_get(key), memcached_strerror_d(pool_get_obj(cache_pool, pool), ret));
		pool_release(cache_pool, pool);
		return 0;
	}
	pool_release(cache_pool, pool);
	return 1;
}

/**
 * @brief	Add a new key/value pair to the memcached server.
 * @param	key			a managed string with the name of the key to be added to the cache.
 * @param	object		a managed string containing the initial value of the new key.
 * @param	expiration	an expiration time, in seconds, for the newly cached data.
 */
int_t cache_add(stringer_t *key, stringer_t *object, time_t expiration) {

	uint32_t pool;
	memcached_return_t ret;

	if ((pool_pull(cache_pool, &pool)) != PL_RESERVED) {
		return 0;
	}
	else if ((ret = memcached_add_d(pool_get_obj(cache_pool, pool), st_char_get(key), st_length_get(key), st_char_get(object), st_length_get(object), expiration, 0)) != MEMCACHED_SUCCESS) {
		log_info("Unable to store the %.*s object. {%s}", st_length_int(key), st_char_get(key), memcached_strerror_d(pool_get_obj(cache_pool, pool), ret));
		pool_release(cache_pool, pool);
		return 0;
	}
	pool_release(cache_pool, pool);
	return 1;
}

/**
 * @brief	Add a new key/value pair to the memcached server, but suppress any error messages.
 * @param	key			a managed string with the name of the key to be added to the cache.
 * @param	object		a managed string containing the initial value of the new key.
 * @param	expiration	an expiration time, in seconds, for the newly cached data.
 */
int_t cache_silent_add(stringer_t *key, stringer_t *object, time_t expiration) {

	uint32_t pool;
	memcached_return_t ret;

	if ((pool_pull(cache_pool, &pool)) != PL_RESERVED) {
		return 0;
	}
	else if ((ret = memcached_add_d(pool_get_obj(cache_pool, pool), st_char_get(key), st_length_get(key), st_char_get(object), st_length_get(object), expiration, 0)) != MEMCACHED_SUCCESS) {
		pool_release(cache_pool, pool);
		return 0;
	}
	pool_release(cache_pool, pool);
	return 1;
}

/**
 * @brief	Append data to the value of a cached keye on a memcached server.
 * @param	key			a managed string with the name of the target memcached key.
 * @param	object		a managed string containing the value of the data being appended to the original key.
 * @param	expiration	an expiration time, in seconds, for the modified cached data.
 */
int_t cache_append(stringer_t *key, stringer_t *object, time_t expiration) {

	uint32_t pool;
	memcached_return_t val;

	if ((pool_pull(cache_pool, &pool)) != PL_RESERVED) {
		return 0;
	}
	else if ((val = memcached_append_d(pool_get_obj(cache_pool, pool), st_char_get(key), st_length_get(key), st_char_get(object), st_length_get(object), expiration, 0)) != MEMCACHED_SUCCESS) {
		if (val == MEMCACHED_NOTSTORED && (val = memcached_add_d(pool_get_obj(cache_pool, pool), st_char_get(key), st_length_get(key), st_char_get(object), st_length_get(object), expiration, 0)) != MEMCACHED_SUCCESS) {
			log_info("Unable to append to the %.*s object. {%s}", st_length_int(key), st_char_get(key), memcached_strerror_d(pool_get_obj(cache_pool, pool), val));
			pool_release(cache_pool, pool);
			return 0;
		}
		else if (val != MEMCACHED_NOTSTORED) {
			log_info("Unable to append to the %.*s object. {%s}",  st_length_int(key), st_char_get(key), memcached_strerror_d(pool_get_obj(cache_pool, pool), val));
			pool_release(cache_pool, pool);
			return 0;
		}
	}
	pool_release(cache_pool, pool);
	return 1;
}

/**
 * @brief	Delete a key from memcached immediately.
 * @note	memcached_delete()
 * @param	key		the name of the key to be deleted from the memcached server.
 * @return	0 on failure, or MEMCACHED_SUCCESS on success.
 */
int_t cache_delete(stringer_t *key) {

	uint32_t pool;
	memcached_return_t val;

	if ((pool_pull(cache_pool, &pool)) != PL_RESERVED) {
		return 0;
	}
	else if ((val = memcached_delete_d(pool_get_obj(cache_pool, pool), st_char_get(key), st_length_get(key), 0)) != MEMCACHED_SUCCESS) {
		log_info("Unable to delete the %.*s object. {%s}", st_length_int(key), st_char_get(key), memcached_strerror_d(pool_get_obj(cache_pool, pool), val));
		pool_release(cache_pool, pool);
		return 0;
	}
	pool_release(cache_pool, pool);
	return 1;
}

/**
 * @brief	Increment the value stored with a key by memcached by a specified offset.
 * @see		memcached_increment_with_initial()
 * @param	key			a managed string containing the name of the memcached key to be adjusted.
 * @param	offset		the offset by which the specified key's value should be incremented.
 * @param	initial		the initial value to seed the key if it does not already exist.
 * @param	expiration	the time, in seconds, when the cached key will expire.
 * @return	0 on failure, or the new incremented value of the data associated with the key, on success.
 */
uint64_t cache_increment(stringer_t *key, uint64_t offset, uint64_t initial, time_t expiration) {

	uint32_t pool;
	uint64_t output;
	memcached_return_t val;

	if ((pool_pull(cache_pool, &pool)) != PL_RESERVED) {
		return 0;
	}
	// Try incrementing. If we can't, try creating the key.
	else if ((val = memcached_increment_with_initial_d(pool_get_obj(cache_pool, pool), st_char_get(key), st_length_get(key), offset, initial, expiration, &output)) != MEMCACHED_SUCCESS) {
		log_pedantic("Unable to increment the %.*s object. {%s}", st_length_int(key), st_char_get(key), memcached_strerror_d(pool_get_obj(cache_pool, pool), val));
		pool_release(cache_pool, pool);
		return 0;
	}

	pool_release(cache_pool, pool);
	return output;
}
