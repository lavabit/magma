
/**
 * @file /magma/src/providers/prime/prime.h
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_PROVIDERS_PRIME_H
#define MAGMA_PROVIDERS_PRIME_H

extern EC_GROUP *prime_curve_group;

/// prime.c
bool_t   prime_start(void);
void     prime_stop(void);

//// secp256k1.c
EC_KEY *      secp256k1_alloc(void);
stringer_t *  secp256k1_compute_kek(EC_KEY *private, EC_KEY *public, stringer_t *output);
void          secp256k1_free(EC_KEY *key);
EC_KEY *      secp256k1_generate(void);
stringer_t *  secp256k1_private_get(EC_KEY *key, stringer_t *output);
EC_KEY *      secp256k1_private_set(stringer_t *key);
stringer_t *  secp256k1_public_get(EC_KEY *key, stringer_t *output);
EC_KEY *      secp256k1_public_set(stringer_t *key);

#endif

