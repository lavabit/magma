
/**
 * @file /check/magma/core/url_check.c
 *
 * @brief URL encoder unit tests.
 */

#include "magma_check.h"

bool_t check_encoding_url(void) {

	bool_t result = true;
	stringer_t *url, *binary;
	byte_t buffer[URL_CHECK_SIZE];

	for (uint64_t i = 0; status() && result && i < URL_CHECK_ITERATIONS; i++) {

		// Fill the buffer with random data and convert the buffer to hex.
		if (rand_write(PLACER(buffer, URL_CHECK_SIZE)) != URL_CHECK_SIZE) {
			return false;
		}
		else if (!(url = url_encode(PLACER(buffer, URL_CHECK_SIZE)))) {
			return false;
		}
		else if (!url_valid_st(url)) {
			return false;
		}

		// Convert the buffer back to binary and compare it with the original array.
		if (!(binary = url_decode(url))) {
			st_free(url);
			return false;
		}
		else if (st_cmp_cs_eq(binary, PLACER(buffer, URL_CHECK_SIZE))) {
			result = false;
		}

		//log_pedantic("%-15.15s = %.*s", "plain", URL_CHECK_SIZE, buffer);
		//log_pedantic("%-15.15s = %.*s", "url", st_length_int(url), st_char_get(url));
		//log_pedantic("%-15.15s = %.*s", "decoded", st_length_int(binary), st_char_get(binary));

		st_free(binary);
		st_free(url);
	}

	return result;
}

