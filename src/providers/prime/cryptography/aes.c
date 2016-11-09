
/**
 * @file /magma/src/providers/prime/cryptography/aes.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

#define PRIME_OBJECT_KEY_LEN 64				// 16 + 16 + 32 = 64
#define PRIME_OBJECT_HEAD_LEN 38			// 2 + 4 + 16 + 16 = 38
#define PRIME_OBJECT_PAYLOAD_PREFIX_LEN 4	// 1 + 3 = 4

// The layout for an encrypted PRIME object.
typedef struct __attribute__ ((packed)) {

	uint16_t type;			// 2b  / Big endian type number.
	uint32_t size;			// 4b  / Big endian object size.
	uchr_t vector[16];		// 16b / Vector shard.
	uchr_t tag[16];			// 16b / Tag shard.
							// ~~~ / Encrypted payload.

} prime_encrypted_object_header_t;

// The layout for the encrypted PRIME payload.
typedef struct __attribute__ ((packed)) {

	uint32_t size : 24;		// 3b  / Big endian payload size.
	uint32_t pad : 8;		// 1b  / The number of padding bytes.
							// ~~~ / The object data.
							// ~~~ / The padding bytes.

} prime_encrypted_payload_prefix_t;

/**
 * @brief	Extract the symmetric cipher portion of the key.
 *
 * @param key	The complete key, which holds the vector and tag shard values along with the cipher key.
 *
 * @return	returns a place holder with the cipher key portion, or NULL if the supplied key was invalid.
 */
placer_t aes_cipher_key(stringer_t *key) {

	placer_t cipher_key = pl_null();

	if (st_empty(key) || st_length_get(key) != PRIME_OBJECT_KEY_LEN) {
		log_error("The cipher key extraction failed because the provided key wasn't the correct length. { length = %zu }", st_length_get(key));
	}
	else {
		cipher_key = pl_init(st_data_get(key) + AES_VECTOR_LEN + AES_TAG_LEN, AES_KEY_LEN);
	}

	return cipher_key;
}

/**
 * @brief   Extract the portion of the key used for the vector shard.
 *
 * @param key	The complete key, which holds the vector and tag shard values along with the cipher key.
 *
 * @return	returns a place holder with the vector shard, or NULL if the supplied key was invalid.
 */
placer_t aes_vector_shard(stringer_t *key) {

	placer_t vector_shard = pl_null();

	if (st_empty(key) || st_length_get(key) != PRIME_OBJECT_KEY_LEN) {
		log_error("The vector shard extraction failed because the provided key wasn't the correct length. { length = %zu }", st_length_get(key));
	}
	else {
		vector_shard = pl_init(st_data_get(key), AES_VECTOR_LEN);
	}

	return vector_shard;
}

/**
 * @brief   Extract the portion of the key used for the tag shard.
 *
 * @param key	The complete key, which holds the vector and tag shard values along with the cipher key.
 *
 * @return	returns a placeholder with the tag shard, or NULL if the supplied key was invalid.
 */
placer_t aes_tag_shard(stringer_t *key) {

	placer_t tag_shard = pl_null();

	if (st_empty(key) || st_length_get(key) != PRIME_OBJECT_KEY_LEN) {
		log_error("The tag shard extraction failed because the provided key wasn't the correct length. { length = %zu }", st_length_get(key));
	}
	else {
		tag_shard = pl_init(st_data_get(key) + AES_VECTOR_LEN, AES_TAG_LEN);
	}

	return tag_shard;
}

stringer_t * aes_object_encrypt(stringer_t *key, uint16_t type, stringer_t *object, stringer_t *output) {

	uint8_t pad = 0;
	EVP_CIPHER_CTX ctx;
	size_t object_size;
	uint16_t big_endian_type = 0;
	uchr_t *object_data, tag[AES_TAG_LEN];
	prime_encrypted_payload_prefix_t prefix;
	prime_encrypted_object_header_t *header = NULL;
	int_t payload_len = 0, available = 0, written = 0;
	uint32_t big_endian_size = 0, big_endian_object_size = 0;
	placer_t cipher_key = pl_null(), tag_key_shard = pl_null(), vector_key_shard = pl_null();
	stringer_t *vector = MANAGEDBUF(AES_VECTOR_LEN), *vector_rand_shard = MANAGEDBUF(AES_VECTOR_LEN),
		*tag_shard = MANAGEDBUF(AES_TAG_LEN), *result = NULL;

	if (st_length_get(key) != PRIME_OBJECT_KEY_LEN) {
		log_pedantic("PRIME object encryption requires a %i byte key. { length = %zu }", PRIME_OBJECT_KEY_LEN, st_length_get(key));
		return NULL;
	}
	else if (st_empty_out(object, &object_data, &object_size)) {
		log_pedantic("PRIME object encryption requires a plain text payload to protect. { length = %zu }", object_size);
		return NULL;
	}

	// Extract the key fragments.
	cipher_key = aes_cipher_key(key);
	tag_key_shard = aes_tag_shard(key);
	vector_key_shard = aes_vector_shard(key);

	// Check to ensure all of the fragments were extracted properly.
	if (pl_empty(cipher_key) || pl_empty(tag_key_shard) || pl_empty(vector_key_shard)) {
		log_pedantic("PRIME object encryption failed because an invalid key was provided.");
		return NULL;
	}

	// Generate a random initialization vector shard which will be combined with the vector key.
	else if (rand_write(vector_rand_shard) != AES_VECTOR_LEN || !st_xor(&vector_key_shard, vector_rand_shard, vector)) {
		log_pedantic("PRIME object encryption failed because a random vector could not be generated.");
		return NULL;
	}

	// Calculate the number of padding bytes required, and then the overall payload length.
	pad = AES_BLOCK_LEN - ((PRIME_OBJECT_PAYLOAD_PREFIX_LEN + object_size) % AES_BLOCK_LEN);
	available = payload_len = PRIME_OBJECT_PAYLOAD_PREFIX_LEN + object_size + pad;

	// Generate the big endian representation of the numbers being serialized.
	big_endian_type = htobe16(type);
	big_endian_size = htobe32(AES_TAG_LEN + AES_VECTOR_LEN + payload_len);
	big_endian_object_size = htobe32(object_size);

	// See if we have a valid output buffer, and make sure it is large enough to hold the result.
	if (output && (!st_valid_destination(st_opt_get(output)) || st_avail_get(output) < (PRIME_OBJECT_HEAD_LEN + payload_len))) {
		log_pedantic("An output string was supplied but it does not represent a buffer capable of holding the output.");
		return NULL;
	}
	// If the output buffer is NULL, then we'll allocate a buffer for the result.
	else if (!output && !(output = result = st_alloc(PRIME_OBJECT_HEAD_LEN + payload_len))) {
		log_pedantic("Could not allocate a buffer large enough to hold encrypted result. { requested = %i }", PRIME_OBJECT_HEAD_LEN + payload_len);
		return NULL;
	}

	// Wipe the buffer.
	st_wipe(output);

	// Write the object header (everything except the tag).
	header = st_data_get(output);
	header->type = big_endian_type;
	header->size = big_endian_size;
	mm_copy(&(header->vector), st_data_get(vector_rand_shard), 16);

	// Setup the payload prefix for later.
	mm_copy(&(prefix), ((uchr_t *)&big_endian_object_size) + 1, 3);
	prefix.pad = pad;

	// Setup the cipher context.
	EVP_CIPHER_CTX_init_d(&ctx);

	// Initialize the cipher context.
	if (EVP_EncryptInit_ex_d(&ctx, EVP_aes_256_gcm_d(), NULL, NULL, NULL) != 1) {
		log_pedantic("An error occurred while trying to initialize chosen symmetric cipher.");
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		st_cleanup(result);
		return NULL;
	}

	// Set the vector length.
	else if (EVP_CIPHER_CTX_ctrl_d(&ctx, EVP_CTRL_GCM_SET_IVLEN, AES_VECTOR_LEN, NULL) != 1) {
		log_pedantic("The initialization vector length could not be properly set to %i bytes.", AES_VECTOR_LEN);
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		st_cleanup(result);
		return NULL;
	}

	// Validate the key length.
	else if (EVP_CIPHER_CTX_key_length_d(&ctx) != AES_KEY_LEN) {
		log_pedantic("The cipher key size isn't what we expected. { key = %i / expected = %i }",
			AES_KEY_LEN, EVP_CIPHER_CTX_key_length_d(&ctx));
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		st_cleanup(result);
		return NULL;
	}

	// Add the vector and cipher key to the encryption context.
	else if (EVP_EncryptInit_ex_d(&ctx, NULL, NULL, st_data_get(key), st_data_get(vector)) != 1) {
		log_pedantic("An error occurred initializing the symmetric cipher with the provided vector and key. { error = %s }",
			ssl_error_string(MEMORYBUF(256), 256));
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		st_cleanup(result);
		return NULL;
	}

	// Encrypt the payload prefix, which consists of a byte 24 bit big endian length, and an 8 bit padding count.
	if (EVP_EncryptUpdate_d(&ctx, st_data_get(output) + PRIME_OBJECT_HEAD_LEN, &available, ((uchr_t *)&prefix), PRIME_OBJECT_PAYLOAD_PREFIX_LEN) != 1 ||
		available != PRIME_OBJECT_PAYLOAD_PREFIX_LEN) {

		log_pedantic("An error occurred while trying to encrypt the input buffer using the chosen symmetric cipher. { error = %s }",
			ssl_error_string(MEMORYBUF(256), 256));
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		st_cleanup(result);
		return NULL;
	}

	written += available;
	available = payload_len - written;

	// Encrypt the plain text buffer.
	if (EVP_EncryptUpdate_d(&ctx, st_data_get(output) + PRIME_OBJECT_HEAD_LEN + written, &available, object_data, object_size) != 1 ||
		available != object_size) {

		log_pedantic("An error occurred while trying to encrypt the input buffer using the chosen symmetric cipher. { error = %s }",
			ssl_error_string(MEMORYBUF(256), 256));
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		st_cleanup(result);
		return NULL;
	}

	written += available;
	available = payload_len - written;

	// Encrypt the padding bytes (if necessary).
	for (int_t i = 0; i < pad; i++) {
		if (EVP_EncryptUpdate_d(&ctx, st_data_get(output) + PRIME_OBJECT_HEAD_LEN + written, &available, ((uchr_t *)&pad), 1) != 1 ||
		available != 1) {

			log_pedantic("An error occurred while trying to encrypt the input buffer using the chosen symmetric cipher. { error = %s }",
				ssl_error_string(MEMORYBUF(256), 256));
			EVP_CIPHER_CTX_cleanup_d(&ctx);
			st_cleanup(result);
			return NULL;
		}

		written += available;
		available = payload_len - written;
	}

	// Calling the finalization routine is what generates the tamper tag, but we'll need to make a separate call to retrieve it.
	if (EVP_EncryptFinal_ex_d(&ctx, st_data_get(output) + PRIME_OBJECT_HEAD_LEN + written, &available) != 1) {
		log_pedantic("An error occurred while trying to complete encryption process. { error = %s }",
			ssl_error_string(MEMORYBUF(256), 256));
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		st_cleanup(result);
		return NULL;
	}

	written += available;
	available = payload_len - written;

	// Retrieve the tag, and then XOR it with the shard value extracted from the key.
	if (EVP_CIPHER_CTX_ctrl_d(&ctx, EVP_CTRL_GCM_GET_TAG, AES_TAG_LEN, &tag[0]) != 1 ||
		st_xor(&tag_key_shard, PLACER(&tag[0], AES_TAG_LEN), tag_shard) != tag_shard) {

		log_pedantic("An error occurred while trying to retrieve the tag value. { error = %s }",
			ssl_error_string(MEMORYBUF(256), 256));
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		st_cleanup(result);
		return NULL;
	}

	EVP_CIPHER_CTX_cleanup_d(&ctx);

	mm_copy(&(header->tag), st_data_get(tag_shard), AES_TAG_LEN);

	if (st_valid_tracked(st_opt_get(output))) {
		st_length_set(output, written + PRIME_OBJECT_HEAD_LEN);
	}

	return output;
}

stringer_t * aes_object_decrypt(stringer_t *key, stringer_t *object, stringer_t *output) {
	return NULL;
}

