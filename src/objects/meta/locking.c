
/**
 * @file /magma/src/objects/new_meta/locking.c
 *
 * @brief Meta object read and write locking functions.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Acquire a read lock for a new_meta user object.
 * @param	user	a pointer to the new_meta user object to be locked.
 * @return	This function returns no value.
 */
void new_meta_user_rlock(new_meta_user_t *user) {

	if (user) {
		// When read/write locking issues have been fixed, this line can be used once again.
		rwlock_lock_read(&(user->lock));
		//log_pedantic("%20.li granted read lock", thread_get_thread_id());
	}

	return;
}

/**
 * @brief	Acquire a write lock for a new_meta user object.
 * @param	user	a pointer to the new_meta user object to be locked.
 * @return	This function returns no value.
 */
void new_meta_user_wlock(new_meta_user_t *user) {

	if (user) {
		// When read/write locking issues have been fixed, this line can be used once again.
		rwlock_lock_write(&(user->lock));
		//log_pedantic("%20.li granted write lock", thread_get_thread_id());
	}

	return;
}

/**
 * @brief	Release the lock for a new_meta user object.
 * @param	user	a pointer to the new_meta user object to be unlocked.
 * @return	This function returns no value.
 */
void new_meta_user_unlock(new_meta_user_t *user) {

	if (user) {
		// When read/write locking issues have been fixed, this line can be used once again.
		rwlock_unlock(&(user->lock));
		//log_pedantic("%20.li unlocking", thread_get_thread_id());
	}

	return;
}
