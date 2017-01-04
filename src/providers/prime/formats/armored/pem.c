
/**
 * @file /magma/src/providers/prime/formats/armored/pem.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

#define PRIME_PEM_LINE_WRAP_LENGTH		64
#define PRIME_PEM_LINE_WRAP_CHARS		"\n"
#define PRIME_PEM_LINE_WRAP_TYPE 		BASE64_LINE_WRAP_LF

stringer_t *prime_pem_headings[] = {
	CONSTANT("-----BEGIN USER KEY-----"),
	CONSTANT("-----BEGIN USER SIGNET-----"),
	CONSTANT("-----BEGIN SIGNET SIGNING REQUEST-----"),
	CONSTANT("-----BEGIN ORGANIZATIONAL KEY-----"),
	CONSTANT("-----BEGIN ORGANIZATIONAL SIGNET-----"),
	CONSTANT("-----BEGIN ENCRYPTED USER KEY-----"),
	CONSTANT("-----BEGIN ENCRYPTED ORGANIZATIONAL KEY-----"),
	CONSTANT("-----BEGIN ENCRYPTED MESSAGE-----")
};

stringer_t *prime_pem_endings[] = {
	CONSTANT("-----END USER KEY-----"),
	CONSTANT("-----END USER SIGNET-----"),
	CONSTANT("-----END SIGNET SIGNING REQUEST-----"),
	CONSTANT("-----END ORGANIZATIONAL KEY-----"),
	CONSTANT("-----END ORGANIZATIONAL SIGNET-----"),
	CONSTANT("-----END ENCRYPTED USER KEY-----"),
	CONSTANT("-----END ENCRYPTED ORGANIZATIONAL KEY-----"),
	CONSTANT("-----END ENCRYPTED MESSAGE-----")
};

stringer_t * prime_pem_begin(prime_type_t type) {

	stringer_t *result = NULL;

	switch (type) {
		case PRIME_USER_KEY:
			result = prime_pem_headings[0];
			break;
		case PRIME_USER_SIGNET:
			result = prime_pem_headings[1];
			break;
		case PRIME_USER_SIGNING_REQUEST:
			result = prime_pem_headings[2];
			break;
		case PRIME_ORG_KEY:
			result = prime_pem_headings[3];
			break;
		case PRIME_ORG_SIGNET:
			result = prime_pem_headings[4];
			break;
		case PRIME_USER_KEY_ENCRYPTED:
			result = prime_pem_headings[5];
			break;
		case PRIME_ORG_KEY_ENCRYPTED:
			result = prime_pem_headings[6];
			break;
		case PRIME_MESSAGE_ENCRYPTED:
			result = prime_pem_headings[7];
			break;
		default:
			log_pedantic("Unrecognized PRIME type.");
			break;
	}

	return result;
}

stringer_t * prime_pem_end(prime_type_t type) {

	stringer_t *result = NULL;

	switch (type) {
		case PRIME_USER_KEY:
			result = prime_pem_endings[0];
			break;
		case PRIME_USER_SIGNET:
			result = prime_pem_endings[1];
			break;
		case PRIME_USER_SIGNING_REQUEST:
			result = prime_pem_endings[2];
			break;
		case PRIME_ORG_KEY:
			result = prime_pem_endings[3];
			break;
		case PRIME_ORG_SIGNET:
			result = prime_pem_endings[4];
			break;
		case PRIME_USER_KEY_ENCRYPTED:
			result = prime_pem_endings[5];
			break;
		case PRIME_ORG_KEY_ENCRYPTED:
			result = prime_pem_endings[6];
			break;
		case PRIME_MESSAGE_ENCRYPTED:
			result = prime_pem_endings[7];
			break;
		default:
			log_pedantic("Unrecognized PRIME type.");
			break;
	}

	return result;
}

stringer_t * prime_pem_wrap(stringer_t *object, stringer_t *output) {

	int_t written = 0;
	uint16_t type = 0;
	uint32_t crc = 0;
	prime_size_t size = 0;
	uchr_t big_endian_crc[3];
	size_t length = 0, body = 0;
	stringer_t *begin = NULL, *end = NULL, *result = NULL, *encoded = NULL, *holder = NULL;

	if (st_empty(object)) {
		return NULL;
	}
	else if (prime_header_read(object, &type, &size)) {
		log_pedantic("An invalid PRIME object was provided for pem encoding.");
		return NULL;
	}
	else if (!(begin = prime_pem_begin(type)) || !(end = prime_pem_end(type))) {
		log_pedantic("The PRIME object type does not support the privacy enhanced message format. { magic = %hhu / type = %s }",
			type, prime_object_type(type));
		return NULL;
	}

	// Calculate a crc24 checksum for the binary object.
	crc = crc24_checksum(st_data_get(object), st_length_get(object));

	// If necessary swap the crc byte order.
#ifdef LITTLE_ENDIAN
	big_endian_crc[0] = ((uchr_t *)&crc)[2];
	big_endian_crc[1] = ((uchr_t *)&crc)[1];
	big_endian_crc[2] = ((uchr_t *)&crc)[0];
#else
	big_endian_crc[0] = ((uchr_t *)&crc)[0];
	big_endian_crc[1] = ((uchr_t *)&crc)[1];
	big_endian_crc[2] = ((uchr_t *)&crc)[2];
#endif

	// Determine the length of the heading and ending strings. Then add extra bytes for the end of line sequence.
	length = body = st_length_get(begin) + st_length_get(end) + PRIME_PEM_LINE_WRAP_TYPE + PRIME_PEM_LINE_WRAP_TYPE;

	// Calculate the length of the base64 encoded data, including line endings.
	length += base64_encoded_length_wrap(st_length_get(object), PRIME_PEM_LINE_WRAP_LENGTH, PRIME_PEM_LINE_WRAP_TYPE);

	// Add a fixed number of bytes for the checksum line.
	length += 5 + PRIME_PEM_LINE_WRAP_TYPE;

	// If an output buffer was supplied, ensure it is sufficient to hold the result, otherwise if NULL was provided, allocate
	// a new buffer.
	if (!(result = st_output(output, length))) {
		log_pedantic("An output buffer could not be setup to hold the result.");
		return NULL;
	}
	// Duplicate the buffer to ensure a secure buffer is allocated if the function was supplied with a secure output buffer.
	else if (!(encoded = st_dupe(result))) {
		log_pedantic("A temporary buffer could not be setup to hold the base64 encoded output.");
		if (!output) st_free(result);
		return NULL;
	}

	else if (!(holder = base64_encode_wrap(object, PRIME_PEM_LINE_WRAP_LENGTH, PRIME_PEM_LINE_WRAP_TYPE, encoded))) {
		log_pedantic("Unable to base64 encode the PRIME object.");
		if (!output) st_free(result);
		st_free(encoded);
		return NULL;
	}

	else if (st_write(result, begin, NULLER(PRIME_PEM_LINE_WRAP_CHARS), encoded, NULLER("="),
		base64_encode_wrap(PLACER(&big_endian_crc, 3), PRIME_PEM_LINE_WRAP_LENGTH, PRIME_PEM_LINE_WRAP_TYPE, MANAGEDBUF(4 + PRIME_PEM_LINE_WRAP_TYPE)),
		end, NULLER(PRIME_PEM_LINE_WRAP_CHARS)) != length) {
		log_pedantic("The PRIME object failed to encode properly. { expected = %zu / actual = %i }", length, written);
		if (!output) st_free(result);
		st_free(encoded);
		return NULL;
	}

	st_free(encoded);

	return result;
}

stringer_t * prime_pem_unwrap(stringer_t *pem, stringer_t *output) {

	return NULL;
}
