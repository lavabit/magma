
/**
 * @file /magma/objects/mail/datatier.c
 *
 * @brief	Functions used to interface with and manage message data.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Set a message invisible in the database.
 * @param	messagenum	the message id of the mail message to be hidden.
 * @return	This function returns no value.
 */
void mail_db_hide_message(uint64_t messagenum) {

	MYSQL_BIND parameters[1];

	mm_wipe(parameters, sizeof(parameters));

	// Messagenum
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &messagenum;
	parameters[0].is_unsigned = true;

	// Hide the corrupt message.
	stmt_exec(stmts.update_message_visibility, parameters);

	return;
}

/**
 * @brief	Delete a mail message from the mysql database and adjust the owner's quota.
 * @param	usernum		the user id to whom the target mail message belongs.
 * @param	messagenum	the message id of the mail message to be deleted.
 * @param	size		the storage size, in bytes, of the message to be deleted.
 * @param	transaction	the mysql connection id on which to execute the statements.
 * @return	0 on failure or 1 on success.
 */
bool_t mail_db_delete_message(uint64_t usernum, uint64_t messagenum, uint32_t size, int_t transaction) {

	MYSQL_BIND parameters[2];
	uint64_t affected;

	mm_wipe(parameters, sizeof(parameters));

	// Messagenum
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &messagenum;
	parameters[0].is_unsigned = true;

	// Usernum
	parameters[1].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[1].buffer_length = sizeof(uint64_t);
	parameters[1].buffer = &usernum;
	parameters[1].is_unsigned = true;

	// Remove from the Messages table.
	if ((affected = stmt_exec_affected_conn(stmts.delete_message, parameters, transaction)) == 0) {
		log_error("Unable to delete the message from Messages table. The user number was %lu, the message number was %lu and the message size was %u.",
				usernum, messagenum, size);
		return false;
	}

	mm_wipe(parameters, sizeof(parameters));

	// Message Size
	parameters[0].buffer_type = MYSQL_TYPE_LONG;
	parameters[0].buffer_length = sizeof(uint32_t);
	parameters[0].buffer = &size;
	parameters[0].is_unsigned = true;

	// Usernum
	parameters[1].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[1].buffer_length = sizeof(uint64_t);
	parameters[1].buffer = &usernum;
	parameters[1].is_unsigned = true;

	// Update the Users table.
	if ((affected = stmt_exec_affected_conn(stmts.update_user_quota_subtract, parameters, transaction)) == 0) {
		log_error("Unable to update the Users table. The user number was %lu, the message number was %lu and the message size was %u.",
				usernum, messagenum, size);
		return false;
	}

	return true;
}

/**
 * @brief	Update a mail message's parent folder in the database.
 * @brief	usernum			the numerical id of the user to whom the mail message belongs.
 * @brief	messagenum		the numerical id of the target mail message of the operation.
 * @brief	source			the numerical id of the parent folder in which the mail message currently resides.
 * @brief	target			the numerical id of the destination folder which is to be the new parent of the mail message.
 * @brief	transaction		a transaction id for the database operation, in case the caller needs to roll back changes on failure.
 * @return	-1 on failure, 0 if the target message could not be located in the database, or 1 on success.
 */
int_t mail_db_update_message_folder(uint64_t usernum, uint64_t messagenum, uint64_t source, uint64_t target, int64_t transaction) {

	uint64_t result;
	MYSQL_BIND parameters[4];

	if (!usernum || !messagenum || !source || !target || transaction < 0) {
		log_pedantic("Passed an invalid message parameter.");
		return -1;
	}

	mm_wipe(parameters, sizeof(parameters));

	// Target Folder
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &target;
	parameters[0].is_unsigned = true;

	// Messagenum
	parameters[1].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[1].buffer_length = sizeof(uint64_t);
	parameters[1].buffer = &messagenum;
	parameters[1].is_unsigned = true;

	// Usernum
	parameters[2].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[2].buffer_length = sizeof(uint64_t);
	parameters[2].buffer = &usernum;
	parameters[2].is_unsigned = true;

	// Source Folder
	parameters[3].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[3].buffer_length = sizeof(uint64_t);
	parameters[3].buffer = &source;
	parameters[3].is_unsigned = true;

	// Since the result is unsigned, an error is indicated by a return value of -1.
	if ((result = stmt_exec_affected_conn(stmts.update_message_folder, parameters, transaction)) != 1 && result == -1) {
		log_pedantic("An error occurred while trying to move a message into a different folder. { user = %lu / message = %lu / source = %lu / "
			"target = %lu / query = -1 / error = %u = %s }", usernum, messagenum, source, target, mysql_stmt_errno_d(pool_get_obj(sql_pool, transaction)),
			mysql_stmt_error_d(pool_get_obj(sql_pool, transaction)));
		return -1;
	}

#ifdef MAGMA_PEDANTIC
	else if (result > 1) {
		log_pedantic("The message folder update affected more than one database row. Since we supplied the primary key this should never happen! "
			"{ user = %lu / message = %lu / source = %lu / target = %lu }", usernum, messagenum, source, target);
		return -1;
	}
#endif

	// If the message couldn't be located we return zero.
	if (!result) {
		return 0;
	}

	return 1;
}

/**
 * @brief	Insert a mail message into the database.
 * @note	This function will also update the user's storage quota information in the database.
 * @param	usernum		the numerical id of the user to whom the mail message will belong.
 * @param	foldernum	the numerical id of the folder that will be the parent folder of the message.
 * @param	status		the status flags of the mail message.
 * @param	size		the size, in bytes, of the mail message on disk.
 * @param	signum		the spam signature for the message.
 * @param	sigkey		the spam key for the message.
 * @param	transaction	the transaction id for the database operation, in case the caller wants to roll back the transaction.
 * @return	0 on failure or the id of the newly inserted mail message on success.
 */
uint64_t mail_db_insert_message(uint64_t usernum, uint64_t foldernum, uint32_t status, uint32_t size, uint64_t signum, uint64_t sigkey, int_t transaction) {

	uint64_t result;
	MYSQL_BIND parameters[7];

	if (!usernum || !foldernum || !size || transaction < 0) {
		log_pedantic("Passed an invalid message parameter.");
		return 0;
	}

	mm_wipe(parameters, sizeof(parameters));

	// Usernum
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &usernum;
	parameters[0].is_unsigned = true;

	// Foldernum
	parameters[1].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[1].buffer_length = sizeof(uint64_t);
	parameters[1].buffer = &foldernum;
	parameters[1].is_unsigned = true;

	// Server
	parameters[2].buffer_type = MYSQL_TYPE_STRING;
	parameters[2].buffer_length = st_length_get(magma.storage.active);
	parameters[2].buffer = st_char_get(magma.storage.active);

	// Status
	parameters[3].buffer_type = MYSQL_TYPE_LONG;
	parameters[3].buffer_length = sizeof(uint32_t);
	parameters[3].buffer = &status;
	parameters[3].is_unsigned = true;

	// Size
	parameters[4].buffer_type = MYSQL_TYPE_LONG;
	parameters[4].buffer_length = sizeof(uint32_t);
	parameters[4].buffer = &size;
	parameters[4].is_unsigned = true;

	// Signature
	if (signum) {
		parameters[5].buffer_type = MYSQL_TYPE_LONGLONG;
		parameters[5].buffer_length = sizeof(uint64_t);
		parameters[5].buffer = &signum;
		parameters[5].is_unsigned = true;
	}
	else {
		parameters[5].buffer_type = MYSQL_TYPE_LONGLONG;
		parameters[5].is_null = ISNULL(true);
	}

	// Signature key
	if (sigkey) {
		parameters[6].buffer_type = MYSQL_TYPE_LONGLONG;
		parameters[6].buffer_length = sizeof(uint64_t);
		parameters[6].buffer = &sigkey;
		parameters[6].is_unsigned = true;
	}
	else {
		parameters[6].buffer_type = MYSQL_TYPE_LONGLONG;
		parameters[6].is_null = ISNULL(true);
	}

	// Execute the insert.
	if (!(result = stmt_insert_conn(stmts.insert_message, parameters, transaction))) {

		// Its possible for a signature to be deleted. So failures are retried without a signature number.
		// That we flip the spam status flag, but its possible that the spam signature was deleted even though
		// the message wasn't trained.
		if ((status & MAIL_MARK_JUNK) == MAIL_MARK_JUNK) {
			status = (status ^ MAIL_MARK_JUNK);
		}
		else {
			status = (status | MAIL_MARK_JUNK);
		}

		parameters[3].buffer_type = MYSQL_TYPE_LONG;
		parameters[3].buffer_length = sizeof(uint32_t);
		parameters[3].buffer = &status;
		parameters[3].is_unsigned = true;

		parameters[5].buffer_type = MYSQL_TYPE_LONGLONG;
		parameters[5].is_null = ISNULL(true);

		parameters[6].buffer_type = MYSQL_TYPE_LONGLONG;
		parameters[6].is_null = ISNULL(true);

		if (!(result = stmt_insert_conn(stmts.insert_message, parameters, transaction))) {
			log_pedantic("An error occurred while inserting the message.");
			return 0;
		}

	}

	// Update the quota.
	mm_wipe(parameters, sizeof(parameters));

	// Size
	parameters[0].buffer_type = MYSQL_TYPE_LONG;
	parameters[0].buffer_length = sizeof(uint32_t);
	parameters[0].buffer = &size;
	parameters[0].is_unsigned = true;

	// Usernum
	parameters[1].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[1].buffer_length = sizeof(uint64_t);
	parameters[1].buffer = &usernum;
	parameters[1].is_unsigned = true;

	if (!stmt_exec_conn(stmts.update_user_quota_add, parameters, transaction)) {
		log_pedantic("Unable to update the user's quota.");
		return 0;
	}

	return result;
}

/**
 * @brief	Insert a duplicate entry for a message in the database.
 * @note	This function will also update the user's storage quota information in the database.
 * @param	usernum		the numerical id ot eh user that owns the message.
 * @param	foldernum	the numerical id of the parent folder containing the message.
 * @param	status		the status flags value for the message.
 * @param	size		the size, in bytes, of the mail message on disk.
 * @param	signum		the spam signature for the message.
 * @param	sigkey		the spam key for the message.
 * @param	created		the UNIX timestamp of when the message was created.
 * @param	transaction	the transaction id for the database operation, in case the caller wants to roll back the transaction.
 * @return	NULL on failure, or the ID of the newly inserted message on success.
 */
uint64_t mail_db_insert_duplicate_message(uint64_t usernum, uint64_t foldernum, uint32_t status, uint32_t size, uint64_t signum, uint64_t sigkey, uint64_t created, int_t transaction) {

	uint64_t result;
	MYSQL_BIND parameters[8];

	if (!usernum || !foldernum || !size || transaction < 0) {
		log_pedantic("Passed an invalid message parameter.");
		return 0;
	}

	mm_wipe(parameters, sizeof(parameters));

	// Usernum
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &usernum;
	parameters[0].is_unsigned = true;

	// Foldernum
	parameters[1].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[1].buffer_length = sizeof(uint64_t);
	parameters[1].buffer = &foldernum;
	parameters[1].is_unsigned = true;

	// Server
	parameters[2].buffer_type = MYSQL_TYPE_STRING;
	parameters[2].buffer_length = st_length_get(magma.storage.active);
	parameters[2].buffer = st_char_get(magma.storage.active);

	// Status
	parameters[3].buffer_type = MYSQL_TYPE_LONG;
	parameters[3].buffer_length = sizeof(uint32_t);
	parameters[3].buffer = &status;
	parameters[3].is_unsigned = true;

	// Size
	parameters[4].buffer_type = MYSQL_TYPE_LONG;
	parameters[4].buffer_length = sizeof(uint32_t);
	parameters[4].buffer = &size;
	parameters[4].is_unsigned = true;

	// Signature
	if (signum) {
		parameters[5].buffer_type = MYSQL_TYPE_LONGLONG;
		parameters[5].buffer_length = sizeof(uint64_t);
		parameters[5].buffer = &signum;
		parameters[5].is_unsigned = true;
	}
	else {
		parameters[5].buffer_type = MYSQL_TYPE_LONGLONG;
		parameters[5].is_null = ISNULL(true);
	}

	// Signature key
	if (sigkey) {
		parameters[6].buffer_type = MYSQL_TYPE_LONGLONG;
		parameters[6].buffer_length = sizeof(uint64_t);
		parameters[6].buffer = &sigkey;
		parameters[6].is_unsigned = true;
	}
	else {
		parameters[6].buffer_type = MYSQL_TYPE_LONGLONG;
		parameters[6].is_null = ISNULL(true);
	}

	// Created
	parameters[7].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[7].buffer_length = sizeof(uint64_t);
	parameters[7].buffer = &created;
	parameters[7].is_unsigned = true;

	// Execute the insert.
	if (!(result = stmt_insert_conn(stmts.insert_message_duplicate, parameters, transaction))) {

		// Its possible for a signature to be deleted. So failures are retried without a signature number.
		// That we flip the spam status flag, but its possible that the spam signature was deleted even though
		// the message wasn't trained.
		if ((status & MAIL_MARK_JUNK) == MAIL_MARK_JUNK) {
			status = (status ^ MAIL_MARK_JUNK);
		}
		else {
			status = (status | MAIL_MARK_JUNK);
		}

		parameters[3].buffer_type = MYSQL_TYPE_LONG;
		parameters[3].buffer_length = sizeof(uint32_t);
		parameters[3].buffer = &status;
		parameters[3].is_unsigned = true;

		parameters[5].buffer_type = MYSQL_TYPE_LONGLONG;
		parameters[5].buffer = NULL;
		parameters[5].is_null = ISNULL(true);

		if (!(result = stmt_insert_conn(stmts.insert_message_duplicate, parameters, transaction))) {
			log_pedantic("An error occurred while inserting the message.");
			return 0;
		}
	}

	// Update the quota.
	mm_wipe(parameters, sizeof(parameters));

	// Size
	parameters[0].buffer_type = MYSQL_TYPE_LONG;
	parameters[0].buffer_length = sizeof(uint32_t);
	parameters[0].buffer = &size;
	parameters[0].is_unsigned = true;

	// Usernum
	parameters[1].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[1].buffer_length = sizeof(uint64_t);
	parameters[1].buffer = &usernum;
	parameters[1].is_unsigned = true;

	if (!stmt_exec_conn(stmts.update_user_quota_add, parameters, transaction)) {
		log_pedantic("Unable to update the user's quota.");
		return 0;
	}

	return result;
}
