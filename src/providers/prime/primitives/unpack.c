
/**
 * @file /magma/src/providers/prime/primitives/unpack.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

prime_size_t prime_count(stringer_t *fields) {

	placer_t payload;
	int_t type, bytes, size;
	uint32_t count = 0;
	prime_reader_t reader;


	if (prime_reader_open(fields, &reader)) {
		return 0;
	}

	// Read in the field type first. We'll get a 0 result if we've reached the end of the field data.
	while ((type = prime_reader_type(&reader)) > 0) {

		// Figure out how many bytes are used to hold the payload size.
		if ((bytes = prime_field_size_length(type)) < 0) {
			return 0;
		}
		// Read in the payload size if bytes is positive. If bytes is zero then we have a fixed length
		// signature field.
		else if (bytes && (size = prime_reader_size(&reader, bytes)) < 0) {
			return 0;
		}

		// Read in a fixed length signature field.
		else if (prime_reader_payload(&reader, (!bytes ? ED25519_SIGNATURE_LEN : size) , payload)) {
			return 0;
		}

		count++;
	}

	// An error occurred.
	if (type < 0) {
		return 0;
	}

	return count;
}

prime_object_t * prime_unpack(stringer_t *data) {

	uint16_t type = 0;
	uint32_t size = 0;

	// Unpack the object header. For now, we won't worry about message objects,
	// which means we can assume the header is only 5 bytes.
	if (prime_header_read(data, &type, &size) || type == PRIME_MESSAGE_ENCRYPTED) {
		return NULL;
	}

	// CHeck to ensure the header size, matches the size of the binary object data (minus the header).
	// Count the fields.
	// Ensure the field types appear in sequentially.
	// Check that each field only appears once (with the few notable exceptions taken into account).
	// Allocate an array of prime_field_t structures to represent each field.
	// Iterate through and setup each field structure with:
	//		the field type,
	//		a place holder wrapped around the payload (and its size).


	return NULL;
}
