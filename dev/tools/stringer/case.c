/**
 * @file /stringer/case.c
 *
 * @brief A collection of functions used for manipulating the capitalization of characters.
 *
 * $Author: $
 * $Date: $
 * $Revision: a46af76362b976a414117a5bfee9dfc2e9fd38b2 $
 *
 */

#include "stringer.h"

void st_lowercase(stringer_t *s) {

	size_t len;
	uchr_t *ptr;

	if (st_empty_out(s, &ptr, &len)) {
		log_pedantic("Passed in a NULL pointer or zero length string.");
		return;
	}

	for (size_t i = 0; i < len; i++) {
		if (*ptr >= 'A' && *ptr <= 'Z') *ptr += 32;
		ptr++;
	}

	return;
}

void mm_lowercase(void *mem, size_t len) {

	uchr_t *local = (uchr_t *)mem;

	if (!mem || !len) {
		log_pedantic("Passed in a NULL pointer or zero length string.");
		return;
	}

	for (size_t i = 0; i < len; i++) {
		if (*local >= 'A' && *local <= 'Z') *local += 32;
		local++;
	}

	return;
}

uchr_t c_lowercase(uchr_t c) {
	if (c >= 'A' && c <= 'Z') c += 32;
	return c;
}

void st_uppercase(stringer_t *s) {

	size_t len;
	uchr_t *ptr;

	if (st_empty_out(s, &ptr, &len)) {
		log_pedantic("Passed in a NULL pointer or zero length string.");
		return;
	}

	for (size_t i = 0; i < len; i++) {
		if (*ptr >= 'a' && *ptr <= 'z') *ptr -= 32;
		ptr++;
	}

	return;
}

void mm_uppercase(void *mem, size_t len) {

	uchr_t *local = (uchr_t *)mem;

	if (!mem || !len) {
		log_pedantic("Passed in a NULL pointer or zero length string.");
		return;
	}

	for (size_t i = 0; i < len; i++) {
		if (*((uchr_t *)mem) >= 'a' && *((uchr_t *)mem) <= 'z')	*((uchr_t *)mem) -= 32;
		local++;
	}

	return;
}

uchr_t c_uppercase(uchr_t c) {
	if (c >= 'z' && c <= 'z') c -= 32;
	return c;
}
