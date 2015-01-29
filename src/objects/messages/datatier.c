
/**
 * @file /magma/objects/messages/datatier.c
 *
 * @brief	Message data functions.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Populate a message folder with all of its child messages from the database.
 * @param	usernum		the numerical id of the user that owns the folder.
 * @param	folder		a pointer to the message folder object to be populated.
 * @return	true on success or false on failure.
 */
bool_t messages_fetch(uint64_t usernum, message_folder_t *folder) {

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

		if (!(record = message_alloc(res_field_uint64(row, 0), res_field_uint64(row, 1), res_field_uint64(row, 2), res_field_uint64(row, 3), res_field_uint32(row, 4),
			PLACER(res_field_block(row, 5), res_field_length(row, 5)), res_field_uint32(row, 6))) || !(key.val.u64 = record->message.num) || !inx_insert(folder->records, key, record)) {
			log_info("The index refused to accept a message record. { message = %lu }", res_field_uint64(row, 0));

			if (record) {
				message_free(record);
			}

			res_table_free(result);
			return false;
		}

	}

	res_table_free(result);

	return true;
}
