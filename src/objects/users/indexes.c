
/**
 * @file /magma/src/objects/users/indexes.c
 *
 * @brief The functions used to search, add and remove user objects from the local index.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Lock a user's object in the cache and decrement their reference counter.
 * @see		meta_user_ref_dec()
 * @param	username	a managed string containing the name of the user to be adjusted.
 * @param	flags		specifies the protocol bound to the reference counter to be decremented (META_PROT_WEB, META_PROT_IMAP, etc.)
 * @return	This function returns no value.
 */
void meta_inx_remove(stringer_t *username, META_PROT flags) {

	meta_user_t *user = NULL;
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

meta_user_t * meta_inx_find(stringer_t *username, META_PROT flags) {

	meta_user_t *user = NULL;
	multi_t key = { .type = M_TYPE_STRINGER, .val.st = username };

	if (st_empty(username)) {
		return NULL;
	}

	// Pull the user from the cache, or add it.
	inx_lock_write(objects.users);

	// Pull the object.
	if (!(user = inx_find(objects.users, key))) {

		// We need to create a new one.
		if (!(user = meta_user_create()) || !inx_insert(objects.users, key, user)) {
			inx_unlock(objects.users);
			meta_user_destroy(user);
			return NULL;
		}

	}

	// Add a reference.
	meta_user_ref_add(user, flags);
	inx_unlock(objects.users);

	return user;
}
