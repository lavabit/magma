
/**
 * @file /check/providers/ecies_check.c
 *
 * @brief The heart of the suite of unit tests for the Magma provide module.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma_check.h"

void check_ecies_cleanup(EC_KEY *key, cryptex_t *ciphered, stringer_t *hex_pub, stringer_t *hex_priv, unsigned char *text, unsigned char *copy, unsigned char *original) {

	if (key) {
		ecies_key_free(key);
	}

	if (ciphered) {
		cryptex_free(ciphered);
	}

	st_cleanup(hex_pub);
	st_cleanup(hex_priv);

	if (text) {
		free(text);
	}

	if (copy) {
		free(copy);
	}

	if (original) {
		free(original);
	}

	return;
}

bool_t check_ecies_sthread(void) {

	int tlen;
	size_t olen;
	EC_KEY *key = NULL;
	cryptex_t *ciphered = NULL;
	stringer_t *hex_priv = NULL, *hex_pub = NULL;
	unsigned char *text = NULL, *copy = NULL, *original = NULL;

	for (uint64_t r = 0; status() && r < ECIES_CHECK_ITERATIONS; r++) {

		// Generate random size for the block of data were going to encrypt. Use a min value of 1 MB and a max of 10 MB.
		do {
			tlen = (rand() % (ECIES_CHECK_SIZE_MAX - ECIES_CHECK_SIZE_MIN)) + ECIES_CHECK_SIZE_MIN;
		} while (tlen < ECIES_CHECK_SIZE_MIN);

		if (!(text = malloc(tlen + 1)) || !(copy = malloc(tlen + 1))) {
			printf("Memory error.\n");
			check_ecies_cleanup(key, ciphered, hex_pub, hex_priv, text, copy, original);
			return false;
		}

		// Wipe and then fill the data blocks with pseudo-random data.
		memset(copy, 0, tlen + 1);
		memset(text, 0, tlen + 1);

		for (uint64_t j = 0; j < tlen; j++) {
			copy[j] = text[j] = rand() % 256;
		}

		// Generate a key for our theoretical user.
		if (!(key = ecies_key_create())) {
			printf("Key creation failed.\n");
			check_ecies_cleanup(key, ciphered, hex_pub, hex_priv, text, copy, original);
			return false;
		}

		// Since we'll store the keys as hex values in real life, extract the appropriate hex values and release the original key structure.
		if (!(hex_pub = ecies_key_public_hex(key)) || !(hex_priv = ecies_key_private_hex(key))) {
			printf("Serialization of the key to a pair of hex strings failed.\n");
			check_ecies_cleanup(key, ciphered, hex_pub, hex_priv, text, copy, original);
			return false;
		}

		if (!(ciphered = ecies_encrypt(hex_pub, ECIES_PUBLIC_HEX, text, tlen))) {
			printf("The encryption process failed!\n");
			check_ecies_cleanup(key, ciphered, hex_pub, hex_priv, text, copy, original);
			return false;
		}

		if (!(original = ecies_decrypt(hex_priv, ECIES_PRIVATE_HEX, ciphered, &olen))) {
			printf("The decryption process failed!\n");
			check_ecies_cleanup(key, ciphered, hex_pub, hex_priv, text, copy, original);
			return false;
		}

		if (olen != tlen || memcmp(original, copy, tlen)) {
			printf("Comparison failure.\n");
			check_ecies_cleanup(key, ciphered, hex_pub, hex_priv, text, copy, original);
			return false;
		}

		check_ecies_cleanup(key, ciphered, hex_pub, hex_priv, text, copy, original);
	}

	return true;
}
