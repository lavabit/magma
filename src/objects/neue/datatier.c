/**
 * @file /magma/objects/neue/datatier.c
 *
 * @brief Functions used to interact with the database to the capacity needed by credentials authentication calculations.
 *
 * Author: Ivan
 * Date: 07/06/2015
 * Revision: Original
 *
 */

#include "magma.h"

/*
 * @brief	Fetches salt for the specified user name from the database.
 * @param	username	Stringer containing username.
 * @param	salt		Pointer to a pointer to a stringer, where the result is stored.
 * @return	0 if the salt is pulled correctly. 1 if the salt for the user is NULL. 2 if the user did not exist. -1 if an unknown error occurred.
 */
int_t credential_salt_fetch(stringer_t *username, stringer_t **salt) {

	int result;
	MYSQL_BIND parameters[1];
	row_t *row;
	stringer_t *temp;
	table_t *query;

	if(st_empty(username)) {
		log_pedantic("NULL username was passed in.");
		goto error;
	}

	mm_wipe(parameters, sizeof(parameters));

	parameters[0].buffer_type = MYSQL_TYPE_STRING;
	parameters[0].buffer_length = st_length_get(username);
	parameters[0].buffer = st_char_get(username);

	if(!(query = stmt_get_result(stmts.select_user_stacie_salt, parameters))) {
		log_error("Failure to query user salt.");
		result = -1;
		goto error;
	}

	if(!res_row_count(query)) {
		log_pedantic("Specified user does not exist in the database.");
		result = 2;
		goto cleanup_query;
	}

	if(res_row_count(query) > 1) {
		log_pedantic("Non-unique username.");
		result = -1;
		goto cleanup_query;
	}

	if(!(row = res_row_next(query))) {
		log_error("Failed to retrieve row.");
		goto cleanup_query;
	}

	temp = res_field_string(row, 0);
	res_table_free(query);

	if(!temp) {
		log_pedantic("No salt found for specified user.");
		result = 1;
		goto error;
	}

	*salt = hex_decode_opts(temp, (MANAGED_T | CONTIGUOUS | SECURE));
	st_free(temp);

	if(!(*salt)) {
		log_error("Failed to decode salt stringer.");
		result = -1;
		goto error;
	}

	return 0;

cleanup_query:
	res_table_free(query);
error:
	*salt = NULL;
	return result;
}
