
#include "framework.h"

int copy_ns_ns(char *target, const stringer_t *source) {

	sizer_t size;
	sizer_t increment;
	
	if (target == NULL || source == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return 0;
	}
	
	// Get the string length.
	size = size_ns(source);
	
	#ifdef DEBUG_FRAMEWORK
	if (size == 0) {
		lavalog("The source string was of a zero length.");
	}
	#endif
	
	// Perform the write.
	for (increment = 0; increment < size + 1; increment++) {
		*(target + increment) = *(source + increment);
	}
	
	return 1;
}

int copy_ns_ns_amt(char *target, const stringer_t *source, const sizer_t amount) {

	sizer_t increment;
	
	if (target == NULL || source == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return 0;
	}
	else if (amount == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("An amount of zero was passed in.");
		#endif
		return 0;
	}
	
	// Perform the write.
	for (increment = 0; increment < amount; increment++) {
		*(target + increment) = *(source + increment);
	}
	
	return 1;
}

int copy_ns_st(char *target, const stringer_t *source) {
	
	sizer_t size;
	sizer_t increment;
	unsigned char *right;
	
	if (target == NULL || source == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return 0;
	}
	
	size = used_st(source);
	if (size == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The source stringer was of zero length.");
		#endif
		return 0;
	}
	
	// Setup.
	right = data_st(source);
	
	// Perform the write.
	for (increment = 0; increment < size; increment++) {
		*(target + increment) = *(right + increment);
	}
	
	*(target + increment + 1) = '\0';
	
	return 1;
}

int copy_ns_st_amt(char *target, const stringer_t *source, const sizer_t amount) {
	
	sizer_t increment;
	unsigned char *right;
	
	if (target == NULL || source == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return 0;
	}
	else if (amount == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("An amount of zero was passed in.");
		#endif
		return 0;
	}
	
	if (amount > used_st(source)) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Asked to copy more data than is being used in the stringer.");
		#endif
		return 0;
	}
	
	// Setup.
	right = data_st(source);
	
	// Perform the write.
	for (increment = 0; increment < amount; increment++) {
		*(target + increment) = *(right + increment);
	}
	
	return 1;
}

int copy_st_ns(stringer_t *target, const char *source) {

	sizer_t size;
	sizer_t increment;
	unsigned char *left;
	
	if (target == NULL || source == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return 0;
	}
	
	// Setup.
	size = size_ns(source);
	left = data_st(target);
	
	if (size > size_st(target)) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The source data is larger than the target.");
		#endif
		return 0;
	}
	
	// Perform the write.
	for (increment = 0; increment < size; increment++) {
		*(left + increment) = *(source + increment);
	}
	
	set_used_st(target, size);
	
	return 1;
}

int copy_st_ns_amt(stringer_t *target, const char *source, const sizer_t amount) {

	sizer_t increment;
	unsigned char *left;
	
	if (target == NULL || source == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return 0;
	}
	else if (amount == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("An amount of zero was passed in.");
		#endif
		return 0;
	}
	
	// Setup.
	left = data_st(target);
	
	if (amount > size_st(target)) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The source data is larger than the target.");
		#endif
		return 0;
	}
	
	// Perform the write.
	for (increment = 0; increment < amount; increment++) {
		*(left + increment) = *(source + increment);
	}
	
	set_used_st(target, amount);

	return 1;
}

int copy_st_st(stringer_t *target, const stringer_t *source) {

	sizer_t size;
	sizer_t increment;
	unsigned char *left;
	unsigned char *right;
	
	if (target == NULL || source == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return 0;
	}
	
	// Setup.
	size = used_st(source);
	left = data_st(target);
	right = data_st(source);
	
	if (size > size_st(target)) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The source data is larger than the target.");
		#endif
		return 0;
	}
	
	// Perform the write.
	for (increment = 0; increment < size; increment++) {
		*(left + increment) = *(right + increment);
	}
	
	set_used_st(target, size);
	
	return 1;
}

int copy_st_st_amt(stringer_t *target, const stringer_t *source, const sizer_t amount) {

	sizer_t size;
	sizer_t increment;
	unsigned char *left;
	unsigned char *right;
	
	if (target == NULL || source == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return 0;
	}
	else if (amount == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("An amount of zero was passed in.");
		#endif
		return 0;
	}
	
	// Setup.
	size = used_st(source);
	left = data_st(target);
	right = data_st(source);
	
	if (amount > size) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Asked to copy more data than is being used in the stringer.");
		#endif
		return 0;
	}
	
	if (amount < size) {
		size = amount;
	}
	
	if (size > size_st(target)) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The source data is larger than the target.");
		#endif
		return 0;
	}
	
	// Perform the write.
	for (increment = 0; increment < size; increment++) {
		*(left + increment) = *(right + increment);
	}
	
	set_used_st(target, size);
	
	return 1;
}
