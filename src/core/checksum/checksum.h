
/**
 * @file /magma/core/checksum/checksum.h
 *
 * @brief	Declarations for hash functions that have been implemented internally.
 */

#ifndef MAGMA_CORE_CHECKSUM_H
#define MAGMA_CORE_CHECKSUM_H

/// crc.c
uint32_t   crc24_init(void);
uint32_t   crc24_final(uint32_t crc);

uint32_t   crc24_checksum(void *buffer, size_t length);
uint32_t   crc32_checksum(void *buffer, size_t length);
uint64_t   crc64_checksum(void *buffer, size_t length);

uint32_t   crc24_update(void *buffer, size_t length, uint32_t crc);
uint32_t   crc32_update(void *buffer, size_t length, uint32_t crc);
uint64_t   crc64_update(void *buffer, size_t length, uint64_t crc);

uint32_t crc24_checksum(void *buffer, size_t length);
uint32_t crc32_checksum(void *buffer, size_t length);
uint64_t crc64_checksum(void *buffer, size_t length);
uint32_t crc32_update(void *buffer, size_t length, uint32_t crc);
uint64_t crc64_update(void *buffer, size_t length, uint64_t crc);

uint32_t hash_adler32(void *buffer, size_t length);
uint32_t hash_murmur32(void *buffer, size_t length);
uint64_t hash_murmur64(void *buffer, size_t length);
uint32_t hash_fletcher32(void *buffer, size_t length);

#endif
