
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
	int_t block_len, out_len, avail_len, used_len = 0;
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

	// Add the vector and key data.
	else if (EVP_EncryptInit_ex_d(&ctx, NULL, NULL, kdata, vdata) != 1) {
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

	st_length_set(output, used_len + avail_len);
	EVP_CIPHER_CTX_cleanup_d(&ctx);

	return output;
}

stringer_t * symmetric_decrypt(cipher_t *cipher, stringer_t *vector, stringer_t *key, stringer_t *input) {

	EVP_CIPHER_CTX ctx;
	stringer_t *output = NULL;
	size_t vlen = 0, klen = 0, ilen = 0;
	int_t block_len, out_len, avail_len, used_len = 0;
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

	// Add the vector and key data.
	else if (EVP_DecryptInit_ex_d(&ctx, NULL, NULL, kdata, vdata) != 1) {
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

	// Decrypt the input buffer.
	else if (EVP_DecryptUpdate_d(&ctx, st_data_get(output), &avail_len, idata, ilen) != 1) {
		log_pedantic("An error occurred while trying to decrypt the input buffer using the chosen symmetric cipher.");
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		st_free(output);
		return NULL;
	}

	used_len += avail_len;
	avail_len = out_len - used_len;

	if (EVP_DecryptFinal_ex_d(&ctx, st_data_get(output) + used_len, &avail_len) != 1) {
		log_pedantic("An error occurred while trying to complete decryption process. {%s}", ERR_error_string_d(ERR_get_error_d(), NULL));
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		st_free(output);
		return NULL;
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

	// Find out how big a the initialization vector should be.
	if (!cipher || !(len = cipher_vector_length(cipher))) {
		return output;
	}

	// Checks whether the provided buffer is sufficient, or if NULL was passed in allocates a buffer.
	else if (!(output = st_output(output, len))) {
		return NULL;
	}

	if (rand_write(PLACER(st_data_get(output), len)) != len) {
		log_pedantic("Unable to fill the initialization vector buffer with %zu bytes of random data.", len);
		return NULL;
	}

	if (st_valid_tracked(*((uint32_t *)output))) {
		st_length_set(output, len);
	}

	return output;
}

/**
 * @brief	Derive a symmetric key from a user's password.
 * @note	Depending on the length of the password, either SHA1, SHA224, or SHA256 may be used as the hashing algorithm.
 * @note	If output is passed as NULL, a new managed string will be allocated to hold the output of the operation.
 * @param	cipher	the specified cipher type.
 * @param	key		the user's password, as a managed string.
 * @param	output	the output managed string to receive the digested password. Can be NULL.
 * @return	NULL on failure, or a pointer to the managed string containing the digested password.
 */
stringer_t * symmetric_key(cipher_t *cipher, stringer_t *key, stringer_t *output) {

	size_t len;

	// Find out how big the initialization vector should be.
	if (!cipher || !(len = cipher_key_length(cipher))) {
		return output;
	}

	// Checks whether the provided buffer is sufficient, or if NULL was passed in, we allocate a buffer.
	else if (!(output = st_output(output, EVP_MAX_KEY_LENGTH))) {
		return NULL;
	}

	if (len <= 20) {
		hash_sha1(key, output);
	}
	else if (len <= 24) {
		hash_sha224(key, output);
	}
	else if (len <= 32) {
		hash_sha256(key, output);
	}

	// If the hash isn't the correct length, trim the output.
	if (st_valid_tracked(*((uint32_t *)output)) && st_length_get(output) != len) {
		st_length_set(output, len);
	}

	return output;
}


