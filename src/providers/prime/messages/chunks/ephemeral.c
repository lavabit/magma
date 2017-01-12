
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
