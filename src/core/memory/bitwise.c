
/**
 * @file /magma/core/memory/bitwise.c
 *
 * @brief	A collection of functions used handle common bit manipulation tasks.
 */

#include "magma.h"

/**
 * @brief	Count the number of bits that are set in a 64-bit number.
 * @param 	value	an unsigned 64-bit value to be checked.
 * @return	the number of bits in value that are set.
 */
uint_t bitwise_count(uint64_t value) {

	uint_t result = 0;

	while (value != 0) {
		value &= value - 1;
		result++;
	}

	return result;
}

/**
 * @brief	Performs a bitwise OR operation on two octets and returns the result as a single octet.
 */
inline uchr_t bitwise_or(uchr_t a, uchr_t b) {
	return a | b;
}

/**
 * @brief	Performs a bitwise XOR operation on two octets and returns the result as a single octet.
 */
inline uchr_t bitwise_xor(uchr_t a, uchr_t b) {
	return a ^ b;
}

/**
 * @brief	Performs a bitwise AND operation on two octets and returns the result as a single octet.
 */
inline uchr_t bitwise_and(uchr_t a, uchr_t b) {
	return a & b;
}

