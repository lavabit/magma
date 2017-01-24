
/**
 * @file /magma/engine/config/cache/cache.h
 *
 * @brief	Types and functions for creating and accessing the cache structures.
 */

#ifndef MAGMA_ENGINE_CONFIG_CACHE_H
#define MAGMA_ENGINE_CONFIG_CACHE_H

typedef struct {
	size_t offset; /* The location in memory to store the setting value. */
	multi_t norm; /* The default value. */
	char *name; /* The search key/name of the setting. */
	char *description; /* Description of the setting and its default value. */
	bool_t required; /* Is this setting required? */
} cache_keys_t;

typedef struct {
	char *name; /* The cache name. */
	uint32_t port; /* The cache port. */
	uint32_t weight; /* The key space weight. */
} cache_t;

/************  CACHE  ************/
void cache_free (void);
bool_t cache_validate (void);
void cache_output_settings (void);
cache_t *cache_alloc (uint32_t number);
bool_t cache_config (stringer_t *name, stringer_t *value);
bool_t cache_set_value (cache_keys_t *setting, cache_t *cache, stringer_t *value);
void cache_output_help(void);
/************  CACHE  ************/
#endif 
