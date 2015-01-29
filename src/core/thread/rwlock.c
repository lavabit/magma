
/**
 * @file /magma/core/thread/rwlock.c
 *
 * @brief	Functions for thread coordination via a read/write lock.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Initialize a pthread read/write lock.
 * @see		pthread_rwlock_init()
 * @param	lock	a pointer to the read/write lock to be initialized.
 * @param	attr	an optional pointer to a set of lock attributes, or the default attributes if NULL is specified.
 * @return	0 on success or an error number on failure.
 */
int rwlock_init(pthread_rwlock_t *lock, pthread_rwlockattr_t * attr) {

#ifdef MAGMA_PEDANTIC
	int result = pthread_rwlock_init(lock, attr);
	if (result) log_pedantic("Could not initialize the read/write lock. {pthread_rwlock_init = %i}", result);
	return result;
#else
	return pthread_rwlock_init(lock, attr);
#endif

}

/**
 * @brief	Attempt to acquire a pthread read/write lock for writing, blocking if necessary.
 * @see		pthread_rwlock_wrlock()
 * @param	lock	a pointer to the read/write lock to be acquired.
 * @return	0 on success or an error number on failure.
 */
int rwlock_lock_write(pthread_rwlock_t *lock) {

#ifdef MAGMA_PEDANTIC
	int result = pthread_rwlock_wrlock(lock);
	if (result) log_pedantic("Could not obtain a write lock. {pthread_rwlock_wrlock = %i}", result);
	return result;
#else
	return pthread_rwlock_wrlock(lock);
#endif

}

/**
 * @brief	Attempt to acquire a pthread read/write lock for reading, blocking if necessary.
 * @see		pthread_rwlock_rdlock()
 * @param	lock	the read-write lock to be tried.
 * @return	0 on success or an error number on failure.
 */
int rwlock_lock_read(pthread_rwlock_t *lock) {

#ifdef MAGMA_PEDANTIC
	int result = pthread_rwlock_rdlock(lock);
	if (result) log_pedantic("Could not obtain a read lock. {pthread_rwlock_rdlock = %i}", result);
	return result;
#else
	return pthread_rwlock_rdlock(lock);
#endif

}

/**
 * @brief	Unlock a pthread read/write lock.
 * @see		pthread_rwlock_unlock()
 * @param	lock	a pointer to the read/write lock to be unlocked.
 * @return	0 on success or an error number on failure.
 */
int rwlock_unlock(pthread_rwlock_t *lock) {

#ifdef MAGMA_PEDANTIC
	int result = pthread_rwlock_unlock(lock);
	if (result) log_pedantic("Could not release the read/write lock. {pthread_rwlock_unlock = %i}", result);
	return result;
#else
	return pthread_rwlock_unlock(lock);
#endif

}

/**
 * @brief	Free a pthread read/write lock and free its resources.
 * @see		pthread_rwlock_destroy()
 * @param	lock	a pointer to the read/write lock to be destroyed.
 * @return	0 on success or an error number on failure.
 */
int rwlock_destroy(pthread_rwlock_t *lock) {
#ifdef MAGMA_PEDANTIC
	int result = pthread_rwlock_destroy(lock);
	if (result) log_pedantic("Could not destroy the read/write lock. {pthread_rwlock_destroy = %i}", result);
	return result;
#else
	return pthread_rwlock_destroy(lock);
#endif
}

/**
 * @brief	Initialize a pthread read/write lock attributes object with default values.
 * @see		pthread_rwlockattr_init()
 * @param	attr	a pointer to the read/write lock attributes object to be initialized.
 * @return	0 on success or an error number on failure.
 */
int rwlock_attr_init(pthread_rwlockattr_t *attr) {
#ifdef MAGMA_PEDANTIC
	int result = pthread_rwlockattr_init(attr);
	if (result) log_pedantic("Could not initialize the read/write lock attribute object. {pthread_rwlockattr_init = %i}", result);
	return result;
#else
	return pthread_rwlockattr_init(attr);
#endif
}

/**
 * @brief	Free a pthread read/write lock attributes object.
 * @see		pthread_rwlockattr_destroy()
 * @param	attr	a pointer to the read/write lock attributes object to be freed.
 * @return	0 on success or an error number on failure.
 */
int rwlock_attr_destroy(pthread_rwlockattr_t *attr) {
#ifdef MAGMA_PEDANTIC
	int result = pthread_rwlockattr_destroy(attr);
	if (result) log_pedantic("Could not destroy the read/write lock attribute object. {pthread_rwlockattr_destroy = %i}", result);
	return result;
#else
	return pthread_rwlockattr_destroy(attr);
#endif
}

/**
 * @brief	Set the kind of a pthread read/write lock attributes object.
 * @see		pthread_rwlockattr_setkind_np()
 * @param	attr	a pointer to the read/write lock attributes object to be adjusted.
 * @param	pref	the kind of the read/write lock: PTHREAD_RWLOCK_PREFER_READER_NP, PTHREAD_RWLOCK_PREFER_WRITER_NP, or PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP.
 * @return	0 on success or an error number on failure.
 */
int rwlock_attr_setkind(pthread_rwlockattr_t *attr, int pref) {
#ifdef MAGMA_PEDANTIC
	int result = pthread_rwlockattr_setkind_np(attr, pref);
	if (result) log_pedantic("Could not set the read/write lock attribute preference. {pthread_rwlockattr_setkind_np = %i}", result);
	return result;
#else
	return pthread_rwlockattr_setkind_np(attr, pref);
#endif
}

/**
 * @brief	Get the kind of a pthread read/write lock attributes object.
 * @see		pthread_rwlockattr_getkind_np()
 * @param	attr	a pointer to the read/write lock attributes object to be queried.
 * @param	pref	a pointer to an integer that will receive the kind of the read/write lock.
 * @return	0 on success or an error number on failure.
 */
int rwlock_attr_getkind(pthread_rwlockattr_t *attr, int *pref) {
#ifdef MAGMA_PEDANTIC
	int result = pthread_rwlockattr_getkind_np(attr, pref);
	if (result) log_pedantic("Could not get the read/write lock attribute preference. {pthread_rwlockattr_getkind_np = %i}", result);
	return result;
#else
	return pthread_rwlockattr_getkind_np(attr, pref);
#endif
}
