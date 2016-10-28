
/**
 * @file /magma/src/providers/prime/keys/keys.h
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_PRIME_KEYS_H
#define MAGMA_PRIME_KEYS_H

/// users.c
prime_user_key_t *  user_key_alloc(void);
void                user_key_free(prime_user_key_t *user);
prime_user_key_t *  user_key_generate(void);
size_t              user_key_get(prime_user_key_t *user, stringer_t *output);
size_t              user_key_length(prime_user_key_t *user);

/// orgs.c
prime_org_key_t *  org_key_alloc(void);
void               org_key_free(prime_org_key_t *org);
prime_org_key_t *  org_key_generate(void);
size_t             org_key_get(prime_org_key_t *org, stringer_t *output);
size_t             org_key_length(prime_org_key_t *org);

#endif

