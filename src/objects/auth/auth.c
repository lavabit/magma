
/**
 * @file /magma/src/objects/auth/auth.c
 *
 * @brief The primary interface for the STACIE authentication functions.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Free an authentication object.
 * @param	auth	a pointer to the authentication object to be freed.
 * @return	This function returns no value.
 */
void auth_free(auth_t *auth) {

	if (!auth) {
		log_pedantic("Passed a NULL authentication object.");
		return;
	}

	st_cleanup(auth->username);
	st_cleanup(auth->seasoning.salt);
	st_cleanup(auth->keys.master);
	st_cleanup(auth->tokens.auth);
	st_cleanup(auth->legacy.key);
	st_cleanup(auth->legacy.token);

	mm_free(auth);
	return;
}


/**
 * @brief	Allocate an empty authentication object.
 * @return	an empty authentication object allocated off the stack.
 */
auth_t * auth_alloc(void) {

	auth_t *auth = NULL;

	if (!(auth = mm_alloc(sizeof(auth_t)))) {
		log_pedantic("Failed to allocate memory for credentials object.");
		return NULL;
	}

	mm_wipe(auth, sizeof(auth_t));
	return auth;
}

auth_t * auth_challenge(stringer_t *username) {

	auth_t *auth = NULL;

	if (st_empty(username)) {
		log_pedantic("An invalid username or password was provided.");
		return NULL;
	}

	if (!(auth = auth_alloc())) {
		log_error("Failed to allocate memory for authentication object.");
		return NULL;
	}

	// The username sanitizer will also sanitize email addresses.
	if (!(auth->username = auth_sanitize_username(username))) {
		log_error("Failed to sanitize username.");
		auth_free(auth);
		return NULL;
	}

	if (auth_data_fetch(auth)) {
		auth_free(auth);
		return NULL;
	}

	return auth;
}


/**
 * @brief Loads a user's authentication information and calculates the tokens for comparison.
 * @note If the username and password combination is valid, but involves an account with legacy authentication
 * , tokens, the plain text password is used to replace the legacy tokens with STACIE compliant values.
 * @param username	the unsanitized username, provided with the login attempt.
 * @param password	the plain text password.
 * @param output	pointer location which will hold the resulting auth_t structure; and must always
 * 					be pointing to NULL when passed in to avoid creating a memory leak.
 * @return if a technical error occurs unrelated to the provided credentials, -1 will be returned, and the
 * 			output be used to store an auth_t structure with the master	symmetric encryption key for the user
 * 			account, finally, if request is processed successfully, but the username and password combination
 * 			is invalid, 1 will be returned, and output will be emptied.
 */
int_t auth_login(stringer_t *username, stringer_t *password, auth_t **output) {

	auth_t *auth = NULL;
	auth_legacy_t *legacy = NULL;

	if (st_empty(username) || st_empty(password)) {
		log_pedantic("An invalid username or password was provided.");
		return -1;
	}
	else if (!output || *output) {
		log_pedantic("The pointer used to hold the outgoing auth structure was invalid.");
		return -1;
	}

	// TODO: Differentiate between errors and invalid usernames.
	if (!(auth = auth_challenge(username))) {
		log_error("Failed to load the user challenge parameters.");
		return -1;
	}

	// If the account uses legacy hash values for authentication the legacy token will be populated.
	if (auth->legacy.token && !(legacy = auth_legacy(auth->username, password))) {
		log_pedantic("Unable to calculate the legacy hash tokens for comparison.");
		return -1;
	}
	// The comparison will return 0 if the two tokens are identical, so this boolean will only activate if
	// the username/password combination is incorrect.
	else if (auth->legacy.token && st_cmp_cs_eq(auth->legacy.token, legacy->token)) {
		log_info("The user provided incorecct login credentials for a legacy account. { username = %.*s }",
			st_length_int(username), st_char_get(username));
		auth_legacy_free(legacy);
		auth_free(auth);
		return 1;
	}
	// We have a valid user login for a legacy account. Convert the tokens before proceeding.
	else if (auth->legacy.token && !st_cmp_cs_eq(auth->legacy.token, legacy->token)) {
		auth->seasoning.salt = stacie_salt_create();

	 //	stacie_

	}



	// It worked. The user keys and tokens are ready for abusage.
	*output = auth;
	return 0;
}
