
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
/*
	if(!(res1 = stacie_seed_extract(MAX_HASH_NUM, user, pass, salt)) || !(res2 = stacie_seed_extract(MAX_HASH_NUM, user, pass, salt))) {
		return false;
	}

	outcome = st_cmp_cs_eq(res1, res2);
	st_free(res1);
	st_free(res2);

	if(outcome) {
		return false;
	}
*/
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
/*
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

	stringer_t *temp_st, *res, *temp_st64;
	temp_st = NULLER("temp_string");
	temp_st64 = NULLER("TEMP1234TEMP5678TEMP9012TEMP3456TEMP7890TEMP1234TEMP5678TEMP9012");

	if(stacie_rounds_calculate(NULL, 0)) {
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

