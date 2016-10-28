
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

#define SECP256K1_KEY_PUB_LEN 33
#define SECP256K1_KEY_PRIV_LEN 32
#define SECP256K1_SHARED_SECRET_LEN 32

#define ED25519_KEY_PUB_LEN 32
#define ED25519_KEY_PRIV_LEN 32
#define ED25519_SIGNATURE_LEN 64

// This allows code to include the PRIME header without first including the OpenSSL headers.
#ifdef HEADER_EC_H
typedef EC_KEY secp256k1_key_t;
#else
typedef void * secp256k1_key_t;
#endif

/// prime.c
bool_t   prime_start(void);
void     prime_stop(void);

/// secp256k1.c
secp256k1_key_t *  secp256k1_alloc(void);
stringer_t *       secp256k1_compute_kek(secp256k1_key_t *private, secp256k1_key_t *public, stringer_t *output);
void               secp256k1_free(secp256k1_key_t *key);
secp256k1_key_t *  secp256k1_generate(void);
stringer_t *       secp256k1_private_get(secp256k1_key_t *key, stringer_t *output);
secp256k1_key_t *  secp256k1_private_set(stringer_t *key);
stringer_t *       secp256k1_public_get(secp256k1_key_t *key, stringer_t *output);
secp256k1_key_t *  secp256k1_public_set(stringer_t *key);

#endif

