
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
		if (object->keys.signing) ed25519_free(object->keys.signing);
		if (object->keys.encryption) secp256k1_free(object->keys.encryption);

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

stringer_t * naked_message_get(prime_message_t *message) {
	return NULL;
}

prime_message_t * naked_message_set(stringer_t *message, prime_org_key_t *destination, prime_user_signet_t *recipient) {

	prime_message_t *result = NULL;

	if (!(result = encrypted_message_alloc())) {
		return NULL;
	}
	else if (!(result->keys.signing = ed25519_generate()) || !(result->keys.encryption)) {
		encrypted_message_free(result);
		return NULL;
	}
	else if (!(result->envelope.ephemeral = ephemeral_chunk_get(result->keys.signing, result->keys.encryption))) {
		encrypted_message_free(result);
		return NULL;
	}

	return result;
}
