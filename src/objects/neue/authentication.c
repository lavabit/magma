/**
 * @file /magma/objects/neue/authentication.c
 *
 * @brief Functions that encapsualte entire user-level account decisions.
 *
 * Author: Ivan
 * Date: 07/10/2015
 * Revision: Original
 *
 */

#include "magma.h"

static user_state_t credential_build_full(stringer_t *username, stringer_t *password, credential_t **credential);

static user_state_t credential_auth_check(credential_t *cred);

/**
 * @brief	Performs the entirety of the login process and creates the meta_user_t object on success.
 * @param	username	Username specified by the user.
 * @param	password	Password specified by the user.
 * @param	protocol	The protocol that is being used to login (can be derived from the connection.)
 * @param	data		The type of data that is being retrieved for the specified user (dictated by the protocol or protocol command.)
 * @param	user		Pointer to the value which stores the location of the meta_user_t object to be allocated and populated by the user data.
 * @return	SUCCESS on success, INTERNAL_ERROR on non-credential related error. AUTHENTICATION_ERROR on any authentication-related error (absence of specified user or incorrect password.)
*/
user_state_t credential_login(stringer_t *username, stringer_t *password, META_PROT protocol, META_GET data, meta_user_t **user) {

	credential_t *cred;
	int_t cred_res, meta_res;
	salt_state_t salt_res;
	stringer_t *salt = NULL;
	user_state_t state;

	if(st_empty(username)) {
		log_pedantic("NULL or empty username stringer was passed in.");
		state = INTERNAL_ERROR;
		goto error;
	}

	if(st_empty(password)) {
		log_pedantic("NULL or empty password stringer was passed in.");
		state = INTERNAL_ERROR;
		goto error;
	}

	if(!user) {
		log_pedantic("NULL pointer to a user object pointer was passed in.");
		state = INTERNAL_ERROR;
		goto error;
	}

	state = credential_build_full(username, password, &cred);

	switch(state) {

	case SUCCESS:
		break;
	case INTERNAL_ERROR:
		goto error;
	case AUTHENTICATION_ERROR:
		goto error;
	default:
		state = INTERNAL_ERROR;
		goto error;

	}

	meta_res = meta_get(cred, protocol, data, user);
	credential_free(cred);

	switch(meta_res) {

	case 1:
		break;
	case 0:
		state = AUTHENTICATION_ERROR;
		goto error;
	case -1:
		log_error("Error occurred during creation of the user object.");
		state = INTERNAL_ERROR;
		goto error;

	}

	return SUCCESS;

cleanup_cred:
	credential_free(cred);
error:
	return state;
}


/**
 * @brief	Authenticates the specific username password combination.
 * @param	username	Stringer containing username.
 * @param	password	Stringer containing password.
 * @return	SUCCESS on success, INTERNAL_ERROR on non-credential related error. AUTHENTICATION_ERROR on any authentication-related error (absence of specified user or incorrect password.)
*/
user_state_t credential_authenticate(stringer_t *username, stringer_t *password) {

	MYSQL_STMT **auth_stmt;
	credential_t *cred;
	table_t *query;
	user_state_t state;

	if(st_empty(username)) {
		log_pedantic("NULL or empty username stringer was passed in.");
		state = INTERNAL_ERROR;
		goto out;
	}

	if(st_empty(password)) {
		log_pedantic("NULL or empty password stringer was passed in.");
		state = INTERNAL_ERROR;
		goto out;
	}

	state = credential_build_full(username, password, &cred);

	switch(state) {

	case SUCCESS:
		break;
	case INTERNAL_ERROR:
		log_error("Failed to build credentials object for specified username and password.");
		goto out;
	case AUTHENTICATION_ERROR:
		goto out;
	default:
		state = INTERNAL_ERROR;
		log_error("Unexpected response code.");
		goto out;

	}

	state = credential_auth_check(cred);
	credential_free(cred);

	switch(state) {

	case SUCCESS:
		break;;
	case INTERNAL_ERROR:
		log_error("Error occurred during an attempt to verify user credentials.");
		goto out;
	case AUTHENTICATION_ERROR:
		goto out;
	default:
		state = INTERNAL_ERROR;
		log_error("Unexpected response code.");
		goto out;


	}

out:
	return state;
}

/**
 * @brief	Perform the salt look up and calculate the credentials for the specified username and password.
 * @param	username	Stringer containing username.
 * @param	password	Stringer containing password.
 * @param	credential	Pointer to the value that stores the memory location of the newly allocated and populated credentials object.
 * return	SUCCESS on success, INTERNAL_ERROR on non-credential related error. AUTHENTICATION_ERROR if the specified user does not exist.
*/
static user_state_t credential_build_full(stringer_t *username, stringer_t *password, credential_t **credential) {

	credential_t *cred;
	int_t cred_res;
	salt_state_t salt_res;
	stringer_t *salt;
	user_state_t state;

	if(!(cred = credential_alloc_auth(username))) {
		log_error("Failed to allocate credentials.");
		state = INTERNAL_ERROR;
		goto error;
	}

	salt_res = credential_salt_fetch(cred->auth.username, &salt);

	switch(salt_res) {

	case USER_SALT:
		cred_res = credential_calc_auth(cred, password, salt);
		st_free(salt);
		break;
	case USER_NO_SALT:
		cred_res = credential_calc_auth(cred, password, NULL);
		break;
	case NO_USER:
		state = AUTHENTICATION_ERROR;
		goto cleanup_cred;
	case ERROR:
		log_error("Error occurred while fetching using salt.");
		state = INTERNAL_ERROR;
		goto cleanup_cred;
	default:
		log_error("Unexpected return value");
		goto cleanup_cred;

	}

	if(!cred_res) {
		log_error("Failed to calculate user credentials.");
		goto cleanup_cred;
	}

	*credential = cred;
	
	return SUCCESS;
	
cleanup_cred:
	credential_free(cred);
error:
	return state;
}

/**
 * @brief	Check the validity of specified user credentials.
 * @param	cred		Object containing user credentials.
 * @return	SUCCESS if credentials are valid, AUTHENTICATION_ERROR if credentials are invalid, INTERNAL_ERROR if some error occurred.
*/

static user_state_t credential_auth_check(credential_t *cred) {

	uint64_t num;
	user_state_t state;

	state = credential_usernum_fetch(cred, &num);
	num = 0;

	return state;
}
