
/**
 * @file /magma/core/memory/bits.c
 *
 * @brief	A collection of functions used handle common bit manipulation tasks.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Count the number of bits that are set in a 64-bit number.
 * @param 	value	an unsigned 64-bit value to be checked.
 * @return	the number of bits in value that are set.
 */
uint_t bits_count(uint64_t value) {

	uint_t result = 0;

	while (value != 0) {
		value &= value - 1;
		result++;
	}

	return result;
}
