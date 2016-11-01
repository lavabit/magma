
/**
 * @file /magma/providers/cryptography/symmetric.c
 *
 * @brief Functions used to encrypt/decrypt data using symmetric ciphers.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/// HIGH: Alter the interfaces here to allow passing in the output buffer, or options which allow us to store the decrypted data in secure memory.

/**
 * @brief	Encrypt a block of data using a symmetric cipher.
 * @param	cipher	the cipher type being used to encrypt the data.
 * @param	vector	the IV used for the encryption operation.
 * @param	key		the symmetric encryption key used to encrypt the data.
 * @param	input	a managed string containing the data to be encrypted.
 * @return	NULL on failure, or a pointer to a managed string containing the encrypted data.
 */
stringer_t * symmetric_encrypt(cipher_t *cipher, stringer_t *vector, stringer_t *key, stringer_t *input) {

	EVP_CIPHER_CTX ctx;
	stringer_t *output = NULL;
	size_t vlen = 0, klen = 0, ilen = 0;
	int_t block_len, out_len, avail_len, used_len = 0, tag_len = 16;
	uchr_t *vdata = NULL, *kdata = NULL, *idata = NULL;

	if (!cipher || st_empty_out(key, &kdata, &klen) || st_empty_out(input, &idata, &ilen)) {
		log_pedantic("A required input parameter is missing.");
		return NULL;
	}

	st_empty_out(vector, &vdata, &vlen);

	// Setup the cipher context, the body length, and store a pointer to the body buffer location.
	EVP_CIPHER_CTX_init_d(&ctx);

	// Initialize the cipher with the envelope key.
	if (EVP_EncryptInit_ex_d(&ctx, (const EVP_CIPHER *)cipher, NULL, NULL, NULL) != 1) {
		log_pedantic("An error occurred while trying to initialize chosen symmetric cipher.");
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		return NULL;
	}

	// Validate the vector length.
	else if (EVP_CIPHER_CTX_iv_length_d(&ctx) != vlen) {
		log_pedantic("The initialization vector length does not match the selected symmetric cipher. {vector = %zu / expected = %i}", vlen,	EVP_CIPHER_CTX_iv_length_d(&ctx));
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		return NULL;
	}

	// Validate the key length.
	else if (EVP_CIPHER_CTX_key_length_d(&ctx) != klen) {
		log_pedantic("The encryption key length does not match the selected symmetric cipher. {key = %zu / expected = %i}", klen, EVP_CIPHER_CTX_key_length_d(&ctx));
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		return NULL;
	}

	// Setup a tag length of 16 bytes for CCM.
	else if ((EVP_CIPHER_flags_d((const EVP_CIPHER *)cipher) & EVP_CIPH_MODE) == EVP_CIPH_CCM_MODE) {

		// OpenSSL v1.1.0 uses EVP_CTRL_AEAD_SET_IVLEN and EVP_CTRL_AEAD_SET_TAG, but in the interim, the GCM defines below
		// will work correctly using CCM or GCM mode.
		if (EVP_CIPHER_CTX_ctrl_d(&ctx, EVP_CTRL_CCM_SET_TAG, tag_len, NULL) != 1) {
			log_pedantic("The authenticated symmetric cipher tag length could not be set to 16 bytes.");
			EVP_CIPHER_CTX_cleanup_d(&ctx);
			return NULL;
		}
	}

	// Add the vector and key data.
	if (EVP_EncryptInit_ex_d(&ctx, NULL, NULL, kdata, vdata) != 1) {
		log_pedantic("An error occurred initializing the symmetric cipher with the provided key and vector data.");
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		return NULL;
	}

	// Determine the block size and calculate the size of the output buffer.
	else if (!(block_len = EVP_CIPHER_CTX_block_size_d(&ctx)) || !(avail_len = out_len = (ilen + block_len + block_len - 1) & ~(block_len - 1))) {
		log_pedantic("An error occurred while trying to extract the block size of the selected cipher.");
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		return NULL;
	}

	// Output buffer.
	else if (!(output = st_alloc(out_len + tag_len))) {
		log_pedantic("Unable to allocate a buffer of %i bytes for the output buffer.", out_len);
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		return NULL;
	}

	// Encrypt the input buffer.
	else if (EVP_EncryptUpdate_d(&ctx, st_data_get(output), &avail_len, idata, ilen) != 1) {
		log_pedantic("An error occurred while trying to encrypt the input buffer using the chosen symmetric cipher.");
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		st_free(output);
		return NULL;
	}

	used_len += avail_len;
	avail_len = out_len - used_len;

	if (EVP_EncryptFinal_ex_d(&ctx, st_data_get(output) + used_len, &avail_len) != 1) {
		log_pedantic("An error occurred while trying to complete encryption process.");
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		st_free(output);
		return NULL;
	}

	// If were using an authenticated cipher, then we have an extra step. Storing the tag length.
	else if ((EVP_CIPHER_flags_d((const EVP_CIPHER *)cipher) & EVP_CIPH_MODE) == EVP_CIPH_GCM_MODE
	|| (EVP_CIPHER_flags_d((const EVP_CIPHER *)cipher) & EVP_CIPH_MODE) == EVP_CIPH_CCM_MODE) {

		// Note that EVP_CTRL_CCM_GET_TAG is aliased to EVP_CTRL_GCM_GET_TAG.
		EVP_CIPHER_CTX_ctrl_d(&ctx, EVP_CTRL_GCM_GET_TAG, tag_len, st_data_get(output) + used_len);
		st_length_set(output, used_len + avail_len + tag_len);

	}
	else {
		st_length_set(output, used_len + avail_len);
	}

	EVP_CIPHER_CTX_cleanup_d(&ctx);

	return output;
}

stringer_t * symmetric_decrypt(cipher_t *cipher, stringer_t *vector, stringer_t *key, stringer_t *input) {

	EVP_CIPHER_CTX ctx;
	stringer_t *output = NULL;
	size_t vlen = 0, klen = 0, ilen = 0;
	int_t block_len, out_len, avail_len, used_len = 0, tag_len = 0;
	uchr_t *vdata = NULL, *kdata = NULL, *idata = NULL;

	if (!cipher || st_empty_out(key, &kdata, &klen) || st_empty_out(input, &idata, &ilen)) {
		log_pedantic("A required input parameter is missing.");
		return NULL;
	}

	st_empty_out(vector, &vdata, &vlen);

	// Setup the cipher context, the body length, and store a pointer to the body buffer location.
	EVP_CIPHER_CTX_init_d(&ctx);

	// Initialize the cipher with the envelope key.
	if (EVP_DecryptInit_ex_d(&ctx, (const EVP_CIPHER *)cipher, NULL, NULL, NULL) != 1) {
		log_pedantic("An error occurred while trying to initialize chosen symmetric cipher.");
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		return NULL;
	}

	// Validate the vector length.
	else if (EVP_CIPHER_CTX_iv_length_d(&ctx) != vlen) {
		log_pedantic("The initialization vector length does not match the selected symmetric cipher. {vector = %zu / expected = %i}", vlen,	EVP_CIPHER_CTX_iv_length_d(&ctx));
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		return NULL;
	}

	// Validate the key length.
	else if (EVP_CIPHER_CTX_key_length_d(&ctx) != klen) {
		log_pedantic("The decryption key length does not match the selected symmetric cipher. {key = %zu / expected = %i}", klen, EVP_CIPHER_CTX_key_length_d(&ctx));
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		return NULL;
	}

	// Setup a tag length of 16 bytes for CCM and GCM.
	else if ((EVP_CIPHER_flags_d((const EVP_CIPHER *)cipher) & EVP_CIPH_MODE) == EVP_CIPH_CCM_MODE) {

		// OpenSSL v1.1.0 uses EVP_CTRL_AEAD_SET_IVLEN and EVP_CTRL_AEAD_SET_TAG, but in the interim, the GCM defines below
		// will work correctly using CCM or GCM mode.
		if (EVP_CIPHER_CTX_ctrl_d(&ctx, EVP_CTRL_CCM_SET_TAG, 16, NULL) != 1) {

			log_pedantic("The authenticated symmetric cipher tag length could not be set to 16 bytes.");
			EVP_CIPHER_CTX_cleanup_d(&ctx);
			return NULL;
		}
	}

	// Add the vector and key data.
	if (EVP_DecryptInit_ex_d(&ctx, NULL, NULL, kdata, vdata) != 1) {
		log_pedantic("An error occurred initializing the symmetric cipher with the provided key and vector data.");
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		return NULL;
	}

	// Determine the block size and calculate the size of the output buffer.
	else if (!(block_len = EVP_CIPHER_CTX_block_size_d(&ctx)) || !(avail_len = out_len = (ilen + block_len + block_len - 1) & ~(block_len - 1))) {
		log_pedantic("An error occurred while trying to extract the block size of the selected cipher.");
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		return NULL;
	}

	// Output buffer.
	else if (!(output = st_alloc(out_len))) {
		log_pedantic("Unable to allocate a buffer of %i bytes for the output buffer.", out_len);
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		return NULL;
	}

	// If were using an authenticated cipher, then we have an extra step. Storing the tag length.
	else if ((EVP_CIPHER_flags_d((const EVP_CIPHER *)cipher) & EVP_CIPH_MODE) == EVP_CIPH_GCM_MODE
		|| (EVP_CIPHER_flags_d((const EVP_CIPHER *)cipher) & EVP_CIPH_MODE) == EVP_CIPH_CCM_MODE) {

		tag_len = 16;

		// Note that EVP_CTRL_CCM_SET_TAG is aliased to EVP_CTRL_GCM_SET_TAG.
		EVP_CIPHER_CTX_ctrl_d(&ctx, EVP_CTRL_GCM_SET_TAG, tag_len, idata + (ilen - tag_len));
	}

	// Decrypt the input buffer.
	if (EVP_DecryptUpdate_d(&ctx, st_data_get(output), &avail_len, idata, ilen - tag_len) != 1) {
		log_pedantic("An error occurred while trying to decrypt the input buffer using the chosen symmetric cipher.");
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		st_free(output);
		return NULL;
	}

	used_len += avail_len;
	avail_len = out_len - used_len;

	// Calling EVP_DecryptFinal_ex() when using CCM will always return an error.
	if ((EVP_CIPHER_flags_d((const EVP_CIPHER *)cipher) & EVP_CIPH_MODE) != EVP_CIPH_CCM_MODE &&
		EVP_DecryptFinal_ex_d(&ctx, st_data_get(output) + used_len, &avail_len) != 1) {

		log_pedantic("An error occurred while trying to complete decryption process. {%s}", ssl_error_string(MEMORYBUF(256), 256));
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		st_free(output);
		return NULL;
	}
	// Normally avail_len gets zero'ed by the call to EVP_DecryptFinal_ex() above, but in CCM mode we don't make that
	// function call, so we zero out the avail space counter, or the st_length_set() call below will incorrectly claim
	// empty trailing space is actual data.
	else if ((EVP_CIPHER_flags_d((const EVP_CIPHER *)cipher) & EVP_CIPH_MODE) == EVP_CIPH_CCM_MODE) {
		avail_len = 0;
	}

	st_length_set(output, used_len + avail_len);
	EVP_CIPHER_CTX_cleanup_d(&ctx);

	return output;
}

/**
 * @brief	Generate an IV data suitable for the specified cipher.
 * @note	If output is NULL, a new managed string will be allocated and returned by the function.
 * @param	cipher	the cipher type to be used.
 * @param	output	the managed string that will store the IV data.
 * @return	NULL on failure, or a pointer to the managed string that contains the IV data.
 */
stringer_t * symmetric_vector(cipher_t *cipher, stringer_t *output) {

	size_t len;
	stringer_t *result = NULL;

	// Find out how big a the initialization vector should be.
	if (!cipher || !(len = cipher_vector_length(cipher))) {
		return output;
	}

	// Checks whether the provided buffer is sufficient, or if NULL was passed in allocates a buffer.
	else if (!(result = st_output(output, len))) {
		return NULL;
	}

	if (rand_write(PLACER(st_data_get(result), len)) != len) {
		log_pedantic("Unable to fill the initialization vector buffer with %zu bytes of random data.", len);
		if (!output) st_free(result);
		return NULL;
	}

	if (st_valid_tracked(*((uint32_t *)result))) {
		st_length_set(result, len);
	}

	return result;
}

/**
 * @brief	Derive a symmetric key from a user's password.
 * @note	Depending on the length of the password, either SHA1, SHA224, or SHA256 may be used as the hashing algorithm.
 * @note	If output is passed as NULL, a new managed string will be allocated to hold the output of the operation.
 * @param	cipher	the specified cipher type.
 * @param	key		the user's password, as a managed string.
 * @param	output	the output managed string to receive the digested password. Can be NULL.
 * @return	NULL on failure, or a pointer to the managed string containing the key.
 */
stringer_t * symmetric_key(cipher_t *cipher, stringer_t *key, stringer_t *output) {

	size_t len;
	stringer_t *result = NULL;

	// Find out how big the initialization vector should be.
	if (!cipher || !(len = cipher_key_length(cipher))) {

		return NULL;
	}

	// Checks whether the provided buffer is sufficient, or if NULL was passed in, we allocate a buffer.
	else if (!(result = st_output(output, EVP_MAX_KEY_LENGTH))) {
		return NULL;
	}

	if (len <= 20) {
		hash_sha1(key, result);
	}
	else if (len <= 24) {
		hash_sha224(key, result);
	}
	else if (len <= 32) {
		hash_sha256(key, result);
	}

	// If the hash isn't the correct length, trim the output.
	if (st_valid_tracked(*((uint32_t *)result)) && st_length_get(result) != len) {
		st_length_set(result, len);
	}

	return result;
}


