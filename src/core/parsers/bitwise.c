
/**
 * @file /magma/core/parsers/bitwise.c
 *
 * @brief	Functions capable of performing binary arithmetic on managed strings.
 */

#include "magma.h"

/**
 * @brief	Perform bitwise operation on two input strings.
 * @note	Function pointers are used to determine which bitwise operation is performed. This isn't very efficient, because even though
 * 		the functions are marked as inline, they aren't actually being inlined by the compiler... but keeping it this way does make it
 * 		easier to read. If profiling reveals a performance problem, the functions can be moved to the header file, and thus inlined,
 * 		replaced with a macro, or even a switch statement that keys of an enumerated variable.
 * @param	a		First stringer input.
 * @param	b		Second stringer input.
 * @param	output	Stringer in which the result is stored, if no stringer is provided
 * 					(output = NULL) then a new stringer will be allocated for the result.
 * @param	bitwise A function pointer to the bitwise operation.
 * @return	Pointer to output if a valid output stringer was provided, or a pointer to a newly allocated
 * 		stringer if no stringer was specified for the output, NULL on error.
 */
stringer_t * st_bitwise(stringer_t *a, stringer_t *b, stringer_t *output, uchr_t (*bitwise)(uchr_t, uchr_t)) {

	size_t olen;
	uint32_t opts;
	stringer_t *result = NULL;
	uchr_t *data, *in_a, *in_b;

	if (st_empty(a) || st_empty(b)) {
		log_pedantic("At least one of the input strings is NULL or empty.");
		return NULL;
	}
	else if ((olen = st_length_get(a)) != st_length_get(b)) {
		log_pedantic("Input strings must be of equal length for a bitwise combination to work.");
		return NULL;
	}
	else if (output && !st_valid_destination((opts = *((uint32_t *)output)))) {
		log_pedantic("An output string was supplied but it does not represent a buffer capable of holding a result.");
		return NULL;
	}
	else if ((result = output) && ((st_valid_avail(opts) && st_avail_get(output) < olen) || (!st_valid_avail(opts) && st_length_get(output) < olen))) {
		log_pedantic("The output buffer supplied is not large enough to hold the result. {avail = %zu / required = %zu}",
			st_valid_avail(opts) ? st_avail_get(output) : st_length_get(output), olen);
		return NULL;
	}
	else if (!output && !(result = st_alloc(olen))) {
		log_pedantic("The output buffer allocation failed. {requested = %zu}", olen);
		return NULL;
	}

	if (!(data = st_data_get(result)) || !(in_a = st_data_get(a)) || !(in_b = st_data_get(b))) {
		log_pedantic("Could not retrieve a pointer to the stringer data.");
		if (!output) st_free(result);
		return NULL;
	}

	for(uint_t i = 0; i < olen; ++i) {
		data[i] = (*bitwise)(in_a[i], in_b[i]);
	}

	st_length_set(result, olen);
	return result;
}

/**
 * @brief	Perform bitwise NOT operation on a managed string.
 * @param	s		Stringer input.
 * @param	output	Stringer in which the result is stored, if no stringer is provided
 * 					(output = NULL) then a new stringer will be allocated for the result.
 * @return	Pointer to output if a valid output stringer was provided, or a pointer to a newly allocated
 * 			stringer if no stringer was specified for the output, NULL on error.
 */
stringer_t * st_not(stringer_t *s, stringer_t *output) {

	size_t olen;
	uint32_t opts;
	uchr_t *data, *in_s;
	stringer_t *result = NULL;

	if (st_empty(s) || (!(olen = st_length_get(s)))) {
		log_pedantic("Input stringer is empty or NULL was passed in.");
		return NULL;
	}
	else if (output && !st_valid_destination((opts = *((uint32_t *)output)))) {
		log_pedantic("An output string was supplied but it does not represent a buffer capable of holding a result.");
		return NULL;
	}
	else if ((result = output) && ((st_valid_avail(opts) && st_avail_get(output) < olen) || (!st_valid_avail(opts) && st_length_get(output) < olen))) {
		log_pedantic("The output buffer supplied is not large enough to hold the result. {avail = %zu / required = %zu}",
			st_valid_avail(opts) ? st_avail_get(output) : st_length_get(output), olen);
		return NULL;
	}
	else if (!output && !(result = st_alloc(olen))) {
		log_pedantic("The output buffer allocation failed. {requested = %zu}", olen);
		return NULL;
	}

	if (!(data = st_data_get(result)) || !(in_s = st_data_get(s))) {
		log_pedantic("Could not retrieve a pointer to the stringer data.");
		if (!output) st_free(result);
		return NULL;
	}

	for (uint_t i = 0; i < olen; ++i) {
		data[i] = ~in_s[i];
	}

	st_length_set(result, olen);
	return result;
}

/**
 * @brief	Perform bitwise OR operation between two input strings.
 * @param	a		First stringer input.
 * @param	b		Second stringer input.
 * @param	output	Stringer in which the result is stored, if no stringer is provided
 * 					(output = NULL) then a new stringer will be allocated for the result.
 * @return	Pointer to output if a valid output stringer was provided, Pointer to a newly allocated
 * 			stringer if no stringer was specified for the output, NULL on error.
 */
stringer_t * st_or(stringer_t *a, stringer_t *b, stringer_t *output) {
	return st_bitwise(a, b, output, &bitwise_or);
}

/**
 * @brief	Perform bitwise XOR operation between two input strings.
 * @param	a		First stringer input.
 * @param	b		Second stringer input.
 * @param	output	Stringer in which the result is stored, if no stringer is provided
 * 					(output = NULL) then a new stringer will be allocated for the result.
 * @return	Pointer to output if a valid output stringer was provided, or a pointer to a newly allocated
 * 			stringer if no stringer was specified for the output, NULL on error.
 */
stringer_t * st_xor(stringer_t *a, stringer_t *b, stringer_t *output) {
	return st_bitwise(a, b, output, &bitwise_xor);
}

/**
 * @brief	Perform bitwise AND operation between two input strings.
 * @param	a		First stringer input.
 * @param	b		Second stringer input.
 * @param	output	Stringer in which the result is stored, if no stringer is provided
 * 					(output = NULL) then a new stringer will be allocated for the result.
 * @return	Pointer to output if a valid output stringer was provided, Pointer to a newly allocated
 * 			stringer if no stringer was specified for the output, NULL on error.
 */
stringer_t * st_and(stringer_t *a, stringer_t *b, stringer_t *output) {
	return st_bitwise(a, b, output, &bitwise_and);
}

