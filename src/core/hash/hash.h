
/**
 * @file /magma/core/hash/hash.h
 *
 * @brief	Declarations for hash functions that have been implemented internally.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_CORE_HASH_H
#define MAGMA_CORE_HASH_H

uint32_t hash_crc32(void *buffer, size_t length);
uint64_t hash_crc64(void *buffer, size_t length);
uint32_t hash_crc32_update(void *buffer, size_t length, uint32_t crc);
uint64_t hash_crc64_update(void *buffer, size_t length, uint64_t crc);

uint32_t hash_adler32(void *buffer, size_t length);
uint32_t hash_murmur32(void *buffer, size_t length);
uint64_t hash_murmur64(void *buffer, size_t length);
uint32_t hash_fletcher32(void *buffer, size_t length);

#endif
