
/**
 * @file /magma/src/providers/prime/messages/chunks/signature.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

void signature_chunk_free(prime_signature_chunk_t *chunk) {

	if (chunk) {
		if (chunk->signature) st_free(chunk->signature);
		if (chunk->slots.author) st_free(chunk->slots.author);
		if (chunk->slots.origin) st_free(chunk->slots.origin);
		if (chunk->slots.destination) st_free(chunk->slots.destination);
		if (chunk->slots.recipient) st_free(chunk->slots.recipient);
		mm_free(chunk);
	}
#ifdef MAGMA_PEDANTIC
	else {
		log_pedantic("An invalid PRIME signature chunk pointer was passed to the free function.");
	}
#endif

	return;
}

void signature_chunk_cleanup(prime_signature_chunk_t *chunk) {
	if (chunk) {
		signature_chunk_free(chunk);
	}
	return;
}

prime_signature_chunk_t * signature_chunk_alloc(void) {

	prime_signature_chunk_t *result = NULL;

	if (!(result = mm_alloc(sizeof(prime_signature_chunk_t)))) {
		log_pedantic("PRIME signature chunk allocation failed.");
		return NULL;
	}

	// We need wipe the buffer to ensure a clean slate.
	mm_wipe(result, sizeof(prime_signature_chunk_t));

	return result;
}
