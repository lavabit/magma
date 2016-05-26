
/**
 * @file /magma/src/objects/auth/auth.h
 *
 * @brief The STACIE authentication functions.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_OBJECTS_AUTH_H
#define MAGMA_OBJECTS_AUTH_H

typedef struct {
	stringer_t *key;
	stringer_t *token;
} auth_legacy_t;

typedef struct {

	struct {
		stringer_t *master;
		stringer_t *password;
	} keys;

	struct {
		stringer_t *auth;
		stringer_t *ephemeral;
	} tokens;

} auth_stacie_t;

typedef struct {

	uint64_t usernum;
	stringer_t *username;

	struct {
		uint8_t tls;
		uint8_t locked;
	} status;

	struct {
		uint64_t bonus;
		stringer_t *salt;
	} seasoning;

	struct {
		stringer_t *master;
	} keys;

	struct {
		stringer_t *auth;
	} tokens;

	struct {
		stringer_t *key;
		stringer_t *token;
	} legacy;

} auth_t;

/// username.c
stringer_t *  auth_sanitize_address(stringer_t *username);
stringer_t *  auth_sanitize_username(stringer_t *username);

/// auth.c
auth_t *  auth_alloc(void);
void      auth_free(auth_t *auth);
auth_t *  auth_challenge(stringer_t *username);
int_t  auth_login(stringer_t *username, stringer_t *password, auth_t **output);

/// datatier.c
int_t   auth_data_fetch(auth_t *auth);

/// legacy.c
auth_legacy_t *  auth_legacy(stringer_t *username, stringer_t *password);
auth_legacy_t *  auth_legacy_alloc(void);
void             auth_legacy_free(auth_legacy_t *legacy);


#endif

