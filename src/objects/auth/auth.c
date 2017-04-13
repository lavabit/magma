
/**
 * @file /magma/objects/auth/auth.c
 *
 * @brief The primary interface for the STACIE authentication functions.
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

	st_cleanup(auth->username, auth->seasoning.salt, auth->seasoning.nonce, auth->keys.master, auth->tokens.verification,
		auth->tokens.ephemeral, auth->legacy.key, auth->legacy.token);
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
	if (auth->tokens.verification && (st_empty(auth->seasoning.nonce = stacie_create_nonce(NULL)) ||
		st_length_get(auth->seasoning.nonce) != STACIE_NONCE_LENGTH)) {
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
int_t auth_response(auth_t *auth, stringer_t *ephemeral) {

	stringer_t *nonce = NULL;
	auth_stacie_t *stacie = NULL;

	// The protocol handler should be making sure the inputs are legal, but just in case we check them here.
	if (!auth || st_empty(auth->username, auth->seasoning.salt, auth->tokens.verification, auth->seasoning.nonce, ephemeral)) {
		log_pedantic("One of the required variables is empty. Unable to authenticate the STACIE response.");
		return -1;
	}
	// We already know the nonce value isn't NULL, but we need to copy it after we make sure auth itself isn't invalid, but
	// before we replace it.
	else if (st_length_get(auth->seasoning.nonce) != STACIE_NONCE_LENGTH) {
		log_pedantic("The nonce isn't valid. The length is incorrect.");
		return -1;
	}
	else if (st_length_get(ephemeral) != STACIE_TOKEN_LENGTH) {
		log_pedantic("The ephemeral token isn't valid. The length is incorrect.");
		return -1;
	}
	// If this fails, we return an error, but preserve the original nonce value. If it works we are guranteed to return a new nonce
	// value regardless of what happens below. We just need to cleanup the original nonce value before returning.
	else if (!(nonce = stacie_create_nonce(NULL))) {
		log_pedantic("Failed to generate a valid replacement nonce.");
		return -1;
	}

	// Technically speaking the bonus rounds value isn't required for this stage.
	if (!(stacie = auth_stacie(auth->seasoning.bonus, auth->username, NULL, auth->seasoning.salt, auth->tokens.verification, auth->seasoning.nonce)) ||
		st_empty(stacie->tokens.ephemeral) || st_length_get(stacie->tokens.ephemeral) != STACIE_TOKEN_LENGTH) {

		log_pedantic("The ephemeral token generation failed.");
		mm_move(st_data_get(auth->seasoning.nonce), st_data_get(nonce), STACIE_NONCE_LENGTH);
		auth_stacie_cleanup(stacie);
		st_free(nonce);

		return -1;
	}
	// If the tokens aren't equal then the attempt fails.
	else if (st_cmp_cs_eq(stacie->tokens.ephemeral, ephemeral)) {
#ifdef MAGMA_AUTH_PEDANTIC
		log_pedantic("The user provided incorecct login credentials for a STACIE account. { username = %.*s }", st_length_int(auth->username),
			st_char_get(auth->username));
#endif

		mm_move(st_data_get(auth->seasoning.nonce), st_data_get(nonce), STACIE_NONCE_LENGTH);
		auth_stacie_free(stacie);
		st_free(nonce);

		return 1;
	}

	// We preserve the ephemeral token, just in case the protocol layer is stateless (like HTTP), and wants to use it as a session identifier.
	mm_move(st_data_get(auth->seasoning.nonce), st_data_get(nonce), STACIE_NONCE_LENGTH);
	auth->tokens.ephemeral = st_dupe(stacie->tokens.ephemeral);
	auth_stacie_free(stacie);
	st_free(nonce);

	return 0;
}

/**
 * @brief Loads a user's authentication information and calculates the tokens for comparison.
 *
 * @note If the username and password combination is valid, but involves an account with legacy authentication
 * 			tokens, the plain text password is used to replace the legacy tokens with STACIE compliant values.
 * 			Also note the complexity of this function. It will be far simpler once support for legacy login tokens
 * 			is removed.
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
	stringer_t *legacy_hex = NULL, *salt_b64 = NULL, *verification_b64 = NULL;

	if (st_empty(username) || st_empty(password)) {
		log_pedantic("An invalid username or password was provided.");
		return -1;
	}
	else if (!output || *output) {
		log_pedantic("The pointer used to hold the outgoing auth structure was invalid.");
		return -1;
	}

	// We require at least 4 characters for the password, otheriwse the load will slow down the server.
	else if (st_length_get(password) < 4) {
		return 1;
	}

	// TODO: Differentiate between errors and invalid usernames.
	if (!(auth = auth_challenge(username))) {
		log_error("Failed to load the user challenge parameters. { username = %.*s }", st_length_int(username), st_char_get(username));
		return -1;
	}

	/************************** BEGIN LEGACY AUTHENTICATION SUPPORT LOGIC **************************/
	// If the account uses legacy hash values for authentication the legacy token will be populated.
	if (auth->legacy.token && !(legacy = auth_legacy(auth->username, password))) {
		log_pedantic("Unable to calculate the legacy hash tokens for comparison.");
		auth_free(auth);
		return -1;
	}
	// The comparison will return 0 if the two tokens are identical, so this boolean will only activate if
	// the username/password combination is incorrect.
	else if (auth->legacy.token && st_cmp_cs_eq(auth->legacy.token, legacy->token)) {
#ifdef MAGMA_AUTH_PEDANTIC
		log_pedantic("The user provided incorecct login credentials for a legacy account. { username = %.*s }", st_length_int(username), st_char_get(username));
#endif
		auth_legacy_free(legacy);
		auth_free(auth);
		return 1;
	}
	// We have a valid user login for an account with legacy credentials. Handle the upgrade before returning.
	else if (auth->legacy.token && !st_cmp_cs_eq(auth->legacy.token, legacy->token)) {

		// Assign a random salt to the user account, and use the plain text password to generate STACIE tokens before proceeding.
		if (!(auth->seasoning.salt = stacie_create_salt(NULL)) ||
			!(stacie = auth_stacie(0, auth->username, password, auth->seasoning.salt, NULL, NULL))) {
			log_pedantic("Unable to calculate the STACIE credentials.");
			auth_legacy_free(legacy);
			auth_free(auth);
			return -1;
		}

		// Store the values we need, and then free the legacy and STACIE structures.
		auth->legacy.key = st_dupe(legacy->key);
		auth->keys.master = st_dupe(stacie->keys.master);
		auth->tokens.verification = st_dupe(stacie->tokens.verification);

		// Convert the verification token, salt, and legacy values into hex so they can be safely stored in the database.
		legacy_hex = hex_encode_st(auth->legacy.token, NULL);
		salt_b64 = base64_encode_mod(auth->seasoning.salt, NULL);
		verification_b64 = base64_encode_mod(auth->tokens.verification, NULL);

		auth_legacy_free(legacy);
		auth_stacie_free(stacie);

		// Ensure all of the needed values were duplicated.
		if (st_empty(auth->legacy.key, auth->legacy.token, auth->keys.master, auth->tokens.verification, legacy_hex, salt_b64, verification_b64)) {
			log_pedantic("Unable to store the credentials in the auth structure.");
			st_cleanup(legacy_hex, salt_b64, verification_b64);
			auth_free(auth);
			return -1;
		}
		// Replace the legacy credentials in the database with STACIE values.
		else if (auth_data_update_legacy(auth->usernum, legacy_hex, salt_b64, verification_b64, auth->seasoning.bonus)) {
			log_pedantic("Unable to update the legacy credentials with the STACIE compatible equivalents.");
			st_cleanup(legacy_hex, salt_b64, verification_b64);
			auth_free(auth);
			return -1;
		}

		// Cleanup the hex equivalents and then return success.
		st_cleanup(legacy_hex, salt_b64, verification_b64);

		// Valid legacy login!
		*output = auth;
		return 0;
	}

	/************************** END LEGACY AUTHENTICATION SUPPORT LOGIC **************************/

	// Generate the STACIE tokens based on the provided inputs.
	else if (!auth->legacy.token && !(stacie = auth_stacie(0, auth->username, password, auth->seasoning.salt, NULL, NULL))) {
		log_pedantic("Unable to calculate the STACIE verification tokens for comparison.");
		auth_free(auth);
		return -1;
	}
	// The comparison will return 0 if the two tokens are identical, so this boolean will only activate if
	// the username/password combination is incorrect.
	else if (!auth->legacy.token && st_cmp_cs_eq(auth->tokens.verification, stacie->tokens.verification)) {
#ifdef MAGMA_AUTH_PEDANTIC
		log_pedantic("The user provided incorecct login credentials for a STACIE account. { username = %.*s }", st_length_int(username), st_char_get(username));
#endif
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
