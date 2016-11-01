
/**
 * @file /magma/check/magma/prime/ed25519_check.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma_check.h"
#include "dime/ed25519/ed25519.h"

// Wrappers around ED25519 functions
typedef struct {
    ed25519_secret_key private_key;
    ed25519_public_key public_key;
} ED25519_KEY;

bool_t check_prime_ed25519_fixed_sthread(stringer_t *errmsg) {

	ed25519_key_t *key = NULL;
	stringer_t *priv1 = hex_decode_st(NULLER("9d61b19deffd5a60ba844af492ec2cc44449c5697b326919703bac031cae7f60"), MANAGEDBUF(32)),
		*priv2 = hex_decode_st(NULLER("4ccd089b28ff96da9db6c346ec114e0f5b8a319f35aba624da8cf6ed4fb8a6fb"), MANAGEDBUF(32)),
		*priv3 = hex_decode_st(NULLER("c5aa8df43f9f837bedb7442f31dcb7b166d38535076f094b85ce3a2e0b4458f7"), MANAGEDBUF(32)),
		*pub1 = hex_decode_st(NULLER("d75a980182b10ab7d54bfed3c964073a0ee172f3daa62325af021a68f707511a"), MANAGEDBUF(32)),
		*pub2 = hex_decode_st(NULLER("3d4017c3e843895a92b70aa74d1b7ebc9c982ccf2ec4968cc0cd55f12af4660c"), MANAGEDBUF(32)),
		*pub3 = hex_decode_st(NULLER("fc51cd8e6218a1a38da47ed00230f0580816ed13ba3303ac5deb911548908025"), MANAGEDBUF(32)),
		*signature1 = hex_decode_st(NULLER("e5564300c360ac729086e2cc806e828a84877f1eb8e5d974d873e065224901555fb8821590a33bacc61e39701cf9b46bd25bf5f0595bbe24655141438e7a100b"), MANAGEDBUF(64)),
		*signature2 = hex_decode_st(NULLER("92a009a9f0d4cab8720e820b5f642540a2b27b5416503f8fb3762223ebdb69da085ac1e43e15996e458f3613d0f11d8c387b2eaeb4302aeeb00d291612bb0c00"), MANAGEDBUF(64)),
		*signature3 = hex_decode_st(NULLER("6291d657deec24024827e69c3abe01a30ce548a284743a445e3680d7db5ac3ac18ff9b538d16f290ae67f760984dc6594a7c15e9716ed28dc027beceea1ec40a"), MANAGEDBUF(64)),
		*message1 = NULLER(""),
		*message2 = hex_decode_st(NULLER("72"), MANAGEDBUF(1)),
		*message3 = hex_decode_st(NULLER("af82"), MANAGEDBUF(2)),
		*comparison;

	// Build a comparison key object using the first serialized private key.
	if (!(key1 = secp256k1_private_set(priv1)) || EC_KEY_check_key_d(key1) != 1) {
		st_sprint(errmsg, "The first serialized secp256k1 private key doesn't appear to be valid.");
		if (key1) secp256k1_free(key1);
		return false;
	}

	// Make sure the private key is leads with the proper number of zero'ed out padding bytes.
	else if ((len = BN_num_bytes_d(EC_KEY_get0_private_key_d(key1))) != 32 && mm_cmp_cs_eq(padding, st_data_get(priv1), 32 - len)) {
		st_sprint(errmsg, "The first serialized secp256k1 private key was not padded properly.");
		secp256k1_free(key1);
		return false;
	}

	// Compare the serialized private key against the original hard coded value.
	else if (!(comparison = secp256k1_private_get(key1, MANAGEDBUF(32))) || st_cmp_cs_eq(priv1, comparison) ||
		(len = BN_num_bytes_d(EC_KEY_get0_private_key_d(key1))) != 1 || mm_cmp_cs_eq(padding, st_data_get(comparison), 32 - len)) {
		st_sprint(errmsg, "The first secp256k1 private key doesn't match the expected value.");
		secp256k1_free(key1);
		return false;
	}

	// Compare the serialized public key with the hard coded value.
	else if (!(comparison = secp256k1_public_get(key1, MANAGEDBUF(33))) || st_cmp_cs_eq(pub1, comparison)) {
		st_sprint(errmsg, "The first secp256k1 public key doesn't match the expected value.");
		secp256k1_free(key1);
		return false;
	}

	// Build a comparison key object using the second serialized private key.
	if (!(key2 = secp256k1_private_set(priv2)) || EC_KEY_check_key_d(key2) != 1) {
		st_sprint(errmsg, "The second serialized secp256k1 private key doesn't appear to be valid.");
		if (key2) secp256k1_free(key2);
		secp256k1_free(key1);
		return false;
	}

	// Make sure the private key is leads with the proper number of zero'ed out padding bytes.
	else if ((len = BN_num_bytes_d(EC_KEY_get0_private_key_d(key2))) != 32 && mm_cmp_cs_eq(padding, st_data_get(priv2), 32 - len)) {
		st_sprint(errmsg, "The second serialized secp256k1 private key was not padded properly.");
		secp256k1_free(key2);
		secp256k1_free(key1);
		return false;
	}

	// Compare the serialized private key against the original hard coded value.
	else if (!(comparison = secp256k1_private_get(key2, MANAGEDBUF(32))) || st_cmp_cs_eq(priv2, comparison) ||
		(len = BN_num_bytes_d(EC_KEY_get0_private_key_d(key2))) != 29 || mm_cmp_cs_eq(padding, st_data_get(comparison), 32 - len)) {
		st_sprint(errmsg, "The second secp256k1 private key doesn't match the expected value.");
		secp256k1_free(key2);
		secp256k1_free(key1);
		return false;
	}

	// Compare the serialized public key with the hard coded value.
	else if (!(comparison = secp256k1_public_get(key2, MANAGEDBUF(33))) || st_cmp_cs_eq(pub2, comparison)) {
		st_sprint(errmsg, "The second secp256k1 public key doesn't match the expected value.");
		secp256k1_free(key2);
		secp256k1_free(key1);
		return false;
	}

	// Compute the key encryption key, then swap public/private keys around and compute the key encryption key again.
	if (!(comparison = secp256k1_compute_kek(key1, key2, MANAGEDBUF(32))) || st_cmp_cs_eq(kek, comparison) ||
		!(comparison = secp256k1_compute_kek(key2, key1, MANAGEDBUF(32))) || st_cmp_cs_eq(kek, comparison) ) {
		st_sprint(errmsg, "The key encryption key computation on the secp256k1 keys didn't produced the expected result.");
		secp256k1_free(key2);
		secp256k1_free(key1);
		return false;
	}

	secp256k1_free(key2);
	secp256k1_free(key1);

	return true;
}

bool_t check_prime_ed25519_keys_sthread(stringer_t *errmsg) {

	size_t len = 0;
	ed25519_key_t *key = NULL;
	uint8_t signature[ED25519_SIGNATURE_LEN];
	stringer_t *fuzzer = MANAGEDBUF(PRIME_CHECK_SIZE_MAX);
	unsigned char ed25519_donna_public_key[ED25519_KEY_PUB_LEN];

	for (uint64_t i = 0; status() && i < PRIME_CHECK_ITERATIONS; i++) {

		// Generate a random ed25519 key pair.
		if (!(key = ed25519_generate())) {
			st_sprint(errmsg, "Failed to generate an ed25519 key pair.");
			return false;
		}

		// Calculate the public key value using the alternate implementation.
		ed25519_publickey(key->private, ed25519_donna_public_key);

		// Compare the values.
		if (st_cmp_cs_eq(PLACER(ed25519_donna_public_key, ED25519_KEY_PUB_LEN), PLACER(key->public, ED25519_KEY_PUB_LEN))) {
			st_sprint(errmsg, "The alternate implementation failed to derive an identical ed25519 public key.");
			ed25519_free(key);
			return false;
		}

		// How much random data will we fuzz with.
		len = (rand() % (PRIME_CHECK_SIZE_MAX - PRIME_CHECK_SIZE_MIN)) + PRIME_CHECK_SIZE_MIN;

		// Create a buffer filled with random data to sign.
		rand_write(PLACER(st_data_get(fuzzer), len));

		// Generate an ed25519 signature using OpenSSL.
		if (ED25519_sign_d(&signature[0], st_data_get(fuzzer), len, key->private) != 1) {
			st_sprint(errmsg, "The ed25519 signature operation failed.");
			ed25519_free(key);
			return false;
		}
		// Verify the ed25519 signature using the alternate implementation.
		else if (ed25519_sign_open(st_data_get(fuzzer), len, key->public, signature)) {
			st_sprint(errmsg, "The alternate implementation failed to verify the ed25519 signature.");
			ed25519_free(key);
			return false;
		}

		// How much random data will we fuzz with for the second check.
		len = (rand() % (PRIME_CHECK_SIZE_MAX - PRIME_CHECK_SIZE_MIN)) + PRIME_CHECK_SIZE_MIN;

		// Create a buffer filled with random data to sign.
		rand_write(PLACER(st_data_get(fuzzer), len));

		// Generate an ed25519 signature using the alternate implementation.
		ed25519_sign(st_data_get(fuzzer), len, key->private, key->public, signature);

		// Verify the ed25519 signature using OpenSSL.
		if (ED25519_verify_d(st_data_get(fuzzer), len, signature, key->public) != 1) {
			st_sprint(errmsg, "The ed25519 signature generated using the alternate implementation failed to verify.");
			ed25519_free(key);
			return false;
		}

		ed25519_free(key);

	}
	return true;
}
