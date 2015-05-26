
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

	uint_t pass_len, hash_num;

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
		hash_num = 2;
	} else {
		hash_num = ((uint_t) 2) << (23 - pass_len);
	}

	hash_num += bonus;

	if(hash_num < 8) {
		hash_num = MIN_HASH_NUM;
	} else if(hash_num > MAX_HASH_NUM){
		hash_num = MAX_HASH_NUM;
	}

	return hash_num;
}

/*
 * @brief	Extract the seed from user password.
 * @param	rounds		Number of hashing rounds.
 * @param	username	User username.
 * @param	password	User password.
 * @salt	salt		User specific salt (optional).
 * @return	Stringer with user's entropy seed.
 */
stringer_t * stacie_seed_extract(uint_t rounds, stringer_t *username, stringer_t *password, stringer_t *salt) {

	bool_t error = false;
	stringer_t *result = NULL, *temp, *temp2 = NULL, *temp3 = NULL, *key = NULL, *piece;
	size_t salt_len;

	if(rounds < MIN_HASH_NUM || rounds > MAX_HASH_NUM) {
		log_pedantic("Invalid number of hash rounds.");
		return NULL;
	}
	else if(st_empty(username) || st_empty(password)) {
		log_pedantic("A NULL pointer was passed in.");
		return NULL;
	}
	else if(!(temp = salt) && !(salt = hash_sha512(username, salt))) {
		log_pedantic("SHA512 hash failed.");
		return NULL;
	}
	else if((salt_len = st_length_get(salt)) < 64) {
		log_error("Salt is too short (must be at least 64 octets.)");
		return NULL;
	}
	else if(!(result = st_alloc_opts((MANAGED_T | JOINTED | HEAP | SECURE), 64))) {
		log_pedantic("Could not allocate secure stringer for result.");
		return NULL;
	}
	
	if(salt_len % 32) {
		log_error("Salt should be aligned to 32 octet boundary.");
	}

	if(salt_len > 1024) {
		log_error("Salt should not exceed 1024 octets.");
	}

	if(salt_len != 128) {
		piece = MEMORYBUF(1027);
		piece = mm_copy(piece, st_data_get(salt), salt_len);
		piece = mm_set(piece + salt_len, 0, 3);
		temp2 = PLACER(piece, salt_len + 3);
		temp3 = hash_sha512(temp2, NULL);
		key = st_append(key, temp3);
		piece = mm_set(piece + salt_len + 2, 1, 1);
		temp3 = hash_sha512(temp2, NULL);
		key = st_append(key, temp3);
	}
	else if(!key) {
		key = salt;
	}

	if(!(result = hmac_sha512(password, key, result))) {
		log_pedantic("Could not compute desired hmac.");
		error = true;
	}

	st_cleanup(temp3);

	for(uint i = 1; !error && i < rounds; ++i) {
		if(!(result = hmac_sha512(result, key, result))) {
			log_pedantic("Failed computing hmac round %d.", i);
			error = true;
		}
	}

	if(!temp) {
		st_cleanup(salt);
	}

	if(temp2) {
		st_cleanup(key);
	}

	if(error) {
		st_cleanup(result);
		return NULL;
	}

	return result;
}

/*
 * @brief 	Derive the hashed key from a seed and user credentials.
 * @param	base		Entropy seed for master key derivation, master key for password key derivation.
 * @param	username	Username.
 * @param	password	User password.
 * @param	salt		Optional random salt value at least 64 bytes.
 * @return	Pointer to derived hashed key.
*/
stringer_t * stacie_hashed_key_derive(stringer_t *base, uint_t rounds, stringer_t *username, stringer_t *password, stringer_t *salt) {

	size_t salt_len = 0;
	stringer_t *result, *hash_input, *count;

	if(st_empty(username) || st_empty(password)) {
		log_pedantic("Empty username or password were passed in.");
		return NULL;
	}
	else if(rounds < MIN_HASH_NUM || rounds > MAX_HASH_NUM) {
		log_pedantic("Invalid hash round number.");
		return NULL;
	}
	else if(!st_empty(salt) && ((salt_len = st_length_get(salt)) < 64)) {
		log_pedantic("The salt length must be at least 64 bytes.");
		return NULL;
	}
	else if(st_empty(base) || st_length_get(base) != 64) {
		log_pedantic("The base seed was either empty or of invalid size.");
		return NULL;
	}
	else if(!(result = st_alloc_opts((MANAGED_T | JOINTED | HEAP | SECURE), 64))) {
		log_pedantic("Failed to allocate secure stringer for hashed key.");
		return NULL;
	}
	else if(!(hash_input = st_alloc_opts((MANAGED_T | JOINTED | HEAP | SECURE), 64 
		+ st_length_get(base) + st_length_get(username) 
		+ salt_len + st_length_get(password) + 3))) {
		log_pedantic("Failed to allocate secure stringer for hash input.");
	}

	for(uint_t i = 0; i < rounds; ++i) {
		hash_input = st_append(hash_input, result);
		st_wipe(result);
		hash_input = st_append(hash_input, base);
		hash_input = st_append(hash_input, username);
		if(salt_len) {
			hash_input = st_append(hash_input, salt);
		}
		hash_input = st_append(hash_input, password);
		count = uint24_put_no(i);
		hash_input = st_append(hash_input, count);
		result = hash_sha512(hash_input, result);
		st_wipe(hash_input);
	}

	return result;
}

stringer_t * stacie_hashed_token_derive(stringer_t *base, stringer_t *username, stringer_t *salt, stringer_t *nonce) {

	size_t salt_len = 0, nonce_len = 0;
	stringer_t *result, *hash_input, *count;

	if(st_empty(base) || (st_length_get(base) != 64)) {
		log_pedantic("An empty or invalid seed was passed.");
		return NULL;
	}
	else if(st_empty(username)) {
		log_pedantic("Empty username was passed in.");
		return NULL;
	}
	else if(!st_empty(salt) && ((salt_len = st_length_get(salt)) < 64)) {
		log_pedantic("The salt length must be at least 64 bytes.");
		return NULL;
	}
	else if(!st_empty(nonce) && ((nonce_len = st_length_get(nonce)) < 64)) {
		log_pedantic("The nonce length must be at least 64 bytes.");
		return NULL;
	}
	else if(!(result = st_alloc_opts((MANAGED_T | JOINTED | HEAP | SECURE), 64))) {
		log_pedantic("Failed to allocate secure stringer for result.");
		return NULL;
	}
	else if(!(hash_input = st_alloc_opts((MANAGED_T | JOINTED | HEAP | SECURE), 64 +
		+ st_length_get(base) + st_length_get(username) +
		+ salt_len + nonce_len + 3))) {
		log_pedantic("Failed to allocate secure stringer for hashed token.");
		return NULL;
	}

	for(uint_t i = 0; i < MIN_HASH_NUM; ++i) {
		hash_input = st_append(hash_input, result);
		st_wipe(result);
		hash_input = st_append(hash_input, base);
		hash_input = st_append(hash_input, username);
		if(salt_len) {
			hash_input = st_append(hash_input, salt);
		}
		if(nonce_len) {
			hash_input = st_append(hash_input, nonce);
		}
		count = uint24_put_no(i);
		hash_input = st_append(hash_input, count);
		result = hash_sha512(hash_input, result);
		st_wipe(hash_input);
	}

	return result;
}

stringer_t * stacie_realm_key_derive(stringer_t *master_key, stringer_t *realm, stringer_t *shard) {

	stringer_t *result, *hash_input;

	if(st_empty(master_key) || (st_length_get(master_key) != 64)) {
		log_pedantic("An empty or invalid master key was passed in.");
		return NULL;
	}
	else if(st_empty(realm)) {
		log_pedantic("An empty realm was passed in.");
		return NULL;
	}
	else if(st_empty(shard) || (st_length_get(shard) != 64)) {
		log_pedantic("An empty or invalid shard was passed in.");
		return NULL;
	}
	else if(!(hash_input = st_alloc_opts((MANAGED_T | JOINTED | HEAP | SECURE), st_length_get(master_key) +
		+ st_length_get(realm) + st_length_get(shard)))) {
		log_pedantic("Failed to allocate secure stringer for hash input.");
	}
	else if(!(result = st_alloc_opts((MANAGED_T | JOINTED | HEAP | SECURE), 64))) {
		log_pedantic("Failed to allocate secure stringer for realm key.");
	}

	hash_input = st_append(hash_input, master_key);
	hash_input = st_append(hash_input, realm);
	hash_input = st_append(hash_input, shard);
	result = hash_sha512(hash_input, result);
	result = st_xor(result, shard, result);

	return result;
}

stringer_t * stacie_realm_cipher_key_derive(stringer_t *realm_key) {

	stringer_t *result, *pl;

	if(st_empty(realm_key) || (st_length_get(realm_key) != 64)) {
		log_pedantic("An empty or invalid realm key was passed in.");
		return NULL;
	}
	else if(!(pl = PLACER(st_data_get(realm_key) + 32, 32))) {
		log_pedantic("Failed to create placer for cipher key.");
		return NULL;
	}
	else if(!(result = st_dupe_opts((MANAGED_T | JOINTED | HEAP | SECURE), pl))) {
		log_pedantic("Failed to allocate secure stringer for cipher key.");
		return NULL;
	}

	return result;
}

stringer_t * stacie_realm_init_vector_derive(stringer_t *realm_key) {

	stringer_t *temp, *result, *pl1, *pl2;

	if(st_empty(realm_key) || (st_length_get(realm_key) != 64)) {
		log_pedantic("An empty or invalid realm key was passed in.");
		return NULL;
	}
	else if(!(pl1 = PLACER(st_data_get(realm_key), 16)) || !(pl2 = PLACER(st_data_get(realm_key) + 16, 16))) {
		log_pedantic("Failed to created placers to be xor'd to derive vector key.");
		return NULL;
	}
	else if(!(temp = st_alloc_opts((MANAGED_T | JOINTED | HEAP | SECURE), 16))) {
		log_pedantic("Failed to allocate secure stringer for vector key.");
		return NULL;
	}
	else if(!(result = st_xor(pl1, pl2, temp))) {
		log_pedantic("Failed to xor strings to derive vector key.");
		st_cleanup(temp);
	}

	return result;
}
