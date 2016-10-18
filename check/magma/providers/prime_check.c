
/**
 * @file /magma/check/magma/providers/prime_check.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma_check.h"

#define check_prime_secp256k1_cleanup(key, pub, priv) ({ st_cleanup(pub, priv); secp256k1_free(key); priv = pub = NULL; key = NULL; })

stringer_t * check_prime_secp256k1_sthread(void) {


	uchr_t y;
	EC_KEY *key = NULL;
	stringer_t *priv = NULL, *pub = NULL;

	for (uint64_t i = 0; status() && i < PRIME_CHECK_ITERATIONS; i++) {

		// Generate a new key pair.
		if (!(key = secp256k1_generate())) {
			return st_aprint("Curve secp256k1 key generation failed.");
		}
		// Extract the public and private components.
		else if (!(pub = secp256k1_public_get(key, NULL)) || !(priv = secp256k1_private_get(key, NULL)) ||
			st_length_get(priv) != 32 || st_length_get(pub) != 33) {
			check_prime_secp256k1_cleanup(key, pub, priv);
			return st_aprint("Curve secp256k1 exponent serialization failed.");
		}

		// Confirm the octet stream starts with 0x02 or 0x03, in accordance with the compressed point representation
		// described by ANSI standard X9.62 section 4.3.6.
		y = *((uchr_t *)st_data_get(pub));
		if (y != 2 && y != 3) {
			check_prime_secp256k1_cleanup(key, pub, priv);
			return st_aprint("Curve secp256k1 public key point does not appear to be compressed properly.");
		}

		more checks
	}

	return NULL;
}
