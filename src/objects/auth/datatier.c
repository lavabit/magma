
/**
 * @file /magma/src/objects/auth/datatier.c
 *
 * @brief Functions used to interact with the database and retrieve the necessary authentication information.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"


/*
 * @brief	Replaces legacy auth tokens with STACIE compatible tokens.
 * @param	usernum	The user account number.
 * @param	legacy The legacy auth token, encoded as a hexadecimal string.
 * @param	salt The user specific salt value, encoded as a hexadecimal string.
 * @param	verification The STACIE verification token, encoded as a hexadecimal string.
 * @param	bonus	The number of bonus hash rounds.
 * @return	0 if the user information was updated correctly, or -1 if an error occurs.
 */
int_t auth_data_update_legacy(uint64_t usernum, stringer_t *legacy, stringer_t *salt, stringer_t *verification, uint32_t bonus) {

	MYSQL_BIND parameters[5];

	mm_wipe(parameters, sizeof(parameters));

	// Ensure a valid auth pointer was passed in and the required STACIE and legacy values are all present in the structure.
	if (st_empty(salt) || st_empty(verification) || bonus > STACIE_ROUNDS_MAX || usernum == 0 || st_empty(legacy)) {
		log_pedantic("The input variables were invalid.");
		return -1;
	}

	// The user specific salt value.
	parameters[0].buffer_type = MYSQL_TYPE_STRING;
	parameters[0].buffer_length = st_length_get(salt);
	parameters[0].buffer = st_char_get(salt);

	// The STACIE compatible password verification token.
	parameters[1].buffer_type = MYSQL_TYPE_STRING;
	parameters[1].buffer_length = st_length_get(verification);
	parameters[1].buffer = st_char_get(verification);

	// The number of bonus rounds to apply during the token derivation process.
	parameters[2].buffer_type = MYSQL_TYPE_LONG;
	parameters[2].buffer_length = sizeof(uint32_t);
	parameters[2].buffer = &(bonus);
	parameters[2].is_unsigned = true;

	// The user number.
	parameters[3].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[3].buffer_length = sizeof(uint64_t);
	parameters[3].buffer = &(usernum);
	parameters[3].is_unsigned = true;

	// The legacy account token.
	parameters[4].buffer_type = MYSQL_TYPE_STRING;
	parameters[4].buffer_length = st_length_get(legacy);
	parameters[4].buffer = st_char_get(legacy);

	if (stmt_exec_affected(stmts.auth_update_legacy_to_stacie, parameters) != 1) {
		log_error("Unable to replace the legacy authentication credentials with the STACIE compatible equivalents.");
		return -1;
	}

	return 0;
}

/*
 * @brief	Fetches the authentication information based on the provided username.
 * @note This function searches the Users table first, and the Mailboxes table second. This way if someone sneaks a username
 * 		into the Mailboxes table, they won't override the Users table.
 * @param	auth	The authentication object providing the username, and used to store the result.
 * @return	0 if the user information is pulled correctly, 1 if the user isn't found, and -1 if an error occurs.
 */
int_t auth_data_fetch(auth_t *auth) {

	row_t *row;
	table_t *query = NULL;
	MYSQL_BIND parameters[1];
	stringer_t *userid = NULL;

	mm_wipe(parameters, sizeof(parameters));

	// Ensure a valid auth pointer was passed in and the username is at least one character long.
	if (!auth || st_empty(auth->username)) {
		log_pedantic("The input variables were invalid.");
		return -1;
	}

	// Get the user information.
	parameters[0].buffer_type = MYSQL_TYPE_STRING;
	parameters[0].buffer_length = st_length_get(auth->username);
	parameters[0].buffer = st_char_get(auth->username);

	// First query the Users tables using the userid.
	if (!(query = stmt_get_result(stmts.auth_get_by_userid, parameters))) {
		log_error("Unable to query the database for the user authentication values.");
		return -1;
	}

	// If we don't get a hit on the userid field in the Users table...
	if (!res_row_count(query)) {

		// Free the first query so we can reuse the variables.
		res_table_free(query);

		// Check whether its possible the provided username is an email address, and return a user
		// not found if no "at" symbol.
		if (!st_search_chr(auth->username, '@', NULL)) {
			return 1;
		}

		// Try searching for the email address in the Mailboxes table.
		if (!(query = stmt_get_result(stmts.auth_get_by_address, parameters))) {
			log_error("Unable to query the database for the user authentication values.");
			return -1;
		}

		// If we still don't get a hit then the username is invalid.
		if (!res_row_count(query)) {
			res_table_free(query);
			return 1;
		}
	}

	// User names must be unique.
	if (res_row_count(query) != 1) {
		log_pedantic("More than one row was returned for a given username. { username = %.*s }", st_length_int(auth->username), st_char_get(auth->username));
		res_table_free(query);
		return -1;
	}

	// Retrieve the row.
	if (!(row = res_row_next(query))) {
		log_error("Failed to retrieve row.");
		res_table_free(query);
		return -1;
	}

	// Store the result.
	if (!(auth->usernum = res_field_uint64(row, 0))) {
		log_pedantic("Invalid user number. { username = %.*s }", st_length_int(auth->username), st_char_get(auth->username));
		res_table_free(query);
		return -1;
	}

	// Save the database username in place of the user supplied version to ensure the password hash is deterministic.
	if ((userid = res_field_string(row, 1))) {
		st_free(auth->username);
		auth->username = userid;
	}
	else {
		log_pedantic("Unable to to store the username. { username = %.*s }", st_length_int(auth->username), st_char_get(auth->username));
		res_table_free(query);
		return -1;
	}

	// Only save the STACIE salt value if it isn't NULL.
	if (res_field_length(row, 2)) {
		auth->seasoning.salt = hex_decode_st(PLACER(res_field_block(row, 2), res_field_length(row, 2)), NULL);
	}

	// Only save the STACIE auth token if the field value isn't NULL.
	if (res_field_length(row, 3)) {
		auth->tokens.verification = hex_decode_st(PLACER(res_field_block(row, 3), res_field_length(row, 3)), NULL);
	}

	auth->seasoning.bonus = res_field_uint32(row, 4);

	// Only save the legacy hash if the field value isn't NULL.
	if (res_field_length(row, 5)) {
		auth->legacy.token = hex_decode_st(PLACER(res_field_block(row, 5), res_field_length(row, 5)), NULL);
	}

	auth->status.tls = res_field_int8(row, 6);
	auth->status.locked = res_field_int8(row, 7);

	res_table_free(query);

	// If the legacy value is NULL then we must have valid STACIE values for authentication.
	if (st_empty(auth->legacy.token) && (st_empty(auth->seasoning.salt) || st_length_get(auth->seasoning.salt) != STACIE_SALT_LENGTH ||
		st_empty(auth->tokens.verification) || st_length_get(auth->tokens.verification) != STACIE_TOKEN_LENGTH || auth->seasoning.bonus > STACIE_ROUNDS_MAX)) {
		log_error("The user should have valid STACIE credentials, but the retrieved values don't look like they are the right length. { username = %.*s }",
			st_length_int(auth->username), st_char_get(auth->username));
		return -1;
	}
	else if (!st_empty(auth->legacy.token) && ((st_length_get(auth->legacy.token) != 64 || !st_empty(auth->seasoning.salt) ||
		!st_empty(auth->tokens.verification) || auth->seasoning.bonus != 0))) {
		log_error("The user should only have valid legacy credentials, but we found STACIE and legacy values. { username = %.*s }", st_length_int(auth->username), st_char_get(auth->username));
		return -1;
	}

	return 0;

}
