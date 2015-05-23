
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


