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

/**
 * @brief	Performs the entirety of the login process and creates the meta_user_t object on success.
 * @param	username	Username specified by the user.
 * @param	password	Password specified by the user.
 * @param	protocol	The protocol that is being used to login (can be derived from the connection.)
 * @param	data		The type of data that is being retrieved for the specified user (dictated by the protocol or protocol command.)
 * @param	user		Pointer to the value which stores the location of the meta_user_t object to be allocated and populated by the user data.
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

	if(!(cred = credential_alloc_auth(username))) {
		log_error("Failed to allocate credentials.");
		state = INTERNAL_ERROR;
		goto error;
	}

	salt_res = credential_salt_fetch(cred->auth.username, &salt);

	switch(salt_res) {

	case USER_SALT:
		credential_calc_auth(cred, password, salt);
		st_free(salt);
		break;
	case USER_NO_SALT:
		credential_calc_auth(cred, password, NULL);
		break;
	case NO_USER:
		state = USER_ERROR;
		goto cleanup_cred;
	case ERROR:
		log_error("Error occurred while fetching using salt.");
		state = INTERNAL_ERROR;
		goto cleanup_cred;
	default:
		log_error("Unexpected return value");
		goto cleanup_cred;

	}

	meta_res = meta_get(cred, protocol, data, user);

	switch(meta_res) {

	case 1:
		break;
	case 0:
		state = USER_ERROR;
		break;
	case -1:
		log_error("Error occurred during creation of the user object.");
		state = INTERNAL_ERROR;
		break;

	}

	return LOGGED_IN;

cleanup_cred:
	credential_free(cred);
error:
	return state;
}
