
/**
 * @file /magma/src/providers/prime/signets/users.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

// Note that we need 1 extra byte for the 0x04 prefix taken from proposed RFC for adding EdDSA to the OpenPGP schema.
// https://tools.ietf.org/id/draft-koch-eddsa-for-openpgp.txt
void user_signet_free(prime_user_signet_t *user) {

	if (user) {
		if (user->signing) ed25519_free(user->signing);
		if (user->encryption) secp256k1_free(user->encryption);
		mm_free(user);
	}

	return;
}

prime_user_signet_t * user_signet_alloc(void) {

	prime_user_signet_t *user = NULL;

	if (!(user = mm_alloc(sizeof(prime_user_signet_t)))) {
		log_pedantic("PRIME user signet allocation failed.");
		return NULL;
	}

	mm_wipe(user, sizeof(prime_user_signet_t));

	return user;
}


