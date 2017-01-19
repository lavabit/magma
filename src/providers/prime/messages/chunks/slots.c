
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
	mm_wipe((uchr_t *)result, sizeof(prime_chunk_slots_t) + (count * SECP256K1_SHARED_SECRET_LEN));

	// Allocate the buffer representing all the slots together.
	result->buffer = pl_init((uchr_t *)result + sizeof(prime_chunk_slots_t), (count * SECP256K1_SHARED_SECRET_LEN));

	// For each actor, setup the location of the keyslot, and use the descending count variable to find the
	// memory offset for the start of the keyslot.
	if ((actors & PRIME_ACTOR_RECIPIENT) == PRIME_ACTOR_RECIPIENT) {
		result->recipient = pl_init((uchr_t *)result + sizeof(prime_chunk_slots_t) + ((count - 1) * SECP256K1_SHARED_SECRET_LEN),
			SECP256K1_SHARED_SECRET_LEN);
		count--;
	}

	if ((actors & PRIME_ACTOR_DESTINATION) == PRIME_ACTOR_DESTINATION) {
		result->destination = pl_init((uchr_t *)result  + sizeof(prime_chunk_slots_t) + ((count - 1) * SECP256K1_SHARED_SECRET_LEN),
			SECP256K1_SHARED_SECRET_LEN);
		count--;
	}

	if ((actors & PRIME_ACTOR_ORIGIN) == PRIME_ACTOR_ORIGIN) {
		result->origin = pl_init((uchr_t *)result + sizeof(prime_chunk_slots_t) + ((count - 1) * SECP256K1_SHARED_SECRET_LEN),
			SECP256K1_SHARED_SECRET_LEN);
		count--;
	}

	if ((actors & PRIME_ACTOR_AUTHOR) == PRIME_ACTOR_AUTHOR) {
		result->author = pl_init((uchr_t *)result + sizeof(prime_chunk_slots_t) + ((count - 1) * SECP256K1_SHARED_SECRET_LEN),
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

placer_t slots_buffer(prime_chunk_slots_t *slots) {

	placer_t result = pl_null();

	if (slots && pl_length_get(slots->buffer)) {
		result = slots->buffer;
	}

	return result;
}

/**
 * @brief	Get the unstreteched key value using the first available kek with a matching keyslot value.
 */
stringer_t * slots_key(prime_chunk_slots_t *slots, prime_chunk_keks_t *keks, stringer_t *output) {

	stringer_t *result = NULL, *empty = st_set(MANAGEDBUF(32), 0, 32);

	// Ensure we have at least one valid key encryption key.
	if (!keks || (keks->author && st_length_get(keks->author) != 32) ||
		(keks->origin && st_length_get(keks->origin) != 32) ||
		(keks->recipient && st_length_get(keks->recipient) != 32) ||
		(keks->destination && st_length_get(keks->destination) != 32)) {
		log_pedantic("Invalid key encryption key. Unable to parse the chunk key.");
		return NULL;
	}
	// Ensure we have at least one valid key slot.
	else if (!slots || (!pl_empty(slots->author) && st_length_get(&(slots->author)) != 32) ||
		(!pl_empty(slots->origin) && st_length_get(&(slots->origin)) != 32) ||
		(!pl_empty(slots->recipient) && st_length_get(&(slots->recipient)) != 32) ||
		(!pl_empty(slots->destination) && st_length_get(&(slots->destination)) != 32)) {
		log_pedantic("Invalid key slots. Unable to parse the chunk key.");
		return NULL;
	}

	// Allocate a buffer for the output if one wasn't provided.
	else if (!(result = st_output(output, SECP256K1_SHARED_SECRET_LEN))) {
		log_pedantic("Allocate an output buffer. Unable to parse the chunk key.");
		return NULL;
	}

	// If we have a key encryption key, and a keyslot value for the same actor, and the key slot value isn't
	// just a string 0's, attempt the XOR operation. If it fails, free any memory we allocated and return NULL.
	else if (keks->author && !pl_empty(slots->author) && st_cmp_cs_eq(empty, &(slots->author))) {
		if (!st_xor(keks->author, &(slots->author), result)) {
			if (!output) st_free(result);
			result = NULL;
		}
	}

	else if (keks->origin && !pl_empty(slots->origin) && st_cmp_cs_eq(empty, &(slots->origin))) {
		if (!st_xor(keks->origin, &(slots->origin), result)) {
			if (!output) st_free(result);
			result = NULL;
		}
	}

	else if (keks->destination && !pl_empty(slots->destination) && st_cmp_cs_eq(empty, &(slots->destination))) {
		if (!st_xor(keks->destination, &(slots->destination), result)) {
			if (!output) st_free(result);
			result = NULL;
		}
	}

	else if (keks->recipient && !pl_empty(slots->recipient) && st_cmp_cs_eq(empty, &(slots->recipient))) {
		if (!st_xor(keks->recipient, &(slots->recipient), result)) {
			if (!output) st_free(result);
			result = NULL;
		}
	}

	// If we didn't have an available key slot for the provided key encryption keys, free any memory we
	// allocated, and return NULL.
	else {
		if (!output) st_free(result);
		result = NULL;
	}

	return result;
}

prime_chunk_slots_t * slots_set(prime_message_chunk_type_t type, stringer_t *key, prime_chunk_keks_t *keks) {

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

stringer_t * slots_get(prime_message_chunk_type_t type, stringer_t *slots, prime_chunk_keks_t *keks, stringer_t *output) {

	size_t size = 0;
	int_t actors = 0, count = 0;
	prime_chunk_slots_t *parsed = NULL;

	if ((actors = slots_actors(type)) == PRIME_ACTOR_NONE || (count = slots_count(type)) < 2 || count > 4 || !slots ||
		st_length_get(slots) % 32 != 0 || st_length_get(slots) != (size = (count * SECP256K1_SHARED_SECRET_LEN)) ||
		!(parsed = slots_alloc(type))) {
		return NULL;
	}

	// Copy the slots into the structure buffer.
	else if (st_length_get(&(parsed->buffer)) != st_length_get(slots))  {
		slots_free(parsed);
		return NULL;
	}

	// Set the buffer place holder to point at the provided slots buffer.
	parsed->buffer = pl_init(st_data_get(slots), size);

	// Generate slots for the actors with slots, starting with the author.
	if ((actors & PRIME_ACTOR_RECIPIENT) == PRIME_ACTOR_RECIPIENT) {
		parsed->recipient = pl_init(st_data_get(slots) + ((count - 1) * SECP256K1_SHARED_SECRET_LEN), SECP256K1_SHARED_SECRET_LEN);
		count--;
	}

	if ((actors & PRIME_ACTOR_DESTINATION) == PRIME_ACTOR_DESTINATION) {
		parsed->destination = pl_init(st_data_get(slots) + ((count - 1) * SECP256K1_SHARED_SECRET_LEN), SECP256K1_SHARED_SECRET_LEN);
		count--;
	}

	if ((actors & PRIME_ACTOR_ORIGIN) == PRIME_ACTOR_ORIGIN) {
		parsed->origin = pl_init(st_data_get(slots) + ((count - 1) * SECP256K1_SHARED_SECRET_LEN), SECP256K1_SHARED_SECRET_LEN);
		count--;
	}

	if ((actors & PRIME_ACTOR_AUTHOR) == PRIME_ACTOR_AUTHOR) {
		parsed->author = pl_init(st_data_get(slots) + ((count - 1) * SECP256K1_SHARED_SECRET_LEN), SECP256K1_SHARED_SECRET_LEN);
		count--;
	}

	if (count) {
		log_pedantic("PRIME keyslot parsing failed. { count = %i }", count);
		slots_free(parsed);
		return NULL;
	}

	return slots_key(parsed, keks, output);
}
