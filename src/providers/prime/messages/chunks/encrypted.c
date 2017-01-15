
/**
 * @file /magma/src/providers/prime/messages/chunks/encrypted.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

void encrypted_chunk_free(prime_encrypted_chunk_t *chunk) {

	if (chunk) {

		if (chunk->signature) st_free(chunk->signature);
		if (chunk->data) st_free(chunk->data);
		if (chunk->trailing) st_free(chunk->trailing);
		if (chunk->encrypted) st_free(chunk->encrypted);

		if (chunk->slots.author) st_free(chunk->slots.author);
		if (chunk->slots.origin) st_free(chunk->slots.origin);
		if (chunk->slots.destination) st_free(chunk->slots.destination);
		if (chunk->slots.recipient) st_free(chunk->slots.recipient);

		mm_free(chunk);
	}
#ifdef MAGMA_PEDANTIC
	else {
		log_pedantic("An invalid PRIME encrypted chunk pointer was passed to the free function.");
	}
#endif

	return;
}

void encrypted_chunk_cleanup(prime_encrypted_chunk_t *chunk) {
	if (chunk) {
		encrypted_chunk_free(chunk);
	}
	return;
}

prime_encrypted_chunk_t * encrypted_chunk_alloc(void) {

	prime_encrypted_chunk_t *result = NULL;

	if (!(result = mm_alloc(sizeof(prime_encrypted_chunk_t)))) {
		log_pedantic("PRIME encrypted chunk allocation failed.");
		return NULL;
	}

	// We need wipe the buffer to ensure a clean slate.
	mm_wipe(result, sizeof(prime_encrypted_chunk_t));

	return result;
}

stringer_t * encrypted_chunk_buffer(prime_encrypted_chunk_t *chunk) {

	stringer_t *buffer = NULL;

	if (chunk) {
		buffer = chunk->encrypted;
	}

	return buffer;
}

/**
 * @brief	Generate an encrypted message chunk. The signing and encryption keys are required, along with the public encryption
 * 			key for at least one actor.
 */
prime_encrypted_chunk_t * encrypted_chunk_get(prime_message_chunk_type_t type, ed25519_key_t *signing, secp256k1_key_t *encryption,
	secp256k1_key_t *author, secp256k1_key_t *origin, secp256k1_key_t *destination, secp256k1_key_t *recipient, stringer_t *data) {

	uint32_t big_endian_length = 0;
	prime_encrypted_chunk_t *result = NULL;
	stringer_t *key = MANAGEDBUF(32), *stretched = MANAGEDBUF(64);

	// We need a signing key, encryption key, and at least one actor.
	if (!signing || signing->type != ED25519_PRIV || !encryption || !data || (!author && !origin && !destination && !recipient)) {
		log_pedantic("Invalid parameters passed to the encrypted chunk generator.");
		return NULL;
	}

	/// HIGH: Add support for payloads that span across multiple chunks.
	// The maximum chunk plain text data size is 16,777,099. The max is limited by the 3 byte length in the chunk header,
	// minus the 32 byte needed for the shards, and then accounting for the 69 required encryted bytes, while still resulting
	// in a total encrypted size that aligns to a 16 byte boundary.
	else if (st_length_get(data) < 1 || st_length_get(data) >= 16777099) {
		log_pedantic("The chunk payload data must be larger than 1 byte, but smaller than 16,777,099 bytes. { length = %zu }", st_length_get(data));
		return NULL;
	}
	else if (!(result = encrypted_chunk_alloc())) {
		return NULL;
	}

	// The entire buffer must be evenly divisible by 16. divisible by
	// 64 signature + 3 data length + 1 flags + 1 padding length = 69
	result->pad = ((st_length_get(data) + 69 + 16 - 1) & ~(16 - 1)) - (st_length_get(data) + 69);
	result->length = st_length_get(data);

	// The spec suggests we pad any payload smaller to 256 bytes, to make it a minimum of 256 bytes.
	if ((result->pad + result->length + 69) < 256) {
		result->pad += (256 - (result->pad + result->length + 69));
	}

	result->flags = 0;
	big_endian_length = htobe32(result->length);

	// Allocate a buffer for the serialized payload, and a buffer to store the padding bytes, then initialize the padding
	// bytes to match the padding amount.
	if (!(result->data = st_alloc(result->length + result->pad + 69)) || !(result->trailing = st_alloc(result->pad)) ||
		!(st_set(result->trailing, result->pad, result->pad))) {
		encrypted_chunk_free(result);
		return NULL;
	}

	// Copy the big endian length into the buffer.
	mm_copy(st_data_get(result->data) + 64, ((uchr_t *)&big_endian_length) + 1, 3);

	// Copy the flags into the buffer.
	mm_copy(st_data_get(result->data) + 67, ((uchr_t *)&result->flags), 1);

	// Copy in the padding length.
	mm_copy(st_data_get(result->data) + 68, ((uchr_t *)&result->pad), 1);

	// Copy in the payload.
	mm_copy(st_data_get(result->data) + 69, st_data_get(data), result->length);

	// Copy in the trailing bytes.
	mm_copy(st_data_get(result->data) + result->length + 69, st_data_get(result->trailing), result->pad);

	// Generate a signature using the serialized plain text buffer.
	ed25519_sign(signing, PLACER(st_data_get(result->data) + 64, result->length + result->pad + 5), MANAGED(st_data_get(result->data), 0, 64));

	// Set the length so the AES function knows how much data needs encrypting.
	st_length_set(result->data, result->length + result->pad + 69);

	// Create the chunk key, and then stretch the key.
	if (rand_write(key) != 32 || !(stretched = hash_sha512(key, stretched))) {
		encrypted_chunk_free(result);
		return NULL;
	}

	// Encrypt the buffer.
	else if (!(result->encrypted = aes_chunk_encrypt(type, stretched, result->data, NULL))) {
		encrypted_chunk_free(result);
		return NULL;
	}

	/// LOW: If we pass in the kek, instead of the secp256k1 key, we won't need to recompute the kek for every chunk.

	// Generate keyslots for any of the actors we recieved keys for, starting with the author.
	else if (author && (!(result->slots.author = secp256k1_compute_kek(encryption, author, NULL)) ||
		!(result->slots.author = st_xor(result->slots.author, key, result->slots.author)))) {
		encrypted_chunk_free(result);
		return NULL;
	}

	// The author keyslot is always required, so if an author key wasn't provided, set the author slot to all zeros.
	else if (!author && (!(result->slots.author = st_alloc(64)) || !st_set(result->slots.author, 0, 64))) {
		encrypted_chunk_free(result);
		return NULL;
	}

	// Origin keyslot.
	else if (origin && (!(result->slots.origin = secp256k1_compute_kek(encryption, origin, NULL)) ||
		!(result->slots.origin = st_xor(result->slots.origin, key, result->slots.origin)))) {
		encrypted_chunk_free(result);
		return NULL;
	}

	// Destination keyslot.
	else if (destination && (!(result->slots.destination = secp256k1_compute_kek(encryption, destination, NULL)) ||
		!(result->slots.destination = st_xor(result->slots.destination, key, result->slots.destination)))) {
		encrypted_chunk_free(result);
		return NULL;
	}

	// Recipient keyslot.
	else if (recipient && (!(result->slots.recipient = secp256k1_compute_kek(encryption, recipient, NULL)) ||
		!(result->slots.recipient = st_xor(result->slots.recipient, key, result->slots.recipient)))) {
		encrypted_chunk_free(result);
		return NULL;
	}

	// The recipient keyslot is always required, so if a recipient key wasn't provided, set the recipient slot to all zeros.
	else if (!recipient && (!(result->slots.recipient = st_alloc(64)) || !st_set(result->slots.recipient, 0, 64))) {
		encrypted_chunk_free(result);
		return NULL;
	}

	// Append the key slots onto the end of the encrypted chunk data.
	else if ((result->slots.author && st_append_out(128, &(result->encrypted), result->slots.author) < 0)||
		(result->slots.origin && st_append_out(128, &(result->encrypted), result->slots.origin) < 0) ||
		(result->slots.destination && st_append_out(128, &(result->encrypted), result->slots.destination) < 0) ||
		(result->slots.recipient && st_append_out(128, &(result->encrypted), result->slots.recipient) < 0)) {
		encrypted_chunk_free(result);
		return NULL;
	}

	return result;
}

prime_encrypted_chunk_t * encrypted_chunk_set(ed25519_key_t *signing, secp256k1_key_t *encryption, secp256k1_key_t *author,
	secp256k1_key_t *origin, secp256k1_key_t *destination, secp256k1_key_t *recipient, stringer_t *chunk) {

	prime_encrypted_chunk_t *result = NULL;

	// We need a signing key, encryption key, and at least one actor.
	if (!signing || signing->type != ED25519_PRIV || !encryption || !chunk || (!author && !origin && !destination && !recipient)) {
		log_pedantic("Invalid parameters passed to the encrypted chunk generator.");
		return NULL;
	}

	/// HIGH: Add support for payloads that span across multiple chunks.
	// The minimum legal chunk size would be 4 + 32 + 80 + 64 = 180, while the max would be 4 + 32 + 69 + 128 + 16,777,099 =
	// 16,777,332, which accounts for the chunk header, and keyslots, which might be included in the buffer, but not in the
	// chunk header length.
	else if (st_length_get(chunk) < 84 || st_length_get(chunk) > 16777332) {
		log_pedantic("The chunk payload data must be larger than 1 byte, but smaller than 16,777,332 bytes. { length = %zu }",
			st_length_get(chunk));
		return NULL;
	}
	else if (!(result = encrypted_chunk_alloc())) {
		return NULL;
	}
	return result;
}

