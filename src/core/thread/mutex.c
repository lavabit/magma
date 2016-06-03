
/**
 * @file /magma/core/thread/mutex.c
 *
 * @brief	Functions for thread coordination via a mutex.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Initialize a pthread mutex with the given attributes.
 * @see		pthread_mutex_init()
 * @param	lock	a pointer to a mutex that will be initialized.
 * @param	attr	a pointer to a mutex attributes holder, or NULL to use system default values.
 * @return	0 on success, or an error number on failure.
 */
int mutex_init(pthread_mutex_t *lock, pthread_mutexattr_t *attr) {

#ifdef MAGMA_PEDANTIC
	int result = pthread_mutex_init(lock, attr);
	if (result) {
		log_pedantic("Could not initialize the mutex. {pthread_mutex_init = %i / error = %s}", result, strerror_r(errno, MEMORYBUF(1024), 1024));
	}
	return result;
#else
	return pthread_mutex_init(lock, attr);
#endif

}

/**
 * @brief	Acquire a pthread mutex, blocking if necessary.
 * @see		pthread_mutex_lock()
 * @param	lock	a pointer to the mutex to be locked.
 * @return	0 on success, or an error number on failure.
 */
int mutex_get_lock(pthread_mutex_t *lock) {

#ifdef MAGMA_PEDANTIC
	int result = pthread_mutex_lock(lock);
	if (result) {
		log_options(M_LOG_PEDANTIC | M_LOG_STACK_TRACE, "Could not lock the mutex. {pthread_mutex_lock = %i / error = %s}", result, strerror_r(errno, MEMORYBUF(1024), 1024));
	}
	return result;
#else
	return pthread_mutex_lock(lock);
#endif

}

/**
 * @brief	Unlock a pthread mutex.
 * @see		pthread_mutex_unlock()
 * @param	lock	a pointer to the mutex to be unlocked.
 * @return	0 on success, or an error number on failure.
 */
int mutex_unlock(pthread_mutex_t *lock) {

#ifdef MAGMA_PEDANTIC
	int result = pthread_mutex_unlock(lock);
	if (result) {
		log_options(M_LOG_PEDANTIC | M_LOG_STACK_TRACE, "Could not unlock the mutex. {pthread_mutex_unlock = %i / error = %s}", result, strerror_r(errno, MEMORYBUF(1024), 1024));
	}
	return result;
#else
	return pthread_mutex_unlock(lock);
#endif

}

/**
 * @brief	Free a pthread mutex.
 * @see		pthread_mutex_destroy()
 * @param	lock	a pointer to the mutex to be freed.
 * @return	0 on success, or an error number on failure.
 */
int mutex_destroy(pthread_mutex_t *lock) {

#ifdef MAGMA_PEDANTIC
	int result = pthread_mutex_destroy(lock);
	if (result) {
		log_pedantic("Could not destroy the mutex. {pthread_mutex_destroy = %i / error = %s}", result, strerror_r(errno, MEMORYBUF(1024), 1024));
	}
	return result;
#else
	return pthread_mutex_destroy(lock);
#endif

}
