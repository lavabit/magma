
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
	uchr_t padding[32];
	EC_KEY *original = NULL, *comparison = NULL, *ephemeral = NULL;
	stringer_t *priv = NULL, *pub = NULL, *kek1 = NULL, *kek2 = NULL;

	// Wipe the padding buffer. We compare this buffer with the leading bytes of the private key, with the length of the comparison
	// equal to the number of expected padding bytes.
	mm_wipe(padding, 32);

	for (uint64_t i = 0; status() && i < PRIME_CHECK_ITERATIONS; i++) {

		// Generate a new key pair.
		if (!(original = secp256k1_generate()) || EC_KEY_check_key_d(original) != 1) {
			st_sprint(errmsg, "Curve secp256k1 key generation failed.");
			if (original) secp256k1_free(original);
			return false;
		}
		// Extract the public and private components.
		else if (!(pub = secp256k1_public_get(original, MANAGEDBUF(33))) || !(priv = secp256k1_private_get(original, MANAGEDBUF(32)))) {
			st_sprint(errmsg, "Curve secp256k1 exponent serialization failed.");
			secp256k1_free(original);
			return false;
		}

		// Confirm the serialized output is the correct length.
		else if (st_length_get(priv) != 32 || st_length_get(pub) != 33) {
			st_sprint(errmsg, "The serialized secp256k1 keys are not the expected length.");
			secp256k1_free(original);
			return false;
		}

		// Make sure the private key is leads with the proper number of zero'ed out padding bytes.
		else if ((len = BN_num_bytes_d(EC_KEY_get0_private_key_d(original))) != 32 && mm_cmp_cs_eq(padding, st_data_get(priv), 32 - len)) {
			st_sprint(errmsg, "The serialized secp256k1 private key was not padded properly.");
			secp256k1_free(original);
			return false;
		}

		// Confirm the octet stream starts with 0x02 or 0x03, in accordance with the compressed point representation
		// described by ANSI standard X9.62 section 4.3.6.
		if (*((uchr_t *)st_data_get(pub)) != 2 && *((uchr_t *)st_data_get(pub)) != 3) {
			st_sprint(errmsg, "Curve secp256k1 public key point does not appear to be represented properly as a compressed point.");
			secp256k1_free(original);
			return false;
		}

		// Build a comparison key object using the serialized private key.
		else if (!(comparison = secp256k1_private_set(priv)) || EC_KEY_check_key_d(comparison) != 1) {
			st_sprint(errmsg, "The serialized secp256k1 private key doesn't appear to be valid.");
			if (comparison) secp256k1_free(comparison);
			secp256k1_free(original);
			return false;
		}

		// Compare the key object created against the original serialized key value, BN_cmp() will return 0 if the BIGNUM values are equivalent.
		else if (BN_cmp_d(EC_KEY_get0_private_key_d(original), EC_KEY_get0_private_key_d(comparison))) {
			st_sprint(errmsg, "The derived secp256k1 private key doesn't match our original key object.");
			secp256k1_free(comparison);
			secp256k1_free(original);
			return false;
		}
		// Compare the elipitical curve points, EC_POINT_cmp() will return 0 if the two representations are equivalent.
		else if (EC_POINT_cmp_d(EC_KEY_get0_group_d(original), EC_KEY_get0_public_key_d(original), EC_KEY_get0_public_key_d(comparison), NULL)) {
			st_sprint(errmsg, "The derived secp256k1 public key doesn't match our original key object.");
			secp256k1_free(comparison);
			secp256k1_free(original);
			return false;
		}

		// Build a comparison key object using the serialized public key.
		secp256k1_free(comparison);

		if (!(comparison = secp256k1_public_set(pub)) || EC_KEY_check_key_d(comparison) != 1) {
			st_sprint(errmsg, "The serialized secp256k1 public key doesn't appear to be valid.");
			if (comparison) secp256k1_free(comparison);
			secp256k1_free(original);
			return false;
		}
		else if (EC_POINT_cmp_d(EC_KEY_get0_group_d(original), EC_KEY_get0_public_key_d(original), EC_KEY_get0_public_key_d(comparison), NULL)) {
			st_sprint(errmsg, "The derived secp256k1 public key doesn't match our original key object.");
			secp256k1_free(comparison);
			secp256k1_free(original);
			return false;
		}

		// Generate an ephemeral key for checking the key encryption key functions.
		else if (!(ephemeral = secp256k1_generate()) || EC_KEY_check_key_d(ephemeral) != 1) {
			st_sprint(errmsg, "Curve secp256k1 key generation failed.");
			if (ephemeral) secp256k1_free(ephemeral);
			secp256k1_free(comparison);
			secp256k1_free(original);
			return false;
		}

		// Compute a KEK using the ephemeral private key, and the original public key.
		else if (!(kek1 = secp256k1_compute_kek(ephemeral, comparison, MANAGEDBUF(32))) || st_length_get(kek1) != 32) {
			st_sprint(errmsg, "Key encryption key derivation failed using a secp256k1 ephemeral key, and a public key object.");
			secp256k1_free(comparison);
			secp256k1_free(ephemeral);
			secp256k1_free(original);
			return false;
		}

		// Compute a KEK using the ephemeral public key, and the original private key.
		else if (!(kek2 = secp256k1_compute_kek(original, ephemeral, MANAGEDBUF(32))) || st_length_get(kek2) != 32) {
			st_sprint(errmsg, "Key encryption key derivation failed using a secp256k1 private key, and the public portion of the ephemeral public key object.");
			secp256k1_free(comparison);
			secp256k1_free(ephemeral);
			secp256k1_free(original);
			return false;
		}

		// If everything worked properly then both KEK values will be equal.
		else if (st_cmp_cs_eq(kek1, kek2)) {
			st_sprint(errmsg, "The two key encryption key values failed to match.");
			secp256k1_free(comparison);
			secp256k1_free(ephemeral);
			secp256k1_free(original);
			return false;
		}

		secp256k1_free(comparison);
		secp256k1_free(ephemeral);
		secp256k1_free(original);
	}

	return true;
}

bool_t check_prime_secp256k1_parameters_sthread(stringer_t *errmsg) {

	EC_KEY *key = NULL;

	log_disable();

	// Test a NULL input.
	if ((key = secp256k1_private_set(NULL))) {
		st_sprint(errmsg, "The secp256k1 private key setup function accepted a NULL input value.");
		secp256k1_free(key);
		return false;
	}
	// Try again with a value that isn't long enough.
	else if ((key = secp256k1_private_set(hex_decode_st(NULLER("00"), MANAGEDBUF(1))))) {
		st_sprint(errmsg, "The secp256k1 private key setup function accepted a private key value that wasn't long enough.");
		secp256k1_free(key);
		return false;
	}
	// Try again with a value that is too long.
	else if ((key = secp256k1_private_set(hex_decode_st(NULLER("000000000000000000000000000000000000000000000000000000000000000001"), MANAGEDBUF(33))))) {
		st_sprint(errmsg, "The secp256k1 private key setup function accepted a private key value that was too long.");
		secp256k1_free(key);
		return false;
	}
	// The private key value must be 1 or higher and properly padded.
	else if ((key = secp256k1_private_set(hex_decode_st(NULLER("0000000000000000000000000000000000000000000000000000000000000000"), MANAGEDBUF(32))))) {
		st_sprint(errmsg, "The secp256k1 private key setup function accepted a private key value of zero.");
		secp256k1_free(key);
		return false;
	}
	// The private key value must be equal to or less than 0xfffffffffffffffffffffffffffffffebaaedce6af48a03bbfd25e8cd0364141.
	else if ((key = secp256k1_private_set(hex_decode_st(NULLER("ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"), MANAGEDBUF(32))))) {
		st_sprint(errmsg, "The secp256k1 private key setup function accepted a private key value larger than allowed.");
		secp256k1_free(key);
		return false;
	}
	// Try with a value that is only one bit higher than allowed.
	else if ((key = secp256k1_private_set(hex_decode_st(NULLER("fffffffffffffffffffffffffffffffebaaedce6af48a03bbfd25e8cd0364142"), MANAGEDBUF(32))))) {
		st_sprint(errmsg, "The secp256k1 private key setup function accepted a private key value one bit higher than allowed.");
		secp256k1_free(key);
		return false;
	}
	// Try with a value that is precisely equal to the maximum legal value.
	else if ((key = secp256k1_private_set(hex_decode_st(NULLER("fffffffffffffffffffffffffffffffebaaedce6af48a03bbfd25e8cd0364141"), MANAGEDBUF(32))))) {
		st_sprint(errmsg, "The secp256k1 private key setup function did not accept the maximum allowable value.");
		return false;
	}

	secp256k1_free(key);
	return true;

}
