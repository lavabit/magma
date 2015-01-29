
/**
 * @file /magma/objects/folders/find.c
 *
 * @brief	Functions to search for specified folders.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Get a magma folder by name.
 * @param	folders		an inx holder containing the collection of magma folders to be searched.
 * @param	target		a managed string containing the name of the magma folder to be found.
 * @param	parent		the numerical id of the parent folder in which to search for the specified magma folder.
 * @param	check_inbox	if true, a case-insensitive comparison will be used if the specified folder name is "Inbox".
 * @return	NULL on failure, or a pointer to the found folder on success.
 */
magma_folder_t * magma_folder_find_name(inx_t *folders, stringer_t *target, uint64_t parent, bool_t check_inbox) {

	bool_t inbox = false;
	inx_cursor_t *cursor;
	magma_folder_t *result = NULL, *active = NULL;

	if (!folders || st_empty(target) || !(cursor = inx_cursor_alloc(folders))) {
		return NULL;
	}

	else if (check_inbox && !st_cmp_ci_eq(target, PLACER("Inbox", 5))) {
		inbox = true;
	}

	// Get the name and compare; "Inbox" gets a special case-insensitive comparison.
	while (!result && (active = inx_cursor_value_next(cursor))) {

		if (!st_cmp_cs_eq(active->name, target) && (active->parent == parent)) {
			result = active;
		}
		else if (inbox && !st_cmp_ci_eq(active->name, target) && (active->parent == parent)) {
			result = active;
		}

	}

	inx_cursor_free(cursor);

	return result;
}

/**
 * @brief	Get a magma folder by its fully qualified name.
 * @see		magma_folder_name()
 * @param	folders		an inx holder containing the collection of magma folders to be searched.
 * @param	target		a managed string containing the fully qualified (expanded) name of the magma folder to be found.
 * @return	NULL on failure, or a pointer to the found folder on success.
 */
magma_folder_t * magma_folder_find_full_name(inx_t *folders, stringer_t *target, bool_t check_inbox) {

	bool_t inbox = false;
	stringer_t *current;
	inx_cursor_t *cursor;
	magma_folder_t *result = NULL, *active = NULL;

	if (!folders || st_empty(target) || !(cursor = inx_cursor_alloc(folders))) {
		return NULL;
	}

	else if (check_inbox && !st_cmp_ci_eq(target, PLACER("Inbox", 5))) {
		inbox = true;
	}

	// Get the name and compare; "Inbox" gets a special case-insensitive comparison.
	while (!result && (active = inx_cursor_value_next(cursor))) {

		if ((current = magma_folder_name(folders, active)) && !st_cmp_cs_eq(current, target)) {
			result = active;
		}
		else if (current && inbox && !st_cmp_ci_eq(current, target)) {
			result = active;
		}

		st_cleanup(current);
	}

	inx_cursor_free(cursor);

	return result;
}

/**
 * @brief	Get a magma folder by number.
 * @param	folders		an inx holder containing the collection of magma folders to be searched.
 * @param	target		the numerical id of the magma folder to be found.
 * @return	NULL on failure, or a pointer to the found folder on success.
 */
magma_folder_t * magma_folder_find_number(inx_t *folders, uint64_t target) {
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = target };

	if (!folders) {
		return NULL;
	}

	return (inx_find(folders, key));
}
