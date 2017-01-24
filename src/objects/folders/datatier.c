
/**
 * @file /magma/objects/folders/datatier.c
 *
 * @brief	Folder data functions.
 */

#include "magma.h"

/**
 * @brief	Fetch all of a user's folders of a specified type from the database.
 * @param	usernum		the numerical id of the requested user.
 * @param	type		the folder class of the folders to be retrieved (can be M_FOLDER_MESSAGES or M_FOLDER_CONTACTS).
 * @return	NULL on failure, or an inx object holding all of the requested folders on success.
 */
inx_t * magma_folder_fetch(uint64_t usernum, uint_t type) {

	row_t *row;
	inx_t *output;
	table_t *result;
	magma_folder_t *record;
	MYSQL_BIND parameters[2];
	void (*folder_free)(magma_folder_t *);
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = 0 };
	magma_folder_t * (*folder_alloc)(uint64_t, uint64_t, uint32_t, stringer_t *);

	mm_wipe(parameters, sizeof(parameters));

	if (!usernum) {
		log_pedantic("Invalid user number requested. { user = 0 }");
		return NULL;
	}
	else if (!magma_folder_funcs(type, &folder_alloc, &folder_free)) {
		log_pedantic("Invalid folder type. { type = %u }", type);
		return NULL;
	}

	// Usernum
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &usernum;
	parameters[0].is_unsigned = true;

	// Type
	parameters[1].buffer_type = MYSQL_TYPE_LONG;
	parameters[1].buffer_length = sizeof(uint_t);
	parameters[1].buffer = &type;
	parameters[1].is_unsigned = true;

	if (!(result = stmt_get_result(stmts.select_folders, parameters))) {
		log_pedantic("No incoming mail domains configured.");
		return NULL;
	}
	else if (!(output = inx_alloc(M_INX_TREE, folder_free))) {
		res_table_free(result);
		return NULL;
	}

	// Loop through each of the row returned.
	while ((row = res_row_next(result))) {

		// Pass the folder fields into the allocator.
		if (!(record = folder_alloc(res_field_uint64(row, 0), res_field_uint64(row, 1), res_field_uint32(row, 2),
			PLACER(res_field_block(row, 3), res_field_length(row, 3))))	|| !(key.val.u64 = record->foldernum) || !inx_insert(output, key, record)) {
			log_info("The index refused to accept a folder record. { folder = %lu }", res_field_uint64(row, 0));

			if (record) {
				folder_free(record);
			}

			res_table_free(result);
			inx_free(output);
			return NULL;
		}
	}

	res_table_free(result);

	return output;
}

/**
 * @brief       Insert a new folder into the database.
 * @param       usernum         the numerical id of the user to whom the new folder belongs.
 * @param       name            a managed string containing the name of the new folder.
 * @param       parent          the numerical id of the mail folder to be the parent of the new mail folder.
 * @param       order           the order number of this folder in its parent folder.
 * @param		type			the type of the new folder (can be M_FOLDER_MESSAGES or M_FOLDER_CONTACTS).
 * @return      0 on failure, or the numerical id of the newly inserted folder in the database on success.
 */
uint64_t magma_folder_insert(uint64_t usernum, stringer_t *name, uint64_t parent, uint32_t order, uint_t type) {

	MYSQL_BIND parameters[5];

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
 * @brief	Delete a folder from the database.
 * @param	usernum		the numerical id of the user requesting the folder removal.
 * @param	foldernum	the numerical id of the folder to be removed.
 * @param	type		the type of the folder to be deleted (can be M_FOLDER_MESSAGES or M_FOLDER_CONTACTS).
 * @return	true on success or false on failure.
 */
bool_t magma_folder_delete(uint64_t usernum, uint64_t foldernum, uint_t type) {

	MYSQL_BIND parameters[3];

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
	if (stmt_exec_affected(stmts.delete_folder, parameters) != 1) {
		return false;
	}

	return true;
}

/**
 * @brief	Rename a folder in the database.
 * @param	usernum		the numerical id of the user requesting the folder renaming.
 * @param	foldernum	the numerical id of the folder to be renamed.
 * @param	type		the type of the folder to be renamed (can be M_FOLDER_MESSAGES or M_FOLDER_CONTACTS).
 * @param	rename		a managed string containing the new name of the specified folder.
 * @return	true on success or false on failure.
 */
bool_t magma_folder_rename(uint64_t usernum, uint64_t foldernum, uint_t type, stringer_t *rename) {

	MYSQL_BIND parameters[4];

	mm_wipe(parameters, sizeof(parameters));

	// New name
	parameters[0].buffer_type = MYSQL_TYPE_STRING;
	parameters[0].buffer_length = st_length_get(rename);
	parameters[0].buffer = st_char_get(rename);

	// Foldernum
	parameters[1].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[1].buffer_length = sizeof(uint64_t);
	parameters[1].buffer = &foldernum;
	parameters[1].is_unsigned = true;

	// Usernum
	parameters[2].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[2].buffer_length = sizeof(uint64_t);
	parameters[2].buffer = &usernum;
	parameters[2].is_unsigned = true;

	// Folder Type
	parameters[3].buffer_type = MYSQL_TYPE_LONG;
	parameters[3].buffer_length = sizeof(uint_t);
	parameters[3].buffer = &(type);
	parameters[3].is_unsigned = true;

	// Should only update one row.
	if (stmt_exec_affected(stmts.rename_folder, parameters) != 1) {
		return false;
	}

	return true;
}
