
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
		if (user->signing) st_free(user->signing);
		if (user->encryption) secp256k1_free(user->encryption);
		mm_free(user);
	}

	return;
}

prime_user_key_t * user_key_alloc(void) {

	prime_user_key_t *user = NULL;

	if (!(user = mm_alloc(sizeof(prime_user_key_t)))) {
		log_pedantic("Allocation of the PRIME user key failed.");
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
	stringer_t *result = NULL;

	if (!user || !(length = user_key_length(user))) {
		log_pedantic("An invalid user key was supplied for serialization.");
		return NULL;
	}

	// This is very primitive serialization logic.
	result = st_append(output, prime_header_user_key_write(length, MANAGEDBUF(5)));
	result = st_append(result, prime_field_write(PRIME_USER_KEY, 1, ED25519_KEY_PRIV_LEN, ed25519_private_get(user->signing, MANAGEDBUF(32)), MANAGEDBUF(34)));
	result = st_append(result, prime_field_write(PRIME_USER_KEY, 2, SECP256K1_KEY_PRIV_LEN, secp256k1_private_get(user->encryption, MANAGEDBUF(32)), MANAGEDBUF(34)));

	return result;
}

prime_user_key_t * user_key_set(stringer_t *key) {

	uint16_t type = 0;
	uint32_t size = 0;
	prime_user_key_t *result = NULL;

	if (prime_header_read(key,  &type,  &size) || type != PRIME_USER_KEY) {
		log_pedantic("An invalid PRIME object was passed in for parsing.");
		return NULL;
	}

	else if (!(result = user_key_alloc())) {
		log_pedantic("Unable to allocate a PRIME user key structure.");
		return NULL;
	}


	result->signing = NULL;
	return NULL;
}

prime_user_key_t * user_key_encrypted_get(prime_user_key_t *user, stringer_t *output) {

//	#error
	return NULL;
}

prime_user_key_t * user_key_encrypted_set(stringer_t *key) {

//	#error
	return NULL;
}
