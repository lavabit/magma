
#include "framework.h"

sizer_t search_st_ns(const stringer_t *haystack, const char *needle) {
	
	char *holder;
	sizer_t inner;
	sizer_t increment;
	sizer_t result = 0;
	sizer_t haystack_length;
	sizer_t needle_length;
	
	if (haystack == NULL || needle == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a NULL pointer.");
		#endif
		return 0;
	}
	
	haystack_length = used_st(haystack);
	needle_length = size_ns(needle);
	
	if (haystack_length == 0 || needle_length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a string of zero length.");
		#endif
		return 0;
	}
		
	// The haystack is too short to contain the needle.
	if (haystack_length < needle_length) {
		return 0;
	}
	
	holder = data_st(haystack);
	
	for (increment = 0; increment <= haystack_length - needle_length && result == 0; increment++) {
		
		for (inner = 0; inner < needle_length; inner++) {
			if (*(holder + inner) != *(needle + inner)) {
				break;
			}
		}
		
		// In theory, inner won't be incremented to match needle_length if a non-matching character is found.
		if (inner == needle_length) {
			result = increment;
		}
		else {
			holder++;
		}
	}
	
	return result;
}

sizer_t search_st_st(const stringer_t *haystack, const stringer_t *needle) {
	
	char *holder;
	char *compare;
	sizer_t inner;
	sizer_t increment;
	sizer_t result = 0;
	sizer_t haystack_length;
	sizer_t needle_length;
	
	if (haystack == NULL || needle == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a NULL pointer.");
		#endif
		return 0;
	}
	
	haystack_length = used_st(haystack);
	needle_length = used_st(needle);
	
	if (haystack_length == 0 || needle_length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a string of zero length.");
		#endif
		return 0;
	}
		
	// The haystack is too short to contain the needle.
	if (haystack_length < needle_length) {
		return 0;
	}
	
	holder = data_st(haystack);
	compare = data_st(needle);
	
	for (increment = 0; increment <= haystack_length - needle_length && result == 0; increment++) {
		
		for (inner = 0; inner < needle_length; inner++) {
			if (*(holder + inner) != *(compare + inner)) {
				break;
			}
		}
		
		// In theory, inner won't be incremented to match needle_length if a non-matching character is found.
		if (inner == needle_length) {
			result = increment;
		}
		else {
			holder++;
		}
	}
	
	return result;
}

sizer_t search_ns_ns(const char *haystack, const char *needle) {
	
	sizer_t inner;
	sizer_t increment;
	sizer_t result = 0;
	sizer_t haystack_length;
	sizer_t needle_length;
	
	if (haystack == NULL || needle == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a NULL pointer.");
		#endif
		return 0;
	}
	
	haystack_length = size_ns(haystack);
	needle_length = size_ns(needle);
	
	if (haystack_length == 0 || needle_length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a string of zero length.");
		#endif
		return 0;
	}
		
	// The haystack is too short to contain the needle.
	if (haystack_length < needle_length) {
		return 0;
	}
	
	for (increment = 0; increment <= haystack_length - needle_length && result == 0; increment++) {
		
		for (inner = 0; inner < needle_length; inner++) {
			if (*(haystack + increment + inner) != *(needle + inner)) {
				break;
			}
		}
		
		// In theory, inner won't be incremented to match needle_length if a non-matching character is found.
		if (inner == needle_length) {
			result = increment;
		}
	}
	
	return result;
}

sizer_t search_ns_st(const char *haystack, const stringer_t *needle) {
	
	char *compare;
	sizer_t inner;
	sizer_t increment;
	sizer_t result = 0;
	sizer_t haystack_length;
	sizer_t needle_length;
	
	if (haystack == NULL || needle == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a NULL pointer.");
		#endif
		return 0;
	}
	
	haystack_length = size_ns(haystack);
	needle_length = used_st(needle);
	
	if (haystack_length == 0 || needle_length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a string of zero length.");
		#endif
		return 0;
	}
		
	// The haystack is too short to contain the needle.
	if (haystack_length < needle_length) {
		return 0;
	}
	
	compare = data_st(needle);
	
	for (increment = 0; increment <= haystack_length - needle_length && result == 0; increment++) {
		
		for (inner = 0; inner < needle_length; inner++) {
			if (*(haystack + increment + inner) != *(compare + inner)) {
				break;
			}
		}
		
		// In theory, inner won't be incremented to match needle_length if a non-matching character is found.
		if (inner == needle_length) {
			result = increment;
		}
	}
	
	return result;
}

sizer_t search_st_ns_amt(const stringer_t *haystack, sizer_t amount, const char *needle) {
	
	char *holder;
	sizer_t inner;
	sizer_t increment;
	sizer_t result = 0;
	sizer_t haystack_length;
	sizer_t needle_length;
	
	if (haystack == NULL || needle == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a NULL pointer.");
		#endif
		return 0;
	}
	
	haystack_length = used_st(haystack);
	needle_length = size_ns(needle);
	
	if (haystack_length == 0 || needle_length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a string of zero length.");
		#endif
		return 0;
	}
	
	#ifdef DEBUG_FRAMEWORK
	if (haystack_length < amount) {
		lavalog("Asked to search beyond the end of the stringer.");
	}
	#endif
	
	if (haystack_length > amount) {
		haystack_length = amount;
	}
	
		
	// The haystack is too short to contain the needle.
	if (haystack_length < needle_length) {
		return 0;
	}
	
	holder = data_st(haystack);
	
	for (increment = 0; increment <= haystack_length - needle_length && result == 0; increment++) {
		
		for (inner = 0; inner < needle_length; inner++) {
			if (*(holder + inner) != *(needle + inner)) {
				break;
			}
		}
		
		// In theory, inner won't be incremented to match needle_length if a non-matching character is found.
		if (inner == needle_length) {
			result = increment;
		}
		else {
			holder++;
		}
	}
	
	return result;
}

sizer_t search_st_st_amt(const stringer_t *haystack, sizer_t amount, const stringer_t *needle) {
	
	char *holder;
	char *compare;
	sizer_t inner;
	sizer_t increment;
	sizer_t result = 0;
	sizer_t haystack_length;
	sizer_t needle_length;
	
	if (haystack == NULL || needle == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a NULL pointer.");
		#endif
		return 0;
	}
	
	haystack_length = used_st(haystack);
	needle_length = used_st(needle);
	
	if (haystack_length == 0 || needle_length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a string of zero length.");
		#endif
		return 0;
	}
	
	#ifdef DEBUG_FRAMEWORK
	if (haystack_length < amount) {
		lavalog("Asked to search beyond the end of the stringer.");
	}
	#endif
	
	if (haystack_length > amount) {
		haystack_length = amount;
	}
	
	// The haystack is too short to contain the needle.
	if (haystack_length < needle_length) {
		return 0;
	}
	
	holder = data_st(haystack);
	compare = data_st(needle);
	
	for (increment = 0; increment <= haystack_length - needle_length && result == 0; increment++) {
		
		for (inner = 0; inner < needle_length; inner++) {
			if (*(holder + inner) != *(compare + inner)) {
				break;
			}
		}
		
		// In theory, inner won't be incremented to match needle_length if a non-matching character is found.
		if (inner == needle_length) {
			result = increment;
		}
		else {
			holder++;
		}
	}
	
	return result;
}

sizer_t search_ns_ns_amt(const char *haystack, sizer_t amount, const char *needle) {
	
	sizer_t inner;
	sizer_t increment;
	sizer_t result = 0;
	sizer_t needle_length;
	
	if (haystack == NULL || needle == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a NULL pointer.");
		#endif
		return 0;
	}

	needle_length = size_ns(needle);
	
	if (amount == 0 || needle_length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a string of zero length.");
		#endif
		return 0;
	}
		
	// The haystack is too short to contain the needle.
	if (amount < needle_length) {
		return 0;
	}
	
	for (increment = 0; increment <= amount - needle_length && result == 0; increment++) {
		for (inner = 0; inner < needle_length; inner++) {
			if (*(haystack + increment + inner) != *(needle + inner)) {
				break;
			}
		}
		
		// In theory, inner won't be incremented to match needle_length if a non-matching character is found.
		if (inner == needle_length) {
			result = increment;
		}
	}
	
	return result;
}

sizer_t search_ns_st_amt(const char *haystack, sizer_t amount, const stringer_t *needle) {
	
	char *compare;
	sizer_t inner;
	sizer_t increment;
	sizer_t result = 0;
	sizer_t haystack_length;
	sizer_t needle_length;
	
	if (haystack == NULL || needle == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a NULL pointer.");
		#endif
		return 0;
	}
	
	haystack_length = size_ns(haystack);
	needle_length = used_st(needle);
	
	if (haystack_length == 0 || needle_length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a string of zero length.");
		#endif
		return 0;
	}
	
	if (haystack_length > amount) {
		haystack_length = amount;
	}
		
	// The haystack is too short to contain the needle.
	if (haystack_length < needle_length) {
		return 0;
	}
	
	compare = data_st(needle);
	
	for (increment = 0; increment <= haystack_length - needle_length && result == 0; increment++) {
		
		for (inner = 0; inner < needle_length; inner++) {
			if (*(haystack + increment + inner) != *(compare + inner)) {
				break;
			}
		}
		
		// In theory, inner won't be incremented to match needle_length if a non-matching character is found.
		if (inner == needle_length) {
			result = increment;
		}
	}
	
	return result;
}

sizer_t search_st_ns_case(const stringer_t *haystack, const char *needle) {
	
	char *holder;
	sizer_t inner;
	sizer_t increment;
	sizer_t result = 0;
	sizer_t haystack_length;
	sizer_t needle_length;
	
	if (haystack == NULL || needle == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a NULL pointer.");
		#endif
		return 0;
	}
	
	haystack_length = used_st(haystack);
	needle_length = size_ns(needle);
	
	if (haystack_length == 0 || needle_length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a string of zero length.");
		#endif
		return 0;
	}
		
	// The haystack is too short to contain the needle.
	if (haystack_length < needle_length) {
		return 0;
	}
	
	holder = data_st(haystack);
	
	for (increment = 0; increment <= haystack_length - needle_length && result == 0; increment++) {
		
		for (inner = 0; inner < needle_length; inner++) {
			if (lowercase_c(*(holder + inner)) != lowercase_c(*(needle + inner))) {
				break;
			}
		}
		
		// In theory, inner won't be incremented to match needle_length if a non-matching character is found.
		if (inner == needle_length) {
			result = increment;
		}
		else {
			holder++;
		}
	}
	
	return result;
}

sizer_t search_st_st_case(const stringer_t *haystack, const stringer_t *needle) {
	
	char *holder;
	char *compare;
	sizer_t inner;
	sizer_t increment;
	sizer_t result = 0;
	sizer_t haystack_length;
	sizer_t needle_length;
	
	if (haystack == NULL || needle == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a NULL pointer.");
		#endif
		return 0;
	}
	
	haystack_length = used_st(haystack);
	needle_length = used_st(needle);
	
	if (haystack_length == 0 || needle_length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a string of zero length.");
		#endif
		return 0;
	}
		
	// The haystack is too short to contain the needle.
	if (haystack_length < needle_length) {
		return 0;
	}
	
	holder = data_st(haystack);
	compare = data_st(needle);
	
	for (increment = 0; increment <= haystack_length - needle_length && result == 0; increment++) {
		
		for (inner = 0; inner < needle_length; inner++) {
			if (lowercase_c(*(holder + inner)) != lowercase_c(*(compare + inner))) {
				break;
			}
		}
		
		// In theory, inner won't be incremented to match needle_length if a non-matching character is found.
		if (inner == needle_length) {
			result = increment;
		}
		else {
			holder++;
		}
	}
	
	return result;
}

sizer_t search_ns_ns_case(const char *haystack, const char *needle) {
	
	sizer_t inner;
	sizer_t increment;
	sizer_t result = 0;
	sizer_t haystack_length;
	sizer_t needle_length;
	
	if (haystack == NULL || needle == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a NULL pointer.");
		#endif
		return 0;
	}
	
	haystack_length = size_ns(haystack);
	needle_length = size_ns(needle);
	
	if (haystack_length == 0 || needle_length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a string of zero length.");
		#endif
		return 0;
	}
		
	// The haystack is too short to contain the needle.
	if (haystack_length < needle_length) {
		return 0;
	}
	
	for (increment = 0; increment <= haystack_length - needle_length && result == 0; increment++) {
		
		for (inner = 0; inner < needle_length; inner++) {
			if (lowercase_c(*(haystack + increment + inner)) != lowercase_c(*(needle + inner))) {
				break;
			}
		}
		
		// In theory, inner won't be incremented to match needle_length if a non-matching character is found.
		if (inner == needle_length) {
			result = increment;
		}
	}
	
	return result;
}

sizer_t search_ns_st_case(const char *haystack, const stringer_t *needle) {
	
	char *compare;
	sizer_t inner;
	sizer_t increment;
	sizer_t result = 0;
	sizer_t haystack_length;
	sizer_t needle_length;
	
	if (haystack == NULL || needle == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a NULL pointer.");
		#endif
		return 0;
	}
	
	haystack_length = size_ns(haystack);
	needle_length = used_st(needle);
	
	if (haystack_length == 0 || needle_length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a string of zero length.");
		#endif
		return 0;
	}
		
	// The haystack is too short to contain the needle.
	if (haystack_length < needle_length) {
		return 0;
	}
	
	compare = data_st(needle);
	
	for (increment = 0; increment <= haystack_length - needle_length && result == 0; increment++) {
		
		for (inner = 0; inner < needle_length; inner++) {
			if (lowercase_c(*(haystack + increment + inner)) != lowercase_c(*(compare + inner))) {
				break;
			}
		}
		
		// In theory, inner won't be incremented to match needle_length if a non-matching character is found.
		if (inner == needle_length) {
			result = increment;
		}
	}
	
	return result;
}

sizer_t search_st_ns_case_amt(const stringer_t *haystack, sizer_t amount, const char *needle) {
	
	char *holder;
	sizer_t inner;
	sizer_t increment;
	sizer_t result = 0;
	sizer_t haystack_length;
	sizer_t needle_length;
	
	if (haystack == NULL || needle == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a NULL pointer.");
		#endif
		return 0;
	}
	
	haystack_length = used_st(haystack);
	needle_length = size_ns(needle);
	
	if (haystack_length == 0 || needle_length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a string of zero length.");
		#endif
		return 0;
	}
	
	#ifdef DEBUG_FRAMEWORK
	if (haystack_length < amount) {
		lavalog("Asked to search beyond the end of the stringer.");
	}
	#endif
	
	if (haystack_length > amount) {
		haystack_length = amount;
	}
	
		
	// The haystack is too short to contain the needle.
	if (haystack_length < needle_length) {
		return 0;
	}
	
	holder = data_st(haystack);
	
	for (increment = 0; increment <= haystack_length - needle_length && result == 0; increment++) {
		
		for (inner = 0; inner < needle_length; inner++) {
			if (lowercase_c(*(holder + inner)) != lowercase_c(*(needle + inner))) {
				break;
			}
		}
		
		// In theory, inner won't be incremented to match needle_length if a non-matching character is found.
		if (inner == needle_length) {
			result = increment;
		}
		else {
			holder++;
		}
	}
	
	return result;
}

sizer_t search_st_st_case_amt(const stringer_t *haystack, sizer_t amount, const stringer_t *needle) {
	
	char *holder;
	char *compare;
	sizer_t inner;
	sizer_t increment;
	sizer_t result = 0;
	sizer_t haystack_length;
	sizer_t needle_length;
	
	if (haystack == NULL || needle == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a NULL pointer.");
		#endif
		return 0;
	}
	
	haystack_length = used_st(haystack);
	needle_length = used_st(needle);
	
	if (haystack_length == 0 || needle_length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a string of zero length.");
		#endif
		return 0;
	}
	
	#ifdef DEBUG_FRAMEWORK
	if (haystack_length < amount) {
		lavalog("Asked to search beyond the end of the stringer.");
	}
	#endif
	
	if (haystack_length > amount) {
		haystack_length = amount;
	}
	
	// The haystack is too short to contain the needle.
	if (haystack_length < needle_length) {
		return 0;
	}
	
	holder = data_st(haystack);
	compare = data_st(needle);
	
	for (increment = 0; increment <= haystack_length - needle_length && result == 0; increment++) {
		
		for (inner = 0; inner < needle_length; inner++) {
			if (lowercase_c(*(holder + inner)) != lowercase_c(*(compare + inner))) {
				break;
			}
		}
		
		// In theory, inner won't be incremented to match needle_length if a non-matching character is found.
		if (inner == needle_length) {
			result = increment;
		}
		else {
			holder++;
		}
	}
	
	return result;
}

sizer_t search_ns_ns_case_amt(const char *haystack, sizer_t amount, const char *needle) {
	
	sizer_t inner;
	sizer_t increment;
	sizer_t result = 0;
	sizer_t needle_length;
	
	if (haystack == NULL || needle == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a NULL pointer.");
		#endif
		return 0;
	}

	needle_length = size_ns(needle);
	
	if (amount == 0 || needle_length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a string of zero length.");
		#endif
		return 0;
	}
	
	// The haystack is too short to contain the needle.
	if (amount < needle_length) {
		return 0;
	}
	
	for (increment = 0; increment <= amount - needle_length && result == 0; increment++) {
		for (inner = 0; inner < needle_length; inner++) {
			if (lowercase_c(*(haystack + increment + inner)) != lowercase_c(*(needle + inner))) {
				break;
			}
		}
		
		// In theory, inner won't be incremented to match needle_length if a non-matching character is found.
		if (inner == needle_length) {
			result = increment;
		}
	}
	
	return result;
}

sizer_t search_ns_st_case_amt(const char *haystack, sizer_t amount, const stringer_t *needle) {
	
	char *compare;
	sizer_t inner;
	sizer_t increment;
	sizer_t result = 0;
	sizer_t haystack_length;
	sizer_t needle_length;
	
	if (haystack == NULL || needle == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a NULL pointer.");
		#endif
		return 0;
	}
	
	haystack_length = size_ns(haystack);
	needle_length = used_st(needle);
	
	if (haystack_length == 0 || needle_length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed a string of zero length.");
		#endif
		return 0;
	}
	
	if (haystack_length > amount) {
		haystack_length = amount;
	}
		
	// The haystack is too short to contain the needle.
	if (haystack_length < needle_length) {
		return 0;
	}
	
	compare = data_st(needle);
	
	for (increment = 0; increment <= haystack_length - needle_length && result == 0; increment++) {
		
		for (inner = 0; inner < needle_length; inner++) {
			if (lowercase_c(*(haystack + increment + inner)) != lowercase_c(*(compare + inner))) {
				break;
			}
		}
		
		// In theory, inner won't be incremented to match needle_length if a non-matching character is found.
		if (inner == needle_length) {
			result = increment;
		}
	}
	
	return result;
}
