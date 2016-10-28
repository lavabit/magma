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

	uint8_t pub[32], priv[64];
	ed25519_key_t *result = NULL;

	ED25519_keypair_d(pub, priv);

	if (!(result = mm_alloc(sizeof(ed25519_key_t)))) {
		log_pedantic("Failed to allocate memory for an ed25519_key_t.");
		return NULL;
	}

	memcpy(result->private, priv, 32);
	memcpy(result->public, pub, 32);
	return result;
}
