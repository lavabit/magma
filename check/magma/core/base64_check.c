
/**
 * @file /check/magma/core/base64_check.c
 *
 * @brief Base64 encoder unit tests.
 */

#include "magma_check.h"

bool_t check_encoding_base64(bool_t secure_on) {

	stringer_t *b64, *binary;
	byte_t buffer[BASE64_CHECK_SIZE];
#ifdef MAGMA_CHECK_H
	for (uint64_t i = 0; status() && i < BASE64_CHECK_ITERATIONS; i++) {
#else
		for (uint64_t i = 0;  i < BASE64_CHECK_ITERATIONS; i++) {
#endif
		// Fill the buffer with random data and convert the buffer to hex.
		if (rand_write(PLACER(buffer, BASE64_CHECK_SIZE)) != BASE64_CHECK_SIZE) {
			return false;
		}

		if (!secure_on && (!(b64 = base64_encode(PLACER(buffer, BASE64_CHECK_SIZE),NULL)))) {
			return false;
		} else if (secure_on && (!(b64 = base64_encode_opts(PLACER(buffer, BASE64_CHECK_SIZE),MANAGED_T | CONTIGUOUS | SECURE,false)))) {
			return false;
		}

		//log_pedantic("base64 = %.*s", st_length_int(hex), st_char_get(hex));

		// Convert the buffer back to binary and compare it with the original array.
		if (!secure_on && (!(binary = base64_decode(b64,NULL)))) {
			st_free(b64);
			return false;
		}
		else if (secure_on && (!(binary = base64_decode_opts(b64,MANAGED_T | CONTIGUOUS | SECURE,false)))) {
			st_free(b64);
			return false;
		}
		else if (st_cmp_cs_eq(binary, PLACER(buffer, BASE64_CHECK_SIZE))) {
			st_free(binary);
			st_free(b64);
			return false;
		}

		st_free(binary);
		st_free(b64);
	}

	return true;
}

bool_t check_encoding_base64_mod(bool_t secure_on) {

	stringer_t *b64, *binary;
	byte_t buffer[BASE64_CHECK_SIZE];

	for (uint64_t i = 0; status() && i < BASE64_CHECK_ITERATIONS; i++) {

		// Fill the buffer with random data and convert the buffer to hex.
		if (rand_write(PLACER(buffer, BASE64_CHECK_SIZE)) != BASE64_CHECK_SIZE) {
			return false;
		}

		if (!secure_on && (!(b64 = base64_encode_mod(PLACER(buffer, BASE64_CHECK_SIZE),NULL)))) {
			return false;
		} else if (secure_on && (!(b64 = base64_encode_opts(PLACER(buffer, BASE64_CHECK_SIZE),MANAGED_T | CONTIGUOUS | SECURE,true)))) {
			return false;
		}

		//log_pedantic("base64mod = %.*s", st_length_int(b64), st_char_get(b64));

		// Convert the buffer back to binary and compare it with the original array.
		if (!secure_on && (!(binary = base64_decode_mod(b64,NULL)))) {
			st_free(b64);
			return false;
		}
		else if (secure_on && (!(binary = base64_decode_opts(b64,MANAGED_T | CONTIGUOUS | SECURE,true)))) {
			st_free(b64);
			return false;
		}
		else if (st_cmp_cs_eq(binary, PLACER(buffer, BASE64_CHECK_SIZE))) {
			st_free(binary);
			st_free(b64);
			return false;
		}

		st_free(binary);
		st_free(b64);
	}

	return true;
}
