
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
salt_state_t credential_salt_fetch(stringer_t *username, stringer_t **salt) {

	salt_state_t result;
	MYSQL_BIND parameters[1];
	row_t *row;
	stringer_t *temp;
	table_t *query;

	if(st_empty(username)) {
		log_pedantic("NULL username was passed in.");
		result = ERROR;
		goto error;
	}

	mm_wipe(parameters, sizeof(parameters));

	parameters[0].buffer_type = MYSQL_TYPE_STRING;
	parameters[0].buffer_length = st_length_get(username);
	parameters[0].buffer = st_char_get(username);

	if(!(query = stmt_get_result(stmts.select_user_salt, parameters))) {
		log_error("Failure to query user salt.");
		result = ERROR;
		goto error;
	}

	if(!res_row_count(query)) {
		log_pedantic("Specified user does not exist in the database.");
		result = NO_USER;
		goto cleanup_query;
	}

	if(res_row_count(query) > 1) {
		log_pedantic("Non-unique username.");
		result = ERROR;
		goto cleanup_query;
	}

	if(!(row = res_row_next(query))) {
		log_error("Failed to retrieve row.");
		result = ERROR;
		goto cleanup_query;
	}

	temp = res_field_string(row, 0);
	res_table_free(query);

	if(!temp) {
		log_pedantic("No salt found for specified user.");
		result = USER_NO_SALT;
		goto error;
	}

	*salt = hex_decode_opts(temp, (MANAGED_T | CONTIGUOUS | SECURE));
	st_free(temp);

	if(!(*salt)) {
		log_error("Failed to decode salt stringer.");
		result = ERROR;
		goto error;
	}

	return USER_SALT;

cleanup_query:
	res_table_free(query);
error:
	*salt = NULL;
	return result;
}

/**
 * @brief	Fetch the usernum of the user specified by the credentials object.
 * @param	cred		Credentials object of the user whose usernum is to be fetched.
 * @param	usernum		The place in memory where the fetched usernum will be stored.
 * @return	SUCCESS if usernum was successfully fetched, AUTHENTICATION_ERROR if user does not exist or did not authenticate, INTERNAL_ERROR if an internal error occurred.
*/
user_state_t    credential_usernum_fetch(credential_t *cred, uint64_t *usernum) {

	MYSQL_BIND parameters[2];
	MYSQL_STMT **auth_stmt;
	row_t *row;
	table_t *query;
	uint64_t row_count;
	user_state_t state;

	if(!cred) {
		log_pedantic("NULL credentials object was passed in.");
		state = INTERNAL_ERROR;
		goto error;
	}

	if(cred->type != CREDENTIAL_AUTH) {
		log_error("Credentials passed are not of authentication type.");
		state = INTERNAL_ERROR;
		goto error;
	}

	if(st_empty(cred->auth.username)) {
		log_error("Credentials have a NULL or empty username stringer.");
		state = INTERNAL_ERROR;
		goto error;
	}

	if(st_empty(cred->auth.password)) {
		log_error("Credentials have a NULL or empty hashed password token stringer.");
		state = INTERNAL_ERROR;
		goto error;
	}

	mm_wipe(parameters, sizeof(parameters));

	switch(cred->authentication) {

	case LEGACY:
		auth_stmt = stmts.select_usernum_auth_legacy;
		break;
	case STACIE:
		auth_stmt = stmts.select_usernum_auth_stacie;
		break;
	default:
		log_error("Invalid authentication type.");
		state = INTERNAL_ERROR;
		goto error;

	}

	parameters[0].buffer_type = MYSQL_TYPE_STRING;
	parameters[0].buffer = st_char_get(cred->auth.username);
	parameters[0].buffer_length = st_length_get(cred->auth.username);

	parameters[1].buffer_type = MYSQL_TYPE_STRING;
	parameters[1].buffer = (chr_t *)st_char_get(cred->auth.password);
	parameters[1].buffer_length = st_length_get(cred->auth.password);

	if(!(query = stmt_get_result(auth_stmt, parameters))) {
		log_error("Database query failed.");
		state = INTERNAL_ERROR;
		goto error;
	}

	row_count = res_row_count(query);

	if(row_count > 1) {
		log_error("Multiple accounts returned with the same name and password hash");
		state = INTERNAL_ERROR;
		goto cleanup_query;
	} else if(!row_count){
		state = AUTHENTICATION_ERROR;
		goto cleanup_query;
	}

	if ((row = res_row_next(query))) {
		log_error("Failed to select row from queried data.");
		state = INTERNAL_ERROR;
		goto cleanup_query;
	}

	*usernum = res_field_uint64(row, 0);
	res_table_free(query);

	if(!(*usernum)) {
		log_error("Failed to retrieve user num from row.");
		state = INTERNAL_ERROR;
		goto error;
	}

	return SUCCESS;

cleanup_query:
	res_table_free(query);
error:
	return state;
}
