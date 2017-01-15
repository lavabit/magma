
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
		if (chunk->keys.signing) ed25519_free(chunk->keys.signing);
		if (chunk->keys.encryption) secp256k1_free(chunk->keys.encryption);
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

stringer_t * ephemeral_chunk_buffer(prime_ephemeral_chunk_t *chunk) {

	stringer_t *buffer = NULL;

	if (chunk) {
		buffer = chunk->buffer;
	}

	return buffer;
}

/**
 * @brief	Generate an ephemeral message chunk. A public encryption key is required, while the public signing key is optional,
 * 			but will be included if a signing key is provided.
 */
prime_ephemeral_chunk_t * ephemeral_chunk_get(ed25519_key_t *signing, secp256k1_key_t *encryption) {

	prime_ephemeral_chunk_t *result = NULL;

	if (!encryption || (signing && signing->type != ED25519_PUB && signing->type != ED25519_PRIV)) {
		log_pedantic("PRIME ephemeral chunk generation failed. Invalid keys provided.");
		return NULL;
	}
	else if (!(result = ephemeral_chunk_alloc())) {
		return NULL;
	}

	// Setup the header, and serialize the payload, then store it in the buffer.
	result->header.type = PRIME_CHUNK_EPHEMERAL;
	result->header.length = (!signing ? 35: 69);

	if (!signing) {
		result->buffer = st_merge("ss", chunk_header_write(PRIME_CHUNK_EPHEMERAL, result->header.length, MANAGEDBUF(4)),
			prime_field_write(PRIME_USER_SIGNET, USER_ENCRYPTION_KEY, SECP256K1_KEY_PUB_LEN, secp256k1_public_get(encryption, MANAGEDBUF(33)), MANAGEDBUF(35)));
	}
	else {
		result->buffer = st_merge("sss", chunk_header_write(PRIME_CHUNK_EPHEMERAL, result->header.length, MANAGEDBUF(4)),
			prime_field_write(PRIME_USER_SIGNET, USER_SIGNING_KEY, ED25519_KEY_PUB_LEN, ed25519_public_get(signing, MANAGEDBUF(32)), MANAGEDBUF(34)),
			(!signing ? NULL : prime_field_write(PRIME_USER_SIGNET, USER_ENCRYPTION_KEY, SECP256K1_KEY_PUB_LEN, secp256k1_public_get(encryption, MANAGEDBUF(33)), MANAGEDBUF(35))));
	}

	if (!result->buffer || (st_length_get(result->buffer) != 39 && st_length_get(result->buffer) != 73)) {
		ephemeral_chunk_free(result);
		return NULL;
	}

	// Dupe the public key structures.
	else if (signing && !(result->keys.signing = ed25519_public_set(ed25519_public_get(signing, MANAGEDBUF(32))))) {
		ephemeral_chunk_free(result);
		return NULL;
	}
	else if (!(result->keys.encryption = secp256k1_public_set(secp256k1_public_get(encryption, MANAGEDBUF(33))))) {
		ephemeral_chunk_free(result);
		return NULL;
	}

	return result;
}

/**
 * @brief	Parse an ephemeral message chunk. A public encryption key is required, while the ephemeral signing key is optional.
 */
prime_ephemeral_chunk_t * ephemeral_chunk_set(stringer_t *chunk) {

	size_t len = 0;
	uchr_t *data = NULL;
	placer_t signing, encryption;
	prime_ephemeral_chunk_t *result = NULL;

	// Ephemeral chunks must be 39 or 73 overall bytes.
	if (st_empty_out(chunk, &data, &len) || (len != 39 && len != 73)) {
		log_pedantic("PRIME ephemeral chunk parsing failed. Invalid string provided.");
		return NULL;
	}
	else if (!(result = ephemeral_chunk_alloc())) {
		return NULL;
	}

	// Setup the header, and serialize the payload, then store it in the buffer.
	else if ((result->header.type = chunk_header_type(chunk)) != PRIME_CHUNK_EPHEMERAL) {
		ephemeral_chunk_free(result);
		return NULL;
	}
	else if ((result->header.length = chunk_header_size(chunk)) == -1 || (result->header.length != 69 && result->header.length != 35)) {
		ephemeral_chunk_free(result);
		return NULL;
	}

	else if (result->header.length == 69) {
		signing = pl_init(st_data_get(chunk) + 4, 34);
		encryption = pl_init(st_data_get(chunk) + 38, 35);

		if (*((uint8_t *)pl_data_get(signing)) != 1 || *((uint8_t *)pl_data_get(signing) + 1) != 32 ||
			!(result->keys.signing = ed25519_public_set(PLACER(pl_data_get(signing) + 2, 32)))) {
			ephemeral_chunk_free(result);
			return NULL;
		}

		else if (*((uint8_t *)pl_data_get(encryption)) != 2 || *((uint8_t *)pl_data_get(encryption) + 1) != 33 ||
			!(result->keys.encryption = secp256k1_public_set(PLACER(pl_data_get(encryption) + 2, 33)))) {
			ephemeral_chunk_free(result);
			return NULL;
		}
	}

	else if (result->header.length == 35) {
		encryption = pl_init(st_data_get(chunk) + 4, 35);

		if (*((uint8_t *)pl_data_get(encryption)) != 2 || *((uint8_t *)pl_data_get(encryption) + 1) != 33 ||
			!(result->keys.encryption = secp256k1_public_set(PLACER(pl_data_get(encryption) + 2, 33)))) {
			ephemeral_chunk_free(result);
			return NULL;
		}
	}

	// Retain a reference to the original chunk data.
	if (!(result->buffer = st_alloc_opts(PLACER_T | JOINTED | HEAP | FOREIGNDATA, 0))) {
		ephemeral_chunk_free(result);
		return NULL;
	}

	st_data_set(result->buffer, st_data_get(chunk));
	st_length_set(result->buffer, st_length_get(chunk));

	return result;
}

