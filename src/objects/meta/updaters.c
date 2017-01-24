
/**
 * @file /magma/objects/meta/updaters.c
 *
 * @brief Update the various elements of the meta object.
 */

#include "magma.h"
//#include "dime/signet/keys.h"
//#include "dime/common/misc.h"
//#include "dime/common/dcrypto.h"

/**
 * @brief	Fetches the user realm keys and extracts the different components.
 *
 * @param	user			a pointer to the meta object that is to be populated.
 * @param	master			the user master key value.
 * @param	locked			the meta lock status of the operation (if META_NEED_LOCK is supplied, the meta user object will be
 * 							locked for the duration of the function.
 *
 * @return	-2 if there is a problem extracting the key values, -1 for a system error, 0 for success, and 1 if the keys were created.
 */
int_t meta_update_realms(meta_user_t *user, stringer_t *master, META_LOCK_STATUS locked) {

	int_t result = 0;
	int64_t transaction = -1;
	stringer_t *shard = MANAGEDBUF(64), *holder = NULL, *realm_key = NULL;

	// Do we need a lock.
	if (locked == META_NEED_LOCK) {
		meta_user_wlock(user);
	}

	if (st_empty(user->realm.mail)) {

		if (!user->usernum || st_empty(master) || st_length_get(master) != STACIE_KEY_LENGTH) {
			log_pedantic("Invalid parameters passed to the realm updater. { username = %.*s }", st_length_int(user->username),
				st_char_get(user->username));
			result = -1;
		}

		else if ((transaction = tran_start()) < 0) {
			log_pedantic("Unable to start storage key SQL transaction. { username = %.*s }", st_length_int(user->username),
				st_char_get(user->username));
			result = -1;
		}

		// Fetch the mail realm shard. If one isn't found, try generating a new shard value for the user.
		else if (meta_data_fetch_shard(user->usernum, 0, PLACER("mail", 4), shard, transaction) == 1) {

			if (!(holder = stacie_shard_create(shard)) || meta_data_insert_shard(user->usernum, 0, PLACER("mail", 4), holder, transaction) < 0) {
				log_pedantic("Unable to create a user shard for the mail realm. { username = %.*s }", st_length_int(user->username),
					st_char_get(user->username));
				tran_rollback(transaction);
				transaction = -1;
				result = -1;
			}
			else if (tran_commit(transaction)) {
				log_pedantic("Unable to create a user shard for the mail realm. { username = %.*s }", st_length_int(user->username),
					st_char_get(user->username));
				transaction = -1;
				result = -1;
			}
			else if ((transaction = tran_start()) < 0 || meta_data_fetch_shard(user->usernum, 0, PLACER("mail", 4), shard, transaction)) {
				log_pedantic("Unable to create and fetch a user shard for the mail realm. { username = %.*s }", st_length_int(user->username),
					st_char_get(user->username));
				result = -1;
			}
		}

		// Release the transaction handle.
		if (transaction >= 0) {
			tran_commit(transaction);
		}

		// If we reach this point and the result is still zero, and the shard value is populated, parse the realm key.
		if (!result && st_length_get(shard) == STACIE_SHARD_LENGTH) {

			// Derive the realm key and store the relevant pieces.
			if (!(user->realm.mail = stacie_realm_key_derive(master, PLACER("mail",  4), shard))) {
				log_pedantic("Unable to parse the realm key. { username = %.*s }", st_length_int(user->username),
					st_char_get(user->username));
				result = -1;
			}

			st_cleanup(realm_key);
		}

	}

	// Do we need to clear the lock.
	if (locked == META_NEED_LOCK) {
		meta_user_unlock(user);
	}

	return result;
}

/**
 * @brief	Fetches the user signet and private key. Relies on the mail realm to decrypt the values.
 *
 * @param	user			a pointer to the meta object that is to be populated.
 * @param	locked			the meta lock status of the operation (if META_NEED_LOCK is supplied, the meta user object will be
 * 							locked for the duration of the function.
 *
 * @return	-2 if there is a problem unscrambling the private key, -1 for a system error, 0 for success, and 1 if the keys were created.
 */
int_t meta_update_keys(meta_user_t *user, stringer_t *master, META_LOCK_STATUS locked) {

	int_t result = 0;
	int64_t transaction = 0;
	key_pair_t pair = {
		NULL, NULL
	};

	// Do we need a lock.
	if (locked == META_NEED_LOCK) {
		meta_user_wlock(user);
	}

	// We only need to fetch and decrypt the user keys if they aren't already stored in the structure.
	if (user->usernum && st_empty(user->prime.key, user->prime.signet)) {

		if ((transaction = tran_start()) < 0) {
			log_pedantic("Unable to start shard SQL transaction. { username = %.*s }", st_length_int(user->username),
				st_char_get(user->username));
			result = -1;
		}

		// Fetch the mail shard. If we can't find a mail shard, try to generate a new shard value.
		else if (meta_data_fetch_keys(user, &pair, transaction) == 1) {

			// Make sure we can retrieve the keys from the database before we return them to the caller.
			if (meta_crypto_keys_create(user->usernum, user->username, master, transaction) < 0) {
				log_pedantic("Unable to create a signet and key for the user. { username = %.*s }", st_length_int(user->username),
					st_char_get(user->username));
				tran_rollback(transaction);
				transaction = -1;
				result = -1;
			}
			else if (tran_commit(transaction)) {
				log_pedantic("Unable to commit the transaction. Insertion of the user signet and key failed. { username = %.*s }", st_length_int(user->username),
					st_char_get(user->username));
				transaction = -1;
				result = -1;
			}
			else if ((transaction = tran_start()) < 0 || meta_data_fetch_keys(user, &pair, transaction)) {
				log_pedantic("Unable to fetch the newly created user signet and key. { username = %.*s }", st_length_int(user->username),
					st_char_get(user->username));
				result = -1;
			}

		}

		// Release the transaction handle.
		if (transaction >= 0) {
			tran_commit(transaction);
		}

		// If we reach this point and the key pair is still empty, then a negative value was returned by the first
		// fetch operation above.
		if (!st_populated(pair.private, pair.public)) {
			log_pedantic("Unable to fetch the user key pair. { username = %.*s }", st_length_int(user->username),
				st_char_get(user->username));
			result = -1;
		}

		// Decrypt the buffer retrieved from the database. Return a different error code if there was a problem decrypting the key.
		else if (!(user->prime.key = prime_key_decrypt(user->realm.mail, pair.private, BINARY, SECURITY))) {
			log_pedantic("Unable to decrypt the private user key. { username = %.*s }", st_length_int(user->username),
				st_char_get(user->username));
			st_cleanup(pair.private, pair.public);
			result = -2;
		}

		// Copy the private key into a secure buffer and assign the public key to the user object.
		else if (!(user->prime.signet = prime_set(pair.public, BINARY, NONE))) {

			log_pedantic("Unable to copy the key pair into the user object. { username = %.*s }", st_length_int(user->username),
				st_char_get(user->username));

			st_cleanup(pair.private, pair.public);
			user->prime.signet = user->prime.key = NULL;
			result = -1;
		}

		st_cleanup(pair.private, pair.public);
	}

	// Do we need to clear the lock.
	if (locked == META_NEED_LOCK) {
		meta_user_unlock(user);
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
int_t meta_update_user(meta_user_t *user, META_LOCK_STATUS locked) {

	uint64_t serial;
	int_t result = 0;

	// Sanity.
	if (!user) {
		return -1;
	}

	// Do we need a lock.
	if (locked == META_NEED_LOCK) {
		meta_user_wlock(user);
	}

	// Check for cached data.
	if (user->usernum && st_populated(user->username, user->verification)) {

		// Check the cache server for an updated serial number. If the serial we find matches, assume the stored data is up to date.
		if ((serial = serial_get(OBJECT_USER, user->usernum)) == meta_user_serial_get(user, OBJECT_USER)) {
			result = 1;
		}
		// If the serial numbers don't match, then refresh the stored data and update the object serial number.
		else if (!(result = meta_data_fetch_user(user))) {
			meta_user_serial_set(user, OBJECT_USER, serial);
		}

	}

	// The user structure is empty, so we need to populate it with information from the database.
	else if (!(result = meta_data_fetch_user(user)) && !(user->serials.user = serial_get(OBJECT_USER, user->usernum))) {
		user->serials.user = serial_increment(OBJECT_USER, user->usernum);
	}

	// Do we need to clear the lock.
	if (locked == META_NEED_LOCK) {
		meta_user_unlock(user);
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
int_t meta_update_aliases(meta_user_t *user, META_LOCK_STATUS locked) {

	uint64_t serial;
	int_t result = 0;

	// Sanity.
	if (!user) {
		return -1;
	}

	// Do we need a lock.
	if (locked == META_NEED_LOCK) {
		meta_user_wlock(user);
	}

	// Check for cached data.
	if (user->usernum && user->aliases) {

		// Check the cache server for an updated serial number. If the serial we find matches, assume the stored data is up to date.
		if ((serial = serial_get(OBJECT_ALIASES, user->usernum)) == meta_user_serial_get(user, OBJECT_ALIASES)) {
			result = 1;
		}
		// If the serial numbers don't match, then refresh the stored data and update the object serial number.
		else if (!(result = meta_data_fetch_mailbox_aliases(user))) {
			meta_user_serial_set(user, OBJECT_ALIASES, serial);
		}

	}

	// The user structure is empty, so we need to populate it with information from the database.
	else if (!(result = meta_data_fetch_mailbox_aliases(user)) && !(user->serials.aliases = serial_get(OBJECT_ALIASES, user->usernum))) {
		user->serials.aliases = serial_increment(OBJECT_ALIASES, user->usernum);
	}

	// Do we need to clear the lock.
	if (locked == META_NEED_LOCK) {
		meta_user_unlock(user);
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
int_t meta_update_contacts(meta_user_t *user, META_LOCK_STATUS locked) {

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

		if (!(user->serials.contacts = serial_get(OBJECT_CONTACTS, user->usernum))) {
			user->serials.contacts = serial_increment(OBJECT_CONTACTS, user->usernum);
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

/**
 * @brief	Update a user's message folders if necessary.
 * @note	This function will try to retrieve the folders from the cache, if possible, or fall back to the database.
 * @param	user	a pointer to the meta user object that will have its message folders updated.
 * @param	locked	if META_NEED_LOCK is specified, the meta user object will be locked for operation.
 * @return	-1 on failure or 1 on success.
 */
int_t meta_update_folders(meta_user_t *user, META_LOCK_STATUS locked) {

	int_t output = 0;
	uint64_t checkpoint;

	if (!user) {
		return -1;
	}

	// Do we need a lock.
	if (locked == META_NEED_LOCK) {
		meta_user_wlock(user);
	}

	if (!user->refs.pop && user->folders && (checkpoint = serial_get(OBJECT_FOLDERS, user->usernum)) != user->serials.folders) {

		if ((user->serials.folders = checkpoint) == 0) {
			user->serials.folders = serial_increment(OBJECT_FOLDERS, user->usernum);
		}

		if ((output = meta_data_fetch_folders(user)) && user->messages) {
			meta_messages_update_sequences(user->folders, user->messages);
		}
	}

	// We need to build the folders table.
	else if (!user->folders) {

		if ((user->serials.folders = serial_get(OBJECT_FOLDERS, user->usernum)) == 0) {
			user->serials.folders = serial_increment(OBJECT_FOLDERS, user->usernum);
		}

		if ((output = meta_data_fetch_folders(user)) && user->messages) {
			meta_messages_update_sequences(user->folders, user->messages);
		}
	}

	// Do we need to clear the lock.
	if (locked == META_NEED_LOCK) {
		meta_user_unlock(user);
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
int_t meta_update_message_folders(meta_user_t *user, META_LOCK_STATUS locked) {

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
