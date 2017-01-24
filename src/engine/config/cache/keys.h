
/**
 * @file /magma/engine/config/cache/keys.h
 *
 * @brief	A collection of keys that define the configuration of cache instances.
 */

#ifndef MAGMA_OPIONS_CACHE_KEYS_H
#define MAGMA_OPIONS_CACHE_KEYS_H

cache_keys_t cache_keys[] = {
	{
		.offset = offsetof (cache_t, name),
		.norm.type = M_TYPE_NULLER,
		.norm.val.st = NULL,
		.name = ".name",
		.description = "The host name or address of the cache instance.",
		.required = true
	},
	{
		.offset = offsetof (cache_t, port),
		.norm.type = M_TYPE_UINT32,
		.norm.val.u32 = 11211,
		.name = ".port",
		.description = "The port used by the cache instance.",
		.required = false
	},
	{
		.offset = offsetof (cache_t, weight),
		.norm.type = M_TYPE_UINT32,
// QUESTION: Why isn't this 0 or 1?
		.norm.val.u32 = 1024,
		.name = ".weight",
		.description = "The relative weight of the cache instance.",
		.required = false
	}
};

#endif

