
/**
 * @file /magma/providers/prime/transposition/binary/fields.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 */

#include "magma.h"

prime_field_t * prime_field_get(prime_object_t *object, prime_field_type_t type) {

	prime_field_t *result = NULL;

	if (!object) {
		return NULL;
	}

	for (int_t i = 0; i < object->count; i++) {

		if (object->fields[i].type == type) {
			result = &(object->fields[i]);
			i = object->count;
		}

	}

	return result;
}

int_t prime_field_size_length(prime_field_type_t field) {

	int_t result = -1;

	// Signature fields are a fixed length, and thus don't include a length parameter. Note that field 253 is the full
	// signet signature, and field 255 is the identifiable signet signature.
	if (field == 0 || (field >= 4 && field <= 15) || field == 253 || field == 255) result = 0;

	// The cryptographic fields, and the informational fields between 16 and 159 all use 1 byte for the
	// length parameter. Note that field 254 is the identifier.
	else if ((field >= 1 && field <= 3) || (field >= 16 && field <= 159) || field == 254) result = 1;

	// The informational fields between 160 and 250 all use 2 bytes for the length parameter.
	else if (field >= 160 && field <= 250) result = 2;

	// Field 251 is for undefined fields, and thus currently lacks an a length because it employs an alternate format.
	/// TODO: else if (field == 251) result = ???;

	// The image field is the only one with a 3 byte length parameter.
	else if (field == 252) result = 3;

	return result;
}

size_t prime_field_size_max(uint16_t type, prime_field_type_t field) {

	size_t result = 0;

	switch (type) {
		case (PRIME_ORG_KEY):
		case (PRIME_ORG_SIGNET):
			// Invalid and/or reserved fields.
			if (field == 0 || (field >= 5 && field <= 15)) result = 0;
			// Fixed length signature fields.
			else if (field == 4 || field == 253 || field == 255) result = PRIME_FIXED_SIZE;
			// Fields with a 1 byte length.
			else if ((field >= 1 && field <= 3) || (field >= 16 && field <= 159) || field == 254) result = PRIME_MAX_1_BYTE;
			// Fields with a 2 byte length.
			else if (field >= 160 && field <= 250) result = PRIME_MAX_2_BYTE;
			// Field 251 is for undefined extensions, and they use a different format.
			/// TODO: else if (field == 251) result = ???;
			// Field 252 is holds the image.
			else if (field == 252) result = PRIME_MAX_3_BYTE;
			break;
		case (PRIME_USER_SIGNING_REQUEST):
		case (PRIME_USER_SIGNET):
		case (PRIME_USER_KEY):
			// Invalid or reserved fields.
			if (field == 0 || (field >= 7 && field <= 15)) result = 0;
			// Fixed length signature fields.
			else if ((field >= 4 && field <= 6) || field == 253 || field == 255) result = PRIME_FIXED_SIZE;
			// Fields with a 1 byte length.
			else if ((field >= 1 && field <= 3) || (field >= 16 && field <= 159) || field == 254) result = PRIME_MAX_1_BYTE;
			// Fields with a 2 byte length.
			else if (field >= 160 && field <= 250) result = PRIME_MAX_2_BYTE;
			// Field 251 is for undefined extensions, and they use a different format.
			/// TODO: else if (field == 251) result = ???;
			// Field 252 is holds the image.
			else if (field == 252) result = PRIME_MAX_3_BYTE;
			break;
		case (PRIME_ORG_KEY_ENCRYPTED):
		case (PRIME_USER_KEY_ENCRYPTED):
		case (PRIME_MESSAGE_ENCRYPTED):
			break;
		default:
			log_pedantic("Unrecognized PRIME type.");
			break;
	}

	return result;
}

stringer_t * prime_field_write(uint16_t type, prime_field_type_t field, size_t size, stringer_t *data, stringer_t *output) {

	uchr_t *payload = NULL;
	stringer_t *result = NULL;
	uint32_t big_endian_size = 0;
	size_t total = 0, size_len = 0, payload_len = 0;

	/// TODO: Add undefined field support.
	if (field == 251) {
		log_pedantic("We don't handle undefined fields, yet.");
		return NULL;
	}

	// Figure out how big the field heading is.
	if (!(size_len = prime_field_size_length(field)) < 0 || st_empty_out(data, &payload, &payload_len)) {
		log_pedantic("Invalid variables were provided to the PRIME heading writer.");
		return NULL;
	}

	// Ensure fixed length fields always use a 64 byte payload.
	else if (size_len == 0 && payload_len != 64) {
		log_error("The payload length provided does not match the size specified for fixed length PRIME fields. { field = %hhu / size = %zu }",
			field, payload_len);
		return NULL;
	}
	// Ensure the data will fit the payload for the given field type.
	else if (size_len > 0 && prime_field_size_max(type, field) < payload_len) {
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
			mm_copy(st_data_get(result) + 1 + size_len, payload, payload_len);
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
			break;
	}

	// If data was written.
	if (result && st_valid_tracked(st_opt_get(result))){
		st_length_set(result, total);
	}

	return result;
}
