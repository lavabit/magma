
/**
 * @file /magma/objects/users/stacie.c
 *
 * @brief Functions used to generate STACIE-specified tokens and keys.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Calculate total number of hash rounds for key derivation.
 * @param	password	User password.
 * @param 	bonus		Number of bonus hash rounds.
 * @return	Total number of hash rounds, 0 on failure.
 */
uint_t stacie_rounds_calculate(stringer_t *password, uint_t bonus) {

	uint_t exp, pass_len, hash_num;

	if(st_empty(password)) {
		log_pedantic("Empty password stringer.");
		return 0;
	}

	if(!(pass_len = utf_length_get(password))) {
		log_pedantic("A 0 length password is clearly invalid.");
		return 0;
	}

	if(bonus >= MAX_HASH_NUM - 2) {
		return MAX_HASH_NUM;
	}

	if(pass_len >= 24) {
		exp = 1;
	} else {
		exp = 24 - pass_len;
	}

	hash_num = ((uint_t) 2 ) << exp;
	hash_num += bonus;

	if(hash_num < 8) {
		hash_num = MIN_HASH_NUM;
	} else if(hash_num > MAX_HASH_NUM){
		hash_num = MAX_HASH_NUM;
	}

	return hash_num;
}

stringer_t* stacie_seed_extract(uint_t rounds, stringer_t *username, stringer_t *password, stringer_t *salt) {

	bool_t error = false;
	stringer_t *result = NULL, *temp, *temp2 = NULL, *temp3, *key = NULL, *piece;
	size_t salt_len;

	if(rounds < MIN_HASH_NUM || rounds > MAX_HASH_NUM) {
		log_pedantic("Invalid number of hash rounds.");
		return NULL;
	}

	if(st_empty(username) || st_empty(password)) {
		log_pedantic("A NULL pointer was passed in.");
		return NULL;
	}

	if(!(temp = salt) && !(salt = hash_sha512(username, salt))) {
		log_pedantic("SHA512 hash failed.");
		return NULL;
	}

	if((salt_len = st_length_get(salt)) < 64) {
		error = true;
	}
	else if(salt_len % 32) {
		error = true;
	}
	else if(salt_len > 1024) {
		error = true;
	}
	else if(salt_len != 128) {
		piece = MEMORYBUF(1027);
		piece = mm_copy(piece, st_data_get(salt), salt_len);
		piece = mm_set(piece + salt_len, 0, 3);
		temp2 = PLACER(piece, salt_len + 3);
		key = hash_sha512(temp2, key);
		piece = mm_set(piece + salt_len + 2, 1, 1);
		temp3 = hash_sha512(temp2, NULL);
		key = st_append(key, temp3);
		st_cleanup(temp2);
		st_cleanup(temp3);
	}
	else if(!key) {
		key = salt;
	}

	if(!error && !(result = hmac_sha512(password, key, result))) {
		log_pedantic("Could not compute desired hmac.");
		error = true;
	}

	for(uint i = 1; !error && i < rounds; ++i) {
		if(!(result = hmac_sha512(result, key, result))) {
			log_pedantic("Failed computing hmac round %d.", i);
			error = true;
		}
	}

	if(!temp) {
		st_cleanup(salt);
	}

	if(!temp2) {
		st_cleanup(key);
	}

	if(error) {
		st_cleanup(result);
		return NULL;
	}

	return result;
}

stringer_t* stacie_hashed_key_derive(stringer_t *base, uint_t rounds, stringer_t *username, stringer_t *password, stringer_t *salt) {
	return NULL;
}

stringer_t* stacie_hashed_token_derive(stringer_t *base, stringer_t *username, stringer_t *salt, stringer_t *nonce) {
	return NULL;
}

stringer_t* stacie_realm_key_derive(stringer_t *master_key, stringer_t *realm, stringer_t *shard) {
	return NULL;
}

stringer_t* stacie_realm_cipher_key_derive(stringer_t *realm_key) {
	return NULL;
}

stringer_t* stacie_realm_init_vector_derive(stringer_t *realm_key) {
	return NULL;
}
