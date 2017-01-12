
/**
 * @file /magma/src/providers/prime/messages/messages.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

void encrypted_message_free(prime_message_t *object) {

	if (object) {
		if (object->keys.kek) st_free(object->keys.kek);
		if (object->keys.ephemeral) secp256k1_free(object->keys.ephemeral);
		if (object->keys.author) secp256k1_free(object->keys.author);
		if (object->keys.origin) secp256k1_free(object->keys.origin);
		if (object->keys.destination) secp256k1_free(object->keys.destination);
		if (object->keys.recipient) secp256k1_free(object->keys.recipient);
		if (object->envelope.ephemeral) ephemeral_chunk_free(object->envelope.ephemeral);
		if (object->envelope.origin) encrypted_chunk_free(object->envelope.origin);
		if (object->envelope.destination) encrypted_chunk_free(object->envelope.destination);
		if (object->metadata.common) encrypted_chunk_free(object->metadata.common);
		if (object->metadata.headers) encrypted_chunk_free(object->metadata.headers);
		if (object->content.body) encrypted_chunk_free(object->content.body);
		if (object->signatures.tree) signature_chunk_free(object->signatures.tree);
		if (object->signatures.author) signature_chunk_free(object->signatures.author);
		if (object->signatures.org) signature_chunk_free(object->signatures.org);
		mm_free(object);
	}
#ifdef MAGMA_PEDANTIC
	else {
		log_pedantic("An invalid PRIME message pointer was passed to the free function.");
	}
#endif

	return;
}

void encrypted_message_cleanup(prime_message_t *object) {
	if (object) {
		encrypted_message_free(object);
	}
	return;
}

prime_message_t * encrypted_message_alloc(void) {

	prime_message_t *result = NULL;

	if (!(result = mm_alloc(sizeof(prime_message_t)))) {
		log_pedantic("PRIME message allocation failed.");
		return NULL;
	}

	// We need wipe the buffer to ensure a clean slate.
	mm_wipe(result, sizeof(prime_message_t));

	return result;
}
