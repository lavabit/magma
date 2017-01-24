
/**
 * @file /check/magma/core/hex_check.c
 *
 * @brief Hex encoder unit tests.
 */

#include "magma_check.h"

bool_t check_encoding_hex(void) {

	stringer_t *hex, *binary;
	byte_t buffer[HEX_CHECK_SIZE];

	for (uint64_t i = 0; status() && i < HEX_CHECK_ITERATIONS; i++) {

		// Fill the buffer with random data and convert the buffer to hex.
		if (rand_write(PLACER(buffer, HEX_CHECK_SIZE)) != HEX_CHECK_SIZE) {
			return false;
		}
		else if (!(hex = hex_encode_st(PLACER(buffer, HEX_CHECK_SIZE), NULL))) {
			return false;
		}
		else if (!hex_valid_st(hex)) {
			return false;
		}

		//log_pedantic("hex = %.*s", st_length_int(hex), st_char_get(hex));

		// Convert the buffer back to binary and compare it with the original array.
		if (!(binary = hex_decode_st(hex, NULL))) {
			st_free(hex);
			return false;
		}
		else if (st_cmp_cs_eq(binary, PLACER(buffer, HEX_CHECK_SIZE))) {
			st_free(binary);
			st_free(hex);
			return false;
		}

		st_free(binary);
		st_free(hex);
	}

	return true;
}
