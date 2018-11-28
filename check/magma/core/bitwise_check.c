
/**
 * @file /check/magma/core/bitwise_check.c
 *
 * @brief Base64 encoder unit tests.
 *
 *
*/

#include "magma_check.h"


bool_t check_bitwise_simple(void) {

	stringer_t *a, *b, *outcome, *check;
	unsigned char    a_buf[] = {0x00, 0xFF, 0x88, 0x44, 0x22};
	unsigned char    b_buf[] = {0x11, 0x33, 0xF0, 0x0F, 0x3C};
	unsigned char  xor_buf[] = {0x11, 0xCC, 0x78, 0x4B, 0x1E};
	unsigned char  and_buf[] = {0x00, 0x33, 0x80, 0x04, 0x20};
	unsigned char   or_buf[] = {0x11, 0xFF, 0xF8, 0x4F, 0x3E};
	unsigned char nota_buf[] = {0xFF, 0x00, 0x77, 0xBB, 0xDD};

	a = PLACER(a_buf, 5);
	b = PLACER(b_buf, 5);
	check = PLACER(xor_buf, 5);

	if(!(outcome = st_xor(a, b, NULL)) || st_cmp_cs_eq(outcome, check)) {
		st_cleanup(outcome);
		return false;
	}

	check = PLACER(and_buf, 5);
	st_free(outcome);

	if(!(outcome = st_and(a, b, NULL)) || st_cmp_cs_eq(outcome, check)) {
		st_cleanup(outcome);
		return false;
	}

	check = PLACER(or_buf, 5);
	st_free(outcome);

	if(!(outcome = st_or(a, b, NULL)) || st_cmp_cs_eq(outcome, check)) {
		st_cleanup(outcome);
		return false;
	}

	check = PLACER(nota_buf, 5);
	st_free(outcome);

	if(!(outcome = st_not(a, NULL)) || st_cmp_cs_eq(outcome,check)) {
		st_cleanup(outcome);
		return false;
	}

	st_free(outcome);

	return true;
}

bool_t check_bitwise_determinism(void) {

	stringer_t *a = NULL, *b = NULL, *res1 = NULL, *res2 = NULL;

	unsigned char a_buf[] = {0x00, 0x33, 0x80, 0x04, 0x20};
	unsigned char b_buf[] = {0x11, 0xCC, 0x78, 0x4B, 0x1E};

	a = PLACER(a_buf, 5);
	b = PLACER(b_buf, 5);

	if(!(res1 = st_xor(a, b, NULL)) || !(res2 = st_xor(a, b, NULL))) {
		st_cleanup(res1, res2);
		return false;
	}
	else if(st_cmp_cs_eq(res1, res2)) {
		st_cleanup(res1, res2);
		return false;
	}

	st_cleanup(res1, res2);
	res1 = res2 = NULL;

	if(!(res1 = st_and(a, b, NULL)) || !(res2 = st_and(a, b, NULL))) {
		st_cleanup(res1, res2);
		return false;
	}
	else if(st_cmp_cs_eq(res1, res2)) {
		st_cleanup(res1, res2);
		return false;
	}

	st_cleanup(res1, res2);
	res1 = res2 = NULL;

	if(!(res1 = st_or(a, b, NULL)) || !(res2 = st_or(a, b, NULL))) {
		st_cleanup(res1, res2);
		return false;
	}
	else if(st_cmp_cs_eq(res1, res2)) {
		st_cleanup(res1, res2);
		return false;
	}

	st_cleanup(res1, res2);
	res1 = res2 = NULL;

	if(!(res1 = st_not(a, NULL)) || !(res2 = st_not(a, NULL))) {
		st_cleanup(res1, res2);
		return false;
	}
	else if(st_cmp_cs_eq(res1, res2)) {
		st_cleanup(res1, res2);
		return false;
	}

	st_cleanup(res1, res2);
	return true;
}

bool_t check_bitwise_parameters(void) {

	stringer_t *a, *b, *outcome, *st_short, *res;

	unsigned char a_buf[] = {0x00, 0x33, 0x80, 0x04, 0x20};
	unsigned char b_buf[] = {0x11, 0xCC, 0x78, 0x4B, 0x1E};

	a = PLACER((unsigned char *)a_buf, 5);
	b = PLACER((unsigned char *)b_buf, 5);
	st_short = PLACER(a,2);

	outcome = st_alloc(5);

	if((res = st_xor(NULL, b, NULL))) {
		st_free(res);
		return false;
	}

	if((res = st_xor(NULL, b, outcome))) {
		st_free(res);
		return false;
	}

	if((res = st_xor(a, NULL, NULL))) {
		st_free(res);
		return false;
	}

	if((res = st_xor(a, NULL, outcome))) {
		st_free(res);
		return false;
	}

	if((res = st_xor(a, b, st_short))) {
		st_free(res);
		return false;
	}

	if((res = st_and(NULL, b, NULL))) {
		st_free(res);
		return false;
	}

	if((res = st_and(NULL, b, outcome))) {
		st_free(res);
		return false;
	}

	if((res = st_and(a, NULL, NULL))) {
		st_free(res);
		return false;
	}

	if((res = st_and(a, NULL, outcome))) {
		st_free(res);
		return false;
	}

	if((res = st_and(a, b, st_short))) {
		st_free(res);
		return false;
	}

	if((res = st_or(NULL, b, NULL))) {
		st_free(res);
		return false;
	}

	if((res = st_or(NULL, b, outcome))) {
		st_free(res);
		return false;
	}

	if((res = st_or(a, NULL, NULL))) {
		st_free(res);
		return false;
	}

	if((res = st_or(a, NULL, outcome))) {
		st_free(res);
		return false;
	}

	if((res = st_or(a, b, st_short))) {
		st_free(res);
		return false;
	}

	if((res = st_not(NULL, NULL))) {
		st_free(res);
		return false;
	}

	if((res = st_not(NULL, outcome))) {
		st_free(res);
		return false;
	}

	if((res = st_not(a, st_short))) {
		st_free(res);
		return false;
	}

	st_free(outcome);
	return true;
}
