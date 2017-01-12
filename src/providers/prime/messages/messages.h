
/**
 * @file /magma/src/providers/prime/messages/messages.h
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef PRIME_MESSAGES_H
#define PRIME_MESSAGES_H

/// messages.c
prime_message_t *  encrypted_message_alloc(void);
void               encrypted_message_cleanup(prime_message_t *object);
void               encrypted_message_free(prime_message_t *object);

#include "chunks/chunks.h"

#endif

