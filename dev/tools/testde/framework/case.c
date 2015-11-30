
#include "framework.h"

inline void uppercase_ns(char *string, sizer_t length) {
	
	sizer_t increment; 
	
	if (string == NULL || length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in a NULL pointer or a zero length.");
		#endif	
		return;
	}
	
	for (increment = 0; increment < length; increment++) {
		if (*string >= 'a' && *string <= 'z') {
			*string -= 32;
		}
		string++;
	}
			
	return;
}

inline void uppercase_st(stringer_t *string) {
	
	sizer_t length;
	sizer_t increment;
	char *holder;
	
	if (string == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in a NULL pointer.");
		#endif	
		return;
	}
	
	length = used_st(string);
	if (length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in a stringer with a zero length.");
		#endif	
		return;
	}
	
	holder = data_st(string);
	
	for (increment = 0; increment < length; increment++) {
		if (*holder >= 'a' && *holder <= 'z') {
			*holder -= 32;
		}
		holder++;
	}
	
	return;
}

inline char uppercase_c(char character) {
	if (character >= 'a' && character <= 'z') {
		return character - 32;
	}		
	return character;
}

inline void lowercase_ns(char *string, sizer_t length) {
	
	sizer_t increment; 
	
	if (string == NULL || length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in a NULL pointer or a zero length.");
		#endif	
		return;
	}
	
	for (increment = 0; increment < length; increment++) {
		if (*string >= 'A' && *string <= 'Z') {
			*string += 32;
		}
		string++;
	}
			
	return;
}

inline void lowercase_st(stringer_t *string) {
	
	sizer_t length;
	sizer_t increment;
	char *holder;
	
	if (string == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in a NULL pointer.");
		#endif	
		return;
	}
	
	length = used_st(string);
	if (length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in a stringer with a zero length.");
		#endif	
		return;
	}
	
	holder = data_st(string);
	
	for (increment = 0; increment < length; increment++) {
		if (*holder >= 'A' && *holder <= 'Z') {
			*holder += 32;
		}
		holder++;
	}
	
	return;
}

inline char lowercase_c(char character) {
	if (character >= 'A' && character <= 'Z') {
		return character + 32;
	}		
	return character;
}
