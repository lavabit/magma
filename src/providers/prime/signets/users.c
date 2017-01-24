
/**
 * @file /magma/providers/prime/signets/users.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 */

#include "magma.h"

// Note that we need 1 extra byte for the 0x04 prefix taken from proposed RFC for adding EdDSA to the OpenPGP schema.
// https://tools.ietf.org/id/draft-koch-eddsa-for-openpgp.txt
void user_signet_free(prime_user_signet_t *user) {

	if (user) {
		if (user->signing) ed25519_free(user->signing);
		if (user->encryption) secp256k1_free(user->encryption);
		if (user->signatures.custody) st_free(user->signatures.custody);
		if (user->signatures.user) st_free(user->signatures.user);
		if (user->signatures.org) st_free(user->signatures.org);
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

size_t user_signet_length(prime_user_signet_t *user) {

	// We know the user signets will be 199 or 264 bytes, at least for now. The larger size will be used for signets
	// with a chain of custody signature. The layout is a 5 byte header, 4x (or 5x) 1 byte field identifiers,
	// 2x 1 byte field lengths, and 1x 33 byte public key, 1x 32 byte public key, and 2x (or 3x) 64 byte signatures.
	size_t result = 0;

	if (user && user->signing && user->encryption && user->signatures.custody && user->signatures.user && user->signatures.org) result = 204;
	else if (user && user->signing && user->encryption && user->signatures.user && user->signatures.org) result = 269;

	return result;
}

stringer_t * user_signet_get(prime_user_signet_t *user, stringer_t *output) {

	size_t length;
	int_t written = 0;
	stringer_t *result = NULL;

	if (!user || !(length = user_signet_length(user))) {
		log_pedantic("An invalid user signet was supplied for serialization.");
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

	// Find out the signet payload length.
	if (!user->signatures.custody) {
		length = st_write(NULL, prime_field_write(PRIME_USER_SIGNET, 1, ED25519_KEY_PUB_LEN, ed25519_public_get(user->signing, MANAGEDBUF(32)), MANAGEDBUF(34)),
			prime_field_write(PRIME_USER_SIGNET, 2, SECP256K1_KEY_PUB_LEN, secp256k1_public_get(user->encryption, MANAGEDBUF(33)), MANAGEDBUF(35)),
			prime_field_write(PRIME_USER_SIGNET, 5, ED25519_SIGNATURE_LEN, user->signatures.user, MANAGEDBUF(65)),
			prime_field_write(PRIME_USER_SIGNET, 6, ED25519_SIGNATURE_LEN, user->signatures.org, MANAGEDBUF(65)));
	}
	else {
		length = st_write(NULL, prime_field_write(PRIME_USER_SIGNET, 1, ED25519_KEY_PUB_LEN, ed25519_public_get(user->signing, MANAGEDBUF(32)), MANAGEDBUF(34)),
			prime_field_write(PRIME_USER_SIGNET, 2, SECP256K1_KEY_PUB_LEN, secp256k1_public_get(user->encryption, MANAGEDBUF(33)), MANAGEDBUF(35)),
			prime_field_write(PRIME_USER_SIGNET, 4, ED25519_SIGNATURE_LEN, user->signatures.custody, MANAGEDBUF(65)),
			prime_field_write(PRIME_USER_SIGNET, 5, ED25519_SIGNATURE_LEN, user->signatures.user, MANAGEDBUF(65)),
			prime_field_write(PRIME_USER_SIGNET, 6, ED25519_SIGNATURE_LEN, user->signatures.org, MANAGEDBUF(65)));
	}

	// Write the signet into the buffer.
	if ((!user->signatures.custody && (written = st_write(output, prime_header_user_signet_write(length, MANAGEDBUF(5)),
		prime_field_write(PRIME_USER_SIGNET, 1, ED25519_KEY_PUB_LEN, ed25519_public_get(user->signing, MANAGEDBUF(32)), MANAGEDBUF(34)),
		prime_field_write(PRIME_USER_SIGNET, 2, SECP256K1_KEY_PUB_LEN, secp256k1_public_get(user->encryption, MANAGEDBUF(33)), MANAGEDBUF(35)),
		prime_field_write(PRIME_USER_SIGNET, 5, ED25519_SIGNATURE_LEN, user->signatures.user, MANAGEDBUF(65)),
		prime_field_write(PRIME_USER_SIGNET, 6, ED25519_SIGNATURE_LEN, user->signatures.org, MANAGEDBUF(65)))) != (length + 5)) ||
		(user->signatures.custody && (written = st_write(output, prime_header_user_signet_write(length, MANAGEDBUF(5)),
		prime_field_write(PRIME_USER_SIGNET, 1, ED25519_KEY_PUB_LEN, ed25519_public_get(user->signing, MANAGEDBUF(32)), MANAGEDBUF(34)),
		prime_field_write(PRIME_USER_SIGNET, 2, SECP256K1_KEY_PUB_LEN, secp256k1_public_get(user->encryption, MANAGEDBUF(33)), MANAGEDBUF(35)),
		prime_field_write(PRIME_USER_SIGNET, 4, ED25519_SIGNATURE_LEN, user->signatures.custody, MANAGEDBUF(65)),
		prime_field_write(PRIME_USER_SIGNET, 5, ED25519_SIGNATURE_LEN, user->signatures.user, MANAGEDBUF(65)),
		prime_field_write(PRIME_USER_SIGNET, 6, ED25519_SIGNATURE_LEN, user->signatures.org, MANAGEDBUF(65)))) != (length + 5))) {
		log_pedantic("The user signet didn't serialize to the expected length. { written = %i }", written);
		st_cleanup(result);
		return NULL;
	}

	return output;
}

prime_user_signet_t * user_signet_set(stringer_t *user) {

	prime_field_t *field = NULL;
	prime_object_t *object = NULL;
	prime_user_signet_t *result = NULL;

	if (!(object = prime_unpack(user))) {
		log_pedantic("Unable to parse the PRIME user signet.");
		return NULL;
	}
	else if (object->type != PRIME_USER_SIGNET) {
		log_pedantic("The object passed in was not a user signet.");
		prime_object_free(object);
		return NULL;
	}

	else if (!(result = user_signet_alloc())) {
		log_pedantic("Unable to allocate a PRIME user signet.");
		prime_object_free(object);
		return NULL;
	}

	// Public signing key, verify the length and import the ed25519 public key.
	else if (!(field = prime_field_get(object, 1)) || st_length_get(&(field->payload)) != 32 ||
		!(result->signing = ed25519_public_set(&(field->payload)))) {
		log_pedantic("Unable to parse the PRIME user signing key.");
		prime_object_free(object);
		user_signet_free(result);
		return NULL;
	}

	// Public encryption key, verify the length and import the compressed secp256k1 public key.
	else if (!(field = prime_field_get(object, 2)) || st_length_get(&(field->payload)) != 33 ||
		!(result->encryption = secp256k1_public_set(&(field->payload)))) {
		log_pedantic("Unable to parse the PRIME user encryption key.");
		prime_object_free(object);
		user_signet_free(result);
		return NULL;
	}

	// Chain of custody signature, if present, is taken over the cryptographic fields. Verify the length and
	// import the ed25519 signature.
	else if ((field = prime_field_get(object, 4)) && (st_length_get(&(field->payload)) != 64 ||
		!(result->signatures.custody = st_import(pl_data_get(field->payload), pl_length_get(field->payload))))) {
		log_pedantic("Unable to parse the PRIME user signet chain of custody signature.");
		prime_object_free(object);
		user_signet_free(result);
		return NULL;
	}

	// Self-signature taken over the cryptographic fields, and if present, the custody signature. Verify the length and
	// then import the ed25519 signature.
	else if (!(field = prime_field_get(object, 5)) || st_length_get(&(field->payload)) != 64 ||
		!(result->signatures.user = st_import(pl_data_get(field->payload), pl_length_get(field->payload)))) {
		log_pedantic("Unable to parse the PRIME user signet self-signature.");
		prime_object_free(object);
		user_signet_free(result);
		return NULL;
	}

	// Organizational signature taken over the cryptographic and user signature fields. Verify the length and then
	// import the ed25519 signature.
	else if (!(field = prime_field_get(object, 6)) || st_length_get(&(field->payload)) != 64 ||
		!(result->signatures.org = st_import(pl_data_get(field->payload), pl_length_get(field->payload)))) {
		log_pedantic("Unable to parse the PRIME user signet organizational signature.");
		prime_object_free(object);
		user_signet_free(result);
		return NULL;
	}

	// We don't need the packed object context any more.
	prime_object_free(object);

	// Verify the signature.
	if (!user_signet_verify_self(result)) {
		log_pedantic("The PRIME user signet signature is invalid.");
		user_signet_free(result);
		return NULL;
	}

	return result;
}

stringer_t * user_signet_fingerprint(prime_user_signet_t *user, stringer_t *output) {

	stringer_t *holder = MANAGEDBUF(264);

	if (!user || !user->signing || !user->encryption || !user->signatures.user || !user->signatures.org ||
		st_length_get(user->signatures.user) != 64 || st_length_get(user->signatures.org) != 64 ||
		(user->signatures.custody && st_length_get(user->signatures.custody) != 64)) {
		return NULL;
	}
	else if ((!user->signatures.custody && st_write(holder, prime_field_write(PRIME_USER_SIGNET, 1, ED25519_KEY_PUB_LEN, ed25519_public_get(user->signing, MANAGEDBUF(32)), MANAGEDBUF(34)),
		prime_field_write(PRIME_USER_SIGNET, 2, SECP256K1_KEY_PUB_LEN, secp256k1_public_get(user->encryption, MANAGEDBUF(33)), MANAGEDBUF(35)),
		prime_field_write(PRIME_USER_SIGNET, 5, ED25519_SIGNATURE_LEN, user->signatures.user, MANAGEDBUF(65)),
		prime_field_write(PRIME_USER_SIGNET, 6, ED25519_SIGNATURE_LEN, user->signatures.org, MANAGEDBUF(65))) != 199) ||
		(user->signatures.custody && st_write(holder, prime_field_write(PRIME_USER_SIGNET, 1, ED25519_KEY_PUB_LEN, ed25519_public_get(user->signing, MANAGEDBUF(32)), MANAGEDBUF(34)),
		prime_field_write(PRIME_USER_SIGNET, 2, SECP256K1_KEY_PUB_LEN, secp256k1_public_get(user->encryption, MANAGEDBUF(33)), MANAGEDBUF(35)),
		prime_field_write(PRIME_USER_SIGNET, 4, ED25519_SIGNATURE_LEN, user->signatures.custody, MANAGEDBUF(65)),
		prime_field_write(PRIME_USER_SIGNET, 5, ED25519_SIGNATURE_LEN, user->signatures.user, MANAGEDBUF(65)),
		prime_field_write(PRIME_USER_SIGNET, 6, ED25519_SIGNATURE_LEN, user->signatures.org, MANAGEDBUF(65))) != 264)) {
		return NULL;
	}

	return hash_sha512(holder, output);
}

bool_t user_signet_verify_chain_of_custody(prime_user_signet_t *user, prime_user_signet_t *previous) {

	stringer_t *holder = MANAGEDBUF(69);

	if (!user || !user->signing || !user->encryption || !previous || !previous->signing || !previous->encryption ||
		!user->signatures.custody || st_length_get(user->signatures.custody) != 64 ||
		!user->signatures.user || st_length_get(user->signatures.user) != 64 ||
		!user->signatures.org || st_length_get(user->signatures.org) != 64 ||
		(previous->signatures.custody && st_length_get(previous->signatures.custody) != 64) ||
		!previous->signatures.user || st_length_get(previous->signatures.user) != 64 ||
		!previous->signatures.org || st_length_get(previous->signatures.org) != 64) {
		return false;
	}

	else if (st_write(holder, prime_field_write(PRIME_USER_SIGNET, 1, ED25519_KEY_PUB_LEN, ed25519_public_get(user->signing, MANAGEDBUF(32)), MANAGEDBUF(34)),
		prime_field_write(PRIME_USER_SIGNET, 2, SECP256K1_KEY_PUB_LEN, secp256k1_public_get(user->encryption, MANAGEDBUF(33)), MANAGEDBUF(35))) != 69) {
		log_pedantic("PRIME user signet verification failed. The signet could not be serialized.");
		return false;
	}

	else if (ed25519_verify(previous->signing, holder, user->signatures.custody)) {
		log_pedantic("PRIME user signet verification failed. The chain of custody signature failed validation.");
		return false;
	}

	return true;
}

bool_t user_signet_verify_org(prime_user_signet_t *user, prime_org_signet_t *org) {

	stringer_t *holder = MANAGEDBUF(199);

	if (!user || !user->signing || !user->encryption || !org || !org->signing ||
		!user->signatures.user || st_length_get(user->signatures.user) != 64 ||
		(user->signatures.custody && st_length_get(user->signatures.custody) != 64) ||
		!user->signatures.org || st_length_get(user->signatures.org) != 64) {
		return false;
	}

	else if ((!user->signatures.custody && st_write(holder, prime_field_write(PRIME_USER_SIGNET, 1, ED25519_KEY_PUB_LEN, ed25519_public_get(user->signing, MANAGEDBUF(32)), MANAGEDBUF(34)),
		prime_field_write(PRIME_USER_SIGNET, 2, SECP256K1_KEY_PUB_LEN, secp256k1_public_get(user->encryption, MANAGEDBUF(33)), MANAGEDBUF(35)),
		prime_field_write(PRIME_USER_SIGNET, 5, ED25519_SIGNATURE_LEN, user->signatures.user, MANAGEDBUF(65))) != 134) ||
		(user->signatures.custody && st_write(holder, prime_field_write(PRIME_USER_SIGNET, 1, ED25519_KEY_PUB_LEN, ed25519_public_get(user->signing, MANAGEDBUF(32)), MANAGEDBUF(34)),
		prime_field_write(PRIME_USER_SIGNET, 2, SECP256K1_KEY_PUB_LEN, secp256k1_public_get(user->encryption, MANAGEDBUF(33)), MANAGEDBUF(35)),
		prime_field_write(PRIME_USER_SIGNET, 4, ED25519_SIGNATURE_LEN, user->signatures.custody, MANAGEDBUF(65)),
		prime_field_write(PRIME_USER_SIGNET, 5, ED25519_SIGNATURE_LEN, user->signatures.user, MANAGEDBUF(65))) != 199)) {
		log_pedantic("PRIME user signet verification failed. The signet could not be serialized.");
		return false;
	}

	else if (ed25519_verify(org->signing, holder, user->signatures.org)) {
		log_pedantic("PRIME user signet verification failed. The organizational signature failed validation.");
		return false;
	}

	return true;
}

bool_t user_signet_verify_self(prime_user_signet_t *user) {

	stringer_t *holder = MANAGEDBUF(134);

	if (!user || !user->signing || !user->encryption || !user->signatures.user || st_length_get(user->signatures.user) != 64 ||
		 !user->signatures.org || st_length_get(user->signatures.org) != 64 ||
		 (user->signatures.custody && st_length_get(user->signatures.custody) != 64)) {
		return false;
	}

	// Verify the self-signature first... by generating a serialized signet to verify.
	else if ((!user->signatures.custody && st_write(holder, prime_field_write(PRIME_USER_SIGNET, 1, ED25519_KEY_PUB_LEN, ed25519_public_get(user->signing, MANAGEDBUF(32)), MANAGEDBUF(34)),
		prime_field_write(PRIME_USER_SIGNET, 2, SECP256K1_KEY_PUB_LEN, secp256k1_public_get(user->encryption, MANAGEDBUF(33)), MANAGEDBUF(35))) != 69) ||
		(user->signatures.custody && st_write(holder, prime_field_write(PRIME_USER_SIGNET, 1, ED25519_KEY_PUB_LEN, ed25519_public_get(user->signing, MANAGEDBUF(32)), MANAGEDBUF(34)),
		prime_field_write(PRIME_USER_SIGNET, 2, SECP256K1_KEY_PUB_LEN, secp256k1_public_get(user->encryption, MANAGEDBUF(33)), MANAGEDBUF(35)),
		prime_field_write(PRIME_USER_SIGNET, 4, ED25519_SIGNATURE_LEN, user->signatures.custody, MANAGEDBUF(65))) != 134)) {
		log_pedantic("PRIME user signet verification failed. The signet could not be serialized.");
		return false;
	}

	else if (ed25519_verify(user->signing, holder, user->signatures.user)) {
		log_pedantic("PRIME user signet verification failed. The user self-signature failed validation.");
		return false;
	}


	return true;
}
