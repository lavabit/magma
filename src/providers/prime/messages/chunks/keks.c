
/**
 * @file /magma/providers/prime/messages/chunks/keks.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

void keks_free(prime_chunk_keks_t *keks) {

	if (keks) {
		st_cleanup(keks->author, keks->origin, keks->destination, keks->recipient);
		mm_free(keks);
	}
#ifdef MAGMA_PEDANTIC
	else {
		log_pedantic("An invalid PRIME kek pointer was passed to the free function.");
	}
#endif

	return;
}

void keks_cleanup(prime_chunk_keks_t *keks) {

	if (keks) {
		keks_free(keks);
	}

	return;
}

prime_chunk_keks_t * keks_alloc(void) {

	prime_chunk_keks_t *result = NULL;

	// Allocate the kek structure.
	if (!(result = mm_alloc(sizeof(prime_chunk_keks_t)))) {
		log_pedantic("PRIME kek allocation failed.");
		return NULL;
	}

	// We need wipe the buffer to ensure a clean slate.
	mm_wipe(result, sizeof(prime_chunk_keks_t));

	return result;
}

/**
 * @brief	Uses the private ephemeral encryption key to compute a KEK with all of the provided actor public keys.
 */
prime_chunk_keks_t * keks_get(prime_chunk_keys_t *keys, prime_chunk_keks_t *keks) {

	prime_chunk_keks_t *result = NULL;

	// Ensure we have the private ephemeral key, and either the author, or recipient public keys, as all encrypted
	// chunk types require at least one of them to be valid.
	if (!keys || (!keys->encryption && secp256k1_type(keys->encryption) != SECP256K1_PRIV) ||
		((!keys->author || secp256k1_type(keys->author) != SECP256K1_PUB) &&
		(!keys->recipient || secp256k1_type(keys->recipient) != SECP256K1_PUB))) {
		return NULL;
	}
	else if (!(result = keks) && !(result = keks_alloc())) {
		return NULL;
	}

	// Author keyslot.
	else if (keys->author && !(result->author = secp256k1_compute_kek(keys->encryption, keys->author, NULL))) {
		if (!keks) keks_free(result);
		return NULL;
	}

	// Origin keyslot.
	else if (keys->origin && !(result->origin = secp256k1_compute_kek(keys->encryption, keys->origin, NULL))) {
		if (!keks) keks_free(result);
		return NULL;
	}

	// Destination keyslot.
	else if (keys->destination && !(result->destination = secp256k1_compute_kek(keys->encryption, keys->destination, NULL))) {
		if (!keks) keks_free(result);
		return NULL;
	}

	// Recipient keyslot.
	else if (keys->recipient && !(result->recipient = secp256k1_compute_kek(keys->encryption, keys->recipient, NULL))) {
		if (!keks) keks_free(result);
		return NULL;
	}

	return result;
}

/**
 * @brief	Uses the public ephemeral encryption key to compute a KEK with all of the provided actor private keys.
 */
prime_chunk_keks_t * keks_set(prime_chunk_keys_t *keys, prime_chunk_keks_t *keks) {

	prime_chunk_keks_t *result = NULL;

	// Make sure the ephemeral encryption key is the public portion, and we have at least one value actor private key.
	if (!keys || (!keys->encryption && secp256k1_type(keys->encryption) != SECP256K1_PUB) ||
		((!keys->author || secp256k1_type(keys->author) != SECP256K1_PRIV) &&
		(!keys->origin || secp256k1_type(keys->origin) != SECP256K1_PRIV) &&
		(!keys->destination || secp256k1_type(keys->destination) != SECP256K1_PRIV) &&
		(!keys->recipient || secp256k1_type(keys->recipient) != SECP256K1_PRIV))) {
		return NULL;
	}
	else if (!(result = keks) && !(result = keks_alloc())) {
		return NULL;
	}

	// Author keyslot.
	else if (keys->author && !(result->author = secp256k1_compute_kek(keys->author, keys->encryption, NULL))) {
		if (!keks) keks_free(result);
		return NULL;
	}

	// Origin keyslot.
	else if (keys->origin && !(result->origin = secp256k1_compute_kek(keys->origin, keys->encryption, NULL))) {
		if (!keks) keks_free(result);
		return NULL;
	}

	// Destination keyslot.
	else if (keys->destination && !(result->destination = secp256k1_compute_kek(keys->destination, keys->encryption, NULL))) {
		if (!keks) keks_free(result);
		return NULL;
	}

	// Recipient keyslot.
	else if (keys->recipient && !(result->recipient = secp256k1_compute_kek(keys->recipient, keys->encryption, NULL))) {
		if (!keks) keks_free(result);
		return NULL;
	}

	return result;
}
