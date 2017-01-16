
/**
 * @file /magma/src/providers/prime/messages/chunks/keyslots.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

prime_chunk_slots_t * keyslots_alloc(void) {

	return NULL;
}

void keyslots_free(prime_chunk_slots_t *keyslots) {

	return;
}

void keyslots_cleanup(prime_chunk_slots_t *keyslots) {

	if (keyslots) {
		keyslots_free(keyslots);
	}

	return;
}
