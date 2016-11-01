
/**
 * @file /magma/src/providers/prime/primitives/headers.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief		Determine whether a given type uses a 5 or 6 byte header.
 * @param 		type	The PRIME type.
 * @return		Returns the number of bytes used by the header, or 0 if an invalid type is supplied.
 */
size_t prime_header_length(prime_type_t type) {

	size_t length = 0;

	switch (type) {
		case (PRIME_ORG_SIGNET):
		case (PRIME_ORG_KEY):
		case (PRIME_ORG_KEY_ENCRYPTED):
		case (PRIME_USER_SIGNING_REQUEST):
		case (PRIME_USER_SIGNET):
		case (PRIME_USER_KEY):
		case (PRIME_USER_KEY_ENCRYPTED):
			length = 5;
			break;
		case (PRIME_MESSAGE_ENCRYPTED):
			length = 6;
			break;
		default:
			log_pedantic("Unrecognized PRIME type.");
	}
	return length;
}

/**
 * @brief
 * @param type
 * @param size
 * @param output
 * @return
 */
stringer_t * prime_header_write(prime_type_t type, size_t size, stringer_t *output) {

	stringer_t *result = NULL;
	uint16_t magic = htobe16(type);
	size_t length = 0, written = 0;
	uint32_t big_endian_size = htobe32(size);

	// Figure out how big this header will be.
	if (!(length = prime_header_length(type))) {
		log_pedantic("Invalid variables were provided to the PRIME heading writer.");
		return NULL;
	}

	// Ensure the object size is valid given the object type.
	else if (size < prime_object_size_min(type)) {
		log_error("The size provided is too small for the specified PRIME object type. { type = %s / min = %zu / size = %zu }",
			prime_object_type(type), prime_object_size_min(type), size);
		return NULL;
	}
	else if (size > prime_object_size_max(type)) {
		log_error("The size provided is too large for the specified PRIME object type. { type = %s / max = %zu / size = %zu }",
			prime_object_type(type), prime_object_size_max(type), size);
		return NULL;
	}

	// Check whether the buffer is valid, or if output is NULL, allocate a buffer to hold the output.
	else if (!(result = st_output(output, length))) {
		log_pedantic("An output buffer could not be setup to hold the result.");
		return NULL;
	}

	switch (type) {
		case (PRIME_ORG_SIGNET):
		case (PRIME_ORG_KEY):
		case (PRIME_ORG_KEY_ENCRYPTED):
		case (PRIME_USER_SIGNING_REQUEST):
		case (PRIME_USER_SIGNET):
		case (PRIME_USER_KEY):
		case (PRIME_USER_KEY_ENCRYPTED):
			written = 5;
			mm_copy(st_data_get(result), ((uchr_t *)&magic), 2);
			mm_copy(st_data_get(result) + 2, ((uchr_t *)&big_endian_size) + 1, 3);
			break;
		case (PRIME_MESSAGE_ENCRYPTED):
			written = 6;
			mm_copy(st_data_get(result), ((uchr_t *)&magic), 2);
			mm_copy(st_data_get(result) + 2, ((uchr_t *)&big_endian_size), 4);
			break;
		default:
			log_pedantic("Unrecognized PRIME type.");
	}

	// Make sure the correct number of bytes was written to the output buffer.
	if (written != length) {
		log_pedantic("PRIME header serialization failed. { expected = %zu / actual = %zu }", length, written);
		if (!output) st_free(result);
		return NULL;
	}
	else if (st_valid_tracked(st_opt_get(result))){
		st_length_set(result, written);
	}

	return result;
}

stringer_t * prime_header_org_signet_write(size_t size, stringer_t *output) {
	return prime_header_write(PRIME_ORG_SIGNET, size, output);
}

stringer_t * prime_header_user_signet_write(size_t size, stringer_t *output) {
	return prime_header_write(PRIME_USER_SIGNET, size, output);
}

stringer_t * prime_header_user_signing_request_write(size_t size, stringer_t *output) {
	return prime_header_write(PRIME_USER_SIGNING_REQUEST, size, output);
}

stringer_t * prime_header_org_key_write(size_t size, stringer_t *output) {
	return prime_header_write(PRIME_ORG_KEY, size, output);
}

stringer_t * prime_header_user_key_write(size_t size, stringer_t *output) {
	return prime_header_write(PRIME_USER_KEY, size, output);
}

stringer_t * prime_header_encrypted_org_key_write(size_t size, stringer_t *output) {
	return prime_header_write(PRIME_ORG_KEY_ENCRYPTED, size, output);
}

stringer_t * prime_header_encrypted_user_key_write(size_t size, stringer_t *output) {
	return prime_header_write(PRIME_USER_KEY_ENCRYPTED, size, output);
}

stringer_t * prime_header_encrypted_message_write(size_t size, stringer_t *output) {
	return prime_header_write(PRIME_MESSAGE_ENCRYPTED, size, output);
}
