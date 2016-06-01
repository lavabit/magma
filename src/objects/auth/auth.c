
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

	st_cleanup(auth->username, auth->seasoning.salt, auth->keys.master, auth->tokens.verification, auth->legacy.key, auth->legacy.token);
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
	auth_stacie_t *stacie = NULL;
	auth_legacy_t *legacy = NULL;
	stringer_t *legacy_hex = NULL, *salt_hex = NULL, *verification_hex = NULL;

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
	// We have a valid user login for an account with legacy credentials.
	else if (auth->legacy.token && !st_cmp_cs_eq(auth->legacy.token, legacy->token)) {

		// Use the plain text password to convert the tokens before proceeding.
		if (!(auth->seasoning.salt = stacie_salt_create()) ||
			!(stacie = auth_stacie(0, auth->username, password, auth->seasoning.salt, NULL, NULL))) {
			log_pedantic("Unable to calculate the STACIE credentials.");
			auth_legacy_free(legacy);
			auth_free(auth);
			return -1;
		}

		// Duplicate the necessary values and then free the internal legacy and stacie structures.
		auth->legacy.key = st_dupe(legacy->key);
		auth->legacy.token = st_dupe(legacy->token);
		auth->keys.master = st_dupe(stacie->keys.master);
		auth->tokens.verification = st_dupe(stacie->tokens.verification);

		auth_legacy_free(legacy);
		auth_stacie_free(stacie);

		legacy_hex = hex_encode_st(auth->legacy.token, NULL);
		salt_hex = hex_encode_st(auth->seasoning.salt, NULL);
		verification_hex = hex_encode_st(auth->tokens.verification, NULL);

		// Ensure all of the needed values were duplicated.
		if (st_empty(auth->legacy.key) || st_empty(auth->legacy.token) || st_empty(auth->keys.master) || st_empty(auth->tokens.verification) ||
			st_empty(legacy_hex) || st_empty(salt_hex) || st_empty(verification_hex)) {
			log_pedantic("Unable to duplicate the credentials.");
			st_cleanup(legacy_hex, salt_hex, verification_hex);
			auth_free(auth);
			return -1;
		}
		// Update the database with the replacement STACIE values.
		else if (auth_data_update_legacy(auth->usernum, legacy_hex, salt_hex, verification_hex, auth->seasoning.bonus)) {
			log_pedantic("Unable to update the legacy credentials with the STACIE compatible equivalents.");
			st_cleanup(legacy_hex, salt_hex, verification_hex);
			auth_free(auth);
			return -1;
		}

		st_cleanup(legacy_hex, salt_hex, verification_hex);

	}



	// It worked. The user keys and tokens are ready for abusage.
	*output = auth;
	return 0;
}
