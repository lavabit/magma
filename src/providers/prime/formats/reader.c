
/**
 * @file /magma/src/providers/prime/primitives/reader.c
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
 * @brief 		Read a single byte using the reader and convert it to a type field.
 * @param reader
 * @return		-1 if an error occurs, 0 if the end of the buffer is reached, or a field type between 1 and 255 if the reader contained data.
 */
int_t prime_reader_type(prime_reader_t *reader) {

	int_t result = -1;

	// If the reader is setup properly and data remains, read in a single byte.
	if (reader && reader->remaining) {
		reader->remaining--;
		result = *((uint8_t *)reader->cursor++);
	}
	// We've reached the end of the buffer.
	else if (reader) {
		result = 0;
	}

	return result;
}

/**
 * @brief 		Read 1, 2 or 3 bytes representing a the payload size for a field, and convert the bytes from big
 * 				endian into the native format before returning the result.
 * @param reader
 * @return		-1 if an error occurs, or a field type between 1 and 255 if the reader contained data.
 */
int_t prime_reader_size(prime_reader_t *reader, int_t bytes) {

	int32_t raw = 0;
	int_t result = -1;

	// Only read in the number of "bytes" specified, if that request is 1, 2, or 3 bytes. Then convert the bytes read
	// in from big endian form, into the native format before returning.
	if (bytes >= 1 && bytes <= 3 && reader && reader->remaining >= bytes) {
		mm_copy(((uchr_t *)&raw) + (sizeof(raw) - bytes), ((uchr_t *)reader->cursor), bytes);
		reader->cursor = ((uchr_t *)reader->cursor) + bytes;
		reader->remaining = (reader->remaining - bytes);
		result = be32toh(raw);
	}

	return result;
}

/**
 * @brief 		Read in the field payload and return the result wrapped up by a place holder.
 * @param reader
 * @return		-1 if an error occurs, or a 0 if the payload is packaged and returned sucessfully.
 */
int_t prime_reader_payload(prime_reader_t *reader, int_t bytes, placer_t *payload) {

	int_t result = -1;

	// Setup a place holder for the field payload and then advance the reader.
	if (payload && reader && reader->remaining >= bytes) {
		*payload = pl_init(reader->cursor, bytes);
		reader->remaining = (reader->remaining - bytes);
		reader->cursor = ((uchr_t *)reader->cursor) + bytes;
		result = 0;
	}

	return result;
}

int_t prime_reader_open(stringer_t *data, prime_reader_t *reader) {

	if (!reader || st_empty(data)) {
		return -1;
	}

	reader->cursor = st_data_get(data);
	reader->remaining = st_length_get(data);
	reader->buffer = pl_init(st_data_get(data), st_length_get(data));
	return 0;
}
