
/**
 * @file /magma/src/providers/prime/messages/chunks/slots.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

#define PRIME_ACTOR_NONE			0x00
#define PRIME_ACTOR_AUTHOR			0x01
#define PRIME_ACTOR_ORIGIN			0x02
#define PRIME_ACTOR_DESTINATION		0x04
#define PRIME_ACTOR_RECIPIENT		0x08

void slots_free(prime_chunk_slots_t *slots) {

	if (slots) {
		mm_free(slots);
	}
#ifdef MAGMA_PEDANTIC
	else {
		log_pedantic("An invalid PRIME keyslot pointer was passed to the free function.");
	}
#endif

	return;
}

void slots_cleanup(prime_chunk_slots_t *slots) {

	if (slots) {
		slots_free(slots);
	}

	return;
}

prime_chunk_slots_t * slots_alloc(prime_message_chunk_type_t type) {

	int_t count = 0, actors = 0;
	prime_chunk_slots_t *result = NULL;

	// Make sure this chunk type has at least 1 keyslot.
	if (!(count = slots_count(type)) || (actors = slots_actors(type)) == PRIME_ACTOR_NONE) {
		return NULL;
	}

	// Append enough trailing space to hold the slots.
	else if (!(result = mm_alloc(sizeof(prime_chunk_slots_t) + (count * SECP256K1_SHARED_SECRET_LEN)))) {
		log_pedantic("PRIME keyslot allocation failed.");
		return NULL;
	}

	// We need wipe the buffer to ensure a clean slate.
	mm_wipe(result, sizeof(prime_chunk_slots_t) + (count * SECP256K1_SHARED_SECRET_LEN));

	// Allocate the buffer representing all the slots together.
	result->buffer = pl_init(result + sizeof(prime_chunk_slots_t), (count * SECP256K1_SHARED_SECRET_LEN));

	// For each actor, setup the location of the keyslot, and use the descending count variable to find the
	// memory offset for the start of the keyslot.
	if ((actors & PRIME_ACTOR_RECIPIENT) == PRIME_ACTOR_RECIPIENT) {
		result->recipient = pl_init(result + sizeof(prime_chunk_slots_t) + ((count - 1) * SECP256K1_SHARED_SECRET_LEN),
			SECP256K1_SHARED_SECRET_LEN);
		count--;
	}

	if ((actors & PRIME_ACTOR_DESTINATION) == PRIME_ACTOR_DESTINATION) {
		result->destination = pl_init(result + sizeof(prime_chunk_slots_t) + ((count - 1) * SECP256K1_SHARED_SECRET_LEN),
			SECP256K1_SHARED_SECRET_LEN);
		count--;
	}

	if ((actors & PRIME_ACTOR_ORIGIN) == PRIME_ACTOR_ORIGIN) {
		result->origin = pl_init(result + sizeof(prime_chunk_slots_t) + ((count - 1) * SECP256K1_SHARED_SECRET_LEN),
			SECP256K1_SHARED_SECRET_LEN);
		count--;
	}

	if ((actors & PRIME_ACTOR_AUTHOR) == PRIME_ACTOR_AUTHOR) {
		result->author = pl_init(result + sizeof(prime_chunk_slots_t) + ((count - 1) * SECP256K1_SHARED_SECRET_LEN),
			SECP256K1_SHARED_SECRET_LEN);
		count--;
	}

	if (count) {
		log_pedantic("PRIME keyslot allocation layout failed. { count = %i }", count);
		slots_free(result);
		return NULL;
	}

	return result;
}

/**
 * @brief	Returns a set of bitwise flags indicating which actors are allowed to access a given chunk type.
 */
int_t slots_actors(prime_message_chunk_type_t type) {

	int_t result = PRIME_ACTOR_NONE;

	switch(type) {

		case PRIME_CHUNK_ORIGIN:
			result = PRIME_ACTOR_AUTHOR | PRIME_ACTOR_ORIGIN | PRIME_ACTOR_RECIPIENT;
			break;

		case PRIME_CHUNK_DESTINATION:
			result = PRIME_ACTOR_AUTHOR | PRIME_ACTOR_DESTINATION | PRIME_ACTOR_RECIPIENT;
			break;

		case PRIME_CHUNK_COMMON:
			result = PRIME_ACTOR_AUTHOR | PRIME_ACTOR_RECIPIENT;
			break;

		case PRIME_CHUNK_HEADERS:
			result = PRIME_ACTOR_AUTHOR | PRIME_ACTOR_RECIPIENT;
			break;

		case PRIME_CHUNK_BODY:
			result = PRIME_ACTOR_AUTHOR | PRIME_ACTOR_RECIPIENT;
			break;

		case PRIME_CHUNK_SIGNATURE_TREE:
			result = PRIME_ACTOR_AUTHOR | PRIME_ACTOR_ORIGIN | PRIME_ACTOR_RECIPIENT;
			break;

		case PRIME_CHUNK_SIGNATURE_AUTHOR:
			result = PRIME_ACTOR_AUTHOR | PRIME_ACTOR_ORIGIN | PRIME_ACTOR_RECIPIENT;
			break;

		case PRIME_CHUNK_SIGNATURE_ORGIN:
			result = PRIME_ACTOR_AUTHOR | PRIME_ACTOR_ORIGIN | PRIME_ACTOR_DESTINATION | PRIME_ACTOR_RECIPIENT;
			break;

		case PRIME_CHUNK_SIGNATURE_DESTINATION:
			result = PRIME_ACTOR_DESTINATION | PRIME_ACTOR_RECIPIENT;
			break;

		default:
			log_pedantic("The provided chunk type wasn't recognized. { type = %i }", type);
			break;
	}

	return result;
}

/**
 * @brief	Returns the number of slots for a given chunk type.
 */
int_t slots_count(prime_message_chunk_type_t type) {
	return bitwise_count(slots_actors(type));
}

stringer_t * slots_buffer(prime_chunk_slots_t *slots) {

	stringer_t *result = NULL;

	if (slots && pl_length_get(slots->buffer)) {
		result = &slots->buffer;
	}

	return result;
}

prime_chunk_slots_t * slots_get(prime_message_chunk_type_t type, stringer_t *key, prime_chunk_keks_t *keks) {

	int_t actors = 0;
	prime_chunk_slots_t *result = NULL;

	if (!key || st_length_get(key) != 32 || !keks || (actors = slots_actors(type)) == PRIME_ACTOR_NONE || slots_count(type) < 2 ||
		slots_count(type) > 4 || (!keks->author && !keks->recipient) || (keks->author && st_length_get(keks->author) != 32) ||
		(keks->origin && st_length_get(keks->origin) != 32) || (keks->recipient && st_length_get(keks->recipient) != 32) ||
		(keks->destination && st_length_get(keks->destination) != 32) || !(result = slots_alloc(type))) {
		return NULL;
	}

	// Generate slots for the actors we recieved keys for, starting with the author.
	else if ((actors & PRIME_ACTOR_AUTHOR) && keks->author && !st_xor(key, keks->author, &(result->author))) {
		slots_free(result);
		return NULL;
	}

	else if ((actors & PRIME_ACTOR_ORIGIN) && keks->origin && !st_xor(key, keks->origin, &(result->origin))) {
		slots_free(result);
		return NULL;
	}

	else if ((actors & PRIME_ACTOR_DESTINATION) && keks->destination && !st_xor(key, keks->destination, &(result->destination))) {
		slots_free(result);
		return NULL;
	}

	else if ((actors & PRIME_ACTOR_RECIPIENT) && keks->recipient && !st_xor(key, keks->recipient, &(result->recipient))) {
		slots_free(result);
		return NULL;
	}

	return result;
}

prime_chunk_slots_t * slots_set(prime_message_chunk_type_t type, stringer_t *slots) {

	size_t size = 0;
	int_t actors = 0, count = 0;
	prime_chunk_slots_t *result = NULL;

	if ((actors = slots_actors(type)) == PRIME_ACTOR_NONE || (count = slots_count(type)) < 2 || count > 4 || !slots ||
		st_length_get(slots) % 32 != 0 || st_length_get(slots) != (size = (count * SECP256K1_SHARED_SECRET_LEN)) ||
		!(result = slots_alloc(type))) {
		return NULL;
	}

	// Copy the slots into the structure buffer.
	else if (st_length_get(&(result->buffer)) != st_length_get(slots) || st_write(&(result->buffer), slots) != size)  {
		slots_free(result);
		return NULL;
	}

	// Generate slots for the actors with slots, starting with the author.
	if ((actors & PRIME_ACTOR_RECIPIENT) == PRIME_ACTOR_RECIPIENT) {
		result->recipient = pl_init(slots + ((count - 1) * SECP256K1_SHARED_SECRET_LEN), SECP256K1_SHARED_SECRET_LEN);
		count--;
	}

	if ((actors & PRIME_ACTOR_DESTINATION) == PRIME_ACTOR_DESTINATION) {
		result->destination = pl_init(slots + ((count - 1) * SECP256K1_SHARED_SECRET_LEN), SECP256K1_SHARED_SECRET_LEN);
		count--;
	}

	if ((actors & PRIME_ACTOR_ORIGIN) == PRIME_ACTOR_ORIGIN) {
		result->origin = pl_init(slots + ((count - 1) * SECP256K1_SHARED_SECRET_LEN), SECP256K1_SHARED_SECRET_LEN);
		count--;
	}

	if ((actors & PRIME_ACTOR_AUTHOR) == PRIME_ACTOR_AUTHOR) {
		result->author = pl_init(slots + ((count - 1) * SECP256K1_SHARED_SECRET_LEN), SECP256K1_SHARED_SECRET_LEN);
		count--;
	}

	if (count) {
		log_pedantic("PRIME keyslot parsing failed. { count = %i }", count);
		slots_free(result);
		return NULL;
	}

	return result;
}
