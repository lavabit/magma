
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

/// aes.c
placer_t      aes_cipher_key(stringer_t *key);
placer_t      aes_tag_shard(stringer_t *key);
placer_t      aes_vector_shard(stringer_t *key);
stringer_t *  aes_chunk_decrypt(stringer_t *key, stringer_t *chunk, stringer_t *output);
stringer_t *  aes_chunk_encrypt(uint8_t type, stringer_t *key, stringer_t *chunk, stringer_t *output);
stringer_t *  aes_artifact_decrypt(stringer_t *key, stringer_t *object, stringer_t *output);
stringer_t *  aes_artifact_encrypt(stringer_t *key, stringer_t *object, stringer_t *output);

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
stringer_t *       secp256k1_compute_kek(secp256k1_key_t *priv, secp256k1_key_t *pub, stringer_t *output);
void               secp256k1_free(secp256k1_key_t *key);
secp256k1_key_t *  secp256k1_generate(void);
stringer_t *       secp256k1_private_get(secp256k1_key_t *key, stringer_t *output);
secp256k1_key_t *  secp256k1_private_set(stringer_t *key);
stringer_t *       secp256k1_public_get(secp256k1_key_t *key, stringer_t *output);
secp256k1_key_t *  secp256k1_public_set(stringer_t *key);

#endif
