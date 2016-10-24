
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
	EC_KEY *key1 = NULL, *key2 = NULL;
	stringer_t *priv1 = hex_decode_st(NULLER("0000000000000000000000000000000000000000000000000000000000000045"), MANAGEDBUF(32)),
		*priv2 = hex_decode_st(NULLER("0000000c562e65f1e2603616804cec8dc4bf8bc5c183bffa66acc6148edbecc3"), MANAGEDBUF(32)),
		*pub1 = hex_decode_st(NULLER("025edd5cc23c51e87a497ca815d5dce0f8ab52554f849ed8995de64c5f34ce7143"), MANAGEDBUF(33)),
		*pub2 = hex_decode_st(NULLER("03a7f953b0fa3f407bbf37cec394800cceeadd0670c3f344524c347312f2c1da96"), MANAGEDBUF(33)),
		*kek = hex_decode_st(NULLER("97495184dc4de0a3dc614d15f699df6a8cb65a752434368fb7f1d2702c53ab19"), MANAGEDBUF(32)),
		*comparison;

	// Wipe the padding buffer. We compare this buffer with the leading bytes of the private key, with the length of the comparison
	// equal to the number of expected padding bytes.
	mm_wipe(padding, 32);

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
		// Compare the key object created against the original serialized key value, BN_cmp() will return 0 if the BIGNUM values are equivalent.
		else if (BN_cmp_d(EC_KEY_get0_private_key_d(key), EC_KEY_get0_private_key_d(holder))) {
			st_sprint(errmsg, "The derived secp256k1 private key doesn't match our original key object.");
			check_prime_secp256k1_keys_cleanup(key, holder, pub, priv);
			return false;
		}
		// Compare the elipitical curve points, EC_POINT_cmp() will return 0 if the two representations are equivalent.
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
