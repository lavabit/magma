
/**
 * @file /magma/core/hash/murmur.c
 *
 * @brief	An x64 implementation of the Murmur hash function.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"


/**
 * @brief	Generate a 32-bit Murmur hash of a block of data.
 * @param	buffer	a pointer to the block of data to be hashed.
 * @param	length	the length, in bytes, of the block of data to be hashed.
 * @return	the 32-bit value of the Murmur hash of the specified block of data.
 */
uint32_t hash_murmur32(void *buffer, size_t length) {

	uint32_t k;
	const int32_t r = 24;
	int32_t h = length;
	unsigned char *data = buffer;
	const uint32_t m = 0x5bd1e995;

	while (length >= 4) {

		k = *(uint32_t *)data;

		k *= m;
		k ^= k >> r;
		k *= m;

		h *= m;
		h ^= k;

		data += 4;
		length -= 4;
	}

	switch (length) {
		case 3:
			h ^= data[2] << 16;
			h ^= data[1] << 8;
			h ^= data[0];
			h *= m;
			break;
		case 2:
			h ^= data[1] << 8;
			h ^= data[0];
			h *= m;
			break;
		case 1:
			h ^= data[0];
			h *= m;
			break;
	}

	h ^= h >> 13;
	h *= m;
	h ^= h >> 15;

	return h;
}


/**
 * @brief	Generate a 64-bit Murmur hash of a block of data.
 * @param	buffer	a pointer to the block of data to be hashed.
 * @param	length	the length, in bytes, of the block of data to be hashed.
 * @return	the 64-bit value of the Murmur hash of the specified block of data.
 */
uint64_t hash_murmur64(void *buffer, size_t length) {

	unsigned char *c;
	const int32_t r = 47;
	const uint64_t m = 0xc6a4a7935bd1e995;
	uint64_t k, h = (length * m), *data = buffer, *end = data + (length/8);

	while (data != end) {
		k = *data++;

		k *= m;
		k ^= k >> r;
		k *= m;

		h ^= k;
		h *= m;
	}

	c = (unsigned char *)data;

	switch(length & 7)	{
		case 7:
			h ^= (uint64_t)(c[6]) << 48;
			h ^= (uint64_t)(c[5]) << 40;
			h ^= (uint64_t)(c[4]) << 32;
			h ^= (uint64_t)(c[3]) << 24;
			h ^= (uint64_t)(c[2]) << 16;
			h ^= (uint64_t)(c[1]) << 8;
			h ^= (uint64_t)(c[0]);
			h *= m;
			break;
		case 6:
			h ^= (uint64_t)(c[5]) << 40;
			h ^= (uint64_t)(c[4]) << 32;
			h ^= (uint64_t)(c[3]) << 24;
			h ^= (uint64_t)(c[2]) << 16;
			h ^= (uint64_t)(c[1]) << 8;
			h ^= (uint64_t)(c[0]);
			h *= m;
			break;
		case 5:
			h ^= (uint64_t)(c[4]) << 32;
			h ^= (uint64_t)(c[3]) << 24;
			h ^= (uint64_t)(c[2]) << 16;
			h ^= (uint64_t)(c[1]) << 8;
			h ^= (uint64_t)(c[0]);
			h *= m;
			break;
		case 4:
			h ^= (uint64_t)(c[3]) << 24;
			h ^= (uint64_t)(c[2]) << 16;
			h ^= (uint64_t)(c[1]) << 8;
			h ^= (uint64_t)(c[0]);
			h *= m;
			break;
		case 3:
			h ^= (uint64_t)(c[2]) << 16;
			h ^= (uint64_t)(c[1]) << 8;
			h ^= (uint64_t)(c[0]);
			h *= m;
			break;
		case 2:
			h ^= (uint64_t)(c[1]) << 8;
			h ^= (uint64_t)(c[0]);
			h *= m;
			break;
		case 1:
			h ^= (uint64_t)(c[0]);
			h *= m;
			break;
	};

	h ^= h >> r;
	h *= m;
	h ^= h >> r;

	return h;
}
