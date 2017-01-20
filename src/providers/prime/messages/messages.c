
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

stringer_t * naked_message_get(prime_message_t *message) {
	return NULL;
}

prime_message_t * naked_message_set(stringer_t *message, prime_org_key_t *destination, prime_user_signet_t *recipient) {

	size_t length = 0;
	uint32_t big_endian_size = 0;
	prime_message_t *result = NULL;
	prime_signature_tree_t *tree = NULL;
	stringer_t *common = NULL, *holder[10];
	placer_t header = pl_null(), body = pl_null();
	uint16_t type = htobe16(PRIME_MESSAGE_NAKED);

	if (!(result = encrypted_message_alloc())) {
		return NULL;
	}
	else if (!(result->keys.signing = ed25519_generate()) || !(result->keys.encryption = secp256k1_generate())) {
		encrypted_message_free(result);
		return NULL;
	}
	else if (!(result->envelope.ephemeral = ephemeral_chunk_get(result->keys.signing, result->keys.encryption))) {
		encrypted_message_free(result);
		return NULL;
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
		(holder[2] ? "From: " : ""), holder[2], (holder[2] ? "\n" : ""),
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

	else if (!(result->signatures.user = signature_full_get(PRIME_SIGNATURE_USER, result->keys.signing, &(result->keks), result->encrypted)) ||
		st_length_get(result->signatures.user) != 161 || !st_append(result->encrypted, result->signatures.user)) {
		encrypted_message_free(result);
		return NULL;
	}

	else if (!(result->signatures.org = signature_full_get(PRIME_SIGNATURE_DESTINATION, destination->signing, &(result->keks), result->encrypted)) ||
		st_length_get(result->signatures.org) != 129 || !st_append(result->encrypted, result->signatures.org)) {
		encrypted_message_free(result);
		return NULL;
	}

	return result;
}
