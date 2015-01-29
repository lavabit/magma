
/**
 * @file /magma/objects/users/contacts.c
 *
 * @brief	The user context interface for contacts.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Update a user's contacts if necessary.
 * @note	This function will try to retrieve the folders from the cache, if possible, or fall back to the database.
 * @param	user	a pointer to the meta user object that will have its message folders updated.
 * @param	locked	if META_NEED_LOCK is specified, the meta user object will be locked for operation.
 * @return	-1 on failure or 1 on success.
 */
int_t meta_contacts_update(meta_user_t *user, META_LOCK_STATUS locked) {

	inx_t *fetch;
	int_t output = 0;
	uint64_t checkpoint;

	if (!user) {
		return -1;
	}

	// Do we need a lock.
	if (locked == META_NEED_LOCK) {
		meta_user_wlock(user);
	}

	// If there is a contacts already available, use the serial number to see if it needs updating.
	if (user->contacts && (checkpoint = serial_get(OBJECT_CONTACTS, user->usernum)) != user->serials.contacts) {

		if (!(user->serials.contacts = checkpoint)) {
			user->serials.contacts = serial_increment(OBJECT_CONTACTS, user->usernum);
		}

		// If the fetch attempt fails, don't free the existing contacts index.
		if ((fetch = contacts_update(user->usernum))) {
			inx_free(user->contacts);
			user->contacts = fetch;
			output = 1;
		}
	}

	// We need to build the folders table.
	else if (!user->contacts) {

		if (!(user->serials.contacts = serial_get(M_FOLDER_CONTACTS, user->usernum))) {
			user->serials.contacts = serial_increment(M_FOLDER_CONTACTS, user->usernum);
		}

		if ((user->contacts = contacts_update(user->usernum))) {
			output = 1;
		}
	}

	// Do we need to clear the lock.
	if (locked == META_NEED_LOCK) {
		meta_user_unlock(user);
	}

	return output;
}
