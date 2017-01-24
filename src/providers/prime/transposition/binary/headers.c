
/**
 * @file /magma/providers/prime/transposition/binary/headers.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 */

#include "magma.h"

/**
 * @brief		Determine whether a given type uses a 5 or 6 byte header.
 * @param 		type	The PRIME type.
 * @return		Returns the number of bytes used by the header, or 0 if an invalid type is supplied.
 */
size_t prime_header_length(prime_artifact_type_t type) {

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
 * @brief			Read the first few bytes of a serialized PRIME object and return the magic
 * 					number and object size in native form.
 * @param data		The serialized object buffer.
 * @param type		A pointer where the type will be stored, if sucessful.
 * @param size		A pointer to where the parsed size will be stored, if the read is sucessful.
 * @return			0 if sucessful, negative values if the object data is invalid, or postive numbers if a programmatic error occurs.
 * 					2 = The object buffer was empty.
 * 					1 = A null pointer was supplied.
 * 					0 = Success.
 * 					-1 = The buffer is too short for a valid PRIME object header.
 *					-2 = The size in the header doesn't match the amount of data supplied. (size = header_len + object_len).
 *					-3 = The header indicates an unrecognized PRIME type.
 */
int_t prime_header_read(stringer_t *object, uint16_t *type, uint32_t *size) {

	uchr_t *object_data = NULL;
	uint32_t big_endian_size = 0;
	size_t object_len = 0, header_len = 0;

	if (!object || !type || !size) {
		log_pedantic("A NULL pointer was supplied to the PRIME read function.");
		return 1;
	}
	else if (st_empty_out(object, &object_data, &object_len)) {
		log_pedantic("The PRIME object buffer was empty.");
		return 2;
	}

	if(object_len < 2) {
		log_pedantic("The PRIME object buffer is too short to hold a valid header.");
		return -1;
	}

	// Read the type and convert from big endian into the native host format.
	*type = be16toh(*((uint16_t *)object_data));

	switch (*type) {
		case PRIME_USER_SIGNING_REQUEST:
		case PRIME_USER_SIGNET:
		case PRIME_ORG_SIGNET:
		case PRIME_USER_KEY:
		case PRIME_ORG_KEY:
			// The buffer wasn't long enough to be valid.
			if (object_len < 5) return -1;
			// Encrypted objects use a 5 byte header. A 2 byte type, and 3 byte length.
			mm_copy(((uchr_t *)&big_endian_size) + 1, ((uchr_t *)object_data) + 2, 3);
			// Convert the size to the host format.
			*size = be32toh(big_endian_size);
			header_len = 5;
			break;
		case PRIME_USER_KEY_ENCRYPTED:
		case PRIME_ORG_KEY_ENCRYPTED:
		case PRIME_MESSAGE_ENCRYPTED:
		case PRIME_MESSAGE_FORWARD:
		case PRIME_MESSAGE_BOUNCE:
		case PRIME_MESSAGE_NAKED:
		case PRIME_MESSAGE_DRAFT:
		case PRIME_MESSAGE_ABUSE:
		case PRIME_MESSAGE_SENT:
			// The buffer wasn't long enough to be valid.
			if (object_len < 6) return -1;
			// Encrypted objects use a 6 byte header. A 2 byte type, and 4 byte length.
			mm_copy(((uchr_t *)&big_endian_size), ((uchr_t *)object_data) + 2, 4);
			// Convert the size to the host format.
			*size = be32toh(big_endian_size);
			header_len = 6;
			break;
		default:
			log_pedantic("Unrecognized PRIME type.");
			return -3;
	}

	// Invalid PRIME object! The provided size doesn't match the amount of data in the buffer.
	if (*size + header_len != object_len) {
		return -2;
	}

	return 0;
}

/**
 * @brief
 * @param type
 * @param size
 * @param output
 * @return
 */
stringer_t * prime_header_write(prime_artifact_type_t type, size_t size, stringer_t *output) {

	stringer_t *result = NULL;
	size_t length = 0, written = 0;
	uint16_t big_endian_type = htobe16(type);
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
			mm_copy(st_data_get(result), ((uchr_t *)&big_endian_type), 2);
			mm_copy(st_data_get(result) + 2, ((uchr_t *)&big_endian_size) + 1, 3);
			break;
		case (PRIME_MESSAGE_ENCRYPTED):
			written = 6;
			mm_copy(st_data_get(result), ((uchr_t *)&big_endian_type), 2);
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
