
#include "framework.h"

int starts_st_ns(const stringer_t *left, const char *right) {
	
	char *left_holder;
	sizer_t increment;
	sizer_t left_length;
	sizer_t right_length;
	
	if (left == NULL || right == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in a NULL pointer.");
		#endif
		return -1;
	}
	
	left_length = used_st(left);
	right_length = size_ns(right);
	
	if (left_length == 0 || right_length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in an empty string.");
		#endif
		return -1;
	}
	
	if (right_length > left_length) {
		return 0;
	}
	
	left_holder = data_st(left);
	
	for (increment = 0; increment < right_length; increment++) {
		if (*(left_holder++) != *(right + increment)) {
			return 0;
		}
	}
	
	return 1;
}

int starts_ns_st(const char *left, const stringer_t *right) {
	
	char *right_holder;
	sizer_t increment;
	sizer_t left_length;
	sizer_t right_length;
	
	if (left == NULL || right == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in a NULL pointer.");
		#endif
		return -1;
	}
	
	left_length = size_ns(left);
	right_length = used_st(right);
	
	if (left_length == 0 || right_length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in an empty string.");
		#endif
		return -1;
	}
	
	if (right_length > left_length) {
		return 0;
	}
	
	right_holder = data_st(right);
	
	for (increment = 0; increment < right_length; increment++) {
		if (*(right_holder++) != *(left + increment)) {
			return 0;
		}
	}
	
	return 1;
}

int starts_st_st(const stringer_t *left, const stringer_t *right) {
	
	char *left_holder;
	char *right_holder;
	sizer_t increment;
	sizer_t left_length;
	sizer_t right_length;
	
	if (left == NULL || right == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in a NULL pointer.");
		#endif
		return -1;
	}
	
	left_length = used_st(left);
	right_length = used_st(right);
	
	if (left_length == 0 || right_length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in an empty string.");
		#endif
		return -1;
	}
	
	if (right_length > left_length) {
		return 0;
	}
	
	left_holder = data_st(left);
	right_holder = data_st(right);
	
	for (increment = 0; increment < right_length; increment++) {
		if (*(left_holder++) != *(right_holder++)) {
			return 0;
		}
	}
	
	return 1;
}

int starts_ns_ns(const char *left, const char *right) {
	
	sizer_t increment;
	sizer_t left_length;
	sizer_t right_length;
	
	if (left == NULL || right == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in a NULL pointer.");
		#endif
		return -1;
	}
	
	left_length = size_ns(left);
	right_length = size_ns(right);
	
	if (left_length == 0 || right_length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in an empty string.");
		#endif
		return -1;
	}
	
	if (right_length > left_length) {
		return 0;
	}
	
	for (increment = 0; increment < right_length; increment++) {
		if (*(left + increment) != *(right + increment)) {
			return 0;
		}
	}
	
	return 1;
}


int starts_st_ns_case(const stringer_t *left, const char *right) {
	
	char *left_holder;
	sizer_t increment;
	sizer_t left_length;
	sizer_t right_length;
	
	if (left == NULL || right == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in a NULL pointer.");
		#endif
		return -1;
	}
	
	left_length = used_st(left);
	right_length = size_ns(right);
	
	if (left_length == 0 || right_length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in an empty string.");
		#endif
		return -1;
	}
	
	if (right_length > left_length) {
		return 0;
	}
	
	left_holder = data_st(left);
	
	for (increment = 0; increment < right_length; increment++) {
		if (lowercase_c(*(left_holder++)) != lowercase_c(*(right + increment))) {
			return 0;
		}
	}
	
	return 1;
}

int starts_ns_st_case(const char *left, const stringer_t *right) {
	
	char *right_holder;
	sizer_t increment;
	sizer_t left_length;
	sizer_t right_length;
	
	if (left == NULL || right == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in a NULL pointer.");
		#endif
		return -1;
	}
	
	left_length = size_ns(left);
	right_length = used_st(right);
	
	if (left_length == 0 || right_length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in an empty string.");
		#endif
		return -1;
	}
	
	if (right_length > left_length) {
		return 0;
	}
	
	right_holder = data_st(right);
	
	for (increment = 0; increment < right_length; increment++) {
		if (lowercase_c(*(right_holder++)) != lowercase_c(*(left + increment))) {
			return 0;
		}
	}
	
	return 1;
}

int starts_st_st_case(const stringer_t *left, const stringer_t *right) {
	
	char *left_holder;
	char *right_holder;
	sizer_t increment;
	sizer_t left_length;
	sizer_t right_length;
	
	if (left == NULL || right == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in a NULL pointer.");
		#endif
		return -1;
	}
	
	left_length = used_st(left);
	right_length = used_st(right);
	
	if (left_length == 0 || right_length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in an empty string.");
		#endif
		return -1;
	}
	
	if (right_length > left_length) {
		return 0;
	}
	
	left_holder = data_st(left);
	right_holder = data_st(right);
	
	for (increment = 0; increment < right_length; increment++) {
		if (lowercase_c(*(left_holder++)) != lowercase_c(*(right_holder++))) {
			return 0;
		}
	}
	
	return 1;
}

int starts_ns_ns_case(const char *left, const char *right) {
	
	sizer_t increment;
	sizer_t left_length;
	sizer_t right_length;
	
	if (left == NULL || right == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in a NULL pointer.");
		#endif
		return -1;
	}
	
	left_length = size_ns(left);
	right_length = size_ns(right);
	
	if (left_length == 0 || right_length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in an empty string.");
		#endif
		return -1;
	}
	
	if (right_length > left_length) {
		return 0;
	}
	
	for (increment = 0; increment < right_length; increment++) {
		if (lowercase_c(*(left + increment)) != lowercase_c(*(right + increment))) {
			return 0;
		}
	}
	
	return 1;
}

int starts_st_ns_amt(const stringer_t *left, const char *right, sizer_t amount) {
	
	char *left_holder;
	sizer_t increment;
	sizer_t left_length;
	
	if (left == NULL || right == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in a NULL pointer.");
		#endif
		return -1;
	}
	
	left_length = used_st(left);
	
	if (left_length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in an empty string.");
		#endif
		return -1;
	}
	
	if (amount > left_length) {
		return 0;
	}
	
	left_holder = data_st(left);
	
	for (increment = 0; increment < amount; increment++) {
		if (*(left_holder++) != *(right + increment)) {
			return 0;
		}
	}
	
	return 1;
}

int starts_ns_st_amt(const char *left, const stringer_t *right, const sizer_t amount) {
	
	char *right_holder;
	sizer_t increment;
	sizer_t right_length;
	
	if (left == NULL || right == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in a NULL pointer.");
		#endif
		return -1;
	}
	
	right_length = used_st(right);
	
	if (right_length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in an empty string.");
		#endif
		return -1;
	}
	
	if (right_length < amount) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The right stringer is shorter than the amount asked to compare.");
		#endif
		return -1;
	}
	
	if (amount < right_length) {
		right_length = amount;
	}

	right_holder = data_st(right);
	
	for (increment = 0; increment < right_length; increment++) {
		if (*(right_holder++) != *(left + increment)) {
			return 0;
		}
	}
	
	return 1;
}

int starts_st_st_amt(const stringer_t *left, const stringer_t *right, const sizer_t amount) {
	
	char *left_holder;
	char *right_holder;
	sizer_t increment;
	sizer_t left_length;
	sizer_t right_length;
	
	if (left == NULL || right == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in a NULL pointer.");
		#endif
		return -1;
	}
	
	left_length = used_st(left);
	right_length = used_st(right);
	
	if (left_length == 0 || right_length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in an empty string.");
		#endif
		return -1;
	}
	
	if (amount < right_length) {
		right_length = amount;
	}
	
	if (right_length > left_length) {
		return 0;
	}
	
	left_holder = data_st(left);
	right_holder = data_st(right);
	
	for (increment = 0; increment < right_length; increment++) {
		if (*(left_holder++) != *(right_holder++)) {
			return 0;
		}
	}
	
	return 1;
}

int starts_ns_ns_amt(const char *left, const char *right, const sizer_t amount) {
	
	sizer_t increment;
	
	if (left == NULL || right == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in a NULL pointer.");
		#endif
		return -1;
	}
	
	for (increment = 0; increment < amount; increment++) {
		if (*(left + increment) != *(right + increment)) {
			return 0;
		}
	}
	
	return 1;
}

int starts_st_ns_case_amt(const stringer_t *left, const char *right, const sizer_t amount) {
	
	char *left_holder;
	sizer_t increment;
	sizer_t left_length;
	
	if (left == NULL || right == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in a NULL pointer.");
		#endif
		return -1;
	}
	
	left_length = used_st(left);
	
	if (left_length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in an empty string.");
		#endif
		return -1;
	}
	
	if (amount > left_length) {
		return 0;
	}
	
	left_holder = data_st(left);
	
	for (increment = 0; increment < amount; increment++) {
		if (lowercase_c(*(left_holder++)) != lowercase_c(*(right + increment))) {
			return 0;
		}
	}
	
	return 1;
}

int starts_ns_st_case_amt(const char *left, const stringer_t *right, const sizer_t amount) {
	
	char *right_holder;
	sizer_t increment;
	sizer_t right_length;
	
	if (left == NULL || right == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in a NULL pointer.");
		#endif
		return -1;
	}
	
	right_length = used_st(right);
	
	if (right_length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in an empty string.");
		#endif
		return -1;
	}
	
	if (right_length < amount) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The right stringer is shorter than the amount asked to compare.");
		#endif
		return -1;
	}
	
	if (amount < right_length) {
		right_length = amount;
	}

	right_holder = data_st(right);
	
	for (increment = 0; increment < right_length; increment++) {
		if (lowercase_c(*(right_holder++)) != lowercase_c(*(left + increment))) {
			return 0;
		}
	}
	
	return 1;
}

int starts_st_st_case_amt(const stringer_t *left, const stringer_t *right, const sizer_t amount) {
	
	char *left_holder;
	char *right_holder;
	sizer_t increment;
	sizer_t left_length;
	sizer_t right_length;
	
	if (left == NULL || right == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in a NULL pointer.");
		#endif
		return -1;
	}
	
	left_length = used_st(left);
	right_length = used_st(right);
	
	if (left_length == 0 || right_length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in an empty string.");
		#endif
		return -1;
	}
	
	if (amount < right_length) {
		right_length = amount;
	}
	
	if (right_length > left_length) {
		return 0;
	}
	
	left_holder = data_st(left);
	right_holder = data_st(right);
	
	for (increment = 0; increment < right_length; increment++) {
		if (lowercase_c(*(left_holder++)) != lowercase_c(*(right_holder++))) {
			return 0;
		}
	}
	
	return 1;
}

int starts_ns_ns_case_amt(const char *left, const char *right, const sizer_t amount) {
	
	sizer_t increment;

	if (left == NULL || right == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in a NULL pointer.");
		#endif
		return -1;
	}
	
	for (increment = 0; increment < amount; increment++) {
		if (lowercase_c(*(left + increment)) != lowercase_c(*(right + increment))) {
			return 0;
		}
	}
	
	return 1;
}
