
/**
 * @file /magma/src/providers/prime/primitives/fields.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

size_t prime_field_size_length(prime_type_t type, prime_field_type_t field) {

	size_t result = 0;

	switch (type) {
		case (PRIME_ORG_KEY):
		case (PRIME_ORG_SIGNET):
		case (PRIME_USER_SIGNING_REQUEST):
		case (PRIME_USER_SIGNET):
		case (PRIME_USER_KEY):
			if ((field >= 1 && field <= 3) || (field >= 16 && field <= 159) || field == 254) result = 1;
			else if (field >= 160 && field <= 250) result = 2;
			else if (field == 252) result = 3;
			break;
		case (PRIME_ORG_KEY_ENCRYPTED):
		case (PRIME_USER_KEY_ENCRYPTED):
		case (PRIME_MESSAGE_ENCRYPTED):
			break;
		default:
			log_pedantic("Unrecognized PRIME type.");
	}

	return result;
}

size_t prime_field_size_max(prime_type_t type, prime_field_type_t field) {

	size_t result = 0;

	switch (type) {
		case (PRIME_ORG_KEY):
		case (PRIME_ORG_SIGNET):
		case (PRIME_USER_SIGNING_REQUEST):
		case (PRIME_USER_SIGNET):
		case (PRIME_USER_KEY):
			if ((field >= 1 && field <= 3) || (field >= 16 && field <= 159) || field == 254) result = PRIME_MAX_1_BYTE;
			else if (field >= 160 && field <= 250) result = PRIME_MAX_2_BYTE;
			else if (field == 252) result = PRIME_MAX_3_BYTE;
			break;
		case (PRIME_ORG_KEY_ENCRYPTED):
		case (PRIME_USER_KEY_ENCRYPTED):
		case (PRIME_MESSAGE_ENCRYPTED):
			break;
		default:
			log_pedantic("Unrecognized PRIME type.");
	}

	return result;
}

stringer_t * prime_field_write(prime_type_t type, prime_field_type_t field, size_t size, stringer_t *data, stringer_t *output) {

	uchr_t *payload = NULL;
	stringer_t *result = NULL;
	uint32_t big_endian_size = 0;
	size_t total = 0, size_len = 0, payload_len = 0;

	// Figure out how big the field heading is.
	if (!(size_len = prime_field_size_length(type, field)) || st_empty_out(data, &payload, &payload_len)) {
		log_pedantic("Invalid variables were provided to the PRIME heading writer.");
		return NULL;
	}

	// Ensure the data will fit given the field type.
	else if (payload_len < prime_field_size_max(type, field)) {
		log_error("The size provided is too small for the specified PRIME field type. { field = %hhu / max = %zu / size = %zu }",
			field, prime_field_size_max(type, field), payload_len);
		return NULL;
	}

	// Update the sizes.
	total = 1 + size_len + payload_len;
	big_endian_size = htobe32(payload_len);

	// Check whether the buffer is valid, or if output is NULL, allocate a buffer to hold the output.
	if (!(result = st_output(output, total))) {
		log_pedantic("An output buffer could not be setup to hold the result.");
		return NULL;
	}

	switch (type) {
		case (PRIME_ORG_SIGNET):
		case (PRIME_ORG_KEY):
		case (PRIME_USER_SIGNING_REQUEST):
		case (PRIME_USER_SIGNET):
		case (PRIME_USER_KEY):
			mm_copy(st_data_get(result), ((uchr_t *)&field), 1);
			mm_copy(st_data_get(result) + 1, ((uchr_t *)&big_endian_size) + (4 - size_len), size_len);
			mm_copy(st_data_get(result) + 1 + size_len, ((uchr_t *)&payload), payload_len);
			break;
		case (PRIME_MESSAGE_ENCRYPTED):
		case (PRIME_ORG_KEY_ENCRYPTED):
		case (PRIME_USER_KEY_ENCRYPTED):
			if (!output) st_free(result);
			result = NULL;
			break;
		default:
			log_pedantic("Unrecognized PRIME type.");
			if (!output) st_free(result);
			result = NULL;
	}

	// If data was written.
	if (result && st_valid_tracked(st_opt_get(result))){
		st_length_set(result, total);
	}

	return result;
}
