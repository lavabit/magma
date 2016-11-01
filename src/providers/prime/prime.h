
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


#include "cryptography/cryptography.h"
#include "primitives/primitives.h"
#include "signets/signets.h"
#include "keys/keys.h"

/// prime.c
bool_t   prime_start(void);
void     prime_stop(void);

#endif

