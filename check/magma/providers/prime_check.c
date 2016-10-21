
/**
 * @file /magma/check/magma/providers/prime_check.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma_check.h"

#define check_prime_secp256k1_cleanup(key, pub, priv) ({ st_cleanup(pub, priv); secp256k1_free(key); priv = pub = NULL; key = NULL; })

bool_t check_prime_secp256k1_sthread(stringer_t *errmsg) {

	uchr_t y;
	EC_KEY *key = NULL;
	stringer_t *priv = NULL, *pub = NULL;

	for (uint64_t i = 0; status() && i < PRIME_CHECK_ITERATIONS; i++) {

		// Generate a new key pair.
		if (!(key = secp256k1_generate())) {
			st_sprint(errmsg, "Curve secp256k1 key generation failed.");
			return false;
		}
		// Extract the public and private components.
		else if (!(pub = secp256k1_public_get(key, NULL)) || !(priv = secp256k1_private_get(key, NULL))) {
			check_prime_secp256k1_cleanup(key, pub, priv);
			st_sprint(errmsg, "Curve secp256k1 exponent serialization failed.");
			return false;
		}

		// Confirm the serialized output is the correct length.
		if (st_length_get(priv) != 32 || st_length_get(pub) != 33) {
			check_prime_secp256k1_cleanup(key, pub, priv);
			st_sprint(errmsg, "The serialized secp256k1 keys are not the expected length.");
			return false;
		}

		// Confirm the octet stream starts with 0x02 or 0x03, in accordance with the compressed point representation
		// described by ANSI standard X9.62 section 4.3.6.
		y = *((uchr_t *)st_data_get(pub));
		if (y != 2 && y != 3) {
			check_prime_secp256k1_cleanup(key, pub, priv);
			st_sprint(errmsg, "Curve secp256k1 public key point does not appear to be compressed properly.");
			return false;
		}

		//more checks
		check_prime_secp256k1_cleanup(key, pub, priv);
	}

	return true;
}
//
//
//bool_t check_ecies_sthread(void) {
//
//	int tlen;
//	size_t olen;
//	EC_KEY *key = NULL;
//	cryptex_t *ciphered = NULL;//
//
//bool_t check_ecies_sthread(void) {
//
//	int tlen;
//	size_t olen;
//	EC_KEY *key = NULL;
//	cryptex_t *ciphered = NULL;
//	stringer_t *hex_priv = NULL, *hex_pub = NULL;
//	unsigned char *text = NULL, *copy = NULL, *original = NULL;
//
//	for (uint64_t r = 0; status() && r < ECIES_CHECK_ITERATIONS; r++) {
//
//		// Generate random size for the block of data were going to encrypt. Use a min value of 1 MB and a max of 10 MB.
//		do {
//			tlen = (rand() % (ECIES_CHECK_SIZE_MAX - ECIES_CHECK_SIZE_MIN)) + ECIES_CHECK_SIZE_MIN;
//		} while (tlen < ECIES_CHECK_SIZE_MIN);
//
//		if (!(text = malloc(tlen + 1)) || !(copy = malloc(tlen + 1))) {
//			printf("Memory error.\n");
//			check_ecies_cleanup(key, ciphered, hex_pub, hex_priv, text, copy, original);
//			return false;
//		}
//
//		// Wipe and then fill the data blocks with pseudo-random data.
//		memset(copy, 0, tlen + 1);
//		memset(text, 0, tlen + 1);
//
//		for (uint64_t j = 0; j < tlen; j++) {
//			copy[j] = text[j] = rand() % 256;
//		}
//
//		// Generate a key for our theoretical user.
//		if (!(key = ecies_key_create())) {
//			printf("Key creation failed.\n");
//			check_ecies_cleanup(key, ciphered, hex_pub, hex_priv, text, copy, original);
//			return false;
//		}
//
//		// Since we'll store the keys as hex values in real life, extract the appropriate hex values and release the original key structure.
//		if (!(hex_pub = ecies_key_public_hex(key)) || !(hex_priv = ecies_key_private_hex(key))) {
//			printf("Serialization of the key to a pair of hex strings failed.\n");
//			check_ecies_cleanup(key, ciphered, hex_pub, hex_priv, text, copy, original);
//			return false;
//		}
//
//		if (!(ciphered = ecies_encrypt(hex_pub, ECIES_PUBLIC_HEX, text, tlen))) {
//			printf("The encryption process failed!\n");
//			check_ecies_cleanup(key, ciphered, hex_pub, hex_priv, text, copy, original);
//			return false;
//		}
//
//		if (!(original = ecies_decrypt(hex_priv, ECIES_PRIVATE_HEX, ciphered, &olen))) {
//			printf("The decryption process failed!\n");
//			check_ecies_cleanup(key, ciphered, hex_pub, hex_priv, text, copy, original);
//			return false;
//		}
//
//		if (olen != tlen || memcmp(original, copy, tlen)) {
//			printf("Comparison failure.\n");
//			check_ecies_cleanup(key, ciphered, hex_pub, hex_priv, text, copy, original);
//			return false;
//		}
//
//		check_ecies_cleanup(key, ciphered, hex_pub, hex_priv, text, copy, original);
//	}
//
//	return true;
//}
//	stringer_t *hex_priv = NULL, *hex_pub = NULL;//
//
//bool_t check_ecies_sthread(void) {
//
//	int tlen;
//	size_t olen;
//	EC_KEY *key = NULL;
//	cryptex_t *ciphered = NULL;
//	stringer_t *hex_priv = NULL, *hex_pub = NULL;
//	unsigned char *text = NULL, *copy = NULL, *original = NULL;
//
//	for (uint64_t r = 0; status() && r < ECIES_CHECK_ITERATIONS; r++) {
//
//		// Generate random size for the block of data were going to encrypt. Use a min value of 1 MB and a max of 10 MB.
//		do {
//			tlen = (rand() % (ECIES_CHECK_SIZE_MAX - ECIES_CHECK_SIZE_MIN)) + ECIES_CHECK_SIZE_MIN;
//		} while (tlen < ECIES_CHECK_SIZE_MIN);
//
//		if (!(text = malloc(tlen + 1)) || !(copy = malloc(tlen + 1))) {
//			printf("Memory error.\n");
//			check_ecies_cleanup(key, ciphered, hex_pub, hex_priv, text, copy, original);
//			return false;
//		}
//
//		// Wipe and then fill the data blocks with pseudo-random data.
//		memset(copy, 0, tlen + 1);
//		memset(text, 0, tlen + 1);
//
//		for (uint64_t j = 0; j < tlen; j++) {
//			copy[j] = text[j] = rand() % 256;
//		}
//
//		// Generate a key for our theoretical user.
//		if (!(key = ecies_key_create())) {
//			printf("Key creation failed.\n");
//			check_ecies_cleanup(key, ciphered, hex_pub, hex_priv, text, copy, original);
//			return false;
//		}
//
//		// Since we'll store the keys as hex values in real life, extract the appropriate hex values and release the original key structure.
//		if (!(hex_pub = ecies_key_public_hex(key)) || !(hex_priv = ecies_key_private_hex(key))) {
//			printf("Serialization of the key to a pair of hex strings failed.\n");
//			check_ecies_cleanup(key, ciphered, hex_pub, hex_priv, text, copy, original);
//			return false;
//		}
//
//		if (!(ciphered = ecies_encrypt(hex_pub, ECIES_PUBLIC_HEX, text, tlen))) {
//			printf("The encryption process failed!\n");
//			check_ecies_cleanup(key, ciphered, hex_pub, hex_priv, text, copy, original);
//			return false;
//		}
//
//		if (!(original = ecies_decrypt(hex_priv, ECIES_PRIVATE_HEX, ciphered, &olen))) {
//			printf("The decryption process failed!\n");
//			check_ecies_cleanup(key, ciphered, hex_pub, hex_priv, text, copy, original);
//			return false;
//		}
//
//		if (olen != tlen || memcmp(original, copy, tlen)) {
//			printf("Comparison failure.\n");
//			check_ecies_cleanup(key, ciphered, hex_pub, hex_priv, text, copy, original);
//			return false;
//		}
//
//		check_ecies_cleanup(key, ciphered, hex_pub, hex_priv, text, copy, original);
//	}
//
//	return true;
//}
//	unsigned char *text = NULL, *copy = NULL, *original = NULL;
//
//	for (uint64_t r = 0; status() && r < ECIES_CHECK_ITERATIONS; r++) {
//
//		// Generate random size for the block of data were going to encrypt. Use a min value of 1 MB and a max of 10 MB.
//		do {
//			tlen = (rand() % (ECIES_CHECK_SIZE_MAX - ECIES_CHECK_SIZE_MIN)) + ECIES_CHECK_SIZE_MIN;
//		} while (tlen < ECIES_CHECK_SIZE_MIN);
//
//		if (!(text = malloc(tlen + 1)) || !(copy = malloc(tlen + 1))) {
//			printf("Memory error.\n");
//			check_ecies_cleanup(key, ciphered, hex_pub, hex_priv, text, copy, original);
//			return false;
//		}
//
//		// Wipe and then fill the data blocks with pseudo-random data.
//		memset(copy, 0, tlen + 1);
//		memset(text, 0, tlen + 1);
//
//		for (uint64_t j = 0; j < tlen; j++) {
//			copy[j] = text[j] = rand() % 256;
//		}
//
//		// Generate a key for our theoretical user.
//		if (!(key = ecies_key_create())) {
//			printf("Key creation failed.\n");
//			check_ecies_cleanup(key, ciphered, hex_pub, hex_priv, text, copy, original);
//			return false;
//		}
//
//		// Since we'll store the keys as hex values in real life, extract the appropriate hex values and release the original key structure.
//		if (!(hex_pub = ecies_key_public_hex(key)) || !(hex_priv = ecies_key_private_hex(key))) {
//			printf("Serialization of the key to a pair of hex strings failed.\n");
//			check_ecies_cleanup(key, ciphered, hex_pub, hex_priv, text, copy, original);
//			return false;
//		}
//
//		if (!(ciphered = ecies_encrypt(hex_pub, ECIES_PUBLIC_HEX, text, tlen))) {
//			printf("The encryption process failed!\n");
//			check_ecies_cleanup(key, ciphered, hex_pub, hex_priv, text, copy, original);
//			return false;
//		}
//
//		if (!(original = ecies_decrypt(hex_priv, ECIES_PRIVATE_HEX, ciphered, &olen))) {
//			printf("The decryption process failed!\n");
//			check_ecies_cleanup(key, ciphered, hex_pub, hex_priv, text, copy, original);
//			return false;
//		}
//
//		if (olen != tlen || memcmp(original, copy, tlen)) {
//			printf("Comparison failure.\n");
//			check_ecies_cleanup(key, ciphered, hex_pub, hex_priv, text, copy, original);
//			return false;
//		}
//
//		check_ecies_cleanup(key, ciphered, hex_pub, hex_priv, text, copy, original);
//	}
//
//	return true;
//}
