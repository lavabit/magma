
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
 * @brief	Initialize the global PRIME structures.
 *
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
 * @brief	Destroy the global PRIME structures.
 *
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

void prime_free(prime_t *object) {

	if (object) {

		switch (object->type) {
			case PRIME_USER_KEY:
				if (object->user) user_key_free(object->user);
				break;
			case PRIME_USER_SIGNET:
				break;
			case PRIME_USER_SIGNING_REQUEST:
				break;
			case PRIME_ORG_KEY:
				if (object->org) org_key_free(object->org);
				break;
			case PRIME_ORG_SIGNET:
				break;
			default:
				log_pedantic("Unrecognized PRIME object type.");
				break;
		}

		// Secure deallocation.
		if ((object->flags & SECURITY) == SECURITY) {
			mm_sec_free(object);
		}
		// Regular allocation.
		else {
			mm_free(object);
		}
	}

	return;
}

prime_t * prime_alloc(prime_type_t type, prime_flags_t flags) {

	prime_t *result = NULL;

	// Secure allocation.
	if ((flags & SECURITY) == SECURITY && !(result = mm_sec_alloc(sizeof(prime_t)))) {
		log_pedantic("PRIME object allocation failed.");
		return NULL;
	}
	// Regular allocation.
	else if ((flags & SECURITY) != SECURITY && !(result = mm_alloc(sizeof(prime_t)))) {
		log_pedantic("PRIME object allocation failed.");
		return NULL;
	}

	// We need wipe the buffer to ensure a clean slate.
	mm_wipe(result, sizeof(prime_t));

	// Save the flags, so the free function knows which deallocator to use.
	result->flags = flags;

	// Switch statement so we can perform any type specific allocation tasks.
	switch (type) {
		case PRIME_USER_KEY:
			result->type = PRIME_USER_KEY;
			break;
		case PRIME_USER_SIGNET:
			result->type = PRIME_USER_SIGNET;
			break;
		case PRIME_USER_SIGNING_REQUEST:
			result->type = PRIME_USER_SIGNING_REQUEST;
			break;
		case PRIME_ORG_KEY:
			result->type = PRIME_ORG_KEY;
			break;
		case PRIME_ORG_SIGNET:
			result->type = PRIME_ORG_SIGNET;
			break;
		default:
			log_pedantic("Unrecognized PRIME type.");
			prime_free(result);
			return NULL;
	}

	return result;
}

stringer_t * prime_get(prime_t *object, prime_encoding_t encoding, stringer_t *output) {

	stringer_t *result = NULL, *binary = NULL;

	if (!object) {
		log_pedantic("An invalid object was provided for serialization.");
		return NULL;
	}

	// Switch statement to call the appropriate allocator.
	switch (object->type) {
		case (PRIME_ORG_KEY):
			result = org_key_get(object->org, output);
			break;
		case (PRIME_USER_KEY):
			result = user_key_get(object->user, output);
			break;
		default:
			log_pedantic("Unrecognized PRIME type.");
			break;
	}

	// If the object was serialized, and an armored output was request, perform the encoding here.
	if (result && (encoding & ARMORED) == ARMORED) {

		// If an output buffer was supplied, we'll need copy the binary data before encoding it. We use the dupe
		// function to ensure a secure buffer is used, if a secure buffer was supplied.
		if (output && !(binary = st_dupe(result))) {
			log_pedantic("A temporary buffer could not be allocated to hold the PRIME object in binary form during encoding.");
			if (!output) st_free(result);
			return NULL;
		}
		else if (!output) {
			binary = result;
		}

		// Armor the binary data. Either the binary serialization allocated a buffer, or we did above, so the binary
		// input must be free regardless of the outcome.
		if (!(result = prime_pem_wrap(binary, output))) {
			log_pedantic("Our attempt to armor the serialized PRIME object failed.");
			st_free(binary);
			return NULL;
		}

		st_free(binary);
	}

	return result;
}

prime_t * prime_set(stringer_t *object, prime_encoding_t encoding, prime_flags_t flags) {

	uint16_t type = 0;
	prime_size_t size = 0;
	prime_t *result = NULL;
	stringer_t *binary = NULL, *output = NULL;

	// If the object data has been armored, we'll need to unwrap it before attempting to parse it.
	if ((encoding & ARMORED) == ARMORED) {

		// If the caller requests memory security, we'll allocate a secure buffer to hold the unwrapped
		// result. Since we know the binary format will be smaller than the armored version, we use the armored
		// length to as the size of our output buffer. This keeps the code simple, at the expense of using extra
		// memory
		if ((flags & SECURITY) == SECURITY &&
			!(output = st_alloc_opts(MANAGED_T | CONTIGUOUS | SECURE, st_length_get(object)))) {
			log_pedantic("The caller asked us to use secure buffers, but we were unable to allocate one to hold the unwrapped PRIME object.");
			return NULL;
		}

		// Unwrap the object. If output is NULL the unwrap function will allocate a normal buffer. Either way, we'll need
		// free it after the object has been parsed.
		if (!(binary = prime_pem_unwrap(object, output))) {
			log_pedantic("Our attempt to remove the PRIME object armor failed.");
			st_cleanup(output);
			return NULL;
		 }

		// We'll use the output variable to track, and then free the binary buffer below.
		output = binary;
	}
	// We were provided a serialized object in binary form already.
	else {
		binary = object;
	}

	// Unpack the object header. For now, we won't worry about message objects,
	// which means we can assume the header is only 5 bytes.
	if (prime_header_read(binary, &type, &size)) {
		st_cleanup(output);
		return NULL;
	}

	// Allocation.
	if (!(result = prime_alloc(type, flags))) {
		log_pedantic("PRIME object allocation failed.");
		st_cleanup(output);
		return NULL;
	}

	// Switch statement to call the appropriate allocator.
	switch (type) {
		case (PRIME_ORG_KEY):
			result->org = org_key_set(binary);
			break;
		case (PRIME_USER_KEY):
			result->user = user_key_set(binary);
			break;
		default:
			log_pedantic("Unrecognized PRIME type.");
			st_cleanup(output);
			prime_free(result);
			return NULL;
	}

	// All done with the binary data, so if we had to unwrap the object above, we need to free the temporary buffer.
	st_cleanup(output);

	// Check object type being parsed was sucessfully setup.
	if ((type == PRIME_ORG_KEY && !result->org) || (type == PRIME_USER_KEY && !result->user)) {
		prime_free(result);
		return NULL;
	}

	return result;
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

stringer_t * prime_key_encrypt(stringer_t *key, prime_t *object, prime_encoding_t encoding, stringer_t *output) {

	stringer_t *result = NULL, *binary = NULL;

	if (!object) {
		log_pedantic("An invalid key object was provided for serialization.");
		return NULL;
	}

	// Switch statement to call the appropriate allocator.
	switch (object->type) {
		case (PRIME_ORG_KEY):
			result = org_encrypted_key_get(key, object->org, output);
			break;
		case (PRIME_USER_KEY):
			result = user_encrypted_key_get(key, object->user, output);
			break;
		default:
			log_pedantic("Unrecognized PRIME key type.");
			return NULL;
	}

	// If the object was serialized, and an armored output was request, perform the encoding here.
	if (result && (encoding & ARMORED) == ARMORED) {

		// If an output buffer was supplied, we'll need copy the binary data before encoding it. We use the dupe
		// function to ensure a secure buffer is used, if a secure buffer was supplied.
		if (output && !(binary = st_dupe(result))) {
			log_pedantic("A temporary buffer could not be allocated to hold the PRIME object in binary form during encoding.");
			if (!output) st_free(result);
			return NULL;
		}
		else if (!output) {
			binary = result;
		}

		// Armor the binary data. Either the binary serialization allocated a buffer, or we did above, so the binary
		// input must be free regardless of the outcome.
		if (!(result = prime_pem_wrap(binary, output))) {
			log_pedantic("Our attempt to armor the serialized PRIME object failed.");
			st_free(binary);
			return NULL;
		}

		st_free(binary);
	}

	return result;
}

prime_t * prime_key_decrypt(stringer_t *key, stringer_t *object, prime_encoding_t encoding, prime_flags_t flags) {

	uint16_t type = 0;
	prime_size_t size = 0;
	prime_t *result = NULL;
	stringer_t *binary = NULL, *output = NULL;

	// If the object data has been armored, we'll need to unwrap it before attempting to parse it.
	if ((encoding & ARMORED) == ARMORED) {

		// If the caller requests memory security, we'll allocate a secure buffer to hold the unwrapped
		// result. Since we know the binary format will be smaller than the armored version, we use the armored
		// length to as the size of our output buffer. This keeps the code simple, at the expense of using extra
		// memory
		if ((flags & SECURITY) == SECURITY &&
			!(output = st_alloc_opts(MANAGED_T | CONTIGUOUS | SECURE, st_length_get(object)))) {
			log_pedantic("The caller asked us to use secure buffers, but we were unable to allocate one to hold the unwrapped PRIME object.");
			return NULL;
		}

		// Unwrap the object. If output is NULL the unwrap function will allocate a normal buffer. Either way, we'll need
		// free it after the object has been parsed.
		if (!(binary = prime_pem_unwrap(object, output))) {
			log_pedantic("Our attempt to remove the PRIME object armor failed.");
			st_cleanup(output);
			return NULL;
		 }

		// We'll use the output variable to track, and then free the binary buffer below.
		output = binary;
	}
	// We were provided a serialized object in binary form already.
	else {
		binary = object;
	}

	// Unpack the object header. For now, we won't worry about message objects,
	// which means we can assume the header is only 5 bytes.
	if (prime_header_read(binary, &type, &size)) {
		st_cleanup(output);
		return NULL;
	}

	// Allocation.
	if (!(result = prime_alloc(type, flags))) {
		log_pedantic("PRIME object allocation failed.");
		st_cleanup(output);
		return NULL;
	}

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
			log_pedantic("Unrecognized PRIME type.");
			st_cleanup(output);
			prime_free(result);
			return NULL;
	}

	// All done with the binary data, so if we had to unwrap the object above, we need to free the temporary buffer.
	st_cleanup(output);

	// Check that whichever key type was requrested, it actually succeeded.
	if ((type == PRIME_ORG_KEY && !result->org) || (type == PRIME_USER_KEY && !result->user)) {
		prime_free(result);
		return NULL;
	}

	return result;
}
