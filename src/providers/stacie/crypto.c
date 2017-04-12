
/**
 * @file /magma/src/providers/stacie/crypto.c
 *
 * @brief Use a realm key to encrypt/decrypt data.
 */

#include "magma.h"

/**
 * @brief
 *
 * @param serial
 * @param vector_key
 * @param tag_key
 * @param cipher_key
 * @param buffer
 *
 * @return
 */
stringer_t * stacie_encrypt(uint16_t serial, stringer_t *vector_key, stringer_t *tag_key, stringer_t *cipher_key, stringer_t *buffer) {

	uint8_t pad = 0;
	uint16_t serial_be;
	size_t buffer_len;
	EVP_CIPHER_CTX ctx;
	uchr_t *buffer_data, tag[16];
	uint32_t buffer_len_be;
	int_t  out_len, avail_len, used_len = 0;
	stringer_t *vector = MANAGEDBUF(16), *vector_shard = MANAGEDBUF(16), *tag_shard = MANAGEDBUF(16), *output = NULL;

	if (st_length_get(vector_key) != 16) {
		log_error("STACIE realm encryption requires a 16 byte vector key. { vector_key_len = %zu }", st_length_get(vector_key));
		return NULL;
	}
	else if (st_length_get(tag_key) != 16) {
		log_error("STACIE realm encryption requires a 16 byte tag key. { tag_key_len = %zu }", st_length_get(tag_key));
		return NULL;
	}
	else if (st_length_get(cipher_key) != 32) {
		log_error("STACIE realm encryption requires a 32 byte cipher key. { cipher_key_len = %zu }", st_length_get(cipher_key));
		return NULL;
	}
	else if (st_empty_out(buffer, &buffer_data, &buffer_len) || buffer_len < STACIE_ENCRYPT_MIN || buffer_len > STACIE_ENCRYPT_MAX) {
		log_error("STACIE realm encryption requires an input buffer with a minimum of 1 byte and a maximum of 16,777,215 bytes. { buffer_len = %zu }",
			buffer_len);
		return NULL;
	}

	// Generate a random initialization vector shard which will be combined with the vector key.
	else if (rand_write(vector_shard) != 16 || !st_xor(vector_key, vector_shard, vector)) {
		log_error("The STACIE realm encryption failed because a random vector could not be generated.");
		return NULL;
	}

	// Calculate the number of padding bytes required.
	pad = STACIE_BLOCK_LENGTH - ((buffer_len + 4) % STACIE_BLOCK_LENGTH);
	avail_len = buffer_len + pad + 4;
	out_len = STACIE_ENVELOPE_LENGTH + avail_len;

	// Create the big endian representations of the serial number and the plain text length.
	serial_be = htobe16(serial);
	buffer_len_be = htobe32(buffer_len);

	// Setup the cipher context, the body length, and store a pointer to the body buffer location.
	EVP_CIPHER_CTX_init_d(&ctx);

	// Initialize the cipher context.
	if (EVP_EncryptInit_ex_d(&ctx, EVP_aes_256_gcm_d(), NULL, NULL, NULL) != 1) {
		log_pedantic("An error occurred while trying to initialize chosen symmetric cipher.");
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		return NULL;
	}

	// Set the vector length.
	if (EVP_CIPHER_CTX_ctrl_d(&ctx, EVP_CTRL_GCM_SET_IVLEN, 16, NULL) != 1) {
		log_pedantic("The initialization vector length could not be properly set to 16 bytes.");
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		return NULL;
	}

	// Validate the key length.
	else if (EVP_CIPHER_CTX_key_length_d(&ctx) != 32) {
		log_pedantic("The encryption key length does not match the selected cipher key length. { key = %i / expected = %i }", 32, EVP_CIPHER_CTX_key_length_d(&ctx));
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		return NULL;
	}

	// Add the vector and key data.
	if (EVP_EncryptInit_ex_d(&ctx, NULL, NULL, st_data_get(cipher_key), st_data_get(vector)) != 1) {
		log_pedantic("An error occurred initializing the symmetric cipher with the provided key and vector data.");
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		return NULL;
	}

	// Allocate the output buffer.
	if (!(output = st_alloc(out_len))) {
		log_pedantic("Unable to allocate a buffer of %i bytes for the output buffer.", avail_len);
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		return NULL;
	}

	// Write the plain text length into the cipher text as a 24 bit big endian integer.
	if (EVP_EncryptUpdate_d(&ctx, st_data_get(output) + STACIE_ENVELOPE_LENGTH, &avail_len, ((uchr_t *)&buffer_len_be) + 1, 3) != 1) {
		log_pedantic("An error occurred while trying to encrypt the input buffer using the chosen symmetric cipher.");
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		st_free(output);
		return NULL;
	}

	used_len += avail_len;
	avail_len = out_len - STACIE_ENVELOPE_LENGTH - used_len;

	// Write the pad length into the cipher text as an 8 bit integer.
	if (EVP_EncryptUpdate_d(&ctx, st_data_get(output) + STACIE_ENVELOPE_LENGTH + used_len, &avail_len, ((uchr_t *)&pad), 1) != 1) {
		log_pedantic("An error occurred while trying to encrypt the input buffer using the chosen symmetric cipher.");
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		st_free(output);
		return NULL;
	}

	used_len += avail_len;
	avail_len = out_len - STACIE_ENVELOPE_LENGTH - used_len;

	// Encrypt the plain text buffer.
	if (EVP_EncryptUpdate_d(&ctx, st_data_get(output) + STACIE_ENVELOPE_LENGTH + used_len, &avail_len, buffer_data, buffer_len) != 1) {
		log_pedantic("An error occurred while trying to encrypt the input buffer using the chosen symmetric cipher.");
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		st_free(output);
		return NULL;
	}

	used_len += avail_len;
	avail_len = out_len - STACIE_ENVELOPE_LENGTH - used_len;

	// Encrypt the padding bytes (if necessary).
	for (int_t i = 0; i < pad; i++) {
		if (EVP_EncryptUpdate_d(&ctx, st_data_get(output) + STACIE_ENVELOPE_LENGTH + used_len, &avail_len, ((uchr_t *)&pad), 1) != 1) {
			log_pedantic("An error occurred while trying to encrypt the input buffer using the chosen symmetric cipher.");
			EVP_CIPHER_CTX_cleanup_d(&ctx);
			st_free(output);
			return NULL;
		}

		used_len += avail_len;
		avail_len = out_len - STACIE_ENVELOPE_LENGTH - used_len;
	}

	if (EVP_EncryptFinal_ex_d(&ctx, st_data_get(output) + STACIE_ENVELOPE_LENGTH + used_len, &avail_len) != 1) {
		log_pedantic("An error occurred while trying to complete encryption process.");
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		st_free(output);
		return NULL;
	}

	used_len += avail_len;

	if (EVP_CIPHER_CTX_ctrl_d(&ctx, EVP_CTRL_GCM_GET_TAG, 16, &tag[0]) != 1 || st_xor(tag_key, PLACER(&tag[0], 16), tag_shard) != tag_shard) {
		log_pedantic("An error occurred while trying to retrieve the tag value.");
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		st_free(output);
		return NULL;
	}

	EVP_CIPHER_CTX_cleanup_d(&ctx);

	mm_move(st_data_get(output), ((uchr_t *)&serial_be), 2);
	mm_move(st_data_get(output) + 2, st_data_get(vector_shard), 16);
	mm_move(st_data_get(output) + 18, st_data_get(tag_shard), 16);
	st_length_set(output, STACIE_ENVELOPE_LENGTH + used_len);

	return output;
}

/**
 * @brief
 *
 * @param vector_key
 * @param tag_key
 * @param cipher_key
 * @param buffer
 *
 * @return
 */
stringer_t * stacie_decrypt(stringer_t *vector_key, stringer_t *tag_key, stringer_t *cipher_key, stringer_t *buffer) {

	uint8_t pad = 0;
	size_t buffer_len;
	EVP_CIPHER_CTX ctx;
	uchr_t *buffer_data;
	uint32_t buffer_len_be = 0;
	int_t  out_len, avail_len, used_len = 0;
	stringer_t *vector = MANAGEDBUF(16), *tag = MANAGEDBUF(16), *output = NULL;

	if (st_length_get(vector_key) != 16) {
		log_error("STACIE realm encryption requires a 16 byte vector key. { vector_key_len = %zu }", st_length_get(vector_key));
		return NULL;
	}
	else if (st_length_get(tag_key) != 16) {
		log_error("STACIE realm encryption requires a 16 byte tag key. { tag_key_len = %zu }", st_length_get(tag_key));
		return NULL;
	}
	else if (st_length_get(cipher_key) != 32) {
		log_error("STACIE realm encryption requires a 32 byte cipher key. { cipher_key_len = %zu }", st_length_get(cipher_key));
		return NULL;
	}
	else if (st_empty_out(buffer, &buffer_data, &buffer_len) || buffer_len < 54) {
		log_error("STACIE realm encryption generates a minimum of 54 bytes. { buffer_len = %zu }",
			buffer_len);
		return NULL;
	}

	st_xor(PLACER(buffer_data + 2, 16), vector_key, vector);
	st_xor(PLACER(buffer_data + 18, 16), tag_key, tag);

	out_len = avail_len = (buffer_len - STACIE_ENVELOPE_LENGTH);

	// Setup the cipher context, the body length, and store a pointer to the body buffer location.
	EVP_CIPHER_CTX_init_d(&ctx);

	// Initialize the cipher context.
	if (EVP_DecryptInit_ex_d(&ctx, EVP_aes_256_gcm_d(), NULL, NULL, NULL) != 1) {
		log_pedantic("An error occurred while trying to initialize chosen symmetric cipher.");
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		return NULL;
	}

	if (EVP_CIPHER_CTX_ctrl_d(&ctx, EVP_CTRL_GCM_SET_IVLEN, 16, NULL) != 1) {
		log_pedantic("The initialization vector length could not be properly set to 16 bytes.");
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		return NULL;
	}

	// Validate the key length.
	else if (EVP_CIPHER_CTX_key_length_d(&ctx) != 32) {
		log_pedantic("The encryption key length does not match the selected cipher key length. { key = %i / expected = %i }", 32, EVP_CIPHER_CTX_key_length_d(&ctx));
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		return NULL;
	}

	// Add the vector and key data.
	if (EVP_DecryptInit_ex_d(&ctx, NULL, NULL, st_data_get(cipher_key), st_data_get(vector)) != 1) {
		log_pedantic("An error occurred initializing the symmetric cipher with the provided key and vector data.");
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		return NULL;
	}

	if (!(output = st_alloc(out_len))) {
		log_pedantic("Unable to allocate a buffer of %i bytes for the output buffer.", avail_len);
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		return NULL;
	}

	if (EVP_DecryptUpdate_d(&ctx, st_data_get(output), &avail_len, buffer_data + STACIE_ENVELOPE_LENGTH, buffer_len - STACIE_ENVELOPE_LENGTH) != 1) {
		log_pedantic("An error occurred while trying to decrypt the input buffer using the chosen symmetric cipher.");
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		st_free(output);
		return NULL;
	}

	used_len += avail_len;
	avail_len = out_len - used_len;

	if (EVP_CIPHER_CTX_ctrl_d(&ctx, EVP_CTRL_GCM_SET_TAG, 16, st_data_get(tag)) != 1) {
		log_pedantic("An error occurred while trying to set the decryption tag value.");
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		st_free(output);
		return NULL;
	}

	if (EVP_DecryptFinal_ex_d(&ctx, st_data_get(output) + used_len, &avail_len) != 1) {
		log_pedantic("An error occurred while trying to complete decryption process. {%s}", ssl_error_string(MEMORYBUF(256), 256));
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		st_free(output);
		return NULL;
	}

	EVP_CIPHER_CTX_cleanup_d(&ctx);

	mm_move(((uchr_t *)&buffer_len_be) + 1, st_data_get(output), 3);
	mm_move((uchr_t *)&pad, st_data_get(output) + 3, 1);
	buffer_len = be32toh(buffer_len_be);

	if (((buffer_len + pad + 4) % 16) != 0 || (buffer_len + pad + 4) != (size_t)used_len) {
		log_pedantic("An invalid length or pad value was found inside the decrypted buffer.");
		st_free(output);
		return NULL;
	}

	buffer_data = st_data_get(output) + buffer_len + 4;
	for (int_t i = 0; i < pad; i++) {
		if (*buffer_data++ != pad) {
			log_pedantic("An invalid padding byte was found inside the decrypted buffer.");
			st_free(output);
			return NULL;
		}
	}

	mm_move(st_data_get(output), st_data_get(output) + 4, buffer_len);
	st_length_set(output, buffer_len);

	return output;
}

