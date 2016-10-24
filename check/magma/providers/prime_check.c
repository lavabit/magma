
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

#define check_prime_secp256k1_keys_cleanup(key, holder, pub, priv) ({ st_cleanup(pub, priv); secp256k1_free(key); if (holder) secp256k1_free(holder); priv = pub = NULL; key = holder = NULL; })

bool_t check_prime_secp256k1_fixed_sthread(stringer_t *errmsg) {

	int len;
	uchr_t padding[32];
	EC_KEY *key = NULL;
	stringer_t *priv1 = hex_decode_st(NULLER("0000000000000000000000000000000000000000000000000000000000000045"), MANAGEDBUF(32)),
		*priv2 = hex_decode_st(NULLER("0000000c562e65f1e2603616804cec8dc4bf8bc5c183bffa66acc6148edbecc3"), MANAGEDBUF(32)),
		*pub1 = hex_decode_st(NULLER("025edd5cc23c51e87a497ca815d5dce0f8ab52554f849ed8995de64c5f34ce7143"), MANAGEDBUF(33)),
		*pub2 = hex_decode_st(NULLER("03a7f953b0fa3f407bbf37cec394800cceeadd0670c3f344524c347312f2c1da96"), MANAGEDBUF(33)), *comparison;

	// Wipe the padding buffer. We compare this buffer with the leading bytes of the private key, with the length of the comparison
	// equal to the number of expected padding bytes.
	mm_wipe(padding, 32);

	// Build a comparison key object using the first serialized private key.
	if (!(key = secp256k1_private_set(priv1)) || EC_KEY_check_key_d(key) != 1) {
		st_sprint(errmsg, "The first serialized secp256k1 private key doesn't appear to be valid.");
		secp256k1_free(key);
		return false;
	}

	// Make sure the private key is leads with the proper number of zero'ed out padding bytes.
	else if ((len = BN_num_bytes_d(EC_KEY_get0_private_key_d(key))) != 32 && mm_cmp_cs_eq(padding, st_data_get(priv1), 32 - len)) {
		st_sprint(errmsg, "The first serialized secp256k1 private key was not padded properly.");
		secp256k1_free(key);
		return false;
	}

	// Compare the serialized private key against the original hard coded value.
	else if (!(comparison = secp256k1_private_get(key, MANAGEDBUF(32))) || st_cmp_cs_eq(priv1, comparison) ||
		(len = BN_num_bytes_d(EC_KEY_get0_private_key_d(key))) != 1 || mm_cmp_cs_eq(padding, st_data_get(comparison), 32 - len)) {
		st_sprint(errmsg, "The first secp256k1 private key doesn't match the expected value.");
		secp256k1_free(key);
		return false;
	}

	// Compare the serialized public key with the hard coded value.
	else if (!(comparison = secp256k1_public_get(key, MANAGEDBUF(33))) || st_cmp_cs_eq(pub1, comparison)) {
		st_sprint(errmsg, "The first secp256k1 public key doesn't match the expected value.");
		secp256k1_free(key);
		return false;
	}

	secp256k1_free(key);

	// Build a comparison key object using the second serialized private key.
	if (!(key = secp256k1_private_set(priv2)) || EC_KEY_check_key_d(key) != 1) {
		st_sprint(errmsg, "The second serialized secp256k1 private key doesn't appear to be valid.");
		secp256k1_free(key);
		return false;
	}

	// Make sure the private key is leads with the proper number of zero'ed out padding bytes.
	else if ((len = BN_num_bytes_d(EC_KEY_get0_private_key_d(key))) != 32 && mm_cmp_cs_eq(padding, st_data_get(priv2), 32 - len)) {
		st_sprint(errmsg, "The second serialized secp256k1 private key was not padded properly.");
		secp256k1_free(key);
		return false;
	}

	// Compare the serialized private key against the original hard coded value.
	else if (!(comparison = secp256k1_private_get(key, MANAGEDBUF(32))) || st_cmp_cs_eq(priv2, comparison) ||
		(len = BN_num_bytes_d(EC_KEY_get0_private_key_d(key))) != 29 || mm_cmp_cs_eq(padding, st_data_get(comparison), 32 - len)) {
		st_sprint(errmsg, "The second secp256k1 private key doesn't match the expected value.");
		secp256k1_free(key);
		return false;
	}

	// Compare the serialized public key with the hard coded value.
	else if (!(comparison = secp256k1_public_get(key, MANAGEDBUF(33))) || st_cmp_cs_eq(pub2, comparison)) {
		st_sprint(errmsg, "The second secp256k1 public key doesn't match the expected value.");
		secp256k1_free(key);
		return false;
	}

	secp256k1_free(key);
	return true;
}


bool_t check_prime_secp256k1_keys_sthread(stringer_t *errmsg) {

	int len;
	uchr_t y, padding[32];
	EC_KEY *key = NULL, *holder = NULL;
	stringer_t *priv = NULL, *pub = NULL;

	// Wipe the padding buffer. We compare this buffer with the leading bytes of the private key, with the length of the comparison
	// equal to the number of expected padding bytes.
	mm_wipe(padding, 32);

	for (uint64_t i = 0; status() && i < PRIME_CHECK_ITERATIONS; i++) {

		// Generate a new key pair.
		if (!(key = secp256k1_generate()) || EC_KEY_check_key_d(key) != 1) {
			st_sprint(errmsg, "Curve secp256k1 key generation failed.");
			return false;
		}
		// Extract the public and private components.
		else if (!(pub = secp256k1_public_get(key, NULL)) || !(priv = secp256k1_private_get(key, NULL))) {
			st_sprint(errmsg, "Curve secp256k1 exponent serialization failed.");
			check_prime_secp256k1_keys_cleanup(key, holder, pub, priv);
			return false;
		}

		// Confirm the serialized output is the correct length.
		else if (st_length_get(priv) != 32 || st_length_get(pub) != 33) {
			st_sprint(errmsg, "The serialized secp256k1 keys are not the expected length.");
			check_prime_secp256k1_keys_cleanup(key, holder, pub, priv);
			return false;
		}

		// Make sure the private key is leads with the proper number of zero'ed out padding bytes.
		else if ((len = BN_num_bytes_d(EC_KEY_get0_private_key_d(key))) != 32 && mm_cmp_cs_eq(padding, st_data_get(priv), 32 - len)) {
			st_sprint(errmsg, "The serialized secp256k1 private key was not padded properly.");
			check_prime_secp256k1_keys_cleanup(key, holder, pub, priv);
			return false;
		}
//		else if (len < 31) {
//			log_pedantic("bits = %i / pad = %i", BN_num_bits_d(EC_KEY_get0_private_key_d(key)),  32 - len);
//			log_pedantic("priv = %s", st_char_get(hex_encode_st(priv, MANAGEDBUF(128))));
//		}

		// Confirm the octet stream starts with 0x02 or 0x03, in accordance with the compressed point representation
		// described by ANSI standard X9.62 section 4.3.6.
		y = *((uchr_t *)st_data_get(pub));
		if (y != 2 && y != 3) {
			st_sprint(errmsg, "Curve secp256k1 public key point does not appear to be compressed properly.");
			check_prime_secp256k1_keys_cleanup(key, holder, pub, priv);
			return false;
		}

		// Build a comparison key object using the serialized private key.
		else if (!(holder = secp256k1_private_set(priv)) || EC_KEY_check_key_d(holder) != 1) {
			st_sprint(errmsg, "The serialized secp256k1 private key doesn't appear to be valid.");
			check_prime_secp256k1_keys_cleanup(key, holder, pub, priv);
			return false;
		}
		// Compare the key object created using serialized key the original. BN_cmp() should return 0 if the two BIGNUM values are equivalent.
		else if (BN_cmp_d(EC_KEY_get0_private_key_d(key), EC_KEY_get0_private_key_d(holder))) {
			st_sprint(errmsg, "The derived secp256k1 private key doesn't match our original key object.");
			check_prime_secp256k1_keys_cleanup(key, holder, pub, priv);
			return false;
		}
		else if (EC_POINT_cmp_d(EC_KEY_get0_group_d(key),EC_KEY_get0_public_key_d(key), EC_KEY_get0_public_key_d(holder), NULL)) {
			st_sprint(errmsg, "The derived secp256k1 public key doesn't match our original key object.");
			check_prime_secp256k1_keys_cleanup(key, holder, pub, priv);
			return false;
		}

		// Build a comparison key object using the serialized public key.
		secp256k1_free(holder);

		if (!(holder = secp256k1_public_set(pub)) || EC_KEY_check_key_d(holder) != 1) {
			st_sprint(errmsg, "The serialized secp256k1 public key doesn't appear to be valid.");
			check_prime_secp256k1_keys_cleanup(key, holder, pub, priv);
			return false;
		}
		else if (EC_POINT_cmp_d(EC_KEY_get0_group_d(key), (const EC_POINT *)EC_KEY_get0_public_key_d(key), (const EC_POINT *)EC_KEY_get0_public_key_d(holder), NULL)) {
			st_sprint(errmsg, "The derived secp256k1 public key doesn't match our original key object.");
			check_prime_secp256k1_keys_cleanup(key, holder, pub, priv);
			return false;
		}

		check_prime_secp256k1_keys_cleanup(key, holder, pub, priv);
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
