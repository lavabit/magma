
/**
 * @file /magma/objects/folders/folders.c
 *
 * @brief	Interface with user folders.
 */

#include "magma.h"

/// TODO: The assumption is that message folders will start using the interface as well.

/**
 * @brief	Get the fully expanded name of a folder.
 * @note	The folder hierarchy will be delimited with a "." character.
 * @param	folders		the inx object holding all of a user's magma folders.
 * @param	target		a pointer to the magma folder object to have its name expanded.
 * @return	NULL on failure or a pointer to a managed string with the fully expanded name of the specified folder.
 */
stringer_t * magma_folder_name(inx_t *folders, magma_folder_t *target) {

	stringer_t *result;
	int_t recursion = 0;

	if (!folders || !target || !target->foldernum || st_empty(target->name)) {
		log_pedantic("Invalid folder context. Unable to construct the name.");
		return NULL;
	}

	else if (!(result = st_dupe_opts(MANAGED_T | JOINTED | HEAP, target->name))) {
		log_pedantic("We were unable to duplicate the folder name.");
		return NULL;
	}

	while (target && target->parent != 0 && recursion++ < FOLDER_RECURSION_LIMIT) {

		// Get the parent target.
		if ((target = magma_folder_find_number(folders, target->parent)) == NULL) {
			log_pedantic("There appears to be a folder with an invalid parent.");
			return result;
		}

		// Append the seperator and then the parent name.
		result = st_append(result, PLACER(".", 1));
		result = st_append(result, target->name);
	}

	return result;
}

/**
 * @brief	Get the number of child folders a specified folder has.
 * @param	folders		an inx object holding all of the user's folders.
 * @param	target		the folder number of the target folder.
 * @return	the number of child folders contained by the specified folder, or 0 on failure.
 */
// QUESTION: Shouldn't failure return -1?
int_t magma_folder_children(inx_t *folders, uint64_t target) {

	int_t result = 0;
	inx_cursor_t *cursor;
	magma_folder_t *active = NULL;

	if (!target || !(cursor = inx_cursor_alloc(folders))) {
		return 0;
	}

	while ((active = inx_cursor_value_next(cursor)) && !result) {

		if (active->parent == target) {
			result++;
		}

	}

	inx_cursor_free(cursor);

	return result;
}

/**
 * @brief	Free a magma folder object and its underlying records.
 * @return	This function returns no value.
 */
void magma_folder_free(magma_folder_t *folder) {

	if (folder) {
		inx_cleanup(folder->records);
		rwlock_destroy(&(folder->lock));
		mm_free(folder);
	}

	return;
}

/**
 * @brief	Create a new magma folder object.
 * @param	foldernum	the numerical id of this folder.
 * @param	parent		the folder id of this folder's containing parent folder.
 * @param	order		the order number of this folder in its parent folder.
 * @param	name		a managed string with the name of the target folder.
 * @return	NULL on failure or a pointer to the newly allocated magma folder object on success.
 */
magma_folder_t * magma_folder_alloc(uint64_t foldernum, uint64_t parent, uint32_t order, stringer_t *name) {

	magma_folder_t *result;

	if (!(result = mm_alloc(align(16, sizeof(magma_folder_t) + sizeof(placer_t)) + align(8, st_length_get(name) + 1)))) {
		log_pedantic("Unable to allocate %zu bytes for a folder structure.", align(16, sizeof(magma_folder_t) + sizeof(placer_t)) + align(8, st_length_get(name) + 1));
		return NULL;
	}
	else if (rwlock_init(&(result->lock), NULL)) {
		log_pedantic("Unable to initialize the magma folder thread lock.");
		mm_free(result);
		return NULL;
	}

	result->foldernum = foldernum;
	result->parent = parent;
	result->order = order;
	result->name = (placer_t *)((chr_t *)result + sizeof(magma_folder_t));
	((placer_t *)result->name)->opts = PLACER_T | JOINTED | STACK | FOREIGNDATA;
	((placer_t *)result->name)->length = st_length_get(name);
	((placer_t *)result->name)->data = (chr_t *)result + align(16, sizeof(magma_folder_t) + sizeof(placer_t));
	mm_copy(st_data_get(result->name), st_data_get(name), st_length_get(name));

	return result;
}

/**
 * @brief	Retrieve the appropriate folder management functions for message folders or contact folders.
 * @param	type			the magma folder class of the desired management functions (M_FOLDER_MESSAGES or M_FOLDER_CONTACTS).
 * @param	folder_alloc	the address of a folder allocation function pointer that will be updated appropriately.
 * @param	folder_free		the address of a folder free function pointer that will be updated appropriately.
 * @return	true on success or false on failure.
 */
bool_t magma_folder_funcs(uint_t type, magma_folder_t * (**folder_alloc)(uint64_t, uint64_t, uint32_t, stringer_t *), void (**folder_free)(magma_folder_t *)) {

	bool_t result = false;

	if (!folder_alloc || !folder_free) {
		return false;
	}

	switch (type) {
		case (M_FOLDER_MESSAGES):
			*folder_free = &message_folder_free;
			*folder_alloc = &message_folder_alloc;
			result = true;
			break;

		case (M_FOLDER_CONTACTS):
			*folder_free = &contact_folder_free;
			*folder_alloc = &contact_folder_alloc;
			result = true;
			break;
	}

	return result;
}
