
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

#ifndef PRIME_H
#define PRIME_H

#define SECP256K1_KEY_PUB_LEN 33
#define SECP256K1_KEY_PRIV_LEN 32
#define SECP256K1_SHARED_SECRET_LEN 32

#define ED25519_KEY_PUB_LEN 32
#define ED25519_KEY_PRIV_LEN 32
#define ED25519_SIGNATURE_LEN 64

#define AES_TAG_LEN 16
#define AES_KEY_LEN 32
#define AES_BLOCK_LEN 16
#define AES_VECTOR_LEN 16

typedef enum {
	ED25519_PUB,
	ED25519_PRIV
} ed25519_key_type_t;

typedef enum {
	BINARY,
	ARMORED
} prime_encoding_t;

typedef enum {
	NONE,
	SECURITY
} prime_flags_t;

typedef enum {
    PRIME_ORG_SIGNET = 1776,             /**< File contains an organizational signet */
	PRIME_ORG_KEY = 1952,               /**< File contains organizational keys*/
	PRIME_ORG_KEY_ENCRYPTED = 1947,     /**< File contains an encrypted organizational key. */

	PRIME_USER_SIGNING_REQUEST = 1215,    /**< File contains an ssr*/
    PRIME_USER_SIGNET = 1789,            /**< File contains a user signet */
	PRIME_USER_KEY = 2013,              /**< File contains user keys*/
	PRIME_USER_KEY_ENCRYPTED = 1976,    /**< File contains an encrypted user key. */

    PRIME_MESSAGE_ENCRYPTED = 1847
} prime_type_t;

// This allows code to include the PRIME header without first including the OpenSSL headers.
#ifdef HEADER_EC_H
typedef EC_KEY secp256k1_key_t;
#else
typedef void secp256k1_key_t;
#endif

typedef struct  __attribute__ ((packed)) {
	ed25519_key_type_t type;
	struct __attribute__ ((packed)) {
		uint8_t private[ED25519_KEY_PRIV_LEN];
		uint8_t public[ED25519_KEY_PUB_LEN];
	};
} ed25519_key_t;

typedef struct {
	ed25519_key_t *signing;
	secp256k1_key_t *encryption;
} prime_user_key_t;

typedef struct {
	ed25519_key_t *signing;
	secp256k1_key_t *encryption;
} prime_org_key_t;

typedef struct {
	prime_type_t type;
	prime_flags_t flags;
	union {
		prime_org_key_t *org;
		prime_user_key_t *user;
	};
} prime_t;

#include "cryptography/cryptography.h"
#include "formats/formats.h"
#include "signets/signets.h"
#include "keys/keys.h"

/// prime.c
prime_t *     prime_alloc(prime_type_t type, prime_flags_t flags);
void          prime_cleanup(prime_t *object);
void          prime_free(prime_t *object);
stringer_t *  prime_get(prime_t *object, prime_encoding_t encoding, stringer_t *output);
prime_t *     prime_key_decrypt(stringer_t *key, stringer_t *object, prime_encoding_t encoding, prime_flags_t flags);
stringer_t *  prime_key_encrypt(stringer_t *key, prime_t *object, prime_encoding_t encoding, stringer_t *output);
prime_t *     prime_key_generate(prime_type_t type);
prime_t *     prime_set(stringer_t *object, prime_encoding_t encoding, prime_flags_t flags);
bool_t        prime_start(void);
void          prime_stop(void);

#endif

