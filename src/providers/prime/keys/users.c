
/**
 * @file /magma/src/providers/prime/keys/users.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

void user_key_free(prime_user_key_t *user) {

	if (user) {
		if (user->signing) ed25519_free(user->signing);
		if (user->encryption) secp256k1_free(user->encryption);
		mm_free(user);
	}

	return;
}

prime_user_key_t * user_key_alloc(void) {

	prime_user_key_t *user = NULL;

	if (!(user = mm_alloc(sizeof(prime_user_key_t)))) {
		log_pedantic("PRIME user key allocation failed.");
		return NULL;
	}

	mm_wipe(user, sizeof(prime_user_key_t));

	return user;
}

prime_user_key_t * user_key_generate(void) {

	prime_user_key_t *user = NULL;

	if (!(user = mm_alloc(sizeof(prime_user_key_t)))) {
		return NULL;
	}

	if (!(user->signing = ed25519_generate()) || !(user->encryption = secp256k1_generate())) {
		log_pedantic("PRIME user key generation failed.");
		user_key_free(user);
		return NULL;
	}

	return user;
}

size_t user_key_length(prime_user_key_t *user) {

	// We know the keys will be 76 bytes, at least for now. A 5 byte header, 2x 1 byte field identifiers, 2x 1 byte
	// field lengths, and 2x private keys each 32 bytes in length.
	size_t result = 76;
	if (!user || !user->signing || !user->encryption) result = 0;
	return result;
}

stringer_t * user_key_get(prime_user_key_t *user, stringer_t *output) {

	size_t length;
	int_t written = 0;
	stringer_t *result = NULL;

	if (!user || !(length = user_key_length(user))) {
		log_pedantic("An invalid user key was supplied for serialization.");
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
	length = st_write(output, prime_field_write(PRIME_USER_KEY, 1, ED25519_KEY_PRIV_LEN, ed25519_private_get(user->signing, MANAGEDBUF(32)), MANAGEDBUF(34)),
		prime_field_write(PRIME_USER_KEY, 2, SECP256K1_KEY_PRIV_LEN, secp256k1_private_get(user->encryption, MANAGEDBUF(32)), MANAGEDBUF(34)));

	// Then output them again into the actual output buffer, but this time include the header. This is very primitive serialization logic.
	if ((written = st_write(output, prime_header_user_key_write(length, MANAGEDBUF(5)),
		prime_field_write(PRIME_USER_KEY, 1, ED25519_KEY_PRIV_LEN, ed25519_private_get(user->signing, MANAGEDBUF(32)), MANAGEDBUF(34)),
		prime_field_write(PRIME_USER_KEY, 2, SECP256K1_KEY_PRIV_LEN, secp256k1_private_get(user->encryption, MANAGEDBUF(32)), MANAGEDBUF(34)))) != (length + 5)) {
		log_pedantic("The user key didn't serialize to the expected length. { written = %i }", written);
		st_cleanup(result);
		return NULL;
	}

	return output;
}

prime_user_key_t * user_key_set(stringer_t *user) {

	prime_field_t *field = NULL;
	prime_object_t *object = NULL;
	prime_user_key_t *result = NULL;

	if (!(object = prime_unpack(user))) {
		log_pedantic("Unable to unpack a PRIME user key.");
		return NULL;
	}
	else if (object->type != PRIME_USER_KEY) {
		log_pedantic("The object passed in was not a user key.");
		prime_object_free(object);
		return NULL;
	}

	else if (!(result = user_key_alloc())) {
		log_pedantic("Unable to allocate a PRIME user key.");
		prime_object_free(object);
		return NULL;
	}

	else if (!(field = prime_field_get(object, 1)) || !(result->signing = ed25519_private_set(&(field->payload)))) {
		log_pedantic("Unable to parse the PRIME user signing key.");
		prime_object_free(object);
		user_key_free(result);
		return NULL;
	}

	else if (!(field = prime_field_get(object, 2)) || !(result->encryption = secp256k1_private_set(&(field->payload)))) {
		log_pedantic("Unable to parse the PRIME user encryption key.");
		prime_object_free(object);
		user_key_free(result);
		return NULL;
	}

	// We don't need the packed object context any more.
	prime_object_free(object);

	return result;
}

stringer_t * user_encrypted_key_get(stringer_t *key, prime_user_key_t *user, stringer_t *output) {

	stringer_t *holder = NULL, *result = NULL;

	// Pack the user key before encrypting it.
	if ((holder = user_key_get(user, NULL))) {

		// Encrypt the key.
		result = aes_object_encrypt(key, holder, output);

		// Free the packed representation of the key.
		st_free(holder);
	}

	return result;
}

prime_user_key_t * user_encrypted_key_set(stringer_t *key, stringer_t *user) {

	stringer_t *packed = NULL;
	prime_user_key_t *result = NULL;

	// Decrypt the key.
	if ((packed = aes_object_decrypt(key, user, NULL))) {

		// Unpack the data into a key structure.
		result = user_key_set(packed);

		// Free the packed key data.
		st_free(packed);
	}

	return result;
}
