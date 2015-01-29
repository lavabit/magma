
/**
 * @file /magma/core/parsers/numbers/digits.c
 *
 * @brief	Functions for counting the digit places in a number, including the sign character for signed numbers.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Count the number of digits needed to represent a base-10 64-bit unsigned integer.
 * @return	the number of digits needed to represent specified integer.
 */
size_t uint64_digits(uint64_t number) {

	size_t length = 1;

	while ((number /= 10) != 0) {
		length++;
	}

	return length;
}

/**
 * @brief	Count the number of digits needed to represent a base-10 32-bit unsigned integer.
 * @return	the number of digits needed to represent the specified integer.
 */
size_t uint32_digits(uint32_t number) {

	size_t length = 1;

	while ((number /= 10) != 0) {
		length++;
	}

	return length;
}

/**
 * @brief	Count the number of digits needed to represent a base-10 16-bit unsigned integer.
 * @return	the number of digits needed to represent the specified integer.
 */

size_t uint16_digits(uint16_t number) {

	size_t length = 1;

	while ((number /= 10) != 0) {
		length++;
	}

	return length;
}

/**
 * @brief	Count the number of digits needed to represent a base-10 8-bit unsigned integer.
 * @return	the number of digits needed to represent the specified integer.
 */
size_t uint8_digits(uint8_t number) {

	size_t length = 1;

	while ((number /= 10) != 0) {
		length++;
	}

	return length;
}

/**
 * @brief	Count the number of digits needed to represent a base-10 64-bit signed integer.
 * @return	the number of digits needed to represent the specified integer, including a negative sign.
 */
size_t int64_digits(int64_t number) {

	size_t length = 1;

	if (number < 0) {
		length++;
	}

	while ((number /= 10) != 0) {
		length++;
	}

	return length;
}

/**
 * @brief	Count the number of digits needed to represent a base-10 32-bit signed integer.
 * @return	the number of digits needed to represent the specified integer, including a negative sign.
 */
size_t int32_digits(int32_t number) {

	size_t length = 1;

	if (number < 0) {
		length++;
	}

	while ((number /= 10) != 0) {
		length++;
	}

	return length;
}

/**
 * @brief	Count the number of digits needed to represent a base-10 16-bit signed integer.
 * @return	the number of digits needed to represent the specified integer, including a negative sign.
 */
size_t int16_digits(int16_t number) {

	size_t length = 1;

	if (number < 0) {
		length++;
	}

	while ((number /= 10) != 0) {
		length++;
	}

	return length;
}

/**
 * @brief	Count the number of digits needed to represent a base-10 8-bit signed integer.
 * @return	the number of digits needed to represent the specified integer, including a negative sign.
 */
size_t int8_digits(int8_t number) {

	size_t length = 1;

	if (number < 0) {
		length++;
	}

	while ((number /= 10) != 0) {
		length++;
	}

	return length;
}
