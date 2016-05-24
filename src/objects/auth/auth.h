
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
		stringer_t *auth;
		stringer_t *key;
	} legacy;

} auth_t;

void auth_free(auth_t *auth);
auth_t * auth_alloc(stringer_t *username);

#endif

