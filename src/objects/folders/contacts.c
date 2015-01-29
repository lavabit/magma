
/**
 * @file /magma/objects/folders/contacts.c
 *
 * @brief	Interface for user contact folders.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Free a contact folder object.
 * @param	folder	a pointer to the contact folder object to be freed.
 * @return	This function returns no value.
 */
void contact_folder_free(contact_folder_t *folder) {

	magma_folder_free(folder);
	return;
}

/**
 * @brief	Allocate and initialize a new contact folder object.
 * @param	foldernum	the numerical id of this contact folder.
 * @param	parent		the numerical id of the parent folder of this contact folder.
 * @param	order		the order number of this folder in its parent folder.
 * @param	name		a managed string containing the name of this folder.
 * @return	NULL on failure, or a pointer to the newly initialized contact folder object on success.
 */
contact_folder_t * contact_folder_alloc(uint64_t foldernum, uint64_t parent, uint32_t order, stringer_t *name) {

	contact_folder_t *result;

	if (!(result = magma_folder_alloc(foldernum, parent, order, name)) || !(result->records = inx_alloc(M_INX_TREE, &contact_free))) {
		mm_cleanup(result);
		return NULL;
	}

	return result;
}

/**
 * @brief	Create a new contact folder.
 * @param	usernum		the numerical id of the user to whom the new imap folder will belong.
 * @param	parent		the numerical id of the parent folder to contain the new contact folder, or 0 for a root folder.
 * @param	name		a managed string containing the fully qualified name of the new contact folder to be created.
 * @param	folders		an inx holder containing the set of folders into which the new contact folder will be inserted.
 * @return	1 on success or <= 0 on failure.
 *          0:	An invalid parameter was passed to the function, or an internal failure occurred.
 *         -1:	The specified new folder name was invalid.
 *         -2:	Part of the folder path name exceeds 16 characters (FOLDER_LENGTH_LIMIT) after being unescaped for quotation marks.
 *         -3:	The specified folder name already exists.
 *         -4:	An invalid parent folder was specified.
 *         -5:	Part of the folder path name exceeds 16 characters (FOLDER_LENGTH_LIMIT) after being unescaped for quotation marks.
 */
int_t contact_folder_create(uint64_t usernum, uint64_t parent, stringer_t *name, inx_t *folders) {

	uint64_t nextparent = parent;
	uint32_t order = 0;
	contact_folder_t *active;
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = 0 };
	magma_folder_t *current;
	size_t numlevels = 1;

	if (st_empty(name) || !usernum || !folders ) {
		log_pedantic("We were passed an invalid pointer.");
		return 0;
	}

	// TODO: Right now we conform to imap name validity but more fine-tuned checks to be made, in line with that of imap folder validation.
	if (!imap_valid_folder_name(name)) {
		return -1;
	} else if (st_length_get(name) > FOLDER_LENGTH_LIMIT) {
		return -2;
	} else if (magma_folder_find_name(folders, name, parent, false)) {
		return -3;
	}

	// Check to see if we exceed the maximum recursion depth.
	while (nextparent) {
		numlevels++;

		if (!(current = magma_folder_find_number(folders, nextparent))) {
			return -4;
		}

		nextparent = current->parent;
	}

	if (numlevels > IMAP_FOLDER_RECURSION_LMIIT) {
		return -5;
	}

	// If a valid parent was provided, determine the next available order value. Otherwise just use zero.
	if (parent) {
		order = magma_folder_children(folders, parent);
	}

	if (!(key.val.u64 = magma_folder_insert(usernum, name, parent, order, M_FOLDER_CONTACTS)) || !(active = contact_folder_alloc(key.val.u64, parent, order, name))) {
		return 0;
	}
	else if (!inx_insert(folders, key, active)) {
		contact_folder_free(active);
		return 0;
	}

	return 1;
}

/**
 * @brief	Remove a user's contact folder.
 * @see		magma_folder_delete()
 * @note	This operation will only succeed if the specified contact folder is empty.
 * @param	usernum		the numerical id of the user requesting the contact folder removal.
 * @param	foldernum	the numerical id of the folder to be removed.
 * @param	folders		an inx holder containing the contact folders to be searched for the folder specified for removal.
 * @return	0 on success or < 0 on failure.
 *         -1:	There was an unexpected internal error.
 *         -2:	The specified folder could not be found.
 *         -3:	Contact folder was not empty.
 */
int_t contact_folder_remove(uint64_t usernum, uint64_t foldernum, inx_t *folders) {

	contact_folder_t *active;
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = foldernum };

	if (!usernum || !folders) {
		log_pedantic("We were passed an invalid pointer.");
		return -1;
	}
	else if (!(active = inx_find(folders, key))) {
		log_pedantic("Could not find contact folder to be removed.");
		return -2;
	}
	// Were assuming that if the record index is NULL, and there are records associated with the folder in the database, the foreign
	// key will prevent us from deleting the folder row.
	else if (active->records && inx_count(active->records)) {
		log_pedantic("Attempt was made to delete a folder with active contact records.");
		return -3;
	}
	else if (magma_folder_children(folders, active->foldernum)) {
		log_pedantic("Attempt was made to delete a folder with child records.");
		return -3;
	}

	if (!magma_folder_delete(usernum, foldernum, M_FOLDER_CONTACTS) || !inx_delete(folders, key)) {
		return -1;
	}

	return 0;
}

/**
 * @brief	Rename a user's contact folder.
 * @see		magma_folder_rename()
 * @param	usernum		the numerical id of the user requesting the contact folder renaming.
 * @param	foldernum	the numerical id of the folder to be renamed.
 * @param	rename		a managed string containing the new name of the specified folder.
 * @param	folders		an inx holder containing the contact folders to be searched for the folder specified to be renamed.
 * @return	true on success or false on failure.
 */
bool_t contact_folder_rename(uint64_t usernum, uint64_t foldernum, stringer_t *rename, inx_t *folders) {

	contact_folder_t *active;
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = foldernum };

	if (!usernum || st_empty(rename) || !folders || !(active = inx_find(folders, key))) {
		log_pedantic("We were passed an invalid pointer.");
		return false;
	}

	if (!imap_valid_folder_name(rename)) {
		return false;
	 } else if (st_length_get(rename) > FOLDER_LENGTH_LIMIT) {
		 return false;
	 }
	 else if (magma_folder_find_name(folders, rename, foldernum, false)) {
		 return false;
	 }

	//if (!magma_folder_rename(usernum, foldernum, M_FOLDER_CONTACTS, rename) || !inx_delete(folders, key)) {
	if (!magma_folder_rename(usernum, foldernum, M_FOLDER_CONTACTS, rename)) {
		return false;
	}

	// A lot simpler... just wipe in place.
	st_free(active->name);
	active->name = st_dupe(rename);

	// TODO: This function should return values more in line with imap_folder_rename() ?
	// We also need a LOT more validation.

	return true;
}
