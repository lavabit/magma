#include "dime/signet-resolver/cache.h"


void *get_cache_obj_data(cached_object_t *object) {
    PUBLIC_FUNC_IMPL(get_cache_obj_data, object);
}

void destroy_cache_entry(cached_object_t *entry) {
    PUBLIC_FUNC_IMPL_VOID(destroy_cache_entry, entry);
}

cached_object_t *find_cached_object(const char *oid, cached_store_t *store) {
    PUBLIC_FUNC_IMPL(find_cached_object, oid, store);
}

cached_object_t *find_cached_object_cmp(const void *key, cached_store_t *store, cached_store_comparator_t cmpfn) {
    PUBLIC_FUNC_IMPL(find_cached_object_cmp, key, store, cmpfn);
}

int cached_object_exists(const unsigned char *hashid, cached_store_t *store) {
    PUBLIC_FUNC_IMPL(cached_object_exists, hashid, store);
}

int cached_object_exists_cmp(const void *key, cached_store_t *store, cached_store_comparator_t cmpfn) {
    PUBLIC_FUNC_IMPL(cached_object_exists_cmp, key, store, cmpfn);
}

cached_object_t *add_cached_object(const char *id, cached_store_t *store, unsigned long ttl, time_t expiration, void *data, int persists, int relaxed) {
    PUBLIC_FUNC_IMPL(add_cached_object, id, store, ttl, expiration, data, persists, relaxed);
}

cached_object_t *add_cached_object_forced(const char *id, cached_store_t *store, unsigned long ttl, time_t expiration, void *data, int persists, int relaxed) {
    PUBLIC_FUNC_IMPL(add_cached_object_forced, id, store, ttl, expiration, data, persists, relaxed);
}

cached_object_t *add_cached_object_cmp(const char *id, const void *key, cached_store_t *store, unsigned long ttl, time_t expiration, void *data, int persists, int relaxed, cached_store_comparator_t cmpfn) {
    PUBLIC_FUNC_IMPL(add_cached_object_cmp, id, key, store, ttl, expiration, data, persists, relaxed, cmpfn);
}

cached_object_t *add_cached_object_cmp_forced(const char *id, const void *key, cached_store_t *store, unsigned long ttl, time_t expiration, void *data, int persists, int relaxed, cached_store_comparator_t cmpfn) {
    PUBLIC_FUNC_IMPL(add_cached_object_cmp_forced, id, key, store, ttl, expiration, data, persists, relaxed, cmpfn);
}

int remove_cached_object(const char *oid, cached_store_t *store) {
    PUBLIC_FUNC_IMPL(remove_cached_object, oid, store);
}

int remove_cached_object_cmp(const void *key, cached_store_t *store, cached_store_comparator_t cmpfn) {
    PUBLIC_FUNC_IMPL(remove_cached_object_cmp, key, store, cmpfn);
}

int load_cache_contents(void) {
    PUBLIC_FUNC_IMPL(load_cache_contents, );
}

int save_cache_contents(void) {
    PUBLIC_FUNC_IMPL(save_cache_contents, );
}

char *get_dime_dir_location(const char *suffix) {
    PUBLIC_FUNC_IMPL(get_dime_dir_location, suffix);
}

char *get_cache_location(void) {
    PUBLIC_FUNC_IMPL(get_cache_location, );
}

int set_cache_location(const char *path) {
    PUBLIC_FUNC_IMPL(set_cache_location, path);
}

int set_cache_permissions(unsigned long flags) {
    PUBLIC_FUNC_IMPL(set_cache_permissions, flags);
}
