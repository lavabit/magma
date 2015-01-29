
#include "framework.h"

char * duplicate_ns(const char *string) {
	
	int state;
	sizer_t size;
	char *result;
	
	if (string == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return NULL;
	}
	
	size = size_ns(string);
	if (size == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A string of zero length was passed in.");
		#endif
		return NULL;
	}
	
	result = allocate_ns(size + 1);
	if (result == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not allocate a buffer of %u bytes.", size);
		#endif
		return NULL;
	}
	
	state = copy_ns_ns_amt(result, string, size);
	if (state != 1) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not copy the string into the new buffer.");
		#endif
		free_ns(result);
		return NULL;
	}
	
	return result;	
}

char * duplicate_ns_amt(const char *string, sizer_t amount) {
	
	int state;
	char *result;
	
	if (string == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return NULL;
	}
	else if (amount == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A string of zero length was passed in.");
		#endif
		return NULL;
	}
	
	result = allocate_ns(amount);
	if (result == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not allocate a buffer of %u bytes.", amount);
		#endif
		return NULL;
	}
	
	state = copy_ns_ns_amt(result, string, amount);
	if (state != 1) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not copy the string into the new buffer.");
		#endif
		free_ns(result);
		return NULL;
	}
	
	return result;	
}

stringer_t * duplicate_st(const stringer_t *string) {

	int state;
	sizer_t size;
	stringer_t *result;
	
	if (string == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return NULL;
	}
	
	// Get the size.
	size = size_st(string);
	if (size == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A stringer of zero length was passed in.");
		#endif
		return NULL;
	}
	
	// Make the allocation.
	result = allocate_st(size);
	if (result == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not allocate a stringer of %u bytes.", size);
		#endif
		return NULL;
	}
	
	// Find out how many bytes are being used.
	size = used_st(string);
	if (size == 0) {
		#ifdef DEBUG_STRINGER
		lavalog("The stringer has no data in the buffer. Just returning an allocated duplicate.");
		#endif
		return result;
	}
	
	// Do the copy.
	state = copy_st_st(result, string);
	if (state != 1) {
		#ifdef DEBUG_STRINGER
		lavalog("The copy failed.");
		#endif
		free_st(result);
		return NULL;
	}
	
	return result;
}
