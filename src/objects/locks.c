
/**
 * @file /magma/objects/locks.c
 *
 * @brief	Functions for managing locks synchronized via memcached.
 */

#include "magma.h"

#define MAGMA_LOCK_TIMEOUT 60
#define MAGMA_LOCK_EXPIRATION 600

 /**
  * @brief	Acquire a named lock, with synchronization provided via memcached.
  * @see	cache_silent_add()
  * @note	The lock will be held for a maximum of 10 minutes, and failed locking attempts will be retried
  * 		periodically for a maxmimum of 1 minute before returing failure.
  * @param	key		a managed string containing the name of the lock to be acquired.
  * @return	-1 on general failure, 0 on memcached failure, or 1 on success.
  */
int_t lock_get(stringer_t *key) {

	uint64_t value;
	stringer_t *lock = MANAGEDBUF(128);
	int_t success, iterations = (MAGMA_LOCK_TIMEOUT * 10);
	const struct timespec delay = { .tv_sec = 0, .tv_nsec = 1000000000 };

	// Build the key.
	if (st_empty(key) || st_sprint(lock, "%.*s.lock", st_length_int(key), st_char_get(key)) <= 0) {
		log_pedantic("Unable generate the accessor for the cluster lock.");
		return -1;
	}

	// Build the lock value.
	value = time(NULL);

	do {

		// Keep the lock for ten minutes.
		if ((success = cache_silent_add(lock, PLACER(&value, sizeof(uint64_t)), MAGMA_LOCK_EXPIRATION)) != 1) {
			nanosleep(&delay, NULL);
		}

	} while (success != 1 && iterations--);

#ifdef MAGMA_PEDANTIC
	if (success != 1) log_pedantic("Unable to obtain a cluster lock for %.*s.", st_length_int(lock), st_char_get(lock));
#endif

	return success;
}

/**
  * @brief	Release a named lock, with synchronization provided via memcached.
  * @see	cache_delete()
  * @note	The lock will be held for 10 seconds, and locking attempts will occur periodically for 60 seconds prior to failure.
  * @param	key		a managed string containing the name of the lock to be released.
  * @return	-1 on general failure, 0 on memcached failure, or 1 on success.
  */
void lock_release(stringer_t *key) {

	stringer_t *lock = MANAGEDBUF(128);

	// Build the key.
	if (st_empty(key) || st_sprint(lock, "%.*s.lock", st_length_int(key), st_char_get(key)) <= 0) {
		log_pedantic("Unable generate the accessor for the cluster lock.");
		return;
	}

	/// LOW: At some point we should add logic to check whether this cluster node even owns the lock before
	/// 	blindly deleting the lock.
	cache_delete(lock);
	return;
}

 /**
  * @brief	Acquire a lock in the magma.user keyspace.
  * @param	usernum		the numerical id of the user for whom the lock will be acquired.
  * @return	-1 on general failure, 0 on memcached failure, or 1 on success.
  */
int_t user_lock(uint64_t usernum) {

	stringer_t *key = MANAGEDBUF(128);

	if (st_sprint(key, "magma.user.%lu", usernum) <= 0) {
		return -1;
	}

	return lock_get(key);
}

/**
 * @brief	Unlock a lock in the magma.user keyspace.
 * @param	usernum		the numerical id of the user for whom the lock will be unlocked.
 * @return	This function returns no value.
 */
void user_unlock(uint64_t usernum) {

	stringer_t *key = MANAGEDBUF(128);

	if (st_sprint(key, "magma.user.%lu", usernum) <= 0) {
		return;
	}

	lock_release(key);

	return;
}
