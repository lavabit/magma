
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

#define PRIME_CHUNK_PREFIX_LEN 4            // 1 + 3 = 4
#define PRIME_CHUNK_KEY_LEN 64              // 16 + 16 + 32 = 64
#define PRIME_CHUNK_HEAD_LEN 36             // 1 + 3 + 16 + 16 = 36

#define PRIME_OBJECT_PREFIX_LEN 6           // 2 + 4 = 6
#define PRIME_OBJECT_KEY_LEN 64             // 16 + 16 + 32 = 64
#define PRIME_OBJECT_HEAD_LEN 38            // 2 + 4 + 16 + 16 = 38
#define PRIME_OBJECT_PAYLOAD_PREFIX_LEN 4   // 1 + 3 = 4

// The layout for an encrypted PRIME object.
typedef struct __attribute__ ((packed)) {

	uint16_t type;			// 2b  / Big endian type number.
	uint32_t size;			// 4b  / Big endian object size.
	uchr_t vector[16];		// 16b / Vector shard.
	uchr_t tag[16];			// 16b / Tag shard.
							// ~~~ / Encrypted payload.

} prime_encrypted_object_header_t;

// The layout for an encrypted PRIME chunk.
typedef struct __attribute__ ((packed)) {

	uint8_t type;			// 1b  / Type number.
	uint8_t size[3];		// 3b  / Big endian object size.
	uchr_t vector[16];		// 16b / Vector shard.
	uchr_t tag[16];			// 16b / Tag shard.
							// ~~~ / Encrypted payload.

} prime_encrypted_chunk_header_t;

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

/**
 * @brief	Create an encrypted chunk. The result includes the 1 byte type, the 3 byte big endian length, the 16 byte tag and the 16 byte vector,
 * 			followed by the encrypted data. Note the caller must pad the data buffer to a 16 byte boundary.
 */
stringer_t * aes_chunk_encrypt(uint8_t type, stringer_t *key, stringer_t *chunk, stringer_t *output) {

	EVP_CIPHER_CTX ctx;
	size_t overall_size = 0;
	uchr_t *chunk_data, tag[AES_TAG_LEN];
	uint32_t big_endian_size = 0, chunk_size = 0;
	prime_encrypted_chunk_header_t *header = NULL;
	int_t payload_len = 0, available = 0, written = 0;
	placer_t cipher_key = pl_null(), tag_key_shard = pl_null(), vector_key_shard = pl_null();
	stringer_t *vector = MANAGEDBUF(AES_VECTOR_LEN), *vector_rand_shard = MANAGEDBUF(AES_VECTOR_LEN),
		*tag_shard = MANAGEDBUF(AES_TAG_LEN), *result = NULL;

	if (st_length_get(key) != PRIME_CHUNK_KEY_LEN) {
		log_pedantic("PRIME chunk encryption requires a %i byte key. { length = %zu }", PRIME_CHUNK_KEY_LEN, st_length_get(key));
		return NULL;
	}
	else if (st_empty_out(chunk, &chunk_data, &overall_size) || (overall_size % AES_BLOCK_LEN) != 0) {
		log_pedantic("PRIME chunk encryption requires a plain text payload aligned to a block size of %i. { length = %zu }", AES_BLOCK_LEN, overall_size);
		return NULL;
	}
	else if (overall_size < 80) {
		log_pedantic("The smallest valid encrypted PRIME chunk is 80 bytes. { length = %zu }", overall_size);
		return NULL;
	}
	// Extract the key fragments.
	cipher_key = aes_cipher_key(key);
	tag_key_shard = aes_tag_shard(key);
	vector_key_shard = aes_vector_shard(key);

	// Check to ensure all of the fragments were extracted properly.
	if (pl_empty(cipher_key) || pl_empty(tag_key_shard) || pl_empty(vector_key_shard)) {
		log_pedantic("PRIME chunk encryption failed because an invalid key was provided.");
		return NULL;
	}

	// Generate a random initialization vector shard which will be combined with the vector key.
	else if (rand_write(vector_rand_shard) != AES_VECTOR_LEN || !st_xor(&vector_key_shard, vector_rand_shard, vector)) {
		log_pedantic("PRIME chunk encryption failed because a random vector could not be generated.");
		return NULL;
	}

	// Calculate the overall length.
	available = payload_len = chunk_size;

	// Generate the big endian representation of the numbers being serialized.
	big_endian_size = htobe32(AES_TAG_LEN + AES_VECTOR_LEN + payload_len);

	// See if we have a valid output buffer, and make sure it is large enough to hold the result.
	if (output && (!st_valid_destination(st_opt_get(output)) || st_avail_get(output) < (PRIME_CHUNK_HEAD_LEN + payload_len))) {
		log_pedantic("An output string was supplied but it does not represent a buffer capable of holding the output.");
		return NULL;
	}
	// If the output buffer is NULL, then we'll allocate a buffer for the result.
	else if (!output && !(output = result = st_alloc(PRIME_CHUNK_HEAD_LEN + payload_len))) {
		log_pedantic("Could not allocate a buffer large enough to hold encrypted result. { requested = %i }", PRIME_CHUNK_HEAD_LEN + payload_len);
		return NULL;
	}

	// Wipe the buffer.
	st_wipe(output);

	// Write the chunk header (everything except the tag).
	header = st_data_get(output);
	header->type = type;

	// Setup the prefix for later.
	mm_copy(&(header->size), ((uchr_t *)&big_endian_size) + 1, 3);
	mm_copy(&(header->vector), st_data_get(vector_rand_shard), 16);

	// Setup the cipher context.
	EVP_CIPHER_CTX_init_d(&ctx);

	// Initialize the cipher context.
	if (EVP_EncryptInit_ex_d(&ctx, EVP_aes_256_gcm_d(), NULL, NULL, NULL) != 1) {
		log_pedantic("An error occurred while trying to initialize chosen symmetric cipher. { error = %s }",
			ssl_error_string(MEMORYBUF(256), 256));
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		st_cleanup(result);
		return NULL;
	}

	// Set the vector length.
	else if (EVP_CIPHER_CTX_ctrl_d(&ctx, EVP_CTRL_GCM_SET_IVLEN, AES_VECTOR_LEN, NULL) != 1) {
		log_pedantic("The initialization vector length could not be properly set to %i bytes. { error = %s }", AES_VECTOR_LEN,
			ssl_error_string(MEMORYBUF(256), 256));
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
	else if (EVP_EncryptInit_ex_d(&ctx, NULL, NULL, pl_data_get(cipher_key), st_data_get(vector)) != 1) {
		log_pedantic("An error occurred initializing the symmetric cipher with the provided vector and key. { error = %s }",
			ssl_error_string(MEMORYBUF(256), 256));
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		st_cleanup(result);
		return NULL;
	}

	// Encrypt the payload.
	if (EVP_EncryptUpdate_d(&ctx, st_data_get(output) + PRIME_CHUNK_HEAD_LEN, &available, chunk_data, overall_size) != 1 ||
		available != overall_size) {

		log_pedantic("An error occurred while trying to encrypt the input buffer using the chosen symmetric cipher. { error = %s }",
			ssl_error_string(MEMORYBUF(256), 256));
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		st_cleanup(result);
		return NULL;
	}

	written += available;
	available = payload_len - written;

	// Calling the finalization routine is what generates the tamper tag, but we'll need to make a separate call to retrieve it.
	if (EVP_EncryptFinal_ex_d(&ctx, st_data_get(output) + PRIME_CHUNK_HEAD_LEN + written, &available) != 1) {
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
		st_length_set(output, written + PRIME_CHUNK_HEAD_LEN);
	}

	return output;
}

stringer_t * aes_chunk_decrypt(stringer_t *key, stringer_t *chunk, stringer_t *output) {

	size_t chunk_size;
	EVP_CIPHER_CTX ctx;
	uchr_t *chunk_data;
	prime_encrypted_chunk_header_t *header = NULL;
	int_t payload_len = 0, available = 0, written = 0;
	uint32_t big_endian_size = 0, size = 0;
	placer_t cipher_key = pl_null(), tag_key_shard = pl_null(), vector_key_shard = pl_null();
	stringer_t *vector = NULL, *tag = NULL, *result = NULL;

	if (st_length_get(key) != PRIME_CHUNK_KEY_LEN) {
		log_pedantic("PRIME chunk decryption requires a %i byte key. { length = %zu }", PRIME_CHUNK_KEY_LEN, st_length_get(key));
		return NULL;
	}
	// Setup the pointers and then ensure we have at least enough bytes for the object header which is processed below.
	else if (st_empty_out(chunk, &chunk_data, &chunk_size) || chunk_size < PRIME_CHUNK_HEAD_LEN) {
		log_pedantic("PRIME chunk decryption requires a valid encrypted payload. { length = %zu }", chunk_size);
		return NULL;
	}

	// Extract the key fragments.
	cipher_key = aes_cipher_key(key);
	tag_key_shard = aes_tag_shard(key);
	vector_key_shard = aes_vector_shard(key);

	// Check to ensure all of the fragments were extracted properly.
	if (pl_empty(cipher_key) || pl_empty(tag_key_shard) || pl_empty(vector_key_shard)) {
		log_pedantic("PRIME chunk decryption failed because an invalid key was provided.");
		return NULL;
	}

	// Process the encrypted object header.
	header = (prime_encrypted_chunk_header_t *)chunk_data;

	// Convert the big endian chunk length to host form.
	mm_copy(((uchr_t *)&big_endian_size) + 1, header->size, 3);
	size = be32toh(big_endian_size);

	// Check that the object prefix size matches the length of encrypted object passed in (minus the prefix length).
	if (chunk_size != size + PRIME_CHUNK_PREFIX_LEN) {
		log_pedantic("PRIME chunk decryption failed because the header length doesn't match the provided chunk. { header = %u / actual = %zu }",
			size + PRIME_CHUNK_PREFIX_LEN, chunk_size);
		return NULL;
	}

	payload_len = available = (chunk_size - PRIME_CHUNK_HEAD_LEN);

	// Compute the vector and tag values by combining the key shards, with the header shards.
	vector = st_xor(PLACER(&(header->vector[0]), AES_VECTOR_LEN), &vector_key_shard, MANAGEDBUF(AES_VECTOR_LEN));
	tag = st_xor(PLACER(&(header->tag[0]), AES_TAG_LEN), &tag_key_shard, MANAGEDBUF(AES_TAG_LEN));

	// Setup the cipher context, the body length, and store a pointer to the body buffer location.
	EVP_CIPHER_CTX_init_d(&ctx);

	// Initialize the cipher context.
	if (EVP_DecryptInit_ex_d(&ctx, EVP_aes_256_gcm_d(), NULL, NULL, NULL) != 1) {
		log_pedantic("An error occurred while trying to initialize chosen symmetric cipher. { error = %s }",
			ssl_error_string(MEMORYBUF(256), 256));
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		return NULL;
	}

	else if (EVP_CIPHER_CTX_ctrl_d(&ctx, EVP_CTRL_GCM_SET_IVLEN, AES_VECTOR_LEN, NULL) != 1) {
		log_pedantic("The initialization vector length could not be properly set to 16 bytes. { error = %s }",
			ssl_error_string(MEMORYBUF(256), 256));
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		return NULL;
	}

	// Validate the key length.
	else if (EVP_CIPHER_CTX_key_length_d(&ctx) != AES_KEY_LEN) {
		log_pedantic("The encryption key length does not match the selected cipher key length. { key = %i / expected = %i }",
			AES_KEY_LEN, EVP_CIPHER_CTX_key_length_d(&ctx));
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		return NULL;
	}

	// Add the vector and key data.
	else if (EVP_DecryptInit_ex_d(&ctx, NULL, NULL, pl_data_get(cipher_key), st_data_get(vector)) != 1) {
		log_pedantic("An error occurred initializing the symmetric cipher with the provided key and vector data. { error = %s }",
			ssl_error_string(MEMORYBUF(256), 256));
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		return NULL;
	}

	// See if we have a valid output buffer, and make sure it is large enough to hold the result.
	else if (output && (!st_valid_destination(st_opt_get(output)) || st_avail_get(output) < payload_len)) {
		log_pedantic("An output string was supplied but it does not represent a buffer capable of holding the output.");
		return NULL;
	}
	// If the output buffer is NULL, then we'll allocate a buffer for the result.
	else if (!output && !(output = result = st_alloc(payload_len))) {
		log_pedantic("Could not allocate a buffer large enough to hold decrypted result. { requested = %i }",
			payload_len);
		return NULL;
	}

	// Wipe the buffer.
	st_wipe(output);

	if (EVP_DecryptUpdate_d(&ctx, st_data_get(output), &available, chunk_data + PRIME_CHUNK_HEAD_LEN, chunk_size - PRIME_CHUNK_HEAD_LEN) != 1) {
		log_pedantic("An error occurred while trying to decrypt the input buffer using the chosen symmetric cipher. { error = %s }",
			ssl_error_string(MEMORYBUF(256), 256));
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		st_cleanup(result);
		return NULL;
	}

	written += available;
	available = payload_len - written;

	// Set the Galois verification tag, which provides protection against tampering by an attacker.
	if (EVP_CIPHER_CTX_ctrl_d(&ctx, EVP_CTRL_GCM_SET_TAG, 16, st_data_get(tag)) != 1) {
		log_pedantic("An error occurred while trying to set the decryption tag value. { error = %s }",
			ssl_error_string(MEMORYBUF(256), 256));
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		st_cleanup(result);
		return NULL;
	}

	if (EVP_DecryptFinal_ex_d(&ctx, st_data_get(output) + written, &available) != 1) {
		log_pedantic("An error occurred while trying to complete decryption process. { error = %s}",
			ssl_error_string(MEMORYBUF(256), 256));
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		st_cleanup(result);
		return NULL;
	}

	written += available;
	available = payload_len - written;

	EVP_CIPHER_CTX_cleanup_d(&ctx);

	// Update the output string length.
	if (st_valid_tracked(st_opt_get(output))) {
		st_length_set(output, written);
	}

	return output;
}

stringer_t * aes_artifact_encrypt(stringer_t *key, stringer_t *object, stringer_t *output) {

	uint8_t pad = 0;
	EVP_CIPHER_CTX ctx;
	size_t overall_size = 0;
	uchr_t *object_data, tag[AES_TAG_LEN];
	uint16_t big_endian_type = 0, type = 0;
	prime_encrypted_object_header_t *header = NULL;
	int_t payload_len = 0, available = 0, written = 0;
	uint32_t big_endian_size = 0, big_endian_object_size = 0, object_size = 0, prefix = 0;
	placer_t cipher_key = pl_null(), tag_key_shard = pl_null(), vector_key_shard = pl_null();
	stringer_t *vector = MANAGEDBUF(AES_VECTOR_LEN), *vector_rand_shard = MANAGEDBUF(AES_VECTOR_LEN),
		*tag_shard = MANAGEDBUF(AES_TAG_LEN), *result = NULL;

	if (st_length_get(key) != PRIME_OBJECT_KEY_LEN) {
		log_pedantic("PRIME object encryption requires a %i byte key. { length = %zu }", PRIME_OBJECT_KEY_LEN, st_length_get(key));
		return NULL;
	}
	else if (st_empty_out(object, &object_data, &overall_size)) {
		log_pedantic("PRIME object encryption requires a plain text payload to protect. { length = %zu }", overall_size);
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

	// Get the type and confirm the header size matches the size of the object provided for encryption.
	else if (prime_header_read(object, &type, &object_size) || (object_size + 5) != overall_size || !object_size) {
		log_pedantic("PRIME object encryption failed because an invalid object was supplied.");
		return NULL;
	}
	// Convert the plain object type into the encrypted object type, and store it in big endian form.
	else if (type == PRIME_ORG_KEY) {
		big_endian_type = htobe16(PRIME_ORG_KEY_ENCRYPTED);
	}
	else if (type == PRIME_USER_KEY) {
		big_endian_type = htobe16(PRIME_USER_KEY_ENCRYPTED);
	}
	else {
		log_pedantic("PRIME object encryption failed because an unrecognized object was supplied. { type = %hu }", type);
		return NULL;
	}

	// Skip the object header.
	object_data += 5;

	// Calculate the number of padding bytes required, and then the overall payload length.
	pad = AES_BLOCK_LEN - ((PRIME_OBJECT_PAYLOAD_PREFIX_LEN + object_size) % AES_BLOCK_LEN);
	available = payload_len = PRIME_OBJECT_PAYLOAD_PREFIX_LEN + object_size + pad;

	// Generate the big endian representation of the numbers being serialized.
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

	// Setup the prefix for later.
	mm_copy(&prefix, ((uchr_t *)&big_endian_object_size) + 1, 3);
	mm_copy(((uchr_t *)&prefix) + 3, &pad, 1);

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
	else if (EVP_EncryptInit_ex_d(&ctx, NULL, NULL, pl_data_get(cipher_key), st_data_get(vector)) != 1) {
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

stringer_t * aes_artifact_decrypt(stringer_t *key, stringer_t *object, stringer_t *output) {

	uint8_t pad = 0;
	uint16_t type = 0;
	size_t object_size;
	EVP_CIPHER_CTX ctx;
	uchr_t *object_data;
	prime_encrypted_object_header_t *header = NULL;
	int_t payload_len = 0, available = 0, written = 0;
	uint32_t big_endian_payload_size = 0, size = 0;
	placer_t cipher_key = pl_null(), tag_key_shard = pl_null(), vector_key_shard = pl_null();
	stringer_t *vector = NULL, *tag = NULL, *result = NULL;

	if (st_length_get(key) != PRIME_OBJECT_KEY_LEN) {
		log_pedantic("PRIME object decryption requires a %i byte key. { length = %zu }", PRIME_OBJECT_KEY_LEN, st_length_get(key));
		return NULL;
	}
	// Setup the pointers and then ensure we have at least enough bytes for the object header which is processed below.
	else if (st_empty_out(object, &object_data, &object_size) || object_size < PRIME_OBJECT_HEAD_LEN) {
		log_pedantic("PRIME object decryption requires a valid encrypted payload. { length = %zu }", object_size);
		return NULL;
	}

	// Extract the key fragments.
	cipher_key = aes_cipher_key(key);
	tag_key_shard = aes_tag_shard(key);
	vector_key_shard = aes_vector_shard(key);

	// Check to ensure all of the fragments were extracted properly.
	if (pl_empty(cipher_key) || pl_empty(tag_key_shard) || pl_empty(vector_key_shard)) {
		log_pedantic("PRIME object decryption failed because an invalid key was provided.");
		return NULL;
	}

	// Process the encrypted object header.
	header = (prime_encrypted_object_header_t *)object_data;
	size = be32toh(header->size);

	// Check that the object prefix size matches the length of encrypted object passed in (minus the prefix length).
	if (object_size != size + PRIME_OBJECT_PREFIX_LEN) {
		log_pedantic("PRIME object decryption failed because the header length doesn't match the provided object. { header = %u / actual = %zu }",
			size + PRIME_OBJECT_PREFIX_LEN, object_size);
		return NULL;
	}

	payload_len = available = (object_size - PRIME_OBJECT_HEAD_LEN);

	// Compute the vector and tag values by combining the key shards, with the header shards.
	vector = st_xor(PLACER(&(header->vector[0]), AES_VECTOR_LEN), &vector_key_shard, MANAGEDBUF(AES_VECTOR_LEN));
	tag = st_xor(PLACER(&(header->tag[0]), AES_TAG_LEN), &tag_key_shard, MANAGEDBUF(AES_TAG_LEN));

	// Map the encrypted object type to the decrypted object type.
	if (be16toh(header->type) == PRIME_ORG_KEY_ENCRYPTED) {
		type = PRIME_ORG_KEY;
	}
	else if (be16toh(header->type) == PRIME_USER_KEY_ENCRYPTED) {
		type = PRIME_USER_KEY;
	}
	else {
		log_pedantic("PRIME object decryption failed because an unrecognized object was provided.");
		return NULL;
	}

	// Setup the cipher context, the body length, and store a pointer to the body buffer location.
	EVP_CIPHER_CTX_init_d(&ctx);

	// Initialize the cipher context.
	if (EVP_DecryptInit_ex_d(&ctx, EVP_aes_256_gcm_d(), NULL, NULL, NULL) != 1) {
		log_pedantic("An error occurred while trying to initialize chosen symmetric cipher.");
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		return NULL;
	}

	else if (EVP_CIPHER_CTX_ctrl_d(&ctx, EVP_CTRL_GCM_SET_IVLEN, AES_VECTOR_LEN, NULL) != 1) {
		log_pedantic("The initialization vector length could not be properly set to 16 bytes.");
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		return NULL;
	}

	// Validate the key length.
	else if (EVP_CIPHER_CTX_key_length_d(&ctx) != AES_KEY_LEN) {
		log_pedantic("The encryption key length does not match the selected cipher key length. { key = %i / expected = %i }",
			AES_KEY_LEN, EVP_CIPHER_CTX_key_length_d(&ctx));
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		return NULL;
	}

	// Add the vector and key data.
	else if (EVP_DecryptInit_ex_d(&ctx, NULL, NULL, pl_data_get(cipher_key), st_data_get(vector)) != 1) {
		log_pedantic("An error occurred initializing the symmetric cipher with the provided key and vector data.");
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		return NULL;
	}

	// See if we have a valid output buffer, and make sure it is large enough to hold the result.
	else if (output && (!st_valid_destination(st_opt_get(output)) || st_avail_get(output) < (payload_len + 1))) {
		log_pedantic("An output string was supplied but it does not represent a buffer capable of holding the output.");
		return NULL;
	}
	// If the output buffer is NULL, then we'll allocate a buffer for the result.
	else if (!output && !(output = result = st_alloc(payload_len + 1))) {
		log_pedantic("Could not allocate a buffer large enough to hold decrypted result. { requested = %i }",
			payload_len + 1);
		return NULL;
	}

	// Wipe the buffer.
	st_wipe(output);

	if (EVP_DecryptUpdate_d(&ctx, st_data_get(output) + 1, &available, object_data + PRIME_OBJECT_HEAD_LEN, object_size - PRIME_OBJECT_HEAD_LEN) != 1) {
		log_pedantic("An error occurred while trying to decrypt the input buffer using the chosen symmetric cipher.");
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		st_cleanup(result);
		return NULL;
	}

	written += available;
	available = payload_len - written;

	// Set the Galois verification tag, which provides protection against tampering by an attacker.
	if (EVP_CIPHER_CTX_ctrl_d(&ctx, EVP_CTRL_GCM_SET_TAG, 16, st_data_get(tag)) != 1) {
		log_pedantic("An error occurred while trying to set the decryption tag value.");
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		st_cleanup(result);
		return NULL;
	}

	if (EVP_DecryptFinal_ex_d(&ctx, st_data_get(output) + 1 + written, &available) != 1) {
		log_pedantic("An error occurred while trying to complete decryption process. {%s}", ssl_error_string(MEMORYBUF(256), 256));
		EVP_CIPHER_CTX_cleanup_d(&ctx);
		st_cleanup(result);
		return NULL;
	}

	written += available;
	available = payload_len - written;

	EVP_CIPHER_CTX_cleanup_d(&ctx);

	// Parse the payload prefix.
	mm_move(((uchr_t *)&big_endian_payload_size) + 1, st_data_get(output) + 1, 3);
	mm_move((uchr_t *)&pad, st_data_get(output) + 1 + 3, 1);

	// Confirm the size of the payload prefix, plain text object and padding bytes equals the number of bytes actually decrypted.
	if (be32toh(big_endian_payload_size) + pad + PRIME_OBJECT_PAYLOAD_PREFIX_LEN != written) {
		log_pedantic("The payload prefix didn't properly describe the decrypted buffer. { size + pad + prefix != actual / %u + %hhu + %i != %i }",
			be32toh(big_endian_payload_size), pad, PRIME_OBJECT_PAYLOAD_PREFIX_LEN, written);
		st_cleanup(result);
		return NULL;
	}

	// Ensure the combination of the prefix, payload, and padding bytes is evenly divisible by the block size.
	if (((be32toh(big_endian_payload_size) + pad + PRIME_OBJECT_PAYLOAD_PREFIX_LEN) % AES_BLOCK_LEN) != 0) {
		log_pedantic("The encrypted payload wasn't evenly divisible by the cipher block length.");
		st_cleanup(result);
		return NULL;
	}

	// Setup a pointer to the padding bytes.
	object_data = st_data_get(output) + 1 + be32toh(big_endian_payload_size) + PRIME_OBJECT_PAYLOAD_PREFIX_LEN;

	// Loop through and verify that each padding byte value is the number of padding bytes present.
	for (int_t i = 0; i < pad; i++) {
		if (*object_data++ != pad) {
			log_pedantic("An invalid padding byte was found inside the decrypted buffer.");
			st_cleanup(result);
			return NULL;
		}
	}

	// Write the decrypted object type and length into the new object heading.
	mm_copy(st_data_get(output), st_data_get(prime_header_write(type, be32toh(big_endian_payload_size), MANAGEDBUF(5))), 5);

	// Update the output string length.
	if (st_valid_tracked(st_opt_get(output))) {
		st_length_set(output, be32toh(big_endian_payload_size) + 5);
	}

	return output;
}

