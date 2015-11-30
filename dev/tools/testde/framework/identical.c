
#include "framework.h"

int identical_st_ns(const stringer_t *left, const char *right) {
	
	sizer_t left_size;
	sizer_t right_size;
	char *left_position;
	
	if (left == NULL || right == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return -1;
	}
	
	left_size = used_st(left);
	right_size = size_ns(right);
	
	if (left_size == 0 || right_size == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A zero length string was passed in.");
		#endif
		return -1;
	}
	
	if (left_size != right_size) {
		return 0;
	}
	
	left_position = data_st(left);
	
	for (left_size = 0; left_size < right_size; left_size++) {
		if (*(left_position++) != *(right + left_size)) {
			return 0;
		}
	}
	
	return 1;
}

int identical_st_st(const stringer_t *left, const stringer_t *right) {
	
	sizer_t left_size;
	sizer_t right_size;
	char *left_position;
	char *right_position;
	
	if (left == NULL || right == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return -1;
	}
	
	left_size = used_st(left);
	right_size = used_st(right);
	
	if (left_size == 0 || right_size == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A zero length string was passed in.");
		#endif
		return -1;
	}
	
	if (left_size != right_size) {
		return 0;
	}
	
	left_position = data_st(left);
	right_position = data_st(right);
	
	for (left_size = 0; left_size < right_size; left_size++) {
		if (*(left_position++) != *(right_position++)) {
			return 0;
		}
	}
	
	return 1;
}

int identical_ns_ns(const char *left, const char *right)  {
	
	sizer_t left_size;
	sizer_t right_size;
	
	if (left == NULL || right == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return -1;
	}
	
	left_size = size_ns(left);
	right_size = size_ns(right);
	
	if (left_size == 0 || right_size == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A zero length string was passed in.");
		#endif
		return -1;
	}
	
	if (left_size != right_size) {
		return 0;
	}
	
	for (left_size = 0; left_size < right_size; left_size++) {
		if (*(left + left_size) != *(right + left_size)) {
			return 0;
		}
	}
	
	return 1;
}

int identical_st_ns_case(const stringer_t *left, const char *right) {
	
	sizer_t left_size;
	sizer_t right_size;
	char *left_position;
	
	if (left == NULL || right == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return -1;
	}
	
	left_size = used_st(left);
	right_size = size_ns(right);
	
	if (left_size == 0 || right_size == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A zero length string was passed in.");
		#endif
		return -1;
	}
	
	if (left_size != right_size) {
		return 0;
	}
	
	left_position = data_st(left);
	
	for (left_size = 0; left_size < right_size; left_size++) {
		if (lowercase_c(*(left_position++)) != lowercase_c(*(right + left_size))) {
			return 0;
		}
	}
	
	return 1;
}

int identical_st_st_case(const stringer_t *left, const stringer_t *right) {
	
	sizer_t left_size;
	sizer_t right_size;
	char *left_position;
	char *right_position;
	
	if (left == NULL || right == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return -1;
	}
	
	left_size = used_st(left);
	right_size = used_st(right);
	
	if (left_size == 0 || right_size == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A zero length string was passed in.");
		#endif
		return -1;
	}
	
	if (left_size != right_size) {
		return 0;
	}
	
	left_position = data_st(left);
	right_position = data_st(right);
	
	for (left_size = 0; left_size < right_size; left_size++) {
		if (lowercase_c(*(left_position++)) != lowercase_c(*(right_position++))) {
			return 0;
		}
	}
	
	return 1;
}

int identical_ns_ns_case(const char *left, const char *right)  {
	
	sizer_t left_size;
	sizer_t right_size;
	
	if (left == NULL || right == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A NULL pointer was passed in.");
		#endif
		return -1;
	}
	
	left_size = size_ns(left);
	right_size = size_ns(right);
	
	if (left_size == 0 || right_size == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("A zero length string was passed in.");
		#endif
		return -1;
	}
	
	if (left_size != right_size) {
		return 0;
	}
	
	for (left_size = 0; left_size < right_size; left_size++) {
		if (lowercase_c(*(left + left_size)) != lowercase_c(*(right + left_size))) {
			return 0;
		}
	}
	
	return 1;
}
