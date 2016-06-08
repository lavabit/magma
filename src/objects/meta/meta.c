
/**
 * @file /magma/src/objects/meta/meta.c
 *
 * @brief The primary interface for the new_meta objects.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Free a meta object.
 *
 * @param	user	a pointer to the meta object to be destroyed.
 *
 * @return	This function returns no value.
 */
void new_meta_free(new_meta_user_t *user) {

	if (user) {

		inx_cleanup(user->aliases);
		inx_cleanup(user->folders);
		inx_cleanup(user->message_folders);
		inx_cleanup(user->messages);
		inx_cleanup(user->contacts);

		st_cleanup(user->username, user->verification);
		st_cleanup(user->keys.public, user->keys.private);

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
new_meta_user_t * new_meta_alloc(void) {

	new_meta_user_t *user;
	pthread_rwlockattr_t attr;

	// Configure the rwlock attributes to prefer write lock requests.
	if (rwlock_attr_init(&attr)) {
		log_pedantic("Unable to initialize the read/write lock attributes.");
		return NULL;
	}
	else if (rwlock_attr_setkind(&attr, PTHREAD_RWLOCK_PREFER_WRITER_NP)) {
		log_pedantic("Unable to set the read/write lock attributes.");
		rwlock_attr_destroy(&attr);
		return NULL;
	}

	if (!(user = mm_alloc(sizeof(new_meta_user_t)))) {
		log_pedantic("Unable to allocate %zu bytes for a user meta information structure.", sizeof(new_meta_user_t));
		rwlock_attr_destroy(&attr);
		return NULL;
	}
	else if (rwlock_init(&(user->lock), &attr) != 0) {
		log_pedantic("Unable to initialize the read/write lock.");
		rwlock_attr_destroy(&attr);
	}
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
 * @param	auth		Authentication object generated during the user log in process.
 * @param	flags		a set of flags specifying the protocol used by the calling function. Values can be META_PROT_NONE,
 * 						META_PROT_SMTP, META_PROT_POP, META_PROT_IMAP, META_PROT_WEB, or META_PROT_GENERIC.
 * @param	get			a set of flags specifying the data to be retrieved (META_GET_NONE, META_GET_MESSAGES,
 * 						META_GET_FOLDERS, or META_GET_CONTACTS)
 * @param	output		the address of a meta user object that will store a pointer to the result of the lookup.
 *
 * @return	-1 on error, 0 on success, 1 for an authentication issue.
 */
int_t new_meta_get(uint64_t usernum, stringer_t *username, stringer_t *master, stringer_t *verification, META_PROTOCOL flags, META_GET get, new_meta_user_t **output) {

	int_t state;
	new_meta_user_t *user = NULL;

	// If the auth structure is empty, or the usernum is invalid, return an error immediately.
	if (!usernum || !st_populated(username, master, verification)) {
		log_pedantic("Invalid parameters were used to get the meta data object.");
		return -1;
	}

	// Pull the user from the usernum, or add it.
	if (!(user = new_meta_inx_find(usernum, flags))) {
		log_pedantic("Could not find an existing user object, nor could we create one.");
		return -1;
	}

	new_meta_user_wlock(user);

	// Pull the user information.
	if ((state = new_meta_user_update(user, META_LOCKED)) < 0) {
		new_meta_user_unlock(user);
		new_meta_inx_remove(usernum, flags);
		return state;
	}

 	 // Are we supposed to get the mailbox aliases.
	if ((get & META_GET_ALIASES) && meta_aliases_update(user, META_LOCKED) < 0) {
		new_meta_user_unlock(user);
		new_meta_inx_remove(usernum, flags);
		return -1;
	}

	// Are we supposed to get the messages.
	if ((get & META_GET_MESSAGES) && meta_messages_update(user, META_LOCKED) < 0) {
		new_meta_user_unlock(user);
		new_meta_inx_remove(usernum, flags);
		return -1;
	}

	if ((get & META_GET_FOLDERS) && meta_message_folders_update(user, META_LOCKED) < 0) {
		new_meta_user_unlock(user);
		new_meta_inx_remove(usernum, flags);
		return -1;
	}

	// Are we supposed to update the folders.
	if ((get & META_GET_FOLDERS) && new_meta_folders_update(user, META_LOCKED) < 0) {
		new_meta_user_unlock(user);
		new_meta_inx_remove(usernum, flags);
		return -1;
	}

	// Are we supposed to update the folders.
	if ((get & META_GET_CONTACTS) && new_meta_contacts_update(user, META_LOCKED) < 0) {
		new_meta_user_unlock(user);
		new_meta_inx_remove(usernum, flags);
		return -1;
	}

	*output = user;
	new_meta_user_unlock(user);

	return 0;
}
