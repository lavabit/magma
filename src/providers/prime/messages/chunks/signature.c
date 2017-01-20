
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

void signature_tree_free(prime_signature_tree_t *chunk) {

	if (chunk) {
		if (chunk->tree) inx_free(chunk->tree);
		mm_free(chunk);
	}
#ifdef MAGMA_PEDANTIC
	else {
		log_pedantic("An invalid PRIME signature chunk pointer was passed to the free function.");
	}
#endif

	return;
}

void signature_tree_cleanup(prime_signature_tree_t *chunk) {
	if (chunk) {
		signature_tree_free(chunk);
	}
	return;
}

prime_signature_tree_t * signature_tree_alloc(void) {

	prime_signature_tree_t *result = NULL;

	if (!(result = mm_alloc(sizeof(prime_signature_tree_t)))) {
		log_pedantic("PRIME signature chunk allocation failed.");
		return NULL;
	}

	// We need wipe the buffer to ensure a clean slate.
	mm_wipe(result, sizeof(prime_signature_tree_t));

	if (!(result->tree = inx_alloc(M_INX_LINKED, &st_free))) {
		log_pedantic("PRIME tree signature index allocation failed.");
		return NULL;
	}

	return result;
}

int_t signature_tree_add(prime_signature_tree_t *chunk, stringer_t *data) {

	multi_t key;
	stringer_t *hash = NULL;

	if (!chunk || !chunk->tree || !data || st_length_get(data) < 35) {
		return -1;
	}

	key.type = M_TYPE_UINT64;
	key.val.u64 = inx_count(chunk->tree);

	if (!(hash = hash_sha512(data, NULL))) {
		return -2;
	}
	else if (!inx_insert(chunk->tree, key, hash)) {
		st_free(hash);
		return -3;
	}

	return 0;
}

stringer_t * signature_tree_get(ed25519_key_t *signing, prime_signature_tree_t *chunk, prime_chunk_keks_t *keks) {

	placer_t buffer;
	uint64_t count = 0;
	inx_cursor_t *cursor;
	prime_chunk_slots_t *slots = NULL;
	uint8_t type = PRIME_SIGNATURE_TREE;
	stringer_t *result = NULL, *combined = NULL, *value = NULL, *stretched = NULL, *signature = NULL,
		*shard = NULL, *key = MANAGEDBUF(32);

	if (!signing || ed25519_type(signing) != ED25519_PRIV || !chunk || !chunk->tree || !(count = inx_count(chunk->tree))) {
		return NULL;
	}

	else if (!(cursor = inx_cursor_alloc(chunk->tree)) || !(combined = st_alloc_opts(MANAGED_T | JOINTED | HEAP, count * SHA512_DIGEST_LENGTH))) {
		if (cursor) inx_cursor_free(cursor);
		return NULL;
	}

	while ((value = inx_cursor_value_next(cursor))) {
		if (!st_append(combined, value)) {
			inx_cursor_free(cursor);
			st_free(combined);
			return NULL;
		}
	}

	// We don't need the cursor anymore.
	inx_cursor_free(cursor);

	// Generate a key and stretch it.
	if (rand_write(key) != 32 || !(stretched = hash_sha512(key, MANAGEDBUF(64)))) {
		st_free(combined);
		return NULL;
	}

	else if (!(signature = ed25519_sign(signing, combined, MANAGEDBUF(64))) ||
		!(shard = st_xor(signature, stretched, MANAGEDBUF(64)))) {
		st_free(combined);
		return NULL;
	}

	st_free(combined);

	if (!(slots = slots_set(PRIME_SIGNATURE_TREE, key, keks))) {
		return NULL;
	}

	buffer = slots_buffer(slots);

	if (!(result = st_alloc(SHA512_DIGEST_LENGTH + 97)) || st_length_get(&buffer) != 96 ||
		st_write(result, PLACER(&type, 1), shard, &buffer) != (SHA512_DIGEST_LENGTH + 97)) {

		st_cleanup(result);
		slots_free(slots);
		return NULL;
	}

	slots_free(slots);

	return result;
}

stringer_t * signature_full_get(prime_message_chunk_type_t type, ed25519_key_t *signing, prime_chunk_keks_t *keks, stringer_t *data) {

	placer_t buffer;
	prime_chunk_slots_t *slots = NULL;
	stringer_t *result = NULL, *stretched = NULL, *signature = NULL, *shard = NULL, *key = MANAGEDBUF(32);

	if (!signing || ed25519_type(signing) != ED25519_PRIV || !data) {
		return NULL;
	}

	// Generate a key and stretch it.
	if (rand_write(key) != 32 || !(stretched = hash_sha512(key, MANAGEDBUF(64)))) {
		return NULL;
	}

	else if (!(signature = ed25519_sign(signing, data, MANAGEDBUF(64))) ||
		!(shard = st_xor(signature, stretched, MANAGEDBUF(64)))) {
		return NULL;
	}

	else if (!(slots = slots_set(type, key, keks))) {
		return NULL;
	}

	buffer = slots_buffer(slots);

	if (!(result = st_alloc(SHA512_DIGEST_LENGTH + 129)) ||
		(st_length_get(&buffer) != 64 && st_length_get(&buffer) != 96 && st_length_get(&buffer) != 128) ||
		st_write(result, PLACER(&type, 1), shard, &buffer) != (st_length_get(&buffer) + SHA512_DIGEST_LENGTH + 1)) {

		st_cleanup(result);
		slots_free(slots);
		return NULL;
	}

	slots_free(slots);
	return result;
}

