
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

/// signature.c
prime_signature_chunk_t *  signature_chunk_alloc(void);
void                       signature_chunk_cleanup(prime_signature_chunk_t *chunk);
void                       signature_chunk_free(prime_signature_chunk_t *chunk);

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
prime_encrypted_chunk_t *  encrypted_chunk_get(prime_message_chunk_type_t type, ed25519_key_t *signing, secp256k1_key_t *encryption,
	secp256k1_key_t *author, secp256k1_key_t *origin, secp256k1_key_t *destination, secp256k1_key_t *recipient, stringer_t *data);
prime_encrypted_chunk_t *  encrypted_chunk_set(ed25519_key_t *signing, secp256k1_key_t *encryption, secp256k1_key_t *author,
	secp256k1_key_t *origin, secp256k1_key_t *destination, secp256k1_key_t *recipient, stringer_t *chunk);

/// keyslots.c
prime_chunk_slots_t *  keyslots_alloc(void);
void                   keyslots_cleanup(prime_chunk_slots_t *keyslots);
void                   keyslots_free(prime_chunk_slots_t *keyslots);

#endif

