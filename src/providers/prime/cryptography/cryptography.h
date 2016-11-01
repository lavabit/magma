
/**
 * @file /magma/src/providers/prime/cryptography/cryptography.h
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef PRIME_CRYPTOGRAPHY_H
#define PRIME_CRYPTOGRAPHY_H

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
typedef void secp256k1_key_t;
#endif

typedef enum {
	ED25519_PUB,
	ED25519_PRIV
} ed25519_key_type_t;

typedef struct  __attribute__ ((packed)) {

	ed25519_key_type_t type;
	struct __attribute__ ((packed)) {
		uint8_t private[ED25519_KEY_PRIV_LEN];
		uint8_t public[ED25519_KEY_PUB_LEN];
	};
} ed25519_key_t;

/// ed25519.c
ed25519_key_t *  ed25519_alloc(void);
void             ed25519_free(ed25519_key_t *key);
ed25519_key_t *  ed25519_generate(void);
stringer_t *     ed25519_private_get(ed25519_key_t *key, stringer_t *output);
ed25519_key_t *  ed25519_private_set(stringer_t *key);
stringer_t *     ed25519_public_get(ed25519_key_t *key, stringer_t *output);
ed25519_key_t *  ed25519_public_set(stringer_t *key);
stringer_t *     ed25519_sign(ed25519_key_t *key, stringer_t *data, stringer_t *output);
int_t            ed25519_verify(ed25519_key_t *key, stringer_t *data, stringer_t *signature);

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
