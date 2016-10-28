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
