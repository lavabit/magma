
/**
 * @file /magma/core/strings/validate.c
 *
 * @brief	A collection of functions used to validate stringer allocation option combinations.
 *
 * $Author$
 * $Date$
 * $Revision:$
 *
 */

#include "magma.h"

/**
 * @brief	A sanity check to determine whether the managed string is a valid placer.
 * @note	The following criteria must be satisfied: placer bit set, jointed bit set, either stack, heap, or secure set,
 * 			and no other bits other than these mentioned should be set.
 * @param opts	the managed string options value to be evaluated.
 * @return	true if the option value reflects a valid placer, or false otherwise.
 */
bool_t st_valid_placer(uint32_t opts) {

	if (!st_valid_opts(opts)) {
		return false;
	} else if (!(opts & PLACER_T) && !(opts & JOINTED) && !(opts & (STACK | HEAP | SECURE)) &&
			(opts & ~(PLACER_T | JOINTED | STACK | HEAP | SECURE))) {
		return false;
	}

	return true;
}

/**
 * @brief	Determine whether the managed string options allow for a data store operation.
 * @param	opts	the options value for the managed string.
 * @return	true if permitted, or false if options are invalid, or indicate the managed string is constant.
 */
bool_t st_valid_destination(uint32_t opts) {

	if (!st_valid_opts(opts)) {
		return false;
	} else if (opts & CONSTANT_T) {
		return false;
	}

	return true;
}

/**
 * @brief	Determine whether the managed string options allow for data appending.
 * @param	opts	the options value for the managed string.
 * @return	true if permitted, or false if options are invalid, the string is not jointed, or either managed or mapped.
 */
bool_t st_valid_append(uint32_t opts) {

	if (!st_valid_opts(opts)) {
		return false;
	} else if (!(opts & (MANAGED_T | MAPPED_T))) {
		return false;
	} else if (!(opts & (JOINTED))) {
		return false;
	}

	return true;
}

/**
 * @brief	Determine whether a managed string is allowed to be freed.
 * @param	opts	the options value of the managed string to be evaluated.
 * @return	true if the managed string can be freed; false otherwise (it is allocated on the stack or contains foreign data).
 */
bool_t st_valid_free(uint32_t opts) {

	if (!st_valid_opts(opts)) {
		return false;
	}
	// QUESTION: Foreign data is OK if it's a placer, right?
	else if ((opts & STACK) || (opts & FOREIGNDATA && !(opts & PLACER_T))) {
		return false;
	}

	return true;
}

/**
 * @brief	Determine whether a managed string provides for data length tracking.
 * @param	opts	the options value of the managed string.
 * @return	false if no length tracking is available; true if there is (string is a placer, managed, or mapped).
 */
bool_t st_valid_tracked(uint32_t opts) {

	if (!st_valid_opts(opts)) {
		return false;
	} else if (!(opts & (PLACER_T | MANAGED_T | MAPPED_T))) {
		return false;
	}

	return true;
}

/**
 * @brief	Determine whether the managed string options are a validly jointed.
 * @param	opts	the options value to test.
 * @return	false if the options are invalid or if the string isn't jointed; true if the string is jointed.
 */
bool_t st_valid_jointed(uint32_t opts) {

	if (!st_valid_opts(opts)) {
		return false;
	}
	else if (!(opts & JOINTED)) {
		return false;
	}

	return true;
}

/**
 * @brief	Determine whether a managed string tracks the total allocated buffer size.
 * @param	opts	the managed string's options value.
 * @return	false if buffer size isn't tracked; true otherwise (the managed string is managed or mapped).
 */
bool_t st_valid_avail(uint32_t opts) {

	if (!st_valid_opts(opts)) {
		return false;
	} else if (!(opts & (MANAGED_T | MAPPED_T))) {
		return false;
	}

	return true;
}

/**
 * @brief	Check to see that a managed string has a valid combination of allocation options.
 * @note	The following rules are enforced:
 * 			1. Each managed string must only be one of the following:
 * 				a. constant, nuller, block, placer, managed, or mapped.
 *				b. jointed or contiguous.
 *				c. allocated on the stack, heap, or secure.
 *			2. A placer cannot be contiguous.
 *			3. A constant must be contiguous and be allocated on the stack.
 *			4. Mapped strings must be contiguous and on the heap.
 *
 * @param	opts	the managed string option mask to be validated.
 * @return	true if the options represent valid managed string allocation options, or false if they do not.
 */
bool_t st_valid_opts(uint32_t opts) {

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

		case (NULLER_T):
		case (BLOCK_T):
		case (MANAGED_T):
			break;

		// Mapped containers must specify a jointed layout and use the heap allocator.
		case (MAPPED_T):
			if (opts & CONTIGUOUS) result = false;
			else if (opts & STACK) result = false;
			break;

	}

	return result;
}
