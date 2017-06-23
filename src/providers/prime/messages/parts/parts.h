
/**
 * @file /magma/src/providers/prime/messages/parts/parts.h
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 */

#ifndef PRIME_PARTS_H
#define PRIME_PARTS_H

/// parts.c
stringer_t *               part_buffer(prime_encrypted_chunk_t *chunk);
stringer_t *               part_decrypt(ed25519_key_t *signing, prime_chunk_keks_t *keks, stringer_t *part, stringer_t *output);
prime_encrypted_chunk_t *  part_encrypt(prime_message_chunk_type_t type, ed25519_key_t *signing, prime_chunk_keks_t *keks, stringer_t *payload);

#endif

