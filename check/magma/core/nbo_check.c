
/**
 * @file /check/magma/core/nbo_check.c
 *
 * @brief Network byte order tests.
 */

#include "magma_check.h"

bool_t check_nbo_simple(void) {

	stringer_t *s;
	uint32_t b32 = 0xABCDEF01;
	uint32_t b24 = 0x00ABCDEF;
	uint16_t b16 = 0x0000ABCD;

	if(!(s = uint32_put_no(b32))) {
		return false;
	}
	else if(uint32_get_no(s) != b32) {
		return false;
	}

	st_free(s);

	if(!(s = uint24_put_no(b24))) {
		return false;
	}
	else if(uint24_get_no(s) != b24) {
		return false;
	}

	st_free(s);

	if(!(s = uint16_put_no(b16))) {
		return false;
	}
	else if(uint16_get_no(s) != b16) {
		return false;
	}

	st_free(s);

	return true;
}

bool_t check_nbo_parameters(void) {

	stringer_t *small_st, *null_st = NULL;

	small_st = st_alloc(1);

	if(uint32_get_no(small_st)) {
		st_free(small_st);
		return false;
	}

	if(uint32_get_no(null_st)) {
		st_free(small_st);
		return false;
	}

	if(uint24_get_no(small_st)) {
		st_free(small_st);
		return false;
	}

	if(uint24_get_no(null_st)) {
		st_free(small_st);
		return false;
	}

	if(uint16_get_no(small_st)) {
		st_free(small_st);
		return false;
	}

	if(uint16_get_no(null_st)) {
		st_free(small_st);
		return false;
	}

	st_free(small_st);
	return true;
}
