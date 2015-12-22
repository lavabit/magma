
/**
 * @file /magma.check/providers/stacie_check.c
 *
 * @brief Checks the code used to generate STACIE-specified keys and tokens.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma_check.h"


/*
 * @brief Check that all calculation results match up accurately with results from Python reference.
 * @return True if passes, false if fails.
*/
bool_t check_stacie_simple(void) {

	uint_t rounds = 0;
	stringer_t *password = PLACER("password", 8), *username = PLACER("user@example.tld", 16),
		*combined_key = NULL, *cipher_key = NULL, *vector_key = NULL, *extracted = NULL,
		*salt = NULL, *nonce = NULL, *seed = NULL, *master_key = NULL, *password_key = NULL,
		*verification_token = NULL,	*ephemeral_login_token = NULL, *shard = NULL, *realm_cipher_key = NULL,
		*realm_vector_key = NULL;

	// An 8 character password must resolve to 65,536 base rounds plus 131,072 (aka 2^17) bonus rounds, or 196,608 total rounds.
	if((rounds = stacie_rounds_calculate(password, 131072)) != 196608) {
		return false;
	}

	// Decode the binary input values, and comparison values from the hard coded versions provided in modified base64.
	if (!(ephemeral_login_token = base64_decode_mod(NULLER("IVl9I1cEPWP3wd_XNYng8qSyzaic4Z_gCVaJcIZTE7rlGkwwh3oF63X8K4T0AvsEJOGOCnWQMirIyKiBpDD59Q"), NULL)) ||
			!(verification_token = base64_decode_mod(NULLER("Xe9Xj70O2M2ctVCclrgO6FaJLiVmKPhMZZKMGVHeTetnxWglvgeu21T7Ms0pCKXiAUG2NuSO2cF5MUAuV1qCgg"), NULL)) ||
			!(password_key = base64_decode_mod(NULLER("SskpKhi42KmNHIpO7v9NdATRDFY5oAl9gu03rGrUzowwLnXAkAxr1h1b5lajgNnWfH2WlE6I6vbXqgjgh4ExMA"), NULL)) ||
			!(master_key = base64_decode_mod(NULLER("SXn5XauE903R7ir03ZS5oE5PwJ2UhxtFS-8LygUUSNQLAabDUDUu805EpVxxuQX-Smvc0NV-q6RdzyQ8M3eysg"), NULL)) ||
			!(shard = base64_decode_mod(NULLER("gD65Kdeda1hB2Q6gdZl0fetGg2viLXWG0vmKN4HxE3Jp3Z0Gkt5prqSmcuY2o8t24iGSCOnFDpP71c3xl9SX9Q"), NULL)) ||
			!(nonce = base64_decode_mod(NULLER("sJWAhD5Okulpjpa63FE4dGI-W3PDACaQtA49vQBOG9_UYhgNMzmLuSeRBEQy15Lv2Wn_lvSmzRkWfky51Fpp7Q"), NULL)) ||
			!(seed = base64_decode_mod(NULLER("6xuALdCjmjaBlMSn8KkM9sSwO-DGM2V7J5r_K6g6Ocgg1VgnKTSsH4nsKokP597Wsdc9modu_ArofThTNIHcpw"), NULL)) ||
			!(salt = base64_decode_mod(NULLER("HQpHA0L4Izkpy1lVY8Cnp03-D67E2bk04WDqNOiSzIMNnbmCjGlMRKxBh9UV5IgXggpRDYTYSRlTWNsohwvLwA"), NULL)) ||
			!(realm_cipher_key = base64_decode_mod(NULLER("E5-hWK1n7StXpyqLlrR7aEEJaWGIMC3Ml4hlSHhb4xI"), NULL)) ||
			!(realm_vector_key = base64_decode_mod(NULLER("EBThm16sL5xQciv2BgPD2w"), NULL))) {
		st_cleanup(ephemeral_login_token, verification_token, realm_vector_key, realm_cipher_key, password_key);
		st_cleanup(master_key, shard, nonce, seed, salt);
		return false;
	}

	// Extract the seed.
	if(!(extracted = stacie_seed_extract(rounds, username, password, salt)) || st_cmp_cs_eq(extracted, seed)) {
		st_cleanup(ephemeral_login_token, verification_token, realm_vector_key, realm_cipher_key, password_key);
		st_cleanup(master_key, shard, nonce, seed, salt);
		st_cleanup(extracted);
		return false;
	}

	st_free(extracted);

	// Extract the master key.
	if(!(extracted = stacie_hashed_key_derive(seed, rounds, username, password, salt)) || st_cmp_cs_eq(master_key, extracted)) {
		st_cleanup(ephemeral_login_token, verification_token, realm_vector_key, realm_cipher_key, password_key);
		st_cleanup(master_key, shard, nonce, seed, salt);
		st_cleanup(extracted);
		return false;
	}

	st_free(extracted);

	// Calculate the password key.
	if(!(extracted = stacie_hashed_key_derive(master_key, rounds, username, password, salt)) || st_cmp_cs_eq(password_key, extracted)) {
		st_cleanup(ephemeral_login_token, verification_token, realm_vector_key, realm_cipher_key, password_key);
		st_cleanup(master_key, shard, nonce, seed, salt);
		st_cleanup(extracted);
		return false;
	}

	st_free(extracted);

	// Calculate the verification token.
	if(!(extracted = stacie_hashed_token_derive(password_key, username, salt, NULL)) || st_cmp_cs_eq(verification_token, extracted)) {
		st_cleanup(ephemeral_login_token, verification_token, realm_vector_key, realm_cipher_key, password_key);
		st_cleanup(master_key, shard, nonce, seed, salt);
		st_cleanup(extracted);
		return false;
	}

	st_free(extracted);


	// Calculate the ephemeral login token.
	if(!(extracted = stacie_hashed_token_derive(verification_token, username, salt, nonce)) || st_cmp_cs_eq(extracted, ephemeral_login_token)) {
		st_cleanup(ephemeral_login_token, verification_token, realm_vector_key, realm_cipher_key, password_key);
		st_cleanup(master_key, shard, nonce, seed, salt);
		st_cleanup(extracted);
		return false;
	}

	st_free(extracted);

	// Calculate the symmetric key for the "mail" realm and check extracted cipher and vector key values.
	if(!(combined_key = stacie_realm_key_derive(master_key, NULLER("mail"), shard)) ||
			!(vector_key = stacie_realm_init_vector_derive(combined_key)) || st_cmp_cs_eq(vector_key, realm_vector_key) ||
			!(cipher_key = stacie_realm_cipher_key_derive(combined_key)) || st_cmp_cs_eq(cipher_key, realm_cipher_key)) {
		st_cleanup(ephemeral_login_token, verification_token, realm_vector_key, realm_cipher_key, password_key);
		st_cleanup(master_key, shard, nonce, seed, salt);
		st_cleanup(combined_key, vector_key, cipher_key);
		return false;
	}

	st_cleanup(ephemeral_login_token, verification_token, realm_vector_key, realm_cipher_key, password_key);
	st_cleanup(master_key, shard, nonce, seed, salt);
	st_cleanup(combined_key, vector_key, cipher_key);

	return true;
}

/*
 * @brief	Check that rounds are calculated accurately using some simple examples.
 * @return	True if passes, false if fails.
*/
bool_t check_stacie_rounds(void) {

	// Ensure a minimum of 8 rounds is returned even if the password is sufficiently long.
	if(stacie_rounds_calculate(PLACER("3.14159265358979323846264338327950288419716939937510582097494459", 64), 0) != 8) {
		return false;
	}

	// Ensure the number of rounds is truncated to the maximum for a 24 bit value, or 16,777,215.
	if(stacie_rounds_calculate(PLACER("password", 8), UINT_MAX) != 16777215) {
		return false;
	}

	// Ensure a single character password is handled correctly and results in 2^23 or 8,388,608
	if(stacie_rounds_calculate(PLACER("A", 1), 0) != 8388608) {
		return false;
	}

	return true;
}

/*
 * @brief	Check that calculations are deterministic.
 * @return	True if passes, false if fails.
*/
bool_t check_stacie_determinism(void) {

	uint_t rounds1, rounds2;
	stringer_t *username = PLACER("this_user", 9), *password = NULL,
		*salt = PLACER("SALT1234SALT5678SALT9012SALT3456SALT7890SALT1234SALT5678SALT9012", 64),
		*nonce = PLACER("NONCE123NONCE456NONCE789NONCE012NONCE345NONCE678NONCE901NONCE234", 64),
		*key = PLACER("KEY12345KEY67890KEY12345KEY67890KEY12345KEY67890KEY12345KEY67890", 64),
		*res1 = NULL, *res2 = NULL, *base = NULL;

	// Check the rounds function using a simple password.
	password = PLACER("A", 1);

	if(!(rounds1 = stacie_rounds_calculate(password, 0)) ||
		!(rounds2 = stacie_rounds_calculate(password, 0)) ||
		rounds1 != rounds2) {
		return false;
	}

	if(!(rounds1 = stacie_rounds_calculate(password, 0xFFFFFFFF)) ||
		!(rounds2 = stacie_rounds_calculate(password, 0xFFFFFFFF)) ||
		rounds1 != rounds2) {
		return false;
	}

	// Change the password value.
	password = PLACER("PASSWORDpasswordPASSWORD", 24);

	if(!(rounds1 = stacie_rounds_calculate(password, 0)) ||
		!(rounds2 = stacie_rounds_calculate(password, 0)) ||
		rounds1 != rounds2) {
		return false;
	}

	if(!(rounds1 = stacie_rounds_calculate(password, 0xFFFFFFFF)) ||
		!(rounds2 = stacie_rounds_calculate(password, 0xFFFFFFFF)) ||
		rounds1 != rounds2) {
		return false;
	}

	// Run deterministic tests on the seed derivation stage.
	if(!(res1 = stacie_seed_key_derive(salt)) ||
		!(res2 = stacie_seed_key_derive(salt)) ||
		st_cmp_cs_eq(res1, res2)) {
		st_cleanup(res1, res2);
		return false;
	}

	st_cleanup(res1, res2);
	res1 = res2 = NULL;

	if(!(res1 = stacie_seed_extract(MIN_HASH_NUM, username, password, salt)) ||
		!(res2 = stacie_seed_extract(MIN_HASH_NUM, username, password, salt)) ||
		st_cmp_cs_eq(res1, res2)) {
		st_cleanup(res1, res2);
		return false;
	}

	st_cleanup(res1, res2);
	res1 = res2 = NULL;

	if(!(res1 = stacie_seed_extract(MIN_HASH_NUM, username, password, NULL)) ||
		!(res2 = stacie_seed_extract(MIN_HASH_NUM, username, password, NULL)) ||
		st_cmp_cs_eq(res1, res2)) {
		st_cleanup(res1, res2);
		return false;
	}

	st_cleanup(res1, res2);
	res1 = res2 = NULL;

	if(!(res1 = stacie_seed_extract(MAX_HASH_NUM, username, password, salt)) ||
		!(res2 = stacie_seed_extract(MAX_HASH_NUM, username, password, salt)) ||
		st_cmp_cs_eq(res1, res2)) {
		st_cleanup(res1, res2);
		return false;
	}

	st_cleanup(res1, res2);
	res1 = res2 = NULL;

	// Run deterministic tests on the hash derivation stage.
	base = stacie_seed_extract(MIN_HASH_NUM, username, password, NULL);

	if(!(res1 = stacie_hashed_key_derive(base, MIN_HASH_NUM, username, password, salt)) ||
		!(res2 = stacie_hashed_key_derive(base, MIN_HASH_NUM, username, password, salt)) ||
		st_cmp_cs_eq(res1, res2)) {
		st_cleanup(res1, res2);
		return false;
	}

	st_cleanup(res1, res2);
	res1 = res2 = NULL;

	if(!(res1 = stacie_hashed_key_derive(base, MIN_HASH_NUM, username, password, NULL)) ||
		!(res2 = stacie_hashed_key_derive(base, MIN_HASH_NUM, username, password, NULL)) ||
		st_cmp_cs_eq(res1, res2)) {
		st_cleanup(res1, res2);
		return false;
	}

	st_cleanup(res1, res2);
	res1 = res2 = NULL;

	// Run deterministic tests on the token derivation stages.
	if(!(res1 = stacie_hashed_token_derive(base, username, salt, nonce)) ||
		!(res2 = stacie_hashed_token_derive(base, username, salt, nonce)) ||
		st_cmp_cs_eq(res1, res2)) {
		st_cleanup(res1, res2);
		return false;
	}

	st_cleanup(res1, res2, base);
	res1 = res2 = NULL;

	// Run deterministic tests on the key derivation stages.
	if(!(res1 = stacie_realm_cipher_key_derive(key)) ||
		!(res2 = stacie_realm_cipher_key_derive(key)) ||
		st_cmp_cs_eq(res1, res2)) {
		st_cleanup(res1, res2);
		return false;
	}

	st_cleanup(res1, res2);
	res1 = res2 = NULL;

	if(!(res1 = stacie_realm_init_vector_derive(key)) ||
		!(res2 = stacie_realm_init_vector_derive(key)) ||
		st_cmp_cs_eq(res1, res2)) {
		st_cleanup(res1, res2);
		return false;
	}

	st_cleanup(res1, res2);
	res1 = res2 = NULL;

	return true;
}

/*
 * @brief	Check that STACIE functions fail when provided with illegal parameters.
 * @return	True if unit test passes, false if it fails.
*/
bool_t check_stacie_parameters(void) {

	stringer_t *temp_st, *res, *temp_st64, *temp_empty;
	temp_st = NULLER("temp_string");
	temp_st64 = NULLER("TEMP1234TEMP5678TEMP9012TEMP3456TEMP7890TEMP1234TEMP5678TEMP9012");
	temp_empty = NULLER("");

	if(stacie_rounds_calculate(NULL, 0)) {
		return false;
	}

	if((res = stacie_seed_key_derive(NULL))) {
		st_free(res);
		return false;
	}

	if((res = stacie_seed_key_derive(temp_empty))) {
		st_free(res);
		return false;
	}

	if((res = stacie_seed_extract(0, temp_st, temp_st, NULL))) {
		st_free(res);
		return false;
	}

	if((res = stacie_seed_extract(0, temp_st, temp_st, temp_st64))) {
		st_free(res);
		return false;
	}

	if((res = stacie_seed_extract(0xFFFFFFFF, temp_st, temp_st, NULL))) {
		st_free(res);
		return false;
	}

	if((res = stacie_seed_extract(0xFFFFFFFF, temp_st, temp_st, temp_st64))) {
		st_free(res);
		return false;
	}

	if((res = stacie_seed_extract(MIN_HASH_NUM, NULL, temp_st, NULL))) {
		st_free(res);
		return false;
	}

	if((res = stacie_seed_extract(MIN_HASH_NUM, NULL, temp_st, temp_st64))) {
		st_free(res);
		return false;
	}

	if((res = stacie_seed_extract(MAX_HASH_NUM, temp_st, NULL, NULL))) {
		st_free(res);
		return false;
	}

	if((res = stacie_seed_extract(MAX_HASH_NUM, temp_st, NULL, temp_st64))) {
		st_free(res);
		return false;
	}

	if((res = stacie_seed_extract(MIN_HASH_NUM, temp_st, temp_st, temp_st))) {
		st_free(res);
		return false;
	}

	if((res = stacie_hashed_key_derive(NULL, MIN_HASH_NUM, temp_st, temp_st, NULL))) {
		st_free(res);
		return false;
	}

	if((res = stacie_hashed_key_derive(temp_st, MIN_HASH_NUM, temp_st, temp_st, NULL))) {
		st_free(res);
		return false;
	}

	if((res = stacie_hashed_key_derive(temp_st64, 0, temp_st, temp_st, NULL))) {
		st_free(res);
		return false;
	}

	if((res = stacie_hashed_key_derive(temp_st64, 0xFFFFFFFF, temp_st, temp_st, NULL))) {
		st_free(res);
		return false;
	}

	if((res = stacie_hashed_key_derive(temp_st64, MIN_HASH_NUM, NULL, temp_st, NULL))) {
		st_free(res);
		return false;
	}

	if((res = stacie_hashed_key_derive(temp_st64, MIN_HASH_NUM, temp_st, NULL, NULL))) {
		st_free(res);
		return false;
	}

	if((res = stacie_hashed_token_derive(NULL, temp_st, NULL, temp_st64))) {
		st_free(res);
		return false;
	}

	if((res = stacie_hashed_token_derive(temp_st, temp_st, NULL, NULL))) {
		st_free(res);
		return false;
	}

	if((res = stacie_hashed_token_derive(temp_st64, NULL, NULL, NULL))) {
		st_free(res);
		return false;
	}

	if((res = stacie_hashed_token_derive(temp_st64, temp_st, temp_st, temp_st64))) {
		st_free(res);
		return false;
	}

	if((res = stacie_hashed_token_derive(temp_st64, temp_st, NULL, temp_st))) {
		st_free(res);
		return false;
	}

	if((res = stacie_realm_key_derive(NULL, temp_st, temp_st64))) {
		st_free(res);
		return false;
	}

	if((res = stacie_realm_key_derive(temp_st, temp_st, temp_st64))) {
		st_free(res);
		return false;
	}

	if((res = stacie_realm_key_derive(temp_st64, NULL, temp_st64))) {
		st_free(res);
		return false;
	}

	if((res = stacie_realm_key_derive(temp_st64, temp_st, NULL))) {
		st_free(res);
		return false;
	}

	if((res = stacie_realm_key_derive(temp_st64, temp_st, temp_st))) {
		st_free(res);
		return false;
	}

	if((res = stacie_realm_cipher_key_derive(NULL))) {
		st_free(res);
		return false;
	}

	if((res = stacie_realm_cipher_key_derive(temp_st))) {
		st_free(res);
		return false;
	}

	if((res = stacie_realm_init_vector_derive(NULL))) {
		st_free(res);
		return false;
	}

	if((res = stacie_realm_init_vector_derive(temp_st))) {
		st_free(res);
		return false;
	}

	return true;
}

