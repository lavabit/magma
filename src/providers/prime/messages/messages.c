
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

		if (object->signatures.tree) st_free(object->signatures.tree);
		if (object->signatures.user) st_free(object->signatures.user);
		if (object->signatures.org) st_free(object->signatures.org);

		if (object->encrypted) st_free(object->encrypted);

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

stringer_t * naked_message_get(stringer_t *message, prime_org_signet_t *org, prime_user_key_t *user) {

	placer_t chunk[7];
	uint8_t type = 0;
	uint32_t size = 0;
	uint16_t object = 0;
	uchr_t *data = NULL;
	size_t remaining = 0;
	prime_chunk_keys_t keys;
	prime_chunk_keks_t *keks = NULL;
	prime_signature_tree_t *tree = NULL;
	prime_ephemeral_chunk_t *ephemeral = NULL;
	stringer_t *headers = NULL, *body = NULL, *result = NULL;

	if (!message || !org || !user) {
		return NULL;
	}

	else if (prime_header_read(message, &object, &size) || object != PRIME_MESSAGE_NAKED ||
		st_length_get(message) != (size + 6) ||	size < 35) {
		return NULL;
	}

	st_empty_out(message, &data, &remaining);

	// Skip the header.
	data += 6;
	remaining -= 6;

	// Imported naked messages have no author. Instead they use an ephemeral signing key, so we need to make sure this message
	// provides an ephemeral signing, and an ephemeral encryption key.
	if (chunk_header_read(PLACER(data, remaining), &type, &size, &chunk[0]) < 0 || type != PRIME_CHUNK_EPHEMERAL ||
		!(ephemeral = ephemeral_chunk_set(&chunk[0])) || !(ephemeral->keys.signing) || !(ephemeral->keys.encryption)) {
		ephemeral_chunk_cleanup(ephemeral);
		return NULL;
	}

	mm_wipe(&keys, sizeof(prime_chunk_keys_t));
	keys.signing = ephemeral->keys.signing;
	keys.encryption = ephemeral->keys.encryption;
	keys.recipient = user->encryption;

	if (!(keks = keks_set(&keys, NULL))) {
		ephemeral_chunk_free(ephemeral);
		return NULL;
	}

	// Common chunk.
	data += st_length_get(&chunk[0]);
	remaining -= st_length_get(&chunk[0]);

	if (chunk_header_read(PLACER(data, remaining), &type, &size, &chunk[1]) < 0 || type != PRIME_CHUNK_COMMON) {
		ephemeral_chunk_cleanup(ephemeral);
		keks_free(keks);
		return NULL;
	}

	// Headers chunk.
	data += st_length_get(&chunk[1]);
	remaining -= st_length_get(&chunk[1]);

	if (chunk_header_read(PLACER(data, remaining), &type, &size, &chunk[2]) < 0 || type != PRIME_CHUNK_HEADERS ||
		!(headers = encrypted_chunk_set(keys.signing, keks, &chunk[2], NULL))) {
		ephemeral_chunk_cleanup(ephemeral);
		keks_free(keks);
		return NULL;
	}

	// Body chunk.
	data += st_length_get(&chunk[2]);
	remaining -= st_length_get(&chunk[2]);

	if (chunk_header_read(PLACER(data, remaining), &type, &size, &chunk[3]) < 0 || type != PRIME_CHUNK_BODY ||
		!(body = encrypted_chunk_set(keys.signing, keks, &chunk[3], NULL))) {
		ephemeral_chunk_cleanup(ephemeral);
		st_free(headers);
		keks_free(keks);
		return NULL;
	}

	// Tree signature.
	data += st_length_get(&chunk[3]);
	remaining -= st_length_get(&chunk[3]);

	if (chunk_header_read(PLACER(data, remaining), &type, &size, &chunk[4]) < 0 || type != PRIME_SIGNATURE_TREE ||
		!(tree = signature_tree_alloc())) {
		ephemeral_chunk_cleanup(ephemeral);
		st_free(headers);
		keks_free(keks);
		st_free(body);
		return NULL;
	}

	signature_tree_add(tree, &chunk[0]);
	signature_tree_add(tree, &chunk[1]);
	signature_tree_add(tree, &chunk[2]);
	signature_tree_add(tree, &chunk[3]);

	if (signature_tree_verify(keys.signing, tree, keks, &chunk[4])) {
		ephemeral_chunk_cleanup(ephemeral);
		signature_tree_free(tree);
		st_free(headers);
		keks_free(keks);
		st_free(body);
		return NULL;
	}

	signature_tree_free(tree);

	// User signature.
	data += st_length_get(&chunk[4]);
	remaining -= st_length_get(&chunk[4]);

	if (chunk_header_read(PLACER(data, remaining), &type, &size, &chunk[5]) < 0 || type != PRIME_SIGNATURE_USER ||
		signature_full_verify(keys.signing, keks, PLACER(st_data_get(message) + 6, st_length_get(message) - 6 - remaining), &chunk[5])) {
		ephemeral_chunk_cleanup(ephemeral);
		st_free(headers);
		keks_free(keks);
		st_free(body);
		return NULL;
	}

	// Org signature.
	data += st_length_get(&chunk[5]);
	remaining -= st_length_get(&chunk[5]);

	if (chunk_header_read(PLACER(data, remaining), &type, &size, &chunk[6]) < 0 || type != PRIME_SIGNATURE_DESTINATION ||
		signature_full_verify(org->signing, keks, PLACER(st_data_get(message) + 6, st_length_get(message) - 6 - remaining), &chunk[6])) {
		ephemeral_chunk_cleanup(ephemeral);
		st_free(headers);
		keks_free(keks);
		st_free(body);
		return NULL;
	}

	result = st_merge("ss", headers, body);
	ephemeral_chunk_cleanup(ephemeral);
	st_free(headers);
	keks_free(keks);
	st_free(body);

	return result;
}

prime_message_t * naked_message_set(stringer_t *message, prime_org_key_t *destination, prime_user_signet_t *recipient) {

	size_t length = 0;
	uint32_t big_endian_size = 0;
	prime_message_t *result = NULL;
	prime_signature_tree_t *tree = NULL;
	stringer_t *common = NULL, *holder[10];
	placer_t header = pl_null(), body = pl_null();
	uint16_t type = htobe16(PRIME_MESSAGE_NAKED);

	if (!message || !destination || !recipient) {
		return NULL;
	}
	else if (!(result = encrypted_message_alloc())) {
		return NULL;
	}
	else if (!(result->keys.signing = ed25519_generate()) || !(result->keys.encryption = secp256k1_generate())) {
		encrypted_message_free(result);
		return NULL;
	}
	else if (!(result->envelope.ephemeral = ephemeral_chunk_get(result->keys.signing, result->keys.encryption))) {
		encrypted_message_free(result);
		return NULL;// Generate the signatures.
	if (!(tree = signature_tree_alloc())) {
		encrypted_message_free(result);
		return NULL;
	}

	signature_tree_add(tree, ephemeral_chunk_buffer(result->envelope.ephemeral));
	signature_tree_add(tree, encrypted_chunk_buffer(result->metadata.common));
	signature_tree_add(tree, encrypted_chunk_buffer(result->metadata.headers));
	signature_tree_add(tree, encrypted_chunk_buffer(result->content.body));
	}

	result->keys.signing = ed25519_private_set(ed25519_private_get(result->keys.signing, MANAGEDBUF(32)));
	result->keys.destination = secp256k1_public_set(secp256k1_public_get(destination->encryption, MANAGEDBUF(33)));
	result->keys.recipient = secp256k1_public_set(secp256k1_public_get(recipient->encryption, MANAGEDBUF(33)));

	keks_get(&(result->keys), &(result->keks));

	length = mail_header_end(message);
	header = pl_init(st_data_get(message), length);
	body = pl_init(st_data_get(message) + length, st_length_get(message) - length);

	holder[0] = mail_header_fetch_cleaned(&header, PLACER("Date", 4));
	holder[1] = mail_header_fetch_cleaned(&header, PLACER("Subject", 7));
	holder[2] = mail_header_fetch_cleaned(&header, PLACER("From", 4));
	holder[3] = mail_header_fetch_cleaned(&header, PLACER("Sender", 6));
	holder[4] = mail_header_fetch_cleaned(&header, PLACER("Reply-To", 8));
	holder[5] = mail_header_fetch_cleaned(&header, PLACER("To", 2));
	holder[6] = mail_header_fetch_cleaned(&header, PLACER("Cc", 2));
	holder[7] = mail_header_fetch_cleaned(&header, PLACER("Bcc", 3));
	holder[8] = mail_header_fetch_cleaned(&header, PLACER("In-Reply-To", 11));
	holder[9] = mail_header_fetch_cleaned(&header, PLACER("Message-ID", 10));

	/// LOW: An effective, albeit kludgey, logic to ensure common headers are formatted correctly, and the values reside on a single line.
	common = st_merge("nsnnsnnsnnsnnsnnsnnsnnsnnsnnsn",
		(holder[0] ? "Date: " : ""), holder[0], (holder[0] ? "\n" : ""),
		(holder[1] ? "Subject: " : ""), holder[1], (holder[1] ? "\n" : ""),
		(holder[2] ? "From: " : ""), holder[2], (holder[2] ? "\encrypted_chunk_buffer(n" : ""),
		(holder[3] ? "Sender: " : ""), holder[3], (holder[3] ? "\n" : ""),
		(holder[4] ? "Reply-To: " : ""), holder[4], (holder[4] ? "\n" : ""),
		(holder[5] ? "To: " : ""), holder[5], (holder[5] ? "\n" : ""),
		(holder[6] ? "Cc: " : ""), holder[6], (holder[6] ? "\n" : ""),
		(holder[7] ? "Bcc: " : ""), holder[7], (holder[7] ? "\n" : ""),
		(holder[8] ? "In-Reply-To: " : ""), holder[8], (holder[8] ? "\n" : ""),
		(holder[9] ? "Message-ID: " : ""), holder[9], (holder[9] ? "\n" : ""));

	st_cleanup(holder[0], holder[1], holder[2], holder[3], holder[4], holder[5], holder[6], holder[7], holder[8], holder[9]);

	if (!(result->metadata.common = encrypted_chunk_get(PRIME_CHUNK_COMMON, result->keys.signing, &(result->keks), common))) {
		encrypted_message_free(result);
		st_cleanup(common);
		return NULL;
	}

	st_cleanup(common);

	// Encrypt the headers, and the body.
	if (!(result->metadata.headers = encrypted_chunk_get(PRIME_CHUNK_HEADERS, result->keys.signing, &(result->keks), &header))) {
		encrypted_message_free(result);
		return NULL;
	}
	else if (!(result->content.body = encrypted_chunk_get(PRIME_CHUNK_BODY, result->keys.signing, &(result->keks), &body))) {
		encrypted_message_free(result);
		return NULL;
	}

	// Generate the signatures.
	if (!(tree = signature_tree_alloc())) {
		encrypted_message_free(result);
		return NULL;
	}

	signature_tree_add(tree, ephemeral_chunk_buffer(result->envelope.ephemeral));
	signature_tree_add(tree, encrypted_chunk_buffer(result->metadata.common));
	signature_tree_add(tree, encrypted_chunk_buffer(result->metadata.headers));
	signature_tree_add(tree, encrypted_chunk_buffer(result->content.body));

	// Calculate the tree signature.
	result->signatures.tree = signature_tree_get(result->keys.signing, tree, &(result->keks));
	signature_tree_free(tree);

	if (st_length_get(result->signatures.tree) != 161) {
		encrypted_message_free(result);
		return NULL;
	}

	length = st_write(NULL, PLACER(&type, 2), PLACER(&big_endian_size, 4),
		ephemeral_chunk_buffer(result->envelope.ephemeral), encrypted_chunk_buffer(result->metadata.common),
		encrypted_chunk_buffer(result->metadata.headers), encrypted_chunk_buffer(result->content.body), result->signatures.tree);

	if (!(result->encrypted = st_alloc_opts(MANAGED_T | JOINTED | HEAP, length + 512)) ||
		st_write(result->encrypted, PLACER(&type, 2), PLACER(&big_endian_size, 4),
		ephemeral_chunk_buffer(result->envelope.ephemeral),	encrypted_chunk_buffer(result->metadata.common),
		encrypted_chunk_buffer(result->metadata.headers), encrypted_chunk_buffer(result->content.body),
		result->signatures.tree) != length) {

		encrypted_message_free(result);
		return NULL;
	}

	else if (!(result->signatures.user = signature_full_get(PRIME_SIGNATURE_USER, result->keys.signing, &(result->keks),
		PLACER(st_data_get(result->encrypted) + 6, st_length_get(result->encrypted) - 6))) || st_length_get(result->signatures.user) != 161 ||
		!st_append(result->encrypted, result->signatures.user)) {
		encrypted_message_free(result);
		return NULL;
	}

	else if (!(result->signatures.org = signature_full_get(PRIME_SIGNATURE_DESTINATION, destination->signing, &(result->keks),
		PLACER(st_data_get(result->encrypted) + 6, st_length_get(result->encrypted) - 6))) || st_length_get(result->signatures.org) != 129 ||
		!st_append(result->encrypted, result->signatures.org)) {
		encrypted_message_free(result);
		return NULL;
	}

	// Store the big endian size.
	big_endian_size = htobe32(st_length_get(result->encrypted) - 6);
	mm_copy(st_data_get(result->encrypted) + 2, &big_endian_size, 4);

	return result;
}
