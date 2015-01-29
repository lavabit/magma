
#include "framework.h"

// This function is used to move bytes around inside of a buffer.
inline void move_bytes(char *start, char *from, int length) {
	while (length--) {
		*start++ = *from++;
	}	
}

// Clear a block of memory.
inline void clear_bl(void *buffer, sizer_t size) {
	
	if (buffer == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in a NULL pointer.");
		#endif
		return;
	}
	
	while (size) {
		*((unsigned char *) (buffer++)) = 0;
		size--;
	}
}

// Clear a block of memory.
inline void clear_st(stringer_t *string) {
	
	sizer_t size;
	char *holder;
	
	if (string == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in a NULL pointer.");
		#endif
		return;
	}
	
	size = size_st(string);
	holder = data_st(string);
	
	while (size) {
		*((unsigned char *) (holder++)) = 0;
		size--;
	}
	
	set_used_st(string, 0);
	
	return;
}
