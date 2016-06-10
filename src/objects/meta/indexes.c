
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
 * @param	protocol		specifies the protocol bound to the reference counter to be decremented (META_PROT_WEB, META_PROT_IMAP, etc.)
 * @return	This function returns no value.
 */
void meta_inx_remove(uint64_t usernum, META_PROTOCOL protocol) {

	meta_user_t *user = NULL;
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = usernum };

	if (!usernum) {
		return;
	}

	// Lock the object cache.
	inx_lock_read(objects.meta);

	// If we find the meta object, decrement the reference counter so it gets gets removed by the prune function.
	if ((user = inx_find(objects.meta, key))) {
		meta_user_ref_dec(user, protocol);
	}

	// Release the object cache.
	inx_unlock(objects.meta);

	return;
}

meta_user_t * meta_inx_find(uint64_t usernum, META_PROTOCOL protocol) {

	meta_user_t *user = NULL;
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = usernum };

	if (!usernum) {
		return NULL;
	}

	// Pull the user from the cache, or add it.
	inx_lock_write(objects.meta);

	// Pull the object.
	if (!(user = inx_find(objects.meta, key))) {

		// We need to create a new one.
		if (!(user = meta_alloc()) || !inx_insert(objects.meta, key, user)) {
			inx_unlock(objects.meta);
			meta_free(user);
			return NULL;
		}
		else {
			user->usernum = usernum;
		}

	}

	// Add a reference.
	meta_user_ref_add(user, protocol);
	inx_unlock(objects.meta);

	return user;
}
