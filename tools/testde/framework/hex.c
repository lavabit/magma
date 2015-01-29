
#include "framework.h"

inline char * encode_hex_c(char input, char *output) {
	
	int number;
	
	if (output == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a null output pointer.");
		#endif
		return NULL;
	}
	
	// The first value.
	number = input / 16;
	
	if (number < 10) {
		*output++ = '0' + number;
	}
	else {
		*output++ = 'A' + (number - 10);
	}
	
	// The second value.
	number = input % 16;
	
	if (number < 10) {
		*output = '0' + number;
	}
	else {
		*output = 'A' + (number - 10);
	}
	
	return output;
}

inline char decode_hex_c(char *input) {
	
	unsigned char result = '\0';
	
	if (input == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a null output pointer.");
		#endif
		return result;
	}
	
	if ((*input >= '0' && *input <= '9') || (*input >= 'A' && *input <= 'F') || (*input >= 'A' && *input <= 'F')) {
		if (*input >= '0' && *input <= '9') {
			result += (16 * (*input - '0'));
		}
		else if (*input >= 'A' && *input <= 'F') {
			result += (16 * ((*input - 'A') + 10));
		}
		else if (*input >= 'a' && *input <= 'f') {
			result += (16 * ((*input - 'a') + 10));
		}
	}
	else {
		return result;
	}
	
	input++;
	
	if ((*input >= '0' && *input <= '9') || (*input >= 'A' && *input <= 'F') || (*input >= 'A' && *input <= 'F')) {
		if (*input >= '0' && *input <= '9') {
			result += (*input - '0');
		}
		else if (*input >= 'A' && *input <= 'F') {
			result += ((*input - 'A') + 10);
		}
		else if (*input >= 'a' && *input <= 'f') {
			result += ((*input - 'a') + 10);
		}
	}
	
	return result;
}
