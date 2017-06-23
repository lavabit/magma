
/**
 * @file /magma/src/providers/prime/messages/parts/parts.c
 *
 * @brief
 */

#include "magma.h"
/***
 * @brief	Turn a plain text payload into an encrypted body part. Typically this will involve converting the payload into an encrypted
 * 			chunk, however, if the payload is large, it may be necessary to split up the data, causing it to span across chunks. The
 * 			parts interface abstracts away spanning chunks, allowing the higher level interfaces to process payloads without concern for
 * 			their size.
 */
prime_encrypted_chunk_t * part_encrypt(prime_message_chunk_type_t type, ed25519_key_t *signing, prime_chunk_keks_t *keks, stringer_t *payload) {

	uchr_t *data = NULL;
	size_t remaining = 0;
	prime_encrypted_chunk_t *result = NULL, *current = NULL, *holder = NULL;

	// We need a signing key, encryption key, and at least one actor.
	if (!signing || !keks || st_empty_out(payload, &data, &remaining) || ed25519_type(signing) != ED25519_PRIV ||
		(!keks->author && !keks->origin && !keks->destination && !keks->recipient)) {
		log_pedantic("Invalid parameters passed to the encrytpted body part function.");
		return NULL;
	}

	// Process the payload. If the entire payload will fit, encrypt the entire buffer (or what remains), otherwise we'll set
	// the spanning chunk flag, and loop around until we finish processing the entire payload.
	do {

		// Process the payload. If the entire payload will fit, encrypt the entire buffer (or what remains), otherwise we'll
		// the next 16,777,098 bytes, set the spanning chunk flag, and loop around to finish the job.
		if (!(holder = encrypted_chunk_set(type, signing, keks, (remaining > 16777098 ? PRIME_CHUNK_FLAG_SPANNING : PRIME_CHUNK_FLAG_NONE),
			PLACER(data, (remaining > 16777098 ? 16777098 : remaining))))) {
			encrypted_chunk_cleanup(result);
			return NULL;
		}

		// Keep track of the result by appending it onto the end of our linked list.
		if (!result) {
			result = current = holder;
		}
		else {
			current->next = holder;
			current = holder;
		}

		// Advance our pointer and calculate the number of remaining bytes.
		data += (remaining > 16777098 ? 16777098 : remaining);
		remaining -= (remaining > 16777098 ? 16777098 : remaining);

	} while (remaining);

	return result;
}

/***
 * @brief	Takes a serialized body part, stored as one or more encrypted chunks, and decrypts each in sequence, returning the
 * 			original plain text payload as the result.
 */
stringer_t * part_decrypt(ed25519_key_t *signing, prime_chunk_keks_t *keks, stringer_t *part, stringer_t *output) {

//	uint8_t type = 0;
//	uint32_t legnth = 0;
//	placer_t chunk = pl_null();

	uchr_t *data = NULL;
	bool_t spanning = false;
	size_t length = 0, remaining = 0;
	stringer_t *result = NULL, *payload = NULL;

	// We need a signing key, encryption key, and at least one actor.
	if (!signing || st_empty_out(part, &data, &remaining) ||ed25519_type(signing) != ED25519_PUB ||
		!keks || (!keks->author && !keks->origin && !keks->destination && !keks->recipient) || !part) {
		log_pedantic("Invalid parameters passed to the encrypted chunk parser.");
		return NULL;
	}

	// Decrypt the first chunk, and if the spanning chunk flag is set, keep looping until we encounter a chunk without the
	// spanning chunk flag, or we exhaust the data buffer.
	do {

		if (!(payload = encrypted_chunk_get(signing, keks, PLACER(data, (length = chunk_header_size(PLACER(data, remaining)))), NULL, &spanning))) {
			log_pedantic("Body part decryption failed.");
			return NULL;
		}
		else if (result) {
			result = st_append(result, payload);
			st_free(payload);
		}
		else {
			result = payload;
		}

		data += length;
		remaining -= length;

	} while (remaining && spanning);

	// If the entire buffer was processed and the spanning flag is still set, then our result is incomplete, so rather than
	// return partial data, we return NULL to indicate an error.
	if (!remaining && spanning) {
		log_pedantic("Body part decryption failed. The last chunk in the span is missing.");
		st_free(result);
		return NULL;
	}

	return result;
}

