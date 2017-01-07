
/**
 * @file /magma/src/providers/prime/signets/orgs.c
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
void org_signet_free(prime_org_signet_t *org) {

	if (org) {
		if (org->signing) ed25519_free(org->signing);
		if (org->encryption) secp256k1_free(org->encryption);
		if (org->signature) st_free(org->signature);
		mm_free(org);
	}

	return;
}

prime_org_signet_t * org_signet_alloc(void) {

	prime_org_signet_t *org = NULL;

	if (!(org = mm_alloc(sizeof(prime_org_signet_t)))) {
		log_pedantic("PRIME organizational signet allocation failed.");
		return NULL;
	}

	mm_wipe(org, sizeof(prime_org_signet_t));

	return org;
}

/**
 * @brief	Derive an organizational signet from the corresponding private key structures.
 */
prime_org_signet_t * org_signet_generate(prime_org_key_t *org) {

	prime_org_signet_t *signet = NULL;
	stringer_t *signing = NULL, *encryption = NULL, *cryptographic = MANAGEDBUF(69);

	// Ensure the org structure contains the necessary private keys.
	if (!org || !org->encryption || !org->signing || !org->signing->type == ED25519_PRIV) {
		return NULL;
	}
	else if (!(signet = mm_alloc(sizeof(prime_org_signet_t)))) {
		return NULL;
	}
	// Store the public singing, and encryption keys.
	else if (!(signing = ed25519_public_get(org->signing, NULL)) ||
		!(encryption = secp256k1_public_get(org->encryption, NULL))) {
		log_pedantic("PRIME organizational signet generation failed, the public keys could not be derived from the provided private keys.");
		org_signet_free(signet);
		st_cleanup(signing);
		return NULL;
	}

	// Generate a serialized signet with the cryptographic fields.
	else if (st_write(cryptographic, prime_field_write(PRIME_ORG_SIGNET, 1, ED25519_KEY_PUB_LEN, signing, MANAGEDBUF(34)),
		prime_field_write(PRIME_ORG_SIGNET, 3, SECP256K1_KEY_PUB_LEN, encryption, MANAGEDBUF(35))) != 69) {
		log_pedantic("PRIME organizational signet generation failed, the serialized cryptographic signet could not be derived.");
		org_signet_free(signet);
		st_free(encryption);
		st_free(signing);
		return NULL;
	}

	// Generate a signature using the serialized cryptographic fields.
	else if (!(signet->signature = ed25519_sign(org->signing, cryptographic, NULL))) {
		log_pedantic("PRIME organizational signet generation failed, the cryptographic signet signature could not be derived.");
		org_signet_free(signet);
		st_free(encryption);
		st_free(signing);
		return NULL;
	}

	// Finally, convert the serialized public keys into usable structures.
	else if (!(signet->signing = ed25519_public_set(signing)) || !(signet->encryption = secp256k1_public_set(encryption))) {
		log_pedantic("PRIME organizational signet generation failed, the serialized public keys could not be parsed.");
		org_signet_free(signet);
		st_free(encryption);
		st_free(signing);
		return NULL;
	}

	// We no longer need the serialized public keys.
	st_free(encryption);
	st_free(signing);

	return signet;
}
