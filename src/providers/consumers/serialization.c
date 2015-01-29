
/**
 * @file /magma/providers/consumers/serialization.c
 *
 * @brief	Distributed cache interface for serializing various data types.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Serialize a size_t into a managed string.
 * @note	This function will reallocate the output managed string if necessary, and append the serialized data, updating the length field to reflect the changes.
 * @param	data	a pointer to the address of a managed string to receive the output, which will be allocated for the caller if NULL is passed.
 * @param	number	the value of the size_t to be serialized.
 * @return	true if the specified value was serialized and stored successfully, or false on failure.
 */
bool_t serialize_sz(stringer_t **data, size_t number) {
	return serialize_uint64(data, number);
}

/**
 * @brief	Serialize an ssize_t into a managed string.
 * @note	This function will reallocate the output managed string if necessary, and append the serialized data, updating the length field to reflect the changes.
 * @param	data	a pointer to the address of a managed string to receive the output, which will be allocated for the caller if NULL is passed.
 * @param	number	the value of the ssize_t to be serialized.
 * @return	true if the specified value was serialized and stored successfully, or false on failure.
 */
bool_t serialize_ssz(stringer_t **data, ssize_t number) {
	return serialize_int64(data, number);
}

/**
 * @brief	Serialize a signed 16-bit integer into a managed string.
 * @note	This function will reallocate the output managed string if necessary, and append the serialized data, updating the length field to reflect the changes.
 * @param	data	a pointer to the address of a managed string to receive the output, which will be allocated for the caller if NULL is passed.
 * @param	number	the value of the signed 16-bit integer to be serialized.
 * @return	true if the specified value was serialized and stored successfully, or false on failure.
 */
bool_t serialize_int16(stringer_t **data, int16_t number) {
	
	stringer_t *holder = NULL;
	size_t length = int16_digits(number);
	
	// Make sure we have room to write the number.
	if (!data) {
		return false;
	}
	else if (!*data && (*data = st_alloc(1024)) == NULL) {
		return false;
	}
	else if (st_avail_get(*data) - st_length_get(*data) < (length+1) && (holder = st_realloc(*data, st_avail_get(*data) + 1024)) == NULL) {
		return false;
	}
	else if (holder) {
		*data = holder;
	}
	
	if (length != snprintf(st_char_get(*data) + st_length_get(*data), st_avail_get(*data) - st_length_get(*data), "%hi", number)) {
		return false;
	}
	
	*(st_char_get(*data) + st_length_get(*data) + length) = ';';
	st_length_set(*data, st_length_get(*data) + length + 1);
	
	return true;
}

/**
 * @brief	Serialize an unsigned 16-bit integer into a managed string.
 * @note	This function will reallocate the output managed string if necessary, and append the serialized data, updating the length field to reflect the changes.
 * @param	data	a pointer to the address of a managed string to receive the output, which will be allocated for the caller if NULL is passed.
 * @param	number	the value of the unsigned 16-bit integer to be serialized.
 * @return	true if the specified value was serialized and stored successfully, or false on failure.
 */
bool_t serialize_uint16(stringer_t **data, uint16_t number) {
	
	stringer_t *holder = NULL;
	size_t length = uint16_digits(number);

	// Make sure we have room to write the number.
	if (!data) {
		return false;
	}
	else if (!*data && (*data = st_alloc(1024)) == NULL) {
		return false;
	}
	else if (st_avail_get(*data) - st_length_get(*data) < (length+1) && (holder = st_realloc(*data, st_avail_get(*data) + 1024)) == NULL) {
		return false;
	}
	else if (holder) {
		*data = holder;
	}
	
	if (length != snprintf(st_char_get(*data) + st_length_get(*data), st_avail_get(*data) - st_length_get(*data), "%hu", number)) {
		return false;
	}
	
	*(st_char_get(*data) + st_length_get(*data) + length) = ';';
	st_length_set(*data, st_length_get(*data) + length + 1);
	
	return true;
}

/**
 * @brief	Serialize a signed 32-bit integer into a managed string.
 * @note	This function will reallocate the output managed string if necessary, and append the serialized data, updating the length field to reflect the changes.
 * @param	data	a pointer to the address of a managed string to receive the output, which will be allocated for the caller if NULL is passed.
 * @param	number	the value of the signed 32-bit integer to be serialized.
 * @return	true if the specified value was serialized and stored successfully, or false on failure.
 */
bool_t serialize_int32(stringer_t **data, int32_t number) {
	
	size_t length = int32_digits(number);
	stringer_t *holder = NULL;
	
	// Make sure we have room to write the number.
	if (!data) {
		return false;
	}
	else if (!*data && (*data = st_alloc(1024)) == NULL) {
		return false;
	}
	else if (st_avail_get(*data) - st_length_get(*data) < (length+1) && (holder = st_realloc(*data, st_avail_get(*data) + 1024)) == NULL) {
		return false;
	}
	else if (holder) {
		*data = holder;
	}
	
	if (length != snprintf(st_char_get(*data) + st_length_get(*data), st_avail_get(*data) - st_length_get(*data), "%i", number)) {
		return false;
	}
	
	*(st_char_get(*data) + st_length_get(*data) + length) = ';';
	st_length_set(*data, st_length_get(*data) + length + 1);
	
	return true;
}

/**
 * @brief	Serialize an unsigned 32-bit integer into a managed string.
 * @note	This function will reallocate the output managed string if necessary, and append the serialized data, updating the length field to reflect the changes.
 * @param	data	a pointer to the address of a managed string to receive the output, which will be allocated for the caller if NULL is passed.
 * @param	number	the value of the unsigned 32-bit integer to be serialized.
 * @return	true if the specified value was serialized and stored successfully, or false on failure.
 */
bool_t serialize_uint32(stringer_t **data, uint32_t number) {
	
	size_t length = uint32_digits(number);
	stringer_t *holder = NULL;
	
	// Make sure we have room to write the number.
	if (!data) {
		return false;
	}
	else if (!*data && (*data = st_alloc(1024)) == NULL) {
		return false;
	}
	else if (st_avail_get(*data) - st_length_get(*data) < (length+1) && (holder = st_realloc(*data, st_avail_get(*data) + 1024)) == NULL) {
		return false;
	}
	else if (holder) {
		*data = holder;
	}

	if (length != snprintf(st_char_get(*data) + st_length_get(*data), st_avail_get(*data) - st_length_get(*data), "%u", number)) {
		return false;
	}
	
	*(st_char_get(*data) + st_length_get(*data) + length) = ';';
	st_length_set(*data, st_length_get(*data) + length + 1);
	
	return true;
}
/**
 * @brief	Serialize a signed 64-bit integer into a managed string.
 * @note	This function will reallocate the output managed string if necessary, and append the serialized data, updating the length field to reflect the changes.
 * @param	data	a pointer to the address of a managed string to receive the output, which will be allocated for the caller if NULL is passed.
 * @param	number	the value of the signed 64-bit integer to be serialized.
 * @return	true if the specified value was serialized and stored successfully, or false on failure.
 */
bool_t serialize_int64(stringer_t **data, int64_t number) {
	
	size_t length = int64_digits(number);
	stringer_t *holder = NULL;
	
	// Make sure we have room to write the number.
	if (!data) {
		return false;
	}
	else if (!*data && (*data = st_alloc(1024)) == NULL) {
		return false;
	}
	else if (st_avail_get(*data) - st_length_get(*data) < (length+1) && (holder = st_realloc(*data, st_avail_get(*data) + 1024)) == NULL) {
		return false;
	}
	else if (holder) {
		*data = holder;
	}
	
	if (length != snprintf(st_char_get(*data) + st_length_get(*data), st_avail_get(*data) - st_length_get(*data), "%li", number)) {
		return false;
	}
	
	*(st_char_get(*data) + st_length_get(*data) + length) = ';';
	st_length_set(*data, st_length_get(*data) + length + 1);
	
	return true;
}

/**
 * @brief	Serialize an unsigned 64-bit integer into a managed string.
 * @note	This function will reallocate the output managed string if necessary, and append the serialized data, updating the length field to reflect the changes.
 * @param	data	a pointer to the address of a managed string to receive the output, which will be allocated for the caller if NULL is passed.
 * @param	number	the value of the unsigned 64-bit integer to be serialized.
 * @return	true if the specified value was serialized and stored successfully, or false on failure.
 */
bool_t serialize_uint64(stringer_t **data, int64_t number) {
	
	size_t length = uint64_digits(number);
	stringer_t *holder = NULL;
	
	// Make sure we have room to write the number.
	if (!data) {
		return false;
	}
	else if (!*data && (*data = st_alloc(1024)) == NULL) {
		return false;
	}
	else if (st_avail_get(*data) - st_length_get(*data) < (length+1) && (holder = st_realloc(*data, st_avail_get(*data) + 1024)) == NULL) {
		return false;
	}
	else if (holder) {
		*data = holder;
	}
	
	if (length != snprintf(st_char_get(*data) + st_length_get(*data), st_avail_get(*data) - st_length_get(*data), "%lu", number)) {
		return false;
	}
	
	*(st_char_get(*data) + st_length_get(*data) + length) = ';';
	st_length_set(*data, st_length_get(*data) + length + 1);
	
	return true;
}

/**
 * @brief	Serialize the contents of a managed string into another managed string.
 * @note	This function will reallocate the output managed string if necessary, and append the serialized data, updating the length field to reflect the changes.
 * @param	data	a pointer to the address of a managed string to receive the output, which will be allocated for the caller if NULL is passed.
 * @param	string	a pointer to a managed string containing the data to be serialized.
 * @return	true if the specified value was serialized and stored successfully, or false on failure.
 */
bool_t serialize_st(stringer_t **data, stringer_t *string) {
	
	size_t length;
	stringer_t *holder = NULL;
	
	// Make sure we have room to write the number.
	if (!data) {
		return false;
	}
	// Check for NULLs.
	if (!string || !(length = st_length_get(string))) {

		if (!serialize_sz(data, 0)) {
			return false;
		}

		return true;
	}
	// Record the length.
	else if (!serialize_sz(data, length)) {
		return false;
	}
	// Previous line ensures that *data should never be NULL.
	/*else if (!*data && (*data = st_alloc(length + (length % 1024))) == NULL) {
		return false;
	}*/
	else if (st_avail_get(*data) - st_length_get(*data) < length && (holder = st_realloc(*data, st_avail_get(*data) + length + (length % 1024))) == NULL) {
		return false;
	}
	else if (holder) {
		*data = holder;
	}
	
	mm_copy(st_char_get(*data) + st_length_get(*data), st_char_get(string), length);
	*(st_char_get(*data) + st_length_get(*data) + length) = ';';
	st_length_set(*data, st_length_get(*data) + length + 1);
	
	return true;
}

/**
 * @brief	Serialize a block of memory into a managed string.
 * @note	This function will reallocate the output managed string if necessary, and append the serialized data, updating the length field to reflect the changes.
 * @param	data	a pointer to the address of a managed string to receive the output, which will be allocated for the caller if NULL is passed.
 * @param	string	a pointer to the start of the memory block to be serialized.
 * @param	length	the length, in bytes, of the memory block to be serialized.
 * @return	1 if the block of memory was serialized and stored successfully, or -1 on failure.
 */
bool_t serialize_ns(stringer_t **data, char *string, size_t length) {
	
	stringer_t *holder = NULL;
	
	// Make sure we have room to write the number.
	if (!data) {
		return false;
	}
	// Check for NULLs.
	if (!string || !length) {

		if (!serialize_sz(data, 0)) {
			return false;
		}

		return true;
	}
	// Record the length.
	else if (!serialize_sz(data, length)) {
		return false;
	}
	// Previous line ensures that *data should never be NULL.
	/*else if (!*data && (*data = st_alloc(length + (length % 1024))) == NULL) {
		return false;
	}*/
	else if (st_avail_get(*data) - st_length_get(*data) < length && (holder = st_realloc(*data, st_avail_get(*data) + length + (length % 1024))) == NULL) {
		return false;
	}
	else if (holder) {
		*data = holder;
	}
	
	mm_copy(st_char_get(*data) + st_length_get(*data), string, length);
	*(st_char_get(*data) + st_length_get(*data) + length) = ';';
	st_length_set(*data, st_length_get(*data) + length + 1);
	
	return true;
}
