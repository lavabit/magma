
/**
 * @file /magma/objects/users/datatier.c
 *
 * @brief	The database interface for the user objects.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Remove all user (non-system) flags from a collection of mail messages, and set the specified flags mask for them.
 * @note	The new mask can contain both user and system flags, but only user flags will be stripped from each message initially.
 * @param	messages	an inx holder containing the collection of messages to have their flags updated.
 * @param	usernum		the numerical of the user to whom the target messages belong, for validation purposes.
 * @param	foldernum	the numerical id of the parent folder containing the messages to be updated, for validation purposes.
 * @param	flags		a mask of all flags that are to be added to any matching messages in the collection.
 * @return	true on success or false on failure.
 */
bool_t meta_data_flags_replace(inx_t *messages, uint64_t usernum, uint64_t foldernum, uint32_t flags) {

	inx_cursor_t *cursor;
	meta_message_t *active;
	MYSQL_BIND parameters[6];
	uint32_t complete = MAIL_STATUS_USER_FLAGS;
	bool_t result = true;

	// Sanity check.
	if (!messages || !usernum || !foldernum) {
		return false;
	}

	// Iterate through and see if any messages have the recent flag set. Store the range.
	if ((cursor = inx_cursor_alloc(messages))) {

		while ((active = inx_cursor_value_next(cursor))) {

			if (active->foldernum == foldernum) {

				mm_wipe(parameters, sizeof(parameters));

				// Complete
				parameters[0].buffer_type = MYSQL_TYPE_LONG;
				parameters[0].buffer_length = sizeof(uint32_t);
				parameters[0].buffer = &complete;
				parameters[0].is_unsigned = true;

				// Complete
				parameters[1].buffer_type = MYSQL_TYPE_LONG;
				parameters[1].buffer_length = sizeof(uint32_t);
				parameters[1].buffer = &complete;
				parameters[1].is_unsigned = true;

				// Replacement Flags
				parameters[2].buffer_type = MYSQL_TYPE_LONG;
				parameters[2].buffer_length = sizeof(uint32_t);
				parameters[2].buffer = &flags;
				parameters[2].is_unsigned = true;

				// Usernum
				parameters[3].buffer_type = MYSQL_TYPE_LONGLONG;
				parameters[3].buffer_length = sizeof(uint64_t);
				parameters[3].buffer = &usernum;
				parameters[3].is_unsigned = true;

				// Foldernum
				parameters[4].buffer_type = MYSQL_TYPE_LONGLONG;
				parameters[4].buffer_length = sizeof(uint64_t);
				parameters[4].buffer = &foldernum;
				parameters[4].is_unsigned = true;

				// Message Numbers
				parameters[5].buffer_type = MYSQL_TYPE_LONGLONG;
				parameters[5].buffer_length = sizeof(uint64_t);
				parameters[5].buffer = &(active->messagenum);
				parameters[5].is_unsigned = true;

				if (!stmt_exec(stmts.update_message_flags_replace, parameters)) {
					log_pedantic("Message flag replace failed. { user = %lu / message = %lu / flags = %u }", usernum, active->messagenum, flags);
					result = false;
				}

			}
		}

		inx_cursor_free(cursor);
	}

	return result;
}

/**
 * @brief	Remove the specified flags mask from a collection of mail messages.
 * @param	messages	an inx holder containing the collection of messages to have their flags removed.
 * @param	usernum		the numerical id of the user to whom the target messages belong, for validation purposes.
 * @param	foldernum	the numerical id of the parent folder containing the messages to be updated, for validation purposes.
 * @param	flags		a mask of all flags that are to be stripped from any matching messages in the collection.
 * @return	true on success or false on failure.
 */
bool_t meta_data_flags_remove(inx_t *messages, uint64_t usernum, uint64_t foldernum, uint32_t flags) {

	inx_cursor_t *cursor;
	meta_message_t *active;
	MYSQL_BIND parameters[5];
	bool_t result = true;

	// Sanity check.
	if (!messages || !usernum || !foldernum) {
		return false;
	}

	// Iterate through and see if any messages have the recent flag set. Store the range.
	if ((cursor = inx_cursor_alloc(messages))) {

		while ((active = inx_cursor_value_next(cursor))) {
			if (active->foldernum == foldernum) {

				mm_wipe(parameters, sizeof(parameters));

				// Flag to Remove
				parameters[0].buffer_type = MYSQL_TYPE_LONG;
				parameters[0].buffer_length = sizeof(uint32_t);
				parameters[0].buffer = &flags;
				parameters[0].is_unsigned = true;

				// Flag to Remove
				parameters[1].buffer_type = MYSQL_TYPE_LONG;
				parameters[1].buffer_length = sizeof(uint32_t);
				parameters[1].buffer = &flags;
				parameters[1].is_unsigned = true;

				// Usernum
				parameters[2].buffer_type = MYSQL_TYPE_LONGLONG;
				parameters[2].buffer_length = sizeof(uint64_t);
				parameters[2].buffer = &usernum;
				parameters[2].is_unsigned = true;

				// Foldernum
				parameters[3].buffer_type = MYSQL_TYPE_LONGLONG;
				parameters[3].buffer_length = sizeof(uint64_t);
				parameters[3].buffer = &foldernum;
				parameters[3].is_unsigned = true;

				// Message Numbers
				parameters[4].buffer_type = MYSQL_TYPE_LONGLONG;
				parameters[4].buffer_length =  sizeof(uint64_t);
				parameters[4].buffer = &(active->messagenum);
				parameters[4].is_unsigned = true;

				if (!stmt_exec(stmts.update_message_flags_remove, parameters)) {
					log_pedantic("Message flag removal failed. { user = %lu / message = %lu / flags = %u }", usernum, active->messagenum, flags);
					result = false;
				}

			}
		}

		inx_cursor_free(cursor);
	}

	return result;
}

/**
 * @brief	Add the specified flags mask to a collection of mail messages.
 * @param	messages	an inx holder containing the collection of messages to have their flags updated.
 * @param	usernum		the numerical id of the user to whom the target messages belong, for validation purposes.
 * @param	foldernum	the numerical id of the parent folder containing the messages to be updated, for validation purposes.
 * @param	flags		a mask of all flags that are to be added to any matching messages in the collection.
 * @return	true on success or false on failure.
 */
bool_t meta_data_flags_add(inx_t *messages, uint64_t usernum, uint64_t foldernum, uint32_t flags) {

	inx_cursor_t *cursor;
	meta_message_t *active;
	MYSQL_BIND parameters[4];
	bool_t result = true;

	// Sanity check.
	if (!messages || !usernum || !foldernum) {
		return false;
	}

	// Iterate through and see if any messages have the recent flag set. Store the range.
	if ((cursor = inx_cursor_alloc(messages))) {

		while ((active = inx_cursor_value_next(cursor))) {
				mm_wipe(parameters, sizeof(parameters));

				// Flag to Add
				parameters[0].buffer_type = MYSQL_TYPE_LONG;
				parameters[0].buffer_length = sizeof(uint32_t);
				parameters[0].buffer = &flags;
				parameters[0].is_unsigned = true;

				// Usernum
				parameters[1].buffer_type = MYSQL_TYPE_LONGLONG;
				parameters[1].buffer_length = sizeof(uint64_t);
				parameters[1].buffer = &usernum;
				parameters[1].is_unsigned = true;

				// Foldernum
				parameters[2].buffer_type = MYSQL_TYPE_LONGLONG;
				parameters[2].buffer_length = sizeof(uint64_t);
				parameters[2].buffer = &foldernum;
				parameters[2].is_unsigned = true;

				// Message Numbers
				parameters[3].buffer_type = MYSQL_TYPE_LONGLONG;
				parameters[3].buffer_length = sizeof(uint64_t);
				parameters[3].buffer = &(active->messagenum);
				parameters[3].is_unsigned = true;

				if (!stmt_exec(stmts.update_message_flags_add, parameters)) {
					log_pedantic("Message flag addition failed. { user = %lu / message = %lu / flags = %u }", usernum, active->messagenum, flags);
					result = false;
				}

			}

		inx_cursor_free(cursor);
	}

	return result;
}

/**
 * @brief	Delete a message folder from the database.
 * @param	usernum		the numerical id of the user that owns the folder to be deleted.
 * @param	foldernum	the folder id of the message folder to be deleted.
 * @return	1 if the message folder was successfully, or <= 0 if there was an error.
 */
uint64_t meta_data_delete_folder(uint64_t usernum, uint64_t foldernum) {

	MYSQL_BIND parameters[3];
	uint_t type = M_FOLDER_MESSAGES;

	mm_wipe(parameters, sizeof(parameters));

	// Foldernum
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &foldernum;
	parameters[0].is_unsigned = true;

	// Usernum
	parameters[1].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[1].buffer_length = sizeof(uint64_t);
	parameters[1].buffer = &usernum;
	parameters[1].is_unsigned = true;

	// Folder Type
	parameters[2].buffer_type = MYSQL_TYPE_LONG;
	parameters[2].buffer_length = sizeof(uint_t);
	parameters[2].buffer = &(type);
	parameters[2].is_unsigned = true;

	// Should only update one row.
	return stmt_exec_affected(stmts.delete_folder, parameters);
}

/**
 * @brief	Update the record for a message folder in the database.
 * @param	usernum		the numerical id of the user that owns the specified folder.
 * @param	foldernum	the id of the folder to have its properties adjusted.
 * @param	name		a managed string containing the new name of the specified folder.
 * @param	parent		the id of the new parent folder to be set for the specified message folder.
 * @param	order		the value of the order for the specified folder.
 * @return	1 on success, or <= 0 on failure.
 */
uint64_t meta_data_update_folder_name(uint64_t usernum, uint64_t foldernum, stringer_t *name, uint64_t parent, uint32_t order) {

	MYSQL_BIND parameters[6];
	uint_t type = M_FOLDER_MESSAGES;

	mm_wipe(parameters, sizeof(parameters));

	// Foldername
	parameters[0].buffer_type = MYSQL_TYPE_STRING;
	parameters[0].buffer_length = st_length_get(name);
	parameters[0].buffer = st_char_get(name);

	// Parent
	parameters[1].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[1].buffer_length = sizeof(uint64_t);
	parameters[1].buffer = &parent;
	parameters[1].is_unsigned = true;

	// Order
	parameters[2].buffer_type = MYSQL_TYPE_LONG;
	parameters[2].buffer_length = sizeof(uint32_t);
	parameters[2].buffer = &order;
	parameters[2].is_unsigned = true;

	// Foldernum
	parameters[3].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[3].buffer_length = sizeof(uint64_t);
	parameters[3].buffer = &foldernum;
	parameters[3].is_unsigned = true;

	// Usernum
	parameters[4].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[4].buffer_length = sizeof(uint64_t);
	parameters[4].buffer = &usernum;
	parameters[4].is_unsigned = true;

	// Folder Type
	parameters[5].buffer_type = MYSQL_TYPE_LONG;
	parameters[5].buffer_length = sizeof(uint_t);
	parameters[5].buffer = &(type);
	parameters[5].is_unsigned = true;

	// Should only update one row.
	return stmt_exec_affected(stmts.update_folder, parameters);
}

/**
 * @brief	Insert a new mail folder into the database.
 * @param	usernum		the numerical id of the user to whom the new folder belongs.
 * @param	name		a managed string containing the name of the new mail folder.
 * @param	parent		the numerical id of the mail folder to be the parent of the new mail folder.
 * @param	order		the order number of this folder in its parent folder.
 * @return	0 on failure, or the numerical id of the newly inserted mail folder in the database on success.
 */
uint64_t meta_data_insert_folder(uint64_t usernum, stringer_t *name, uint64_t parent, uint32_t order) {

	MYSQL_BIND parameters[5];
	uint_t type = M_FOLDER_MESSAGES;

	mm_wipe(parameters, sizeof(parameters));

	// Usernum
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &usernum;
	parameters[0].is_unsigned = true;

	// Foldername
	parameters[1].buffer_type = MYSQL_TYPE_STRING;
	parameters[1].buffer_length = st_length_get(name);
	parameters[1].buffer = st_char_get(name);

	// Order
	parameters[2].buffer_type = MYSQL_TYPE_LONG;
	parameters[2].buffer_length = sizeof(uint32_t);
	parameters[2].buffer = &order;
	parameters[2].is_unsigned = true;

	// Parent.
	parameters[3].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[3].buffer_length = sizeof(uint64_t);
	parameters[3].buffer = &parent;
	parameters[3].is_unsigned = true;

	// Folder Type
	parameters[4].buffer_type = MYSQL_TYPE_LONG;
	parameters[4].buffer_length = sizeof(uint_t);
	parameters[4].buffer = &(type);
	parameters[4].is_unsigned = true;

	return stmt_insert(stmts.insert_folder, parameters);
}

/**
 * @brief	Update the per-user entry in the Log table for the specified protocol.
 * @note	This function will set the last session timestamp for the user, and increment the sessions counter in the database.
 * @param	user	the meta user object of the user making the logging request.
 * @param	prot	the protocol associated with the log request: META_PROT_POP, META_PROT_IMAP, or META_PROT_WEB.
 * @return	This function returns no value.
 */
void meta_data_update_log(meta_user_t *user, META_PROT prot) {

	MYSQL_BIND parameters[1];

	if (!user || !user->usernum) {
		return;
	}

	mm_wipe(parameters, sizeof(parameters));

	// Usernum
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &(user->usernum);
	parameters[0].is_unsigned = true;

	if (prot == META_PROT_POP && stmt_exec_affected(stmts.update_log_pop, parameters) != 1) {
		log_pedantic("Unable to update the POP statistics.");
	} else if (prot == META_PROT_IMAP && stmt_exec_affected(stmts.update_log_imap, parameters) != 1) {
		log_pedantic("Unable to update the IMAP statistics.");
	} else if (prot == META_PROT_WEB && stmt_exec_affected(stmts.update_log_web, parameters) != 1) {
		log_pedantic("Unable to update a users web statistics.");
	}

	return;
}

/**
 * @brief	Update a user's lock in the database.
 * @param	usernum		the numerical id of the user for whom the lock will be set.
 * @param	lock		the new value to which the specified user's lock will be set.
 * @return	This function returns no value.
 */
void meta_data_update_lock(uint64_t usernum, uint8_t lock) {

	MYSQL_BIND parameters[2];

	// We only want to allow the lock to be reset back to zero.
	if (!usernum || lock) {
		log_pedantic("Invalid lock update request. {lock = %i / usernum = %lu}", lock, usernum );
		return;
	}

	mm_wipe(parameters, sizeof(parameters));

	// Lock
	parameters[0].buffer_type = MYSQL_TYPE_TINY;
	parameters[0].buffer_length = sizeof(uint8_t);
	parameters[0].buffer = &lock;
	parameters[0].is_unsigned = true;

	// Usernum
	parameters[1].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[1].buffer_length = sizeof(uint64_t);
	parameters[1].buffer = &usernum;
	parameters[1].is_unsigned = true;

	if (stmt_exec_affected(stmts.update_user_lock, parameters) != 1) {
		log_pedantic("Unable to update the user lock. {usernum = %lu / lock = %hhu}", usernum, lock);
	}

	return;
}

/**
 * @brief	Insert a tag for a message into the database.
 * @param	message		the meta message object of the message to be tagged.
 * @param	tag			a managed string containing the name of the tag.
 * @return	0 on success or -1 on failure.
 */
int_t meta_data_insert_tag(meta_message_t *message, stringer_t *tag) {

	MYSQL_BIND parameters[2];

	mm_wipe(parameters, sizeof(parameters));

	// Messagenum
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &(message->messagenum);
	parameters[0].is_unsigned = true;

	// Tag
	parameters[1].buffer_type = MYSQL_TYPE_STRING;
	parameters[1].buffer_length = st_length_get(tag);
	parameters[1].buffer = st_char_get(tag);

	// If the message already has already been tagged the statement could indicate that no rows were updated. Since we don't consider that a true error condition we have to
	// check for (my_ulonglong)-1 to indicate an error condition which should be passed along to the user.
	if (stmt_exec_affected(stmts.insert_message_tag, parameters) == (my_ulonglong)-1) {
		return -1;
	}

	return 0;
}

/**
 * @brief	Remove all tags associated with a message in the database.
 * @param	message		a pointer to the meta message object of the message to have all of its tags stripped.
 * @return	0 on success or -1 on failure.
 */
int_t meta_data_truncate_tags(meta_message_t *message) {

	MYSQL_BIND parameters[1];

	mm_wipe(parameters, sizeof(parameters));

	// Messagenum
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &(message->messagenum);
	parameters[0].is_unsigned = true;

	// If the message doesn't have the tag in question, then no rows will be updated. Since we don't consider that a true error condition we have to
	// check for (my_ulonglong)-1 so we know when to indicate a server error to the user.
	if (stmt_exec_affected(stmts.delete_message_tags, parameters) == (my_ulonglong)-1) {
		return -1;
	}

	return 0;
}

/**
 * @brief	Remove a tag from a message in the database..
 * @param	message		a pointer to the meta message object of the message to have the tag stripped.
 * @param	tag			a managed string containing the name of the tag to be deleted.
 * @return	0 on success or -1 on failure.
 */
int_t meta_data_delete_tag(meta_message_t *message, stringer_t *tag) {

	MYSQL_BIND parameters[2];

	mm_wipe(parameters, sizeof(parameters));

	// Messagenum
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &(message->messagenum);
	parameters[0].is_unsigned = true;

	// Tag
	parameters[1].buffer_type = MYSQL_TYPE_STRING;
	parameters[1].buffer_length = st_length_get(tag);
	parameters[1].buffer = st_char_get(tag);

	// If the message doesn't have the tag in question, then no rows will be updated. Since we don't consider that a true error condition we have to
	// check for (my_ulonglong)-1 so we know when to indicate a server error to the user.
	if (stmt_exec_affected(stmts.delete_message_tag, parameters) == (my_ulonglong)-1) {
		return -1;
	}

	return 0;
}

/**
 * @brief	Fetch all the tags attached to messages of a specified user from the database.
 * @param	usernum		the numerical id of the user for whom the message tags will be fetched.
 * @return	NULL on failure, or an inx holder containing a list of all the user's messages' tags as managed strings on success.
 */
inx_t * meta_data_fetch_all_tags(uint64_t usernum) {

	row_t *row;
	inx_t *list;
	table_t *result;
	stringer_t *tagname;
	multi_t key;
	uint64_t i = 1;

	// Fetch the advertisements.
	if (!(result = stmt_get_result(stmts.select_all_message_tags, NULL))) {
		log_pedantic("Unable to retrieve user message tags.");
		return NULL;
	}

	// Allocate a linked list to hold the tags.
	if (!(list = inx_alloc(M_INX_LINKED, &st_free))) {
		log_pedantic("Could not allocate a list to hold user message tags.");
		res_table_free(result);
		return NULL;
	}

	key.type = M_TYPE_UINT64;

	while ((row = res_row_next(result))) {

			key.val.u64 = i++;

			if (!(tagname = res_field_string(row, 0))) {
				log_error("Could not allocate buffer to hold user message tag name.");
				res_table_free(result);
				inx_free(list);
				return NULL;
			}

			if (!inx_insert(list, key, tagname)) {
				log_error("Could not process results of user message tag request.");
				res_table_free(result);
				inx_free(list);
				st_free(tagname);
				return NULL;
			}

	}

	res_table_free(result);

	return list;
}

/**
 * @brief	Fetch  the tags for a specified message from the database.
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

	if (!res_row_count(result) || !(message->tags = ar_alloc(res_row_count(result)))) {
//	else if (!res_row_count(result) || !(message->tags = ar_alloc(res_row_count(result)))) {
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

	// If were updating, free the existing list of messages.
	inx_cleanup(user->messages);
	user->messages = NULL;

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

	if (!(user->messages = inx_alloc(M_INX_LINKED, &meta_message_free))) {
		log_error("Could not create a linked list for the messages.");
		res_table_free(result);
		return false;
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
		if (!inx_insert(user->messages, key, message)) {
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

	if (meta_check_message_encryption(user) < 0) {
		log_info("Storage encryption check failed on messages for user: %s", st_char_get(user->username));
	}

	return true;
}

/**
 * @brief	Adjust the encrypted status of a message, in accordance with the user's secure flag.
 * @param	user		a pointer to the meta user object owning the specified message.
 * @param	message		a pointer to the meta message that should have its on-disk data updated accordingly.
 * @param	oprivkey	if re-encryption is specified (user's secure flag is set and the message is already encrypted),
 * 						this is a pointer to a managed string that contains the user's original private key in binary form.
 * @return	true on success or false on failure.
 */
bool_t adjust_message_encryption(meta_user_t *user, meta_message_t *message, stringer_t *oprivkey) {

	inx_t *mholder;
	multi_t nkey;
	stringer_t *fcontents, *ftmpname;
	message_fheader_t *fheader, new_fheader;
	cryptex_t *enc_data = NULL;
	uint32_t transaction;
	size_t data_length, mdatalen;
	uchr_t *mdataptr;
	chr_t *msgpath, *write_data;
	int_t fd;
	bool_t do_encrypt = ((user->flags & META_USER_ENCRYPT_DATA) == META_USER_ENCRYPT_DATA);
	bool_t message_encrypted = ((message->status & MAIL_STATUS_ENCRYPTED) == MAIL_STATUS_ENCRYPTED);

	// Nothing to do if encryption is off and message is already decrypted
	if (!do_encrypt && !message_encrypted) {
		return true;
	}

	if (!(msgpath = mail_message_path(message->messagenum, message->server))) {
		log_pedantic("Unable to get file path of mail message.");
		return false;
	}

	if (!(fcontents = file_load(msgpath))) {
		log_pedantic("Unable to retrieve contents of old message data for encryption change operation.");
		ns_free(msgpath);
		return false;
	}

	if (st_length_get(fcontents) < sizeof(message_fheader_t)) {
		log_pedantic("Mail message was missing full file header: { %s }", msgpath);
		ns_free(msgpath);
		return false;
	}

	// Do some sanity checking on the message header
	fheader = (message_fheader_t *) st_data_get (fcontents);

	if ((fheader->magic1 != FMESSAGE_MAGIC_1) || (fheader->magic2 != FMESSAGE_MAGIC_2)) {
		log_pedantic("Mail message had incorrect file format: { %s }", msgpath);
		ns_free(msgpath);
		return false;
	}

	mdataptr = (uchr_t *) fheader;
	mdataptr += sizeof(message_fheader_t);
	mdatalen = st_length_get(fcontents) - sizeof(message_fheader_t);

	// Prepare the new message file header to be written
	new_fheader.magic1 = FMESSAGE_MAGIC_1;
	new_fheader.magic2 = FMESSAGE_MAGIC_2;
	new_fheader.reserved = 0;
	new_fheader.flags = fheader->flags;

	// We are left with 3 possible cases:

	// If encryption on and the message isn't encrypted, encrypt it.
	if (do_encrypt && !message_encrypted) {

		if (fheader->flags & FMESSAGE_OPT_ENCRYPTED) {
			log_pedantic("Message state mismatch: unencrypted in database but encrypted on disk.");
		}

		if (!(enc_data = ecies_encrypt(user->storage_pubkey, ECIES_PUBLIC_BINARY, mdataptr, mdatalen))) {
			log_pedantic("Unable to encrypt contents of user's message.");
			ns_free(msgpath);
			ns_free(fcontents);
			return false;
		}

		data_length = (size_t) cryptex_total_length(enc_data);

		if (!(write_data = ns_import(enc_data, data_length))) {
			log_pedantic("Unable to allocate buffer for user's encrypted message.");
			ns_free(msgpath);
			ns_free(fcontents);
			cryptex_free(enc_data);
			return false;
		}

		new_fheader.flags |= FMESSAGE_OPT_ENCRYPTED;
		cryptex_free(enc_data);
	// If encryption is off and the message is encrypted, decrypt it.
	} else if (!do_encrypt && message_encrypted) {

		if (!(fheader->flags & FMESSAGE_OPT_ENCRYPTED)) {
			log_pedantic("Message state mismatch: encrypted in database but unencrypted on disk.");
		}

		if (!(write_data = (chr_t *) ecies_decrypt(user->storage_privkey, ECIES_PRIVATE_BINARY, (cryptex_t *) mdataptr, &data_length))) {
			log_pedantic("Unable to decrypt contents of user's message.");
			ns_free(msgpath);
			ns_free(fcontents);
			return false;
		}

		new_fheader.flags &= ~FMESSAGE_OPT_ENCRYPTED;
	}

	ns_free(fcontents);

	if ((fd = get_temp_file_handle(NULL,&ftmpname)) < 0) {
		log_pedantic("Unable to get file descriptor for temp file.");
		ns_free(msgpath);
		ns_free(write_data);
		return false;
	}

	if ((write(fd, &new_fheader, sizeof(new_fheader)) != sizeof(new_fheader)) || (write(fd, write_data, data_length) != data_length)) {
		log_pedantic("Write of message data to temp file failed.");
		ns_free(msgpath);
		close(fd);
		unlink(st_char_get(ftmpname));
		return false;
	}

	fsync(fd);
	close(fd);
	ns_free(write_data);

	/* We have the transformed contents of a message in a temp file. Now make an atomic transaction of updating the file,
	 * along with updating the message's flags in the database.

	 * Construct a dummy holder for our message so we can update the flags in the database
	 * We don't need to free them since they were already allocated for us by the caller. */
	if (!(mholder = inx_alloc(M_INX_LINKED, NULL))) {
		log_pedantic("Could not allocate holder for encrypted user message.");
		ns_free(msgpath);
		unlink(st_char_get(ftmpname));
		return false;
	}

	nkey.type = M_TYPE_UINT64;
	nkey.val.u64 = message->messagenum;

	// Add this single message to the structure.
	if (!inx_insert(mholder, nkey, message)) {
		log_pedantic("Could not prepare encrypted message for update in database.");
		ns_free(msgpath);
		unlink(st_char_get(ftmpname));
		inx_free(mholder);
		return false;
	}

	if ((transaction = tran_start()) < 0) {
		log_pedantic("Unable to start transaction for user message encryption.");
		ns_free(msgpath);
		unlink(st_char_get(ftmpname));
		inx_free(mholder);
		return false;
	}

	// If we're encrypting, add the encrypted flag to the message
	if (do_encrypt && !message_encrypted && !(meta_data_flags_add(mholder, user->usernum, message->foldernum, MAIL_STATUS_ENCRYPTED))) {
		log_pedantic("Unable to set encryption flag for message in database.");
		ns_free(msgpath);
		unlink(st_char_get(ftmpname));
		inx_free(mholder);
		tran_rollback(transaction);
		return false;
	// or if we're decrypting, remove the encrypted flag from the message.
	} else if (!do_encrypt && message_encrypted && !(meta_data_flags_remove(mholder, user->usernum, message->foldernum, MAIL_STATUS_ENCRYPTED))) {
		log_pedantic("Unable to clear encryption flag for message in database.");
		ns_free(msgpath);
		unlink(st_char_get(ftmpname));
		inx_free(mholder);
		tran_rollback(transaction);
		return false;
	}

	inx_free(mholder);

	// Update the message flags in-memory.
	if (do_encrypt) {
		message->status |= MAIL_STATUS_ENCRYPTED;

	} else {
		message->status &= ~MAIL_STATUS_ENCRYPTED;
	}

	if (rename(st_char_get(ftmpname), msgpath) < 0) {
		//if (rename(msgpath,st_char_get(ftmpname)) <  0) {
		log_pedantic("Rename of encrypted temp file failed.");
		ns_free(msgpath);
		unlink(st_char_get(ftmpname));
		tran_rollback(transaction);
		return false;
	}

	// QUESTION: What do we do if rename() succeeds but the transaction fails?
	if (tran_commit(transaction)) {
		log_pedantic("Transaction commit for file encryption failed.");
	}

	ns_free(msgpath);
	unlink(st_char_get(ftmpname));

	return true;
}

/**
 * @brief	Encrypt all of a user's messages that aren't tagged as encrypted already.
 * @param	user	the meta user object to have its messages processed.
 * @return	This function returns no value.
 */
void encrypt_user_messages(meta_user_t *user) {

	inx_cursor_t *cursor;
	meta_message_t *message;
	int_t pending = 0;

	if ((cursor = inx_cursor_alloc(user->messages))) {

		// Cycle through all the messages and decrypt only the ones that need to be decrypted
		while ((message = inx_cursor_value_next(cursor))) {

			if (message->status & MAIL_STATUS_ENCRYPTED)
				continue;

			if (!adjust_message_encryption(user,message,NULL)) {
				log_pedantic("Message encryption operation failed.");
				pending++;
			}

		}

		inx_cursor_free(cursor);
	}
	else {
		log_pedantic("Unable to allocate cursor for batch encryption routine.");
	}

	if (pending) {
		log_info("Failed to encrypt entire queued message batch. Submitting for sleep + reprocessing.");
		// QUESTION: Is this how we do it?
		enqueue(encrypt_user_messages, user);
	} else {
		log_info("Message encryption batch successfully completed for user: %s", st_char_get(user->username));
		meta_user_ref_dec(user, META_PROT_GENERIC);
	}

	return;
}

/**
 * @brief	Decrypt all of a user's messages that aren't tagged as unencrypted already.
 * @param	user	the meta user object to have its messages processed.
 * @return	This function returns no value.
 */
void decrypt_user_messages(meta_user_t *user) {

	inx_cursor_t *cursor;
	meta_message_t *message;
	int_t pending = 0;

	if ((cursor = inx_cursor_alloc(user->messages))) {

		// Cycle through all the messages and decrypt only the ones that need to be decrypted
		while ((message = inx_cursor_value_next(cursor))) {

			if (!(message->status & MAIL_STATUS_ENCRYPTED))
				continue;

			if (!adjust_message_encryption(user,message,NULL)) {
				log_pedantic("Message decryption operation failed.");
				pending++;
			}

		}

		inx_cursor_free(cursor);
	}
	else {
		log_pedantic("Unable to allocate cursor for batch decryption routine.");
	}

	if (pending) {
		log_info("Failed to decrypt entire queued message batch. Submitting for sleep + reprocessing.");
		// QUESTION: Is this how we do it?
		enqueue(encrypt_user_messages, user);
	} else {
		log_info("Message decryption batch successfully completed for user: %s", st_char_get(user->username));
		meta_user_ref_dec(user, META_PROT_GENERIC);
	}

	return;
}

/**
 * @brief	Make sure that a user's messages' on-disk encryption statuses match the user's security settings.
 * @note	If the user's secure flag is on, then all messages should be encrypted. Any unencrypted messages need to be encrypted
 * 			in place. If the secure flag has been disabled and there are still encrypted messages on disk, the messages
 * 			need to be reverted to plaintext form.
 * @param	user	the meta user object that owns the specified messages.
 * @return	-1 on failure, 0 if there were no messages to be sync'ed, or 1 if additional encryption processing is required.
 */
int_t meta_check_message_encryption(meta_user_t *user) {

	inx_cursor_t *cursor;
	meta_message_t *message;
	int_t result = 0;
	bool_t do_encrypt = ((user->flags & META_USER_ENCRYPT_DATA) == META_USER_ENCRYPT_DATA);

	if (!user)
		return -1;

	if ((do_encrypt && !user->storage_privkey) || (!do_encrypt && !user->storage_pubkey))
		return -1;

	if ((cursor = inx_cursor_alloc(user->messages))) {

		while ((message = inx_cursor_value_next(cursor))) {
			// Only send this off if there are any messages that need to be changed.

			if (do_encrypt != ((message->status & MAIL_STATUS_ENCRYPTED) == MAIL_STATUS_ENCRYPTED)) {
				result = 1;
				break;
			}

		}

		inx_cursor_free(cursor);
	}
	else {
		return -1;
	}

	if (result) {
		meta_user_ref_add(user, META_PROT_GENERIC);

		if (do_encrypt)
			enqueue(encrypt_user_messages, user);
		else
			enqueue(decrypt_user_messages, user);
	}

	return result;
}

/**
 * @brief	Retrieve all of a user's message folders from the database.
 * @note	If the user already had a working set of message folders they will be deleted first.
 * @param	user	a pointer to the meta user object of the user making the request, which will be updated on success.
 * @return	-1 on failure or 1 on success.
 */
bool_t meta_data_fetch_folders(meta_user_t *user) {

	row_t *row;
	multi_t key;
	table_t *result;
	meta_folder_t *folder;
	MYSQL_BIND parameters[2];
	uint_t type = M_FOLDER_MESSAGES;

	// Sanity check.
	if (!user || !user->usernum) {
		log_pedantic("Invalid data passed for structure build.");
		return false;
	}

	mm_wipe(parameters, sizeof(parameters));

	// Usernum
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &(user->usernum);
	parameters[0].is_unsigned = true;

	// Folder Type
	parameters[1].buffer_type = MYSQL_TYPE_LONG;
	parameters[1].buffer_length = sizeof(uint_t);
	parameters[1].buffer = &(type);
	parameters[1].is_unsigned = true;

	if (!(result = stmt_get_result(stmts.select_folders, parameters))) {
		return false;
	}
	else if (!(row = res_row_next(result))) {
		res_table_free(result);
		return true;
	}

	// If were updating, free the existing list of folders.
	inx_cleanup(user->folders);

	if (!(user->folders = inx_alloc(M_INX_LINKED, &mm_free))) {
		log_error("Could not create a linked list for the folders.");
		res_table_free(result);
		return false;
	}

	while (row) {

		// We are using a fixed folder name buffer of 128 bytes, so make sure the folder name is 127 bytes or less.
		// We limit names to only 16 characters, but with modified UTF-7 escaping we can end up with longer strings.
		if (res_field_length(row, 3) > 127) {
			log_error("The folder name found in the database was longer than 127 bytes. {usernum = %lu}", user->usernum);
			res_table_free(result);
			return false;
		}

		else if ((folder = mm_alloc(sizeof(meta_folder_t))) == NULL) {
			log_pedantic("Could not allocate %zu bytes to hold the message meta information.", sizeof(meta_folder_t));
			res_table_free(result);
			return false;
		}

		// Store the data.
		folder->foldernum = res_field_uint64(row, 0);
		folder->parent = res_field_uint64(row, 1);
		folder->order = res_field_uint32(row, 2);
		mm_copy(folder->name, res_field_block(row, 3), res_field_length(row, 3));

		if (!folder->foldernum || *(folder->name) == '\0') {
			log_error("One of the critical message variables was zero or NULL. {usernum = %lu}", user->usernum);
			mm_free(folder);
			res_table_free(result);
			return false;
		}

		key.type = M_TYPE_UINT64;
		key.val.u64 = folder->foldernum;

		// Add this message to the structure.
		if (!inx_insert(user->folders, key, folder)) {
			log_error("Could not append the folder to the linked list.");
			mm_free(folder);
			res_table_free(result);
			return false;
		}

		row = res_row_next(result);
	}

	res_table_free(result);

	return true;
}

/**
 * @brief	Populate a meta user object with user information stored in the database.
 * @note	Fields fetched from the database include: flags, hashed password, lock & ssl status, and quota information.
 * @param	user	a pointer to the partially populated meta user object to be be updated, with the usernum field already supplied.
 * @return	true on success or false on failure.
 */
bool_t meta_data_fetch_user(meta_user_t *user) {

	row_t *row;
	table_t *result;
	MYSQL_BIND parameters[1];

	mm_wipe(parameters, sizeof(parameters));

	// Get the user information.
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &(user->usernum);
	parameters[0].is_unsigned = true;

	if (!(result = stmt_get_result(stmts.select_user_record, parameters))) {
		return false;
	}
	else if (!(row = res_row_next(result))) {
		res_table_free(result);
		return false;
	}

	// Reset the flags.
	user->flags = 0;

	// Update the secure flag
	if (res_field_int8(row, 1) == 1) {
		user->flags = (user->flags | META_USER_ENCRYPT_DATA);
	}

	// Update the Password Hash
	st_cleanup(user->passhash);
	user->passhash = res_field_string(row, 0);

	// Update the lock status
	user->lock_status = res_field_int8(row, 2);

	// Update the SSL flag.
	if (res_field_int8(row, 3) == 1) {
		user->flags = (user->flags | META_USER_SSL);
	} else {
		user->flags = (user->flags | META_USER_SSL) ^ META_USER_SSL;
	}

	// Over Quota
	if (res_field_int8(row, 4) == 1) {
		user->flags = (user->flags | META_USER_OVERQUOTA);
	} else {
		user->flags = (user->flags | META_USER_OVERQUOTA) ^ META_USER_OVERQUOTA;
	}

	res_table_free(result);

	// Inactive lock
	/// LOW: This user fetch function was intended only to refresh existing user configurations/preferences. In theory
	/// an inactivity lock would have been cleared when the session was created so we shouldn't need to even check that value at this stage.
	if (user->lock_status == 2) {
		meta_data_update_lock(user->usernum, 0);
		user->lock_status = 0;
	}

	return true;
}

/**
 * @brief	Build a meta user object by username, hashed password, and hashed key storage password.
 * @param	user		a user meta object with the username field populated.
 * @param	cred		Credential object containing user password, passkey and authentication type.
 * @return	-1 for unexpected program/system error, 0 for password auth failure, or 1 on success.
 */
int_t meta_data_user_build(meta_user_t *user, credential_t *cred) {

	row_t *row;
	table_t *result;
	MYSQL_BIND parameters[2];
	MYSQL_STMT **auth_stmt;

	// Sanity check.
	if (!user || st_empty(user->username) || !cred->auth.password || st_empty(cred->auth.password)) {
		log_pedantic("Invalid data passed for structure build.");
		return -1;
	}

	switch(cred->authentication) {

	case LEGACY:
		auth_stmt = stmts.select_user;
		break;
	case STACIE:
		auth_stmt = stmts.select_user_auth;
		break;
	default:
		log_error("Invalid authentication type.");
		break;
	}

	// Clear it out, just in case we don't have a valid password hash and then overwrite the buffer with a new passhash.
	if (user->passhash) {
		st_free(user->passhash);
		user->passhash = NULL;
	}

	mm_wipe(parameters, sizeof(parameters));

	// Get the user information.
	parameters[0].buffer_type = MYSQL_TYPE_STRING;
	parameters[0].buffer_length = st_length_get(user->username);
	parameters[0].buffer = st_char_get(user->username);

	parameters[1].buffer_type = MYSQL_TYPE_STRING;
	parameters[1].buffer_length = st_length_get(cred->auth.password);
	parameters[1].buffer = st_char_get(cred->auth.password);

	if (!(result = stmt_get_result(auth_stmt, parameters))) {
		return -1;
	} else if (!(row = res_row_next(result))) {
		res_table_free(result);
		return 0;
	}

	// Reset the flags.
	user->flags = 0;

	// Update the secure flag.
	if (res_field_int8(row, 0) == 1) {
		user->flags = (user->flags | META_USER_ENCRYPT_DATA);
	}

	// User number.
	user->usernum = res_field_uint64(row, 2);

	// Store the lock status.
	user->lock_status = res_field_int8(row, 1);

	// SSL
	if (res_field_int8(row, 3) == 1) {
		user->flags = (user->flags | META_USER_SSL);
	} else {
		user->flags = (user->flags | META_USER_SSL) ^ META_USER_SSL;
	}

	// Over Quota
	if (res_field_int8(row, 4) == 1) {
		user->flags = (user->flags | META_USER_OVERQUOTA);
	} else {
		user->flags = (user->flags | META_USER_OVERQUOTA) ^ META_USER_OVERQUOTA;
	}

	res_table_free(result);

	if (!user->usernum) {
		log_pedantic("Invalid user number found. {username = %.*s}", st_length_int(user->username), st_char_get(user->username));
		return -1;
	}

	// Inactive lock
	if (user->lock_status == 2) {
		meta_data_update_lock(user->usernum, 0);
		user->lock_status = 0;
	}

	if ((user->passhash = st_dupe(cred->auth.password)) == NULL) {
		log_pedantic("Unable to copy password hash.");
		return -1;
	}

	if (user->storage_privkey) {
		st_free(user->storage_privkey);
		user->storage_privkey = NULL;
	}

	if (user->storage_pubkey) {
		st_free(user->storage_pubkey);
		user->storage_pubkey = NULL;
	}

	// Finally, we make sure that the user has generated an EC pair for on-drive mail storage encryption - if they need one.
	// But they still might need a storage key if the the secure flag is off and there is a batch of encrypted mail messages that needs to be decrypted.
	if ((user->flags & META_USER_ENCRYPT_DATA) &&
		(meta_data_user_build_storage_keys (user->usernum, cred->auth.key, &(user->storage_privkey), &(user->storage_pubkey), false, false, 0) < 0)) {
		log_pedantic("A user with the secure storage feature enabled does not have storage keys and the creation attempt failed.");
		return -1;
	} else if (!(user->flags & META_USER_ENCRYPT_DATA) &&
		(meta_data_user_build_storage_keys (user->usernum, cred->auth.key, &(user->storage_privkey), &(user->storage_pubkey), true, false, 0) < 0)) {
		//log_pedantic("A user with the secure storage feature disabled does not have storage keys in the database.");
	}

	return 1;
}

/**
 * @brief	Retrieve the on-disk mail storage key pair associated with the user, or create it if it doesn't exist.
 * @param	usernum			the user id of the target account.
 * @param	passkey			the single-round hashed password for storage key management, which can be NULL if dont_create is specified.
 * @param	priv_out		a pointer to a managed string to receive a copy of the ECIES private key if not NULL.
 * @param	pub_out			a pointer to a managed string to receive a copy of the ECIES public key if not NULL.
 * @param	dont_create		if true, ensures that storage keys won't be created if they don't already exist.
 * @param	do_trans		specifies whether or not a transaction id will be supplied for database operations.
 * @param	tid				if do_trans is set, the mysql transaction id to be used for all database operations.
 * @return	-1 on general failure, 0 if keys were successfully retrieved, or 1 if keys were generated.
 */
int_t meta_data_user_build_storage_keys(uint64_t usernum, stringer_t *passkey, stringer_t **priv_out, stringer_t **pub_out, bool dont_create, bool_t do_trans, uint32_t tid) {

	row_t *row;
	table_t *table;
	MYSQL_BIND parameters[1];
	scramble_t *scrkey;
	stringer_t *spub_b64 = NULL, *spriv_b64 = NULL, *spub_bin = NULL, *spriv_bin = NULL, *tmpkey;
	int_t result = 1;

	// Attempt to load the storage keys from the database
	mm_wipe(parameters, sizeof(parameters));

	// Get the user information.
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &usernum;
	parameters[0].is_unsigned = true;

	if (do_trans) {
		table = stmt_get_result_conn(stmts.select_user_storage_keys, parameters, tid);
	} else {
		table = stmt_get_result(stmts.select_user_storage_keys, parameters);
	}

	//if (!(table = stmt_get_result(stmts.select_user_storage_keys, parameters))) {
	if (!table) {
		log_pedantic("Error executing storage key retrieval statement.");
		return -1;
	}
	else if (!(row = res_row_next(table)) && dont_create) {
		//log_pedantic("Error retrieving storage keys from database; dont_create was set.");
		res_table_free(table);
		return -1;
	}

	// If we didn't receive a row, but we're OK with creating a new keypair, skip this part.
	if (row) {
		// Our keypair comes base-64 encoded, but the private key is also encrypted with our passkey
		if (res_field_length(row, 0)) {
			spub_b64 = st_import(res_field_block(row, 0), res_field_length(row, 0));
		}

		if (res_field_length(row, 1)) {
			spriv_b64 = st_import(res_field_block(row, 1), res_field_length(row, 1));
		}

	}

	res_table_free(table);

	// Did we get the pub and priv key from the database? If not, generate them and store them.
	if (!dont_create && (st_empty(spub_b64) || st_empty(spriv_b64))) {

		EC_KEY *new_ecies_key;
		unsigned char *pubkeybuf;
		char *privkeybuf;
		size_t priv_len, pub_len;

		log_info("No storage keys found for user; generating a new set of keys.");

		// Create the ECIES key pair first and extract the pub and priv keys, wrapping them in managed strings.
		if (!(new_ecies_key = ecies_key_create())) {
			log_info("Unable to create new ECIES key.");
			return -1;
		} else if (!(pubkeybuf = ecies_key_public_bin(new_ecies_key, &pub_len)) || pub_len <= 0) {
			log_info("Unable to derive ECIES public key.");
			ecies_key_free(new_ecies_key);
			return -1;
		} else if (!(spub_bin = st_import(pubkeybuf,pub_len))) {
			log_info("Unable to import ECIES public key.");
			ecies_key_free(new_ecies_key);
			mm_free(pubkeybuf);
			return -1;
		}

		mm_free(pubkeybuf);

		if (!(privkeybuf = ecies_key_private_bin (new_ecies_key, &priv_len)) || priv_len <= 0) {
			log_info("Unable to derive ECIES private key.");
			ecies_key_free(new_ecies_key);
			st_free(spub_bin);
			return -1;
		}

		// Allocate and store a copy of the private key securely
		if (!(spriv_bin = st_alloc_opts(MANAGED_T | CONTIGUOUS | SECURE, priv_len))) {
			log_pedantic("Unable to wrap ECIES private key.");
			ecies_key_free(new_ecies_key);
			mm_sec_free(privkeybuf);
		} else if (!st_copy_in(spriv_bin, privkeybuf, priv_len)) {
			log_pedantic("Unable to initialize wrapped ECIES private key.");
			ecies_key_free(new_ecies_key);
			mm_sec_free(privkeybuf);
			st_free(spriv_bin);
		}

		ecies_key_free(new_ecies_key);
		mm_sec_free(privkeybuf);

		// Store the ECIES keys we just generated into the database
		if (meta_data_user_save_storage_keys(usernum, passkey, spub_bin, spriv_bin, do_trans, tid) < 0) {
			log_pedantic("Unable to save user storage keys.");
			result = -1;
		}

	// Otherwise if dont_create wasn't set, we must have found them.
	} else if (!dont_create) {
		result = 0;
	}
	// But if dont_create is set and both of the keys are empty, there is a problem.
	else if (dont_create && st_empty(spub_b64) && st_empty(spriv_b64)) {
		st_cleanup(spub_b64);
		st_cleanup(spriv_b64);
		result = -1;
	} else {
		result = 0;
	}

	if (result >= 0) {
		// If the keys were in the database (result=0) we only have the base64 versions. If not, we have both.
		if (!result) {

			// Make sure we have something to write to whatever values are expected.
			if (pub_out && !spub_b64) {
				st_cleanup(spriv_b64);
				return -1;
			} else if (pub_out && !(spub_bin = base64_decode_mod(spub_b64, NULL))) {
				log_pedantic("Unable to base64 decode public storage key in database.");
				st_cleanup(spriv_b64);
				st_free(spub_b64);
				return -1;
			}

			if (priv_out && !spriv_b64) {
				st_cleanup(spub_b64);
				return -1;
			} else if (priv_out) {

				if (!(tmpkey = base64_decode_opts(spriv_b64, MANAGED_T | CONTIGUOUS | SECURE, true))) {
					log_pedantic("Unable to base64 decode private storage key in database.");
					st_free(spriv_b64);
					st_cleanup(spub_b64);
					return -1;
				}

				scrkey = (scramble_t *) st_data_get (tmpkey);

				// The private key still needs to be decrypted.
				spriv_bin = scramble_decrypt(passkey, scrkey);
				st_free(tmpkey);

				if (!spriv_bin) {
					log_pedantic("Unable to decrypt private storage key.");
					st_free(spriv_b64);
					st_free(spub_b64);
					return -1;
				}

			}

		}

		// Save these values if we're supposed to
		if (priv_out) {
			*priv_out = spriv_bin;
		}
		else if (spriv_bin) {
			st_free(spriv_bin);
		}

		if (pub_out) {
			*pub_out = spub_bin;
		}
		else if (spub_bin) {
			st_free(spub_bin);
		}

		st_cleanup(spriv_b64);
		st_cleanup(spub_b64);
	}

	return result;
}

/**
 * @brief	Persist the user's storage keys into the mysql database.
 * @param	usernum		the user id of the target account.
 * @param	passkey		the passkey that will be used to encrypt the user's prvate key symmetrically in the database.
 * @param	pubkey		the user's storage public key as a base64 encoded string.
 * @param	privkey		the user's encrypted storage private key as a base64 encoded string.
 * @param	do_trans		specifies whether or not a transaction id will be supplied for database operations.
 * @param	tid				if do_trans is set, the mysql transaction id to be used for all database operations.
 * @return	-1 on failure or 0 on success.
 */
int_t meta_data_user_save_storage_keys(uint64_t usernum, stringer_t *passkey, stringer_t *pubkey, stringer_t *privkey, bool_t do_trans, uint32_t tid) {

	MYSQL_BIND parameters[5];
	scramble_t *sprivkey;
	stringer_t *pkbuf, *pubkey_b64, *privkey_b64;
	uint64_t tmp;
	int_t result = 0;

	if (!(pubkey_b64 = base64_encode_mod(pubkey,NULL))) {
		log_info("Unable to base64 encode public key for storage.");
		return -1;
	}

	if (!(sprivkey = scramble_encrypt(passkey, privkey))) {
		log_info("Unable to scramble private storage key for database.");
		st_free(pubkey_b64);
		return -1;
	}

	if (!(pkbuf = st_import(sprivkey, scramble_total_length(sprivkey)))) {
		log_info("Unable to import private storage key.");
		st_free(pubkey_b64);
		scramble_free(sprivkey);
		return -1;
	}

	scramble_free(sprivkey);

	//if (!(privkey_b64 = base64_encode_opts(pkbuf, MANAGED_T | CONTIGUOUS | SECURE, true))) {
	if (!(privkey_b64 = base64_encode_mod(pkbuf, NULL))) {
		log_info("Unable to base64 encode private key for storage.");
		st_free(pubkey_b64);
		st_free(pkbuf);
		return -1;
	}

	st_free(pkbuf);

	mm_wipe(parameters, sizeof(parameters));

	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &usernum;
	parameters[0].is_unsigned = true;
	parameters[1].buffer_type = MYSQL_TYPE_STRING;
	parameters[1].buffer_length = st_length_get(pubkey_b64);
	parameters[1].buffer = st_char_get(pubkey_b64);
	parameters[2].buffer_type = MYSQL_TYPE_STRING;
	parameters[2].buffer_length = st_length_get(privkey_b64);
	parameters[2].buffer = st_char_get(privkey_b64);
	mm_copy(&parameters[3],&parameters[1],sizeof(parameters[1]));
	mm_copy(&parameters[4],&parameters[2],sizeof(parameters[2]));

	if (do_trans) {
		tmp = stmt_exec_affected_conn(stmts.update_user_storage_keys, parameters, tid);
	} else {
		tmp = stmt_exec_affected(stmts.update_user_storage_keys, parameters);
	}
	// QUESTION: Make sure this is the right comparison
	if (tmp >= 1) {
	} else {
		log_info("Unable to update user's storage keys [%u].", (unsigned int) tmp);
		result = -1;
	}

	st_free(pubkey_b64);
	st_free(privkey_b64);

	return result;
}

/**
 * @brief	Mark a user alert message as acknowledged in the database.
 * @note	If the table is not updated immediately, another check is made to see if the alert is still pending. If so, false is returned.
 * @param	alertnum	the numerical id of the alert message to be acknowledged.
 * @param	usernum		the numerical id of the user to whom the alert message belongs.
 * @param	transaction	the mysql transaction id of the acknowledgment operation, in cases batch changes need to be rolled back.
 * @return	true if the alert was acknowledged successfully, or false on failure.
 */
bool_t meta_data_acknowledge_alert(uint64_t alertnum, uint64_t usernum, uint32_t transaction) {

	row_t *row;
	table_t *result;
	bool_t pending = false;
	MYSQL_BIND parameters[2];

	// Sanity check.
	if (!usernum || !alertnum) {
		log_pedantic("Invalid data passed for structure build.");
		return false;
	}

	mm_wipe(parameters, sizeof(parameters));

	// Alertnum.
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &alertnum;
	parameters[0].is_unsigned = true;

	// Usernum.
	parameters[1].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[1].buffer_length = sizeof(uint64_t);
	parameters[1].buffer = &usernum;
	parameters[1].is_unsigned = true;

	if (stmt_exec_affected_conn(stmts.update_alerts_acknowledge, parameters, transaction) == 1) {
		return true;
	}

	// If none of the rows were updated we check to make sure the alert is still pending before returning false.
	mm_wipe(parameters, sizeof(parameters));

	// Usernum.
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &usernum;
	parameters[0].is_unsigned = true;

	if (!(result = stmt_get_result_conn(stmts.select_alerts, parameters, transaction))) {
		return false;
	}

	// If the alert is returned then its still pending and we should return false to indicate the error condition.
	while (!pending && (row = res_row_next(result))) {

		if (alertnum == res_field_uint64(row, 0)) {
			pending = true;
		}

	}

	res_table_free(result);

	return pending ? false : true;
}

/**
 * @brief	Get all unacknowledged alert messages for a user.
 * @param	usernum		the numerical id of the user for whom the alert messages will be fetched.
 * @return	NULL on failure, or a pointer to an inx holder containing all of the user's alert messages on success.
 */
inx_t * meta_data_fetch_alerts(uint64_t usernum) {

	row_t *row;
	table_t *result;
	inx_t *alerts = NULL;
	meta_alert_t *record;
	MYSQL_BIND parameters[1];
	multi_t key = { .type = M_TYPE_UINT64 };

	// Sanity check.
	if (!usernum) {
		log_pedantic("Cannot fetch alerts for null user.");
		return NULL;
	}

	mm_wipe(parameters, sizeof(parameters));

	// Usernum.
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &usernum;
	parameters[0].is_unsigned = true;

	if (!(result = stmt_get_result(stmts.select_alerts, parameters))) {
		return NULL;
	}

	if (!(alerts = inx_alloc(M_INX_LINKED, &mm_free))) {
		log_error("Could not create a linked list for the alert messages.");
		res_table_free(result);
		return NULL;
	}

	while ((row = res_row_next(result))) {

		// Pass the alert data to the allocator and hope for the best.
		if ((record = alert_alloc(res_field_uint64(row, 0), PLACER(res_field_block(row, 1), res_field_length(row, 1)),
			PLACER(res_field_block(row, 2), res_field_length(row, 2)), res_field_uint64(row, 3))) &&
			(!(key.val.u64 = record->alertnum) || !inx_insert(alerts, key, record))) {
			log_info("Was not able to read user alert message. { no = %lu }", record->alertnum);
			res_table_free(result);
			mm_free(record);
			inx_free(alerts);
			return NULL;
		}

	}

	res_table_free(result);
	return alerts;
}

/**
 * @brief	Get all the mailbox aliases for a specified user.
 * @param	user	a pointer to the partially populated meta user object to be queried, with its usernum field set.
 * @return	true on success or false on failure.
 */
bool_t meta_data_fetch_mailbox_aliases(meta_user_t *user) {

	row_t *row;
	table_t *result;
	meta_alias_t *record;
	MYSQL_BIND parameters[1];
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = 1 };

	// Sanity check.
	if (!user || !(user->usernum)) {
		log_pedantic("Invalid data passed for structure build.");
		return false;
	}

	mm_wipe(parameters, sizeof(parameters));

	// Usernum.
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &(user->usernum);
	parameters[0].is_unsigned = true;

	if (!(result = stmt_get_result(stmts.select_mailbox_aliases, parameters))) {
		return false;
	}

	// If were updating, free the existing list of mailbox aliases.
	inx_cleanup(user->aliases);

	if (!(user->aliases = inx_alloc(M_INX_LINKED, &mm_free))) {
		log_error("Could not create a linked list for the mailbox aliases.");
		res_table_free(result);
		return false;
	}

	// Pass the alias data to the allocator and hope for the best. An aliasnum of NULL will be stored as 0 to indicate the
	// Mailbox address does not have an Alias record.
	while ((row = res_row_next(result))) {

		if ((record = alias_alloc(res_field_uint64(row, 0), PLACER(res_field_block(row, 1), res_field_length(row, 1)),
			PLACER(res_field_block(row, 2), res_field_length(row, 2)), res_field_uint8(row, 3), res_field_uint64(row, 4))) &&
			(!inx_insert(user->aliases, key, record))) {
			log_info("The index refused to accept the mailbox alias record. { address = %.*s / alias = %lu }", st_length_int(record->address),
				st_char_get(record->address), record->aliasnum);
			mm_free(record);
		}
		else {
			key.val.u64++;
		}

	}

	res_table_free(result);

	return true;
}

/**
 * @brief	Check to see if a specified mailbox exists.
 * @param	address		a managed string containing the email address to be matched against any of the rows in the Mailboxes table.
 * @return	true if the specified email address exists as a configured mailbox in the database or false otherwise.
 */
bool_t meta_data_check_mailbox(stringer_t *address) {

	table_t *table;
	MYSQL_BIND parameters[1];
	bool_t result = true;

	// Sanity check.
	if (st_empty(address)) {
		log_pedantic("Invalid mailbox address was specified.");
		return false;
	}

	mm_wipe(parameters, sizeof(parameters));

	// Usernum.
	parameters[0].buffer_type = MYSQL_TYPE_STRING;
	parameters[0].buffer_length = st_length_get(address);
	parameters[0].buffer = st_char_get(address);

	if (!(table = stmt_get_result(stmts.select_mailbox_address_any, parameters))) {
		return false;
	}

	// If the query resulted in no rows then there was no match.
	if (!res_row_next(table)) {
		result = false;
	}

	res_table_free(table);

	return result;
}

