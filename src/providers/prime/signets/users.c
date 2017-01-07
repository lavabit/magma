
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
		if (user->signature) st_free(user->signature);
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

/**
 * @brief	Derive a user signet signing request from the corresponding private key structures.
 */
prime_user_signet_t * user_signet_request_generate(prime_user_key_t *user) {

	prime_user_signet_t *request = NULL;
	stringer_t *signing = NULL, *encryption = NULL, *cryptographic = MANAGEDBUF(69);

	// Ensure the user structure contains the necessary private keys.
	if (!user || !user->encryption || !user->signing || !user->signing->type == ED25519_PRIV) {
		return NULL;
	}
	else if (!(request = mm_alloc(sizeof(prime_user_signet_t)))) {
		return NULL;
	}

	// Store the public singing, and encryption keys.
	else if (!(signing = ed25519_public_get(user->signing, NULL)) ||
		!(encryption = secp256k1_public_get(user->encryption, NULL))) {
		log_pedantic("PRIME user signet generation failed, the public keys could not be derived from the provided private keys.");
		user_signet_free(request);
		st_cleanup(signing);
		return NULL;
	}

	// Generate a serialized signet with the cryptographic fields.
	else if (st_write(cryptographic, prime_field_write(PRIME_USER_SIGNING_REQUEST, 1, ED25519_KEY_PUB_LEN, signing, MANAGEDBUF(34)),
		prime_field_write(PRIME_USER_SIGNING_REQUEST, 2, SECP256K1_KEY_PUB_LEN, encryption, MANAGEDBUF(35))) != 69) {
		log_pedantic("PRIME user signet generation failed, the serialized cryptographic signet could not be derived.");
		user_signet_free(request);
		st_free(encryption);
		st_free(signing);
		return NULL;
	}

	// Generate a signature using the serialized cryptographic fields.
	else if (!(request->signature = ed25519_sign(user->signing, cryptographic, NULL))) {
		log_pedantic("PRIME user signet generation failed, the cryptographic signet signature could not be derived.");
		user_signet_free(request);
		st_free(encryption);
		st_free(signing);
		return NULL;
	}

	// Finally, convert the serialized public keys into usable structures.
	else if (!(request->signing = ed25519_public_set(signing)) || !(request->encryption = secp256k1_public_set(encryption))) {
		log_pedantic("PRIME user signet generation failed, the serialized public keys could not be parsed.");
		user_signet_free(request);
		st_free(encryption);
		st_free(signing);
		return NULL;
	}

	// We no longer need the serialized public keys.
	st_free(encryption);
	st_free(signing);

	return request;
}
