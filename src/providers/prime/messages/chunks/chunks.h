
/**
 * @file /magma/src/providers/prime/messages/chunks/chunks.h
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef PRIME_CHUNKS_H
#define PRIME_CHUNKS_H

/// chunks.c
int32_t                  chunk_header_size(stringer_t *chunk);
prime_message_chunk_type_t   chunk_header_type(stringer_t *chunk);
stringer_t *             chunk_header_write(prime_message_chunk_type_t type, size_t size, stringer_t *output);

/// keks.c
prime_chunk_keks_t *  keks_alloc(void);
void                  keks_cleanup(prime_chunk_keks_t *keks);
void                  keks_free(prime_chunk_keks_t *keks);
prime_chunk_keks_t *  keks_get(prime_chunk_keys_t *keys);
prime_chunk_keks_t *  keks_set(prime_chunk_keys_t *keys);

/// slots.c
int_t                  slots_actors(prime_message_chunk_type_t type);
prime_chunk_slots_t *  slots_alloc(prime_message_chunk_type_t type);
placer_t               slots_buffer(prime_chunk_slots_t *slots);
void                   slots_cleanup(prime_chunk_slots_t *slots);
int_t                  slots_count(prime_message_chunk_type_t type);
void                   slots_free(prime_chunk_slots_t *slots);
stringer_t *           slots_get(prime_message_chunk_type_t type, stringer_t *slots, prime_chunk_keks_t *keks, stringer_t *output);
stringer_t *           slots_key(prime_chunk_slots_t *slots, prime_chunk_keks_t *keks, stringer_t *output);
prime_chunk_slots_t *  slots_set(prime_message_chunk_type_t type, stringer_t *key, prime_chunk_keks_t *keks);

/// signature.c
prime_signature_chunk_t *  signature_chunk_alloc(void);
void                       signature_chunk_cleanup(prime_signature_chunk_t *chunk);
void                       signature_chunk_free(prime_signature_chunk_t *chunk);
stringer_t *               signature_chunk_full_get(prime_message_chunk_type_t type, ed25519_key_t *signing, prime_chunk_keks_t *keks, stringer_t *data);
stringer_t *               signature_chunk_tree_get(ed25519_key_t *signing, prime_signature_chunk_t *chunk, prime_chunk_keks_t *keks);
int_t                      signature_chunk_tree_add(prime_signature_chunk_t *chunk, stringer_t *data);

/// ephemeral.c
prime_ephemeral_chunk_t *  ephemeral_chunk_alloc(void);
stringer_t *               ephemeral_chunk_buffer(prime_ephemeral_chunk_t *chunk);
void                       ephemeral_chunk_cleanup(prime_ephemeral_chunk_t *chunk);
void                       ephemeral_chunk_free(prime_ephemeral_chunk_t *chunk);
prime_ephemeral_chunk_t *  ephemeral_chunk_get(ed25519_key_t *signing, secp256k1_key_t *encryption);
prime_ephemeral_chunk_t *  ephemeral_chunk_set(stringer_t *chunk);

/// encrypted.c
prime_encrypted_chunk_t *  encrypted_chunk_alloc(void);
stringer_t *               encrypted_chunk_buffer(prime_encrypted_chunk_t *chunk);
void                       encrypted_chunk_cleanup(prime_encrypted_chunk_t *chunk);
void                       encrypted_chunk_free(prime_encrypted_chunk_t *chunk);
prime_encrypted_chunk_t *  encrypted_chunk_get(prime_message_chunk_type_t type, ed25519_key_t *signing, prime_chunk_keks_t *keks, stringer_t *data);
stringer_t *               encrypted_chunk_set(ed25519_key_t *signing, prime_chunk_keks_t *keks, stringer_t *chunk, stringer_t *output);

#endif

