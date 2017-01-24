
/**
 * @file /magma/providers/prime/signets/signets.h
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 */

#ifndef PRIME_SIGNETS_H
#define PRIME_SIGNETS_H

/// orgs.c
prime_org_signet_t *  org_signet_alloc(void);
stringer_t *          org_signet_fingerprint(prime_org_signet_t *org, stringer_t *output);
void                  org_signet_free(prime_org_signet_t *org);
prime_org_signet_t *  org_signet_generate(prime_org_key_t *org);
stringer_t *          org_signet_get(prime_org_signet_t *org, stringer_t *output);
size_t                org_signet_length(prime_org_signet_t *org);
prime_org_signet_t *  org_signet_set(stringer_t *org);
bool_t                org_signet_verify(prime_org_signet_t *org);

/// users.c
prime_user_signet_t *  user_signet_alloc(void);
stringer_t *           user_signet_fingerprint(prime_user_signet_t *user, stringer_t *output);
void                   user_signet_free(prime_user_signet_t *user);
stringer_t *           user_signet_get(prime_user_signet_t *user, stringer_t *output);
size_t                 user_signet_length(prime_user_signet_t *user);
prime_user_signet_t *  user_signet_set(stringer_t *user);
bool_t                 user_signet_verify_chain_of_custody(prime_user_signet_t *user, prime_user_signet_t *previous);
bool_t                 user_signet_verify_org(prime_user_signet_t *user, prime_org_signet_t *org);
bool_t                 user_signet_verify_self(prime_user_signet_t *user);

/// requests.c
prime_user_signet_t *  user_request_generate(prime_user_key_t *user);
stringer_t *           user_request_get(prime_user_signet_t *user, stringer_t *output);
size_t                 user_request_length(prime_user_signet_t *user);
prime_user_signet_t *  user_request_rotation(prime_user_key_t *user, prime_user_key_t *previous);
prime_user_signet_t *  user_request_set(stringer_t *user);
prime_user_signet_t *  user_request_sign(prime_user_signet_t *request, prime_org_key_t *org);
bool_t                 user_request_verify_chain_of_custody(prime_user_signet_t *user, prime_user_signet_t *previous);
bool_t                 user_request_verify_self(prime_user_signet_t *user);

#endif

