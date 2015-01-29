
/**
 * @file /magma/objects/neue/neue.c
 *
 * @brief	Neue entry points.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	This function is not called from anywhere and may be removed.
 */
void neue_rlock(neue_t *neue) {

	if (neue) {
		rwlock_lock_read(&(neue->lock));
	}

	return;
}

/**
 * @brief	This function is not called from anywhere and may be removed.
 */
void neue_wlock(neue_t *neue) {

	if (neue) {
		rwlock_lock_write(&(neue->lock));
		//log_pedantic("%20.li granted write lock", thread_get_thread_id());
	}

	return;
}

/**
 * @brief	This function is not called from anywhere and may be removed.
 */
void neue_unlock(neue_t *neue) {

	if (neue) {
		rwlock_unlock(&(neue->lock));
		//log_pedantic("%20.li unlocking", thread_get_thread_id());
	}

	return;
}
