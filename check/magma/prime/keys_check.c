
/**
 * @file /magma/check/magma/prime/keys_check.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma_check.h"

bool_t check_prime_keys_org_sthread(stringer_t *errmsg) {

	prime_t *holder = NULL;
	stringer_t *packed = NULL, *key = MANAGEDBUF(64);

	// Create a STACIE realm key.
	rand_write(key);

	// Allocate an org key.
	if (!(holder = prime_alloc(PRIME_ORG_KEY, NONE))) {
		st_sprint(errmsg, "Organizational key allocation failed.");
		return false;
	}

	prime_free(holder);

	// Generate an org key.
	if (!(holder = prime_key_generate(PRIME_ORG_KEY))) {
		st_sprint(errmsg, "Organizational key generation failed.");
		return false;
	}

	// Serialize the org key.
	else if (!(packed = prime_get(holder, BINARY, MANAGEDBUF(256)))) {
		st_sprint(errmsg, "Organizational key serialization failed.");
		prime_free(holder);
		return false;
	}

	prime_free(holder);

	// Unpack the serialized org key.
	if (!(holder = prime_set(packed, BINARY, NONE))) {
		st_sprint(errmsg, "Organizational key parsing failed.");
		return false;
	}

	// Encrypt the org key.
	else if (!(packed = prime_key_encrypt(key, holder, BINARY, MANAGEDBUF(256)))) {
		st_sprint(errmsg, "Organizational key encryption failed.");
		prime_free(holder);
		return false;
	}

	prime_free(holder);

	// Decrypt the org key.
	if (!(holder = prime_key_decrypt(key, packed, BINARY, NONE))) {
		st_sprint(errmsg, "Encrypted organizational key parsing failed.");
		return false;
	}

	prime_free(holder);

	// Perform the same checks, but this time make the functions
	// allocate memory for the output. Generate an org key.
	if (!(holder = prime_key_generate(PRIME_ORG_KEY))) {
		st_sprint(errmsg, "Organizational key generation failed.");
		return false;
	}

	// Serialize the org key.
	else if (!(packed = prime_get(holder, BINARY, NULL))) {
		st_sprint(errmsg, "Organizational key serialization failed.");
		prime_free(holder);
		return false;
	}

	prime_free(holder);

	// Unpack the serialized org key.
	if (!(holder = prime_set(packed, BINARY, NONE))) {
		st_sprint(errmsg, "Organizational key parsing failed.");
		st_free(packed);
		return false;
	}

	st_free(packed);

	// Encrypt the org key.
	if (!(packed = prime_key_encrypt(key, holder, BINARY, NULL))) {
		st_sprint(errmsg, "Organizational key encryption failed.");
		prime_free(holder);
		return false;
	}

	prime_free(holder);

	// Decrypt the org key.
	if (!(holder = prime_key_decrypt(key, packed, BINARY, NONE))) {
		st_sprint(errmsg, "Encrypted organizational key parsing failed.");
		st_free(packed);
		return false;
	}

	prime_free(holder);
	st_free(packed);

	return true;
}

bool_t check_prime_keys_user_sthread(stringer_t *errmsg) {

	prime_t *holder = NULL;
	stringer_t *packed = NULL, *key = MANAGEDBUF(64);

	// Create a STACIE realm key.
	rand_write(key);

	// Allocate a user key.
	if (!(holder = prime_alloc(PRIME_USER_KEY, NONE))) {
		st_sprint(errmsg, "User key allocation failed.");
		return false;
	}

	prime_free(holder);

	// Generate a user key.
	if (!(holder = prime_key_generate(PRIME_USER_KEY))) {
		st_sprint(errmsg, "User key generation failed.");
		return false;
	}

	// Serialize the user key.
	else if (!(packed = prime_get(holder, BINARY, MANAGEDBUF(256)))) {
		st_sprint(errmsg, "User key serialization failed.");
		prime_free(holder);
		return false;
	}

	prime_free(holder);

	// Unpack the serialized user key.
	if (!(holder = prime_set(packed, BINARY, NONE))) {
		st_sprint(errmsg, "User key parsing failed.");
		return false;
	}

	// Encrypt the user key.
	else if (!(packed = prime_key_encrypt(key, holder, BINARY, MANAGEDBUF(256)))) {
		st_sprint(errmsg, "User key encryption failed.");
		prime_free(holder);
		return false;
	}

	prime_free(holder);

	// Decrypt the user key.
	if (!(holder = prime_key_decrypt(key, packed, BINARY, NONE))) {
		st_sprint(errmsg, "Encrypted user key parsing failed.");
		return false;
	}

	prime_free(holder);

	// Perform the same checks, but this time make the functions
	// allocate memory for the output. Generate a new user key.
	if (!(holder = prime_key_generate(PRIME_USER_KEY))) {
		st_sprint(errmsg, "User key generation failed.");
		return false;
	}

	// Serialize the user key.
	else if (!(packed = prime_get(holder, BINARY, NULL))) {
		st_sprint(errmsg, "User key serialization failed.");
		prime_free(holder);
		return false;
	}

	prime_free(holder);

	// Unpack the serialized user key.
	if (!(holder = prime_set(packed, BINARY, NONE))) {
		st_sprint(errmsg, "User key parsing failed.");
		st_free(packed);
		return false;
	}

	st_free(packed);

	// Encrypt the user key.
	if (!(packed = prime_key_encrypt(key, holder, BINARY, NULL))) {
		st_sprint(errmsg, "User key encryption failed.");
		prime_free(holder);
		return false;
	}

	prime_free(holder);

	// Decrypt the user key.
	if (!(holder = prime_key_decrypt(key, packed, BINARY, NONE))) {
		st_sprint(errmsg, "Encrypted user key parsing failed.");
		st_free(packed);
		return false;
	}

	prime_free(holder);
	st_free(packed);

	return true;
}

bool_t check_prime_keys_parameters_sthread(stringer_t *errmsg) {

	prime_t *holder = NULL;

	// Attempt allocation of a non-key type using the key allocation function.
	if ((holder = prime_alloc(PRIME_ORG_KEY_ENCRYPTED, NONE)) ||
		(holder = prime_alloc(PRIME_USER_KEY_ENCRYPTED, NONE)) ||
		(holder = prime_alloc(PRIME_MESSAGE_ENCRYPTED, NONE))) {
		st_sprint(errmsg, "Allocation parameter checks failed.");
		prime_free(holder);
		return false;
	}

	return true;

}
