
/**
 * @file /check/providers/stacie_check.c
 *
 * @brief Checks the code used to generate STACIE-specified keys and tokens.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma_check.h"


/**
 * @brief Check that all calculation results match up accurately with results from Python reference.
 * @return True if passes, false if fails.
*/
bool_t check_stacie_simple(void) {

	uint_t rounds = 0;
	stringer_t *password = PLACER("SiliconSally", 12), *username = PLACER("user@example.tld", 16),
		*combined_key = NULL, *cipher_key = NULL, *vector_key = NULL, *extracted = NULL,
		*salt = NULL, *nonce = NULL, *seed = NULL, *master_key = NULL, *password_key = NULL,
		*verification_token = NULL,	*ephemeral_login_token = NULL, *shard = NULL, *realm_cipher_key = NULL,
		*realm_vector_key = NULL;

	// A 12 character password must resolve to 4,096 base rounds plus 128 bonus rounds, or 4,224 total rounds.
	if((rounds = stacie_rounds_calculate(password, 128)) != 4224) {
		return false;
	}

	// Decode the binary input values, and comparison values from the hard coded versions provided in modified base64.
	if (!(salt = base64_decode_mod(NULLER("lyrtpzN8cBRZvsiHX6y4j-pJOjIyJeuw5aVXzrItw1G4EOa-6CA4R9BhVpinkeH0UeXyOeTisHR3Ik3yuOhxbWPyesMJvfp0IBtx0f0uorb8wPnhw5BxDJVCb1TOSE50PFKGBFMkc63Koa7vMDj-WEoDj2X0kkTtlW6cUvF8i-M"), NULL)) ||
		!(nonce = base64_decode_mod(NULLER("oDdYAHOsiX7Nl2qTwT18onW0hZdeTO3ebxzZp6nXMTo__0_vr_AsmAm3vYRwWtSCPJz0sA2o66uhNm6YenOGz0NkHcSAVgQhKdEBf_BTYkyULDuw2fSkbO7mlnxEhxqrJEc27ZVam6ogYABfHZjgVUTAi_SICyKAN7KOMuImL2g"), NULL)) ||
		!(ephemeral_login_token = base64_decode_mod(NULLER("_-MOUkFbJLAEFX8I4k9Bf-8VVk9DQlETAliooUH5unLUmgPkxc2peQaQUXvyoRrM87DF3kSfFhCh_uyv3BMb7Q"), NULL)) ||
		!(verification_token = base64_decode_mod(NULLER("4smP8S5oGOR_OUp_T4M1RgOnOeChgme5Xv-ZX8_kt8lYKdPTUlc4oPFgg-5rAyhiqQOfxNa5HyYaefcb_haQ9Q"), NULL)) ||
		!(password_key = base64_decode_mod(NULLER("SeoGINQ3MXFo_xPt_uxYIgOzpU1BjDj9BfNlzbvlA2vOswkAC0sDnViURlhSRa8i91z6B-pQ8etRSaBkyDG_NA"), NULL)) ||
		!(master_key = base64_decode_mod(NULLER("u-pbHPk-YbEY76DFA-X55HOS8BLxPGMY6oViDUed4fXrmlV1pRIpDem26P_1RBeCaWc09btoSP3E_fEF0ffxZA"), NULL)) ||
		!(shard = base64_decode_mod(NULLER("gD65Kdeda1hB2Q6gdZl0fetGg2viLXWG0vmKN4HxE3Jp3Z0Gkt5prqSmcuY2o8t24iGSCOnFDpP71c3xl9SX9Q"), NULL)) ||
		!(seed = base64_decode_mod(NULLER("J7MWPKEl1fVJxa0SDreYIE-lcv6uK9BXIaRYFG6GcHxUo5pmme7i9JcYRvd_yCzg59A7gZAZmbCJ-1uRKOm7Kw"), NULL)) ||
		!(realm_cipher_key = base64_decode_mod(NULLER("QLkIIqMf2eLUxcobqwrjCfCXRcCHL5ZCeHq5Guh-9q4"), NULL)) ||
		!(realm_vector_key = base64_decode_mod(NULLER("f5YGmmqvTOsFLyWtIXjPZw"), NULL))) {
		st_cleanup(ephemeral_login_token, verification_token, realm_vector_key, realm_cipher_key, password_key);
		st_cleanup(master_key, shard, nonce, seed, salt);
		return false;
	}

	// Extract the seed.
	if(!(extracted = stacie_entropy_seed_derive(rounds, password, salt)) || st_cmp_cs_eq(extracted, seed)) {
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
			!(vector_key = stacie_realm_init_vector(combined_key)) || st_cmp_cs_eq(vector_key, realm_vector_key) ||
			!(cipher_key = stacie_realm_cipher_key(combined_key)) || st_cmp_cs_eq(cipher_key, realm_cipher_key)) {
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

/**
 * @brief	Check that rounds are calculated accurately using some simple examples.
 * @return	True if passes, false if fails.
*/
bool_t check_stacie_rounds(void) {

	// Ensure a minimum of 8 rounds is returned even if the password is sufficiently long.
	if(stacie_rounds_calculate(PLACER("3.14159265358979323846264338327950288419716939937510582097494459", 64), 0) != 8) {
		return false;
	}
	// Even with 4 bonus rounds, long passwords yield the minimum value of 8.
	else if (stacie_rounds_calculate(PLACER("3.14159265358979323846264338327950288419716939937510582097494459", 64), 4) != 8) {
		return false;
	}
	// With a long password and 8 bonus rounds, the total should be 10.
	else if (stacie_rounds_calculate(PLACER("3.14159265358979323846264338327950288419716939937510582097494459", 64), 8) != 10) {
		return false;
	}


	// Check the number of rounds for an 8 character password.
	if (stacie_rounds_calculate(PLACER("password", 8), 0) != 65536) {
		return false;
	}
	// And with 128 bonus rounds.
	else if (stacie_rounds_calculate(PLACER("password", 8), 128) != 65664) {
		return false;
	}
	// Ensure the number of rounds is truncated to the maximum for a 24 bit value, or 16,777,216.
	else if(stacie_rounds_calculate(PLACER("password", 8), UINT_MAX) != 16777216) {
		return false;
	}

	// Ensure a single character password is handled correctly and results in 2^23 or 8,388,608
	if(stacie_rounds_calculate(PLACER("A", 1), 0) != 8388608) {
		return false;
	}
	// Then try the same password with bonus rounds and ensure we can grow the number until we hit the max.
	else if (stacie_rounds_calculate(PLACER("A", 1), 8388606) != 16777214) {
		return false;
	}
	else if (stacie_rounds_calculate(PLACER("A", 1), 8388607) != 16777215) {
		return false;
	}
	else if (stacie_rounds_calculate(PLACER("A", 1), 8388608) != 16777216) {
		return false;
	}
	else if (stacie_rounds_calculate(PLACER("A", 1), 8388609) != 16777216) {
		return false;
	}

	// Ensure an invalid UTF8 string returns 0.
	if (stacie_rounds_calculate(hex_decode_st(NULLER("C380C3"), MANAGEDBUF(64)), 0) != 0) {
		return false;
	}

	// Try using the string password, only with an accented alpha character.
	if (stacie_rounds_calculate(hex_decode_st(NULLER("70C3A17373776F7264"), MANAGEDBUF(64)), 0) != 65536) {
		return false;
	}
	// Try the same word, but with the UTF8 byte order mark at the beginning, which should not be included in the length.
	else if (stacie_rounds_calculate(hex_decode_st(NULLER("EFBBBF70C3A17373776F7264"), MANAGEDBUF(64)), 0) != 65536) {
		return false;
	}

	// Try two different Spanish words which are the same length in bytes, but different in character length.
	if (stacie_rounds_calculate(hex_decode_st(NULLER("7465616D6F6D61E1B8BF61"), MANAGEDBUF(64)), 0) != 32768 ||
		stacie_rounds_calculate(hex_decode_st(NULLER("636F6E7472617365C3B161"), MANAGEDBUF(64)), 0) != 16384) {
		return false;
	}

	// Try three different Chinese words of different lengths.
	if (stacie_rounds_calculate(hex_decode_st(NULLER("E68891E788B1E4BDA0"), MANAGEDBUF(64)), 0) != 2097152 ||
		stacie_rounds_calculate(hex_decode_st(NULLER("E5B890E58FB7E5AF86E7A081"), MANAGEDBUF(64)), 0) != 1048576 ||
		stacie_rounds_calculate(hex_decode_st(NULLER("E68891E79A84E4B8ADE69687E5BE97E5BE88E5A5BD"), MANAGEDBUF(64)), 0) != 131072) {
		return false;
	}

	return true;
}

/**
 * @brief	Check that calculations are deterministic.
 * @return	True if passes, false if fails.
*/
bool_t check_stacie_determinism(void) {

	uint_t rounds1, rounds2;
	stringer_t *username = PLACER("DongleDonkey", 12), *password = NULL,
		*salt = PLACER("3.141592653589793238462643383279502884197169399375105820974944592307816406286208998628034825342" \
			"117067982148086513282306647093844", 128),
		*nonce = PLACER("2.718281828459045235360287471352662497757247093699959574966967627724076630353547594571382178525" \
			"166427427466391932003059921817413", 128),
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


	/*if(!(res1 = stacie_entropy_seed_derive(STACIE_ROUNDS_MAX, password, salt)) ||
		!(res2 = stacie_entropy_seed_derive(STACIE_ROUNDS_MAX, password, salt)) ||
		st_cmp_cs_eq(res1, res2)) {
		st_cleanup(res1, res2);
		return false;
	}

	st_cleanup(res1, res2);
	res1 = res2 = NULL;*/


	if(!(res1 = stacie_entropy_seed_derive(STACIE_ROUNDS_MIN, password, salt)) ||
		!(res2 = stacie_entropy_seed_derive(STACIE_ROUNDS_MIN, password, salt)) ||
		st_cmp_cs_eq(res1, res2)) {
		st_cleanup(res1, res2);
		return false;
	}

	base = res2;
	st_cleanup(res1);
	res1 = res2 = NULL;

	// Run deterministic tests on the hash derivation stage.
	if(!(res1 = stacie_hashed_key_derive(base, STACIE_ROUNDS_MIN, username, password, salt)) ||
		!(res2 = stacie_hashed_key_derive(base, STACIE_ROUNDS_MIN, username, password, salt)) ||
		st_cmp_cs_eq(res1, res2)) {
		st_cleanup(res1, res2, base);
		return false;
	}

	st_cleanup(res1, res2);
	res1 = res2 = NULL;
/*
	if(!(res1 = stacie_hashed_key_derive(base, STACIE_ROUNDS_MIN, username, password, salt)) ||
		!(res2 = stacie_hashed_key_derive(base, STACIE_ROUNDS_MIN, username, password, salt)) ||
		st_cmp_cs_eq(res1, res2)) {
		st_cleanup(res1, res2, base);
		return false;
	}

	st_cleanup(res1, res2);
	res1 = res2 = NULL;
*/
	// Run deterministic tests on the token derivation stages.
	if(!(res1 = stacie_hashed_token_derive(base, username, salt, nonce)) ||
		!(res2 = stacie_hashed_token_derive(base, username, salt, nonce)) ||
		st_cmp_cs_eq(res1, res2)) {
		st_cleanup(res1, res2, base);
		return false;
	}

	st_cleanup(res1, res2, base);
	res1 = res2 = NULL;

	// Run deterministic tests on the key derivation stages.
	if(!(res1 = stacie_realm_cipher_key(key)) ||
		!(res2 = stacie_realm_cipher_key(key)) ||
		st_cmp_cs_eq(res1, res2)) {
		st_cleanup(res1, res2);
		return false;
	}

	st_cleanup(res1, res2);
	res1 = res2 = NULL;

	if(!(res1 = stacie_realm_init_vector(key)) ||
		!(res2 = stacie_realm_init_vector(key)) ||
		st_cmp_cs_eq(res1, res2)) {
		st_cleanup(res1, res2);
		return false;
	}

	st_cleanup(res1, res2);
	res1 = res2 = NULL;

	return true;
}

/**
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

	if((res = stacie_entropy_seed_derive(0, temp_st, NULL))) {
		st_free(res);
		return false;
	}

	if((res = stacie_entropy_seed_derive(0, temp_st, temp_st64))) {
		st_free(res);
		return false;
	}

	if((res = stacie_entropy_seed_derive(0xFFFFFFFF, temp_st, NULL))) {
		st_free(res);
		return false;
	}

	if((res = stacie_entropy_seed_derive(0xFFFFFFFF, temp_st, temp_st64))) {
		st_free(res);
		return false;
	}

	if((res = stacie_entropy_seed_derive(STACIE_ROUNDS_MIN, temp_st, NULL))) {
		st_free(res);
		return false;
	}

	if((res = stacie_entropy_seed_derive(STACIE_ROUNDS_MIN, temp_st, temp_st64))) {
		st_free(res);
		return false;
	}

	if((res = stacie_entropy_seed_derive(STACIE_ROUNDS_MIN, temp_st, temp_st))) {
		st_free(res);
		return false;
	}

	if((res = stacie_hashed_key_derive(NULL, STACIE_ROUNDS_MIN, temp_st, temp_st, NULL))) {
		st_free(res);
		return false;
	}

	if((res = stacie_hashed_key_derive(temp_st, STACIE_ROUNDS_MIN, temp_st, temp_st, NULL))) {
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

	if((res = stacie_hashed_key_derive(temp_st64, STACIE_ROUNDS_MIN, NULL, temp_st, NULL))) {
		st_free(res);
		return false;
	}

	if((res = stacie_hashed_key_derive(temp_st64, STACIE_ROUNDS_MIN, temp_st, NULL, NULL))) {
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

	if((res = stacie_realm_cipher_key(NULL))) {
		st_free(res);
		return false;
	}

	if((res = stacie_realm_cipher_key(temp_st))) {
		st_free(res);
		return false;
	}

	if((res = stacie_realm_init_vector(NULL))) {
		st_free(res);
		return false;
	}

	if((res = stacie_realm_init_vector(temp_st))) {
		st_free(res);
		return false;
	}

	return true;
}

