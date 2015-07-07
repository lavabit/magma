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
 * @brief	Fetches salt for specified username.
 * @param	username	Stringer containing username.
 * @return	Stringer containing salt. If error or user does not exist return NULL, if user exists but no salt exists return an empty stringer.
 */
stringer_t * credential_fetch_salt(stringer_t *username) {

	MYSQL_BIND parameters[1];
	row_t *row;
	stringer_t *temp, *result = NULL;
	table_t *query;

	parameters[0].buffer_type = MYSQL_TYPE_STRING;
	parameters[0].buffer_length = st_length_get(username);
	parameters[0].buffer = st_char_get(username);

	if(!(query = stmt_get_result(stmts.select_user_stacie_salt, parameters))) {
		log_error("Failure to query user salt.");
		goto end;
	}

	if(!res_row_count(query)) {
		log_error("Failed query.");
		goto cleanup_query;
	}
	else if(res_row_count(query) > 1) {
		log_pedantic("Non-unique username.");
		goto cleanup_query;
	}

	if(!(row = res_row_next(query))) {
		log_error("Failed to retrieve row.");
		goto cleanup_query;
	}

	if(!(temp = res_field_string(row, 0))) {

		if(!(result = st_alloc_opts((MANAGED_T | JOINTED | SECURE), 0))) {
			log_error("Failed to allocate stringer.");
			goto cleanup_query;
		}

		mm_set(st_data_get(result, 0, 1));
		st_length_set(result, 0);
		goto cleanup_query;
	}
	else {

		if(!(result = hex_decode_opts(temp, (MANAGED_T | JOINTED | SECURE)))) {
			log_error("Failed to duplicate salt stringer.");
		}

	}

cleanup_temp:
	st_free(temp);
cleanup_query:
	res_table_free(query);
end:
	return result;
}
