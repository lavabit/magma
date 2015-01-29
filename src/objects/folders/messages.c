
/**
 * @file /magma/objects/folders/messages.c
 *
 * @brief	Interface with user message folders.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Free a message folder object.
 * @return	This function returns no value.
 */
void message_folder_free(message_folder_t *folder) {

	magma_folder_free(folder);
	return;
}

/**
 * @brief	Create and initialize a new message folder object.
 * @param	foldernum	the numerical id of the new message folder.
 * @param	parent		the numerical id of the parent folder containing the target message folder.
 * @param	order		the order number of this folder in its parent folder.
 * @param	name		a managed string containing the name of the new message folder.
 * @return	NULL on failure or a pointer to the newly allocated and initialized message folder object on success.
 */
message_folder_t * message_folder_alloc(uint64_t foldernum, uint64_t parent, uint32_t order, stringer_t *name) {

	message_folder_t *result;

	if (!(result = magma_folder_alloc(foldernum, parent, order, name)) || !(result->records = inx_alloc(M_INX_TREE, &message_free))) {
		mm_cleanup(result);
		return NULL;
	}

	return result;
}

/// TODO: Add validation: check whether the parent exists, and enforce the length and depth limits. Also make sure the name doesn't conflict
/// with any built in folder names. Finally, setup a list of error codes and return the appropriate value.
/**
 * @note	This function isn't called from anywhere else at the moment.
 */
int_t message_folder_create(uint64_t usernum, uint64_t parent, stringer_t *name, inx_t *folders) {

	uint32_t order = 0;
	message_folder_t *active;
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = 0 };

	if (st_empty(name) || !usernum || !folders ) {
		log_pedantic("We were passed an invalid pointer.");
		return 0;
	}

	// If a valid parent was provided, determine the next available order value. Otherwise just use zero.
	if (parent) {
		order = magma_folder_children(folders, parent);
	}

	if (!(key.val.u64 = magma_folder_insert(usernum, name, parent, order, M_FOLDER_MESSAGES)) || !(active = message_folder_alloc(key.val.u64, parent, order, name))) {
		return 0;
	}
	else if (!inx_insert(folders, key, active)) {
		message_folder_free(active);
		return 0;
	}

	return 1;
}

/**
 * @brief	Remove a user's message folder.
 * @see		magma_folder_delete()
 * @note	This operation will only succeed if the specified message folder is empty.
 * @param	usernum		the numerical id of the user requesting the message folder removal.
 * @param	foldernum	the numerical id of the folder to be removed.
 * @param	folders		an inx holder containing the message folders to be searched for the folder specified for removal.
 * @return	true on success or false on failure.
 */
bool_t message_folder_remove(uint64_t usernum, uint64_t foldernum, inx_t *folders) {

	message_folder_t *active;
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = foldernum };

	if (!usernum || !folders || !(active = inx_find(folders, key))) {
		log_pedantic("We were passed an invalid pointer.");
		return false;
	}
	// Were assuming that if the record index is NULL, and there are records associated with the folder in the database, the foreign
	// key will prevent us from deleting the folder row.
	else if (active->records && inx_count(active->records)) {
		log_pedantic("Attempt was made to delete a folder with active message records.");
		return false;
	}
	else if (magma_folder_children(folders, active->foldernum)) {
		log_pedantic("Attempt was made to delete a folder with child records.");
		return false;
	}

	if (!magma_folder_delete(usernum, foldernum, M_FOLDER_MESSAGES) || !inx_delete(folders, key)) {
		return false;
	}

	return true;
}
