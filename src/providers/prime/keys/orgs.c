
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
		if (org->signing) ed25519_free(org->signing);
		if (org->encryption) secp256k1_free(org->encryption);
		mm_free(org);
	}

	return;
}

prime_org_key_t * org_key_alloc(void) {

	prime_org_key_t *org = NULL;

	if (!(org = mm_alloc(sizeof(prime_org_key_t)))) {
		log_pedantic("PRIME organizational key allocation failed.");
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
	int_t written = 0;
	stringer_t *result = NULL;

	if (!org || !(length = org_key_length(org))) {
		log_pedantic("An invalid org key was supplied for serialization.");
		return NULL;
	}

	// See if we have a valid output buffer, or if output is NULL, allocate a buffer to hold the output.
	else if (output && (!st_valid_destination(st_opt_get(output)) || st_avail_get(output) < length)) {
		log_pedantic("An output string was supplied but it does not represent a buffer capable of holding the output.");
		return NULL;
	}
	else if (!output && !(result = st_alloc(length))) {
		log_pedantic("Could not allocate a buffer large enough to hold encoded result. { requested = %zu }", length);
		return NULL;
	}
	else if (!output) {
		output = result;
	}

	st_wipe(output);

	// Calculate the size, by writing out all the fields (minus the header) using a NULL output.
	length = st_write(NULL, prime_field_write(PRIME_ORG_KEY, 1, ED25519_KEY_PRIV_LEN, ed25519_private_get(org->signing, MANAGEDBUF(32)), MANAGEDBUF(34)),
		prime_field_write(PRIME_ORG_KEY, 3, SECP256K1_KEY_PRIV_LEN, secp256k1_private_get(org->encryption, MANAGEDBUF(32)), MANAGEDBUF(34)));

	// Then output them again into the actual output buffer, but this time include the header. This is very primitive serialization logic.
	if ((written = st_write(output, prime_header_org_key_write(length, MANAGEDBUF(5)),
		prime_field_write(PRIME_ORG_KEY, 1, ED25519_KEY_PRIV_LEN, ed25519_private_get(org->signing, MANAGEDBUF(32)), MANAGEDBUF(34)),
		prime_field_write(PRIME_ORG_KEY, 3, SECP256K1_KEY_PRIV_LEN, secp256k1_private_get(org->encryption, MANAGEDBUF(32)), MANAGEDBUF(34)))) != (length + 5)) {
		log_pedantic("The organizational key didn't serialize to the expected length. { written = %i }", written);
		st_cleanup(result);
		return NULL;
	}

	return output;
}

prime_org_key_t * org_key_set(stringer_t *org) {

	prime_field_t *field = NULL;
	prime_object_t *object = NULL;
	prime_org_key_t *result = NULL;

	if (!(object = prime_unpack(org))) {
		log_pedantic("Unable to parse the PRIME organizational key.");
		return NULL;
	}
	else if (object->type != PRIME_ORG_KEY) {
		log_pedantic("The object passed in was not an organizational key.");
		prime_object_free(object);
		return NULL;
	}

	else if (!(result = org_key_alloc())) {
		log_pedantic("Unable to allocate a PRIME organizational key.");
		prime_object_free(object);
		return NULL;
	}

	else if (!(field = prime_field_get(object, 1)) || !(result->signing = ed25519_private_set(&(field->payload)))) {
		log_pedantic("Unable to parse the PRIME organizational signing key.");
		prime_object_free(object);
		org_key_free(result);
		return NULL;
	}

	else if (!(field = prime_field_get(object, 3)) || !(result->encryption = secp256k1_private_set(&(field->payload)))) {
		log_pedantic("Unable to parse the PRIME organizational encryption key.");
		prime_object_free(object);
		org_key_free(result);
		return NULL;
	}

	// We don't need the packed object context any more.
	prime_object_free(object);

	return result;
}

stringer_t * org_encrypted_key_get(stringer_t *key, prime_org_key_t *org, stringer_t *output) {

	stringer_t *holder = NULL, *result = NULL;

	// Pack the org key before encrypting it.
	if ((holder = org_key_get(org, NULL))) {

		// Encrypt the key.
		result = aes_artifact_encrypt(key, holder, output);

		// Free the packed representation of the key.
		st_free(holder);
	}

	return result;
}

prime_org_key_t * org_encrypted_key_set(stringer_t *key, stringer_t *org) {

	stringer_t *packed = NULL;
	prime_org_key_t *result = NULL;

	// Decrypt the key.
	if ((packed = aes_artifact_decrypt(key, org, NULL))) {

		// Unpack the data into a key structure.
		result = org_key_set(packed);

		// Free the packed key data.
		st_free(packed);
	}

	return result;
}
