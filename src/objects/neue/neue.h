
/**
 * @file /magma/objects/users/neue.h
 *
 * @brief	Neue user objects.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_OBJECTS_NEUE_H
#define MAGMA_OBJECTS_NEUE_H

enum {
	M_NEUE_FLAGS_XXXX = 1,//!< M_NEUE_FLAGS_XXXX
	M_NEUE_FLAGS_YYYY = 2 //!< M_NEUE_FLAGS_YYYY
};

typedef enum {
	NONE,
	LEGACY,
	STACIE
} auth_type;

typedef struct {

	int_t type;
	auth_type authentication;

	/// TODO: Add usernum to structure and call the appropriate fetch function to lookup its value.

	union {

		struct {
			stringer_t *address; /* The email address after being broiled. */
			stringer_t *domain; /* What domain the account is associated with. */
		} mail;

		struct {
			stringer_t *username; /* The username provided for authentication. */
			stringer_t *domain; /* What domain the account is associated with. */
			stringer_t *password; /* The salted password hash. */
			stringer_t *key; /* The key used to decrypt the private asymmetric key. */
		} auth;

	};

} credential_t;

// All of a user's information is stored using this structure.
typedef struct {

	struct {
		time_t stamp; /* When the credentials were authenticated. */
		uint64_t flags; /* User account flags. */
		uint64_t usernum; /* User number. */
	} info;

	credential_t *cred; /* User credentials. */
	pthread_rwlock_t lock; /* Master user lock. */

} neue_t;

/// credentials.c
stringer_t *    credential_address(stringer_t *s);
credential_t *  credential_alloc_auth(stringer_t *username, stringer_t *password);
credential_t *  credential_alloc_mail(stringer_t *address);
void            credential_free(credential_t *cred);
stringer_t *    credential_username(stringer_t *s);

/// datatier.c

stringer_t *    credential_fetch_salt(stringer_t *username);

#endif

