
/**
 * @file /magma/providers/consumers/deserialization.c
 *
 * @brief	Distributed cache interface for de-serializing various data types.
 */

#include "magma.h"

/**
 * @brief	Count the number of bytes before a semicolon or non-numeric character is found.
 * @param	data		a pointer to a null-terminated string to be scanned.
 * @param	remaining	the maximum number of characters to be scanned.
 * @return	-1 if no match is found, or the length, in bytes, of the data preceding the matched character.
 */
inline int_t deserialize_count_digits(chr_t *data, size_t remaining) {
	
	chr_t *holder = data;
	
	// Search for the terminating semicolon.
	while (remaining-- && *holder >= '0' && *holder <= '9') {
		holder++;
	}
	if (*holder != ';') {
		return -1;
	}
	return holder - data;
}

/**
 * @brief	Deserialize an size_t from a serialized object, and update its position.
 * @see		deserialize_uint64
 * @param	serial	a pointer to a serialized object from which the data will be extracted.
 * @param	number	a pointer to an size_t variable which will receive the de-serialized data at the serialized object's current position.
 * @return	true on success or false on failure.
 */
bool_t deserialize_sz(serialization_t *serial, size_t *number) {
	return deserialize_uint64(serial, number);
}

/**
 * @brief	Deserialize an ssize_t from a serialized object, and update its position.
 * @see		deserialize_int64
 * @param	serial	a pointer to a serialized object from which the data will be extracted.
 * @param	number	a pointer to an ssize_t variable which will receive the de-serialized data at the serialized object's current position.
 * @return	true on success or false on failure.
 */
bool_t deserialize_ssz(serialization_t *serial, ssize_t *number) {
	return deserialize_int64(serial, number);
}

/**
 * @brief	Deserialize an unsigned 64-bit integer from a serialized object, and update its position.
 * @param	serial	a pointer to a serialized object from which the data will be extracted.
 * @param	number	a pointer to an unsigned 64-bit integer which will receive the de-serialized data at the serialized object's current position.
 * @return	true on success or false on failure.
 */
bool_t deserialize_uint64(serialization_t *serial, uint64_t *number) {

	size_t length;
	
	if (!serial || !serial->data || !number) {
		log_pedantic("Sanity check failed.");
		return false;
	}
	
	// Reset the number, just in case we exit early.
	*number = 0;
	
	// Make sure we have data left.
	if (st_length_get(serial->data) <= serial->position) {
		log_pedantic("Bounds violation.");
		return false;
	}
	
	if ((length = deserialize_count_digits(st_char_get(serial->data) + serial->position, st_length_get(serial->data) - serial->position)) <= 0) {
		log_pedantic("Serialization error.");
		return false;
	}
	
	// Extract the number.
	if (!uint64_conv_bl(st_char_get(serial->data) + serial->position, length, number)) {
		log_pedantic("Extraction error.");
		return false;
	}
	
	// Update the position and return success.
	serial->position += length + 1;
	return true;
}

/**
 * @brief	Deserialize an unsigned 32-bit integer from a serialized object, and update its position.
 * @param	serial	a pointer to a serialized object from which the data will be extracted.
 * @param	number	a pointer to an unsigned 32-bit integer which will receive the de-serialized data at the serialized object's current position.
 * @return	true on success or false on failure.
 */
bool_t deserialize_uint32(serialization_t *serial, uint32_t *number) {

	size_t length;
	
	if (!serial || !serial->data || !number) {
		log_pedantic("Sanity check failed.");
		return false;
	}
	
	// Reset the number, just in case we exit early.
	*number = 0;
	
	// Make sure we have data left.
	if (st_length_get(serial->data) <= serial->position) {
		log_pedantic("Bounds violation.");
		return false;
	}
	
	if ((length = deserialize_count_digits(st_char_get(serial->data) + serial->position,  st_length_get(serial->data) - serial->position)) <= 0) {
		log_pedantic("Serialization error.");
		return false;
	}
	
	// Extract the number.
	if (!uint32_conv_bl(st_char_get(serial->data) + serial->position, length, number)) {
		log_pedantic("Extraction error.");
		return false;
	}
	
	// Update the position and return success.
	serial->position += length + 1;
	return true;
}

/**
 * @brief	Deserialize a signed 32-bit integer from a serialized object, and update its position.
 * @param	serial	a pointer to a serialized object from which the data will be extracted.
 * @param	number	a pointer to a signed 32-bit integer which will receive the de-serialized data at the serialized object's current position.
 * @return	true on success or false on failure.
 */
bool_t deserialize_int32(serialization_t *serial, int_t*number) {

	size_t length;
	
	if (!serial || !serial->data || !number) {
		log_pedantic("Sanity check failed.");
		return false;
	}
	
	// Reset the number, just in case we exit early.
	*number = 0;
	
	// Make sure we have data left.
	if (st_length_get(serial->data) <= serial->position) {
		log_pedantic("Bounds violation.");
		return false;
	}
	
	if ((length = deserialize_count_digits(st_char_get(serial->data) + serial->position,  st_length_get(serial->data) - serial->position)) <= 0) {
		log_pedantic("Serialization error.");
		return false;
	}
	
	// Extract the number.
	if (!int32_conv_bl(st_char_get(serial->data) + serial->position, length, number)) {
		log_pedantic("Extraction error.");
		return false;
	}
	
	// Update the position and return success.
	serial->position += length + 1;
	return true;
}

/**
 * @brief	Deserialize a signed 64-bit integer from a serialized object, and update its position.
 * @param	serial	a pointer to a serialized object from which the data will be extracted.
 * @param	number	a pointer to a signed 64-bit integer which will receive the de-serialized data at the serialized object's current position.
 * @return	true on success or false on failure.
 */
bool_t deserialize_int64(serialization_t *serial, long *number) {

	size_t length;
	
	if (!serial || !serial->data || !number) {
		log_pedantic("Sanity check failed.");
		return false;
	}
	
	// Reset the number, just in case we exit early.
	*number = 0;
	
	// Make sure we have data left.
	if (st_length_get(serial->data) <= serial->position) {
		log_pedantic("Bounds violation.");
		return false;
	}
	
	if ((length = deserialize_count_digits(st_char_get(serial->data) + serial->position,  st_length_get(serial->data) - serial->position)) <= 0) {
		log_pedantic("Serialization error.");
		return false;
	}
	
	// Extract the number.
	if (!int64_conv_bl(st_char_get(serial->data) + serial->position, length, number)) {
		log_pedantic("Extraction error.");
		return false;
	}
	
	// Update the position and return success.
	serial->position += length + 1;
	return true;
}

/**
 * @brief	Deserialize a signed 16 bit integer from a serialized object, and update its position.
 * @param	serial	a pointer to a serialized object from which the data will be extracted.
 * @param	number	a pointer to a signed 16-bit integer which will receive the de-serialized data at the serialized object's current position.
 * @return	true on success or false on failure.
 */
bool_t deserialize_int16(serialization_t *serial, short *number) {

	size_t length;
	
	if (!serial || !serial->data || !number) {
		log_pedantic("Sanity check failed.");
		return false;
	}
	
	// Reset the number, just in case we exit early.
	*number = 0;
	
	// Make sure we have data left.
	if (st_length_get(serial->data) <= serial->position) {
		log_pedantic("Bounds violation.");
		return false;
	}
	
	if ((length = deserialize_count_digits(st_char_get(serial->data) + serial->position,  st_length_get(serial->data) - serial->position)) <= 0) {
		log_pedantic("Serialization error.");
		return false;
	}
	
	// Extract the number.
	if (!int16_conv_bl(st_char_get(serial->data) + serial->position, length, number)) {
		log_pedantic("Extraction error.");
		return false;
	}
	
	// Update the position and return success.
	serial->position += length + 1;
	return true;
}

/**
 * @brief	Deserialize an unsigned 16 bit integer from a serialized object, and update its position.
 * @param	serial	a pointer to a serialized object from which the data will be extracted.
 * @param	number	a pointer to an unsigned 16-bit integer which will receive the de-serialized data at the serialized object's current position.
 * @return	true on success or false on failure..
 */
bool_t deserialize_uint16(serialization_t *serial, unsigned short *number) {

	ssize_t length;
	
	if (!serial || !serial->data || !number) {
		log_pedantic("Sanity check failed.");
		return false;
	}
	
	// Reset the number, just in case we exit early.
	*number = 0;
	
	// Make sure we have data left.
	if (st_length_get(serial->data) <= serial->position) {
		log_pedantic("Bounds violation.");
		return false;
	}
	
	if ((length = deserialize_count_digits(st_char_get(serial->data) + serial->position,  st_length_get(serial->data) - serial->position)) <= 0) {
		log_pedantic("Serialization error.");
		return false;
	}
	
	// Extract the number.
	if (!uint16_conv_bl(st_char_get(serial->data) + serial->position, length, number)) {
		log_pedantic("Extraction error.");
		return false;
	}
	
	// Update the position and return success.
	serial->position += length + 1;
	return true;
}

/**
 * @brief	Deserialize a managed string from a serialized object, and update its position.
 * @note	A managed string is stored as a 64 bit length field, followed by opaque data.
 * @param	serial	a pointer to a serialized object from which the data will be extracted.
 * @param	number	a pointer to the address of a managed string, will receive the de-serialized data at the serialized object's current position.
 * @return	true on success or false on failure..
 */
bool_t deserialize_st(serialization_t *serial, stringer_t **string) {

	stringer_t *result;
	size_t length;
	
	if (!serial || !serial->data || !string) {
		log_pedantic("Sanity check failed.");
		return false;
	}
	
	// Set the stringer to NULL, just in case we exit early.
	*string = NULL;
	
	if (!deserialize_sz(serial, &length)) {
		log_pedantic("Could not get the length of the stringer.");
		return false;
	}
	
	// The value was NULL.
	else if (!length) {
		return true;
	}
	
	// Make sure we have data left.
	else if (st_length_get(serial->data) <= serial->position) {
		log_pedantic("Bounds violation.");
		return false;
	}
	
	else if (!(result = st_alloc(length))) {
		log_pedantic("Could not allocate memory to hold the stringer.");
		return false;
	}

	// Copy the data into the stringer.
	mm_copy(st_char_get(result), st_char_get(serial->data) + serial->position, length);
	st_length_set(result, length);
	
	// Return sucess.
	serial->position += length + 1;
	*string = result;
	
	return true;
}

/**
 * @brief	Deserialize a null-terminated string from a serialized object, and update its position.
 * @note	A null-terminated string is stored as a 64 bit length field, followed by the string contents.
 * @param	serial	a pointer to a serialized object from which the data will be extracted.
 * @param	string	a pointer to the address of a null-terminated string that will receive the de-serialized data at the serialized object's current position.
 * @param	length	a pointer to a size_t that will receive the length of the de-serialized null-terminated string on success.
 * @return	true on success or false on failure..
 */
bool_t deserialize_ns(serialization_t *serial, chr_t **string, size_t *length) {
	
	chr_t *result;
	size_t bytes;
	
	if (!serial || !serial->data || !string || !length) {
		log_pedantic("Sanity check failed.");
		return false;
	}
	
	// Set the stringer to NULL, just in case we exit early.
	*string = NULL;
	*length = 0;
	
	if (!deserialize_sz(serial, &bytes)) {
		log_pedantic("Could not get the length of the stringer.");
		return false;
	}
	
	// The value was NULL.
	else if (!bytes) {
		return true;
	}
	
	// Make sure we have data left.
	else if (st_length_get(serial->data) <= serial->position) {
		log_pedantic("Bounds violation.");
		return false;
	}
	
	else if (!(result = ns_alloc(bytes + 1))) {
		log_pedantic("Could not allocate memory to hold the stringer.");
		return false;
	}

	// Copy the data into the stringer.
	mm_copy(result, st_char_get(serial->data) + serial->position, bytes);
	
	// Return sucess.
	serial->position += bytes + 1;
	*string = result;
	*length = bytes;
	
	return true;
}
