
#include "framework.h"

short int get_octet_v4(const stringer_t *string, const unsigned short octet) {

	sizer_t length;
	sizer_t increment;
	char *start_holder;
	char *end_holder;
	unsigned short cur_octet = 1;
	unsigned short result = 0;
	
	if (string == NULL || octet < 1 || octet > 4) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a NULL string value or an invalid octet.");
		#endif
		return -1;
	}
	
	// If the length is less than seven, its guaranteed to be an invalid IPv4 address.
	length = used_st(string);
	if (length < 7) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed an IP address that is too short to be valid.");
		#endif
		return -1;
	}
	
	// Setup.
	start_holder = end_holder = data_st(string);
	
	// Make sure it starts with a digit.
	if (*end_holder < '0' || *end_holder > '9') {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed an IP address that doesn't start with a number.");
		#endif
		return -1;
	}
	
	for (increment = 0; increment < length; increment++) {
		// Check for invalid characters. Check for consecutive dots. Check for IPs ending in dots.
		if (((*end_holder < '0' || *end_holder > '9') && *end_holder != '.') || (*end_holder == '.' && increment == (length - 1)) ||
			(*end_holder == '.' && (*(end_holder + 1) < '0' || *(end_holder + 1) > '9'))) {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Passed an invalid IP address.");
			#endif
			return -1;
		}
		else if (*end_holder == '.' && cur_octet < octet) {
			cur_octet++;
			if (cur_octet == octet) {
				start_holder = end_holder + 1;
			}
		}
		else if (*end_holder == '.' && cur_octet == octet) {
			if (extract_us_ns(start_holder, end_holder - start_holder, &result) != 1) {
				#ifdef DEBUG_FRAMEWORK
				lavalog("Could not extract the octet.");
				#endif
				return -1;
			}
			increment = length;
		}
		end_holder++;
	}
	
	// Exception for the last octet.
	if (octet == 4) {
		if (extract_us_ns(start_holder, end_holder - start_holder, &result) != 1) {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Could not extract the octet.");
			#endif
			return -1;
		}
	}
	
	if (result > 255) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The octet value appears to be larger than 255.");
		#endif
		return -1;
	}
	
	return result;
	
}

short int get_octet_v4_ns(const char *string, const unsigned short octet) {

	sizer_t length;
	sizer_t increment;
	char *start_holder;
	char *end_holder;
	unsigned short cur_octet = 1;
	unsigned short result = 0;
	
	if (string == NULL || octet < 1 || octet > 4) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a NULL string value or an invalid octet.");
		#endif
		return -1;
	}
	
	// If the length is less than seven, its guaranteed to be an invalid IPv4 address.
	length = size_ns(string);
	if (length < 7) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed an IP address that is too short to be valid.");
		#endif
		return -1;
	}
	
	// Setup.
	start_holder = end_holder = (char *)string;
	
	// Make sure it starts with a digit.
	if (*end_holder < '0' || *end_holder > '9') {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed an IP address that doesn't start with a number.");
		#endif
		return -1;
	}
	
	for (increment = 0; increment < length; increment++) {
		// Check for invalid characters. Check for consecutive dots. Check for IPs ending in dots.
		if (((*end_holder < '0' || *end_holder > '9') && *end_holder != '.') || (*end_holder == '.' && increment == (length - 1)) ||
			(*end_holder == '.' && (*(end_holder + 1) < '0' || *(end_holder + 1) > '9'))) {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Passed an invalid IP address.");
			#endif
			return -1;
		}
		else if (*end_holder == '.' && cur_octet < octet) {
			cur_octet++;
			if (cur_octet == octet) {
				start_holder = end_holder + 1;
			}
		}
		else if (*end_holder == '.' && cur_octet == octet) {
			if (extract_us_ns(start_holder, end_holder - start_holder, &result) != 1) {
				#ifdef DEBUG_FRAMEWORK
				lavalog("Could not extract the octet.");
				#endif
				return -1;
			}
			increment = length;
		}
		end_holder++;
	}
	
	// Exception for the last octet.
	if (octet == 4) {
		if (extract_us_ns(start_holder, end_holder - start_holder, &result) != 1) {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Could not extract the octet.");
			#endif
			return -1;
		}
	}
	
	if (result > 255) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The octet value appears to be larger than 255.");
		#endif
		return -1;
	}
	
	return result;
	
}
