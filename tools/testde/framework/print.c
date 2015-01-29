
#include "framework.h"

stringer_t * merge_strings(const char *format, ...) {

	va_list args;
	char *nuller;
	placer_t placer;
	sizer_t length = 0;
	sizer_t current_length;
	sizer_t inner_increment;
	unsigned int number;
	unsigned int increment;
	stringer_t *result;
	stringer_t *stringer;
	unsigned char *result_position;
	unsigned char *current_position;
	#ifdef DEBUG_FRAMEWORK
	sizer_t total = 0;
	#endif

	if (format == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return NULL;
	}

	// How many arguments were passed in.
	number = size_ns(format);
	if (number == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A according to the format, zero arguments were passed in.");
		#endif
		return NULL;
	}

	// Iterate through and determine how long this combined string should be.
	va_start(args, format);
	for (increment = 0; increment < number; increment++) {
		if (*(format + increment) == 's') {
			stringer = va_arg(args, stringer_t *);
			if (stringer == NULL) {
				#ifdef DEBUG_FRAMEWORK
				lavalog("According to the format, there should have been another variable, but none was found.");
				#endif
				return NULL;
			}
			length += used_st(stringer);
		}
		else if (*(format + increment) == 'n') {
			nuller = va_arg(args, char *);
			if (nuller == NULL) {
				#ifdef DEBUG_FRAMEWORK
				lavalog("According to the format, there should have been another variable, but none was found.");
				#endif
				return NULL;
			}
			length += size_ns(nuller);
		}
		else if (*(format + increment) == 'p') {
			placer = va_arg(args, placer_t);
			if (placer.size == 0) {
				#ifdef DEBUG_FRAMEWORK
				lavalog("According to the format, there should have been another variable, but none was found.");
				#endif
				return NULL;
			}
			length += size_pl(placer);
		}
		else {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Invalid type code passed in.");
			#endif
			return NULL;
		}
	}
	va_end(args);

	// Allocate a buffer for the new string.
	result = allocate_st(length);
	if (result == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to allocate %u bytes for the combined string.", length);
		#endif
		return NULL;
	}

	// Setup.
	result_position = data_st(result);

	va_start(args, format);
	for (increment = 0; increment < number; increment++) {
		if (*(format + increment) == 's') {
			stringer = va_arg(args, stringer_t *);
			if (stringer == NULL) {
				#ifdef DEBUG_FRAMEWORK
				lavalog("According to the format, there should have been another variable, but none was found.");
				#endif
				free_st(result);
				return NULL;
			}
			current_length = used_st(stringer);
			current_position = data_st(stringer);
			for (inner_increment = 0; inner_increment < current_length; inner_increment++) {
				*(result_position++) = *(current_position++);
				#ifdef DEBUG_FRAMEWORK
				if (++total > length) {
					lavalog("We've exceeded the space in the allocated buffer. This should never happen.");
					free_st(result);
					return NULL;
				}
				#endif
			}
		}
		else if (*(format + increment) == 'n') {
			nuller = va_arg(args, char *);
			if (nuller == NULL) {
				#ifdef DEBUG_FRAMEWORK
				lavalog("According to the format, there should have been another variable, but none was found.");
				#endif
				free_st(result);
				return NULL;
			}
			current_length = size_ns(nuller);
			current_position = nuller;
			for (inner_increment = 0; inner_increment < current_length; inner_increment++) {
				*(result_position++) = *(current_position++);
				#ifdef DEBUG_FRAMEWORK
				if (++total > length) {
					lavalog("We've exceeded the space in the allocated buffer. This should never happen.");
					free_st(result);
					return NULL;
				}
				#endif
			}
		}
		else if (*(format + increment) == 'p') {
			placer = va_arg(args, placer_t);
			if (placer.size == 0) {
				#ifdef DEBUG_FRAMEWORK
				lavalog("According to the format, there should have been another variable, but none was found.");
				#endif
				free_st(result);
				return NULL;
			}
			current_length = size_pl(placer);
			current_position = data_pl(placer);
			for (inner_increment = 0; inner_increment < current_length; inner_increment++) {
				*(result_position++) = *(current_position++);
				#ifdef DEBUG_FRAMEWORK
				if (++total > length) {
					lavalog("We've exceeded the space in the allocated buffer. This should never happen.");
					free_st(result);
					return NULL;
				}
				#endif
			}
		}
		else {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Invalid format. This error should have been caught above.");
			#endif
			free_st(result);
			return NULL;
		}
	}
	va_end(args);

	set_used_st(result, length);

	return result;
}

sizer_t sprintf_st(stringer_t *string, const char *format, ...) {

	sizer_t size;
	va_list args;

	if (string == NULL || format == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in. Returning error.");
		#endif
		return 0;
	}

	// Initialize our dynamic array.
	va_start(args, format);

	// Print into the string.
	size = vsnprintf(data_st(string), size_st(string), format, args);

	// Cleanup.
	va_end(args);

	// Set the size amount in the stringer.
	set_used_st(string, size);

	return size;
}
