
/**
 * @file /magma/objects/contacts/datatier.c
 *
 * @brief	Functions for handling user contacts in the database.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Touch the last updated time stamp of a contact entry in the database.
 * @param	contactnum	the numerical id of the contact entry to have its time stamp updated.
 * @param	usernum		the numerical id of the user to whom the contact entry belongs.
 * @param	foldernum	the numerical id of the parent folder containing the contact entry.
 * @return	1 if the contact time stamp was updated successfully, 0 if the specified contact was not found, or -1 on general failure.
 */
int_t contact_update_stamp(uint64_t contactnum, uint64_t usernum, uint64_t foldernum) {

	int64_t affected;
	MYSQL_BIND parameters[3];

	mm_wipe(parameters, sizeof(parameters));

	// Contact Number
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &contactnum;
	parameters[0].is_unsigned = true;

	// User Number
	parameters[1].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[1].buffer_length = sizeof(uint64_t);
	parameters[1].buffer = &usernum;
	parameters[1].is_unsigned = true;

	// Folder
	parameters[2].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[2].buffer_length = sizeof(uint64_t);
	parameters[2].buffer = &foldernum;
	parameters[2].is_unsigned = true;

	// Since the updated column is always updated this function should only return 0 if the query doesn't match any rows, otherwise 1 to indicate success.
	if ((affected = stmt_exec_affected(stmts.update_contact_stamp, parameters)) == -1) {
		log_pedantic("The contact entry time stamp update triggered an error. { usernum = %lu / foldernum = %lu / contact = %lu }", usernum, foldernum, contactnum);
		return -1;
	}

	log_check(affected > 2);

	return (int_t)affected;
}

/**
 * @brief	Delete a user's contact entry and its associated details from the database.
 * @param	contactnum		the numerical id of the contact entry to be deleted.
 * @param	usernum			the numerical id of the user to whom the specified contact entry belongs.
 * @param	foldernum		the numerical id of the parent contact folder containing the contact entry.
 * @return	1 if the specified contact was deleted successfully, 0 if no matching contact was found in the database, or -1 on general failure.
 */
// QUESTION: How many affected rows should there be? Couldn't it be much higher if there are several details deleted?
int_t contact_delete(uint64_t contactnum, uint64_t usernum, uint64_t foldernum) {

	int64_t affected;
	MYSQL_BIND parameters[3];

	mm_wipe(parameters, sizeof(parameters));

	// Contact Number
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &contactnum;
	parameters[0].is_unsigned = true;

	// User Number
	parameters[1].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[1].buffer_length = sizeof(uint64_t);
	parameters[1].buffer = &usernum;
	parameters[1].is_unsigned = true;

	// Folder
	parameters[2].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[2].buffer_length = sizeof(uint64_t);
	parameters[2].buffer = &foldernum;
	parameters[2].is_unsigned = true;

	// Since the updated column is always updated this function should only return 0 if the query doesn't match any rows, otherwise 1 to indicate success.
	if ((affected = stmt_exec_affected(stmts.delete_contact, parameters)) == -1) {
		log_pedantic("The contact entry time stamp update triggered an error. { usernum = %lu / foldernum = %lu / contact = %lu }", usernum, foldernum, contactnum);
		return -1;
	}

	log_check(affected > 2);

	return (int_t)affected;
}

/**
 * @brief	Update a user contact entry in the database.
 * @param	contactnum		the numerical id of the contact entry to be modified.
 * @param	usernum			the numerical id of the user to whom the specified contact entry belongs.
 * @param	cur_folder		the numerical id of the parent contact containing the specified contact entry.
 * @param	target_folder	if not 0, sets the new parent contact folder to which the specified contact entry will belong.
 * @param	name			if not NULL, sets the new name of the specified contact entry.
 * @return	-1 on error, 0 if the specified contact entry was not found in the database, or 1 if the contact entry was successfully updated.
 */
int_t contact_update(uint64_t contactnum, uint64_t usernum, uint64_t cur_folder, uint64_t target_folder, stringer_t *name) {

	int64_t affected;
	MYSQL_BIND parameters[5];

	mm_wipe(parameters, sizeof(parameters));

	// Destination Folder
	if (target_folder) {
		parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
		parameters[0].buffer_length = sizeof(uint64_t);
		parameters[0].buffer = &target_folder;
		parameters[0].is_unsigned = true;
	}
	else {
		parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
		parameters[0].is_null = ISNULL(true);
	}

	// Name
	if (!st_empty(name)) {
		parameters[1].buffer_type = MYSQL_TYPE_STRING;
		parameters[1].buffer_length = st_length_get(name);
		parameters[1].buffer = st_char_get(name);
	}
	else {
		parameters[1].buffer_type = MYSQL_TYPE_LONGLONG;
		parameters[1].is_null = ISNULL(true);
	}

	// Contact Number
	parameters[2].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[2].buffer_length = sizeof(uint64_t);
	parameters[2].buffer = &contactnum;
	parameters[2].is_unsigned = true;

	// User Number
	parameters[3].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[3].buffer_length = sizeof(uint64_t);
	parameters[3].buffer = &usernum;
	parameters[3].is_unsigned = true;

	// Current Folder
	parameters[4].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[4].buffer_length = sizeof(uint64_t);
	parameters[4].buffer = &cur_folder;
	parameters[4].is_unsigned = true;

	// Since the updated column is always updated this function should only return 0 if the query doesn't match any rows, otherwise 1 to indicate success.
	if ((affected = stmt_exec_affected(stmts.update_contact, parameters)) == -1) {
		log_pedantic("The contact entry update triggered an error. { usernum = %lu / foldernum = %lu / contact = %lu }", usernum, cur_folder, contactnum);
		return -1;
	}

	log_check(affected > 2);

	return (int_t)affected;
}

/**
 * @brief	Insert a new contact entry into the database.
 * @param	usernum		the numerical id of the user to whom the contact entry belongs.
 * @param	foldernum	the numerical id of the parent folder to contain the contact entry.
 * @param	name		a pointer to a managed string containing the name of the new contact entry.
 * @return	-1 on failure, 0 if no item was inserted into the database, or the id of the newly inserted contact entry in the database on success.
 */
uint64_t contact_insert(uint64_t usernum, uint64_t foldernum, stringer_t *name) {

	MYSQL_BIND parameters[3];

	mm_wipe(parameters, sizeof(parameters));

	// Usernum
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &usernum;
	parameters[0].is_unsigned = true;

	// Folder
	parameters[1].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[1].buffer_length = sizeof(uint64_t);
	parameters[1].buffer = &foldernum;
	parameters[1].is_unsigned = true;

	// Name
	parameters[2].buffer_type = MYSQL_TYPE_STRING;
	parameters[2].buffer_length = st_length_get(name);
	parameters[2].buffer = st_char_get(name);

	return stmt_insert(stmts.insert_contact, parameters);
}

/**
 * @brief	Delete the specified contact detail of a contact entry from the database.
 * @param	contactnum	the numerical id of the contact entry to have the specified detail removed.
 * @param	key			a managed string containing the name of the contact detail to be removed from the entry.
 * @return	-1 on error, 0 if no matching detail was found in the database, or 1 if the delete operation was successful.
 */
int_t contact_detail_delete(uint64_t contactnum, stringer_t *key) {

	int64_t affected;
	MYSQL_BIND parameters[4];

	mm_wipe(parameters, sizeof(parameters));

	// Contact Number
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &contactnum;
	parameters[0].is_unsigned = true;

	// Key
	parameters[1].buffer_type = MYSQL_TYPE_STRING;
	parameters[1].buffer_length = st_length_get(key);
	parameters[1].buffer = st_char_get(key);

	if ((affected = stmt_exec_affected(stmts.delete_contact_details, parameters)) == -1) {
		log_pedantic("The contact detail deletion request triggered an error. { contact = %lu / key = %.*s }", contactnum, st_length_int(key), st_char_get(key));
		return -1;
	}

	log_check(affected > 2);

	return (int_t)affected;
}

/**
 * @brief	Update a specified contact detail in the database, or insert it if it does not exist.
 * @param	contactnum	the numerical id of the contact entry to be modified.
 * @param	key			a managed string containing the name of the contact detail to be updated.
 * @param	value		a managed string containing the new value of the specified contact detail.
 * @param	flags		a bitmask of flags to be associated with the specified contact entry detail.
 * @return	-1 on error, 0 if no update was necessary, 1 if a new contact detail was inserted into the database, or 2 if the
 * 			specified contact detail was updated successfully.
 */
int_t contact_detail_upsert(uint64_t contactnum, stringer_t *key, stringer_t *value, uint64_t flags) {

	int64_t affected;
	MYSQL_BIND parameters[4];

	mm_wipe(parameters, sizeof(parameters));

	// Contact Number
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &contactnum;
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

	if ((affected = stmt_exec_affected(stmts.upsert_contact_detail, parameters)) == -1) {
		log_pedantic("The contact detail upsert triggered an error. { contact = %lu / key = %.*s }", contactnum, st_length_int(key), st_char_get(key));
		return -1;
	}

	log_check(affected > 2);

	return (int_t)affected;
}

/**
 * @brief	Populate a contact entry with its details from the database.
 * @param	contact		a pointer to the contact entry object that will be updated.
 * @return	-1 on failure or 1 on success.
 */
int_t contact_details_fetch(contact_t *contact) {

	row_t *row;
	table_t *result;
	MYSQL_BIND parameters[1];
	contact_detail_t *record;
	multi_t key = { .type = M_TYPE_STRINGER, .val.st = NULL };

	mm_wipe(parameters, sizeof(parameters));

	if (!contact || !contact->contactnum) {
		log_pedantic("Invalid data passed to contact details fetch.");
		return -1;
	}

	// Contact Number
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &(contact->contactnum);
	parameters[0].is_unsigned = true;

	if (!(result = stmt_get_result(stmts.select_contact_details, parameters))) {
		log_pedantic("Unable to fetch the contact details.");
		return -1;
	}

	// Loop through each of the row and create a contact record. When were finished we'll fetch the contact details for each record found.
	while ((row = res_row_next(result))) {

		if (!(record = contact_detail_alloc(PLACER(res_field_block(row, 0), res_field_length(row, 0)),
			PLACER(res_field_block(row, 1), res_field_length(row, 1)), res_field_uint64(row, 2))) ||
			!(key.val.st = record->key) || !inx_insert(contact->details, key, record)) {
			log_info("The index refused to accept a contact record. { contact = %lu }", res_field_uint64(row, 0));

			if (record) {
				contact_detail_free(record);
			}

			res_table_free(result);
			return -1;
		}

	}

	res_table_free(result);
	return 1;
}

/**
 * @brief	Retrieve all of a user's contact entries in a specified contacts folder from the database.
 * @param	usernum		the numerical id of the user whose contacts will be retrieved.
 * @param	folder		a pointer to the contact folder which will have its contents listed.
 * @return	-1 on failure or 1 on success.
 */
int_t contacts_fetch(uint64_t usernum, contact_folder_t *folder) {

	row_t *row;
	table_t *result;
	contact_t *record;
	inx_cursor_t *cursor;
	MYSQL_BIND parameters[2];
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = 0 };

	mm_wipe(parameters, sizeof(parameters));

	if (!usernum || !folder || !folder->foldernum) {
		log_pedantic("Invalid data passed for contact fetch.");
		return -1;
	}

	// Usernum
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &(usernum);
	parameters[0].is_unsigned = true;

	// Folder Number
	parameters[1].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[1].buffer_length = sizeof(uint64_t);
	parameters[1].buffer = &(folder->foldernum);
	parameters[1].is_unsigned = true;

	if (!(result = stmt_get_result(stmts.select_contacts, parameters))) {
		log_pedantic("Unable to fetch the folder contacts.");
		return -1;
	}

	// Loop through each of the row and create a contact record. When were finished we'll fetch the contact details for each record found.
	while ((row = res_row_next(result))) {
		if (!(record = contact_alloc(res_field_uint64(row, 0), PLACER(res_field_block(row, 1), res_field_length(row, 1)))) ||
			!(key.val.u64 = record->contactnum) || !inx_insert(folder->records, key, record)) {
			log_info("The index refused to accept a contact record. { contact = %lu }", res_field_uint64(row, 0));
			if (record) contact_free(record);
			res_table_free(result);
			return -1;
		}
	}

	res_table_free(result);

	/// LOW: Should we bother with error checking?
	if ((cursor = inx_cursor_alloc(folder->records))) {
		while ((record = inx_cursor_value_next(cursor))) {
			contact_details_fetch(record);
		}

		inx_cursor_free(cursor);
	}

	return 1;
}
