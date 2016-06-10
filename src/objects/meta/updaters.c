
/**
 * @file /magma/src/objects/meta/updaters.c
 *
 * @brief Update the various elements of the meta object.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 *
 * @param user
 * @param master
 * @param locked
 *
 * @return	-2 if there is a problem unscrambling the private key, -1 for a system error, 0 for success, and 1 if the keys were created.
 */
int_t meta_update_keys(new_meta_user_t *user, stringer_t *master, META_LOCK_STATUS locked) {

	int_t result = 0;
	int64_t transaction = 0;
	stringer_t *holder = NULL;
	scramble_t *scramble = NULL;
	key_pair_t pair = {
		NULL, NULL
	};

	// Do we need a lock.
	if (locked == META_NEED_LOCK) {
		new_meta_user_wlock(user);
	}

	// We only need to fetch and decrypt the user keys if they aren't already stored in the structure.
	if (user->usernum && st_empty(user->keys.private, user->keys.public)) {

		if ((transaction = tran_start()) < 0) {
			log_pedantic("Unable to start storage key SQL transaction. { username = %.*s }", st_length_int(user->username),
				st_char_get(user->username));
			result = -1;
		}

		// Fetch the keys. If none are found, try to generate a new key pair for the user.
		else if (new_meta_data_fetch_keys(user, &pair, transaction) == 1) {

			// Make sure we can retrieve the keys from the database before we return them to the caller.
			if (meta_crypto_keys_create(user->usernum, user->username, master, transaction) < 0) {
				log_pedantic("Unable to create and fetch a newly created user key pair. { username = %.*s }", st_length_int(user->username),
					st_char_get(user->username));
				tran_rollback(transaction);
				result = -1;
			}
			else if (tran_commit(transaction)) {
				log_pedantic("Unable to create and fetch a newly created user key pair. { username = %.*s }", st_length_int(user->username),
					st_char_get(user->username));
				result = -1;
			}


			else if ((transaction = tran_start()) < 0 || new_meta_data_fetch_keys(user, &pair, transaction)) {

				log_pedantic("Unable to create and fetch a newly created user key pair. { username = %.*s }", st_length_int(user->username),
					st_char_get(user->username));

				// Unless the second call to tran_start() fails, we'll have a valid transaction handle at this point.
				if (transaction >= 0) tran_commit(transaction);
				result = -1;
			}
			else {
				tran_commit(transaction);
			}

		}

		// If we reach this point and the key pair is still empty, then a negative value was returned by the first
		// fetch operation above.
		if (!st_populated(pair.private, pair.public)) {
			log_pedantic("Unable to fetch the user key pair. { username = %.*s }", st_length_int(user->username),
				st_char_get(user->username));
			result = -1;
		}

		// Decrypt the buffer retrieved from the database. Return a different error code if there was a problem decrypting the key.
		else if (!(scramble = scramble_import(pair.private)) || !(holder = scramble_decrypt(master, scramble))) {
			log_pedantic("Unable to decrypt the private user key. { username = %.*s }", st_length_int(user->username),
				st_char_get(user->username));
			st_cleanup(pair.private, pair.public);
			result = -2;
		}

		/// BUG: We shouldn't need to duplicate the private key. This is a short term solution because we can't store the decrypted data in
		/// 		a secure buffer yet.
		// Copy the private key into a secure buffer and assign the public key to the user object.
		else if (!(user->keys.public = pair.public) || !(user->keys.private = st_dupe_opts(MANAGED_T | CONTIGUOUS | SECURE, holder))) {

			log_pedantic("Unable to copy the key pair into the user object. { username = %.*s }", st_length_int(user->username),
				st_char_get(user->username));

			st_cleanup(holder, pair.private, pair.public);
			user->keys.public = user->keys.private = NULL;
			result = -1;
		}

		else {
			st_cleanup(holder, pair.private);
		}
	}

	// Do we need to clear the lock.
	if (locked == META_NEED_LOCK) {
		new_meta_user_unlock(user);
	}

	return result;
}

/**
 * @brief	Build a user's meta information from specified data parameters.
 *
 * @note	The user object will be pulled from the cache, if possible, or it falls back to a database lookup using the user number and
 * 			the verification token.
 * .
 * @param	user			a pointer to the meta object that is to be populated.
 * @param	usernum			the user number.
 * @param	verification	the verification token for the specified user number.
 * @param	locked			the meta lock status of the operation (if META_NEED_LOCK is supplied, the meta user object will be
 * 							locked for the duration of the function.
 *
 * @return	-1 on error, 0 on success, 1 for an authentication issue.
 */
int_t new_meta_update_user(new_meta_user_t *user, META_LOCK_STATUS locked) {

	uint64_t serial;
	int_t result = 0;

	// Sanity.
	if (!user) {
		return -1;
	}

	// Do we need a lock.
	if (locked == META_NEED_LOCK) {
		new_meta_user_wlock(user);
	}

	// Check for cached data.
	if (user->usernum && st_populated(user->username, user->verification)) {

		// Check the cache server for an updated serial number. If the serial we find matches, assume the stored data is up to date.
		if ((serial = serial_get(OBJECT_USER, user->usernum)) == new_meta_user_serial_get(user, OBJECT_USER)) {
			result = 1;
		}
		// If the serial numbers don't match, then refresh the stored data and update the object serial number.
		else if (!(result = new_meta_data_fetch_user(user))) {
			new_meta_user_serial_set(user, OBJECT_USER, serial);
		}

	}

	// The user structure is empty, so we need to populate it with information from the database.
	else if (!(result = new_meta_data_fetch_user(user)) && !(user->serials.user = serial_get(OBJECT_USER, user->usernum))) {
		user->serials.user = serial_increment(OBJECT_USER, user->usernum);
	}

	// Do we need to clear the lock.
	if (locked == META_NEED_LOCK) {
		new_meta_user_unlock(user);
	}

	return result;
}

/**
 * @brief Pulls the list of user mailboxes and their display names from the database.
 *
 * @param user				a pointer to the meta object that is to be populated.
 * @param	locked			the meta lock status of the operation (if META_NEED_LOCK is supplied, the meta user object will be
 * 							locked for the duration of the function.
 * @return
 */
int_t meta_update_aliases(new_meta_user_t *user, META_LOCK_STATUS locked) {

	uint64_t serial;
	int_t result = 0;

	// Sanity.
	if (!user) {
		return -1;
	}

	// Do we need a lock.
	if (locked == META_NEED_LOCK) {
		new_meta_user_wlock(user);
	}

	// Check for cached data.
	if (user->usernum && user->aliases) {

		// Check the cache server for an updated serial number. If the serial we find matches, assume the stored data is up to date.
		if ((serial = serial_get(OBJECT_ALIASES, user->usernum)) == new_meta_user_serial_get(user, OBJECT_ALIASES)) {
			result = 1;
		}
		// If the serial numbers don't match, then refresh the stored data and update the object serial number.
		else if (!(result = new_meta_data_fetch_mailbox_aliases(user))) {
			new_meta_user_serial_set(user, OBJECT_ALIASES, serial);
		}

	}

	// The user structure is empty, so we need to populate it with information from the database.
	else if (!(result = new_meta_data_fetch_mailbox_aliases(user)) && !(user->serials.aliases = serial_get(OBJECT_ALIASES, user->usernum))) {
		user->serials.aliases = serial_increment(OBJECT_ALIASES, user->usernum);
	}

	// Do we need to clear the lock.
	if (locked == META_NEED_LOCK) {
		new_meta_user_unlock(user);
	}

	return result;
}

/**
 * @brief	Update a user's contacts if necessary.
 * @note	This function will try to retrieve the folders from the cache, if possible, or fall back to the database.
 * @param	user	a pointer to the meta user object that will have its message folders updated.
 * @param	locked	if META_NEED_LOCK is specified, the meta user object will be locked for operation.
 * @return	-1 on failure or 1 on success.
 */
int_t new_meta_update_contacts(new_meta_user_t *user, META_LOCK_STATUS locked) {

	inx_t *fetch;
	int_t output = 0;
	uint64_t checkpoint;

	if (!user) {
		return -1;
	}

	// Do we need a lock.
	if (locked == META_NEED_LOCK) {
		new_meta_user_wlock(user);
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

		if (!(user->serials.contacts = serial_get(OBJECT_CONTACTS, user->usernum))) {
			user->serials.contacts = serial_increment(OBJECT_CONTACTS, user->usernum);
		}

		if ((user->contacts = contacts_update(user->usernum))) {
			output = 1;
		}
	}

	// Do we need to clear the lock.
	if (locked == META_NEED_LOCK) {
		new_meta_user_unlock(user);
	}

	return output;
}

/**
 * @brief	Update a user's message folders if necessary.
 * @note	This function will try to retrieve the folders from the cache, if possible, or fall back to the database.
 * @param	user	a pointer to the meta user object that will have its message folders updated.
 * @param	locked	if META_NEED_LOCK is specified, the meta user object will be locked for operation.
 * @return	-1 on failure or 1 on success.
 */
int_t new_meta_update_folders(new_meta_user_t *user, META_LOCK_STATUS locked) {

	int_t output = 0;
	uint64_t checkpoint;

	if (!user) {
		return -1;
	}

	// Do we need a lock.
	if (locked == META_NEED_LOCK) {
		new_meta_user_wlock(user);
	}

	if (!user->refs.pop && user->folders && (checkpoint = serial_get(OBJECT_FOLDERS, user->usernum)) != user->serials.folders) {

		if ((user->serials.folders = checkpoint) == 0) {
			user->serials.folders = serial_increment(OBJECT_FOLDERS, user->usernum);
		}

		if ((output = new_meta_data_fetch_folders(user)) && user->messages) {
			meta_messages_update_sequences(user->folders, user->messages);
		}
	}

	// We need to build the folders table.
	else if (!user->folders) {

		if ((user->serials.folders = serial_get(OBJECT_FOLDERS, user->usernum)) == 0) {
			user->serials.folders = serial_increment(OBJECT_FOLDERS, user->usernum);
		}

		if ((output = new_meta_data_fetch_folders(user)) && user->messages) {
			meta_messages_update_sequences(user->folders, user->messages);
		}
	}

	// Do we need to clear the lock.
	if (locked == META_NEED_LOCK) {
		new_meta_user_unlock(user);
	}

	return output;
}

/**
 * @brief	Refresh a user's message folders if stale, or retrieve them from the database if they are not in memory.
 * @see		messages_update()
 * @param	user	a pointer to the meta user object requesting the folders.
 * @param	locked	if META_LOCKED, lock the meta user object for the duration of the request.
 * @return	-1 on failure, or 1 on success.
 */
int_t meta_update_message_folders(new_meta_user_t *user, META_LOCK_STATUS locked) {

	inx_t *fetch;
	int_t output = 0;
	uint64_t checkpoint;

	if (!user) {
		return -1;
	}

	// Do we need a lock.
	if (locked == META_NEED_LOCK) {
		new_meta_user_wlock(user);
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
		new_meta_user_unlock(user);
	}

	return output;
}
