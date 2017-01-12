
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
		if (chunk->payload.plain.buffer) st_free(chunk->payload.plain.buffer);
		if (chunk->payload.encrypted.buffer) st_free(chunk->payload.encrypted.buffer);
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
