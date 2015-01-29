
/**
 * @file /magma/web/portal/contacts.c
 *
 * @brief	Functions for handling user contact list entries.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/// TODO: Right now this function doesn't really do anything. It needs to be updated if/once flag values are determined.
/**
 * @brief	Pack a json array representing the flags associated with a user contact entry detail.
 * @param	detail	a pointer to the user contact detail to have its flags queried.
 * @return	NULL on failure or a pointer to a json array containing strings describing the specified contact detail's flags.
 */
json_t * portal_contact_detail_flags(contact_detail_t *detail) {

	json_t *array;

	if ((array = json_array_d())) {

		if (detail->flags & CONTACT_DETAIL_FLAG_CRITICAL) {
			json_array_append_new_d(array, json_string_d("critical"));
		}

	}
	return array;
}

/**
 * @brief	Retrieve all the details about a contact entry as a json object.
 * @param	contact		a pointer to the contact object whose details will be retrieved.
 * @return	a pointer to a json object containing the details of the specified contact entry.
 */
json_t * portal_contact_details(contact_t *contact) {

	json_error_t err;
	inx_cursor_t *cursor;
	contact_detail_t *detail;
	json_t *flags = NULL, *item = NULL, *result = NULL;

	if (!(result = json_pack_ex_d(&err, JSON_ENSURE_ASCII, "{s:I}", "contactID", contact->contactnum))) {
		log_error("Failed to pack user contact details.  { contact = %lu / error = %s }", contact->contactnum, err.text);
		return NULL;
	}
	else if (!(flags = json_array_d()) || !(item = json_pack_ex_d(&err, JSON_ENSURE_ASCII, "{s:s, s:o}", "value", st_char_get(contact->name),
		"flags", flags)) || json_object_set_new_d(result, "name", item)) {
		log_pedantic("The contact name failed to pack properly. { contact = %lu }", contact->contactnum);

		// If item is a valid pointer then it should have captured the reference to flags.
		if (item) json_decref_d(item);
		else if (flags) json_decref_d(flags);
		json_decref_d(result);
		return NULL;
	}

	if ((cursor = inx_cursor_alloc(contact->details))) {

		while ((detail = inx_cursor_value_next(cursor))) {

			if (!(flags = portal_contact_detail_flags(detail))) {
				log_pedantic("Contact detail flags failed to serialize. { contact = %lu }", contact->contactnum);
			}
			else if (!(item = json_pack_ex_d(&err, JSON_ENSURE_ASCII, "{s:s, s:o}", "value", st_char_get(detail->value), "flags", flags))) {
				log_pedantic("A contact detail record failed to pack properly. { contact = %lu / error = %s }", contact->contactnum, err.text);
				json_decref_d(flags);
			}

			else if (json_object_set_new_d(result, st_char_get(detail->key), item)) {
				log_pedantic("An error occurred while trying to set a contact detail key. { contact = %lu }", contact->contactnum);
				json_decref_d(item);
			}

		}

		inx_cursor_free(cursor);
	}

	return result;
}
