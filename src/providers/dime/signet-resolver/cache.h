#ifndef CACHE_H
#define CACHE_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>
#include <pwd.h>

#include "dime/common/error.h"

#define CACHE_PERM_LOAD   0x1
#define CACHE_PERM_SAVE   0x2
#define CACHE_PERM_READ   0x4
#define CACHE_PERM_ADD    0x8
#define CACHE_PERM_DELETE 0x16

#define CACHE_PERM_NONE      0
#define CACHE_PERM_ALL_FLAGS (CACHE_PERM_LOAD | CACHE_PERM_SAVE | CACHE_PERM_READ | CACHE_PERM_ADD | CACHE_PERM_DELETE)
#define CACHE_PERM_DEFAULT   CACHE_PERM_ALL_FLAGS


typedef enum {
    cached_data_unknown = 0,
    cached_data_drec = 1,
    cached_data_dnskey = 2,
    cached_data_ds = 3,
    cached_data_ocsp = 4,
    cached_data_signet = 5
} cached_data_type_t;

typedef struct cached_object {
    time_t timestamp;                       ///< The UTC timestamp for when this object was cached. Used with ttl.
    unsigned char id[32];                   ///< The identifier of the cached item as a SHA-256 hash.
    cached_data_type_t dtype;               ///< The type of the cached data being stored.
    unsigned long ttl;                      ///< Optional time-to-live in seconds.
    time_t expiration;                      ///< The time when the record will expire, in UTC.
    int relaxed;                            ///< Whether or not the cache follows a relaxed eviction policy
    void *data;                             ///< The data associated with the cached object.
    struct cached_object *prev;             ///< A pointer to the previous cached object in the linked list.
    struct cached_object *next;             ///< A pointer to the next cached object in the linked list.
    unsigned char persists;                 ///< Not everything in the cache should be persisted.
    struct cached_object *shadow;           ///< A saved copy of the "real" cache entry to be saved, if this cached
                                            ///< object is merely temporarily overriding it.
} cached_object_t;


typedef struct {
    cached_data_type_t dtype;               ///< The type of data that will be stored within.
    const char *description;                ///< A text description of the cache store.
    unsigned char internal;                 ///< Determines whether the cached store is for internal use only.
                                            ///< If it is, objects will not be cloned before being returned to the caller.
    cached_object_t *head;                  ///< A pointer to the head of the cached object list.
    pthread_mutex_t lock;                   ///< For multi-threaded safe code.
    void (*destructor)(void *);             ///< A function pointer to a destructor used to free cached object data.
    void * (*serialize)(void *, size_t *);  ///< A function pointer to a routine used to serialize cached data.
    void * (*deserialize)(void *, size_t);  ///< A function pointer to a routine used to deserialize cached data.
    void (*dump)(FILE *, void *, int);      ///< A function pointer to a routine used to dump the cached data.
    void * (*clone)(void *);                ///< An optional pointer to a routine that can be used to clone data.
                                            ///<    If not specified, the serialize and deserialize routine will be used
                                            ///<    together to recreate the functionality of this function.
} cached_store_t;


typedef int (*cached_store_comparator_t)(const void *, const void *);
typedef unsigned int (*custom_deserializer_t)(unsigned char **, unsigned char **, const unsigned char *);
typedef size_t (*custom_serializer_t)(unsigned char **, size_t *, const void *);

typedef void (*custom_dumper_t)(FILE *, void *, int);


extern cached_store_t cached_stores[];


//Public interface.

// Cache loading and saving.
PUBLIC_FUNC_DECL(int,               load_cache_contents,          void);
PUBLIC_FUNC_DECL(int,               save_cache_contents,          void);
PUBLIC_FUNC_DECL(char *,            get_dime_dir_location,        const char *suffix);
PUBLIC_FUNC_DECL(char *,            get_cache_location,           void);
PUBLIC_FUNC_DECL(int,               set_cache_location,           const char *path);
PUBLIC_FUNC_DECL(int,               set_cache_permissions,        unsigned long flags);

// Searching for objects in the cache.
PUBLIC_FUNC_DECL(cached_object_t *, find_cached_object,           const char *oid, cached_store_t *store);
PUBLIC_FUNC_DECL(cached_object_t *, find_cached_object_cmp,       const void *key, cached_store_t *store, cached_store_comparator_t cmpfn);
PUBLIC_FUNC_DECL(int,               cached_object_exists,         const unsigned char *hashid, cached_store_t *store);
PUBLIC_FUNC_DECL(int,               cached_object_exists_cmp,     const void *key, cached_store_t *store, cached_store_comparator_t cmpfn);

// Adding new objects to the cache.
PUBLIC_FUNC_DECL(cached_object_t *, add_cached_object,            const char *id, cached_store_t *store, unsigned long ttl, time_t expiration, void *data, int persists, int relaxed);
PUBLIC_FUNC_DECL(cached_object_t *, add_cached_object_cmp,        const char *id, const void *key, cached_store_t *store, unsigned long ttl, time_t expiration, void *data, int persists, int relaxed, cached_store_comparator_t cmpfn);
PUBLIC_FUNC_DECL(cached_object_t *, add_cached_object_forced,     const char *id, cached_store_t *store, unsigned long ttl, time_t expiration, void *data, int persists, int relaxed);
PUBLIC_FUNC_DECL(cached_object_t *, add_cached_object_cmp_forced, const char *id, const void *key, cached_store_t *store, unsigned long ttl, time_t expiration, void *data, int persists, int relaxed, cached_store_comparator_t cmpfn);

// Removing and destroying objects in the cache.
PUBLIC_FUNC_DECL(int,               remove_cached_object,         const char *oid, cached_store_t *store);
PUBLIC_FUNC_DECL(int,               remove_cached_object_cmp,     const void *key, cached_store_t *store, cached_store_comparator_t cmpfn);
PUBLIC_FUNC_DECL(void,              destroy_cache_entry,          cached_object_t *entry);

// Other.
PUBLIC_FUNC_DECL(void *,            get_cache_obj_data,           cached_object_t *object);


// Internal functions

// Generic cache functions.
cached_object_t * _create_cached_object(cached_data_type_t dtype, unsigned long ttl, time_t expiration, void *data, int persists, int relaxed);
cached_store_t *  _get_cached_store_by_type(cached_data_type_t dtype);
void              _dump_cache(cached_data_type_t dtype, int do_data, int ephemeral);
void              _dump_cache_data(FILE *fp, const cached_object_t *obj, int brief);
cached_object_t * _clone_cached_object(const cached_object_t *obj);

// Helper functions for writing object data to the persistent cache.
size_t            _mem_append_serialized(unsigned char **buf, size_t *blen, const unsigned char *data, size_t dlen);
size_t            _mem_append_serialized_string(unsigned char **buf, size_t *blen, const char *string);
size_t            _mem_append_serialized_array(unsigned char **buf, size_t *blen, const unsigned char **array, size_t itemsize);
size_t            _mem_append_serialized_str_array(unsigned char **buf, size_t *blen, const char **array);
size_t            _mem_append_serialized_array_cb(unsigned char **buf, size_t *blen, const char **array, custom_serializer_t sfn);

// Helper functions for deserializing object data from the persistent cache into memory.
unsigned int      _deserialize_data(unsigned char *dst, unsigned char **bufptr, const unsigned char *bufend, size_t len);
unsigned char *   _deserialize_vardata(unsigned char **bufptr, const unsigned char *bufend, size_t *outlen);
unsigned int      _deserialize_string(char **dst, unsigned char **bufptr, const unsigned char *bufend);
unsigned int      _deserialize_array(unsigned char ***dst, unsigned char **bufptr, const unsigned char *bufend, size_t itemsize);
unsigned int      _deserialize_str_array(char ***dst, unsigned char **bufptr, const unsigned char *bufend);
unsigned int      _deserialize_array_cb(unsigned char ***dst, unsigned char **bufptr, const unsigned char *bufend, custom_deserializer_t dfn);

// Functions for removing/replacing/checking cache expiration.
int               _is_object_expired(cached_object_t *obj, int *refresh);
cached_object_t * _unlink_object(cached_object_t *object, int destroy, int stale);
cached_object_t * _replace_object(cached_object_t *oobj, cached_object_t *nobj, int shadow);
unsigned int      _evict_if_stale(cached_object_t **objptr);

// Synchronization of the cache stores.
void              _lock_cache_store(cached_store_t *store);
void              _unlock_cache_store(cached_store_t *store);


/* signet callbacks*/
void *                  _deserialize_signet_cb(void *data, size_t len);
void *                  _serialize_signet_cb(void *record, size_t *outlen);
void                    _destroy_signet_cb(void *record);
void                    _dump_signet_cb(FILE *fp, void *record, int brief);



#endif
