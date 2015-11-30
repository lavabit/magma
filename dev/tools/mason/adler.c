
/**
 * @file /mason/adler.c
 *
 * @brief An x86 implementation of the Adler hash algorithim.
 *
 * $Author: Ladar Levison $
 * $Date: 2010/07/03 16:59:52 $
 * $Revision: cafc504385e565e4ff418602dce85acc8bcdddff $
 *
 */

#include "mason.h"

uint32_t hash_adler32(char *buffer, size_t length) {

	size_t ilen;
	uint32_t a = 1, b = 0;

	while (length > 0) {

		// Every 5550 octets we need to modulo.
		ilen = length > 5550 ? 5550 : length;
		length -= ilen;

		do {
			a += *(char *)buffer++;
			b += a;
		} while (--ilen);

		a = (a & 0xffff) + (a >> 16) * (65536 - 65521);
      b = (b & 0xffff) + (b >> 16) * (65536 - 65521);
	}

	// If a is greater than the mod number, modulo.
	if (a >= 65521) {
		a -= 65521;
	}

	b = (b & 0xffff) + (b >> 16) * (65536 - 65521);
	if (b >= 65521) {
		b -= 65521;
	}

	return (b << 16) | a;
}
