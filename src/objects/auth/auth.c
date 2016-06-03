
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

	st_cleanup(auth->username, auth->seasoning.salt, auth->keys.master, auth->tokens.verification, auth->seasoning.nonce, auth->legacy.key, auth->legacy.token);
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

/**
 * @brief	Used to sanitize and normalize a username, then retrieve the authentication information for that account. For STACIE
 * 			authentications, this function creates a nonce value, which the user's client can combine with the verification token
 * 			value to derive an ephemeral login token, which may only be used once.
 *
 * @param username	the unsanitized username, which may also be an email address.
 *
 * @return
 */
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

	// Setup the nonce value if we're dealing with a STACIE authentication challenge.
	if (auth->tokens.verification && st_empty(auth->seasoning.nonce = stacie_nonce_create())) {
		log_pedantic("Failed to generate a valid nonce value.");
		auth_free(auth);
		return NULL;
	}

	return auth;
}

/**
 * @brief	Test an ephemeral token for validity. If the token is invalid, generate a different nonce for the next attempt.
 *
 * @param auth	the challenge values, including the verification token, and a nonce value.
 * @param ephemeral	the ephemeral token value provided by the user for comparison.
 *
 * @return	return -1 if an error occurs, 0 if the response is validated, and 1 if the ephemeral token is invalid.
 */
int_t auth_response(auth_t auth, stringer_t *ephemeral) {

	/// NEXT: EMPTY STUB!
	return -1;
}

/**
 * @brief Loads a user's authentication information and calculates the tokens for comparison.
 *
 * @note If the username and password combination is valid, but involves an account with legacy authentication
 * 			tokens, the plain text password is used to replace the legacy tokens with STACIE compliant values.
 *
 * @param username	the unsanitized username, provided with the login attempt.
 * @param password	the plain text password.
 * @param output	pointer location which will hold the resulting auth_t structure, but only if the inputs authenticate
 * 					successfully, and 0 is returned; also note that output must be pointing at NULL when its passed in
 * 					to avoid creating a memory leak.
 *
 * @return 			if a technical error occurs unrelated to the provided credentials, -1 will be returned, if the username
 * 					and password combination are invalid, 1 will be returned. The output parameter
 * 					is invalid, 1 will be returned, and the output will be emptied.
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
		auth_free(auth);
		return -1;
	}
	// The comparison will return 0 if the two tokens are identical, so this boolean will only activate if
	// the username/password combination is incorrect.
	else if (auth->legacy.token && st_cmp_cs_eq(auth->legacy.token, legacy->token)) {
		log_info("The user provided incorecct login credentials for a legacy account. { username = %.*s }", st_length_int(username), st_char_get(username));
		auth_legacy_free(legacy);
		auth_free(auth);
		return 1;
	}
	// We have a valid user login for an account with legacy credentials. Handle the upgrade before returning.
	else if (auth->legacy.token && !st_cmp_cs_eq(auth->legacy.token, legacy->token)) {

		// Assign a random salt to the user account, and use the plain text password to generate STACIE tokens before proceeding.
		if (!(auth->seasoning.salt = stacie_salt_create()) ||
			!(stacie = auth_stacie(0, auth->username, password, auth->seasoning.salt, NULL, NULL))) {
			log_pedantic("Unable to calculate the STACIE credentials.");
			auth_legacy_free(legacy);
			auth_free(auth);
			return -1;
		}

		// Store the values we need, and then free the legacy and STACIE structures.
		auth->legacy.key = st_dupe(legacy->key);
		auth->legacy.token = st_dupe(legacy->token);
		auth->keys.master = st_dupe(stacie->keys.master);
		auth->tokens.verification = st_dupe(stacie->tokens.verification);

		// Convert the verification token, salt, and legacy values into hex so they can be safely stored in the database.
		legacy_hex = hex_encode_st(auth->legacy.token, NULL);
		salt_hex = hex_encode_st(auth->seasoning.salt, NULL);
		verification_hex = hex_encode_st(auth->tokens.verification, NULL);

		auth_legacy_free(legacy);
		auth_stacie_free(stacie);

		// Ensure all of the needed values were duplicated.
		if (st_empty(auth->legacy.key, auth->legacy.token, auth->keys.master, auth->tokens.verification, legacy_hex, salt_hex, verification_hex)) {
			log_pedantic("Unable to store the credentials in the auth structure.");
			st_cleanup(legacy_hex, salt_hex, verification_hex);
			auth_free(auth);
			return -1;
		}
		// Replace the legacy credentials in the database with STACIE values.
		else if (auth_data_update_legacy(auth->usernum, legacy_hex, salt_hex, verification_hex, auth->seasoning.bonus)) {
			log_pedantic("Unable to update the legacy credentials with the STACIE compatible equivalents.");
			st_cleanup(legacy_hex, salt_hex, verification_hex);
			auth_free(auth);
			return -1;
		}

		// Cleanup the hex equivalents and then return success.
		st_cleanup(legacy_hex, salt_hex, verification_hex);

		// Valid legacy login!
		*output = auth;
		return 0;
	}
	// Generate the STACIE tokens based on the provided inputs.
	else if (!auth->legacy.token && !(stacie = auth_stacie(0, auth->username, password, auth->seasoning.salt, NULL, NULL))) {
		log_pedantic("Unable to calculate the STACIE verification tokens for comparison.");
		auth_free(auth);
		return -1;
	}
	// The comparison will return 0 if the two tokens are identical, so this boolean will only activate if
	// the username/password combination is incorrect.
	else if (!auth->legacy.token && st_cmp_cs_eq(auth->tokens.verification, stacie->tokens.verification)) {
		log_info("The user provided incorecct login credentials for a STACIE account. { username = %.*s }", st_length_int(username), st_char_get(username));
		auth_stacie_free(stacie);
		auth_free(auth);
		return 1;
	}
	// We have a valid user login for an account with STACIE credentials. Store required values and free the STACIE structure.
	else if (!auth->legacy.token && !st_cmp_cs_eq(auth->tokens.verification, stacie->tokens.verification)) {

		auth->keys.master = st_dupe(stacie->keys.master);
		auth_stacie_free(stacie);

		if (st_empty(auth->keys.master, auth->tokens.verification)) {
			log_pedantic("Unable to store the credentials in the auth structure.");
			auth_free(auth);
			return -1;
		}

		// Valid STACIE login!
		*output = auth;
		return 0;
	}

	// We only end up here if cosmic rays cause auth_challenge to return an auth structure that lacks a STACIE
	// verification token, and a legacy token. In other words, we should never reach this point.
	log_error("The authentication challenge returned a result without a token for comparison. { username = %.*s }", st_length_int(username), st_char_get(username));
	auth_free(auth);
	return -1;
}
