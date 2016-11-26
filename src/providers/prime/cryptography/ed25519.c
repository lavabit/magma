/**
 * @file /magma/src/providers/prime/ed25519.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"
#include <openssl/curve25519.h>

void ed25519_free(ed25519_key_t *key) {

#ifdef MAGMA_PEDANTIC
	if (!key) {
		log_pedantic("Attempted to free a NULL ed25519 key pointer.");
	}
#endif

	if (key) {
		mm_free(key);
	}

	return;
}

ed25519_key_t * ed25519_alloc(void) {

	ed25519_key_t *result = NULL;

	if (!(result = mm_alloc(sizeof(ed25519_key_t)))) {
		log_pedantic("Failed to allocate memory for an ed25519 key structure.");
		return NULL;
	}

	mm_wipe(result, sizeof(ed25519_key_t));

	return result;
}

ed25519_key_t * ed25519_generate(void) {

	ed25519_key_t *result = NULL;
	uint8_t pub[ED25519_KEY_PUB_LEN], priv[ED25519_KEY_PRIV_LEN + ED25519_KEY_PUB_LEN];

	ED25519_keypair_d(pub, priv);

	if (!(result = ed25519_alloc())) {
		return NULL;
	}

	result->type = ED25519_PRIV;
	mm_copy(result->private, priv, ED25519_KEY_PRIV_LEN);
	mm_copy(result->public, pub, ED25519_KEY_PUB_LEN);
	return result;
}

stringer_t *ed25519_public_get(ed25519_key_t *key, stringer_t *output) {

	if (!key || (key->type != ED25519_PRIV && key->type != ED25519_PUB)) {
		log_pedantic("An invalid ed25519 public key was passed in.");
		return NULL;
	}

	// See if we have a valid output buffer, or if output is NULL, allocate a buffer to hold the output.
	if (output && (!st_valid_destination(st_opt_get(output)) || st_avail_get(output) < ED25519_KEY_PUB_LEN)) {
		log_pedantic("An output string was supplied but it does not represent a buffer capable of holding the output.");
		return NULL;
	}
	else if (!output && !(output = st_alloc(ED25519_KEY_PUB_LEN))) {
		log_pedantic("Could not allocate a buffer large enough to hold encoded result. { requested = %i }", ED25519_KEY_PUB_LEN);
		return NULL;
	}

	// Wipe the buffer so any leading bytes we don't use will be zero'ed out for padding purposes.
	st_wipe(output);

	mm_copy(st_data_get(output), key->public, ED25519_KEY_PUB_LEN);
	if (st_valid_tracked(st_opt_get(output))) st_length_set(output, ED25519_KEY_PUB_LEN);
	return output;
}

stringer_t *ed25519_private_get(ed25519_key_t *key, stringer_t *output) {

	if (!key || key->type != ED25519_PRIV) {
		log_pedantic("An invalid ed25519 private key was supplied.");
		return NULL;
	}
	// See if we have a valid output buffer, or if output is NULL, allocate a buffer to hold the output.
	else if (output && (!st_valid_destination(st_opt_get(output)) || st_avail_get(output) < ED25519_KEY_PRIV_LEN)) {
		log_pedantic("An output string was supplied but it does not represent a buffer capable of holding the output.");
		return NULL;
	}
	else if (!output && !(output = st_alloc(ED25519_KEY_PRIV_LEN))) {
		log_pedantic("Could not allocate a buffer large enough to hold encoded result. { requested = %i }", ED25519_KEY_PRIV_LEN);
		return NULL;
	}
	// Wipe the buffer so any leading bytes we don't use will be zero'ed out for padding purposes.
	st_wipe(output);

	// Get the secret component.
	mm_copy(st_data_get(output), key->private, ED25519_KEY_PRIV_LEN);
	if (st_valid_tracked(st_opt_get(output))) st_length_set(output, ED25519_KEY_PRIV_LEN);
	return output;
}

ed25519_key_t * ed25519_public_set(stringer_t *key) {

	ed25519_key_t *output = NULL;

	if (st_empty(key) || st_length_get(key) != ED25519_KEY_PUB_LEN) {
		log_info("An invalid key was passed in.");
		return NULL;
	}
	else if (!(output = ed25519_alloc())) {
		log_info("An error occurred while trying to setup the ed25519 key.");
		return NULL;
	}

	output->type = ED25519_PUB;
	mm_copy(output->public, st_data_get(key), ED25519_KEY_PUB_LEN);
	return output;
}

ed25519_key_t * ed25519_private_set(stringer_t *key) {

	ed25519_key_t *output = NULL;

	if (st_empty(key) || st_length_get(key) != ED25519_KEY_PRIV_LEN) {
		log_info("An invalid key was passed in.");
		return NULL;
	}
	else if (!(output = ed25519_alloc())) {
		log_info("An error occurred while trying to setup the ed25519 key.");
		return NULL;
	}

	output->type = ED25519_PRIV;
	mm_copy(output->private, st_data_get(key), ED25519_KEY_PRIV_LEN);
	ED25519_keypair_from_seed_d(output->public, output->private, output->private);
	return output;
}

stringer_t * ed25519_sign(ed25519_key_t *key, stringer_t *data, stringer_t *output) {

	int res = 0;
	stringer_t *result = NULL;

	if (!key || key->type != ED25519_PRIV) {
		log_pedantic("An invalid ed25519 private key was supplied.");
		return NULL;
	}
	else if (st_empty(data)) {
		log_pedantic("An invalid data buffer was supplied for signing.");
		return NULL;
	}
	// See if we have a valid output buffer, or if output is NULL, allocate a buffer to hold the output.
	else if (output && (!st_valid_destination(st_opt_get(output)) || st_avail_get(output) < ED25519_SIGNATURE_LEN)) {
		log_pedantic("An output string was supplied but it does not represent a buffer capable of holding the output.");
		return NULL;
	}
	else if (!output && !(result = st_alloc(ED25519_SIGNATURE_LEN))) {
		log_pedantic("Could not allocate a buffer large enough to hold the resulting signature. { requested = %i }", ED25519_SIGNATURE_LEN);
		return NULL;
	}
	else if (result) {
		output = result;
	}

	// Generate an ed25519 signature using OpenSSL.
	if ((res = ED25519_sign_d(st_data_get(output), st_data_get(data), st_length_get(data), key->private)) != 1) {
		log_info("An error occurred while trying to compute the signature. { result = %i / curve = ed25519 / error = %s}",
			res, ssl_error_string(MEMORYBUF(256), 256));
		st_cleanup(result);
		return NULL;
	}

	if (st_valid_tracked(st_opt_get(output))) st_length_set(output, ED25519_SIGNATURE_LEN);
	return output;
}

/**
 * @brief			Verify an Ed25519 signature using the EdDSA algorithm.
 * @param key		The public signing key.
 * @param data		The data being verified.
 * @param signature	The signature.
 * @return			0 for sucessful signature verification, -1 for a signature verification failures, -2 for processing or parameter issue.
 */
int_t ed25519_verify(ed25519_key_t *key, stringer_t *data, stringer_t *signature) {

	int res = 0;

	if (!key || (key->type != ED25519_PRIV && key->type != ED25519_PUB)) {
		log_pedantic("An invalid ed25519 public key was supplied.");
		return -2;
	}
	else if (st_empty(signature) || st_length_get(signature) != ED25519_SIGNATURE_LEN) {
		log_pedantic("An invalid signature was supplied for verification. { len = %zu / required = %i }", st_length_get(signature), ED25519_SIGNATURE_LEN);
		return -2;
	}
	else if (st_empty(data)) {
		log_pedantic("An invalid data buffer was supplied for verification.");
		return -2;
	}


	// Verify an ed25519 signature using OpenSSL.
	if ((res = ED25519_verify_d(st_data_get(data), st_length_get(data), st_data_get(signature), key->public)) != 1) {
		log_info("An error occurred while trying to verify the signature. { result = %i / curve = ed25519 / error = %s}",
			res, ssl_error_string(MEMORYBUF(256), 256));
		return -1;
	}

	return 0;
}
