
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

/// messages.c
prime_message_t *  encrypted_message_alloc(void);
void               encrypted_message_cleanup(prime_message_t *object);
void               encrypted_message_free(prime_message_t *object);

/// signature.c
prime_signature_chunk_t *  signature_chunk_alloc(void);
void                       signature_chunk_cleanup(prime_signature_chunk_t *chunk);
void                       signature_chunk_free(prime_signature_chunk_t *chunk);

/// ephemeral.c
prime_ephemeral_chunk_t *  ephemeral_chunk_alloc(void);
void                       ephemeral_chunk_cleanup(prime_ephemeral_chunk_t *chunk);
void                       ephemeral_chunk_free(prime_ephemeral_chunk_t *chunk);

/// encrypted.c
prime_encrypted_chunk_t *  encrypted_chunk_alloc(void);
void                       encrypted_chunk_cleanup(prime_encrypted_chunk_t *chunk);
void                       encrypted_chunk_free(prime_encrypted_chunk_t *chunk);

#endif

