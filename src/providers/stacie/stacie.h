
/**
 * @file /magma/src/providers/stacie/stacie.h
 *
 * @brief These functions implement the Safely Turning Authentication Credentials Into Entropy (STACIE)
 *			standard. This standard defines how process passwords into encryption keys, and/or authentication
 * 			tokens, derive realm specific encryption keys, and perform symmetric encryption using the realm keys.
 * 			The inputs passed into these functions must be santized, and normalized to ensure a deterministic output.
 *
 * @see	https://tools.ietf.org/html/draft-ladar-stacie
 */

#ifndef STACIE_H
#define STACIE_H

// The STACIE number constants for clamping hash rounds between 8 and 16,777,216, which represents the number of possible
// values for an unsigned 24 bit integer, if you include 0. In other words 0 to 16,777,215 equals 16,777,216.
#define STACIE_KEY_ROUNDS_MIN		8
#define STACIE_KEY_ROUNDS_MAX		16777216

// The STACIE token derivation stage uses a fixed number of hash rounds, and that number is dictated by this parameter.
#define STACIE_TOKEN_ROUNDS			8

// This STACIE implementation will always use salt and nonce values which are 128 bytes in length.
#define STACIE_SALT_LENGTH		128
#define STACIE_NONCE_LENGTH		128

// This STACIE implementation uses SHA-2/512 resulting in key, token, and shard lengths of 64 bytes.
#define STACIE_KEY_LENGTH		64
#define STACIE_TOKEN_LENGTH		64
#define STACIE_SHARD_LENGTH		64

// This STACIE implementation only supports realm encryption of buffers up to 16,777,215 bytews in length.
#define STACIE_ENCRYPT_MIN		1
#define STACIE_ENCRYPT_MAX		16777215
#define STACIE_BLOCK_LENGTH		16
#define STACIE_ENVELOPE_LENGTH	34

/// realms.c
stringer_t *  stacie_realm_cipher(stringer_t *realm_key);
stringer_t *  stacie_realm_key(stringer_t *master_key, stringer_t *realm,  stringer_t *salt, stringer_t *shard);
stringer_t *  stacie_realm_tag(stringer_t *realm_key);
stringer_t *  stacie_realm_vector(stringer_t *realm_key);

/// creation.c
stringer_t *  stacie_create_nonce(stringer_t *output);
stringer_t *  stacie_create_salt(stringer_t *output);
stringer_t *  stacie_create_shard(stringer_t *output);

/// passwords.c
stringer_t *  stacie_derive_seed(uint32_t rounds, stringer_t *password, stringer_t *salt);
stringer_t *  stacie_derive_key(stringer_t *base, uint32_t rounds, stringer_t *username, stringer_t *password, stringer_t *salt);
uint32_t      stacie_derive_rounds(stringer_t *password, uint32_t bonus);

/// tokens.c
stringer_t *  stacie_derive_token(stringer_t *base, stringer_t *username, stringer_t *salt, stringer_t *nonce);

/// crypto.c
stringer_t *  stacie_decrypt(stringer_t *vector_key, stringer_t *tag_key, stringer_t *cipher_key, stringer_t *buffer);
stringer_t *  stacie_encrypt(uint16_t serial, stringer_t *vector_key, stringer_t *tag_key, stringer_t *cipher_key, stringer_t *buffer);

#endif

