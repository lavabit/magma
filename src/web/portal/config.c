
/**
 * @file /magma/web/portal/config.c
 *
 * @brief	Utilities functions for handling user config entries.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Get a user config entry's flags as a json object.
 * @param	entry	a pointer to the user config entry object to have its flags serialized.
 * @return	NULL on failure or a pointer to a json object storing the user's flags on success.
 */
json_t * portal_config_entry_flags(user_config_entry_t *entry) {

	json_t *array, *flag;

	if ((array = json_array_d())) {

		if ((entry->flags & USER_CONF_STATUS_CRITICAL) && (flag = json_string_d("critical")) && json_array_append_new_d(array, flag)) {
			json_decref_d(flag);
			return NULL;
		}

	}
	return array;
}

/***
 * @brief	Get a user config entry as a json object.
 * @param	entry	a pointer to the user config entry object to be serialized.
 * @return	NULL on failure, or a pointer to a json object on success.
 */
json_t * portal_config_entry(user_config_entry_t *entry) {

	json_error_t err;
	json_t *flags = NULL, *item = NULL;

	if (!(flags = portal_config_entry_flags(entry))) {
		log_pedantic("Config entry flags failed to serialize. { key = %.*s / flags = %lu }", st_length_int(entry->key), st_char_get(entry->key), entry->flags);
		return NULL;
	}

	else if (!(item = json_pack_ex_d(&err, JSON_ENSURE_ASCII, "{s:s, s:o}", "value", st_char_get(entry->value), "flags", flags))) {
		log_pedantic("The config entry failed to pack properly. { key = %.*s / value = %.*s / error = %s }", st_length_int(entry->key), st_char_get(entry->key),
			st_length_int(entry->value), st_char_get(entry->value), err.text);

		// If item is a valid pointer then it should have captured the reference to flags.
		json_decref_d(flags);
		return NULL;
	}

	return item;
}

/**
 * @brief	Get a collection of user config options as a json object.
 * @param	collection	a pointer to a user config object that contains all the config options pairs to be serialized.
 * @return	NULL on failure, or a pointer to a json object containing the collection of user config options on success.
 */
json_t * portal_config_collection(user_config_t *collection) {

	inx_cursor_t *cursor;
	user_config_entry_t *entry;
	json_t *result = NULL, *item;

	if (!(result = json_object_d())) {
		log_error("Unable to allocate a JSON container object for the config entry collection. { usernum  = %lu }", collection->usernum);
		return NULL;
	}

	if ((cursor = inx_cursor_alloc(collection->entries))) {

		while ((entry = inx_cursor_value_next(cursor))) {

			// If the entry was serialized into a JSON object but were unable to append it to the array we need make sure the reference is released.
			if (!(item = portal_config_entry(entry))) {

				if (entry->flags & USER_CONF_STATUS_CRITICAL) {
					log_options(M_LOG_CRITICAL, "Failed to serialize portal config entry for { user = %u, key = %s }", (unsigned int) collection->usernum, st_char_get(entry->key));
					json_decref_d(result);
					return NULL;
				}

			} else if (json_object_set_new_d(result, st_char_get(entry->key), item)) {
				json_decref_d(item);

				if (entry->flags & USER_CONF_STATUS_CRITICAL) {
					log_options(M_LOG_CRITICAL, "Failed to add portal config entry for { user = %u, key = %s }", (unsigned int) collection->usernum, st_char_get(entry->key));
					json_decref_d(result);
					return NULL;
				}
			}
		}

		inx_cursor_free(cursor);
	}

	return result;
}
