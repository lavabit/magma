
/**
 * @file /magma.check/providers/symmetric_check.c
 *
 * @brief The logic used to test the symmetric cipher functions.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma_check.h"

bool_t check_symmetric_sthread(chr_t *name) {

	size_t len;
	cipher_t *cipher;
	int_t klen, vlen;
	bool_t result = true;
	stringer_t *key, *vector, *encrypted, *decrypted;
	byte_t buffer[SYMMETRIC_CHECK_SIZE_MAX];

	for (uint64_t i = 0; status() && i < SYMMETRIC_CHECK_ITERATIONS; i++) {

		encrypted = decrypted = vector = key = NULL;

		// Pick a random length.
		len = (rand() % (SYMMETRIC_CHECK_SIZE_MAX - SYMMETRIC_CHECK_SIZE_MIN)) + SYMMETRIC_CHECK_SIZE_MIN;

		// Fill the buffer with random data and convert the buffer to encrypted.
		if (rand_write(PLACER(buffer, len)) != len) {
			return false;
		}
		else if (!(cipher = cipher_name(NULLER(name))) || (vlen = cipher_vector_length(cipher)) < 0 ||	(klen = cipher_key_length(cipher)) <= 0) {
			return false;
		}
		else if (!(key = rand_choices("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", klen))) {
			return false;
		}
		else if (vlen && !(vector = rand_choices("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", vlen))) {
			st_free(key);
			return false;
		}
		else if (!(encrypted = symmetric_encrypt(cipher, vector, key, PLACER(buffer, len)))) {
			if (vector) {
				st_free(vector);
			}
			st_free(key);
			return false;
		}

		// Convert the buffer back to binary and compare it with the original array.
		if (!(decrypted = symmetric_decrypt(cipher, vector, key, encrypted))) {
			if (vector) {
				st_free(vector);
			}
			st_free(encrypted);
			st_free(key);
			return false;
		}
		else if (st_cmp_cs_eq(decrypted, PLACER(buffer, len))) {
			result = false;
		}


		/*stringer_t *hex[3] = { hex_encode_st(PLACER(buffer, len), NULL),	hex_encode_st(encrypted, NULL), hex_encode_st(decrypted, NULL) };
		log_pedantic("%-15.15s = %.*s", "plain", st_length_int(hex[0]), st_char_get(hex[0]));
		log_pedantic("%-15.15s = %.*s", "encrypted", st_length_int(hex[1]), st_char_get(hex[1]));
		log_pedantic("%-15.15s = %.*s", "decrypted", st_length_int(hex[2]), st_char_get(hex[2]));
		st_free(hex[0]); st_free(hex[1]);	st_free(hex[2]);*/


		if (vector) {
			st_free(vector);
		}

		st_free(encrypted);
		st_free(decrypted);
		st_free(key);
	}

	return true;
}
