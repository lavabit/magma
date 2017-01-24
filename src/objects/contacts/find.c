
/**
 * @file /magma/objects/contacts/find.c
 *
 * @brief	Functions to search a collection of contacts.
 */

#include "magma.h"

/**
 * @note	This is a bit of a weird function that currently isn't being called from anywhere. It may or may not be here to stay.
 */
contact_t * contact_find_detail(contact_folder_t *folder, stringer_t *key, stringer_t *target) {

	bool_t blank = false;
	inx_cursor_t *cursor;
	contact_detail_t *detail;
	contact_t *result = NULL, *active = NULL;
	multi_t multi = { .type = M_TYPE_STRINGER, .val.st = key };

	if (st_empty(key) || !folder || !folder->records || !(cursor = inx_cursor_alloc(folder->records))) {
		return NULL;
	}
	else if (st_empty(target)) {
		blank = true;
	}

	// Get the contact.
	while (!result && (active = inx_cursor_value_next(cursor))) {

		// Look for the detail record. If a detail entry is found compare the value to our target, otherwise if the target is empty (by the blank boolean) we will
		// match empty
		if ((detail = inx_find(active->details, multi)) && ((detail && !st_cmp_ci_eq(detail->value, target)) || (blank && (!detail || st_empty(detail->value))))) {
			result = active;
		}

	}

	inx_cursor_free(cursor);

	return result;
}

/**
 * @brief	Get a contact entry by name (case insensitive).
 * @param	folder	a pointer to the contact folder to have its contents searched for the entry.
 * @param	target	a managed string containing the name of the target entry.
 * @return	NULL on failure or a pointer to the found user contact entry on success.
 */
contact_t * contact_find_name(contact_folder_t *folder, stringer_t *target) {

	inx_cursor_t *cursor;
	contact_t *result = NULL, *active = NULL;

	if (st_empty(target) || !folder || !folder->records || !(cursor = inx_cursor_alloc(folder->records))) {
		return NULL;
	}

	// Get the name and compare.
	while (!result && (active = inx_cursor_value_next(cursor))) {

		if (!st_cmp_ci_eq(active->name, target)) {
			result = active;
		}

	}

	inx_cursor_free(cursor);

	return result;
}

/**
 * @brief	Find a contact entry in a contact folder by number.
 * @param	folder	a pointer to a contact folder object containing the contact entries to be searched.
 * @param	target	the numerical id of the contact entry to be located in the specified folder.
 * @return	NULL on failure or a pointer to the found user contact entry on success.
 */
contact_t * contact_find_number(contact_folder_t *folder, uint64_t target) {

	contact_t *result = NULL;
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = target };

	if (!target || !folder || !folder->records) {
		return NULL;
	}

	result = inx_find(folder->records, key);

#ifdef MAGMA_PEDANTIC
	if (!result) {
		log_pedantic("The requested folder number was not in the folders index. { folder = %lu }", target);
	}
#endif

	return result;
}
