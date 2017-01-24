
/**
 * @file /magma/objects/meta/datatier.c
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
 *
 * @note	This function will set the last session timestamp for the user, and increment the sessions counter in the database.
 *
 * @param	user	the meta user object of the user making the logging request.
 * @param	prot	the protocol associated with the log request: META_PROT_POP, META_PROT_IMAP, or META_PROT_WEB.
 *
 * @return	This function returns no value.
 */
void meta_data_update_log(meta_user_t *user, META_PROTOCOL prot) {

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
 * @brief	Update a user's lock in the database.
 *
 * @param	usernum		the numerical id of the user for whom the lock will be set.
 * @param	lock		the new value to which the specified user's lock will be set.
 *
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
 * @brief	Retrieve all of a user's message folders from the database.
 *
 * @note	If the user already had a working set of message folders they will be deleted first.
 *
 * @param	user	a pointer to the meta user object of the user making the request, which will be updated on success.
 *
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
 * @brief	Get all the mailbox aliases for a specified user.
 *
 * @param	user	a pointer to the partially populated meta user object to be queried, with its usernum field set.
 *
 * @return	true on success or false on failure.
 */
int_t meta_data_fetch_mailbox_aliases(meta_user_t *user) {

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
int_t meta_data_fetch_user(meta_user_t *user) {

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

	// Store the username.
	if (st_empty(user->username) && !(user->username = res_field_string(row, 0))) {
		log_pedantic("Unable to copy username into the user object.");
		res_table_free(result);
		return -1;
	}

	// Store the verification token.
	if (st_empty(user->verification) && !(user->verification = base64_decode_mod(PLACER(res_field_block(row, 1), res_field_length(row, 1)), NULL))) {
		log_pedantic("Unable to copy password hash.");
		res_table_free(result);
		return -1;
	}

	// Transport Layer Security
	if (res_field_int8(row, 2) == 1) {
		user->flags = (user->flags | META_USER_TLS);
	}
	else {
		user->flags = (user->flags | META_USER_TLS) ^ META_USER_TLS;
	}

	// Over Quota
	if (res_field_int8(row, 3) == 1) {
		user->flags = (user->flags | META_USER_OVERQUOTA);
	}
	else {
		user->flags = (user->flags | META_USER_OVERQUOTA) ^ META_USER_OVERQUOTA;
	}

	// Update the secure storage flag. Note that if secure storage is enabled, then transport layer security
	// must also be used to protect the data in transit.
	if (res_field_int8(row, 4) == 1) {
		user->flags = (user->flags | META_USER_ENCRYPT_DATA);
	}
	else {
		user->flags = (user->flags | META_USER_ENCRYPT_DATA) ^ META_USER_ENCRYPT_DATA;
	}

	res_table_free(result);
	return 0;
}

/**
 * @brief	Retrieve the encryption keys for a user account.
 *
 * @param	user	a meta object with the user number field populated.
 * @param	output	a key pair object to hold the results.
 * @param	transaction	the mysql transaction id of the acknowledgment operation, in cases batch changes need to be rolled back.
 *
 * @return	-1 for unexpected program/system error, 0 on success, 1 if no rows are found.
 */
int_t meta_data_fetch_keys(meta_user_t *user, key_pair_t *output, int64_t transaction) {

	row_t *row;
	table_t *result;
	MYSQL_BIND parameters[1];
	stringer_t *public = NULL, *private = NULL;

	// Sanity check.
	if (!user || !user->usernum || !output || !st_empty(output->public, output->private)) {
		log_pedantic("Invalid parameters passed to the storage key fetch function.");
		return -1;
	}

	// This function is used to fetch user keys.
	mm_wipe(parameters, sizeof(parameters));

	// Usernum.
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &(user->usernum);
	parameters[0].is_unsigned = true;

	if (!(result = stmt_get_result_conn(stmts.meta_fetch_mail_keys, parameters, transaction))) {
		return -1;
	}
	else if (!(row = res_row_next(result))) {
		res_table_free(result);
		return 1;
	}

	// Store the public key. Using a hard coded length. This will need to change if the key format/type ever changes.
	if (res_field_length(row, 0) != 98 || !(public = base64_decode_mod(PLACER(res_field_block(row, 0), res_field_length(row, 0)), NULL))) {
		log_pedantic("Unable to decode and store a public user key. { username = %.*s / length = %zu }", st_length_int(user->username),
			st_char_get(user->username), res_field_length(row, 0));
		res_table_free(result);
		return -1;
	}

	// Store the private key. Using a hard coded length. This will need to change if the key format/type ever changes. Also note, we don't
	// need secure memory at this stage because the private key is still encrypted.
	if (res_field_length(row, 1) != 171 || !(private = base64_decode_mod(PLACER(res_field_block(row, 1), res_field_length(row, 1)), NULL))) {
		log_pedantic("Unable to decode and store a private user key. { username = %.*s / length = %zu }", st_length_int(user->username),
			st_char_get(user->username), res_field_length(row, 1));
		res_table_free(result);
		st_free(public);
		return -1;
	}

	// Free the query results and return the decoded values.
	res_table_free(result);
	output->private = private;
	output->public = public;
	return 0;
}

/**
 * @brief	Store the encryption keys for a user account.
 *
 * @param	usernum		the numerical id of the user to whom the alert message belongs.
 * @param	username	the plain text username.
 * @param	input		the public and private key pair.
 * @param	transaction	the mysql transaction id of the acknowledgment operation, in cases batch changes need to be rolled back.
 *
 * @return	-1 for unexpected program/system error, 0 on success, 1 if the query executes, but no rows are affected.
 */
int_t meta_data_insert_keys(uint64_t usernum, stringer_t *username, key_pair_t *input, int64_t transaction) {

	int64_t affected;
	MYSQL_BIND parameters[3];
	stringer_t *public = NULL, *private = NULL;

	// Sanity check.
	if (!usernum || !input || !st_populated(input->public, input->private)) {
		log_pedantic("Invalid parameters passed to the storage key insert function.");
		return -1;
	}

	// Encode the key values using base64 modified. We are also checking to make sure the encoded values are the expected length. Also,
	// note the key length is hard coded here but is dynamic below, on purpose, to make changes easier.
	if (!(public = base64_encode_mod(input->public, NULL)) || !(private = base64_encode_mod(input->private, NULL))) {
		log_pedantic("Unable to store a user key pair. { username = %.*s }", st_length_int(username),
			st_char_get(username));
		st_cleanup(public, private);
		return -1;
	}

	// This function is used to update a user structure.
	mm_wipe(parameters, sizeof(parameters));

	// Usernum.
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &(usernum);
	parameters[0].is_unsigned = true;

	// Public Key
	parameters[1].buffer_type = MYSQL_TYPE_STRING;
	parameters[1].buffer_length = st_length_get(public);
	parameters[1].buffer = st_char_get(public);


	parameters[2].buffer_type = MYSQL_TYPE_STRING;
	parameters[2].buffer_length = st_length_get(private);
	parameters[2].buffer = st_char_get(private);

	if ((affected = stmt_exec_affected_conn(stmts.meta_insert_mail_keys, parameters, transaction)) != 1 && affected == -1) {
		log_pedantic("Unable to insert the user signet and key. A database error occurred. { username = %.*s }",
			st_length_int(username), st_char_get(username));
		st_cleanup(public, private);
		return -1;
	}
	else if (!affected) {
		log_pedantic("Unable to insert the user key pair. It's possible the database already holds keys for this user. { username = %.*s }",
			st_length_int(username), st_char_get(username));
		st_cleanup(public, private);
		return 1;
	}

	st_cleanup(public, private);
	return 0;
}

/**
 * @brief	Retrieve a shard value for user account.
 *
 * @param	usernum		the numerical id of the user to whom the alert message belongs.
 * @param	serial		the nummerid serial number associated with the shard value.
 * @param	label		the textual label associated with the shard value.
 * @param	output		the buffer where the binary output should be stored.
 * @param	transaction	the mysql transaction id of the acknowledgment operation, in cases batch changes need to be rolled back.
 *
 * @return	-1 for unexpected program/system error, 0 on success, 1 if no rows are found.
 */
int_t meta_data_fetch_shard(uint64_t usernum, uint16_t serial, stringer_t *label, stringer_t *output, int64_t transaction) {

	row_t *row;
	table_t *result;
	MYSQL_BIND parameters[3];

	// Sanity check.
	if (!usernum || st_empty(label) || !output || !st_valid_destination(st_opt_get(output)) ||
		st_avail_get(output) != STACIE_SHARD_LENGTH) {
		log_pedantic("Invalid parameters passed to the shard fetch function.");
		return -1;
	}

	// This function is used to fetch a user shard for value for a given realm.
	mm_wipe(parameters, sizeof(parameters));

	// Usernum.
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &usernum;
	parameters[0].is_unsigned = true;

	// Serial
	parameters[1].buffer_type = MYSQL_TYPE_SHORT;
	parameters[1].buffer_length = sizeof(uint16_t);
	parameters[1].buffer = &serial;
	parameters[1].is_unsigned = true;

	// Label
	parameters[2].buffer_type = MYSQL_TYPE_STRING;
	parameters[2].buffer = st_data_get(label);
	parameters[2].buffer_length = st_length_get(label);

	if (!(result = stmt_get_result_conn(stmts.meta_fetch_shard, parameters, transaction))) {
		return -1;
	}
	else if (!(row = res_row_next(result))) {
		res_table_free(result);
		return 1;
	}

	// Store the shard value.
	if (res_field_length(row, 0) != 86 || !(output = base64_decode_mod(PLACER(res_field_block(row, 0), res_field_length(row, 0)), output))) {
		log_pedantic("Unable to decode and store a user shard value. { usernum = %lu / label = %.*s / serial = %hu / length = %zu }",
			usernum, st_length_int(label), st_char_get(label), serial, res_field_length(row, 0));
		res_table_free(result);
		return -1;
	}

	// Free the query results and return the decoded values.
	res_table_free(result);
	return 0;
}

/**
* @brief	Store the user realm shard value.
 *
 * @param	usernum		the numerical id of the user to whom the alert message belongs.
 * @param	serial		the nummerid serial number associated with the shard value.
 * @param	label		the textual label associated with the shard value.
 * @param	shard		the binary shard value.
 * @param	transaction	the mysql transaction id of the acknowledgment operation, in cases batch changes need to be rolled back.
 *
 * @return	-1 for unexpected program/system error, 0 on success, 1 if the query executes, but no rows are affected.
 */
int_t meta_data_insert_shard(uint64_t usernum, uint16_t serial, stringer_t *label, stringer_t *shard, int64_t transaction) {

	int64_t affected;
	MYSQL_BIND parameters[4];
	stringer_t *b64_shard = NULL;

	// Sanity check.
	if (!usernum || st_empty(label) || st_empty(shard) || st_length_get(shard) != STACIE_SHARD_LENGTH) {
		log_pedantic("Invalid parameters passed to the shard insert function.");
		return -1;
	}

	// Encode the key values using base64 modified. We are also checking to make sure the encoded values are the expected length. Also,
	// note the key length is hard coded here but is dynamic below, on purpose, to make changes easier.
	if (!(b64_shard = base64_encode_mod(shard, MANAGEDBUF(87)))) {
		log_pedantic("Unable to store a user realm shard. { usernum = %lu }", usernum);
		return -1;
	}

	// This function is used to update a user structure.
	mm_wipe(parameters, sizeof(parameters));

	// Usernum.
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &usernum;
	parameters[0].is_unsigned = true;

	// Serial
	parameters[1].buffer_type = MYSQL_TYPE_SHORT;
	parameters[1].buffer_length = sizeof(uint16_t);
	parameters[1].buffer = &serial;
	parameters[1].is_unsigned = true;

	// Label
	parameters[2].buffer_type = MYSQL_TYPE_STRING;
	parameters[2].buffer = st_data_get(label);
	parameters[2].buffer_length = st_length_get(label);

	// Shard
	parameters[3].buffer_type = MYSQL_TYPE_STRING;
	parameters[3].buffer = st_data_get(b64_shard);
	parameters[3].buffer_length = st_length_get(b64_shard);

	if ((affected = stmt_exec_affected_conn(stmts.meta_insert_shard, parameters, transaction)) != 1 && affected == -1) {
		log_pedantic("Unable to insert the user shard value. A database error occurred. { usernum = %lu / label = %.*s }",
			usernum, st_length_int(label), st_char_get(label));
		return -1;
	}
	else if (!affected) {
		log_pedantic("Unable to insert the user shard value. It's possible the database already holds a shard for this user. {  usernum = %lu / label = %.*s }",
			usernum, st_length_int(label), st_char_get(label));
		return 1;
	}

	return 0;
}

/**
 * @brief	Mark a user alert message as acknowledged in the database.
 *
 * @note	If the table is not updated immediately, another check is made to see if the alert is still pending. If so, false is returned.
 *
 * @param	alertnum	the numerical id of the alert message to be acknowledged.
 * @param	usernum		the numerical id of the user to whom the alert message belongs.
 * @param	transaction	the mysql transaction id of the acknowledgment operation, in cases batch changes need to be rolled back.
 *
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
 *
 * @param	usernum		the numerical id of the user for whom the alert messages will be fetched.
 *
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
 * @brief	Remove all user (non-system) flags from a collection of mail messages, and set the specified flags mask for them.
 *
 * @note	The new mask can contain both user and system flags, but only user flags will be stripped from each message initially.
 *
 * @param	messages	an inx holder containing the collection of messages to have their flags updated.
 * @param	usernum		the numerical of the user to whom the target messages belong, for validation purposes.
 * @param	foldernum	the numerical id of the parent folder containing the messages to be updated, for validation purposes.
 * @param	flags		a mask of all flags that are to be added to any matching messages in the collection.
 *
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
 *
 * @param	messages	an inx holder containing the collection of messages to have their flags removed.
 * @param	usernum		the numerical id of the user to whom the target messages belong, for validation purposes.
 * @param	foldernum	the numerical id of the parent folder containing the messages to be updated, for validation purposes.
 * @param	flags		a mask of all flags that are to be stripped from any matching messages in the collection.
 *
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
 *
 * @param	messages	an inx holder containing the collection of messages to have their flags updated.
 * @param	usernum		the numerical id of the user to whom the target messages belong, for validation purposes.
 * @param	foldernum	the numerical id of the parent folder containing the messages to be updated, for validation purposes.
 * @param	flags		a mask of all flags that are to be added to any matching messages in the collection.
 *
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
 *
 * @param	usernum		the numerical id of the user that owns the folder to be deleted.
 * @param	foldernum	the folder id of the message folder to be deleted.
 *
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
 *
 * @param	usernum		the numerical id of the user that owns the specified folder.
 * @param	foldernum	the id of the folder to have its properties adjusted.
 * @param	name		a managed string containing the new name of the specified folder.
 * @param	parent		the id of the new parent folder to be set for the specified message folder.
 * @param	order		the value of the order for the specified folder.
 *
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
 *
 * @param	usernum		the numerical id of the user to whom the new folder belongs.
 * @param	name		a managed string containing the name of the new mail folder.
 * @param	parent		the numerical id of the mail folder to be the parent of the new mail folder.
 * @param	order		the order number of this folder in its parent folder.
 *
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
 * @brief	Insert a tag for a message into the database.
 *
 * @param	message		the meta message object of the message to be tagged.
 * @param	tag			a managed string containing the name of the tag.
 *
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
	if (stmt_exec_affected(stmts.insert_message_tag, parameters) == -1) {
		return -1;
	}

	return 0;
}

/**
 * @brief	Remove all tags associated with a message in the database.
 *
 * @param	message		a pointer to the meta message object of the message to have all of its tags stripped.
 *
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
	if (stmt_exec_affected(stmts.delete_message_tags, parameters) == -1) {
		return -1;
	}

	return 0;
}

/**
 * @brief	Remove a tag from a message in the database.
 *
 * @param	message		a pointer to the meta message object of the message to have the tag stripped.
 * @param	tag			a managed string containing the name of the tag to be deleted.
 *
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
	if (stmt_exec_affected(stmts.delete_message_tag, parameters) == -1) {
		return -1;
	}

	return 0;
}

/**
 * @brief	Fetch all the tags attached to messages of a specified user from the database.
 *
 * @param	usernum		the numerical id of the user for whom the message tags will be fetched.
 *
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


