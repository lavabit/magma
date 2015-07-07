
/**
 * @file /magma/objects/users/users.c
 *
 * @brief	Functions for handling the user context.
 *
 * $Author:$
 * $Author$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Increment a meta user's reference counter for a specified protocol and update the activity timestamp.
 * @note	META_PROT_GENERIC can be specified for protocol non-specific accounting purposes.
 * @param	user		a pointer to the meta user object to be adjusted.
 * @param	protocol	the protocol identifier for the session.
 * @return	This function returns no value.
 */
void meta_user_ref_add(meta_user_t *user, META_PROT protocol) {

	if (user) {

		// Acquire the reference counter lock.
		mutex_get_lock(&(user->refs.lock));

		// Increment the right counter.
		if ((protocol & META_PROT_WEB) == META_PROT_WEB) user->refs.web++;
		else if ((protocol & META_PROT_IMAP) == META_PROT_IMAP) user->refs.imap++;
		else if ((protocol & META_PROT_POP) == META_PROT_POP) user->refs.pop++;
		else if ((protocol & META_PROT_SMTP) == META_PROT_SMTP) user->refs.smtp++;
		else if ((protocol & META_PROT_GENERIC) == META_PROT_GENERIC) user->refs.generic++;

		// Update the activity time stamp.
		 user->refs.stamp = time(NULL);

		// Release the reference counter lock.
		mutex_unlock(&(user->refs.lock));

	}

	return;
}

/**
 * @brief	Decrement a user's reference counter for a specified protocol and update the activity timestamp.
 * @note	META_PROT_GENERIC can be specified for protocol non-specific accounting purposes.
 * @param	user		a pointer to the meta user object to be adjusted.
 * @param	protocol	the protocol identifier for the session (META_PROT_WEB, META_PROT_IMAP, META_PROT_POP, META_PROT_SMTP, META_PROT_GENERIC).
 * @return	This function returns no value.
 */
void meta_user_ref_dec(meta_user_t *user, META_PROT protocol) {

	if (user) {

		// Acquire the reference counter lock.
		mutex_get_lock(&(user->refs.lock));

		// Decrement the right counter.
		if ((protocol & META_PROT_WEB) == META_PROT_WEB) user->refs.web--;
		else if ((protocol & META_PROT_IMAP) == META_PROT_IMAP) user->refs.imap--;
		else if ((protocol & META_PROT_POP) == META_PROT_POP) user->refs.pop--;
		else if ((protocol & META_PROT_SMTP) == META_PROT_SMTP) user->refs.smtp--;
		else if ((protocol & META_PROT_GENERIC) == META_PROT_GENERIC) user->refs.generic--;

		// Update the activity time stamp.
		user->refs.stamp = time(NULL);

		// Release the reference counter lock.
		mutex_unlock(&(user->refs.lock));

	}

	return;
}

/**
 * @brief	Get the total session reference count for a user for all protocols.
 * @param	user		a pointer to the meta user object to be examined.
 * @return	the total number of references held by the user.
 */
uint64_t meta_user_ref_total(meta_user_t *user) {

	uint64_t result = 0;

	if (user) {

		// Acquire the reference counter lock.
		mutex_get_lock(&(user->refs.lock));

		// Sum the total.
		result = user->refs.web + user->refs.imap + user->refs.pop + user->refs.smtp + user->refs.generic;

		// Release the reference counter lock.
		mutex_unlock(&(user->refs.lock));

	}

	return result;
}

/**
 * @brief	Get the activity timestamp for a meta user object.
 * @param	user	a pointer to the meta user object to be examined.
 * @return	a timestamp containing the last time the meta user object's reference count changed.
 */
time_t meta_user_ref_stamp(meta_user_t *user) {

	time_t result = 0;

	if (user) {

		// Acquire the reference counter lock.
		mutex_get_lock(&(user->refs.lock));

		// Sum the total.
		result = user->refs.stamp;

		// Release the reference counter lock.
		mutex_unlock(&(user->refs.lock));

	}

	return result;
}

/**
 * @brief	Acquire a read lock for a meta user object.
 * @param	user	a pointer to the meta user object to be locked.
 * @return	This function returns no value.
 */
void meta_user_rlock(meta_user_t *user) {

	if (user) {
		// When read/write locking issues have been fixed, this line can be used once again.
		// rwlock_lock_read(&(user->lock));
		mutex_get_lock(&(user->lock));
		//log_pedantic("%20.li granted read lock", thread_get_thread_id());
	}

	return;
}

/**
 * @brief	Acquire a write lock for a meta user object.
 * @param	user	a pointer to the meta user object to be locked.
 * @return	This function returns no value.
 */
void meta_user_wlock(meta_user_t *user) {

	if (user) {
		// When read/write locking issues have been fixed, this line can be used once again.
		// rwlock_lock_write(&(user->lock));
		mutex_get_lock(&(user->lock));
		//log_pedantic("%20.li granted write lock", thread_get_thread_id());
	}

	return;
}

/**
 * @brief	Release the lock for a meta user object.
 * @param	user	a pointer to the meta user object to be unlocked.
 * @return	This function returns no value.
 */
void meta_user_unlock(meta_user_t *user) {

	if (user) {
		// When read/write locking issues have been fixed, this line can be used once again.
		// rwlock_unlock(&(user->lock));
		mutex_unlock(&(user->lock));
		//log_pedantic("%20.li unlocking", thread_get_thread_id());
	}

	return;
}

/**
 * @brief	Destroy a meta user object and free all of its underlying data.
 * @param	user	a pointer to the meta user object to be destroyed.
 * @return	This function returns no value.
 */
void meta_user_destroy(meta_user_t *user) {

	if (user) {

		inx_cleanup(user->ads);
		inx_cleanup(user->aliases);
		inx_cleanup(user->folders);
		inx_cleanup(user->messages);
		inx_cleanup(user->message_folders);
		inx_cleanup(user->contacts);

		st_cleanup(user->username);
		st_cleanup(user->passhash);

		st_cleanup(user->storage_privkey);
		st_cleanup(user->storage_pubkey);

		// When read/write locking issues have been fixed, this line can be used once again.
		// rwlock_destroy(&(user->lock));
		mutex_destroy(&(user->lock));
		mutex_destroy(&(user->refs.lock));

		mm_free(user);
	}

	return;
}

/**
 * @brief	Lock a user's object in the cache and decrement their reference counter.
 * @see		meta_user_ref_dec()
 * @param	username	a managed string containing the name of the user to be adjusted.
 * @param	flags		specifies the protocol bound to the reference counter to be decremented (META_PROT_WEB, META_PROT_IMAP, etc.)
 * @return	This function returns no value.
 */
void meta_remove(stringer_t *username, META_PROT flags) {

	meta_user_t *user;
	multi_t key = { .type = M_TYPE_STRINGER, .val.st = username };

	if (st_empty(username)) {
		return;
	}

	// Cache lock.
	inx_lock_read(objects.users);

	if ((user = inx_find(objects.users, key))) {
		meta_user_ref_dec(user, flags);
	}

	// Release the cache.
	inx_unlock(objects.users);

	return;
}

/**
 * @brief	Locate and remove a user's object from the cache if the reference counter is zero.
 * @param	username	a managed string containing the name of the user whose data should be removed from the object cache.
 * @return returns 1 if the user object was successfully pruned, 0 if the user isn't found, a non-zero reference count causes a return of -1,
 * 						and a value of -2 is returned if an error occurs while trying to delete the user from the object cache index.
 */
int_t meta_user_prune(stringer_t *username) {

	int_t state = 0;
	bool_t deleted = false;
	meta_user_t *user = NULL;
	uint64_t count = 0, expired = 0;
	multi_t key = { .type = M_TYPE_STRINGER, .val.st = username };

	if (st_empty(username)) {
		return 0;
	}

	// Cache lock.
	inx_lock_write(objects.users);

	if ((user = inx_find(objects.users, key)) && !meta_user_ref_total(user)) {
		deleted = inx_delete(objects.users, key);
		count = inx_count(objects.users);
		state = 1;
	}
	else if (user) {
		state = -1;
	}

	// Release the cache.
	inx_unlock(objects.users);

	if (state == 1 && deleted == false) {
		log_error("An error occurred while trying to delete a user from the object cache.");
		state = -1;
	}

	stats_set_by_name("objects.users.total", count);
	stats_adjust_by_name("objects.users.expired", expired);

	return state;
}

/**
 * @brief	Allocate and initialize a meta user object.
 * @return	NULL on failure, or a pointer to the newly allocated meta user object on success.
 */
meta_user_t * meta_user_create(void) {

	meta_user_t *user;
	// pthread_rwlockattr_t attr;

	// Configure the rwlock attributes to prefer write lock requests.
	//	if (rwlock_attr_init(&attr)) {
	//		log_pedantic("Unable to initialize the read/write lock attributes.");
	//		return NULL;
	//	}
	//	else if (rwlock_attr_setkind(&attr, PTHREAD_RWLOCK_PREFER_WRITER_NP)) {
	//		log_pedantic("Unable to set the read/write lock attributes.");
	//		rwlock_attr_destroy(&attr);
	//		return NULL;
	//	}

	if (!(user = mm_alloc(sizeof(meta_user_t)))) {
		log_pedantic("Unable to allocate %zu bytes for a user meta information structure.", sizeof(meta_user_t));
	//	rwlock_attr_destroy(&attr);
		return NULL;
	}
	// else if (rwlock_init(&(user->lock), &attr) != 0) {
	//	log_pedantic("Unable to initialize the read/write lock.");
	//	rwlock_attr_destroy(&attr);
	else if (mutex_init(&(user->lock), NULL) != 0) {
		log_pedantic("Unable to initialize the user lock.");
		mm_free(user);
		return NULL;
	}
	else if (mutex_init(&(user->refs.lock), NULL) != 0) {
		log_pedantic("Unable to initialize the user reference lock.");
	//	rwlock_destroy(&(user->lock));
	//	rwlock_attr_destroy(&attr);
		mutex_destroy(&(user->lock));
		mm_free(user);
		return NULL;
	}

	// rwlock_attr_destroy(&attr);

	return user;
}

/**
 * @brief	Update a meta user object from the cache.
 * @param	user	a pointer to the meta user object that should be updated.
 * @param	locked	if META_NEED_LOCK is specified, a writer's lock will be acquired for the meta user object.
 * @return	-1 on general failure, 0 if the user or its mailbox aliases could not be fetched, or 1 on success.
 */
int_t meta_user_update(meta_user_t *user, META_LOCK_STATUS locked) {

	int_t output = 0;
	uint64_t checkpoint;

	if (!user) {
		return -1;
	}

	// Do we need a lock.
	if (locked == META_NEED_LOCK) {
		meta_user_wlock(user);
	}

	// The message and folder update functions won't alter a context being used by a POP connection, but as of this writing the user configuration
	// doesn't affect anything that would confuse/alter an existing POP connection.
	if ((checkpoint = serial_get(OBJECT_USER, user->usernum)) != user->serials.user) {

		if (!checkpoint) {
			checkpoint = serial_increment(OBJECT_USER, user->usernum);
		}

		if ((output = meta_data_fetch_user(user)) && (output = meta_data_fetch_mailbox_aliases(user))) {
			user->serials.user = checkpoint;
		}

	}

	// Do we need to clear the lock.
	if (locked == META_NEED_LOCK) {
		meta_user_unlock(user);
	}

	return output;
}

/**
 * @brief	Build a user's meta information from specified data parameters.
 * @note	The user object will be pulled from the cache, if possible, or falls back to a database lookup by username+password.
 * @param	user		a pointer to the meta user object of the user that is to be filled.
 * @param	cred		Credentials object containing user authentication.
 * @param	locked		the meta lock status of the operation (if META_NEED_LOCK is supplied, the meta user object will be
 * 						locked for the duration of the function.
 * @return	-1 on error, 1 on success.
 */
// TODO: The return convention here is pretty weird. It should probably be cleaned up at some point.
int_t meta_user_build(meta_user_t *user, credential_t *cred, META_LOCK_STATUS locked) {

	uint64_t serial;
	int_t result = 0;

	// Sanity.
	if (!user || st_empty(cred->auth.username) || st_empty(cred->auth.password) || st_empty(cred->auth.key)) {
		return -1;
	}

	if(cred->type != CREDENTIAL_AUTH) {
		log_error("Credential object needs to be of type CREDENTIAL_AUTH to be used to build a meta_user_t.");
		return -1;
	}

	// Do we need a lock.
	if (locked == META_NEED_LOCK) {
		meta_user_wlock(user);
	}

	// Check for cached data.
	if (user->username && user->passhash && user->usernum) {

		// Check for a new checkpoint, otherwise assume the stored data is good.
		if ((serial = serial_get(OBJECT_USER, user->usernum)) == user->serials.user) {
			result = 1;
		}
		else if ((result = meta_data_fetch_user(user)) && (result = meta_data_fetch_mailbox_aliases(user))) {
			user->serials.user = serial;
		}

	}

	// We don't have a stored structure so check the database.
	else {

		// If were reusing a cache structure, we might need to destroy the old copy of the username.
		st_cleanup(user->username);

		if (!(user->username = st_dupe(cred->auth.username))) {
			result = -1;
		}
		else if ((result = meta_data_user_build(user, cred->auth.password, cred->auth.key, cred->authentication)) == 1 && (result = meta_data_fetch_mailbox_aliases(user)) &&
			!(user->serials.user = serial_get(OBJECT_USER, user->usernum))) {
			user->serials.user = serial_increment(OBJECT_USER, user->usernum);
		}

	}

	// Do we need to clear the lock.
	if (locked == META_NEED_LOCK) {
		meta_user_unlock(user);
	}

	return result;
}

/**
 * @brief	Set an object serial number for a given meta user structure.
 * @param	user	the meta user object to be adjusted.
 * @param	object	the object to receive the new serial number.
 * @param	serial	the new serial number to be set.
 * @return	This function returns no value.
 */
void meta_user_serial_set(meta_user_t *user, uint64_t object, uint64_t serial) {

	if (!user || !serial) {
		return;
	}
	else if (object == OBJECT_USER) {
		user->serials.user = serial;
	}
	else if (object == OBJECT_FOLDERS) {
		user->serials.folders = serial;
	}
	else if (object == OBJECT_MESSAGES) {
		user->serials.messages = serial;
	}
	else if (object == OBJECT_CONTACTS) {
		user->serials.contacts = serial;
	}

	return;
}

/**
 * @brief	Get an object serial number for a given meta user structure.
 * @param	user	the meta user structure to be examined.
 * @param	object	the object to query for the serial number.
 * @return	the serial number value of the specified object, or 0 on failure.
 */
uint64_t meta_user_serial_get(meta_user_t *user, uint64_t object) {

	uint64_t serial = 0;

	if (!user) {
		return serial;
	}
	else if (object == OBJECT_USER) {
		serial = user->serials.user;
	}
	else if (object == OBJECT_FOLDERS) {
		serial = user->serials.folders;
	}
	else if (object == OBJECT_MESSAGES) {
		serial = user->serials.messages;
	}
	else if (object == OBJECT_CONTACTS) {
		serial = user->serials.contacts;
	}

	return serial;
}

/**
 * @brief	Check an object's serial number to see if it is up-to-date.
 * @note	The object's serial number will be incremented regardless of whether it is consistent with the cache.
 * @param	user	the meta user object to whom the object belongs.
 * @param	object	the serial object to be checked for changes.
 * @return	0 if the object did not to be refreshed, or 1 if it doesn't match the internal checkpoint and should be updated.
 */
bool_t meta_user_serial_check(meta_user_t *user, uint64_t object) {

	int_t result = false;

	// If the serial number indicates no outside changes we can increment it without forcing a refresh.{
	if (user && meta_user_serial_get(user, object) == serial_get(object, user->usernum)) {
		meta_user_serial_set(user, object, serial_increment(object, user->usernum));
	}
	// Increment the reference counter and queue a session update.
	else if (user) {
		serial_increment(object, user->usernum);
		result = true;
	}

	return result;
}

/**
 * @brief	Lookup user by information and and return a meta user object.
 * @note	If the user is not found, one will be created.
 * @param	cred		Credentials object containing user log in information.
 * @param	flags		a set of flags specifying the protocol used by the calling function. Values can be META_PROT_NONE,
 * 						META_PROT_SMTP, META_PROT_POP, META_PROT_IMAP,  META_PROT_WEB, or META_PROT_GENERIC.
 * @param	get			a set of flags specifying the data to be retrieved (META_GET_NONE, META_GET_MESSAGES,
 * 						META_GET_FOLDERS, or META_GET_CONTACTS)
 * @param	output		the address of a meta user object that will store a pointer to the result of the lookup.
 * @return	-1 on error, 0 if the username information exists but there was an error, and 1 on success.
 */
int_t meta_get(credential_t *cred, META_PROT flags, META_GET get, meta_user_t **output) {

	int_t state;
	meta_user_t *user = NULL;
	stringer_t *mailbox = NULL;
	multi_t key = { .type = M_TYPE_STRINGER, .val.st = cred->auth.username };

	if (st_empty(cred->auth.username)) {
		return 0;
	}

	if(cred->type != CREDENTIAL_AUTH) {
		log_error("Specified credential object is not an authentication type.");
		return -1;
	}

	// Fail if the specified mailbox does not even exist.
	if (cred->auth.domain && !(mailbox = st_merge("sns", cred->auth.username, "@", cred->auth.domain))) {
		log_error("Unable to allocate space for user mailbox name.");
		return -1;
	} else if (cred->auth.domain && (!meta_data_check_mailbox(mailbox))) {
		st_free(mailbox);
		return 0;
	}

	st_cleanup(mailbox);

	// Pull the user from the cache, or add it.
	inx_lock_write(objects.users);

	// Pull the object.
	if (!(user = inx_find(objects.users, key))) {

		// We need to create a new one.
		if (!(user = meta_user_create()) || !inx_insert(objects.users, key, user)) {
			inx_unlock(objects.users);
			meta_user_destroy(user);
			log_pedantic("Could not create user info object.");
			return -1;
		}

	}

	// Add a reference.
	meta_user_ref_add(user, flags);
	inx_unlock(objects.users);

	meta_user_wlock(user);

	// Pull the user information.
	if ((state = meta_user_build(user, cred, META_LOCKED)) < 0) {
		meta_user_unlock(user);
		// QUESTION: Why not using meta_user_ref_dec() on these?
		meta_remove(cred->auth.username, flags);
		return state;
	}
	// If we have the user information already.
	else if (st_cmp_cs_eq(user->passhash, cred->auth.password)) {
		meta_user_unlock(user);
		meta_remove(cred->auth.username, flags);
		return 0;
	}

	// Are we supposed to get the messages.
	if ((get & META_GET_MESSAGES) && meta_messages_update(user, META_LOCKED) < 0) {
		meta_user_unlock(user);
		meta_remove(cred->auth.username, flags);
		return -1;
	}

	if ((get & META_GET_FOLDERS) && meta_message_folders_update(user, META_LOCKED) < 0) {
		meta_user_unlock(user);
		meta_remove(cred->auth.username, flags);
		return -1;
	}

	// Are we supposed to update the folders.
	if ((get & META_GET_FOLDERS) && meta_folders_update(user, META_LOCKED) < 0) {
		meta_user_unlock(user);
		meta_remove(cred->auth.username, flags);
		return -1;
	}

	// Are we supposed to update the folders.
	if ((get & META_GET_CONTACTS) && meta_contacts_update(user, META_LOCKED) < 0) {
		meta_user_unlock(user);
		meta_remove(cred->auth.username, flags);
		return -1;
	}

	*output = user;
	meta_user_unlock(user);

	return 1;
}
