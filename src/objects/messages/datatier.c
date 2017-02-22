
/**
 * @file /magma/objects/messages/datatier.c
 *
 * @brief	Message data functions.
 */

#include "magma.h"

/**
 * @brief	Populate a message folder with all of its child messages from the database.
 * @param	usernum		the numerical id of the user that owns the folder.
 * @param	folder		a pointer to the message folder object to be populated.
 * @return	true on success or false on failure.
 */
bool_t meta_data_fetch_folder_messages(uint64_t usernum, message_folder_t *folder) {

	row_t *row;
	table_t *result;
	message_t *record;
	MYSQL_BIND parameters[2];
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = 0 };

	mm_wipe(parameters, sizeof(parameters));

	if (!usernum || !folder || !folder->foldernum) {
		log_pedantic("Invalid data passed for message fetch.");
		return false;
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

	if (!(result = stmt_get_result(stmts.select_message_folder, parameters))) {
		log_pedantic("Unable to fetch the folder messages.");
		return false;
	}

	// Loop through each of the row and create a message record.
	while ((row = res_row_next(result))) {

		if (!(record = message_alloc(res_field_uint64(row, 0), res_field_uint64(row, 1), res_field_uint64(row, 2),
			res_field_uint64(row, 3), res_field_uint32(row, 4), PLACER(res_field_block(row, 5),
			res_field_length(row, 5)), res_field_uint32(row, 6))) || !(key.val.u64 = record->message.num) ||
			!inx_append(folder->records, key, record)) {

			log_error("The messages index refused to accept a metadata record. { usernum = %lu / message = %lu }",
				usernum, res_field_uint64(row, 0));

			if (record) message_free(record);
			res_table_free(result);
			return false;
		}

	}

	res_table_free(result);

	return true;
}

/**
 * @brief	Fetch the tags for a specified message from the database.
 * @note	The results of the operation will be stored in the specified meta message object's "tags" member.
 * @param	message		the meta message object for which the tags will be looked up.
 * @return	This function returns no value.
 */
void meta_data_fetch_message_tags(meta_message_t *message) {

	row_t *row;
	table_t *result;
	MYSQL_BIND parameters[1];

	mm_wipe(parameters, sizeof(parameters));

	// Messagenum
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &(message->messagenum);
	parameters[0].is_unsigned = true;

	if (!(result = stmt_get_result(stmts.select_message_tags, parameters))) {
		return;
	}

	else if (!res_row_count(result) || !(message->tags = ar_alloc(res_row_count(result)))) {
		res_table_free(result);
		return;
	}

	while ((row = res_row_next(result))) {
		ar_append(&(message->tags), ARRAY_TYPE_STRINGER, res_field_string(row, 0));
	}

	res_table_free(result);

	return;
}

/**
 * @brief	Fetch all of a user's stored messages from the database and attach them to the meta user object.
 * @note	Any of the user's existing messages will be destroyed first to allow for updates.
 * @param	user	the meta user object whose mail messages will be retrieved.
 * @return	true on success or false on failure.
 */
bool_t meta_data_fetch_messages(meta_user_t *user) {

	row_t *row;
	multi_t key;
	table_t *result;
	inx_cursor_t *cursor;
	MYSQL_BIND parameters[1];
	meta_message_t *message;

	// Sanity check.
	if (!user || !user->usernum) {
		log_pedantic("Invalid data passed for structure build.");
		return false;
	}

	// If we're updating an existing index, free the current collection of messages.
	if (user->messages) {
		inx_truncate(user->messages);
	}

	// Otherwise we need to allocate an index to hold the result, and abort if the allocation fails.
	else if (!(user->messages = inx_alloc(M_INX_LINKED, &meta_message_free))) {
		log_error("Could not create a linked list for the messages.");
		return false;
	}

	mm_wipe(parameters, sizeof(parameters));

	// Usernum.
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &(user->usernum);
	parameters[0].is_unsigned = true;

	if (!(result = stmt_get_result(stmts.select_messages, parameters))) {
		return false;
	}
	else if (!(row = res_row_next(result))) {
		res_table_free(result);
		return true;
	}

	while (row) {

		// We are using a fixed server name buffer of 33 bytes, so make sure the server name is 32 bytes or less.
		if (res_field_length(row, 2) > 32) {
			log_error("The server name found in the database was longer than 32 bytes. {usernum = %lu}", user->usernum);
			res_table_free(result);
			return false;
		}

		else if (!(message = mm_alloc(sizeof(meta_message_t)))) {
			log_pedantic("Could not allocate %zu bytes to hold the message meta information.", sizeof(meta_message_t));
			res_table_free(result);
			return false;
		}

		// Store the data.
		message->messagenum = res_field_uint64(row, 0);
		message->foldernum = res_field_uint64(row, 1);
		mm_copy(message->server, res_field_block(row, 2), res_field_length(row, 2));
		message->status = res_field_uint32(row, 3);
		message->size = res_field_uint32(row, 4);
		message->signum = res_field_uint64(row, 5);
		message->sigkey = res_field_uint64(row, 6);
		message->created = res_field_uint64(row, 7);

		if (!message->messagenum || !message->foldernum || !message->size || *(message->server) == '\0') {
			log_error("One of the critical message variables was zero or NULL. {usernum = %lu}", user->usernum);
			mm_free(message);
			res_table_free(result);
			return false;
		}

		key.type = M_TYPE_UINT64;
		key.val.u64 = message->messagenum;

		// Add this message to the structure.
		if (!inx_append(user->messages, key, message)) {
			log_error("Could not append the message to the linked list.");
			mm_free(message);
			res_table_free(result);
			return false;
		}

		row = res_row_next(result);
	}

	res_table_free(result);

	if ((cursor = inx_cursor_alloc(user->messages))) {

		while ((message = inx_cursor_value_next(cursor))) {

			if (message->status & MAIL_STATUS_TAGGED) {
				meta_data_fetch_message_tags(message);
			}

		}

		inx_cursor_free(cursor);
	}

	/// TODO: Do we still need this once the refactorization is complete?
	/*if (meta_check_message_encryption(user) < 0) {
		log_info("Storage encryption check failed on messages for user: %s", st_char_get(user->username));
	}*/

	return true;
}
