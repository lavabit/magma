
/**
 * @file /magma/core/thread/keys.c
 *
 * @brief	Functions for handling thread local storage keys.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Create a thread-specific data key.
 * @see		pthread_key_create()
 * @param	key			a pointer to pthread key to be initialized for thread-local storage.
 * @param	destructor	if not NULL, an optional function pointer to be called at thread exit to release the data.
 * @return	0 on success, or an error number on failure.
 */
int tkey_init(pthread_key_t *key, void(*destructor)(void*)) {

#ifdef MAGMA_PEDANTIC
	int result = pthread_key_create(key, destructor);
	if (result) log_pedantic("Could not create thread the specific storage key. {pthread_key_create = %i}", result);
	return result;
#else
	return pthread_key_create(key, destructor);
#endif

}

/**
 * @brief	Get the calling thread's value for a pthread key.
 * @see		pthread_getspecific()
 * @param	key		the pthread key to be queried.
 * @return	NULL on failure or a pointer to the key's thread-specific data value on success.
 */
void * tkey_get(pthread_key_t key) {
	return pthread_getspecific(key);
}

/**
 * @brief	Set the calling thread's value for a pthread key.
 * @see		pthread_setspecific()
 * @param	key		the pthread key to have its thread-local value modified.
 * @param	value	a pointer to the new thread-specific value of the specified key.
 * @return	0 on success, or an error number on failure.
 */
int tkey_set(pthread_key_t key, void *value) {

	#ifdef MAGMA_PEDANTIC
	int result = pthread_setspecific(key, value);
	if (result) log_pedantic("Could not update the thread specific storage key. {pthread_setspecific = %i}", result);
	return result;
#else
	return pthread_setspecific(key, value);
#endif

}
