
/**
 * @file /check/providers/scramble_check.c
 *
 * @brief Check the scrambler.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma_check.h"

bool_t check_scramble_sthread(void) {

	size_t rlen;
	scramble_t *scramble;
	stringer_t *output, *key = MANAGEDBUF(512);
	unsigned char *original;

	for (uint64_t i = 0; status() && i < SCRAMBLE_CHECK_ITERATIONS; i++) {

		// Pick a random length.
		do {
			rlen = (rand() % (SCRAMBLE_CHECK_SIZE_MAX - SCRAMBLE_CHECK_SIZE_MIN)) + SCRAMBLE_CHECK_SIZE_MIN;
		} while (rlen < SCRAMBLE_CHECK_SIZE_MIN);

		if (!(original = mm_alloc(rlen))) {
			return false;
		}

		// Fill it with random data.
		for (uint64_t j = 0; j < rlen; j++) {
			original[j] = rand() % 256;
		}

		// Generate a random key.
		rand_write(key);
		st_length_set(key, 128 + (rand() % 384));

		// Encrypt the data block.
		if (!(scramble = scramble_encrypt(key, PLACER(original, rlen)))) {
			mm_free(original);
			return false;
		}

		// Decrypt the data block and verify.
		if (!(output = scramble_decrypt(key, scramble))) {
			scramble_free(scramble);
			mm_free(original);
			return false;
		}

		// Verify the output is identical to the input.
		if (st_length_get(output) != rlen || memcmp(st_data_get(output), original, rlen)) {
			scramble_free(scramble);
			mm_free(original);
			st_free(output);
			return false;
		}

		scramble_free(scramble);
		mm_free(original);
		st_free(output);
	}

	return true;
}
