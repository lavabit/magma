
#include "framework.h"

inline unsigned char * data_st(const stringer_t *string) {
	
	if (string == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return NULL;
	}
	
	return (unsigned char *)(string + sizeof(sizer_t) + sizeof(sizer_t));
}

inline sizer_t size_st(const stringer_t *string) {
	
	sizer_t size;
	
	if (string == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return 0;
	}
	
	size = *(sizer_t *)string;
	
	return size;
	
}

inline sizer_t used_st(const stringer_t *string) {
	
	sizer_t size;
	
	if (string == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return 0;
	}
	
	size = *(sizer_t *)(string + sizeof(sizer_t));
	
	return size;
	
}

inline void set_used_st(stringer_t *string, const sizer_t used) {
	
	if (string == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return;
	}
	
	#ifdef DEBUG_FRAMEWORK
	if (used > size_st(string)) {
		lavalog("Trying to set the used variable to %u when the allocated size is %u.", used, size_st(string));
	}
	#endif 

	*(sizer_t *)(string + sizeof(sizer_t)) = used;
	
	return;
}
	
inline sizer_t size_ns(const unsigned char *string) {
	
	sizer_t size = 0;
	
	if (string == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return 0;
	}
	
	// Loop until we hit the NULL, then return the length.
	while (*((char *) string++) != '\0') {
		size++;
	}
	
	return size;
}

inline void free_bl(void *buffer) {
	
	if (buffer == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in a NULL pointer.");
		#endif
	}
	else {
		free(buffer);
	}
	
	return;
}

inline void free_ns(char *string) {
	
	if (string == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in a NULL pointer.");
		#endif
	}
	else {
		free(string);
	}
	
	return;
}

inline void free_st(stringer_t *string) {
	
	if (string == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in a NULL pointer.");
		#endif
	}
	else {
		free(string);
	}
	
	return;
}

char * allocate_ns(const sizer_t size) {
	
	char *result;

	// No zero length strings.
	if (size == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Attempted to allocate a zero length string. Returning NULL.");
		#endif
		return NULL;
	}
	
	// Do the allocation.
	result = malloc(size);
	
	// If memory was allocated clear.
	if (result != NULL) {
		clear_bl(result, size);
	}
	
	// If no memory was allocated, discover that here.
	#ifdef DEBUG_FRAMEWORK
	else {
		lavalog("Could not allocate %u bytes.", size);
	}
	#endif
	
	return result;
}

char * reallocate_ns(char *string, const sizer_t size) {
	
	char *result;
	
	if (string == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in a NULL pointer.");
		#endif
		return NULL;
	}

	// No zero length strings.
	if (size == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Attempted to reallocate a zero length string. Use free_ns instead.");
		#endif
		return NULL;
	}
	
	// Do the allocation.
	result = realloc(string, size);
	
	#ifdef DEBUG_FRAMEWORK
	if (result == NULL) {
		lavalog("Could not allocate %u bytes.", size);
	}
	#endif
		
	return result;
}

stringer_t * allocate_st(const sizer_t size) {
	
	stringer_t *result;

	// No zero length strings.
	if (size == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Attempted to allocate a zero length string.");
		#endif
		return NULL;
	}
	
	// Do the allocation. Include room for two sizer_ts plus a terminating NULL.
	result = malloc(sizeof(sizer_t) + sizeof(sizer_t) + size + 1);
	
	// If memory was allocated clear.
	if (result != NULL) {
		clear_bl(result, sizeof(sizer_t) + sizeof(sizer_t) + size + 1);
		*(sizer_t *)result = size;
	}
	
	// If no memory was allocated, discover that here.
	#ifdef DEBUG_FRAMEWORK
	else {
		lavalog("Could not allocate %u bytes.", sizeof(sizer_t) + sizeof(sizer_t) + size + 1);
	}
	#endif
	
	return result;
}

stringer_t * reallocate_st(stringer_t *string, const sizer_t size) {

	stringer_t *result;
	sizer_t original;
	
	if (string == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in a NULL pointer.");
		#endif
		return NULL;
	}
	
	// No zero length strings.
	if (size == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Attempted to reallocate a zero length string. Use free_st instead.");
		#endif
		return NULL;
	}
	
	// Store the original stringer size.
	original = size_st(string);
	
	// Do the allocation. Include room for two sizer_ts.
	result = realloc(string, sizeof(sizer_t) + sizeof(sizer_t) + size + 1);
	
	// If memory was allocated clear.
	if (result != NULL && size > original) {
		clear_bl(result + original + sizeof(sizer_t) + sizeof(sizer_t), size + 1 - original);
		*(sizer_t *)result = size;
	}
	else if (result != NULL) {
		*(sizer_t *)result = size;
	}
	#ifdef DEBUG_FRAMEWORK
	else {
		lavalog("Could not allocate %u bytes.", sizeof(sizer_t) + sizeof(sizer_t) + size + 1);
	}
	#endif
		
	return result;
	
}

stringer_t * import_ns(const char *string) {
	
	int state;
	sizer_t size;
	stringer_t *result;
	
	// No zero length strings.
	if (string == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in a NULL pointer.");
		#endif
		return NULL;
	}
	
	// Get the string length.
	size = size_ns(string);
	
	if (size == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A zero length string was passed in.");
		#endif
		return NULL;
	}
	
	// Allocate a stringer.
	result = allocate_st(size);
	if (result == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not create a stringer.");
		#endif
		return NULL;
	}
	
	// Write the block string into the stringer.
	state = copy_st_ns_amt(result, string, size);
	if (state != 1) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not copy the NULL string into the stringer.");
		#endif
		free_st(result);
		return NULL;
	}
		
	return result;
}

stringer_t * import_bl(const void *block, const sizer_t size) {
	
	int state;
	stringer_t *result;
	
	if (block == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in a NULL pointer.");
		#endif
		return NULL;
	}
	
	if (size == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A zero length block was passed in.");
		#endif
		return NULL;
	}
	
	// Allocate a stringer.
	result = allocate_st(size);
	if (result == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not create a stringer.");
		#endif
		return NULL;
	}
	
	// Write the block string into the stringer.
	state = copy_st_ns_amt(result, block, size);
	if (state != 1) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not copy block of data into the stringer.");
		#endif
		free_st(result);
		return NULL;
	}
	
	return result;
}

sizer_t export_ns(char **target, const stringer_t *string) {
	
	int state;
	char *result;
	sizer_t size;
	
	if (string == NULL || target == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in a NULL pointer.");
		#endif
		return 0;
	}
	
	// In case we exit early.
	*target = NULL;
	
	size = used_st(string);
	if (size == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in a stringer that is using zero bytes.");
		#endif
		return 0;
	}
	
	result = allocate_ns(size + 1);
	if (result == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to allocate a buffer of %u bytes.", size + 1);
		#endif
		return 0;
	}
	
	state = copy_ns_st_amt(result, string, size);
	if (state != 1) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to copy the stringer into the buffer.");
		#endif
		free_ns(result);
		return 0;
	}
	
	*target = result;
	
	return size;
}

void * allocate_bl(const sizer_t size) {
	
	void *result;

	// No zero length strings.
	if (size == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Attempted to allocate a zero length string. Returning NULL.");
		#endif
		return NULL;
	}
	
	// Do the allocation.
	result = malloc(size);
	
	// If memory was allocated clear.
	if (result != NULL) {
		clear_bl(result, size);
	}
	
	// If no memory was allocated, discover that here.
	#ifdef DEBUG_FRAMEWORK
	else {
		lavalog("Could not allocate %u bytes.", size);
	}
	#endif
	
	return result;
}

void * reallocate_bl(void *buffer, const sizer_t new_size, const sizer_t orig_size) {
	
	int state;
	char *result;
	
	if (buffer == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in a NULL pointer.");
		#endif
		return NULL;
	}

	// No zero length strings.
	if (new_size == 0 || orig_size == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Attempted to allocate a zero length string.");
		#endif
		return NULL;
	}
	
	if (new_size < orig_size) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Asked to copy more data than we will have room for in the new buffer.");
		#endif
		return NULL;
	}
	
	// Do the allocation.
	result = malloc(new_size);
	
	// If memory was allocated clear.
	if (result != NULL) {
		clear_bl(result, new_size);
	}
	
	// If no memory was allocated, discover that here.
	else {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not allocate %u bytes.", new_size);
		#endif
		return NULL;
	}
		
	// Copy the data into the new buffer.
	state = copy_ns_ns_amt(result, buffer, orig_size);
	if (state != 1) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not copy the data into the new buffer.");
		#endif
		free_bl(result);
		return NULL;
	}
		
	return result;
}
