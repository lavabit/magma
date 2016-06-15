
/**
 * @file /magma/objects/config/datatier.c
 *
 * @brief	The database routines for the user configuration interface.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Delete a user config entry from the database by key.
 * @param	usernum		the numerical userid of the user to whom the requested config key belongs.
 * @param	key			a managed string containing the name of the config key to be deleted.
 * @return	1 if the config option was successfully deleted, 0 if the config key couldn't be found in the database, or -1 on general failure.
 */
int_t user_config_delete(uint64_t usernum, stringer_t *key) {

	int64_t affected;
	MYSQL_BIND parameters[4];

	mm_wipe(parameters, sizeof(parameters));

	// User Number
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &usernum;
	parameters[0].is_unsigned = true;

	// Key
	parameters[1].buffer_type = MYSQL_TYPE_STRING;
	parameters[1].buffer_length = st_length_get(key);
	parameters[1].buffer = st_char_get(key);

	if ((affected = stmt_exec_affected(stmts.delete_user_config, parameters)) == -1) {
		log_pedantic("The user config deletion request triggered an error. { user = %lu / key = %.*s }", usernum, st_length_int(key), st_char_get(key));
		return -1;
	}

	log_check(affected > 2);

	return (int_t)affected;
}

/**
 * @brief	Update a user config entry in the database, or insert it if it does not already exist.
 * @param	usernum		the numerical id of the user to whom the specified config entry belongs.
 * @param	key			a pointer to a managed string containing the name of the config entry to be updated or inserted.
 * @param	value		a pointer to a managed string containing the value of the specified config entry key.
 * @param	flags		a bitmask of flags for the config entry (USER_CONF_STATUS_CRITICAL is supported).
 * @return	2 if the config entry was updated, 1 if a new config key was inserted into the database, 0 if no update was necessary, or -1 on general failure.
 */
int_t user_config_upsert(uint64_t usernum, stringer_t *key, stringer_t *value, uint64_t flags) {

	int64_t affected;
	MYSQL_BIND parameters[4];

	mm_wipe(parameters, sizeof(parameters));

	// User Number
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &usernum;
	parameters[0].is_unsigned = true;

	// Key
	parameters[1].buffer_type = MYSQL_TYPE_STRING;
	parameters[1].buffer_length = st_length_get(key);
	parameters[1].buffer = st_char_get(key);

	// Value
	parameters[2].buffer_type = MYSQL_TYPE_STRING;
	parameters[2].buffer_length = st_length_get(value);
	parameters[2].buffer = st_char_get(value);

	// Flags
	parameters[3].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[3].buffer_length = sizeof(uint64_t);
	parameters[3].buffer = &flags;
	parameters[3].is_unsigned = true;

	if ((affected = stmt_exec_affected(stmts.upsert_user_config, parameters)) == -1) {
		log_pedantic("The user config upsert triggered an error. { user = %lu / key = %.*s }", usernum, st_length_int(key), st_char_get(key));
		return -1;
	}

	log_check(affected > 2);

	return (int_t)affected;
}

/**
 * @brief	Fetch a user's config collection from the database.
 * @param	collection	a pointer to the specified user config collection object (with usernum field set by the caller) to be populated from the database.
 * @return	true on success or false on failure.
 */
bool_t user_config_fetch(user_config_t *collection) {

	row_t *row;
	table_t *result;
	MYSQL_BIND parameters[1];
	user_config_entry_t *record;
	multi_t key = { .type = M_TYPE_STRINGER, .val.st = NULL };

	mm_wipe(parameters, sizeof(parameters));

	if (!collection || !collection->usernum) {
		log_pedantic("Invalid data passed to user config fetch.");
		return false;
	}

	// User Number
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &(collection->usernum);
	parameters[0].is_unsigned = true;

	if (!(result = stmt_get_result(stmts.select_user_config, parameters))) {
		log_pedantic("Unable to fetch the user config entries.");
		return false;
	}

	// Loop through each row and create a user config entry.
	while ((row = res_row_next(result))) {

		if (!(record = user_config_entry_alloc(PLACER(res_field_block(row, 0), res_field_length(row, 0)),
			PLACER(res_field_block(row, 1), res_field_length(row, 1)), res_field_uint64(row, 2))) ||
			!(key.val.st = record->key) || !inx_insert(collection->entries, key, record)) {
			log_info("The index refused to accept a user config entry. { user  = %lu }", res_field_uint64(row, 0));

			if (record) {
				user_config_entry_free(record);
			}

			res_table_free(result);
			return false;
		}

	}

	res_table_free(result);

	return true;
}
