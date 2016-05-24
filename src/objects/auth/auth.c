
/**
 * @file /magma/src/objects/auth/auth.c
 *
 * @brief The primary interface for the STACIE authentication functions.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Free an authentication object.
 * @param	auth	a pointer to the authentication object to be freed.
 * @return	This function returns no value.
 */
void auth_free(auth_t *auth) {

	if (!auth) {
		log_pedantic("Passed a NULL authentication object.");
		return;
	}

	st_cleanup(auth->username);
	st_cleanup(auth->seasoning.salt);
	st_cleanup(auth->keys.master);
	st_cleanup(auth->tokens.auth);
	st_cleanup(auth->legacy.auth);
	st_cleanup(auth->legacy.key);

	mm_free(auth);
	return;
}

auth_t * auth_alloc(stringer_t *username) {

	auth_t *auth = NULL;



	return auth;
}

