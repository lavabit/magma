
/**
 * @file /magma/objects/config/config.c
 *
 * @brief	The user configuration interface.
 */

#include "magma.h"

/**
 * @brief	Destroy a user config entry.
 * @param	entry	a pointer to the user config entry object to be freed.
 * @return	This function returns no value.
 */
void user_config_entry_free(user_config_entry_t *entry) {

	if (entry) {
		mm_free(entry);
	}

	return;
}

/**
 * @brief	Free a user config collection object.
 * @param	collection		a pointer to the user config collection object to be freed.
 * @return	This function returns no value.
 */
void user_config_free(user_config_t *collection) {

	if (collection) {
		inx_cleanup(collection->entries);
		mm_free(collection);
	}

	return;
}

/**
 * @brief	Allocate a user config collection object for a user.
 * @param	usernum	the numerical user id for the requested user.
 * @return	NULL on failure or a pointer to the newly allocated user config collection object on success.
 */
user_config_t * user_config_alloc(uint64_t usernum) {

	user_config_t *result;

	if (!(result = mm_alloc(align(16, sizeof(user_config_t))))) {
		log_pedantic("Unable to allocate %zu bytes for a user configuration collection.", align(16, sizeof(user_config_t)));
		return NULL;
	}
	else if (!(result->entries = inx_alloc(M_INX_TREE, &user_config_entry_free))) {
		log_pedantic("Unable to initialize the user configuration collection.");
		mm_free(result);
		return NULL;
	}

	result->usernum = usernum;

	return result;
}

/**
 * @brief	Create and initialize new user config entry object.
 * @param	key		a managed string containing the key of the config entry object.
 * @param	value	a managed string containing the value of the config entry object.
 * @param	flags	a bitmask reflecting the flags of the config entry object.
 * @return	NULL on failure, or a pointer to the newly allocated and initialized user config entry object on success.
 */
user_config_entry_t * user_config_entry_alloc(stringer_t *key, stringer_t *value, uint64_t flags) {

	user_config_entry_t *result;

	if (!(result = mm_alloc(align(16, sizeof(user_config_entry_t) + sizeof(placer_t) + sizeof(placer_t)) +
		align(8, st_length_get(key) + 1) + align(8, st_length_get(value) + 1)))) {
		log_pedantic("Unable to allocate %zu bytes for a user configuration entry.", align(16, sizeof(user_config_entry_t) + sizeof(placer_t) + sizeof(placer_t)) +
			align(8, st_length_get(key) + 1) + align(8, st_length_get(value) + 1));
		return NULL;
	}

	result->flags = flags;

	result->key = (placer_t *)((chr_t *)result + sizeof(user_config_entry_t));
	result->value = (placer_t *)((chr_t *)result + sizeof(user_config_entry_t) + sizeof(placer_t));

	((placer_t *)result->key)->opts = PLACER_T | JOINTED | STACK | FOREIGNDATA;
	((placer_t *)result->key)->length = st_length_get(key);
	((placer_t *)result->key)->data = (chr_t *)result + align(16, sizeof(user_config_entry_t) + sizeof(placer_t) + sizeof(placer_t));

	((placer_t *)result->value)->opts = PLACER_T | JOINTED | STACK | FOREIGNDATA;
	((placer_t *)result->value)->length = st_length_get(value);
	((placer_t *)result->value)->data = (chr_t *)result + align(16, sizeof(user_config_entry_t) + sizeof(placer_t) + sizeof(placer_t)) +
		align(8, st_length_get(key) + 1);

	mm_copy(st_data_get(result->key), st_data_get(key), st_length_get(key));
	mm_copy(st_data_get(result->value), st_data_get(value), st_length_get(value));

	return result;
}

/**
 * @brief	Create a new user config collection object for the specified user and populate it with config entries from the database.
 * @param	usernum		the numerical user id for the user to be queried.
 * @return	NULL on failure, or a pointer to the newly created user config collection object on success.
 */
user_config_t * user_config_create(uint64_t usernum) {

	user_config_t *collection = NULL;

	if (!(collection = user_config_alloc(usernum)) || !user_config_fetch(collection)) {
		log_pedantic("Unable to load the user's config settings. { usernum = %lu }", usernum);

		if (collection) {
			user_config_free(collection);
		}

		return NULL;
	}

	collection->serial = serial_get(OBJECT_CONFIG, collection->usernum);

	return collection;
}

/**
 * @brief	Update a collection of user config options from the database, if necessary.
 * @note	This function is not currently being used anywhere.
 * 			If this function returns -1, the config entries are left in an undefined state and should not be relied upon.
 * @param	collection	a pointer to a collection of user config entries to be checked for updates.
 * @return	1 if the collection is up-to-date, 0 if it was refreshed to update changes, or -1 if the refresh operation failed.
 */
int_t user_config_update(user_config_t *collection) {

	int_t ret = 1;

	if (collection && collection->serial != serial_get(OBJECT_CONFIG, collection->usernum)) {

		// Remove all of the existing records.
		inx_truncate(collection->entries);

		// Then execute a fresh fetch request.
		if (!user_config_fetch(collection)) {
			ret = -1;
		}
		else {
			ret = 0;
		}
	}

	return ret;
}

/**
 * @brief	Change the value of, or delete a key from a user's config collection.
 * @param	collection		a pointer to a collection of user's config entries.
 * @param	key				a pointer to a managed string containing the name of the user config key to be modified.
 * @param	value			a pointer to a managed string containing the name of the new value of the key, or NULL if it is to be deleted.
 * @return	-1 on error or 1 on success.
 */
int_t user_config_edit(user_config_t *collection, stringer_t *key, stringer_t *value) {

	int_t modified = 0;
	user_config_entry_t *entry = NULL;
	multi_t multi = { .type = M_TYPE_STRINGER, .val.st = NULL };

	if (!collection || !collection->usernum || !collection->entries || st_empty(key)) {
		log_pedantic("Invalid config edit request.");
		return -1;
	}

	// A NULL value is used to remove a config key completely.
	else if (!value) {

		if ((modified = user_config_delete(collection->usernum, key)) < 0 || !(multi.val.st = key)) {
			log_pedantic("Failed to delete user config entry.");
			return -1;
		}

		// If the delete was successful, remove any existing entries from the context. Deleting a config entry that doesn't exist isn't considered an
		// error so there is no need to check the result.
		inx_delete(collection->entries, multi);

	}
	// If the value isn't empty we need to update the database.
	else {

		multi.val.st = key;

		// Don't update a value if it's already the same.
		if ((entry = inx_find(collection->entries, multi)) && (!st_cmp_cs_eq(entry->value, value))) {
			return 1;
		}

		if ((modified = user_config_upsert(collection->usernum, key, value, 0)) < 0 || !(entry = user_config_entry_alloc(key, value, 0))) {
			log_pedantic("Config entry creation failed.");
			return -1;
		}
		 // If the edit was successful, create a new detail record and update the contact record.
		else if (!(multi.val.st = entry->key) || !inx_replace(collection->entries, multi, entry)) {
			log_pedantic("The config entry was unable to accept the updated value.");
			user_config_entry_free(entry);
			return -1;
		}

	}

	// A value greater than 0 indicates the database was modified, while a return value of 0 indicates the database matched our updated value, or a matching record couldn't be located.
	if (modified > 0) {
		serial_increment(OBJECT_CONFIG, collection->usernum);
	}

	return 1;
}
