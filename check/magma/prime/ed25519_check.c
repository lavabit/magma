
/**
 * @file /check/magma/prime/ed25519_check.c
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

/**
 * @note			The static test vectors below were extracted from the official home for the EdDSA whitepaper.
 * @see				http://ed25519.cr.yp.to/python/sign.input

 * @param errmsg
 * @return
 */
bool_t check_prime_ed25519_fixed_sthread(stringer_t *errmsg) {

	uchr_t byte;
	int_t pos = 0;
	ed25519_key_t *key = NULL;
	uint8_t signature[ED25519_SIGNATURE_LEN];
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
		*comparison = NULL;

	// Build a the first serialized private key and check the derived public key.
	if (!(key = ed25519_private_set(priv1)) || st_cmp_cs_eq(pub1, PLACER(key->public, ED25519_KEY_PUB_LEN))) {
		st_sprint(errmsg, "The first serialized ed25519 private key doesn't appear to be valid.");
		if (key) ed25519_free(key);
		return false;
	}

	// Compare the serialized private key against the original hard coded value.
	else if (!(comparison = ed25519_private_get(key, MANAGEDBUF(32))) || st_cmp_cs_eq(priv1, comparison)) {
		st_sprint(errmsg, "The first ed25519 private key doesn't match the expected value.");
		ed25519_free(key);
		return false;
	}

	// Compare the serialized public key with the hard coded value.
	else if (!(comparison = ed25519_public_get(key, MANAGEDBUF(32))) || st_cmp_cs_eq(pub1, comparison)) {
		st_sprint(errmsg, "The first ed25519 public key doesn't match the expected value.");
		ed25519_free(key);
		return false;
	}

	// Generate a test signature for the first message and compare it against the expected value.
	else if (ED25519_sign_d(&signature[0], st_data_get(message1), st_length_get(message1), key->private) != 1 ||
		st_cmp_cs_eq(PLACER(signature, ED25519_SIGNATURE_LEN), signature1)) {
		st_sprint(errmsg, "The first ed25519 test message didn't generate the expected signature.");
		ed25519_free(key);
		return false;
	}

	// Verify the test signature. First by passing the public key directly, then by passing the serialized public key buffer.
	else if (ED25519_verify_d(st_data_get(message1), st_length_get(message1), &signature[0], key->public) != 1 ||
		st_cmp_cs_eq(PLACER(signature, ED25519_SIGNATURE_LEN), signature1) ||
		ED25519_verify_d(st_data_get(message1), st_length_get(message1), &signature[0], st_data_get(ed25519_public_get(key, MANAGEDBUF(32)))) != 1) {
		st_sprint(errmsg, "The first ed25519 signature didn't verify properly.");
		ed25519_free(key);
		return false;
	}

	// Tweak a random bit and make sure the signature fails.
	pos = (rand_get_uint32() % ED25519_SIGNATURE_LEN);
	byte = signature[pos];
	if (byte == UINT8_MAX) byte--;
	else byte++;
	signature[pos] = bitwise_xor(signature[pos], byte);

	// Verify the test signature after the modification.
	if (ED25519_verify_d(st_data_get(message1), st_length_get(message1), &signature[0], key->public) == 1) {
		st_sprint(errmsg, "The first ed25519 signature verified with a randomly modified signature. {%.*s)", 64,
			st_char_get(hex_encode_st(PLACER(&signature[0], ED25519_SIGNATURE_LEN), MANAGEDBUF(64))));
		ed25519_free(key);
		return false;
	}

	ed25519_free(key);
	key = NULL;

	// Build a the second serialized private key and check the derived public key.
	if (!(key = ed25519_private_set(priv2)) || st_cmp_cs_eq(pub2, PLACER(key->public, ED25519_KEY_PUB_LEN))) {
		st_sprint(errmsg, "The second serialized ed25519 private key doesn't appear to be valid.");
		if (key) ed25519_free(key);
		return false;
	}

	// Compare the serialized private key against the original hard coded value.
	else if (!(comparison = ed25519_private_get(key, MANAGEDBUF(32))) || st_cmp_cs_eq(priv2, comparison)) {
		st_sprint(errmsg, "The second ed25519 private key doesn't match the expected value.");
		ed25519_free(key);
		return false;
	}

	// Compare the serialized public key with the hard coded value.
	else if (!(comparison = ed25519_public_get(key, MANAGEDBUF(32))) || st_cmp_cs_eq(pub2, comparison)) {
		st_sprint(errmsg, "The second ed25519 public key doesn't match the expected value.");
		ed25519_free(key);
		return false;
	}

	// Generate a test signature for the second message and compare it against the expected value.
	if (!(comparison = ed25519_sign(key, message2, MANAGEDBUF(ED25519_SIGNATURE_LEN))) ||
		st_cmp_cs_eq(comparison, signature2)) {
		st_sprint(errmsg, "The second ed25519 test message didn't generate the expected signature.");
		ed25519_free(key);
		return false;
	}

	// Verify the test signature.
	else if (ed25519_verify(key, message2, comparison)) {
		st_sprint(errmsg, "The second ed25519 signature didn't verify properly.");
		ed25519_free(key);
		return false;
	}

	// Tweak a random bit and make sure the signature fails.
	pos = (rand_get_uint32() % ED25519_SIGNATURE_LEN);
	byte = *((uchr_t *)st_data_get(comparison) + pos);
	if (byte == UINT8_MAX) byte--;
	else byte++;
	 *((uchr_t *)st_data_get(comparison) + pos) = bitwise_xor( *((uchr_t *)st_data_get(comparison) + pos), byte);

	// Verify the test signature after the modification.
	if (ed25519_verify(key, message2, comparison) != -1) {
		st_sprint(errmsg, "The second ed25519 signature verified with a randomly modified signature. {%.*s)", 64,
			st_char_get(hex_encode_st(comparison, MANAGEDBUF(64))));
		ed25519_free(key);
		return false;
	}

	ed25519_free(key);
	key = NULL;

	// Build a the third serialized private key and check the derived public key.
	if (!(key = ed25519_private_set(priv3)) || st_cmp_cs_eq(pub3, PLACER(key->public, ED25519_KEY_PUB_LEN))) {
		st_sprint(errmsg, "The third serialized ed25519 private key doesn't appear to be valid.");
		if (key) ed25519_free(key);
		return false;
	}

	// Compare the serialized private key against the original hard coded value.
	else if (!(comparison = ed25519_private_get(key, MANAGEDBUF(32))) || st_cmp_cs_eq(priv3, comparison)) {
		st_sprint(errmsg, "The third ed25519 private key doesn't match the expected value.");
		ed25519_free(key);
		return false;
	}

	// Compare the serialized public key with the hard coded value.
	else if (!(comparison = ed25519_public_get(key, MANAGEDBUF(32))) || st_cmp_cs_eq(pub3, comparison)) {
		st_sprint(errmsg, "The third ed25519 public key doesn't match the expected value.");
		ed25519_free(key);
		return false;
	}

	// Generate a test signature for the third message and compare it against the expected value.
	if (!(comparison = ed25519_sign(key, message3, MANAGEDBUF(ED25519_SIGNATURE_LEN))) ||
		st_cmp_cs_eq(comparison, signature3)) {
		st_sprint(errmsg, "The third ed25519 test message didn't generate the expected signature.");
		ed25519_free(key);
		return false;
	}

	// Verify the test signature.
	else if (ed25519_verify(key, message3, comparison)) {
		st_sprint(errmsg, "The third ed25519 signature didn't verify properly.");
		ed25519_free(key);
		return false;
	}

	// Tweak a random bit and make sure the signature fails.
	pos = (rand_get_uint32() % ED25519_SIGNATURE_LEN);
	byte = *((uchr_t *)st_data_get(comparison) + pos);
	if (byte == UINT8_MAX) byte--;
	else byte++;
	 *((uchr_t *)st_data_get(comparison) + pos) = bitwise_xor( *((uchr_t *)st_data_get(comparison) + pos), byte);

	// Verify the test signature after the modification.
	if (ed25519_verify(key, message3, comparison) != -1) {
		st_sprint(errmsg, "The third ed25519 signature verified with a randomly modified signature. {%.*s)", 64,
			st_char_get(hex_encode_st(comparison, MANAGEDBUF(64))));
		ed25519_free(key);
		return false;
	}

	// Try a final time, looping through and using an increasing amount of the signature, but never the entire thing.
	for (int_t i = 0; i < ED25519_SIGNATURE_LEN; i++) {
		if (ed25519_verify(key, message3, PLACER(comparison, i)) != -2) {
			st_sprint(errmsg, "The truncated ed25519 signature verified when it shouldn't have. {%.*s)", (i + 1) * 2,
				st_char_get(hex_encode_st(PLACER(comparison, i), MANAGEDBUF(64))));
			ed25519_free(key);
			return false;
		}
	}

	ed25519_free(key);
	key = NULL;

	return true;

}

bool_t check_prime_ed25519_fuzz_lib_sthread(stringer_t *errmsg) {

	size_t len = 0;
	ed25519_key_t *key = NULL, *pub = NULL;
	uint8_t signature[ED25519_SIGNATURE_LEN];
	stringer_t *fuzzer = MANAGEDBUF(PRIME_CHECK_SIZE_MAX), *serialized = NULL;
	unsigned char ed25519_donna_public_key[ED25519_KEY_PUB_LEN];

	for (uint64_t i = 0; status() && i < PRIME_CHECK_ITERATIONS; i++) {

		// Generate a random ed25519 key pair.
		if (!(key = ed25519_generate()) || !(serialized = ed25519_public_get(key, MANAGEDBUF(ED25519_KEY_PUB_LEN))) ||
			!(pub = ed25519_public_set(serialized))) {
			st_sprint(errmsg, "Failed to generate an ed25519 key pair.");
			if (key) ed25519_free(key);
			return false;
		}

		// Calculate the public key value using the alternate implementation.
		ed25519_publickey_donna(key->private, ed25519_donna_public_key);

		// Compare the values.
		if (st_cmp_cs_eq(PLACER(ed25519_donna_public_key, ED25519_KEY_PUB_LEN), PLACER(key->public, ED25519_KEY_PUB_LEN)) ||
			st_cmp_cs_eq(PLACER(ed25519_donna_public_key, ED25519_KEY_PUB_LEN), PLACER(pub->public, ED25519_KEY_PUB_LEN)) ||
			st_cmp_cs_eq(PLACER(ed25519_donna_public_key, ED25519_KEY_PUB_LEN), serialized)) {
			st_sprint(errmsg, "The alternate implementation failed to derive an identical ed25519 public key.");
			ed25519_free(key);
			ed25519_free(pub);
			return false;
		}

		// Determine how much random data will we fuzz with, then fill the buffer with random bytes.
		len = (rand() % (PRIME_CHECK_SIZE_MAX - PRIME_CHECK_SIZE_MIN)) + PRIME_CHECK_SIZE_MIN;
		rand_write(PLACER(st_data_get(fuzzer), len));
		st_length_set(fuzzer, len);

		// Generate an ed25519 signature using OpenSSL.
		if (ED25519_sign_d(&signature[0], st_data_get(fuzzer), len, key->private) != 1) {
			st_sprint(errmsg, "The ed25519 signature operation failed.");
			ed25519_free(key);
			ed25519_free(pub);
			return false;
		}
		else if (ED25519_verify_d(st_data_get(fuzzer), len, &signature[0], pub->public) != 1) {
			st_sprint(errmsg, "The ed25519 signature verification failed.");
			ed25519_free(key);
			ed25519_free(pub);
			return false;
		}
		// Verify the ed25519 signature using the alternate implementation.
		else if (ed25519_sign_open_donna(st_data_get(fuzzer), len, key->public, signature)) {
			st_sprint(errmsg, "The alternate implementation failed to verify the ed25519 signature.");
			ed25519_free(key);
			ed25519_free(pub);
			return false;
		}

		// Determine how much random data will we fuzz with for the second check, then fill the buffer with random bytes.
		len = (rand() % (PRIME_CHECK_SIZE_MAX - PRIME_CHECK_SIZE_MIN)) + PRIME_CHECK_SIZE_MIN;
		rand_write(PLACER(st_data_get(fuzzer), len));
		st_length_set(fuzzer, len);

		// Generate an ed25519 signature using the alternate implementation.
		ed25519_sign_donna(st_data_get(fuzzer), len, key->private, key->public, signature);

		// Verify the ed25519 signature using OpenSSL.
		if (ED25519_verify_d(st_data_get(fuzzer), len, signature, key->public) != 1) {
			st_sprint(errmsg, "The ed25519 signature generated using the alternate implementation failed to verify.");
			ed25519_free(key);
			ed25519_free(pub);
			return false;
		}

		ed25519_free(key);
		ed25519_free(pub);
	}

	return true;
}

bool_t check_prime_ed25519_fuzz_provider_sthread(stringer_t *errmsg) {

	size_t len = 0;
	ed25519_key_t *key = NULL, *pub = NULL;
	stringer_t *fuzzer = MANAGEDBUF(PRIME_CHECK_SIZE_MAX), *managed = NULL, *serialized = NULL;

	for (uint64_t i = 0; status() && i < PRIME_CHECK_ITERATIONS; i++) {

		// Generate a random ed25519 key pair.
		if (!(key = ed25519_generate()) || !(serialized = ed25519_public_get(key, MANAGEDBUF(ED25519_KEY_PUB_LEN))) ||
			!(pub = ed25519_public_set(serialized))) {
			st_sprint(errmsg, "Failed to generate an ed25519 key pair.");
			if (key) ed25519_free(key);
			return false;
		}

		// Determine how much random data will we fuzz with, then fill the buffer with random bytes.
		len = (rand() % (PRIME_CHECK_SIZE_MAX - PRIME_CHECK_SIZE_MIN)) + PRIME_CHECK_SIZE_MIN;
		rand_write(PLACER(st_data_get(fuzzer), len));
		st_length_set(fuzzer, len);

		// Generate an ed25519 signature using the PRIME interface.
		if (!(managed = ed25519_sign(key, fuzzer, MANAGEDBUF(ED25519_SIGNATURE_LEN)))) {
			st_sprint(errmsg, "The ed25519 PRIME interface failed to generate a signature.");
			ed25519_free(key);
			ed25519_free(pub);
			return false;
		}
		// Verify the ed25519 signature using the PRIME interface.
		else if (ed25519_verify(key, fuzzer, managed) || ed25519_verify(pub, fuzzer, managed)) {
			st_sprint(errmsg, "The ed25519 signature generated using the PRIME interface failed to verify.");
			ed25519_free(key);
			ed25519_free(pub);
			return false;
		}
		// Verify the ed25519 signature using the library interface.
		else if (ED25519_verify_d(st_data_get(fuzzer), len, st_data_get(managed), key->public) != 1 ||
			ED25519_verify_d(st_data_get(fuzzer), len, st_data_get(managed), pub->public) != 1) {
			st_sprint(errmsg, "The direct library call failed to verify an ed25519 signature which passed using the PRIME interface.");
			ed25519_free(key);
			ed25519_free(pub);
			return false;
		}
		// Verify the ed25519 signature using the alternate implementation.
		else if (ed25519_sign_open_donna(st_data_get(fuzzer), len, key->public, st_data_get(managed)) ||
			ed25519_sign_open_donna(st_data_get(fuzzer), len, pub->public, st_data_get(managed))) {
			st_sprint(errmsg, "The alternate implementation failed to verify the ed25519 signature which passed using PRIME interface.");
			ed25519_free(key);
			ed25519_free(pub);
			return false;
		}

		ed25519_free(key);
		ed25519_free(pub);
	}

	return true;
}

bool_t check_prime_ed25519_parameters_sthread(stringer_t *errmsg) {

	stringer_t *holder = NULL;
	ed25519_key_t *key = NULL;

	// Test a NULL input.
	if ((key = ed25519_private_set(NULL))) {
		st_sprint(errmsg, "The ed25519 private key setup function accepted a NULL input value.");
		ed25519_free(key);
		return false;
	}
	// Try again with a value that isn't long enough.
	else if ((key = ed25519_private_set(hex_decode_st(NULLER("00"), MANAGEDBUF(1))))) {
		st_sprint(errmsg, "The ed25519 private key setup function accepted a private key value that wasn't long enough.");
		ed25519_free(key);
		return false;
	}
	// Try again with a value that is too long.
	else if ((key = ed25519_private_set(hex_decode_st(NULLER("000000000000000000000000000000000000000000000000000000000000000001"), MANAGEDBUF(33))))) {
		st_sprint(errmsg, "The ed25519 private key setup function accepted a private key value that was too long.");
		ed25519_free(key);
		return false;
	}

	// Try a seed value of zero.
	else if (!(key = ed25519_private_set(hex_decode_st(NULLER("0000000000000000000000000000000000000000000000000000000000000000"), MANAGEDBUF(32))))) {
		st_sprint(errmsg, "The ed25519 private key setup function refused to accept a private key seed value of zero.");
		return false;
	}

	ed25519_free(key);

	// The private key seed may be any 32 byte value, so try the maximum value.
	if (!(key = ed25519_private_set(hex_decode_st(NULLER("ffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff"), MANAGEDBUF(32))))) {
		st_sprint(errmsg, "The ed25519 private key setup function accepted a private key value larger than allowed.");
		return false;
	}

	ed25519_free(key);

	// Test a NULL input.
	if ((key = ed25519_public_set(NULL))) {
		st_sprint(errmsg, "The ed25519 public key setup function accepted a NULL input value.");
		ed25519_free(key);
		return false;
	}
	// Try with a value of the wrong length.
	else if ((key = ed25519_public_set(hex_decode_st(NULLER("0200"), MANAGEDBUF(2))))) {
		st_sprint(errmsg, "The ed25519 public key setup function accepted a public key value that wasn't long enough.");
		ed25519_free(key);
		return false;
	}
	// Try a value with a value that is too long.
	else if ((key = ed25519_public_set(hex_decode_st(NULLER("000000000000000000000000000000000000000000000000000000000000000000"), MANAGEDBUF(33))))) {
		st_sprint(errmsg, "The ed25519 public key setup function accepted a public key value that was too long.");
		ed25519_free(key);
		return false;
	}

	// Create a key object using all zeros as the public scalar.
	else if (!(key = ed25519_public_set(hex_decode_st(NULLER("0000000000000000000000000000000000000000000000000000000000000000"), MANAGEDBUF(32))))) {
		st_sprint(errmsg, "The ed25519 public key setup function refused a public key value of zero.");
		return false;
	}

	// Try and fetch a private key from an object that only has a public key.
	else if (ed25519_private_get(key, MANAGEDBUF(32))) {
		st_sprint(errmsg, "A ed25519 private key was returned when only a public key should have been available.");
		ed25519_free(key);
		return false;
	}

	// Try and get a public key but provide an output buffer that is too small.
	else if (ed25519_public_get(key, MANAGEDBUF(31))) {
		st_sprint(errmsg, "A ed25519 public key was returned in an output buffer that wasn't large enough to hold the value.");
		ed25519_free(key);
		return false;
	}

	// Try and get a public key using a valid output buffer.
	else if (!ed25519_public_get(key, MANAGEDBUF(32))) {
		st_sprint(errmsg, "A ed25519 public key wasn't returned even though a valid output buffer was provided.");
		ed25519_free(key);
		return false;
	}

	// Try and generate a signature when the private key isn't available.
	else if (ed25519_sign(key, NULLER("Hello world!"), MANAGEDBUF(64))) {
		st_sprint(errmsg, "An ed25519 signature was generated without access to the private key.");
		ed25519_free(key);
		return false;
	}

	else if (!(holder = ed25519_public_get(key, MANAGEDBUF(32)))) {
		st_sprint(errmsg, "The ed25519 public key serialization failed even though a valid public key structure was provided.");
		ed25519_free(key);
		return false;
	}

	ed25519_free(key);

	// Setup a private key for more tests.s
	if (!(key = ed25519_private_set(holder))) {
		st_sprint(errmsg, "The ed25519 private key setup function accepted failed to accept a valid seed value.");
		return false;
	}

	// Try and generate a signature when the data buffer is NULL, or empty.
	else if (ed25519_sign(key, NULL, MANAGEDBUF(64)) || ed25519_sign(key, MANAGEDBUF(30), MANAGEDBUF(64))) {
		st_sprint(errmsg, "An ed25519 signature was generated even though the data buffer was invalid.");
		ed25519_free(key);
		return false;
	}

	// Try and generate a signature when the output buffer is too small.
	else if (ed25519_sign(key, NULLER("Hello world!"), MANAGEDBUF(32))) {
		st_sprint(errmsg, "An ed25519 signature was generated even though the data buffer was invalid.");
		ed25519_free(key);
		return false;
	}

	// Now let's generate a valid signature for further testing.
	else if (!(holder = ed25519_sign(key, NULLER("Hello world!"), MANAGEDBUF(64)))) {
		st_sprint(errmsg, "An ed25519 signing operation failed even though all the inputs were valid.");
		ed25519_free(key);
		return false;
	}

	// First let's verify the signature was indeed good.
	if (ed25519_verify(key, NULLER("Hello world!"), holder)) {
		st_sprint(errmsg, "The ed25519 signature failed to verify when it should have.");
		ed25519_free(key);
		return false;
	}

	// First let's verify the signature was indeed good.
	if (ed25519_verify(key, NULLER("Hello world!!"), holder) != -1 || ed25519_verify(key, NULLER("Hello world!!"), NULL) != -2 ||
		ed25519_verify(key, NULL, holder) != -2 || ed25519_verify(key, NULL, NULL) != -2 || ed25519_verify(NULL, NULLER("Hello world!"), holder) != -2) {
		st_sprint(errmsg, "The ed25519 verify operation didn't fail properly with invalid inputs.");
		ed25519_free(key);
		return false;
	}

	ed25519_free(key);
	return true;
}
