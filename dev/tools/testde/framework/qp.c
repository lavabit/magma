
#include "framework.h"

stringer_t * encode_qp_st(const stringer_t *string) {
	
	char hex[2];
	sizer_t size;
	sizer_t position;
	sizer_t add = 0;
	sizer_t used = 0;
	stringer_t *result;
	unsigned char *input_holder;
	unsigned char *output_holder;
	
	// Input validation.
	if (string == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return NULL;
	}
	
	// Size.
	size = used_st(string);
	if (size == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Asked to encode a zero length string.");
		#endif
		return NULL;
	}
	
	// Get setup.
	input_holder = data_st(string);
	
	// Increment through the stringer and count the characters which need to be encoded.
	for(position = 0; position < size; ++position) {
		if ((*input_holder >= '!' && *input_holder <= '~' && *input_holder != '=') || *input_holder == '\n' \
			|| *input_holder == '\r' || *input_holder == '\t' || *input_holder == ' ') {
			input_holder++;
		}
		else {
			add++;
			input_holder++;
		}
	}
	
	// Allocate the amount of space need to store the hex values.
	result = allocate_st(size + (2 * add));
	if (result == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not allocate a stringer large enough to encode the data.");
		#endif
		return NULL;
	}
	
	// Get setup.
	input_holder = data_st(string);
	output_holder = data_st(result);
	
	// Increment through the stringer and copy the data into the new stringer.
	for(position = 0; position < size; ++position) {
		if ((*input_holder >= '!' && *input_holder <= '~' && *input_holder != '=') || *input_holder == '\n' \
			|| *input_holder == '\r' || *input_holder == '\t' || *input_holder == ' ') {
			*output_holder++ = *input_holder++;
			used++;
		}
		else {
			*output_holder++ = '=';
			encode_hex_c(*input_holder++, &hex[0]);
			*output_holder++ = hex[0];
			*output_holder++ = hex[1];
			used += 3;
		}
	}
	
	// Record how big the new string is.
	set_used_st(result, used);
	
	return result;
}

stringer_t * decode_qp_st(const stringer_t *string) {
	
	sizer_t length;
	sizer_t increment;
	sizer_t result_length = 0;
	stringer_t *result;
	char *input_holder;
	char *output_holder;
	
	// Input validation.
	if (string == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return NULL;
	}
	
	length = used_st(string);
	if (length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Asked to decode a string of zero length.");
		#endif
		return NULL;
	}
	
	result = allocate_st(length);
	if (result == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not allocate a buffer for the decoded result.");
		#endif
		return NULL;
	}
	
	input_holder = data_st(string);
	output_holder = data_st(result);
	
	for (increment = 0; increment < length; increment++) {
		// We found a QP character.
		if (*(input_holder + increment) == '=') {
			// Make sure we have at least three characters left in the buffer.
			if (length - increment > 3) {
				increment++;
				// Look for lines which end in =\r\n; when found, skip them. They are 'soft' line breaks.
				if (*(input_holder + increment) == '\r' && *(input_holder + increment + 1) == '\n') {
					increment++;
				}
				// If its a valid hex character.
				else if ((*(input_holder + increment) >= '0' && *(input_holder + increment) <= '9') || (*(input_holder + increment) >= 'A' && *(input_holder + increment) <= 'F') || \
					(*(input_holder + increment) >= 'A' && *(input_holder + increment) <= 'F')) {
					*(output_holder++) = decode_hex_c((char *)input_holder + increment);
					result_length++;
					// Make sure both characters were valid hex. If so increment by 2. Otherwise advance by one.
					if ((*(input_holder + increment + 1) >= '0' && *(input_holder + increment + 1) <= '9') || (*(input_holder + increment + 1)  >= 'A' && \
						*(input_holder + increment + 1)  <= 'F') || (*(input_holder + increment + 1)  >= 'A' && *(input_holder + increment + 1)  <= 'F')) {
						increment++;
					}
				}
				// Was an invalid hex sequence, so just copy it through.
				else {
					*(output_holder++) = '=';
					result_length++;
					*(output_holder++) = *(input_holder + increment);
					result_length++;
				}
			}
		}
		// A non-QP character, so just copy it over.
		else {
			*(output_holder++) = *(input_holder + increment);;
			result_length++;
		}
	}
	
	set_used_st(result, result_length);	
	
	return result;
}

stringer_t * encode_qp_ns(const char *string) {
	
	char hex[2];
	sizer_t size;
	sizer_t position;
	sizer_t add = 0;
	sizer_t used = 0;
	stringer_t *result;
	unsigned char *output_holder;
	
	// Input validation.
	if (string == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return NULL;
	}
	
	// Size.
	size = size_ns(string);
	if (size == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Asked to encode a zero length string.");
		#endif
		return NULL;
	}

	// Increment through the stringer and count the characters which need to be encoded.
	for(position = 0; position < size; position++) {
		if ((*(string + position) >= '!' && *(string + position) <= '~' && *(string + position) != '=') || *(string + position) == '\n' \
			|| *(string + position) == '\r' || *(string + position) == '\t' || *(string + position) == ' ') {
			add++;
		}
		else {
			add += 3;
		}
	}
	
	// Allocate the amount of space need to store the hex values.
	result = allocate_st(add);
	if (result == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not allocate a stringer large enough to encode the data.");
		#endif
		return NULL;
	}
	
	// Get setup.
	output_holder = data_st(result);
	
	// Increment through the stringer and copy the data into the new stringer.
	for(position = 0; position < size; position++) {
		if ((*(string + position) >= '!' && *(string + position) <= '~' && *(string + position) != '=') || *(string + position) == '\n' \
			|| *(string + position) == '\r' || *(string + position) == '\t' || *(string + position) == ' ') {
			*output_holder++ = *(string + position);
			used++;
		}
		else {
			*output_holder++ = '=';
			encode_hex_c(*(string + position), &hex[0]);
			*output_holder++ = hex[0];
			*output_holder++ = hex[1];
			used += 3;
		}
	}
	
	// Record how big the new string is.
	set_used_st(result, used);
	
	return result;
}

stringer_t * decode_qp_ns(const char *string) {
	
	sizer_t length;
	sizer_t increment;
	sizer_t result_length = 0;
	stringer_t *result;
	char *output_holder;
	
	// Input validation.
	if (string == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return NULL;
	}
	
	length = size_ns(string);
	if (length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Asked to decode a string of zero length.");
		#endif
		return NULL;
	}
	
	result = allocate_st(length);
	if (result == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not allocate a buffer for the decoded result.");
		#endif
		return NULL;
	}
	
	output_holder = data_st(result);
	
	for (increment = 0; increment < length; increment++) {
		// We found a QP character.
		if (*(string + increment) == '=') {
			// Make sure we have at least three characters left in the buffer.
			if (length - increment > 3) {
				increment++;
				// Look for lines which end in =\r\n; when found, skip them. They are 'soft' line breaks.
				if (*(string + increment) == '\r' && *(string + increment + 1) == '\n') {
					increment++;
				}
				// If its a valid hex character.
				else if ((*(string + increment) >= '0' && *(string + increment) <= '9') || (*(string + increment) >= 'A' && *(string + increment) <= 'F') || \
					(*(string + increment) >= 'A' && *(string + increment) <= 'F')) {
					*(output_holder++) = decode_hex_c((char *)string + increment);
					result_length++;
					// Make sure both characters were valid hex. If so increment by 2. Otherwise advance by one.
					if ((*(string + increment + 1) >= '0' && *(string + increment + 1) <= '9') || (*(string + increment + 1)  >= 'A' && \
						*(string + increment + 1)  <= 'F') || (*(string + increment + 1)  >= 'A' && *(string + increment + 1)  <= 'F')) {
						increment++;
					}
				}
				// Was an invalid hex sequence, so just copy it through.
				else {
					*(output_holder++) = '=';
					result_length++;
					*(output_holder++) = *(string + increment);
					result_length++;
				}
			}
		}
		// A non-QP character, so just copy it over.
		else {
			*(output_holder++) = *(string + increment);;
			result_length++;
		}
	}
	
	set_used_st(result, result_length);	
	
	return result;
}

/*
stringer_t * encode_qp_st_amt(const stringer_t *string, sizer_t amount);
stringer_t * decode_qp_st_amt(const stringer_t *string, sizer_t amount);
stringer_t * encode_qp_ns_amt(const char *string, sizer_t amount);
stringer_t * decode_qp_ns_amt(const char *string, sizer_t amount);
*/

stringer_t * encode_qp_st_amt(const stringer_t *string, const sizer_t amount) {
	
	char hex[2];
	sizer_t size;
	sizer_t position;
	sizer_t add = 0;
	sizer_t used = 0;
	stringer_t *result;
	unsigned char *input_holder;
	unsigned char *output_holder;
	
	// Input validation.
	if (string == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return NULL;
	}
	
	// Size.
	size = used_st(string);
	if (size == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Asked to encode a zero length string.");
		#endif
		return NULL;
	}
	
	if (amount > size) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Asked to encode more data than is being used in the stringer.");
		#endif
		return 0;
	}
	
	if (amount < size) {
		size = amount;
	}
	
	// Get setup.
	input_holder = data_st(string);
	
	// Increment through the stringer and count the characters which need to be encoded.
	for(position = 0; position < size; ++position) {
		if ((*input_holder >= '!' && *input_holder <= '~' && *input_holder != '=') || *input_holder == '\n' \
			|| *input_holder == '\r' || *input_holder == '\t' || *input_holder == ' ') {
			input_holder++;
		}
		else {
			add++;
			input_holder++;
		}
	}
	
	// Allocate the amount of space need to store the hex values.
	result = allocate_st(size + (2 * add));
	if (result == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not allocate a stringer large enough to encode the data.");
		#endif
		return NULL;
	}
	
	// Get setup.
	input_holder = data_st(string);
	output_holder = data_st(result);
	
	// Increment through the stringer and copy the data into the new stringer.
	for(position = 0; position < size; ++position) {
		if ((*input_holder >= '!' && *input_holder <= '~' && *input_holder != '=') || *input_holder == '\n' \
			|| *input_holder == '\r' || *input_holder == '\t' || *input_holder == ' ') {
			*output_holder++ = *input_holder++;
			used++;
		}
		else {
			*output_holder++ = '=';
			encode_hex_c(*input_holder++, &hex[0]);
			*output_holder++ = hex[0];
			*output_holder++ = hex[1];
			used += 3;
		}
	}
	
	// Record how big the new string is.
	set_used_st(result, used);
	
	return result;
}

stringer_t * decode_qp_st_amt(const stringer_t *string, const sizer_t amount) {	
	
	sizer_t length;
	sizer_t increment;
	sizer_t result_length = 0;
	stringer_t *result;
	char *input_holder;
	char *output_holder;
	
	// Input validation.
	if (string == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return NULL;
	}
	
	length = used_st(string);
	if (length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Asked to decode a string of zero length.");
		#endif
		return NULL;
	}
	
	if (amount > length) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Asked to copy more data than is being used in the stringer.");
		#endif
		return 0;
	}
	
	if (amount < length) {
		length = amount;
	}
	
	result = allocate_st(length);
	if (result == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not allocate a buffer for the decoded result.");
		#endif
		return NULL;
	}
	
	input_holder = data_st(string);
	output_holder = data_st(result);
	
	for (increment = 0; increment < length; increment++) {
		// We found a QP character.
		if (*(input_holder + increment) == '=') {
			// Make sure we have at least three characters left in the buffer.
			if (length - increment > 3) {
				increment++;
				// Look for lines which end in =\r\n; when found, skip them. They are 'soft' line breaks.
				if (*(input_holder + increment) == '\r' && *(input_holder + increment + 1) == '\n') {
					increment++;
				}
				// If its a valid hex character.
				else if ((*(input_holder + increment) >= '0' && *(input_holder + increment) <= '9') || (*(input_holder + increment) >= 'A' && *(input_holder + increment) <= 'F') || \
					(*(input_holder + increment) >= 'A' && *(input_holder + increment) <= 'F')) {
					*(output_holder++) = decode_hex_c((char *)input_holder + increment);
					result_length++;
					// Make sure both characters were valid hex. If so increment by 2. Otherwise advance by one.
					if ((*(input_holder + increment + 1) >= '0' && *(input_holder + increment + 1) <= '9') || (*(input_holder + increment + 1)  >= 'A' && \
						*(input_holder + increment + 1)  <= 'F') || (*(input_holder + increment + 1)  >= 'A' && *(input_holder + increment + 1)  <= 'F')) {
						increment++;
					}
				}
				// Was an invalid hex sequence, so just copy it through.
				else {
					*(output_holder++) = '=';
					result_length++;
					*(output_holder++) = *(input_holder + increment);
					result_length++;
				}
			}
		}
		// A non-QP character, so just copy it over.
		else {
			*(output_holder++) = *(input_holder + increment);;
			result_length++;
		}
	}
	
	set_used_st(result, result_length);	
	
	return result;
}

stringer_t * encode_qp_ns_amt(const char *string, const sizer_t amount) {
	
	char hex[2];
	sizer_t position;
	sizer_t add = 0;
	sizer_t used = 0;
	stringer_t *result;
	unsigned char *output_holder;
	
	// Input validation.
	if (string == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return NULL;
	}

	// Increment through the stringer and count the characters which need to be encoded.
	for(position = 0; position < amount; position++) {
		if ((*(string + position) >= '!' && *(string + position) <= '~' && *(string + position) != '=') || *(string + position) == '\n' \
			|| *(string + position) == '\r' || *(string + position) == '\t' || *(string + position) == ' ') {
			add++;
		}
		else {
			add += 3;
		}
	}
	
	// Allocate the amount of space need to store the hex values.
	result = allocate_st(add);
	if (result == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not allocate a stringer large enough to encode the data.");
		#endif
		return NULL;
	}
	
	// Get setup.
	output_holder = data_st(result);
	
	// Increment through the stringer and copy the data into the new stringer.
	for(position = 0; position < amount; position++) {
		if ((*(string + position) >= '!' && *(string + position) <= '~' && *(string + position) != '=') || *(string + position) == '\n' \
			|| *(string + position) == '\r' || *(string + position) == '\t' || *(string + position) == ' ') {
			*output_holder++ = *(string + position);
			used++;
		}
		else {
			*output_holder++ = '=';
			encode_hex_c(*(string + position), &hex[0]);
			*output_holder++ = hex[0];
			*output_holder++ = hex[1];
			used += 3;
		}
	}
	
	// Record how big the new string is.
	set_used_st(result, used);
	
	return result;
}

stringer_t * decode_qp_ns_amt(const char *string, const sizer_t amount) {
	
	sizer_t increment;
	sizer_t result_length = 0;
	stringer_t *result;
	char *output_holder;
	
	// Input validation.
	if (string == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return NULL;
	}

	result = allocate_st(amount);
	if (result == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not allocate a buffer for the decoded result.");
		#endif
		return NULL;
	}
	
	output_holder = data_st(result);
	
	for (increment = 0; increment < amount; increment++) {
		// We found a QP character.
		if (*(string + increment) == '=') {
			// Make sure we have at least three characters left in the buffer.
			if (amount - increment > 3) {
				increment++;
				// Look for lines which end in =\r\n; when found, skip them. They are 'soft' line breaks.
				if (*(string + increment) == '\r' && *(string + increment + 1) == '\n') {
					increment++;
				}
				// If its a valid hex character.
				else if ((*(string + increment) >= '0' && *(string + increment) <= '9') || (*(string + increment) >= 'A' && *(string + increment) <= 'F') || \
					(*(string + increment) >= 'A' && *(string + increment) <= 'F')) {
					*(output_holder++) = decode_hex_c((char *)string + increment);
					result_length++;
					// Make sure both characters were valid hex. If so increment by 2. Otherwise advance by one.
					if ((*(string + increment + 1) >= '0' && *(string + increment + 1) <= '9') || (*(string + increment + 1)  >= 'A' && \
						*(string + increment + 1)  <= 'F') || (*(string + increment + 1)  >= 'A' && *(string + increment + 1)  <= 'F')) {
						increment++;
					}
				}
				// Was an invalid hex sequence, so just copy it through.
				else {
					*(output_holder++) = '=';
					result_length++;
					*(output_holder++) = *(string + increment);
					result_length++;
				}
			}
		}
		// A non-QP character, so just copy it over.
		else {
			*(output_holder++) = *(string + increment);;
			result_length++;
		}
	}
	
	set_used_st(result, result_length);	
	
	return result;
}
