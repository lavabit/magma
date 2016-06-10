
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

/**
 * @brief	A checked cleanup function which can be used free a STACIE object.
 * @note	Use this function when the STACIE object is guaranteed not to be NULL. Call the cleanup function directly if the STACIE object
 * 			pointer could potentially be NULL.
 * @see		auth_stacie_cleanup()
 *
 * @param stacie	the STACIE object which will be freed.
 * @return	This function returns no value.
 */
void auth_stacie_free(auth_stacie_t *stacie) {

	if (stacie) {
		st_cleanup(stacie->keys.master);
		st_cleanup(stacie->keys.password);
		st_cleanup(stacie->tokens.verification);
		st_cleanup(stacie->tokens.ephemeral);
		mm_free(stacie);
	}

#ifdef MAGMA_PEDANTIC
	else {
		log_pedantic("Invalid call to auth_stacie_free(). The STACIE object was set to NULL.");
	}
#endif

	return;
}

/**
 * @brief	A checked cleanup function which can be used free a STACIE object.
 * @note	Use this function when the STACIE object pointer could potentially be NULL. Call the free function directly if the STACIE object
 * 			is guaranteed not to be NULL.
 * @see		auth_stacie_free()
 *
 * @param	stacie	the STACIE object which will be freed.
 *
 * @return	This function returns no value.
 */
void auth_stacie_cleanup(auth_stacie_t *stacie) {
	if (stacie) {
		auth_stacie_free(stacie);
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
auth_stacie_t * auth_stacie(uint32_t bonus, stringer_t *username, stringer_t *password, stringer_t *salt, stringer_t *verification, stringer_t *nonce) {

	uint32_t rounds = 0;
	stringer_t *seed = NULL;
	auth_stacie_t *stacie = NULL;

	// Make sure all three required inputs are valid pointers and hold at least one character.
	if (bonus > STACIE_ROUNDS_MAX || st_empty(username) || st_empty(salt)) {
		log_pedantic("A required parameter, needed to calculate the STACIE values, is missing or invalid.");
		return NULL;
	}
	// Ensure the nonce and verification token are provided when the plain text password is unavailable.
	else if (st_empty(password) && (st_empty(verification) || st_empty(nonce))) {
		log_pedantic("The verification token should only be available if the password wasn't provided.");
		return NULL;
	}
	// Ensure the password is empty, and the nonce is provided when a verification token is passed in.
	else if (!st_empty(verification) && (!st_empty(password) || st_empty(nonce))) {
		log_pedantic("When a verification token is passed in, the password must be empty, and the nonce must be populated.");
		return NULL;
	}
	// Allocate a structure for the output.
	else if (!(stacie = auth_stacie_alloc())) {
		log_pedantic("We were unable to allocate a buffer to hold the STACIE encryption keys and authentication tokens.");
		return NULL;
	}

	// If the password isn't empty, calculate the key values, and the verification token.
	if (!st_empty(password)) {

		// We don't expressly check the length of the return values. We rely on upon the STACIE functions to check the length of inputs
		// and output values.
		if (!(rounds = stacie_rounds_calculate(password, bonus))) {
			log_pedantic("An error ocurred while trying to calculate the number of hash rounds needed to derive the encryption keys.");
			auth_stacie_free(stacie);
			return NULL;
		}
		else if (!(seed = stacie_entropy_seed_derive(rounds, password, salt))) {
			log_pedantic("An error ocurred while trying to calculate the entropy seed value.");
			auth_stacie_free(stacie);
			return NULL;
		}
		else if (!(stacie->keys.master = stacie_hashed_key_derive(seed, rounds, username, password, salt))) {
			log_pedantic("An error ocurred while trying to calculate the master key.");
			auth_stacie_free(stacie);
			return NULL;
		}
		else if (!(stacie->keys.password = stacie_hashed_key_derive(stacie->keys.master, rounds, username, password, salt))) {
			log_pedantic("An error ocurred while trying to calculate the password key.");
			auth_stacie_free(stacie);
			return NULL;
		}
		else if (!(stacie->tokens.verification = stacie_hashed_token_derive(stacie->keys.password, username, salt, NULL))) {
			log_pedantic("An error ocurred while trying to calculate the verification token.");
			auth_stacie_free(stacie);
			return NULL;
		}
	}

	// The ephemeral token is only calculated if a nonce is provided. A trenary expression ensures the provided verification token is used
	// if the password wasn't provided.
	if (!st_empty(nonce) && !(stacie->tokens.ephemeral = stacie_hashed_token_derive((st_empty(verification) ? stacie->tokens.verification : verification),
		username, salt, nonce))) {
		log_pedantic("An error ocurred while trying to calculate the verification token.");
		auth_stacie_free(stacie);
		return NULL;
	}

	// And return success.
	return stacie;
}
