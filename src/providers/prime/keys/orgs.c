
/**
 * @file /magma/src/providers/prime/keys/orgs.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

void org_key_free(prime_org_key_t *org) {

	if (org) {
		if (org->signing) st_free(org->signing);
		if (org->encryption) secp256k1_free(org->encryption);
		mm_free(org);
	}

	return;
}

prime_org_key_t * org_key_alloc(void) {

	prime_org_key_t *org = NULL;

	if (!(org = mm_alloc(sizeof(prime_org_key_t)))) {
		log_pedantic("PRIME org key allocation failed.");
		return NULL;
	}

	mm_wipe(org, sizeof(prime_org_key_t));

	return org;
}

prime_org_key_t * org_key_generate(void) {

	prime_org_key_t *org = NULL;

	if (!(org = mm_alloc(sizeof(prime_org_key_t)))) {
		return NULL;
	}

	if (!(org->signing = ed25519_generate()) || !(org->encryption = secp256k1_generate())) {
		log_pedantic("PRIME org key generation failed.");
		org_key_free(org);
		return NULL;
	}

	return org;
}

size_t org_key_length(prime_org_key_t *org) {

	// We know the keys will be 76 bytes, at least for now. A 5 byte header, 2x 1 byte field identifiers, 2x 1 byte
	// field lengths, and 2x private keys each 32 bytes in length.
	size_t result = 76;
	if (!org || !org->signing || !org->encryption) result = 0;
	return result;
}

stringer_t * org_key_get(prime_org_key_t *org, stringer_t *output) {

	size_t length;
	stringer_t *result = NULL;

	if (!org || !(length = org_key_length(org))) {
		log_pedantic("An invalid org key was supplied for serialization.");
		return NULL;
	}

	// This is very primitive serialization logic.
	result = st_append(output, prime_header_org_key_write(length, MANAGEDBUF(5)));
	result = st_append(result, prime_field_write(PRIME_ORG_KEY, 1, ED25519_KEY_PRIV_LEN, ed25519_private_get(org->signing, MANAGEDBUF(32)), MANAGEDBUF(34)));
	result = st_append(result, prime_field_write(PRIME_ORG_KEY, 3, SECP256K1_KEY_PRIV_LEN, secp256k1_private_get(org->encryption, MANAGEDBUF(32)), MANAGEDBUF(34)));

	return result;
}
