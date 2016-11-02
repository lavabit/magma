
/**
 * @file /magma/check/magma/prime/prime_objects.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma_check.h"

bool_t check_prime_keys_sthread(stringer_t *errmsg) {

	prime_key_t *holder = NULL;

	// Allocate an org key.
	if (!(holder = prime_key_alloc(PRIME_ORG_KEY))) {
		st_sprint(errmsg, "Org key allocation failed.");
		return false;
	}
	else {
		prime_key_free(holder);
	}

	// Allocate a user key.
	if (!(holder = prime_key_alloc(PRIME_USER_KEY))) {
		st_sprint(errmsg, "User key allocation failed.");
		return false;
	}
	else {
		prime_key_free(holder);
	}

	// Attempt allocation of a non-key type using the key allocation function.
	if ((holder = prime_key_alloc(PRIME_ORG_SIGNET)) || (holder = prime_key_alloc(PRIME_USER_SIGNET)) || (holder = prime_key_alloc(PRIME_USER_SIGNING_REQUEST))) {
		st_sprint(errmsg, "User key allocation failed.");
		prime_key_free(holder);
		return false;
	}

	return true;
}

bool_t check_prime_writers_sthread(stringer_t *errmsg) {

	// The minimum valid key length is 68, so lets try that.
	if (status() && st_cmp_cs_eq(prime_header_org_key_write(68, MANAGEDBUF(5)), hex_decode_st(NULLER("07a0000044"), MANAGEDBUF(5)))) {
		st_sprint(errmsg, "Invalid PRIME header for an org key.");
		return false;
	}

	else if (status() && st_cmp_cs_eq(prime_header_user_key_write(68, MANAGEDBUF(5)), hex_decode_st(NULLER("07dd000044"), MANAGEDBUF(5)))) {
		st_sprint(errmsg, "Invalid PRIME header for a user key.");
		return false;
	}

	// We don't have minimums setup yet, so we're using 1024 for the length.
	else if (status() && st_cmp_cs_eq(prime_header_org_signet_write(1024, MANAGEDBUF(5)), hex_decode_st(NULLER("06f0000400"), MANAGEDBUF(5)))) {
		st_sprint(errmsg, "Invalid PRIME header for an org signet.");
		return false;
	}
	else if (status() && st_cmp_cs_eq(prime_header_encrypted_org_key_write(1024, MANAGEDBUF(5)), hex_decode_st(NULLER("079b000400"), MANAGEDBUF(5)))) {
		st_sprint(errmsg, "Invalid PRIME header for an encrypted org key.");
		return false;
	}
	else if (status() && st_cmp_cs_eq(prime_header_user_signet_write(1024, MANAGEDBUF(5)), hex_decode_st(NULLER("06fd000400"), MANAGEDBUF(5)))) {
		st_sprint(errmsg, "Invalid PRIME header for a user signet.");
		return false;
	}
	else if (status() && st_cmp_cs_eq(prime_header_user_signing_request_write(1024, MANAGEDBUF(5)), hex_decode_st(NULLER("04bf000400"), MANAGEDBUF(5)))) {
		st_sprint(errmsg, "Invalid PRIME header for a user signing request.");
		return false;
	}
	else if (status() && st_cmp_cs_eq(prime_header_encrypted_user_key_write(1024, MANAGEDBUF(5)), hex_decode_st(NULLER("07b8000400"), MANAGEDBUF(5)))) {
		st_sprint(errmsg, "Invalid PRIME header for an encrypted user key.");
		return false;
	}
	else if (status() && st_cmp_cs_eq(prime_header_encrypted_message_write(1024, MANAGEDBUF(6)), hex_decode_st(NULLER("073700000400"), MANAGEDBUF(6)))) {
		st_sprint(errmsg, "Invalid PRIME header for an encrypted message.");
		return false;
	}

	// Try creating objects that are intentionally too small.
	if (status() && prime_header_org_key_write(34, MANAGEDBUF(5))) {
		st_sprint(errmsg, "PRIME header returned for invalid org key size.");
		return false;
	}

	else if (status() && prime_header_user_key_write(34, MANAGEDBUF(5))) {
		st_sprint(errmsg, "PRIME header returned for invalid user key size.");
		return false;
	}

	return true;
}

bool_t check_prime_unpacker_sthread(stringer_t *errmsg) {

	log_enable();
	stringer_t *key = base64_decode(NULLER("B90AAEQBIAzFqb5wsMLLwJV1uUfVecHirAQVnHZbvlDqDqkwGZwzAiAk/epj8HtmvA/VUnMC9TfWwh1veCK9Bp+uExSfeuHCug=="), MANAGEDBUF(76));


	prime_unpack(key);


	return true;
}
