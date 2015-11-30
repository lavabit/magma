
#include "framework.h"

extern int (*RAND_bytes_d)(unsigned char *buf, int num);

unsigned short random_us(void) {
	
	unsigned short result;
	
	if (RAND_bytes_d((unsigned char *)&result, sizeof(unsigned short)) != 1) {
		#ifdef DEBUG_FRAMEWORK 
		lavalog("Could not create a random short.");
		#endif
	}

	return result;
}

stringer_t * random_st_choices(sizer_t length, char *choices, sizer_t choices_len) {

	unsigned char *holder;
	sizer_t increment;
	stringer_t *result;
	
	if (length <= 0 || choices == NULL || choices_len <= 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed an invalid parameter.");
		#endif
		return NULL;
	}
	
	result = allocate_st(length);
	if (result == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not allocate %i bytes for the random string.", length);
		#endif
		return NULL;
	}
	
	holder = data_st(result);
	
	if (RAND_bytes_d(holder, length) != 1) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not generate a random string of bytes.");
		#endif
		return NULL;
	}
	
	// Iterate through and replace the random bytes with characters from the choices buffer.
	for (increment = 0; increment < length; increment++) {
		*holder = *(choices + (*holder % choices_len));
		holder++;
	}
	
	set_used_st(result, length);
	return result;
}
