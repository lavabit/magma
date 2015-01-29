
#include "framework.h"

int remove_st_ns(stringer_t **target, const char *pattern) {
	
	char *holder;
	char *new_holder;
	sizer_t hits = 0;
	sizer_t increment;
	sizer_t target_length;
	sizer_t pattern_length;
	sizer_t new_length;
	stringer_t *new_target;
	
	if (!target || !*target || !pattern) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Sanity check failed. A NULL pointer was passed in.");
		#endif
		return -1;
	}
	
	// Setup our lengths.
	pattern_length = size_ns(pattern);
	target_length = used_st(*target);
	
	// If one of the passed in strings is empty, we can't replace anything.
	if (pattern_length == 0 || target_length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Either the search pattern or the target were empty.");
		#endif
		return -2;
	}
	
	// Check to make sure the target is big enough to hold the pattern.
	if (target_length < pattern_length) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The target isn't long enough to contain the pattern.");
		#endif
		return 0;
	}
	
	// Setup.
	holder = data_st(*target);

	// Increment through the entire target and find out how many times the pattern is present.
	for (increment = 0; increment <= (target_length - pattern_length); increment++) {
		if (starts_ns_ns_amt(holder++, pattern, pattern_length) == 1) {
			hits++;
			increment += pattern_length - 1;
			holder += pattern_length - 1;
		}
	}
	
	// Did we get any hits?
	if (hits == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Searched the target and did not find the pattern.");
		#endif
		return 0;
	}
	
	// Calculate out the new length.
	new_length = target_length - (pattern_length * hits);
	
	// Allocate a new stringer.
	new_target = allocate_st(new_length);
	if (new_target == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not allocate %u bytes for the new string.", new_length);
		#endif
		return -3;
	}
	
	// Setup.
	holder = data_st(*target);
	new_holder = data_st(new_target);
	
	// Increment through the entire target and copy the bytes.
	for (increment = 0; increment <= (target_length - pattern_length); increment++) {
		if (starts_ns_ns_amt(holder, pattern, pattern_length) == 1) {
			increment += pattern_length - 1;
			holder += pattern_length;
		}
		else {
			*new_holder++ = *holder++;
		}
	}
	
	// Copy the remaining bits.
	for (; increment < target_length; increment++) {
		*new_holder++ = *holder++;
	}
	
	// Set the new stringer length.
	set_used_st(new_target, new_length);
	
	// Free the old stringer and set the new target.
	free_st(*target);
	*target = new_target;
	
	return hits;
}

int remove_st_st(stringer_t **target, const stringer_t *pattern) {
	
	char *holder;
	char *new_holder;
	sizer_t hits = 0;
	sizer_t increment;
	sizer_t target_length;
	sizer_t pattern_length;
	sizer_t new_length;
	stringer_t *new_target;
	
	if (!target || !*target || !pattern) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Sanity check failed. A NULL pointer was passed in.");
		#endif
		return -1;
	}
	
	// Setup our lengths.
	pattern_length = used_st(pattern);
	target_length = used_st(*target);
	
	// If one of the passed in strings is empty, we can't replace anything.
	if (pattern_length == 0 || target_length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Either the search pattern or the target were empty.");
		#endif
		return -2;
	}
	
	// Check to make sure the target is big enough to hold the pattern.
	if (target_length < pattern_length) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The target isn't long enough to contain the pattern.");
		#endif
		return 0;
	}
	
	// Setup.
	holder = data_st(*target);

	// Increment through the entire target and find out how many times the pattern is present.
	for (increment = 0; increment <= (target_length - pattern_length); increment++) {
		if (starts_ns_st_amt(holder++, pattern, pattern_length) == 1) {
			hits++;
			increment += pattern_length - 1;
			holder += pattern_length - 1;
		}
	}
	
	// Did we get any hits?
	if (hits == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Searched the target and did not find the pattern.");
		#endif
		return 0;
	}
	
	// Calculate out the new length.
	new_length = target_length - (pattern_length * hits);
	
	// Allocate a new stringer.
	new_target = allocate_st(new_length);
	if (new_target == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not allocate %u bytes for the new string.", new_length);
		#endif
		return -3;
	}
	
	// Setup.
	holder = data_st(*target);
	new_holder = data_st(new_target);
	
	// Increment through the entire target and copy the bytes.
	for (increment = 0; increment <= (target_length - pattern_length); increment++) {
		if (starts_ns_st_amt(holder, pattern, pattern_length) == 1) {
			increment += pattern_length - 1;
			holder += pattern_length;
		}
		else {
			*new_holder++ = *holder++;
		}
	}
	
	// Copy the remaining bits.
	for (; increment < target_length; increment++) {
		*new_holder++ = *holder++;
	}
	
	// Set the new stringer length.
	set_used_st(new_target, new_length);
	
	// Free the old stringer and set the new target.
	free_st(*target);
	*target = new_target;
	
	return hits;
}

int remove_ns_ns(char **target, const char *pattern) {
	
	char *holder;
	char *new_holder;
	char *new_target;
	sizer_t hits = 0;
	sizer_t increment;
	sizer_t target_length;
	sizer_t pattern_length;
	sizer_t new_length;
	
	if (!target || !*target || !pattern) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Sanity check failed. A NULL pointer was passed in.");
		#endif
		return -1;
	}
	
	// Setup our lengths.
	pattern_length = size_ns(pattern);
	target_length = size_ns(*target);
	
	// If one of the passed in strings is empty, we can't replace anything.
	if (pattern_length == 0 || target_length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Either the search pattern or the target were empty.");
		#endif
		return -2;
	}
	
	// Check to make sure the target is big enough to hold the pattern.
	if (target_length < pattern_length) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The target isn't long enough to contain the pattern.");
		#endif
		return 0;
	}
	
	// Setup.
	holder = *target;

	// Increment through the entire target and find out how many times the pattern is present.
	for (increment = 0; increment <= (target_length - pattern_length); increment++) {
		if (starts_ns_ns_amt(holder++, pattern, pattern_length) == 1) {
			hits++;
			increment += pattern_length - 1;
			holder += pattern_length - 1;
		}
	}
	
	// Did we get any hits?
	if (hits == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Searched the target and did not find the pattern.");
		#endif
		return 0;
	}
	
	// Calculate out the new length.
	new_length = target_length - (pattern_length * hits);
	
	// Allocate a new stringer.
	new_target = allocate_ns(new_length + 1);
	if (new_target == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not allocate %u bytes for the new string.", new_length);
		#endif
		return -3;
	}
	
	// Setup.
	holder = *target;
	new_holder = new_target;
	
	// Increment through the entire target and copy the bytes.
	for (increment = 0; increment <= (target_length - pattern_length); increment++) {
		if (starts_ns_ns_amt(holder, pattern, pattern_length) == 1) {
			increment += pattern_length - 1;
			holder += pattern_length;
		}
		else {
			*new_holder++ = *holder++;
		}
	}
	
	// Copy the remaining bits.
	for (; increment < target_length; increment++) {
		*new_holder++ = *holder++;
	}
	
	// Free the old stringer and set the new target.
	free_ns(*target);
	*target = new_target;
	
	return hits;
}

int remove_ns_st(char **target, const stringer_t *pattern) {
	
	char *holder;
	char *new_holder;
	char *new_target;
	sizer_t hits = 0;
	sizer_t increment;
	sizer_t target_length;
	sizer_t pattern_length;
	sizer_t new_length;
	
	if (!target || !*target || !pattern) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Sanity check failed. A NULL pointer was passed in.");
		#endif
		return -1;
	}
	
	// Setup our lengths.
	pattern_length = used_st(pattern);
	target_length = size_ns(*target);
	
	// If one of the passed in strings is empty, we can't replace anything.
	if (pattern_length == 0 || target_length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Either the search pattern or the target were empty.");
		#endif
		return -2;
	}
	
	// Check to make sure the target is big enough to hold the pattern.
	if (target_length < pattern_length) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The target isn't long enough to contain the pattern.");
		#endif
		return 0;
	}
	
	// Setup.
	holder = *target;

	// Increment through the entire target and find out how many times the pattern is present.
	for (increment = 0; increment <= (target_length - pattern_length); increment++) {
		if (starts_ns_st_amt(holder++, pattern, pattern_length) == 1) {
			hits++;
			increment += pattern_length - 1;
			holder += pattern_length - 1;
		}
	}
	
	// Did we get any hits?
	if (hits == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Searched the target and did not find the pattern.");
		#endif
		return 0;
	}
	
	// Calculate out the new length.
	new_length = target_length - (pattern_length * hits);
	
	// Allocate a new stringer.
	new_target = allocate_ns(new_length + 1);
	if (new_target == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not allocate %u bytes for the new string.", new_length);
		#endif
		return -3;
	}
	
	// Setup.
	holder = *target;
	new_holder = new_target;
	
	// Increment through the entire target and copy the bytes.
	for (increment = 0; increment <= (target_length - pattern_length); increment++) {
		if (starts_ns_st_amt(holder, pattern, pattern_length) == 1) {
			increment += pattern_length - 1;
			holder += pattern_length;
		}
		else {
			*new_holder++ = *holder++;
		}
	}
	
	// Copy the remaining bits.
	for (; increment < target_length; increment++) {
		*new_holder++ = *holder++;
	}
	
	// Free the old stringer and set the new target.
	free_ns(*target);
	*target = new_target;
	
	return hits;
}
