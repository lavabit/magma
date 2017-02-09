
/**
 * @file /magma/providers/consumers/consumers.h
 *
 * @brief	Interfaces for clients that consume/load/retrieve data across various network protocols.
 */

#ifndef MAGMA_PROVIDERS_CONSUMERS_H
#define MAGMA_PROVIDERS_CONSUMERS_H


typedef struct {
	size_t position;
	stringer_t *data;
} serialization_t;

/// cache.c
int_t         cache_add(stringer_t *key, stringer_t *object, time_t expiration);
int_t         cache_append(stringer_t *key, stringer_t *object, time_t expiration);
uint64_t      cache_decrement(stringer_t *key, uint64_t offset, uint64_t initial, time_t expiration);
int_t         cache_delete(stringer_t *key);
void          cache_flush(void);
stringer_t *  cache_get(stringer_t *key);
uint64_t      cache_get_u64(stringer_t *key);
uint64_t      cache_increment(stringer_t *key, uint64_t offset, uint64_t initial, time_t expiration);
int_t         cache_set(stringer_t *key, stringer_t *object, time_t expiration);
int_t         cache_set_u64(stringer_t *key, uint64_t value, time_t expiration);
int_t         cache_silent_add(stringer_t *key, stringer_t *object, time_t expiration);
bool_t        cache_start(void);
void          cache_stop(void);
bool_t        lib_load_cache(void);
const         char * lib_version_cache(void);

//! Serialization
bool_t serialize_sz (stringer_t **data, size_t number);
bool_t serialize_ssz (stringer_t **data, ssize_t number);
bool_t serialize_int16 (stringer_t **data, int16_t number);
bool_t serialize_int32 (stringer_t **data, int32_t number);
bool_t serialize_int64 (stringer_t **data, int64_t number);
bool_t serialize_st (stringer_t **data, stringer_t *string);
bool_t serialize_uint64 (stringer_t **data, int64_t number);
bool_t serialize_uint16 (stringer_t **data, uint16_t number);
bool_t serialize_uint32 (stringer_t **data, uint32_t number);
bool_t serialize_ns (stringer_t **data, char *string, size_t length);

int_t deserialize_count_digits (chr_t *data, size_t remaining);
bool_t deserialize_sz (serialization_t *serial, size_t *number);
bool_t deserialize_ssz (serialization_t *serial, ssize_t *number);
bool_t deserialize_int32 (serialization_t *serial, int_t *number);
bool_t deserialize_int64 (serialization_t *serial, long int *number);
bool_t deserialize_st (serialization_t *serial, stringer_t **string);
bool_t deserialize_int16 (serialization_t *serial, short int *number);
bool_t deserialize_uint32 (serialization_t *serial, uint32_t *number);
bool_t deserialize_uint64 (serialization_t *serial, uint64_t *number);
bool_t deserialize_ns (serialization_t *serial, chr_t **string, size_t *length);
bool_t deserialize_uint16 (serialization_t *serial, short unsigned int *number);

#endif

