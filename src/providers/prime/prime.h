
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
	BINARY,                              /**< Serialized object in binary form. >*/
	ARMORED                              /**< Armor the object using the Privacy Enhanced Message format. >*/
} prime_encoding_t;

typedef enum {
	NONE,
	SECURITY                             /**< Store the object in secure memory. >*/
} prime_flags_t;

typedef enum {
    PRIME_ORG_SIGNET = 1776,             /**< Organizational signet. >*/
	PRIME_ORG_KEY = 1952,                /**< Organizational key. >*/
	PRIME_ORG_KEY_ENCRYPTED = 1947,      /**< Encrypted organizational key. >*/

	PRIME_USER_SIGNING_REQUEST = 1215,    /**< User signing request. >*/
    PRIME_USER_SIGNET = 1789,             /**< User signet. >*/
	PRIME_USER_KEY = 2013,                /**< User key. >*/
	PRIME_USER_KEY_ENCRYPTED = 1976,      /**< Encrypted user key. >*/

    PRIME_MESSAGE_ENCRYPTED = 1847        /**< Encrypted message. >*/
} prime_type_t;

// This allows code to include the PRIME header without first including the OpenSSL headers.
#ifdef HEADER_EC_H
typedef EC_KEY secp256k1_key_t;
#else
typedef void secp256k1_key_t;
#endif

typedef struct __attribute__ ((packed)) {
	ed25519_key_type_t type;
	struct __attribute__ ((packed)) {
		uint8_t private[ED25519_KEY_PRIV_LEN];
		uint8_t public[ED25519_KEY_PUB_LEN];
	};
} ed25519_key_t;

typedef struct __attribute__ ((packed)) {
	ed25519_key_t *signing;
	secp256k1_key_t *encryption;
} prime_user_key_t;

typedef struct __attribute__ ((packed)) {
	ed25519_key_t *signing;
	secp256k1_key_t *encryption;
} prime_org_key_t;

typedef struct __attribute__ ((packed)) {
	ed25519_key_t *signing;              /**< User signing key, field 1. >*/
	secp256k1_key_t *encryption;         /**< User encryption key, field 2. >*/

	struct {
		stringer_t *custody;             /**< User chain of custody signature, field 4. >*/
		stringer_t *user;                /**< User signature, field 5. >*/
		stringer_t *org;                 /**< Organizational signature, field 6. >*/
	} signatures;
} prime_user_signet_t;

typedef struct __attribute__ ((packed)) {
	ed25519_key_t *signing;              /**< Organizational signing key, field 1. >*/
	secp256k1_key_t *encryption;         /**< Organizational encryption key, field 3. >*/
	stringer_t *signature;               /**< Organizational signature, field 4. >*/
} prime_org_signet_t;

typedef struct __attribute__ ((packed)) {
	prime_type_t type;
	prime_flags_t flags;
	struct {
		union {
			prime_org_key_t *org;
			prime_user_key_t *user;
		};
	} key;

	struct {
		union {
			prime_org_signet_t *org;
			prime_user_signet_t *user;
		};
	} signet;
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
prime_t *     prime_key_generate(prime_type_t type, prime_flags_t flags);
prime_t *     prime_request_generate(prime_t *object, prime_t *previous);
prime_t *     prime_request_sign(prime_t *request, prime_t *org);
prime_t *     prime_set(stringer_t *object, prime_encoding_t encoding, prime_flags_t flags);
prime_t *     prime_signet_generate(prime_t *object);
bool_t        prime_start(void);
void          prime_stop(void);

#endif

