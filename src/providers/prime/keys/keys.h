
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

#ifndef PRIME_KEYS_H
#define PRIME_KEYS_H

typedef struct {
	ed25519_key_t *signing;
	secp256k1_key_t *encryption;
} prime_user_key_t;

typedef struct {
	ed25519_key_t *signing;
	secp256k1_key_t *encryption;
} prime_org_key_t;

typedef struct {
	prime_type_t type;
	union {
		prime_org_key_t *org;
		prime_user_key_t *user;
	};
} prime_key_t;

/// keys.c
prime_key_t *  prime_key_alloc(prime_type_t type);
void           prime_key_free(prime_key_t *key);
prime_key_t *  prime_key_generate(prime_type_t type);
stringer_t *   prime_key_get(prime_key_t *key, stringer_t *output);
prime_key_t *  prime_key_set(stringer_t *key);

/// users.c
prime_user_key_t *  user_key_alloc(void);
void                user_key_free(prime_user_key_t *user);
prime_user_key_t *  user_key_generate(void);
stringer_t *        user_key_get(prime_user_key_t *user, stringer_t *output);
size_t              user_key_length(prime_user_key_t *user);
prime_user_key_t *  user_key_set(stringer_t *key);

/// orgs.c
prime_org_key_t *  org_key_alloc(void);
void               org_key_free(prime_org_key_t *org);
prime_org_key_t *  org_key_generate(void);
stringer_t *       org_key_get(prime_org_key_t *org, stringer_t *output);
size_t             org_key_length(prime_org_key_t *org);
prime_org_key_t *  org_key_set(stringer_t *key);

#endif

