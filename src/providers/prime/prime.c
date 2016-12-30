
/**
 * @file /magma/src/providers/prime/prime.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

EC_GROUP *prime_curve_group = NULL;

/**
 * @brief	Initialize the PRIME structures.
 * @return	returns true if everything initializes properly, or false if an error occurrs.
 */
bool_t prime_start(void) {

	if (!(prime_curve_group = EC_GROUP_new_by_curve_name_d(NID_secp256k1))) {
		log_error("An error occurred while trying to create the elliptical group. {%s}", ssl_error_string(MEMORYBUF(256), 256));
		return false;
	}
	else if (EC_GROUP_precompute_mult_d(prime_curve_group, NULL) != 1) {
		log_error("Unable to precompute the required elliptical curve point data. {%s}", ssl_error_string(MEMORYBUF(256), 256));
		EC_GROUP_free_d(prime_curve_group);
		prime_curve_group = NULL;
		return false;
	}

	EC_GROUP_set_point_conversion_form_d(prime_curve_group, POINT_CONVERSION_COMPRESSED);

	return true;
}

/**
 * @brief	Destroy any initialized PRIME structures.
 * @return	This function returns no value.
 */
void prime_stop(void) {

	EC_GROUP *group;

	if (prime_curve_group) {
		group = prime_curve_group;
		prime_curve_group = NULL;
		EC_GROUP_free_d(group);
	}

	return;
}

void prime_free(prime_t *key) {

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

prime_t * prime_alloc(prime_type_t type) {

	prime_t *result = NULL;

	if (!(result = mm_alloc(sizeof(prime_t)))) {
		log_pedantic("PRIME key allocation failed.");
		return NULL;
	}

	mm_wipe(result, sizeof(prime_t));

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

stringer_t * prime_binary_get(prime_t *key, stringer_t *output) {

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

prime_t * prime_binary_set(stringer_t *key) {

	uint16_t type = 0;
	prime_size_t size = 0;
	prime_t *result = NULL;

	// Unpack the object header. For now, we won't worry about message objects,
	// which means we can assume the header is only 5 bytes.
	if (prime_header_read(key, &type, &size)) {
		return NULL;
	}

	if (!(result = mm_alloc(sizeof(prime_t)))) {
		log_pedantic("PRIME key allocation failed.");
		return NULL;
	}

	mm_wipe(result, sizeof(prime_t));

	// Switch statement to call the appropriate allocator.
	switch (type) {
		case (PRIME_ORG_KEY):
			result->type = PRIME_ORG_KEY;
			result->org = org_key_set(key);
			break;
		case (PRIME_USER_KEY):
			result->type = PRIME_USER_KEY;
			result->user = user_key_set(key);
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

stringer_t * prime_armored_get(prime_t *object, stringer_t *output) {

	return NULL;
}

prime_t * prime_armored_set(prime_t *object) {

	return NULL;
}

prime_t * prime_key_generate(prime_type_t type) {

	prime_t *result = NULL;

	if (!(result = mm_alloc(sizeof(prime_t)))) {
		log_pedantic("PRIME key allocation failed.");
		return NULL;
	}

	mm_wipe(result, sizeof(prime_t));

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

stringer_t * prime_key_encrypted_get(stringer_t *key, prime_t *object, stringer_t *output) {

	if (!object) {
		log_pedantic("An invalid key object was provided for serialization.");
		return NULL;
	}

	// Switch statement to call the appropriate allocator.
	switch (object->type) {
		case (PRIME_ORG_KEY):
			output = org_encrypted_key_get(key, object->org, output);
			break;
		case (PRIME_USER_KEY):
			output = user_encrypted_key_get(key, object->user, output);
			break;
		default:
			log_pedantic("Unrecognized PRIME key type.");
			return NULL;
	}

	return output;
}

prime_t * prime_key_encrypted_set(stringer_t *key, stringer_t *object) {

	uint16_t type = 0;
	prime_size_t size = 0;
	prime_t *result = NULL;

	// Unpack the object header. For now, we won't worry about message objects,
	// which means we can assume the header is only 5 bytes.
	if (prime_header_read(object, &type, &size)) {
		return NULL;
	}

	if (!(result = mm_alloc(sizeof(prime_t)))) {
		log_pedantic("PRIME key allocation failed.");
		return NULL;
	}

	mm_wipe(result, sizeof(prime_t));

	// Switch statement to call the appropriate allocator.
	switch (type) {
		case (PRIME_ORG_KEY_ENCRYPTED):
			result->type = PRIME_ORG_KEY;
			result->org = org_encrypted_key_set(key, object);
			break;
		case (PRIME_USER_KEY_ENCRYPTED):
			result->type = PRIME_USER_KEY;
			result->user = user_encrypted_key_set(key, object);
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
