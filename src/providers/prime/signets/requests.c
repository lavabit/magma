
/**
 * @file /magma/providers/prime/signets/requests.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

size_t user_request_length(prime_user_signet_t *user) {

	// We know the user signing requests will be 134 or 199 bytes, at least for now. The larger size will be used for signets
	// with a chain of custody signature. The layout is a 5 byte header, 3x (or 4x) 1 byte field identifiers,
	// 2x 1 byte field lengths, and 1x 33 byte public key, 1x 32 byte public key, and 1x (or 2x) 64 byte signatures.
	size_t result = 0;

	// If an org signature is found, then this isn't a signing request, but is a signet instead.
	if (user && user->signatures.org) result = 0;
	else if (user && user->signing && user->encryption && user->signatures.custody && user->signatures.user) result = 139;
	else if (user && user->signing && user->encryption && user->signatures.user) result = 204;

	return result;
}

/**
 * @brief	Accepts a user signet signing request and signs it using the provided org key, returning a valid signet.
 */
prime_user_signet_t * user_request_sign(prime_user_signet_t *request, prime_org_key_t *org) {

	prime_user_signet_t *signet = NULL;
	stringer_t *signing = NULL, *encryption = NULL, *cryptographic = MANAGEDBUF(199);

	// Ensure a valid signing request was provided.
	if (!request || !request->encryption || !request->signing || !request->signing->type == ED25519_PUB ||
		st_empty(request->signatures.user) || st_length_get(request->signatures.user) != 64 ||
		(request->signatures.custody && st_length_get(request->signatures.custody) != 64)) {
		return NULL;
	}
	// Ensure the user structure contains the necessary organizational private key.
	else if (!org || !org->signing || !org->signing->type == ED25519_PRIV) {
		return NULL;
	}
	else if (!user_request_verify_self(request)) {
		return NULL;
	}

	else if (!(signet = mm_alloc(sizeof(prime_user_signet_t)))) {
		return NULL;
	}

	// Store the public singing, and encryption keys.
	else if (!(signing = ed25519_public_get(request->signing, MANAGEDBUF(ED25519_KEY_PUB_LEN))) ||
		!(encryption = secp256k1_public_get(request->encryption, MANAGEDBUF(SECP256K1_KEY_PUB_LEN)))) {
		log_pedantic("PRIME user signet generation failed, the public keys could not be derived from the provided signing request.");
		user_signet_free(signet);
		return NULL;
	}

	// Generate a serialized signet with the cryptographic fields for signing. Note the branching based on whether a chain
	// of custody signature is available.
	else if ((!request->signatures.custody && st_write(cryptographic, prime_field_write(PRIME_USER_SIGNET, 1, ED25519_KEY_PUB_LEN, signing, MANAGEDBUF(34)),
		prime_field_write(PRIME_USER_SIGNET, 2, SECP256K1_KEY_PUB_LEN, encryption, MANAGEDBUF(35)),
		prime_field_write(PRIME_USER_SIGNET, 5, ED25519_SIGNATURE_LEN, request->signatures.user, MANAGEDBUF(65))) != 134) ||
		(request->signatures.custody && st_write(cryptographic, prime_field_write(PRIME_USER_SIGNET, 1, ED25519_KEY_PUB_LEN, signing, MANAGEDBUF(34)),
		prime_field_write(PRIME_USER_SIGNET, 2, SECP256K1_KEY_PUB_LEN, encryption, MANAGEDBUF(35)),
		prime_field_write(PRIME_USER_SIGNET, 4, ED25519_SIGNATURE_LEN, request->signatures.custody, MANAGEDBUF(65)),
		prime_field_write(PRIME_USER_SIGNET, 5, ED25519_SIGNATURE_LEN, request->signatures.user, MANAGEDBUF(65))) != 199)) {
		log_pedantic("PRIME user signet generation failed, the cryptographic fields could not be serialized for signing.");
		user_signet_free(signet);
		return NULL;
	}

	// Generate a signature using the serialized cryptographic fields.
	else if (!(signet->signatures.org = ed25519_sign(org->signing, cryptographic, NULL))) {
		log_pedantic("PRIME user signet generation failed, the cryptographic signet signature could not be derived.");
		user_signet_free(signet);
		return NULL;
	}

	// Convert the serialized public keys into usable structures.
	else if (!(signet->signing = ed25519_public_set(signing)) || !(signet->encryption = secp256k1_public_set(encryption))) {
		log_pedantic("PRIME user signet generation failed, the serialized public keys could not be parsed.");
		user_signet_free(signet);
		return NULL;
	}

	// Duplicate the user signature, and if available, duplicate the chain of custody signature.
	else if (!(signet->signatures.user = st_dupe(request->signatures.user)) ||
		(request->signatures.custody && !(signet->signatures.custody = st_dupe(request->signatures.custody)))) {
		log_pedantic("PRIME user signet generation failed, the signatures could note be duplicated.");
		user_signet_free(signet);
		return NULL;
	}

	return signet;
}

/**
 * @brief	Derive a user signet signing request from the corresponding private key structures.
 */
prime_user_signet_t * user_request_generate(prime_user_key_t *user) {

	prime_user_signet_t *request = NULL;
	stringer_t *signing = NULL, *encryption = NULL, *cryptographic = MANAGEDBUF(69);

	// Ensure the user structure contains the necessary private keys.
	if (!user || !user->encryption || !user->signing || !user->signing->type == ED25519_PRIV) {
		return NULL;
	}
	else if (!(request = mm_alloc(sizeof(prime_user_signet_t)))) {
		return NULL;
	}

	// Store the public signing, and encryption keys.
	else if (!(signing = ed25519_public_get(user->signing, MANAGEDBUF(ED25519_KEY_PUB_LEN))) ||
		!(encryption = secp256k1_public_get(user->encryption, MANAGEDBUF(SECP256K1_KEY_PUB_LEN))) ||
		st_length_get(signing) != ED25519_KEY_PUB_LEN || st_length_get(encryption) != SECP256K1_KEY_PUB_LEN) {
		log_pedantic("PRIME user signing request generation failed, the public keys could not be derived from the provided private keys.");
		user_signet_free(request);
		return NULL;
	}

	// Generate a serialized signet with the cryptographic fields.
	else if (st_write(cryptographic, prime_field_write(PRIME_USER_SIGNING_REQUEST, 1, ED25519_KEY_PUB_LEN, signing, MANAGEDBUF(34)),
		prime_field_write(PRIME_USER_SIGNING_REQUEST, 2, SECP256K1_KEY_PUB_LEN, encryption, MANAGEDBUF(35))) != 69) {
		log_pedantic("PRIME user signing request generation failed, the cryptographic fields could not be serialized for signing.");
		user_signet_free(request);
		return NULL;
	}

	// Generate a signature using the serialized cryptographic fields.
	else if (!(request->signatures.user = ed25519_sign(user->signing, cryptographic, NULL))) {
		log_pedantic("PRIME user signing request generation failed, the cryptographic signet signature could not be derived.");
		user_signet_free(request);
		return NULL;
	}

	// Finally, convert the serialized public keys into usable structures.
	else if (!(request->signing = ed25519_public_set(signing)) || !(request->encryption = secp256k1_public_set(encryption))) {
		log_pedantic("PRIME user signing request generation failed, the serialized public keys could not be parsed.");
		user_signet_free(request);
		return NULL;
	}

	return request;
}

/**
 * @brief	Generate a user signet signing rotation request using the previous, and new user private key structures.
 */
prime_user_signet_t * user_request_rotation(prime_user_key_t *user, prime_user_key_t *previous) {

	prime_user_signet_t *request = NULL;
	stringer_t *signing = NULL, *encryption = NULL, *cryptographic = MANAGEDBUF(134);

	// Ensure the user structure contains the necessary private keys.
	if (!user || !user->encryption || !user->signing || !user->signing->type == ED25519_PRIV) {
		return NULL;
	}
	// Ensure we have the previous user private signing key. We don't need the previous encryption key so we don't
	// bother checking for it.
	else if (!previous || !previous->signing || !previous->signing->type == ED25519_PRIV) {
		return NULL;
	}
	else if (!(request = mm_alloc(sizeof(prime_user_signet_t)))) {
		return NULL;
	}

	// Store the public singing, and encryption keys.
	else if (!(signing = ed25519_public_get(user->signing, MANAGEDBUF(ED25519_KEY_PUB_LEN))) ||
		!(encryption = secp256k1_public_get(user->encryption, MANAGEDBUF(SECP256K1_KEY_PUB_LEN))) ||
		st_length_get(signing) != ED25519_KEY_PUB_LEN || st_length_get(encryption) != SECP256K1_KEY_PUB_LEN) {
		log_pedantic("PRIME user signing request generation failed, the public keys could not be derived from the provided private keys.");
		user_signet_free(request);
		return NULL;
	}

	// Generate a serialized signet with the cryptographic fields.
	else if (st_write(cryptographic, prime_field_write(PRIME_USER_SIGNING_REQUEST, 1, ED25519_KEY_PUB_LEN, signing, MANAGEDBUF(34)),
		prime_field_write(PRIME_USER_SIGNING_REQUEST, 2, SECP256K1_KEY_PUB_LEN, encryption, MANAGEDBUF(35))) != 69) {
		log_pedantic("PRIME user signing request generation failed, the cryptographic fields could not be serialized for signing.");
		user_signet_free(request);
		return NULL;
	}

	// Generate a chain of custody signature using the serialized cryptographic fields.
	else if (!(request->signatures.custody = ed25519_sign(previous->signing, cryptographic, NULL)) ||
		st_length_get(request->signatures.custody) != 64) {
		log_pedantic("PRIME user signing request generation failed, the cryptographic signet signature could not be derived.");
		user_signet_free(request);
		return NULL;
	}

	// Generate a serialized signet with the cryptographic fields, plus the chain of custody signature we just created.
	else if (st_write(cryptographic, prime_field_write(PRIME_USER_SIGNING_REQUEST, 1, ED25519_KEY_PUB_LEN, signing, MANAGEDBUF(34)),
		prime_field_write(PRIME_USER_SIGNING_REQUEST, 2, SECP256K1_KEY_PUB_LEN, encryption, MANAGEDBUF(35)),
		prime_field_write(PRIME_USER_SIGNING_REQUEST, 4, ED25519_SIGNATURE_LEN, request->signatures.custody, MANAGEDBUF(65))) != 134) {
		log_pedantic("PRIME user signing request generation failed, the cryptographic fields could not be serialized for signing.");
		user_signet_free(request);
		return NULL;
	}

	// Generate a signature using the serialized cryptographic fields.
	else if (!(request->signatures.user = ed25519_sign(user->signing, cryptographic, NULL)) ||
		st_length_get(request->signatures.user) != 64) {
		log_pedantic("PRIME user signet generation failed, the cryptographic signet signature could not be derived.");
		user_signet_free(request);
		return NULL;
	}

	// Finally, convert the serialized public keys into usable structures.
	else if (!(request->signing = ed25519_public_set(signing)) || !(request->encryption = secp256k1_public_set(encryption))) {
		log_pedantic("PRIME user signet generation failed, the serialized public keys could not be parsed.");
		user_signet_free(request);
		return NULL;
	}

	return request;
}

stringer_t * user_request_get(prime_user_signet_t *user, stringer_t *output) {

	size_t length;
	int_t written = 0;
	stringer_t *result = NULL;

	if (!user || !(length = user_request_length(user))) {
		log_pedantic("An invalid user signing request was supplied for serialization.");
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
			prime_field_write(PRIME_USER_SIGNET, 5, ED25519_SIGNATURE_LEN, user->signatures.user, MANAGEDBUF(65)));
	}
	else {
		length = st_write(NULL, prime_field_write(PRIME_USER_SIGNET, 1, ED25519_KEY_PUB_LEN, ed25519_public_get(user->signing, MANAGEDBUF(32)), MANAGEDBUF(34)),
			prime_field_write(PRIME_USER_SIGNET, 2, SECP256K1_KEY_PUB_LEN, secp256k1_public_get(user->encryption, MANAGEDBUF(33)), MANAGEDBUF(35)),
			prime_field_write(PRIME_USER_SIGNET, 4, ED25519_SIGNATURE_LEN, user->signatures.custody, MANAGEDBUF(65)),
			prime_field_write(PRIME_USER_SIGNET, 5, ED25519_SIGNATURE_LEN, user->signatures.user, MANAGEDBUF(65)));
	}

	// Write the signet into the buffer.
	if ((!user->signatures.custody && (written = st_write(output, prime_header_user_signing_request_write(length, MANAGEDBUF(5)),
		prime_field_write(PRIME_USER_SIGNET, 1, ED25519_KEY_PUB_LEN, ed25519_public_get(user->signing, MANAGEDBUF(32)), MANAGEDBUF(34)),
		prime_field_write(PRIME_USER_SIGNET, 2, SECP256K1_KEY_PUB_LEN, secp256k1_public_get(user->encryption, MANAGEDBUF(33)), MANAGEDBUF(35)),
		prime_field_write(PRIME_USER_SIGNET, 5, ED25519_SIGNATURE_LEN, user->signatures.user, MANAGEDBUF(65)))) != (length + 5)) ||
		(user->signatures.custody && (written = st_write(output, prime_header_user_signing_request_write(length, MANAGEDBUF(5)),
		prime_field_write(PRIME_USER_SIGNET, 1, ED25519_KEY_PUB_LEN, ed25519_public_get(user->signing, MANAGEDBUF(32)), MANAGEDBUF(34)),
		prime_field_write(PRIME_USER_SIGNET, 2, SECP256K1_KEY_PUB_LEN, secp256k1_public_get(user->encryption, MANAGEDBUF(33)), MANAGEDBUF(35)),
		prime_field_write(PRIME_USER_SIGNET, 4, ED25519_SIGNATURE_LEN, user->signatures.custody, MANAGEDBUF(65)),
		prime_field_write(PRIME_USER_SIGNET, 5, ED25519_SIGNATURE_LEN, user->signatures.user, MANAGEDBUF(65)))) != (length + 5))) {
		log_pedantic("The user signing request didn't serialize to the expected length. { written = %i }", written);
		st_cleanup(result);
		return NULL;
	}

	return output;
}

prime_user_signet_t * user_request_set(stringer_t *user) {

	prime_field_t *field = NULL;
	prime_object_t *object = NULL;
	prime_user_signet_t *result = NULL;

	if (!(object = prime_unpack(user))) {
		log_pedantic("Unable to parse the PRIME user signing request.");
		return NULL;
	}
	else if (object->type != PRIME_USER_SIGNING_REQUEST) {
		log_pedantic("The object passed in was not a user signing request.");
		prime_object_free(object);
		return NULL;
	}

	else if (!(result = user_signet_alloc())) {
		log_pedantic("Unable to allocate a PRIME user signing request.");
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

	// Chain of custody signature taken over the cryptographic fields, verify the length and import the ed25519 signature.
	else if (!(field = prime_field_get(object, 4)) || st_length_get(&(field->payload)) != 64 ||
		!(result->signatures.custody = st_import(pl_data_get(field->payload), pl_length_get(field->payload)))) {
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


	// We don't need the packed object context any more.
	prime_object_free(object);

	// Verify the signature.
	if (!user_request_verify_self(result)) {
		log_pedantic("The PRIME user signing request signature is invalid.");
		user_signet_free(result);
		return NULL;
	}

	return result;
}

bool_t user_request_verify_self(prime_user_signet_t *user) {

	stringer_t *holder = MANAGEDBUF(134);

	if (!user || !user->signing || !user->encryption || !user->signatures.user || st_length_get(user->signatures.user) != 64 ||
		 (user->signatures.custody && st_length_get(user->signatures.custody) != 64)) {
		return false;
	}

	// Verify the self-signature first... by generating a serialized signet to verify.
	else if ((!user->signatures.custody && st_write(holder, prime_field_write(PRIME_USER_SIGNET, 1, ED25519_KEY_PUB_LEN, ed25519_public_get(user->signing, MANAGEDBUF(32)), MANAGEDBUF(34)),
		prime_field_write(PRIME_USER_SIGNET, 2, SECP256K1_KEY_PUB_LEN, secp256k1_public_get(user->encryption, MANAGEDBUF(33)), MANAGEDBUF(35))) != 69) ||
		(user->signatures.custody && st_write(holder, prime_field_write(PRIME_USER_SIGNET, 1, ED25519_KEY_PUB_LEN, ed25519_public_get(user->signing, MANAGEDBUF(32)), MANAGEDBUF(34)),
		prime_field_write(PRIME_USER_SIGNET, 2, SECP256K1_KEY_PUB_LEN, secp256k1_public_get(user->encryption, MANAGEDBUF(33)), MANAGEDBUF(35)),
		prime_field_write(PRIME_USER_SIGNET, 4, ED25519_SIGNATURE_LEN, user->signatures.custody, MANAGEDBUF(65))) != 134)) {
		log_pedantic("PRIME user signing request verification failed. The signing request could not be serialized.");
		return false;
	}

	else if (ed25519_verify(user->signing, holder, user->signatures.user)) {
		log_pedantic("PRIME user signing request verification failed. The self-signature failed validation.");
		return false;
	}


	return true;
}

bool_t user_request_verify_chain_of_custody(prime_user_signet_t *user, prime_user_signet_t *previous) {

	stringer_t *holder = MANAGEDBUF(69);

	if (!user || !user->signing || !user->encryption || !previous || !previous->signing || !previous->encryption ||
		!user->signatures.user || st_length_get(user->signatures.user) != 64 ||
		(previous->signatures.custody && st_length_get(previous->signatures.custody) != 64) ||
		!previous->signatures.user || st_length_get(previous->signatures.user) != 64 ||
		!previous->signatures.org || st_length_get(previous->signatures.org) != 64) {
		return false;
	}

	// Verify the self-signature first... by generating a serialized signet to verify.
	else if (st_write(holder, prime_field_write(PRIME_USER_SIGNET, 1, ED25519_KEY_PUB_LEN, ed25519_public_get(user->signing, MANAGEDBUF(32)), MANAGEDBUF(34)),
		prime_field_write(PRIME_USER_SIGNET, 2, SECP256K1_KEY_PUB_LEN, secp256k1_public_get(user->encryption, MANAGEDBUF(33)), MANAGEDBUF(35))) != 69) {
		log_pedantic("PRIME user signing request verification failed. The signing request could not be serialized.");
		return false;
	}

	else if (ed25519_verify(previous->signing, holder, user->signatures.custody)) {
		log_pedantic("PRIME user signing request verification failed. The chain of custody failed validation.");
		return false;
	}


	return true;
}
