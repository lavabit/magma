
#include "framework.h"

stringer_t * get_token(const stringer_t *string, const char token, const unsigned int fragment) {

	char *start;
	char *holder;
	sizer_t length;
	sizer_t increment;
	stringer_t *result;
	unsigned int position = 1;
	
	if (string == NULL || fragment == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Invalid argument passed in for tokenization.");
		#endif
		return NULL;
	}
	
	// Figure out the length.
	length = used_st(string);
	if (length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The passed in string is of zero length.");
		#endif
		return NULL;
	}
	
	holder = data_st(string);
	
	// Find the start of the desired token.
	for (increment = 0; increment < length && position < fragment; increment++) {
		if (*holder++ == token) {
			position++;
		}
	}
	
	if (position == fragment) {
		start = holder;
	}
	else {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not find the desired fragment.");
		#endif
		return NULL;
	}
	
	// Find the end of the desired fragment.
	for (; increment < length && position <= fragment; increment++) {
		if (*holder++ == token) {
			position++;
		}
	}
	
	if (holder == start) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The string appears to end where the desired fragment should begin.");
		#endif
		return NULL;
	}
	
	// Import the string. If we didn't reach the end, omit the token.
	if (increment < length) {
		result = import_bl(start, holder - start - 1);
	}
	else {
		result = import_bl(start, holder - start);
	}
	
	#ifdef DEBUG_FRAMEWORK
	if (result == NULL) {
		lavalog("An error occurred while attempting to import the fragment.");
	}
	#endif
	
	return result;
}

stringer_t * get_token_ns(const char *string, const sizer_t length, const char token, const unsigned int fragment) {

	const char *start;
	sizer_t increment;
	stringer_t *result;
	unsigned int position = 1;
	
	if (string == NULL || fragment == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Invalid argument passed in for tokenization.");
		#endif
		return NULL;
	}
	
	// Check the length.
	if (length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The passed in string is of zero length.");
		#endif
		return NULL;
	}
	
	// Find the start of the desired token.
	for (increment = 0; increment < length && position < fragment; increment++) {
		if (*(string + increment) == token) {
			position++;
		}
	}
	
	if (position == fragment) {
		start = (string + increment);
	}
	else {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not find the desired fragment.");
		#endif
		return NULL;
	}
	
	// Find the end of the desired fragment.
	for (; increment < length && position <= fragment; increment++) {
		if (*(string + increment) == token) {
			position++;
		}
	}
	
	if ((string + increment) == start) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The string appears to end where the desired fragment should begin.");
		#endif
		return NULL;
	}
	
	// Import the string. If we didn't reach the end, omit the token.
	if (increment < length) {
		result = import_bl(start, (string + increment) - start - 1);
	}
	else {
		result = import_bl(start, (string + increment) - start);
	}
	
	#ifdef DEBUG_FRAMEWORK
	if (result == NULL) {
		lavalog("An error occurred while attempting to import the fragment.");
	}
	#endif
	
	return result;
}
