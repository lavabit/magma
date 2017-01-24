
/**
 * @file /check/magma/core/zbase32_check.c
 *
 * @brief Zbase32 encoder unit tests.
 */

#include "magma_check.h"

bool_t check_encoding_zbase32(void) {

	stringer_t *zb32, *binary;
	byte_t buffer[ZBASE32_CHECK_SIZE];

	for (uint64_t i = 0; status() && i < ZBASE32_CHECK_ITERATIONS; i++) {

		// Fill the buffer with random data and convert the buffer to hex.
		if (rand_write(PLACER(buffer, ZBASE32_CHECK_SIZE)) != ZBASE32_CHECK_SIZE) {
			return false;
		}
		else if (!(zb32 = zbase32_encode(PLACER(buffer, ZBASE32_CHECK_SIZE)))) {
			return false;
		}

		//log_pedantic("zb32 = %.*s", st_length_int(zb32), st_char_get(zb32));

		// Convert the buffer back to binary and compare it with the original array.
		if (!(binary = zbase32_decode(zb32))) {
			st_free(zb32);
			return false;
		}
		else if (st_cmp_cs_eq(binary, PLACER(buffer, ZBASE32_CHECK_SIZE))) {
			st_free(binary);
			st_free(zb32);
			return false;
		}

		st_free(binary);
		st_free(zb32);
	}

	return true;
}

