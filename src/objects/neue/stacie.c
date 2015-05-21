
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

	if(!password) {
		log_pedantic("A NULL pointer was passed in.");
		return 0;
	}

	if(!(pass_len = utf_length_get(password))) {
		log_pedantic("A 0 length password is clearly invalid.");
		return 0;
	}

	if(bonus >= MAX_HASH_NUM - 1) {
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
	stringer_t *result = NULL, *temp, *temp2, *temp3, *key = NULL, *piece;
	size_t salt_len;

	if(rounds < MIN_HASH_NUM || rounds > MAX_HASH_NUM) {
		log_pedantic("Invalid number of hash rounds.");
		return NULL;
	}

	if(!username || !password) {
		log_pedantic("A NULL pointer was passed in.");
		return NULL;
	}

	if(!(temp = salt) && !(salt = digest_sha512(username, salt))) {
		log_pedantic("SHA512 hash failed.");
		return NULL;
	}

	if((salt_len = st_length_get(salt)) < 64) {
		error = true;
	} else if(salt_len % 32) {
		error = true;
	} else if(salt_len > 1024) {
		error = true;
	} else if(salt_len != 128) {
		piece = MEMORYBUF(salt_len + 3);
		piece = mm_copy(piece, st_data_get(salt), salt_len);
		piece = mm_set(piece + salt_len, 0, 3);
		temp2 = PLACER(salt_len + 3, piece);
		key = digest_sha512(temp2, key);
		piece = mm_set(piece + salt_len + 2, 1, 1);
		temp3 = digest_sha512(temp2, temp3);
		key = st_append(key, temp3);
		st_cleanup(temp2);
		st_cleanup(temp3);
	}

	if(!temp) {
		st_cleanup(salt);
	}

}

stringer_t* stacie_hashed_key_derive(stringer_t *base, uint_t rounds, stringer_t *username, stringer_t *password, stringer_t *salt);

stringer_t* stacie_hashed_token_derive(stringer_t *base, stringer_t *username, stringer_t *salt, stringer_t *nonce);

stringer_t* stacie_realm_key_derive(stringer_t *master_key, stringer_t *realm, stringer_t *shard);

stringer_t* stacie_realm_cipher_key_derive(stringer_t *realm_key);

stringer_t* stacie_realm_init_vector_derive(stringer_t *realm_key);
