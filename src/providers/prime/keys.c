
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

void org_key_free(prime_org_key_t *org) {

	if (org) {
		if (org->signing) st_free(org->signing);
		if (org->encryption) secp256k1_free(org->encryption);
	}

	return;
}

void user_key_free(prime_user_key_t *user) {

	if (user) {
		if (user->signing) st_free(user->signing);
		if (user->encryption) secp256k1_free(user->encryption);
	}

	return;
}

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

prime_org_key_t * org_key_alloc(void) {

	prime_org_key_t *org = NULL;

	if (!(org = mm_alloc(sizeof(prime_org_key_t)))) {
		log_pedantic("Allocation of the PRIME org key failed.");
		return NULL;
	}

	mm_wipe(org, sizeof(prime_org_key_t));

	return org;
}

prime_user_key_t * user_key_alloc(void) {

	prime_user_key_t *user = NULL;

	if (!(user = mm_alloc(sizeof(prime_user_key_t)))) {
		log_pedantic("Allocation of the PRIME user key failed.");
		return NULL;
	}

	mm_wipe(user, sizeof(prime_user_key_t));

	return user;
}

prime_key_t * prime_key_alloc(prime_type_t type) {

	prime_key_t *result = NULL;

	if (!(result = mm_alloc(sizeof(prime_key_t)))) {
		log_pedantic("Allocation of the PRIME key failed.");
		return NULL;
	}

	switch (type) {

		case PRIME_ORG_KEY:
			result->type = PRIME_ORG_KEY;
			result->org = org_key_alloc();
			break;
		case PRIME_USER_KEY:
			result->type = PRIME_USER_KEY;
			result->user = user_key_alloc();
			break;

		default:
			log_pedantic("Unrecognized key type.");
			prime_key_free(result);
			return NULL;
	}

	return result;
}
