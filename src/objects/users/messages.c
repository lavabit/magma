
/**
 * @file /magma/objects/users/messages.c
 *
 * @brief	The user context interface for message folders.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/// TODO: The serial number scheme needs to refreshing. There should be a serial number to indicate changes in the folder structure, and then
/// individual serial numbers for each folder to indicate when the contents are changed. If that were the case this function really only needs to
/// update the list of folders.

// Build or update the collection of message folders.
/**
 * @brief	Refresh a user's message folders if stale, or retrieve them from the database if they are not in memory.
 * @see		messages_update()
 * @param	user	a pointer to the meta user object requesting the folders.
 * @param	locked	if META_LOCKED, lock the meta user object for the duration of the request.
 * @return	-1 on failure, or 1 on success.
 */
int_t meta_message_folders_update(meta_user_t *user, META_LOCK_STATUS locked) {

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

	// If there is a message folder index already available, use the serial number to see if it needs updating.
	if (user->message_folders && (checkpoint = serial_get(OBJECT_FOLDERS, user->usernum)) != user->serials.folders) {

		if (!(user->serials.folders = checkpoint)) {
			user->serials.folders = serial_increment(OBJECT_FOLDERS, user->usernum);
		}

		// If the fetch attempt fails, don't free the existing message folder index.
		if ((fetch = messages_update(user->usernum))) {
			inx_free(user->message_folders);
			user->message_folders = fetch;
			output = 1;
		}

	}

	// We need to build the message folder index from scratch.
	else if (!user->message_folders) {

		if (!(user->serials.folders = serial_get(OBJECT_FOLDERS, user->usernum))) {
			user->serials.folders = serial_increment(OBJECT_FOLDERS, user->usernum);
		}

		if ((user->message_folders = messages_update(user->usernum))) {
			output = 1;
		}

	}

	// Do we need to clear the lock.
	if (locked == META_NEED_LOCK) {
		meta_user_unlock(user);
	}

	return output;
}
