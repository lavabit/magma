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

ed25519_key_t * ed25519_key_generate(void) {

	ed25519_key_t *result = NULL;
	uint8_t pub[ED25519_KEY_PUB_LEN], priv[ED25519_KEY_PRIV_LEN];

	ED25519_keypair_d(pub, priv);

	if (!(result = mm_alloc(sizeof(ed25519_key_t)))) {
		log_pedantic("Failed to allocate memory for an ed25519_key_t.");
		return NULL;
	}

	result->type = ED25519_PRIV;
	memcpy(result->private, priv, 32);
	memcpy(result->public, pub, 32);
	return result;
}

stringer_t *ed25519_private_get(ed25519_key_t *key, stringer_t *output) {

	stringer_t *result = NULL;

	if (!key || key->type != ED25519_PRIV) {
		log_pedantic("An invalid ED25519 private key was supplied.");
		return NULL;
	}
	// See if we have a valid output buffer, or if output is NULL, allocate a buffer to hold the output.
	else if (output && (!st_valid_destination(st_opt_get(output)) || st_avail_get(output) < ED25519_KEY_PRIV_LEN)) {
		log_pedantic("An output string was supplied but it does not represent a buffer capable of holding the output.");
		return NULL;
	}
	else if (!output && !(result = st_alloc(ED25519_KEY_PRIV_LEN))) {
		log_pedantic("Could not allocate a buffer large enough to hold encoded result. { requested = %i }", ED25519_KEY_PRIV_LEN);
		return NULL;
	}
	else if (result) {
		output = result;
	}

	// Wipe the buffer so any leading bytes we don't use will be zero'ed out for padding purposes.
	st_wipe(output);

	// Get the secret component.
	mm_copy(st_data_get(output), key->private, ED25519_KEY_PRIV_LEN);
	st_length_set(output, ED25519_KEY_PRIV_LEN);
	return output;
}
