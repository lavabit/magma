
/**
 * @file /magma/core/strings/print.c
 *
 * @brief	Functions for printing formatted data to managed strings.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Print to a managed string and return the number of bytes written.
 * @see		vsnprintf()
 * @note	If the destination string pointer is NULL the function will simply return how much room would have been necessary.
 * @param	s		a pointer to the managed string that will receive the output of the print operation.
 * @param	format	a format string specifying the arguments of the print operation.
 * @param	args	a va_list containing the parameters to the print format string.
 * @return	-1 on failure, or the number of characters printed to the string, excluding the terminating null byte.
 */
size_t st_vsprint(stringer_t *s, chr_t *format, va_list args) {

	int_t length;
	size_t avail;
	uint32_t opts = *((uint32_t *)s);

#ifdef MAGMA_PEDANTIC
	if (!st_valid_destination(opts)) {
		log_pedantic("Invalid string options. { opt = %u = %s }", opts, st_info_opts(opts, MEMORYBUF(128), 128));
		return 0;
	}
#endif

	// If the target string is NULL, we just calculate how much room would be needed.
	if (!s) {
		length = vsnprintf(NULL, 0, format, args);
		return length;
	}

	// Print the provided format into the newly allocated buffer.
	length = vsnprintf(st_data_get(s), (avail = st_avail_get(s)), format, args);

#ifdef MAGMA_PEDANTIC
	if (length > avail) {
		log_pedantic("The output buffer was not large enough to hold the output, so the result was truncated.");
	}
#endif

	// If the length of the data segment is tracked explicitly, set the string length. Block strings assume the length of the buffer is the
	// the length of the data, and NULL strings don't explicitly track of their length so we skip this step for those string types.
	if (st_valid_tracked(opts)) {
		st_length_set(s, length > avail ? avail : length);
	}
	return length > avail ? avail : length;
}

/**
 * @brief	Print to a managed string and return the number of bytes written.
 * @param	s		a pointer to the managed string that will receive the output of the print operation.
 * @param	format	a format string specifying the arguments of the print operation.
 * @param	...		a variable argument list containing the parameters to the print format string.
 * @return	-1 on failure, or the number of characters printed to the string, excluding the terminating null byte.
 */
size_t st_sprint(stringer_t *s, chr_t *format, ...) {

	va_list list;
	size_t result;

	va_start(list, format);
	result = st_vsprint(s, format, list);
	va_end(list);

	return result;
}

/**
 * @brief	Print to a managed string and return a pointer to the result.
 * @see		st_print()
 * @param	s		a pointer to the managed string that will receive the output of the print operation.
 * @param	format	a format string specifying the arguments of the print operation.
 * @param	...		a variable argument list containing the parameters to the print format string.
 * @return	a pointer to the managed string that received the printed output.
 */
stringer_t * st_quick(stringer_t *s, chr_t *format, ...) {

	va_list list;

	va_start(list, format);
	st_vsprint(s, format, list);
	va_end(list);

	return s;
}


/**
 * @brief	Return a managed string containing sprintf()-style formatted data.
 * @param	opts	an options value to be passed to the allocation of the resulting managed string.
 * @param	format	a format string for the output string data.
 * @param	args	a variable argument list of parameters to be formatted as output.
 * @return	NULL on failure or a managed string containing the final formatted data on success.
 */
stringer_t * st_vaprint_opts(uint32_t opts, chr_t *format, va_list args) {

	void *out;
	va_list copy;
	int_t length, expected;
	stringer_t *result = NULL;

#ifdef MAGMA_PEDANTIC
	if (!st_valid_destination(opts)) {
		log_pedantic("Invalid string options. { opt = %u = %s }", opts, st_info_opts(opts, MEMORYBUF(128), 128));
		return NULL;
	}
#endif

	// Calculate the length.
	va_copy(copy, args);
	expected = length = vsnprintf(NULL, 0, format, copy);

	// Allocate a properly sized buffer.
	if (!length || !(result = st_alloc_opts(opts, length + 1)) || !(out = st_data_get(result))) {
		if (result) st_free(result);
		return NULL;
	}

	// Print the provided format into the newly allocated buffer.
	if ((length = vsnprintf(out, length + 1, format, args)) != expected) {
		log_pedantic("The print operation did not generate the amount of data we expected.");
		st_free(result);
		result = NULL;
	}

	if (st_valid_tracked(opts)) {
		st_length_set(result, length);
	}

	return result;
}

/**
 * @brief	Return a managed string containing sprintf()-style formatted data.
 * @see		st_vaprint_opts()
 * @param	format	a format string for the output string data.
 * @param	...		a variable argument list of parameters to be formatted as output.
 * @return	NULL on failure or a managed string containing the final formatted data on success.
 */
stringer_t * st_aprint(chr_t *format, ...) {

	va_list list;
	stringer_t *result;

	va_start(list, format);
	result = st_vaprint_opts(MANAGED_T | CONTIGUOUS | HEAP, format, list);
	va_end(list);

	return result;
}

/**
 * @brief	Return a managed string containing sprintf()-style formatted data, with custom allocation options.
 * @param	opts	the option value of the newly allocated managed string that will contain the result.
 * @param	format	a format string for the output string data.
 * @param	...		a variable argument list of parameters to be formatted as output.
 * @return	NULL on failure or a managed string of the specified allocation options containing the final formatted data on success.
 */
stringer_t * st_aprint_opts(uint32_t opts, chr_t *format, ...) {

	va_list list;
	stringer_t *result;

	va_start(list, format);
	result = st_vaprint_opts(opts, format, list);
	va_end(list);

	return result;
}

