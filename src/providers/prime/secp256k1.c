
/**
 * @file /magma/src/providers/prime/secp256k1.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Allocate a new key pair using the secp256k1 curve.
 * @see		NID_secp256k1
 * @return	NULL on failure, or a pointer to the newly allocated key pair.
 */
EC_KEY * secp256k1_alloc(void) {

	EC_KEY *key = NULL;

	// Create a key and assign the group.
	if (prime_curve_group) {

		if (!(key = EC_KEY_new_d())) {
			log_info("An error occurred while initializing an empty secp256k1 key context. {%s}", ERR_error_string_d(ERR_get_error_d(), NULL));
		}
		else if (EC_KEY_set_group_d(key, prime_curve_group) != 1) {
			log_info("Unable to assign the default group to our empty key context.. {%s}", ERR_error_string_d(ERR_get_error_d(), NULL));
			EC_KEY_free_d(key);
			key = NULL;
		}

	}

	// If an error occurs above, or the PRIME module wasn't initialized, so we attempt key creation from scratch.
	if (!key) {

		if (!(key = EC_KEY_new_by_curve_name_d(NID_secp256k1))) {
			log_info("An error occurred while trying to create a new key using the secp256k1 curve. {%s}", ERR_error_string_d(ERR_get_error_d(), NULL));
			return NULL;
		}

		EC_KEY_set_conv_form_d(key, POINT_CONVERSION_COMPRESSED);
	}

	return key;
}

/**
 * @brief	Generate a random secp256k1 key pair.
 * @return	NULL on failure, or a new, randomly generated secp256k1 key pair on success.
 */
EC_KEY * secp256k1_generate(void) {

	EC_KEY *key = NULL;

	if (!(key = secp256k1_alloc())) {
		log_info("Unable to allocate an empty secp256k1 key context.");
		return NULL;
	}

	// This should generate a random key pair.
	if (EC_KEY_generate_key_d(key) != 1) {
		log_info("An error occurred while trying to generate a random secp256k1 key pair. {%s}", ERR_error_string_d(ERR_get_error_d(), NULL));
		EC_KEY_free_d(key);
		return NULL;
	}

	return key;
}

/**
 * @brief	Return a secp256k1 private key as a big endian integer inside a managed string.
 * @param	key	the input secp256k1 key pair.
 * @return	NULL on failure, or the private key as a big endian integer.
 */
stringer_t *secp256k1_private_get(EC_KEY *key, stringer_t *output) {

	int to_len;
	const BIGNUM *bn;
	stringer_t *result = NULL;

	// See if we have a valid output buffer, or if output is NULL, allocate a buffer to hold the output.
	if (output && (!st_valid_destination(st_opt_get(output)) || st_avail_get(output) < 32)) {
		log_pedantic("An output string was supplied but it does not represent a buffer capable of holding the output.");
		return NULL;
	}
	else if (!output && !(result = st_alloc(32))) {
		log_pedantic("Could not allocate a buffer large enough to hold encoded result. { requested = 32 }");
		return NULL;
	}
	else if (result) {
		output = result;
	}

	// Get the secret component as a BIGNUM structure.
	if (!(bn = EC_KEY_get0_private_key_d(key))) {
		log_pedantic("No private key available. {%s}", ERR_error_string_d(ERR_get_error_d(), NULL));
		st_cleanup(result);
		return NULL;
	}
	// Confirm the private key component will fit inside the 32 bytes provided, then write the BIGNUM out as
	// a big endian integer.
	else if (BN_num_bits_d(bn) > 256 || (to_len = BN_bn2bin_d(bn, st_data_get(output))) != 32) {
		log_pedantic("MPI conversion failed. {%s}", ERR_error_string_d(ERR_get_error_d(), NULL));
		st_cleanup(result);
		return NULL;
	}

	// Update the output buffer length.
	st_length_set(output, to_len);
	return output;
}

/**
 * @brief
 * @param key
 * @return
 */
EC_KEY *secp256k1_private_set(stringer_t *key) {

	BN_CTX *ctx = NULL;
	EC_POINT *pub = NULL;
	EC_KEY *output = NULL;
	BIGNUM *number = NULL;

	if (st_empty(key) || st_length_get(key) != 32) {
		log_info("An invalid key was passed in.");
		return NULL;
	}
	else if (!(output = secp256k1_alloc()) || !(ctx = BN_CTX_new_d()) || !(pub = EC_POINT_new_d(EC_KEY_get0_group_d(output)))) {
		log_info("An error occurred while trying to create a new key using the secp256k1 curve. {%s}", ERR_error_string_d(ERR_get_error_d(), NULL));
		if (output) EC_KEY_free_d(output);
		if (pub) EC_POINT_free_d(pub);
		if (ctx) BN_CTX_free_d(ctx);
		return NULL;
	}

	BN_CTX_start_d(ctx);

	// Decode a big endian integer into a BIGNUM structure.
	if (!(number = BN_bin2bn_d(st_data_get(key), st_length_get(key), NULL))) {
		log_info("An error occurred while parsing the binary elliptical curve point data used to represent the private key. {%s}", ERR_error_string_d(ERR_get_error_d(), NULL));
		EC_KEY_free_d(output);
		EC_POINT_free_d(pub);
		BN_CTX_free_d(ctx);
		return NULL;
	}

	// Set the decoded BIGNUM as the private exponent of the key object, then compute the public key point on the curve, and finally
	// set the result of the calculation up as the public component of the key object.
	if (EC_KEY_set_private_key_d(output, number) != 1 || EC_POINT_mul_d(EC_KEY_get0_group_d(output), pub, number, NULL, NULL, ctx) != 1 ||
		EC_KEY_set_public_key_d(output, pub) != 1) {

		log_info("The provided private key data could not be translated into a valid key structure. {%s}", ERR_error_string_d(ERR_get_error_d(), NULL));
		EC_KEY_free_d(output);
		EC_POINT_free_d(pub);
		BN_CTX_free_d(ctx);
		BN_free_d(number);
		return NULL;
	}

	// The above function calls duplicate the point and BIGNUM strcutures, so the local copies are no longer needed.
	EC_POINT_free_d(pub);
	BN_CTX_free_d(ctx);
	BN_free_d(number);

	return output;
}

