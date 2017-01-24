
/**
 * @file /magma/providers/cryptography/scramble.c
 *
 * @brief	Functions used to handle symmetric encryption.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"
#include "deprecated.h"

/// TODO: Create a scramble_export() function, and/or alter the import interfaces to make using managed strings easier. ie. the import/export
/// 		functions could provide wrappers around dsecrypt/encrypt which return strings instead of scramble objects.
/// HIGH: Alter the interfaces here to allow passing in the output buffer, or options which allow us to store the decrypted data in secure memory.

/**
 * @brief	Get the total length of a scrambled object.
 * @param	buffer	a pointer to the header of the scrambled data.
 * @return	the total length, in bytes, of the scrambled object.
 */
uint64_t scramble_total_length(scramble_t *buffer) {
	return scramble_vector_length(buffer) + scramble_body_length(buffer) + sizeof(scramble_head_t);
}

/*uint64_t scramble_orig_hash(scramble_t *buffer) {
	scramble_head_t *head = (scramble_head_t *)buffer;
	return head->hash.original;
}*/

/**
 * @brief	Get a hash of a scrambled object's (encrypted) body data.
 * @param	buffer	a pointer to the header of the scrambled data.
 * @return	the 64-bit bash of the specified scrambled object's body data.
 */
uint64_t scramble_body_hash(scramble_t *buffer) {

	scramble_head_t *head = (scramble_head_t *)buffer;

	return head->hash.scrambled;
}

/**
 * @brief	Get the length of the original (unencrypted) data underlying a scrambled object.
 * @param	buffer	a pointer to the header of the scrambled data.
 * @return	the length, in bytes, of the scrambled object's original data.
 */
uint64_t scramble_orig_length(scramble_t *buffer) {

	scramble_head_t *head = (scramble_head_t *)buffer;

	return head->length.original;
}

/**
 * @brief	Get the length of a scrambled object's (encrypted) body data.
 * @param	buffer	a pointer to the header of the scrambled data.
 * @return	the length, in bytes, of the scrambled object's body data.
 */
uint64_t scramble_body_length(scramble_t *buffer) {

	scramble_head_t *head = (scramble_head_t *)buffer;

	return head->length.scrambled;
}

/**
 * @brief	Get the length of a scrambled object's IV.
 * @param	buffer	a pointer to the header of the scrambled data.
 * @return	the length, in bytes, of the scrambled object's IV.
 */
uint64_t scramble_vector_length(scramble_t *buffer) {

	scramble_head_t *head = (scramble_head_t *)buffer;

	return head->length.vector;
}

/**
 * @brief	Get a pointer to the start of the encrypted data from a scrambled data header.
 * @param	buffer	a pointer to the header of the scrambled data.
 * @return	a pointer to the start of the scrambled data header's associated encrypted data body.
 */
void * scramble_body_data(scramble_t *buffer) {

	return buffer + sizeof(scramble_head_t) + scramble_vector_length(buffer);
}

/**
 * @brief	Get a pointer to the start of the IV from a scrambled data header.
 * @param	buffer	a pointer to the header of the scrambled data.
 * @return	a pointer to the start of the scrambled data header's associated IV block.
 */
void * scramble_vector_data(scramble_t *buffer) {

	return buffer + sizeof(scramble_head_t);
}

/**
 * @brief	Return a managed string as a scrambled buffer, after validation.
 * @note	The returned pointer is inside the existing stringer, so it doesn't need be freed.
 * @param	s	a managed string containing the serialized scrambled data.
 * @return	NULL on failure, or a pointer to the scrambled data header on success.
 */
scramble_t * scramble_import(stringer_t *s) {

	void *bptr;
	scramble_t *buffer;
	uint64_t blen, vlen;
	scramble_head_t *head;
#ifdef MAGMA_PEDANTIC
	uint32_t hash = 0;
#endif

	if (st_empty(s) || st_length_get(s) < sizeof(scramble_t)) {
		log_pedantic("The provided string is not large enough to hold scrambled data. {s = %zu / min = %zu}", st_empty(s) ? 0 : st_length_get(s), sizeof(scramble_t));
		return NULL;
	}
	else if (!(buffer = head = st_data_get(s)) || !(bptr = scramble_body_data(buffer))) {
		log_pedantic("The string appears to be corrupted because NULL data pointers were generated.");
		return NULL;
	}
	else if (!(blen = head->length.scrambled) || !(vlen = head->length.vector) || st_length_get(s) != (blen + vlen + sizeof(scramble_head_t))) {
		log_pedantic("The provided string length doesn't contain the amount of data indicated by the scramble header. {length = %zu / expected = %zu + %lu + %lu}",
				st_length_get(s), sizeof(scramble_head_t), vlen, blen);
		return NULL;
	}

#ifdef MAGMA_PEDANTIC
	// This step should be unnecessary in production since all of the descrambling functions should be validating the scrambled data buffer hashes as well.
	else if (head->hash.scrambled != (hash = hash_adler32(bptr, blen))) {
		log_pedantic("The scrambled data buffer appears to be corrupted. {hash = %u / expected = %u}", hash, head->hash.scrambled);
		return NULL;
	}
#endif

	return buffer;
}

/**
 * @brief	Allocate a new scrambled data block.
 * @param	length	the length, in bytes, of the scrambled data buffer (should include the IV and encrypted body length).
 * @return	a pointer to a newly allocated scrambled data header.
 */
scramble_t * scramble_alloc(size_t length) {

	return mm_alloc(sizeof(scramble_head_t) + length);
}

/**
 * @brief	Free a scrambled data block.
 * @param	buffer	a pointer to the scrambled data header to be freed.
 * @return	This function returns no value.
 */
void scramble_free(scramble_t *buffer) {

	mm_free(buffer);

	return;
}

/**
 * @brief	Performed a checked free of a scramble buffer.
 * @see		scramble_free
 * @param	block	the scramble buffer to be freed.
 * @return	This function returns no value.
 */
void scramble_cleanup(scramble_t *buffer) {

	if (buffer) {
		mm_free(buffer);
	}

	return;
}

/**
 * @brief	Scramble a block of data using an encryption key.
 * @note	This function is configured to use AES 256 in CBC mode.
 * @param	key		a managed string containing the symmetric encryption key.
 * @param	input	a managed string containing the data to be encrypted.
 * @return	NULL on failure, or a pointer to a scrambled data header containing the encrypted data and its metadata.
 */
scramble_t * scramble_encrypt(stringer_t *key, stringer_t *input) {

	size_t len;
	scramble_t *output;
	scramble_head_t *head;
	cipher_t *cipher = cipher_id(NID_aes_256_cbc);
	stringer_t *encrypted, *vector = MANAGEDBUF(64), *digest = MANAGEDBUF(64);

	if (!cipher) {
		log_pedantic("An error occurred retrieving NID for scramble routine.");
		return NULL;
	}

	if (!symmetric_vector(cipher, vector)) {
		log_pedantic("An error occurred while trying to generate the initialization vector.");
		return NULL;
	}
	else if (!symmetric_key(cipher, key, digest)) {
		log_pedantic("An error occurred while trying to digest key.");
		return NULL;
	}
	else if (!(encrypted = symmetric_encrypt(cipher, vector, digest, input))) {
		log_pedantic("An error occurred while trying to encrypt the data.");
		return NULL;
	}

	// We need a buffer large enough to hold the encrypted data plus the initialization vector.
	else if (!(len = st_length_get(vector) + st_length_get(encrypted)) || !(head = (scramble_head_t *)(output = scramble_alloc(len)))) {
		log_pedantic("Unable to allocate a buffer to hold the scramble output.");
		st_free(encrypted);
		return NULL;
	}

	// Setup the header.
	head->engine = cipher_numeric_id(cipher);

	head->length.vector = st_length_get(vector);
	head->length.original = st_length_get(input);
	head->length.scrambled = st_length_get(encrypted);

	//head->hash.original = hash_adler32(st_data_get(input), st_length_get(input));
	head->hash.scrambled = hash_adler32(st_data_get(encrypted), st_length_get(encrypted));

	// Copy the vector and ciphered data into the result buffer and then free the original encrypted buffer.
	mm_copy(scramble_vector_data(output), st_data_get(vector), st_length_get(vector));
	mm_copy(scramble_body_data(output), st_data_get(encrypted), st_length_get(encrypted));
	st_free(encrypted);

	return output;
}

/**
 * @brief	Un-scramble a block of data using an decryption key.
 * @param	key		a managed string containing the symmetric decryption key.
 * @param	input	a pointer to the scrambled data header to be decrypted.
 * @return	NULL on failure, or a managed string containing the verified decrypted data.
 */
stringer_t * scramble_decrypt(stringer_t *key, scramble_t *input) {

	uint32_t hash;
	cipher_t *cipher;
	scramble_head_t *head;
	stringer_t *result, *digest = MANAGEDBUF(64);

	if (!key || !input || !(head = (scramble_head_t *)input) || head->engine == NID_undef || !(cipher = cipher_id(head->engine))) {
		log_pedantic("Invalid parameters provided.");
		return NULL;
	}
	else if (!symmetric_key(cipher, key, digest)) {
		log_pedantic("An error occurred while trying to digest key.");
		return NULL;
	}
	else if ((head->hash.scrambled != (hash = hash_adler32(scramble_body_data(input), scramble_body_length(input))))) {
		log_info("The encrypted data is corrupt. {hash = %u != %u}", head->hash.scrambled, hash);
		return NULL;
	}
	else if (!(result = symmetric_decrypt(cipher, PLACER(scramble_vector_data(input), scramble_vector_length(input)), digest,
		PLACER(scramble_body_data(input), scramble_body_length(input))))) {
		log_pedantic("An error occurred while trying to decrypt the data.");
		return NULL;
	}
	//else if (head->length.original != st_length_get(result) || head->hash.original != (hash = hash_adler32(st_data_get(result), st_length_get(result)))) {
	else if (head->length.original != st_length_get(result)) {
		log_info("The decrypted data is corrupt. {input = %lu != %lu}", head->length.original, st_length_get(result));
		st_free(result);
		return NULL;
	}

	return result;
}
