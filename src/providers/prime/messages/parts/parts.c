
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

	while (remaining) {
//
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
	}

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

	// We need a signing key, encryption key, and at least one actor.
	if (!signing || ed25519_type(signing) != ED25519_PUB ||
		!keks || (!keks->author && !keks->origin && !keks->destination && !keks->recipient) || !part) {
		log_pedantic("Invalid parameters passed to the encrypted chunk parser.");
		return NULL;
	}

	return NULL;
//
//	size_t part_size;
//	uint8_t curr_type;
//	uint32_t chunk_size = 0;
//	placer_t slice = pl_null();
//	stringer_t *result = NULL, *curr_chunk = NULL;
//
//	// Set the data buffer of the part stringer to the beginning of the payload.
//	st_data_set(part, st_data_get(payload));
//
//	while (true) {
//
//		// Read the chunk headers from the beginning of the remaining payload.
//		if (chunk_header_read(payload, &curr_type, &chunk_size, &slice) != 0) break;
//		part_size += st_length_get(&slice);
//
//		// TODO: Instead, check if the spanning-chunk flag (128) is set in the header.
//		// Check if the chunk is still a part of the part that we are decrypting.
//		if (curr_type != type) break;
//
//		// Pass this into encrypted_chunk_get().
//		if (!(curr_chunk = encrypted_chunk_get(signing, keks, &slice, NULL))) {
//			st_cleanup(result);
//			return NULL;
//		}
//
//		// Either append curr_chunk onto result or set result equal to chunk.
//		result = st_append(result, curr_chunk);
//
//		// Free curr_chunk.
//		st_free(curr_chunk);
//
//		// Update payload and loop if necessary.
//		st_data_set(payload, st_data_get(payload) + st_length_get(&slice));
//		st_length_set(payload, st_length_get(payload) - st_length_get(&slice));
//	}
//
//	// Construct the part-spanning chunk placer.
//	st_length_set(part, part_size);
//
//	return result;
}

