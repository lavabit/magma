
/**
 * @file /magma/src/objects/meta/datatier.c
 *
 * @brief The meta object database interface.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Update the per-user entry in the Log table for the specified protocol.
 * @note	This function will set the last session timestamp for the user, and increment the sessions counter in the database.
 * @param	user	the new_meta user object of the user making the logging request.
 * @param	prot	the protocol associated with the log request: META_PROT_POP, META_PROT_IMAP, or META_PROT_WEB.
 * @return	This function returns no value.
 */
void new_meta_data_update_log(new_meta_user_t *user, META_PROTOCOL prot) {

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

	if (prot == META_PROTOCOL_POP && stmt_exec_affected(stmts.update_log_pop, parameters) != 1) {
		log_pedantic("Unable to update the POP statistics.");
	} else if (prot == META_PROTOCOL_IMAP && stmt_exec_affected(stmts.update_log_imap, parameters) != 1) {
		log_pedantic("Unable to update the IMAP statistics.");
	} else if (prot == META_PROTOCOL_WEB && stmt_exec_affected(stmts.update_log_web, parameters) != 1) {
		log_pedantic("Unable to update a users web statistics.");
	}

	return;
}

/**
 * @brief	Retrieve all of a user's message folders from the database.
 * @note	If the user already had a working set of message folders they will be deleted first.
 * @param	user	a pointer to the meta user object of the user making the request, which will be updated on success.
 * @return	-1 on failure or 1 on success.
 */
bool_t new_meta_data_fetch_folders(new_meta_user_t *user) {

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
 * @brief	Get all the mailbox aliases for a specified user.
 *
 * @param	user	a pointer to the partially populated meta user object to be queried, with its usernum field set.
 *
 * @return	true on success or false on failure.
 */
int_t new_meta_data_fetch_mailbox_aliases(new_meta_user_t *user) {

	row_t *row;
	table_t *result;
	meta_alias_t *record;
	MYSQL_BIND parameters[1];
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = 1 };

	// Sanity check.
	if (!user || !user->usernum) {
		log_pedantic("Invalid parameters passed to the user fetch function.");
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

	// Allocate a new index to hold the result.
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
 * @brief	Build a meta user object by username, hashed password, and hashed key storage password.
 *
 * @param	user	a meta object with the user number field populated.
 *
 * @return	-1 for unexpected program/system error, 0 on success, 1 if no rows are found.
 */
int_t new_meta_data_fetch_user(new_meta_user_t *user) {

	row_t *row;
	table_t *result;
	MYSQL_BIND parameters[1];

	// Sanity check.
	if (!user || !user->usernum) {
		log_pedantic("Invalid parameters passed to the user fetch function.");
		return -1;
	}

	// This function is used to update a user structure.
	mm_wipe(parameters, sizeof(parameters));

	// Usernum.
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &(user->usernum);
	parameters[0].is_unsigned = true;

	if (!(result = stmt_get_result(stmts.meta_fetch_user, parameters))) {
		return -1;
	}
	else if (!(row = res_row_next(result))) {
		res_table_free(result);
		return 1;
	}

	// Users.auth, Users.`ssl`, Uesrs.overquota, Dispatch.secure,

	// Store the verification token.
	if (!(user->verification = res_field_string(row, 0))) {
		log_pedantic("Unable to copy password hash.");
		res_table_free(result);
		return -1;
	}

	// Transport Layer Security
	if (res_field_int8(row, 1) == 1) {
		user->flags = (user->flags | META_USER_TLS);
	}
	else {
		user->flags = (user->flags | META_USER_TLS) ^ META_USER_TLS;
	}

	// Over Quota
	if (res_field_int8(row, 2) == 1) {
		user->flags = (user->flags | META_USER_OVERQUOTA);
	}
	else {
		user->flags = (user->flags | META_USER_OVERQUOTA) ^ META_USER_OVERQUOTA;
	}

	// Update the secure storage flag. Note that if secure storage is enabled, then transport layer security
	// must also be used to protect the data in transit.
	if (res_field_int8(row, 3) == 1) {
		user->flags = (user->flags | META_USER_ENCRYPT_DATA);
	}
	else {
		user->flags = (user->flags | META_USER_ENCRYPT_DATA) ^ META_USER_ENCRYPT_DATA;
	}

	res_table_free(result);
	return 0;
/*
	if (user->storage_privkey) {
		st_free(user->storage_privkey);
		user->storage_privkey = NULL;
	}

	if (user->storage_pubkey) {
		st_free(user->storage_pubkey);
		user->storage_pubkey = NULL;
	}

	// Finally, we make check if the user has generated a public and private key pair for encrypted storage. And generate new ones if encryption is enabled.
	// But they still might need a storage key if the the secure flag is off and there is a batch of encrypted mail messages that needs to be decrypted.
	if ((user->flags & META_USER_ENCRYPT_DATA) &&
		(meta_data_user_build_storage_keys (user->usernum, master, &(user->storage_privkey), &(user->storage_pubkey), false, false, 0) < 0)) {
		log_pedantic("A user with the secure storage feature enabled does not have storage keys and the creation attempt failed.");
		return -1;
	} else if (!(user->flags & META_USER_ENCRYPT_DATA) &&
		(meta_data_user_build_storage_keys (user->usernum, master, &(user->storage_privkey), &(user->storage_pubkey), true, false, 0) < 0)) {
		log_pedantic("A user with the secure storage feature disabled does not have storage keys in the database.");
	}

	return 1;
*/

}
