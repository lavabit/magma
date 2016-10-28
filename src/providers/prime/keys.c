
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
				log_pedantic("Unrecognized key type.");
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
			log_pedantic("Unrecognized key type.");
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
			log_pedantic("Unrecognized key type.");
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

	size_t length = 0;
	stringer_t *result = NULL;

	if (!key || (key->type == PRIME_ORG_KEY && !(length = org_key_length(key->org))) || (key->type == PRIME_USER_KEY && !(length = user_key_length(key->user)))) {
		log_pedantic("An invalid key object was provided for serialization.");
		return NULL;
	}

	// See if we have a valid output buffer, or if output is NULL, allocate a buffer to hold the output.
	else if (output && (!st_valid_destination(st_opt_get(output)) || st_avail_get(output) < length)) {
		log_pedantic("An output string was supplied but it does not represent a buffer capable of holding the output.");
		return NULL;
	}
	else if (!output && length && !(result = st_alloc(length))) {
		log_pedantic("Could not allocate a buffer large enough to hold encoded result. { requested = %zu }", length);
		return NULL;
	}
	else if (result) {
		output = result;
	}

	// Wipe the buffer so any leading bytes we don't use will be zero'ed out for padding purposes.
	st_wipe(output);

	// Switch statement to call the appropriate allocator.
	switch (key->type) {
		case (PRIME_ORG_KEY):
			length = org_key_get(key->org, output);
			break;
		case (PRIME_USER_KEY):
			length = user_key_get(key->user, output);
			break;
		default:
			log_pedantic("Unrecognized key type.");
			if (result) mm_free(result);
			return NULL;
	}

	// Check that whichever key type was requrested, it actually succeeded.
	if ((key->type == PRIME_ORG_KEY && !length) || (key->type == PRIME_USER_KEY && !length)) {
		if (result) mm_free(result);
		return NULL;
	}

	return output;
}
