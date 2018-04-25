
/**
 * @file /magma/core/parsers/numbers/clamp.c
 *
 * @brief Provides a collection of functions for clamping a number to a specific range.
 */

#include "magma.h"

/**
 * @brief	Ensure a number is between the min and max values, otherwise return the boundary value.
 *
 * @param	min	minimum value for the clamp
 * @param	max	maximum value for the clamp
 * @param	number number to be clamped
 * @return	returns the min value if number is less than min, and returns max if number was greater than the max value, otherwise
 * 		the number value is returned without any modifications. The original value for number is also returned if min is greater
 * 		than max or max is less than min.
 */
uint8_t uint8_clamp(uint8_t min, uint8_t max, uint8_t number) {
	if (min > max || max < min) {
		log_pedantic("The range clamp is invalid. The %s value was %s than the %s value, so the original number value was returned. " \
			"{ min = %hhu / max = %hhu / number = %hhu )", (min > max ? "min" : "max"), (min > max ? "greater" : "less"),
			(min > max ? "max" : "min"), min, max, number);
		return number;
	}

	return number > max ? max : (number < min ? min : number);
}

/**
 * @brief	Ensure a number is between the min and max values, otherwise return the boundary value.
 *
 * @param	min	minimum value for the clamp
 * @param	max	maximum value for the clamp
 * @param	number number to be clamped
 * @return	returns the min value if number is less than min, and returns max if number was greater than the max value, otherwise
 * 		the number value is returned without any modifications. The original value for number is also returned if min is greater
 * 		than max or max is less than min.
 */
uint16_t uint16_clamp(uint16_t min, uint16_t max, uint16_t number) {
	if (min > max || max < min) {
		log_pedantic("The range clamp is invalid. The %s value was %s than the %s value, so the original number value was returned. " \
			"{ min = %hu / max = %hu / number = %hu )", (min > max ? "min" : "max"), (min > max ? "greater" : "less"),
			(min > max ? "max" : "min"), min, max, number);
		return number;
	}

	return number > max ? max : (number < min ? min : number);
}

/**
 * @brief	Ensure a number is between the min and max values, otherwise return the boundary value.
 *
 * @param	min	minimum value for the clamp
 * @param	max	maximum value for the clamp
 * @param	number number to be clamped
 * @return	returns the min value if number is less than min, and returns max if number was greater than the max value, otherwise
 * 		the number value is returned without any modifications. The original value for number is also returned if min is greater
 * 		than max or max is less than min.
 */
uint32_t uint32_clamp(uint32_t min, uint32_t max, uint32_t number) {
	if (min > max || max < min) {
		log_pedantic("The range clamp is invalid. The %s value was %s than the %s value, so the original number value was returned. " \
			"{ min = %u / max = %u / number = %u )", (min > max ? "min" : "max"), (min > max ? "greater" : "less"),
			(min > max ? "max" : "min"), min, max, number);
		return number;
	}

	return number > max ? max : (number < min ? min : number);
}

/**
 * @brief	Ensure a number is between the min and max values, otherwise return tuint_the boundary value.
 *
 * @param	min	minimum value for the clamp
 * @param	max	maximum value for the clamp
 * @param	number number to be clamped
 * @return	returns the min value if number is less than min, and returns max if number was greater than the max value, otherwise
 * 		the number value is returned without any modifications. The original value for number is also returned if min is greater
 * 		than max or max is less than min.
 */
uint64_t uint64_clamp(uint64_t min, uint64_t max, uint64_t number) {
	if (min > max || max < min) {
		log_pedantic("The range clamp is invalid. The %s value was %s than the %s value, so the original number value was returned. " \
			"{ min = %lu / max = %lu / number = %lu )", (min > max ? "min" : "max"), (min > max ? "greater" : "less"),
			(min > max ? "max" : "min"), min, max, number);
		return number;
	}

	return number > max ? max : (number < min ? min : number);
}

/**
 * @brief	Ensure a number is between the min and max values, otherwise return the boundary value.
 *
 * @param	min	minimum value for the clamp
 * @param	max	maximum value for the clamp
 * @param	number number to be clamped
 * @return	returns the min value if number is less than min, and returns max if number was greater than the max value, otherwise
 * 		the number value is returned without any modifications. The original value for number is also returned if min is greater
 * 		than max or max is less than min.
 */
int8_t int8_clamp(int8_t min, int8_t max, int8_t number) {
	if (min > max || max < min) {
		log_pedantic("The range clamp is invalid. The %s value was %s than the %s value, so the original number value was returned. " \
			"{ min = %hhu / max = %hhu / number = %hhu )", (min > max ? "min" : "max"), (min > max ? "greater" : "less"),
			(min > max ? "max" : "min"), min, max, number);
		return number;
	}

	return number > max ? max : (number < min ? min : number);
}

/**
 * @brief	Ensure a number is between the min and max values, otherwise return the boundary value.
 *
 * @param	min	minimum value for the clamp
 * @param	max	maximum value for the clamp
 * @param	number number to be clamped
 * @return	returns the min value if number is less than min, and returns max if number was greater than the max value, otherwise
 * 		the number value is returned without any modifications. The original value for number is also returned if min is greater
 * 		than max or max is less than min.
 */
int16_t int16_clamp(int16_t min, int16_t max, int16_t number) {
	if (min > max || max < min) {
		log_pedantic("The range clamp is invalid. The %s value was %s than the %s value, so the original number value was returned. " \
			"{ min = %hu / max = %hu / number = %hu )", (min > max ? "min" : "max"), (min > max ? "greater" : "less"),
			(min > max ? "max" : "min"), min, max, number);
		return number;
	}

	return number > max ? max : (number < min ? min : number);
}

/**
 * @brief	Ensure a number is between the min and max values, otherwise return the boundary value.
 *
 * @param	min	minimum value for the clamp
 * @param	max	maximum value for the clamp
 * @param	number number to be clamped
 * @return	returns the min value if number is less than min, and returns max if number was greater than the max value, otherwise
 * 		the number value is returned without any modifications. The original value for number is also returned if min is greater
 * 		than max or max is less than min.
 */
int32_t int32_clamp(int32_t min, int32_t max, int32_t number) {
	if (min > max || max < min) {
		log_pedantic("The range clamp is invalid. The %s value was %s than the %s value, so the original number value was returned. " \
			"{ min = %u / max = %u / number = %u )", (min > max ? "min" : "max"), (min > max ? "greater" : "less"),
			(min > max ? "max" : "min"), min, max, number);
		return number;
	}

	return number > max ? max : (number < min ? min : number);
}

/**
 * @brief	Ensure a number is between the min and max values, otherwise return the boundary value.
 *
 * @param	min	minimum value for the clamp
 * @param	max	maximum value for the clamp
 * @param	number number to be clamped
 * @return	returns the min value if number is less than min, and returns max if number was greater than the max value, otherwise
 * 		the number value is returned without any modifications. The original value for number is also returned if min is greater
 * 		than max or max is less than min.
 */
int64_t int64_clamp(int64_t min, int64_t max, int64_t number) {
	if (min > max || max < min) {
		log_pedantic("The range clamp is invalid. The %s value was %s than the %s value, so the original number value was returned. " \
			"{ min = %lu / max = %lu / number = %lu )", (min > max ? "min" : "max"), (min > max ? "greater" : "less"),
			(min > max ? "max" : "min"), min, max, number);
		return number;
	}

	return number > max ? max : (number < min ? min : number);
}

