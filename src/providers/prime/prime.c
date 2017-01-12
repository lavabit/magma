
/**
 * @file /magma/src/providers/prime/prime.c
 *
 * @brief The public PRIME interface. All of the necessary functionality should be available using these functions.
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
			case PRIME_ORG_KEY:
				if (object->key.org) org_key_free(object->key.org);
				break;
			case PRIME_USER_KEY:
				if (object->key.user) user_key_free(object->key.user);
				break;
			case PRIME_ORG_SIGNET:
				if (object->signet.org) org_signet_free(object->signet.org);
				break;
			case PRIME_USER_SIGNET:
				if (object->signet.user) user_signet_free(object->signet.user);
				break;
			case PRIME_USER_SIGNING_REQUEST:
				if (object->signet.user) user_signet_free(object->signet.user);
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
#ifdef MAGMA_PEDANTIC
	else {
		log_pedantic("An invalid PRIME object pointer was passed to the free function.");
	}
#endif

	return;
}

void prime_cleanup(prime_t *object) {
	if (object) {
		prime_free(object);
	}
	return;
}

prime_t * prime_alloc(prime_type_t type, prime_flags_t flags) {

	prime_t *result = NULL;

	if (!(result = mm_alloc(sizeof(prime_t)))) {
		log_pedantic("PRIME object allocation failed.");
		return NULL;
	}

	// We need wipe the buffer to ensure a clean slate.
	mm_wipe(result, sizeof(prime_t));

	// Save the flags, so the free function knows which deallocator to use.
	result->flags = flags;

	// Switch statement so we can perform any type specific allocation tasks.
	switch (type) {
		case PRIME_ORG_KEY:
			result->type = PRIME_ORG_KEY;
			break;
		case PRIME_USER_KEY:
			result->type = PRIME_USER_KEY;
			break;
		case PRIME_ORG_SIGNET:
			result->type = PRIME_ORG_SIGNET;
			break;
		case PRIME_USER_SIGNET:
			result->type = PRIME_USER_SIGNET;
			break;
		case PRIME_USER_SIGNING_REQUEST:
			result->type = PRIME_USER_SIGNING_REQUEST;
			break;
		default:
			log_pedantic("Unrecognized PRIME type.");
			prime_free(result);
			return NULL;
	}

	return result;
}

/**
 * @brief	Serializes a PRIME object and returns it in binary form, or with an ASCII armor encoding.
 */
stringer_t * prime_get(prime_t *object, prime_encoding_t encoding, stringer_t *output) {

	stringer_t *result = NULL, *binary = NULL;

	if (!object) {
		log_pedantic("An invalid object was provided for serialization.");
		return NULL;
	}

	// Switch statement to call the appropriate allocator.
	switch (object->type) {
		case PRIME_ORG_KEY:
			result = org_key_get(object->key.org, output);
			break;
		case PRIME_USER_KEY:
			result = user_key_get(object->key.user, output);
			break;
		case PRIME_ORG_SIGNET:
			result = org_signet_get(object->signet.org, output);
			break;
		case PRIME_USER_SIGNET:
			result = user_signet_get(object->signet.user, output);
			break;
		case PRIME_USER_SIGNING_REQUEST:
			result = user_request_get(object->signet.user, output);
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

/**
 * @brief	Parses a serialized PRIME object into a working context.
 */
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
		case PRIME_ORG_KEY:
			result->key.org = org_key_set(binary);
			break;
		case PRIME_USER_KEY:
			result->key.user = user_key_set(binary);
			break;
		case PRIME_ORG_SIGNET:
			result->signet.org = org_signet_set(binary);
			break;
		case PRIME_USER_SIGNET:
			result->signet.user = user_signet_set(binary);
			break;
		case PRIME_USER_SIGNING_REQUEST:
			result->signet.user = user_request_set(binary);
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
	if ((type == PRIME_ORG_KEY && !result->key.org) || (type == PRIME_USER_KEY && !result->key.user) ||
		(type == PRIME_ORG_SIGNET && !result->signet.org) || (type == PRIME_USER_SIGNET && !result->signet.user) ||
		(type == PRIME_USER_SIGNING_REQUEST && !result->signet.user)) {
		prime_free(result);
		return NULL;
	}

	return result;
}

/**
 * @brief	Generates a new organizational or user key.
 */
prime_t * prime_key_generate(prime_type_t type, prime_flags_t flags) {

	prime_t *result = NULL;

	if (!(result = prime_alloc(type, flags))) {
		log_pedantic("PRIME key allocation failed.");
		return NULL;
	}

	// Switch statement to call the appropriate allocator.
	switch (type) {
		case PRIME_ORG_KEY:
			result->type = PRIME_ORG_KEY;
			result->key.org = org_key_generate();
			break;
		case PRIME_USER_KEY:
			result->type = PRIME_USER_KEY;
			result->key.user = user_key_generate();
			break;
		default:
			log_pedantic("Unrecognized PRIME key type.");
			mm_free(result);
			return NULL;
	}

	// Check that whichever key type was requrested, it actually succeeded.
	if ((type == PRIME_ORG_KEY && !result->key.org) || (type == PRIME_USER_KEY && !result->key.user)) {
		mm_free(result);
		return NULL;
	}

	return result;
}

/**
 * @brief	Takes an organizational key, and generates the corresponding signet.
 */
prime_t * prime_signet_generate(prime_t *object) {

	prime_t *result = NULL;

	if (!object || object->type != PRIME_ORG_KEY || !object->key.org) {
		log_pedantic("Invalid PRIME organizational key passed in for signet generation.");
		return NULL;
	}
	else if (!(result = prime_alloc(PRIME_ORG_SIGNET, NONE))) {
		log_pedantic("PRIME signet allocation failed.");
		return NULL;
	}
	else if (!(result->signet.org = org_signet_generate(object->key.org))) {
		log_pedantic("PRIME signet generation failed.");
		prime_free(result);
		return NULL;
	}

	return result;
}

/**
 * @brief	Takes an organizational signet, or a user signet, and returns the corresponding cryptographic fingerprint.
 */
stringer_t * prime_signet_fingerprint(prime_t *object, stringer_t *output) {

	stringer_t *result = NULL;

	if (!object) {
		log_pedantic("Invalid PRIME signet passed in for fingerprinting.");
		return NULL;
	}

	// Switch statement to call the appropriate allocator.
	switch (object->type) {
		case PRIME_ORG_SIGNET:
			result = org_signet_fingerprint(object->signet.org, output);
			break;
		case PRIME_USER_SIGNET:
			result = user_signet_fingerprint(object->signet.user, output);
			break;
		default:
			log_pedantic("Unrecognized PRIME signet type.");
			return NULL;
	}

	return result;
}

/**
 * @brief	Takes a signet, and validates the self signature. If the object is a user signet, and the validator is a user signet,
 * 			then the custody signature is verified. If the validator is an organizational signet, then the organizational signature
 * 			is validated. If the object is a signing request, then the validator must be a user signet, and the chain of custody
 * 			signature is verified.
 */
bool_t prime_signet_validate(prime_t *object, prime_t *validator) {

	// The object can be an org, or user signet. If it's an org signet, then the validator signet must be NULL. Otherwise, if a user
	// signet was was passed in, then the validator can be NULL, an org signet, or the previous user signet.
	if (!object || (object->type != PRIME_ORG_SIGNET && object->type != PRIME_USER_SIGNET && object->type != PRIME_USER_SIGNING_REQUEST) ||
		(object->type == PRIME_ORG_SIGNET && validator) ||
		(object->type == PRIME_ORG_SIGNET && !object->signet.org) ||
		((object->type == PRIME_USER_SIGNET || object->type == PRIME_USER_SIGNING_REQUEST) && !object->signet.user) ||
		(object->type == PRIME_USER_SIGNING_REQUEST && validator && validator->type != PRIME_USER_SIGNET) ||
		(validator && validator->type != PRIME_ORG_SIGNET && validator->type != PRIME_USER_SIGNET) ||
		(validator && validator->type == PRIME_ORG_SIGNET && !validator->signet.org) ||
		(validator && validator->type == PRIME_USER_SIGNET && !validator->signet.user)) {
		log_pedantic("Invalid PRIME signet passed in for validation.");
		return false;
	}

	// The self-signatures should have been validated when the signet object was created, but we check it again here just to be sure.
	else if (object->type == PRIME_ORG_SIGNET && !org_signet_verify(object->signet.org)) {
		log_pedantic("The PRIME organizational signet provided an invalid self-signature.");
		return false;
	}
	else if (object->type == PRIME_USER_SIGNET && !user_signet_verify_self(object->signet.user)) {
		log_pedantic("The PRIME user signet provided an invalid self-signature.");
		return false;
	}
	else if (object->type == PRIME_USER_SIGNING_REQUEST && !user_request_verify_self(object->signet.user)) {
		log_pedantic("The PRIME user signing request provided an invalid self-signature.");
		return false;
	}

	// The org signature should be validated.
	else if (object->type == PRIME_USER_SIGNET && validator && validator->type == PRIME_ORG_SIGNET &&
		!user_signet_verify_org(object->signet.user, validator->signet.org)) {
		log_pedantic("The PRIME user signet provided an invalid organizational signature.");
		return false;
	}

	// The chain of custody should be validated.
	else if (object->type == PRIME_USER_SIGNET && validator && validator->type == PRIME_USER_SIGNET &&
		!user_signet_verify_chain_of_custody(object->signet.user, validator->signet.user)) {
		log_pedantic("The PRIME user signet provided an invalid chain of custody signature.");
		return false;
	}
	else if (object->type == PRIME_USER_SIGNING_REQUEST && validator && validator->type == PRIME_USER_SIGNET &&
		!user_request_verify_chain_of_custody(object->signet.user, validator->signet.user)) {
		log_pedantic("The PRIME user signing request provided an invalid chain of custody signature.");
		return false;
	}

	return true;
}

/**
 * @brief	Takes a user key, and possibly the previous user key, and generate a signet signing request.
 */
prime_t * prime_request_generate(prime_t *object, prime_t *previous) {

		prime_t *result = NULL;

		// If the previous object isn't NULL, confirm that it is also a user key.
		if (!object || object->type != PRIME_USER_KEY || (previous && previous->type != PRIME_USER_KEY)) {
			log_pedantic("Invalid PRIME user key passed in for signet generation.");
			return NULL;
		}
		else if (!(result = prime_alloc(PRIME_USER_SIGNING_REQUEST, NONE))) {
			log_pedantic("PRIME signet allocation failed.");
			return NULL;
		}
		// If a previous key was provided, generate a rotation request, otherwise generate a new signing request.
		else if ((!previous && !(result->signet.user = user_request_generate(object->key.user))) ||
			(previous && !(result->signet.user = user_request_rotation(object->key.user, previous->key.user)))) {
			log_pedantic("PRIME signet generation failed.");
			prime_free(result);
			return NULL;
		}

		return result;
}

/**
 * @brief	Takes a user signing request, and an organizational key, and returns a signed user signet.
 */
prime_t * prime_request_sign(prime_t *request, prime_t *org) {

	prime_t *result = NULL;

	// If the previous object isn't NULL, confirm that it is also a user key.
	if (!request || request->type != PRIME_USER_SIGNING_REQUEST || !org || org->type != PRIME_ORG_KEY) {
		log_pedantic("Invalid PRIME signing request passed in for signet generation.");
		return NULL;
	}
	else if (!(result = prime_alloc(PRIME_USER_SIGNET, NONE))) {
		log_pedantic("PRIME signet allocation failed.");
		return NULL;
	}
	// If a previous key was provided, generate a rotation request, otherwise generate a new signing request.
	else if (!(result->signet.user = user_request_sign(request->signet.user, org->key.org))) {
		log_pedantic("PRIME signet generation failed.");
		prime_free(result);
		return NULL;
	}

	return result;
}

/**
 * @brief	Encrypt a message.
 */
 prime_t * prime_message_encrypt(stringer_t *message, prime_t *author, prime_t *origin, prime_t *destination, prime_t *recipient) {


	 return NULL;
 }

/**
 * @brief	Encrypt an organizational or user key using a STACIE realm key.
 */
stringer_t * prime_key_encrypt(stringer_t *key, prime_t *object, prime_encoding_t encoding, stringer_t *output) {

	stringer_t *result = NULL, *binary = NULL;

	if (!object) {
		log_pedantic("An invalid key object was provided for serialization.");
		return NULL;
	}

	// Switch statement to call the appropriate allocator.
	switch (object->type) {
		case PRIME_ORG_KEY:
			result = org_encrypted_key_get(key, object->key.org, output);
			break;
		case PRIME_USER_KEY:
			result = user_encrypted_key_get(key, object->key.user, output);
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

/**
 * @brief	Decrypt an organizational or user key using a STACIE realm key.
 */
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

	// Translate the support encrypted object types into their unencrypted equivalent.
	switch (type) {
		case PRIME_ORG_KEY_ENCRYPTED:
			type = PRIME_ORG_KEY;
			break;
		case PRIME_USER_KEY_ENCRYPTED:
			type = PRIME_USER_KEY;
			break;
		default:
			log_pedantic("Unrecognized PRIME type.");
			st_cleanup(output);
			prime_free(result);
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
		case PRIME_ORG_KEY:
			result->key.org = org_encrypted_key_set(key, binary);
			break;
		case PRIME_USER_KEY:
			result->key.user = user_encrypted_key_set(key, binary);
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
	if ((type == PRIME_ORG_KEY && !result->key.org) || (type == PRIME_USER_KEY && !result->key.user)) {
		prime_free(result);
		return NULL;
	}

	return result;
}
