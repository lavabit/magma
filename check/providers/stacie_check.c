
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
 * @briefCheck that all calculation results match up accurately with results from python reference.
 * @return True if passes, false if fails.
 * @note 	There are some memory leaks in this function if it doesn't succeed.
*/
bool_t check_stacie_simple(void) {

	int cmp;
	unsigned int bonus = 0, rounds = 196608, res1 = 0;

	stringer_t *password = PLACER("password", 8),
		   *username = PLACER("user@example.tld", 16),
		   *salt_b64 = NULLER("HQpHA0L4Izkpy1lVY8Cnp03-D67E2bk04WDqNOiSzIMNnbmCjGlMRKxBh9UV5IgXggpRDYTYSRlTWNsohwvLwA"),
		   *nonce_b64 = NULLER("sJWAhD5Okulpjpa63FE4dGI-W3PDACaQtA49vQBOG9_UYhgNMzmLuSeRBEQy15Lv2Wn_lvSmzRkWfky51Fpp7Q"),
		   *seed_b64 = NULLER("6xuALdCjmjaBlMSn8KkM9sSwO-DGM2V7J5r_K6g6Ocgg1VgnKTSsH4nsKokP597Wsdc9modu_ArofThTNIHcpw"),
		   *master_key_b64 = NULLER("SXn5XauE903R7ir03ZS5oE5PwJ2UhxtFS-8LygUUSNQLAabDUDUu805EpVxxuQX-Smvc0NV-q6RdzyQ8M3eysg"),
		   *password_key_b64 = NULLER("SskpKhi42KmNHIpO7v9NdATRDFY5oAl9gu03rGrUzowwLnXAkAxr1h1b5lajgNnWfH2WlE6I6vbXqgjgh4ExMA"),
		   *verification_token_b64 = NULLER("Xe9Xj70O2M2ctVCclrgO6FaJLiVmKPhMZZKMGVHeTetnxWglvgeu21T7Ms0pCKXiAUG2NuSO2cF5MUAuV1qCgg"),
		   *ephemeral_login_token_b64 = NULLER("IVl9I1cEPWP3wd_XNYng8qSyzaic4Z_gCVaJcIZTE7rlGkwwh3oF63X8K4T0AvsEJOGOCnWQMirIyKiBpDD59Q"),
		   *realm = NULLER("mail"),
		   *shard_b64 = NULLER("gD65Kdeda1hB2Q6gdZl0fetGg2viLXWG0vmKN4HxE3Jp3Z0Gkt5prqSmcuY2o8t24iGSCOnFDpP71c3xl9SX9Q"),
		   *realm_vector_key_b64 = NULLER("EBThm16sL5xQciv2BgPD2w"),
		   *realm_cipher_key_b64 = NULLER("E5-hWK1n7StXpyqLlrR7aEEJaWGIMC3Ml4hlSHhb4xI"),
		   *realm_key, *res2, *salt, *nonce, *seed, *master_key, *password_key, *verification_token,
		   *ephemeral_login_token, *shard, *realm_cipher_key, *realm_vector_key;

	if(!(salt = base64_decode_mod(salt_b64, NULL)) || !(nonce = base64_decode_mod(nonce_b64, NULL)) || !(seed = base64_decode_mod(seed_b64, NULL))) {
		return false;
	} else if(!(master_key = base64_decode_mod(master_key_b64, NULL)) || !(password_key = base64_decode_mod(password_key_b64, NULL))) {
		return false;
	} else if(!(verification_token = base64_decode_mod(verification_token_b64, NULL)) || !(ephemeral_login_token = base64_decode_mod(ephemeral_login_token_b64, NULL))) {
		return false;
	} else if(!(shard = base64_decode_mod(shard_b64, NULL)) || !(realm_cipher_key = base64_decode_mod(realm_cipher_key_b64, NULL))) {
		return false;
	} else if(!(realm_vector_key = base64_decode_mod(realm_vector_key_b64, NULL))) {
		return false;
	}

	bonus = (((unsigned int) 2) << 16);

	if((res1 = stacie_rounds_calculate(password, bonus)) != rounds) {
		return false;
	}

	if(!(res2 = stacie_seed_extract(rounds, username, password, salt))) {
		return false;
	}


	cmp = st_cmp_cs_eq(res2, seed);
	st_free(res2);

	if(cmp) {
		return false;
	}

	if(!(res2 = stacie_hashed_key_derive(seed, rounds, username, password, salt))) {
		return false;
	}

	cmp = st_cmp_cs_eq(master_key, res2);
	st_free(res2);

	if(cmp) {
		return false;
	}

	if(!(res2 = stacie_hashed_key_derive(master_key, rounds, username, password, salt))) {
		return false;
	}

	cmp = st_cmp_cs_eq(password_key, res2);
	st_free(res2);

	if(cmp) {
		return false;
	}

	if(!(res2 = stacie_hashed_token_derive(password_key, username, salt, NULL))) {
		return false;
	}

	cmp = st_cmp_cs_eq(verification_token, res2);
	st_free(res2);

	if(cmp) {
		return false;
	}

	if(!(res2 = stacie_hashed_token_derive(verification_token, username, salt, nonce))) {
		return false;
	}

	cmp = st_cmp_cs_eq(ephemeral_login_token, res2);
	st_free(res2);

	if(cmp) {
		return false;
	}

	if(!(realm_key = stacie_realm_key_derive(master_key, realm, shard))) {
		return false;
	}

	if(!(res2 = stacie_realm_init_vector_derive(realm_key))) {
		return false;
	}

	cmp = st_cmp_cs_eq(res2, realm_vector_key);
	st_free(res2);

	if(cmp) {
		return false;
	}

	if(!(res2 = stacie_realm_cipher_key_derive(realm_key))) {
		return false;
	}

	cmp = st_cmp_cs_eq(res2, realm_cipher_key);
	st_free(res2);
	st_free(realm_key);

	if(cmp) {
		return false;
	}

	st_free(salt);
	st_free(nonce);
	st_free(seed);
	st_free(master_key);
	st_free(password_key);
	st_free(verification_token_b64);
	st_free(ephemeral_login_token);
	st_free(shard);
	st_free(realm_cipher_key);
	st_free(realm_vector_key);

	return true;
}


/*
 * @brief	Check that rounds are calculated accurately using some simple examples.
 * @return	True if passes, false if fails.
*/
bool_t check_stacie_rounds(void) {

	stringer_t *temp_pw;

	temp_pw = PLACER("THIS_IS_A_VERY_LONG_PASSWORD_MORE_THAN_24", 41);

	if(stacie_rounds_calculate(temp_pw, 0) != 8) {
		return false;
	}

	if(stacie_rounds_calculate(temp_pw, 0xFFFFFFFF) != 0x00FFFFFF) {
		return false;
	}

	temp_pw = PLACER("A", 1);

	if(stacie_rounds_calculate(temp_pw, 0) != 0x00800000) {
		return false;
	}

	return true;
}

/*
 * @brief	Check that calculations are deterministic.
 * @return	True if passes, false if fails.
*/
bool_t check_stacie_determinism(void) {

	int outcome;
	uint_t res_int1, res_int2;
	stringer_t *base, *user, *pass, *salt, *res1 = NULL, *res2 = NULL, *nonce, *key;

//	stacie_rounds_calculate
	pass = PLACER("A", 1);

	if(!(res_int1 = stacie_rounds_calculate(pass, 0)) || !(res_int2 = stacie_rounds_calculate(pass, 0))) {
		return false;
	}

	if(res_int1 != res_int2) {
		return false;
	}

	if(!(res_int1 = stacie_rounds_calculate(pass, 0xFFFFFFFF)) || !(res_int2 = stacie_rounds_calculate(pass, 0xFFFFFFFF))) {
		return false;
	}

	if(res_int1 != res_int2) {
		return false;
	}

	pass = PLACER("PASSWORDpasswordPASSWORD", 24);

	if(!(res_int1 = stacie_rounds_calculate(pass, 0)) || !(res_int2 = stacie_rounds_calculate(pass, 0))) {
		return false;
	}

	if(res_int1 != res_int2) {
		return false;
	}

	if(!(res_int1 = stacie_rounds_calculate(pass, 0xFFFFFFFF)) || !(res_int2 = stacie_rounds_calculate(pass, 0xFFFFFFFF))) {
		return false;
	}

	if(res_int1 != res_int2) {
		return false;
	}

//	stacie_seed_extract
	user = PLACER("this_user", 9);
	pass = PLACER("test_password", 13);
	salt = PLACER("SALT1234SALT5678SALT9012SALT3456SALT7890SALT1234SALT5678SALT9012", 64);

	if(!(res1 = stacie_seed_key_derive(salt)) || !(res2 = stacie_seed_key_derive(salt))) {
		return false;
	}

	outcome = st_cmp_cs_eq(res1, res2);
	st_free(res1);
	st_free(res2);

	if(outcome) {
		return false;
	}

	if(!(res1 = stacie_seed_extract(MIN_HASH_NUM, user, pass, salt)) || !(res2 = stacie_seed_extract(MIN_HASH_NUM, user, pass, salt))) {
		return false;
	}

	outcome = st_cmp_cs_eq(res1, res2);
	st_free(res1);
	st_free(res2);

	if(outcome) {
		return false;
	}

	if(!(res1 = stacie_seed_extract(MIN_HASH_NUM, user, pass, NULL)) || !(res2 = stacie_seed_extract(MIN_HASH_NUM, user, pass, NULL))) {
		return false;
	}

	outcome = st_cmp_cs_eq(res1, res2);
	st_free(res1);
	st_free(res2);

	if(outcome) {
		return false;
	}

	if(!(res1 = stacie_seed_extract(MAX_HASH_NUM, user, pass, salt)) || !(res2 = stacie_seed_extract(MAX_HASH_NUM, user, pass, salt))) {
		return false;
	}

	outcome = st_cmp_cs_eq(res1, res2);
	st_free(res1);
	st_free(res2);

	if(outcome) {
		return false;
	}

//	stacie_hashed_key_derive

	res1 = stacie_seed_extract(MIN_HASH_NUM, user, pass, NULL);
	base = MANAGEDBUF(64);
	mm_copy(st_data_get(base), st_data_get(res1), st_length_get(res1));
	st_length_set(base, 64);
	st_free(res1);

	if(!(res1 = stacie_hashed_key_derive(base, MIN_HASH_NUM, user, pass, salt)) || !(res2 = stacie_hashed_key_derive(base, MIN_HASH_NUM, user, pass, salt))) {
		return false;
	}

	outcome = st_cmp_cs_eq(res1, res2);
	st_free(res1);
	st_free(res2);

	if(outcome) {
		return false;
	}

	if(!(res1 = stacie_hashed_key_derive(base, MIN_HASH_NUM, user, pass, NULL)) || !(res2 = stacie_hashed_key_derive(base, MIN_HASH_NUM, user, pass, NULL))) {
		return false;
	}

	outcome = st_cmp_cs_eq(res1, res2);
	st_free(res1);
	st_free(res2);

	if(outcome) {
		return false;
	}
/* Test times out with max hash numbers.
	if(!(res1 = stacie_hashed_key_derive(base, MAX_HASH_NUM, user, pass, salt)) || !(res2 = stacie_hashed_key_derive(base, MAX_HASH_NUM, user, pass, salt))) {
		return false;
	}

	outcome = st_cmp_cs_eq(res1, res2);
	st_free(res1);
	st_free(res2);

	if(outcome) {
		return false;
	}

	if(!(res1 = stacie_hashed_key_derive(base, MAX_HASH_NUM, user, pass, NULL)) || !(res2 = stacie_hashed_key_derive(base, MAX_HASH_NUM, user, pass, NULL))) {
		return false;
	}

	outcome = st_cmp_cs_eq(res1, res2);
	st_free(res1);
	st_free(res2);

	if(outcome) {
		return false;
	}
*/
//	stacie_hashed_token_derive

	nonce = PLACER("NONCE123NONCE456NONCE789NONCE012NONCE345NONCE678NONCE901NONCE234", 64);

	if(!(res1 = stacie_hashed_token_derive(base, user, salt, nonce)) || !(res2 = stacie_hashed_token_derive(base, user, salt, nonce))) {
		return false;
	}

	outcome = st_cmp_cs_eq(res1, res2);
	st_free(res1);
	st_free(res2);

	if(outcome) {
		return false;
	}

	if(!(res1 = stacie_hashed_token_derive(base, user, NULL, nonce)) || !(res2 = stacie_hashed_token_derive(base, user, NULL, nonce))) {
		return false;
	}

	outcome = st_cmp_cs_eq(res1, res2);
	st_free(res1);
	st_free(res2);

	if(outcome) {
		return false;
	}

//	stacie_realm_cipher_key_derive

	key = PLACER("KEY12345KEY67890KEY12345KEY67890KEY12345KEY67890KEY12345KEY67890", 64);
	
	if(!(res1 = stacie_realm_cipher_key_derive(key)) || !(res2 = stacie_realm_cipher_key_derive(key))) {
		return false;
	}

	outcome = st_cmp_cs_eq(res1, res2);
	st_free(res1);
	st_free(res2);

	if(outcome) {
		return false;
	}
	
	if(!(res1 = stacie_realm_init_vector_derive(key)) || !(res2 = stacie_realm_init_vector_derive(key))) {
		return false;
	}

	outcome = st_cmp_cs_eq(res1, res2);
	st_free(res1);
	st_free(res2);

	if(outcome) {
		return false;
	}
	
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

