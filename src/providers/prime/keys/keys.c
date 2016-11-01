
/**
 * @file /magma/src/providers/prime/keys.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

void prime_key_free(prime_key_t *key) {

	if (key) {

		switch (key->type) {
			case PRIME_ORG_KEY:
				if (key->org) org_key_free(key->org);
				break;
			case PRIME_USER_KEY:
				if (key->user) user_key_free(key->user);
				break;
			default:
				log_pedantic("Unrecognized PRIME key type.");
				return;
		}

		mm_free(key);
	}

	return;
}

prime_key_t * prime_key_alloc(prime_type_t type) {

	prime_key_t *result = NULL;

	if (!(result = mm_alloc(sizeof(prime_key_t)))) {
		log_pedantic("PRIME key allocation failed.");
		return NULL;
	}

	mm_wipe(result, sizeof(prime_key_t));

	// Switch statement to call the appropriate allocator.
	switch (type) {
		case (PRIME_ORG_KEY):
			result->type = PRIME_ORG_KEY;
			result->org = org_key_alloc();
			break;
		case (PRIME_USER_KEY):
			result->type = PRIME_USER_KEY;
			result->user = user_key_alloc();
			break;
		default:
			log_pedantic("Unrecognized PRIME key type.");
			mm_free(result);
			return NULL;
	}

	// Check that whichever key type was requrested, it actually succeeded.
	if ((type == PRIME_ORG_KEY && !result->org) || (type == PRIME_USER_KEY && !result->user)) {
		mm_free(result);
		return NULL;
	}

	return result;
}

prime_key_t * prime_key_generate(prime_type_t type) {

	prime_key_t *result = NULL;

	if (!(result = mm_alloc(sizeof(prime_key_t)))) {
		log_pedantic("PRIME key allocation failed.");
		return NULL;
	}

	mm_wipe(result, sizeof(prime_key_t));

	// Switch statement to call the appropriate allocator.
	switch (type) {
		case (PRIME_ORG_KEY):
			result->type = PRIME_ORG_KEY;
			result->org = org_key_generate();
			break;
		case (PRIME_USER_KEY):
			result->type = PRIME_USER_KEY;
			result->user = user_key_generate();
			break;
		default:
			log_pedantic("Unrecognized PRIME key type.");
			mm_free(result);
			return NULL;
	}

	// Check that whichever key type was requrested, it actually succeeded.
	if ((type == PRIME_ORG_KEY && !result->org) || (type == PRIME_USER_KEY && !result->user)) {
		mm_free(result);
		return NULL;
	}

	return result;
}

stringer_t * prime_key_get(prime_key_t *key, stringer_t *output) {

	if (!key) {
		log_pedantic("An invalid key object was provided for serialization.");
		return NULL;
	}

	// Switch statement to call the appropriate allocator.
	switch (key->type) {
		case (PRIME_ORG_KEY):
			output = org_key_get(key->org, output);
			break;
		case (PRIME_USER_KEY):
			output = user_key_get(key->user, output);
			break;
		default:
			log_pedantic("Unrecognized PRIME key type.");
			return NULL;
	}

	return output;
}
