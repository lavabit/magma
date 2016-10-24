
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
 * @brief	Free a secp256k1 key structure.
 * @see		EC_KEY_free()
 *
 * @param	key	the managed string to be freed.
 * @return	This function returns no value.
 */
void secp256k1_free(EC_KEY *key) {

#ifdef MAGMA_PEDANTIC
	if (!key) {
		log_pedantic("Attempted to free a NULL secp256k1 key pointer.");
	}
#endif

	if (key) {
		EC_KEY_free_d(key);
	}

	return;
}

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
	if (!key && !(key = EC_KEY_new_by_curve_name_d(NID_secp256k1))) {
		log_info("An error occurred while trying to create a new key using the secp256k1 curve. {%s}", ERR_error_string_d(ERR_get_error_d(), NULL));
		return NULL;
	}

	EC_KEY_set_conv_form_d(key, POINT_CONVERSION_COMPRESSED);

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
 * @brief	Return a secp256k1 public key as a compressed point ecoded as a big endian integer.
 * @param	key	the input secp256k1 key pair.
 * @return	NULL on failure, or the public key as a big endian integer.
 */
stringer_t *secp256k1_public_get(EC_KEY *key, stringer_t *output) {

	size_t len = 0;
	stringer_t *result = NULL;

	if (!key) {
		log_pedantic("An invalid key pointer was passed in.");
		return NULL;
	}

	// See if we have a valid output buffer, or if output is NULL, allocate a buffer to hold the output.
	if (output && (!st_valid_destination(st_opt_get(output)) || st_avail_get(output) < 33)) {
		log_pedantic("An output string was supplied but it does not represent a buffer capable of holding the output.");
		return NULL;
	}
	else if (!output && !(result = st_alloc(33))) {
		log_pedantic("Could not allocate a buffer large enough to hold encoded result. { requested = 33 }");
		return NULL;
	}
	else if (result) {
		output = result;
	}

	// Confirm the compressed point will result in 33 bytes of data, then write out the public key as a compressed point.
	if (EC_POINT_point2oct_d(EC_KEY_get0_group_d(key), EC_KEY_get0_public_key_d(key), EC_KEY_get_conv_form_d(key), NULL, 0, NULL) != 33 ||
		(len = EC_POINT_point2oct_d(EC_KEY_get0_group_d(key), EC_KEY_get0_public_key_d(key), EC_KEY_get_conv_form_d(key), st_data_get(output), 33, NULL)) != 33) {
		log_pedantic("Serialization of the public key into a multiprecision integer failed. { len = %zu / error = %s }", len, ERR_error_string_d(ERR_get_error_d(), NULL));
		st_cleanup(result);
		return NULL;
	}

	// Update the output buffer length.
	st_length_set(output, len);
	return output;
}

/**
 * @brief	Return a secp256k1 private key as a big endian integer inside a managed string.
 * @param	key	the input secp256k1 key pair.
 * @return	NULL on failure, or the private key as a big endian integer.
 */
stringer_t *secp256k1_private_get(EC_KEY *key, stringer_t *output) {

	size_t len = 0;
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

	// Wipe the buffer so any leading bytes we don't use will be zero'ed out for padding purposes.
	st_wipe(output);

	// Get the secret component as a BIGNUM structure.
	if (!(bn = EC_KEY_get0_private_key_d(key))) {
		log_pedantic("No private key available. {%s}", ERR_error_string_d(ERR_get_error_d(), NULL));
		st_cleanup(result);
		return NULL;
	}
	// Confirm the private key component will fit inside the 32 bytes provided, then write the BIGNUM out as
	// a big endian integer.
	if (BN_num_bits_d(bn) > 256 || (len = BN_num_bytes_d(bn)) > 32) {
		log_pedantic("Serialization of the private key failed because the size was larger then expected. { len = %zu / bits = %i / error = %s }",
			len, BN_num_bits_d(bn), ERR_error_string_d(ERR_get_error_d(), NULL));
		st_cleanup(result);
		return NULL;
	}
	// Write the key into the output buffer. Advance the pointer to account for any padding that might be needed.
	else if (BN_bn2bin_d(bn, st_data_get(output) + (32 - len)) != len) {
		log_pedantic("Serialization of the private key into a multiprecision integer failed. { len = %zu / bits = %i / error = %s }",
			len, BN_num_bits_d(bn), ERR_error_string_d(ERR_get_error_d(), NULL));
		st_cleanup(result);
		return NULL;
	}

	// Update the output buffer length.
	st_length_set(output, 32);
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

