/**
 * @file /stringer/stringer-check.c
 *
 * @brief A collection of functions used handle common bit manipulation tasks.
 *
 * $Author: Ladar Levison $
 * $Date: 2010/12/12 23:59:04 $
 * $Revision: a46af76362b976a414117a5bfee9dfc2e9fd38b2 $
 *
 */

#include "stringer.h"


/**
 * Is the string a valid placer. Checks for the placer type, the jointed layout, and any of the three memory
 * types. Then checks to make sure no other bits are set outside of those mentioned.
 *
 * @param opts
 * @return
 */
bool_t st_opts_placer(uint32_t opts) {

	if (!st_opts_valid(opts)) return false;
	else if (!(opts & PLACER_T) && !(opts & JOINTED) && !(opts & (STACK | HEAP | SECURE)) &&
			(opts & ~(PLACER_T | JOINTED | STACK | HEAP | SECURE))) return false;
	return true;
}



/**
 * Is the resulting string a suitable destination for storing the output of an operation.
 *
 * @param opts
 * @return
 */
bool_t st_opts_destination(uint32_t opts) {

	if (!st_opts_valid(opts)) return false;
	else if (!(opts & (NULLER_T | BLOCK_T | MANAGED_T | MAPPED_T))) return false;
	return true;
}


/**
 * Is there any data to free.
 *
 * @param opts
 * @return
 */
bool_t st_opts_free(uint32_t opts) {

	if (!st_opts_valid(opts)) return false;
	else if (opts & STACK) return false;
	return true;
}

/**
 * Is the data length tracked.
 *
 * @param opts
 * @return
 */
bool_t st_opts_tracked(uint32_t opts) {

	if (!st_opts_valid(opts)) return false;
	else if (!(opts & (PLACER_T | BLOCK_T | MANAGED_T | MAPPED_T))) return false;
	return true;
}

/**
 * Is the stringer jointed.
 *
 * @param opts
 * @return
 */
bool_t st_opts_jointed(uint32_t opts) {

	if (!st_opts_valid(opts)) return false;
	else if (!(opts & JOINTED)) return false;
	return true;
}

/**
 * Is the total buffer size tracked.
 *
 * @param opts
 * @return
 */
bool_t st_opts_avail(uint32_t opts) {

	if (!st_opts_valid(opts)) return false;
	else if (!(opts & (MANAGED_T | MAPPED_T))) return false;
	return true;
}

/**
 * Rules used to check for valid combinations of string options.
 *
 * @param opts The bit flags being checked.
 * @return Returns false if the combination of options is valid, or false if a rule is violated.
 */
bool_t st_opts_valid(uint32_t opts) {

	bool_t result = true;

	// Type
	if (bits_count(opts & (CONSTANT_T | NULLER_T | BLOCK_T | PLACER_T | MANAGED_T | MAPPED_T)) != 1) {
		result = false;
	}
	// Layout
	else if (bits_count(opts & (JOINTED | CONTIGUOUS)) != 1) {
		result = false;
	}
	// Allocation
	else if (bits_count(opts & (STACK | HEAP | SECURE)) != 1) {
		result = false;
	}


	switch (opts & (CONSTANT_T | NULLER_T | BLOCK_T | PLACER_T | MANAGED_T | MAPPED_T)) {

		// Constants must use the stack allocator, and specify a contiguous layout.
		case (CONSTANT_T):
			if (opts != (CONSTANT_T | CONTIGUOUS | STACK)) result = false;
			break;

		// Placer's must specify a jointed layout, but can use any allocation method.
		case (PLACER_T):
			if (opts & CONTIGUOUS) result = false;
			break;

		// Must use either the heap, or secure memory pool.
		case (NULLER_T):
			if (opts & STACK) result = false;
			break;

		// Must use either the heap, or secure memory pool.
		case (BLOCK_T):
			if (opts & STACK) result = false;
			break;

		// Must use either the heap, or secure memory pool.
		case (MANAGED_T):
			if (opts & STACK) result = false;
			break;

		// Mapped containers must specify a jointed layout and use the heap allocator.
		case (MAPPED_T):
			if (opts & CONTIGUOUS) result = false;
			else if (opts & STACK) result = false;
			break;

	}

	return result;
}
