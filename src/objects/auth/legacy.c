
/**
 * @file /magma/objects/auth/legacy.c
 *
 * @brief Functions needed to support/convert password hashes using the deprecated legacy strategy.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @page legacy Removing Legacy Authentication Support
 *
 * - Database
 * 		Remove the legacy field from the Users table.
 *
 * - Config
 * 		Remove the "salt" parameter from the global config.
 *
 * - Queries
 * 		Remove/update queries which reference the legacy field.
 *
 * - Auth
 * 		Remove the legacy struct from the auth_t typedef.
 *
 * - Code
 * 		Remove the legacy routines and any code which references the auth->legacy
 * 		structure, and all references to the auth_legact_t structure.
 *
 * - Structures
 * 			auth_t (legacy)
 * 			auth_legacy_t
 */

void auth_legacy_free(auth_legacy_t *legacy) {
	if (legacy) {
		st_cleanup(legacy->key);
		st_cleanup(legacy->token);
		mm_free(legacy);
	}
	return;
}

auth_legacy_t * auth_legacy_alloc(void) {

	auth_legacy_t *legacy = NULL;

	if ((legacy = mm_alloc(sizeof(auth_legacy_t)))) {
		mm_wipe(legacy, sizeof(auth_legacy_t));
	}

	return legacy;
}

/**
 * @brief	Calculates the legacy symmetric encryption key and authentication token.
 * @param	username	a managed string holding the sanitzed username.
 * @param	password	a managed string holding the plain text user password.
 * @return	an auth_legacy_t structure is returned upon success, and NULL upon failure.
 **/
auth_legacy_t * auth_legacy(stringer_t *username, stringer_t *password) {

	auth_legacy_t *legacy = NULL;
	stringer_t *input = NULL, *intermediate = MANAGEDBUF(64);

	// Make sure all three required inputs are valid pointers and hold at least one character.
	if (st_empty(username) || st_empty(password) || st_empty(magma.secure.salt)) {
		log_error("A variable needed to calculate the legacy authentication and encryption values was invalid.");
		return NULL;
	}
	else if (!(legacy = auth_legacy_alloc())) {
		log_error("We were unable to allocate a buffer to hold the legacy hash values.");
		return NULL;
	}

	// Combine the three inputs into a single buffer.
	else if (!(input = st_merge("sss", username, magma.secure.salt, password))) {
		log_error("Unable to combine the three input values needed to calculate the legacy authentication and encryption values.");
		auth_legacy_free(legacy);
		return NULL;
	}

	// Hash the inputs together and we'll get the legacy symmetric encryption key.
	else if (!(legacy->key = st_alloc_opts(MANAGED_T | CONTIGUOUS | SECURE, 64)) || !(hash_sha512(input, legacy->key))) {
		log_error("Unable to calculate the legacy hash values.");
		auth_legacy_free(legacy);
		st_free(input);
		return NULL;
	}

	// Free and reuse the holder variable.
	st_free(input);

	// Prepare the inputs for the intermediary hash.
	if (!(input = st_merge("ss", password, legacy->key))) {
		log_error("Failed to merge the legacy authentication inputs for the intermediate hash round.");
		auth_legacy_free(legacy);
		return NULL;
	}

	// Hash the password with the output of the first round. Note that if the hash function returns NULL and overwrites
	// the intermediate string pointer, the buffer will be freed automatically because it was allocated off the stack.
	else if (!(hash_sha512(input, intermediate))) {
		log_error("Unable to calculate the legacy hash values.");
		auth_legacy_free(legacy);
		st_free(input);
		return NULL;
	}

	// Free and reuse the holder variable.
	st_free(input);

	// Prepare the inputs for the intermediary hash.
	if (!(input = st_merge("ss", password, intermediate))) {
		log_error("Failed to merge the legacy authentication inputs for the final hash round.");
		auth_legacy_free(legacy);
		return NULL;
	}

	// Hash the inputs together and we'll get the legacy authentication token.
	if (!(legacy->token = st_alloc(64)) || !(hash_sha512(input, legacy->token))) {
		log_error("Failed to merge the legacy authentication inputs for the final hash round.");
		auth_legacy_free(legacy);
		st_free(input);
		return NULL;
	}

	// Free the inputs.
	st_free(input);

	// And return success.
	return legacy;
}
