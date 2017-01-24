
/**
 * @file /magma/providers/prime/signets/orgs.c
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

size_t org_signet_length(prime_org_signet_t *org) {

	// We know the org signets will be 76 bytes, at least for now. A 5 byte header, 3x 1 byte field identifiers,
	// 2x 1 byte field lengths, and 1x 33 byte public key, 1x 32 byte public key, and 1x 64 byte signature.
	size_t result = 139;
	if (!org || !org->signing || !org->encryption || !org->signature) result = 0;
	return result;
}

stringer_t * org_signet_get(prime_org_signet_t *org, stringer_t *output) {

	size_t length;
	int_t written = 0;
	stringer_t *result = NULL;

	if (!org || !(length = org_signet_length(org))) {
		log_pedantic("An invalid org signet was supplied for serialization.");
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
	length = st_write(NULL, prime_field_write(PRIME_ORG_SIGNET, 1, ED25519_KEY_PUB_LEN, ed25519_public_get(org->signing, MANAGEDBUF(32)), MANAGEDBUF(34)),
		prime_field_write(PRIME_ORG_SIGNET, 3, SECP256K1_KEY_PUB_LEN, secp256k1_public_get(org->encryption, MANAGEDBUF(33)), MANAGEDBUF(35)),
		prime_field_write(PRIME_ORG_SIGNET, 4, ED25519_SIGNATURE_LEN, org->signature, MANAGEDBUF(65)));

	// Then output them again into the actual output buffer, but this time include the header. This is very primitive serialization logic.
	if ((written = st_write(output, prime_header_org_signet_write(length, MANAGEDBUF(5)),
		prime_field_write(PRIME_ORG_SIGNET, 1, ED25519_KEY_PUB_LEN, ed25519_public_get(org->signing, MANAGEDBUF(32)), MANAGEDBUF(34)),
		prime_field_write(PRIME_ORG_SIGNET, 3, SECP256K1_KEY_PUB_LEN, secp256k1_public_get(org->encryption, MANAGEDBUF(33)), MANAGEDBUF(35)),
		prime_field_write(PRIME_ORG_SIGNET, 4, ED25519_SIGNATURE_LEN, org->signature, MANAGEDBUF(65)))) != (length + 5)) {
		log_pedantic("The organizational signet didn't serialize to the expected length. { written = %i }", written);
		st_cleanup(result);
		return NULL;
	}

	return output;
}

prime_org_signet_t * org_signet_set(stringer_t *org) {

	prime_field_t *field = NULL;
	prime_object_t *object = NULL;
	prime_org_signet_t *result = NULL;

	if (!(object = prime_unpack(org))) {
		log_pedantic("Unable to parse the PRIME organizational signet.");
		return NULL;
	}
	else if (object->type != PRIME_ORG_SIGNET) {
		log_pedantic("The object passed in was not an organizational signet.");
		prime_object_free(object);
		return NULL;
	}

	else if (!(result = org_signet_alloc())) {
		log_pedantic("Unable to allocate a PRIME organizational signet.");
		prime_object_free(object);
		return NULL;
	}

	// Public signing key, verify the length and import the ed25519 public key.
	else if (!(field = prime_field_get(object, 1)) || st_length_get(&(field->payload)) != 32 ||
		!(result->signing = ed25519_public_set(&(field->payload)))) {
		log_pedantic("Unable to parse the PRIME organizational signing key.");
		prime_object_free(object);
		org_signet_free(result);
		return NULL;
	}

	// Public encryption key, verify the length and import the compressed secp256k1 public key.
	else if (!(field = prime_field_get(object, 3)) || st_length_get(&(field->payload)) != 33 ||
		!(result->encryption = secp256k1_public_set(&(field->payload)))) {
		log_pedantic("Unable to parse the PRIME organizational encryption key.");
		prime_object_free(object);
		org_signet_free(result);
		return NULL;
	}

	// Self signature taken over the cryptographic fields, verify the length and import the ed25519 signature.
	else if (!(field = prime_field_get(object, 4)) || st_length_get(&(field->payload)) != 64 ||
		!(result->signature = st_import(pl_data_get(field->payload), pl_length_get(field->payload)))) {
		log_pedantic("Unable to parse the PRIME organizational signet signature.");
		prime_object_free(object);
		org_signet_free(result);
		return NULL;
	}

	// We don't need the packed object context any more.
	prime_object_free(object);

	// Verify the signature.
	if (!org_signet_verify(result)) {
		log_pedantic("The PRIME organizational signet signature is invalid.");
		org_signet_free(result);
		return NULL;
	}

	return result;
}

bool_t org_signet_verify(prime_org_signet_t *org) {

	stringer_t *holder = MANAGEDBUF(69);

	if (!org || !org->signing || !org->encryption || !org->signature || st_length_get(org->signature) != 64) {
		return false;
	}
	else if (st_write(holder, prime_field_write(PRIME_ORG_SIGNET, 1, ED25519_KEY_PUB_LEN, ed25519_public_get(org->signing, MANAGEDBUF(32)), MANAGEDBUF(34)),
		prime_field_write(PRIME_ORG_SIGNET, 3, SECP256K1_KEY_PUB_LEN, secp256k1_public_get(org->encryption, MANAGEDBUF(33)), MANAGEDBUF(35))) != 69) {
		return false;
	}
	else if (ed25519_verify(org->signing, holder, org->signature)) {
		return false;
	}

	return true;
}

stringer_t * org_signet_fingerprint(prime_org_signet_t *org, stringer_t *output) {

	stringer_t *holder = MANAGEDBUF(134);

	if (!org || !org->signing || !org->encryption || !org->signature || st_length_get(org->signature) != 64) {
		return false;
	}
	else if (st_write(holder, prime_field_write(PRIME_ORG_SIGNET, 1, ED25519_KEY_PUB_LEN, ed25519_public_get(org->signing, MANAGEDBUF(32)), MANAGEDBUF(34)),
		prime_field_write(PRIME_ORG_SIGNET, 3, SECP256K1_KEY_PUB_LEN, secp256k1_public_get(org->encryption, MANAGEDBUF(33)), MANAGEDBUF(35)),
		prime_field_write(PRIME_ORG_SIGNET, 4, ED25519_SIGNATURE_LEN, org->signature, MANAGEDBUF(65))) != 134) {
		return false;
	}

	return hash_sha512(holder, output);
}
