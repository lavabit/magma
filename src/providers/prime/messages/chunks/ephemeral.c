
/**
 * @file /magma/src/providers/prime/messages/chunks.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

void ephemeral_chunk_free(prime_ephemeral_chunk_t *chunk) {

	if (chunk) {
		if (chunk->buffer) st_free(chunk->buffer);
		mm_free(chunk);
	}
#ifdef MAGMA_PEDANTIC
	else {
		log_pedantic("An invalid PRIME ephemeral chunk pointer was passed to the free function.");
	}
#endif

	return;
}

void ephemeral_chunk_cleanup(prime_ephemeral_chunk_t *chunk) {
	if (chunk) {
		ephemeral_chunk_free(chunk);
	}
	return;
}

prime_ephemeral_chunk_t * ephemeral_chunk_alloc(void) {

	prime_ephemeral_chunk_t *result = NULL;

	if (!(result = mm_alloc(sizeof(prime_ephemeral_chunk_t)))) {
		log_pedantic("PRIME ephemeral chunk allocation failed.");
		return NULL;
	}

	// We need wipe the buffer to ensure a clean slate.
	mm_wipe(result, sizeof(prime_ephemeral_chunk_t));

	return result;
}

/**
 * @brief	Generate an ephemeral message chunk. A public encryption key is required, while the public signing key is optional,
 * 			but will be included if a signing key is provided.
 */
prime_ephemeral_chunk_t * ephemeral_chunk_get(secp256k1_key_t *encryption, ed25519_key_t *signing) {

	prime_ephemeral_chunk_t *result = NULL;

	if (!encryption || (signing && (signing->type == ED25519_PUB || signing->type == ED25519_PRIV))) {
		log_pedantic("PRIME ephemeral chunk generation failed. Invalid keys provided.");
		return NULL;
	}
	else if (!(result = ephemeral_chunk_alloc())) {
		return NULL;
	}

	// Setup the header, and serialize the payload, then store it in the buffer.
	result->header.type = PRIME_CHUNK_EPHEMERAL;
	result->header.length = (!signing ? 35: 69);
	result->buffer = st_merge("sss", chunk_header_write(PRIME_CHUNK_EPHEMERAL, result->header.length, MANAGEDBUF(4)),
		prime_field_write(PRIME_USER_SIGNET, USER_SIGNING_KEY, ED25519_KEY_PUB_LEN, ed25519_public_get(signing, MANAGEDBUF(32)), MANAGEDBUF(34)),
		(!signing ? NULL : prime_field_write(PRIME_USER_SIGNET, USER_ENCRYPTION_KEY, SECP256K1_KEY_PUB_LEN, secp256k1_public_get(encryption, MANAGEDBUF(33)), MANAGEDBUF(35))));

	if (!result->buffer || (st_length_get(result->buffer) != 39 && st_length_get(result->buffer) != 73)) {
		ephemeral_chunk_free(result);
		return NULL;
	}

	if (!signing) {
		result->fields.encryption = pl_init(st_data_get(result->buffer) + 4, 35);
	}
	else {
		result->fields.signing = pl_init(st_data_get(result->buffer) + 4, 34);
		result->fields.encryption = pl_init(st_data_get(result->buffer) + 38, 35);
	}

	return result;
}

prime_ephemeral_chunk_t * ephemeral_chunk_set(secp256k1_key_t *encryption, ed25519_key_t *signing) {
#error setter not done

}

