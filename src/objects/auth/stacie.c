
/**
 * @file /magma/src/objects/auth/stacie.c
 *
 * @brief Functions needed to support/convert password hashes using the deprecated stacie strategy.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

void auth_stacie_free(auth_stacie_t *stacie) {
	if (stacie) {
		st_cleanup(stacie->keys.master);
		st_cleanup(stacie->keys.password);
		st_cleanup(stacie->tokens.auth);
		st_cleanup(stacie->tokens.ephemeral);
		mm_free(stacie);
	}
	return;
}

auth_stacie_t * auth_stacie_alloc(void) {

	auth_stacie_t *stacie = NULL;

	if ((stacie = mm_alloc(sizeof(auth_stacie_t)))) {
		mm_wipe(stacie, sizeof(auth_stacie_t));
	}

	return stacie;
}

/**
 * @brief	Calculates the STACIE key and token values using the provided inputs.
 *
 * @note	If the password is NULL, then a nonce is required, and only the ephemeral token will be populated in the
 * 			resulting auth_stacie_t structure. Likewise, if the nonce is NULL, then the password must be provided, and
 * 			the ephemeral token will be absent in the resulting auth_stacie_t structure. If the password and nonce are
 * 			both set to NULL, then an error is triggered and NULL is returned.
 *
 * @param	bonus		the number of bonus hash rounds to be applied for this particular user account.
 * @param	username	a managed string holding the normalized username.
 * @param	password	a managed string holding the plain text user password.
 * @param	salt		a managed string with the salt value for the current user.
 * @param	nonce		the randomly generated nonce value for the ephemeral token.
 *
 * @return	an auth_stacie_t structure is returned upon success, and NULL upon failure.
 **/
auth_stacie_t * auth_stacie(uint32_t bonus, stringer_t *username, stringer_t *password, stringer_t *salt, stringer_t *nonce) {

	auth_stacie_t *stacie = NULL;

	// Make sure all three required inputs are valid pointers and hold at least one character.
	if (bonus > STACIE_ROUNDS_MAX || st_empty(username) || st_empty(password) || st_empty(salt)) {
		log_pedantic("A required parameter, needed to calculate the STACIE values, is missing or invalid.");
		return NULL;
	}
	else if (st_empty(password) && st_empty(nonce)) {
		log_pedantic("If the password and nonce values are both empty, there is nothing to derive.");
		return NULL;
	}
	else if (!(stacie = auth_stacie_alloc())) {
		log_error("We were unable to allocate a buffer to hold the STACIE encryption keys and authentication tokens.");
		return NULL;
	}

	// And return success.
	return stacie;
}
