
/**
 * @file /magma/providers/prime/messages/chunks/chunks.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 */

#include "magma.h"

int32_t chunk_buffer_size(stringer_t *chunk) {

	size_t len = 0;
	uint8_t type = 0;
	uchr_t *data = NULL;
	int32_t result = -1;
	uint32_t big_endian_size = 0;

	if (st_empty_out(chunk, &data, &len) || len < 4 || (type = chunk_header_type(chunk)) == PRIME_CHUNK_INVALID) {
		log_pedantic("The chunk buffer is invalid.");
		return result;
	}

	else if (type < PRIME_SIGNATURE_TREE) {
		mm_copy(((uchr_t *)&big_endian_size) + 1, ((uchr_t *)data) + 1, 3);
		result = be32toh(big_endian_size);
	}

	else {
		result = 64;
	}

	// Use the chunk type to calculate the header length, 1 byte for signatures/fixed length chunks, or 4 bytes for variable
	// length chunks. Then, using the type, determine the number of keyslots associated with the chunk, and multiply the
	// by number of slots by 32 bytes. The result, should be the number of overhead bytes which we'll then add to our payload length.
	result += (type < PRIME_SIGNATURE_TREE ? 4 : 1) + (type > PRIME_CHUNK_EPHEMERAL ? (slots_count(type) * SECP256K1_SHARED_SECRET_LEN) : 0);

	return result;
}

int32_t chunk_header_size(stringer_t *chunk) {

	size_t len = 0;
	uint8_t type = 0;
	uchr_t *data = NULL;
	int32_t result = -1;
	uint32_t big_endian_size = 0;

	if (st_empty_out(chunk, &data, &len) || len < 4 || (type = chunk_header_type(chunk)) == PRIME_CHUNK_INVALID) {
		log_pedantic("The chunk buffer is invalid.");
		return result;
	}

	else if (type < PRIME_SIGNATURE_TREE) {
		mm_copy(((uchr_t *)&big_endian_size) + 1, ((uchr_t *)data) + 1, 3);
		result = be32toh(big_endian_size);
	}

	else {
		result = 64;
	}

	return result;
}

prime_message_chunk_type_t chunk_header_type(stringer_t *chunk) {

	size_t len = 0;
	uint8_t type = 0;
	uchr_t *data = NULL;
	prime_message_chunk_type_t result = PRIME_CHUNK_INVALID;

	if (st_empty_out(chunk, &data, &len)) {
		log_pedantic("The chunk buffer is invalid.");
		return result;
	}

	type = *((uint8_t *)data);

	// We use a switch statement to explicitly assign the chunk type to the result. This ensures we don't
	// return an unsupported chunk type.
	switch(type) {

		// Envelope
		case PRIME_CHUNK_EPHEMERAL:
			result = PRIME_CHUNK_EPHEMERAL;
			break;
		case PRIME_CHUNK_ORIGIN:
			result = PRIME_CHUNK_ORIGIN;
			break;
		case PRIME_CHUNK_DESTINATION:
			result = PRIME_CHUNK_DESTINATION;
			break;

		// Metadata
		case PRIME_CHUNK_COMMON:
			result = PRIME_CHUNK_COMMON;
			break;
		case PRIME_CHUNK_HEADERS:
			result = PRIME_CHUNK_HEADERS;
			break;

		// Body
		case PRIME_CHUNK_BODY:
			result = PRIME_CHUNK_BODY;
			break;

		// Signatures
		case PRIME_SIGNATURE_TREE:
			result = PRIME_SIGNATURE_TREE;
			break;
		case PRIME_SIGNATURE_USER:
			result = PRIME_SIGNATURE_USER;
			break;
		case PRIME_SIGNATURE_ORGIN:
			result = PRIME_SIGNATURE_ORGIN;
			break;
		case PRIME_SIGNATURE_DESTINATION:
			result = PRIME_SIGNATURE_DESTINATION;
			break;
		default:
			log_pedantic("The chunk type is invalid. { type = %i }", type);
			break;
	};
	return result;
}

int_t chunk_header_read(stringer_t *data, uint8_t *type, uint32_t *size, placer_t *chunk) {

	int32_t holder = 0;

	if (!data || !type || !size || !chunk) {
		log_pedantic("A NULL pointer was supplied to the PRIME chunk read function.");
		return 1;
	}

	else if ((*type = chunk_header_type(data)) == PRIME_CHUNK_INVALID) {
		return -1;
	}

	else if ((holder = chunk_header_size(data)) < 0) {
		return -1;
	}

	// The chunk
	*chunk = pl_init(st_data_get(data), holder + (*type < PRIME_SIGNATURE_TREE ? 4 : 1) +
		(*type > PRIME_CHUNK_EPHEMERAL ? (slots_count(*type) * SECP256K1_SHARED_SECRET_LEN) : 0));
	*size = holder;

	// Bounds check, ensure the provided data buffer is large enough to hold the calculated length.
	if (pl_length_get(*chunk) > st_length_get(data)) {
		log_pedantic("The chunk appears invalid. The buffer is to small. { expected = %zu / actual = %zu }",
			pl_length_get(*chunk), st_length_get(data));
		*type = PRIME_CHUNK_INVALID;
		*size = 0;
		return -1;
	}

	return 0;
}

int_t chunk_buffer_read(stringer_t *data, uint8_t *type, uint32_t *payload_size, uint32_t *buffer_size, placer_t *chunk) {

	int32_t result = 0;

	if (!data || !type || !payload_size || !buffer_size || !chunk) {
		log_pedantic("A NULL pointer was supplied to the PRIME chunk read function.");
		return 1;
	}

	else if ((result = chunk_header_read(data, type, payload_size, chunk)) >= 0) {
		*buffer_size = *payload_size + (*type < PRIME_SIGNATURE_TREE ? 4 : 1) + (*type > PRIME_CHUNK_EPHEMERAL ? (slots_count(*type) * SECP256K1_SHARED_SECRET_LEN) : 0);

		// Bounds check, ensure the provided data buffer is large enough to hold the calculated length.
		if (pl_length_get(*chunk) > st_length_get(data)) {
			log_pedantic("The chunk appears invalid. The buffer is to small. { expected = %zu / actual = %zu }",
				pl_length_get(*chunk), st_length_get(data));
			*type = PRIME_CHUNK_INVALID;
			*payload_size = 0;
			*buffer_size = 0;
			return -1;
		}
	}

	return result;
}

stringer_t * chunk_header_write(prime_message_chunk_type_t type, size_t size, stringer_t *output) {

	stringer_t *result = NULL;
	uint32_t big_endian_size = htobe32(size);

	// Ensure the chunk payload is less than (2^24) - 1, or 16,777,215 in length.
	if (PRIME_MAX_3_BYTE < size) {
		log_error("The size provided is too large for the specified PRIME chunk type. { type = %i / max = %i / size = %zu }",
			type, PRIME_MAX_3_BYTE, size);
		return NULL;
	}

	// Check whether the buffer is valid, or if output is NULL, allocate a buffer to hold the output.
	else if (!(result = st_output(output, 4))) {
		log_pedantic("An output buffer could not be setup to hold the result.");
		return NULL;
	}

	mm_copy(st_data_get(result), ((uchr_t *)&type), 1);
	mm_copy(st_data_get(result) + 1, ((uchr_t *)&big_endian_size) + 1, 3);

	if (st_valid_tracked(st_opt_get(result))){
		st_length_set(result, 4);
	}

	return result;
}

