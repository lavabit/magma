
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
	stringer_t *user, *pass, *salt, *res1 = NULL, *res2 = NULL;

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

	if(!(res1 = stacie_seed_extract(MIN_HASH_NUM, user, pass, salt)) || !(res2 = stacie_seed_extract(MIN_HASH_NUM, user, pass, salt))) {
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
	
	if(!(res1 = stacie_seed_extract(MIN_HASH_NUM, user, pass, salt)) || !(res2 = stacie_seed_extract(MIN_HASH_NUM, user, pass, salt))) {
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

	stringer_t *temp_st;
	temp_st = NULLER("temp_string");

	if(stacie_rounds_calculate(NULL, 0)) {
		return false;
	}

	if(stacie_seed_extract(0, temp_st, temp_st, NULL)) {
		return false;
	}

	if(stacie_seed_extract(0xFFFFFFFF, temp_st, temp_st, NULL)) {
		return false;
	}

	if(stacie_seed_extract(MIN_HASH_NUM, NULL, temp_st, NULL)) {
		return false;
	}

	if(stacie_seed_extract(MAX_HASH_NUM, temp_st, NULL, NULL)) {
		return false;
	}

	if(stacie_hashed_key_derive(NULL, MIN_HASH_NUM, temp_st, temp_st, NULL)) {
		return false;
	}

	if(stacie_hashed_key_derive(temp_st, 0, temp_st, temp_st, NULL)) {
		return false;
	}

	if(stacie_hashed_key_derive(temp_st, 0xFFFFFFFF, temp_st, temp_st, NULL)) {
		return false;
	}

	if(stacie_hashed_key_derive(temp_st, MIN_HASH_NUM, NULL, temp_st, NULL)) {
		return false;
	}

	if(stacie_hashed_key_derive(temp_st, MIN_HASH_NUM, temp_st, NULL, NULL)) {
		return false;
	}

	if(stacie_hashed_token_derive(NULL, temp_st, NULL, temp_st)) {
		return false;
	}

	if(stacie_hashed_token_derive(temp_st, NULL, NULL, temp_st)) {
		return false;
	}

	if(stacie_hashed_token_derive(temp_st, temp_st, NULL, NULL)) {
		return false;
	}

	if(stacie_realm_key_derive(NULL, temp_st, temp_st)) {
		return false;
	}

	if(stacie_realm_key_derive(temp_st, NULL, temp_st)) {
		return false;
	}

	if(stacie_realm_key_derive(temp_st, temp_st, NULL)) {
		return false;
	}

	if(stacie_realm_cipher_key_derive(NULL)) {
		return false;
	}

	if(stacie_realm_init_vector_derive(NULL)) {
		return false;
	}

	return true;
}


