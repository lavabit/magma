
/**
 * @file /magma/core/memory/align.c
 *
 * @brief	Functions for memory alignment operations.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/***
 * @brief	Align a size to a specified boundary.
 * @param	alignment	the number to which the specified size should be aligned.
 * @param	length		the size, in bytes, of the memory area to be aligned.
 * @return	the aligned value of the specified length.
 */
size_t align(size_t alignment, size_t length) {
	return (length + (alignment ? alignment : 1) - 1) & ~((alignment ? alignment : 1) - 1);
}
