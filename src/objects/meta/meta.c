
/**
 * @file /magma/objects/meta/meta.c
 *
 * @brief The primary interface for the meta objects.
 */

#include "magma.h"

/**
 * @brief	Free a meta user object.
 *
 * @param	user	a pointer to the meta user object to be destroyed.
 *
 * @return	This function returns no value.
 */
void meta_free(meta_user_t *user) {

	if (user) {

		prime_cleanup(user->prime.key);
		prime_cleanup(user->prime.signet);

		inx_cleanup(user->aliases);
		inx_cleanup(user->folders);
		inx_cleanup(user->message_folders);
		inx_cleanup(user->messages);
		inx_cleanup(user->contacts);

		st_cleanup(user->username, user->verification, user->realm.mail);

		// When read/write locking issues have been fixed, this line can be used once again.
		rwlock_destroy(&(user->lock));
		mutex_destroy(&(user->refs.lock));

		mm_free(user);
	}

	return;
}

/**
 * @brief	Allocate and initialize a meta user object.
 *
 * @return	NULL on failure, or a pointer to the newly allocated meta user object on success.
 */
meta_user_t * meta_alloc(void) {

	meta_user_t *user;
	pthread_rwlockattr_t attr;

	// Configure the rwlock attributes structure to prefer write lock requests.
	if (rwlock_attr_init(&attr)) {
		log_pedantic("Unable to initialize the read/write lock attributes.");
		return NULL;
	}
	else if (rwlock_attr_setkind(&attr, PTHREAD_RWLOCK_PREFER_WRITER_NP)) {
		log_pedantic("Unable to set the read/write lock attributes.");
		rwlock_attr_destroy(&attr);
		return NULL;
	}

	// Allocate the meta user object.
	if (!(user = mm_alloc(sizeof(meta_user_t)))) {
		log_pedantic("Unable to allocate %zu bytes for a user meta information structure.", sizeof(meta_user_t));
		rwlock_attr_destroy(&attr);
		return NULL;
	}

	// Wipe the meta user object and set all of the pointers to NULL.
	mm_wipe(user, sizeof(meta_user_t));

	// Initialize the rwlock.
	if (rwlock_init(&(user->lock), &attr) != 0) {
		log_pedantic("Unable to initialize the read/write lock.");
		rwlock_attr_destroy(&attr);
		mm_free(user);
		return NULL;
	}
	// Initialize the mutex.
	else if (mutex_init(&(user->refs.lock), NULL) != 0) {
		log_pedantic("Unable to initialize the user reference lock.");
		rwlock_destroy(&(user->lock));
		rwlock_attr_destroy(&attr);
		mm_free(user);
		return NULL;
	}

	rwlock_attr_destroy(&attr);

	return user;
}

/**
 * @brief	Lookup user and return their meta user object.
 *
 * @note	If the user is not found in the local session cache, the session will be constructed using the database, and then cached.
 *
 * @param 	usernum			the numeric identifier for the user account.
 * @param 	username		the official username stored in the database.
 * @param	master			the user account's master encryption key which will be used to unlock the private storage key.
 * @param 	verification	the verification token.
 * @param	protocol			a set of protocol specifying the protocol used by the calling function. Values can be META_PROT_NONE,
 * 							META_PROT_SMTP, META_PROT_POP, META_PROT_IMAP, META_PROT_WEB, or META_PROT_GENERIC.
 * @param	get				a set of protocol specifying the data to be retrieved (META_GET_NONE, META_GET_MESSAGES,
 * 							META_GET_FOLDERS, or META_GET_CONTACTS)
 * @param	output			the address of a meta user object that will store a pointer to the result of the lookup.
 *
 * @return	-1 on error, 0 on success, 1 for an authentication issue.
 */
int_t meta_get(uint64_t usernum, stringer_t *username, stringer_t *master, stringer_t *verification, META_PROTOCOL protocol, META_GET get, meta_user_t **output) {

	int_t state;
	meta_user_t *user = NULL;

	// If the auth structure is empty, or the usernum is invalid, return an error immediately.
	if (!usernum || !st_populated(username, master, verification)) {
		log_pedantic("Invalid parameters were used to get the meta data object.");
		return -1;
	}

	// Pull the user from the usernum, or add it.
	if (!(user = meta_inx_find(usernum, protocol))) {
		log_pedantic("Could not find an existing user object, nor could we create one.");
		return -1;
	}

	meta_user_wlock(user);

	// Pull the user information.
	if ((state = meta_update_user(user, META_LOCKED)) < 0) {
		meta_user_unlock(user);
		meta_inx_remove(usernum, protocol);
		return state;
	}

	// The auth_t object should have checked the verification token already, but we check here just to be sure.
	else if (st_empty(user->verification) || st_cmp_cs_eq(verification, user->verification)) {
		meta_user_unlock(user);
		meta_inx_remove(usernum, protocol);
		return 1;
	}

	// Are we supposed to get the realm keys.
	if ((get & META_GET_KEYS) && meta_update_realms(user, master, META_LOCKED) < 0) {
		meta_user_unlock(user);
		meta_inx_remove(usernum, protocol);
		return -1;
	}

	// Are we supposed to get the mailbox keys.
	if ((get & META_GET_KEYS) && meta_update_keys(user, META_LOCKED) < 0) {
		meta_user_unlock(user);
		meta_inx_remove(usernum, protocol);
		return -1;
	}

 	 // Are we supposed to get the mailbox aliases.
	if ((get & META_GET_ALIASES) && meta_update_aliases(user, META_LOCKED) < 0) {
		meta_user_unlock(user);
		meta_inx_remove(usernum, protocol);
		return -1;
	}

	// Are we supposed to get the messages.
	if ((get & META_GET_MESSAGES) && meta_messages_update(user, META_LOCKED) < 0) {
		meta_user_unlock(user);
		meta_inx_remove(usernum, protocol);
		return -1;
	}

	if ((get & META_GET_FOLDERS) && meta_update_message_folders(user, META_LOCKED) < 0) {
		meta_user_unlock(user);
		meta_inx_remove(usernum, protocol);
		return -1;
	}

	// Are we supposed to update the folders.
	if ((get & META_GET_FOLDERS) && meta_update_folders(user, META_LOCKED) < 0) {
		meta_user_unlock(user);
		meta_inx_remove(usernum, protocol);
		return -1;
	}

	// Are we supposed to update the folders.
	if ((get & META_GET_CONTACTS) && meta_update_contacts(user, META_LOCKED) < 0) {
		meta_user_unlock(user);
		meta_inx_remove(usernum, protocol);
		return -1;
	}

	*output = user;
	meta_user_unlock(user);

	return 0;
}
