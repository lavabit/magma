#include <stdint.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <pthread.h>

#include "dime/signet-resolver/cache.h"
#include "dime/signet-resolver/dns.h"
#include "dime/signet-resolver/mrec.h"
#include "dime/signet-resolver/signet-ssl.h"
#include "dime/signet/keys.h"
#include "dime/signet/signet.h"
#include "dime/common/misc.h"
#include "dime/common/error.h"



// The permissions for the cache - all of them by default.
static unsigned int _cache_flags = CACHE_PERM_DEFAULT;

static char *_cachefile = NULL;
static char *_dime_dir = NULL;
static uid_t _last_uid = 0;

// This is the global table that stores all the cache management functions for the different types of data supported by the object cache.
cached_store_t cached_stores[cached_data_signet + 1] = {
    { cached_data_unknown, "unknown", 0, NULL, PTHREAD_MUTEX_INITIALIZER, NULL, NULL, NULL, NULL, NULL },
    { cached_data_drec, "DIME management records", 0, NULL, PTHREAD_MUTEX_INITIALIZER, &_destroy_dime_record_cb,
      &_serialize_dime_record_cb, &_deserialize_dime_record_cb, &_dump_dime_record_cb, NULL },
    { cached_data_dnskey, "DNSKEY records", 1, NULL, PTHREAD_MUTEX_INITIALIZER, &_destroy_dnskey_record_cb,
      &_serialize_dnskey_record_cb, &_deserialize_dnskey_record_cb, &_dump_dnskey_record_cb, &_clone_dnskey_record_cb },
    { cached_data_ds, "DS records", 1, NULL, PTHREAD_MUTEX_INITIALIZER, &_destroy_ds_record_cb,
      &_serialize_ds_record_cb, &_deserialize_ds_record_cb, &_dump_ds_record_cb, NULL },
    { cached_data_ocsp, "OCSP", 1, NULL, PTHREAD_MUTEX_INITIALIZER, &_destroy_ocsp_response_cb,
      &_serialize_ocsp_response_cb, &_deserialize_ocsp_response_cb, &_dump_ocsp_response_cb, NULL },
    { cached_data_signet, "signets", 0, NULL, PTHREAD_MUTEX_INITIALIZER, &_destroy_signet_cb,
      &_serialize_signet_cb, &_deserialize_signet_cb, &_dump_signet_cb, NULL }
};


/**
 * @brief   Get a pointer to a cached object store by its type.
 * @param   dtype   the numerical identifier of the cached data type.
 * @return  a pointer to the requested cached object store on success, or NULL on failure.
 */
cached_store_t *_get_cached_store_by_type(cached_data_type_t dtype) {

    if (!dtype || (dtype > (sizeof(cached_stores) / sizeof(cached_store_t)))) {
        return NULL;
    }

    return &(cached_stores[dtype]);
}


/**
 * @brief   Get the full pathname of the DIME base directory.
 * @param   suffix  if not NULL, an optional suffix containing a filename or path to be appended to the end of the DIME directory name.
 * @return  NULL on failure, or a null-terminated string containing the requested DIME directory pathname on success.
 */
char *_get_dime_dir_location(const char *suffix) {

    struct passwd pw, *pwresult;
    struct stat sb;
    char *result = NULL, *cdir = NULL, *pwbuf;
    long pwsize;

    if (!suffix) {
        suffix = "";
    }

    if (_dime_dir && _last_uid == getuid()) {

        if (!str_printf(&result, "%s/%s", _dime_dir, suffix)) {
            RET_ERROR_PTR(ERR_NOMEM, "could not allocate space for DIME directory pathname");
        }

        return result;
    }

    _last_uid = getuid();

    if (_dime_dir) {
        free(_dime_dir);
        _dime_dir = NULL;
    }

    // This probably shouldn't be a fatal error.
    if ((pwsize = sysconf(_SC_GETPW_R_SIZE_MAX)) == -1) {
        pwsize = 8192;
    }

    if (!(pwbuf = malloc(pwsize))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_PTR(ERR_NOMEM, "could not allocate space for user account info");
    }

    if (getpwuid_r(_last_uid, &pw, pwbuf, pwsize, &pwresult)) {
        PUSH_ERROR_SYSCALL("getpwuid_r");
        free(pwbuf);
        RET_ERROR_PTR(ERR_UNSPEC, "error encountered while looking up user account info");
    } else if (!pwresult) {
        free(pwbuf);
        RET_ERROR_PTR(ERR_UNSPEC, "could not look up user account info");
    }

    if (!str_printf(&cdir, "%s/.dime", pwresult->pw_dir)) {
        free(pwbuf);
        RET_ERROR_PTR(ERR_NOMEM, "could not allocate buffer for cache directory name");
    }

    free(pwbuf);

    if (stat(cdir, &sb) < 0) {

        // The directory didn't exist, so try to create it.
        if (errno == ENOENT) {

            if (mkdir(cdir, S_IRWXU) < 0) {
                PUSH_ERROR_SYSCALL("mkdir");
                free(cdir);
                RET_ERROR_PTR(ERR_UNSPEC, "could not create user DIME storage directory in ~/.dime");
            }

        } else {
            PUSH_ERROR_SYSCALL("stat");
            free(cdir);
            RET_ERROR_PTR(ERR_UNSPEC, "error looking up user DIME storage directory");
        }

        // Make sure it's actually a directory.
    } else if (!S_ISDIR(sb.st_mode)) {
        free(cdir);
        RET_ERROR_PTR(ERR_UNSPEC, "~/.dime exists but it is not a directory");
    }

    _dime_dir = cdir;

    // The .dime directory exists, so append the cache filename to it.
    if (!str_printf(&result, "%s/%s", _dime_dir, suffix)) {
        RET_ERROR_PTR(ERR_NOMEM, "could not allocate space for DIME directory pathname");
    }

    return result;
}


/**
 * @brief   Get the full pathname of the DIME cache file.
 * @note    The default location returned by this function can be overridden by setting the DIME_CACHE_FILE environment variable.
 * @return  NULL on failure, or a null-terminated string containing the filename of the DIME object cache file on success.
 * @free_using{free}
 */
char *_get_cache_location(void) {

    char *result;
    char *cfile = NULL;

    // If the path has already been set explicitly, return that.
    if (_cachefile) {

        if (!(result = strdup(_cachefile))) {
            PUSH_ERROR_SYSCALL("strdup");
            RET_ERROR_PTR(ERR_NOMEM, "could not get cache filename because of memory allocation problem");
        }

        return result;
    }


    // Before we calculate the default cache file path, check to see if it's been set from the environment.
    if ((cfile = getenv("DIME_CACHE_FILE"))) {

        if (!(result = strdup(cfile))) {
            PUSH_ERROR_SYSCALL("strdup");
            RET_ERROR_PTR(ERR_NOMEM, "could not allocate space for cache filename");
        }

        _dbgprint(2, "Cache location set from environment: %s\n", cfile);
        return result;
    }

    free(cfile);

    // Otherwise get it through the default DIME path.
    if (!(result = _get_dime_dir_location(".cache"))) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not determine default DIME directory path");
    }

    return result;
}


/**
 *  @brief  Set the pathname of the DIME management record cache file.
 *  @param  path    a null-terminated string containing the fully qualified path of the cache file to be used.
 *  @return 0 on success or -1 on failure.
 */
int _set_cache_location(const char *path) {

    if (!path) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if (*path != '/') {
        RET_ERROR_INT(ERR_BAD_PARAM, "cache filename must be full pathname");
    }

    _last_uid = getuid();

    if (_cachefile) {
        free(_cachefile);
    }

    if (!(_cachefile = strdup(path))) {
        PUSH_ERROR_SYSCALL("strdup");
        RET_ERROR_INT(ERR_NOMEM, "could not allocate space for cache filename");
    }

    _dbgprint(2, "Cache location set explicitly: %s\n", path);

    return 0;
}


/**
 * @brief   Allocate and initialize a new cached object.
 * @see     _add_cached_object()
 * @note    The data parameter specifies an object-specific value that will be passed to other parts
 *              of the cache management subsystem, such as functions for deletion, destruction, search, etc.
 * @param   dtype       the data type identifying the cached store to which this cached object will belong.
 * @param   ttl     if not 0, an optional time-to-live value in seconds specifying in how many
 *                              seconds from now the cached object will reach expiration.
 * @param   expiration  if not 0, an optional UTC time value specifying the exact time at which this
 *                              cached object will reach expiration.
 * @param   data        a pointer to the object-specific data to be associated with the cache entry.
 * @param   persists    if set, the cached object will be persisted to disk when the cache is saved.
 * @param   relaxed     if set, use a relaxed cache policy. A relaxed cache policy ensures that even if
 *                              the entry's ttl has expired, if its absolute (UTC) expiration time has not been
 *                              reached, it will not be evicted from the cache, but will allow for a notification
 *                              to the caller that it is time for the cached entry to be refreshed.
 * @return  a pointer to a newly allocated cached object for the specified data on success, or NULL on failure.
 */
cached_object_t *_create_cached_object(cached_data_type_t dtype, unsigned long ttl, time_t expiration, void *data, int persists, int relaxed) {

    cached_object_t *result;
    time_t now;

    if (time(&now) == (time_t)-1) {
        PUSH_ERROR_SYSCALL("time");
        RET_ERROR_PTR(ERR_UNSPEC, "could not calculate timestamp for new cached object");
    }

    if (!(result = malloc(sizeof(cached_object_t)))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_PTR(ERR_NOMEM, "could not allocate space for new cached object");
    }

    // Initialize the new object.
    memset(result, 0, sizeof(cached_object_t));

    result->timestamp = now;
    result->dtype = dtype;
    result->ttl = ttl;
    result->expiration = expiration;
    result->data = data;
    result->persists = persists;
    result->relaxed = relaxed;

    return result;
}

/**
 * @brief   Get the data associated with a cached object, freeing the parent cached object.
 * @note    This function is useful as a wrapper for returning cached data without returning the meta-information associated with
 *                      the same data that is stored inside its cached_object_t holder.
 *              If the object belongs to an internal store, no memory will be freed.
 * @param   object  a pointer to the cached object to have its opaque data returned, and enclosing structure freed.
 * @return  NULL on failure, or a pointer to the cached object's data on success.
 */
void *_get_cache_obj_data(cached_object_t *object) {

    void *result;
    cached_store_t *store;

//if (object) return (object->data); else return NULL;

    // A null object isn't an error since this function is intended to be used blindly as a wrapper.
    if (!object || !object->data) {
        return NULL;
    }

    if (!(store = _get_cached_store_by_type(object->dtype))) {
        RET_ERROR_PTR(ERR_UNSPEC, "unable to get store for cached object");
    }

    result = object->data;

    // If the object belongs to an internal store, we don't want to free it.
    if (store->internal) {
        return result;
    }

    // If it's not internal, we are dealing with a deep copy of the cached data and it's OK to release it.
    object->data = NULL;
    _destroy_cache_entry(object);

    return result;
}


/**
 * @brief   Destroy a cached object.
 * @param   entry   a pointer to the cached object to be destroyed.
 */
void _destroy_cache_entry(cached_object_t *entry) {

    cached_store_t *store;

    if (!entry) {
        return;
    }

    if (!(store = _get_cached_store_by_type(entry->dtype))) {
        fprintf(stderr, "Error destroying cached object: could not find associated cached store.\n");
    } else {

        if (store->destructor && entry->data) {
            store->destructor(entry->data);

            if (get_last_error()) {
                fprintf(stderr, "Cache destructor generated error(s):\n");
                dump_error_stack();
                _clear_error_stack();
            }

        }

    }

    // TODO: What should we do if there is a cached object being shadowed by this one?

    memset(entry, 0, sizeof(cached_object_t));
    free(entry);

}


/**
 * @brief   Find a cached object in a cached store.
 * @param   oid a null-terminated string containing the unique name or identifier of the cached object.
 * @param   store   a pointer to the cached store to be searched for the target object.
 * @return  a pointer to the specified cached object, if found, or NULL on failure.
 */
cached_object_t *_find_cached_object(const char *oid, cached_store_t *store) {

    cached_object_t *ptr;
    unsigned char hashid[SHA_256_SIZE];

    if (!oid || !store) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (!(_cache_flags & CACHE_PERM_READ)) {
        RET_ERROR_PTR(ERR_PERM, NULL);
    }

    // We use a SHA-256 hash of the object's identifier to locate it in the cache.
    if (_compute_sha_hash(256, (unsigned char *)oid, strlen(oid), hashid) < 0) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not compute SHA hash of cached object name");
    }

    _lock_cache_store(store);
    ptr = store->head;

    while (ptr) {

        // As we go along, evict any stale items.
        if (_evict_if_stale(&ptr)) {
            continue;
        }

        if (!memcmp(ptr->id, hashid, SHA_256_SIZE)) {
            ptr = _clone_cached_object(ptr);
            _unlock_cache_store(store);

            if (!ptr) {
                RET_ERROR_PTR(ERR_UNSPEC, "unable to create deep copy of cloned object");
            }

            return ptr;
        }

        ptr = ptr->next;
    }

    _unlock_cache_store(store);

    return NULL;
}


/**
 * @brief   Find a cached object in a cached store using a custom comparator.
 * @note    This function is used when the simple unique object ID is not sufficient to identify an object.
 * @param   key a pointer to an object-specific key that will be passed to the custom comparison function.
 * @param   store   a pointer to the cached store in which to search for the cached object.
 * @param   cmpfn   a custom comparison function that will compare the key value to the objects in the
 *                      specified cached store and determine whether the two objects match one another.
 * @return  a pointer to the found cached object on success, or NULL on failure.
 */
cached_object_t *_find_cached_object_cmp(const void *key, cached_store_t *store, cached_store_comparator_t cmpfn) {

    cached_object_t *ptr;

    if (!key || !store || !cmpfn) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (!(_cache_flags & CACHE_PERM_READ)) {
        RET_ERROR_PTR(ERR_PERM, NULL);
    }

    _lock_cache_store(store);
    ptr = store->head;

    while (ptr) {

        // As we go along, evict any stale items.
        if (_evict_if_stale(&ptr)) {
            continue;
        }

        if (!cmpfn(ptr->data, key)) {
            ptr = _clone_cached_object(ptr);
            _unlock_cache_store(store);

            if (!ptr) {
                RET_ERROR_PTR(ERR_UNSPEC, "unable to create deep copy of cloned object");
            }

            return ptr;
        }

        ptr = ptr->next;
    }

    _unlock_cache_store(store);

    return NULL;
}


/**
 * @brief   Check by a cached object's hashed id to see if it already exists in a cached object store.
 * @param   store   a pointer to the cached store in which to search for the cached object.
 * @param   hashid  the (already) hashed unique id of the object to be located in the cache.
 * @return  1 if the object described was present in the cache, 0 if it was not, or -1 on error.
 */
int _cached_object_exists(const unsigned char *hashid, cached_store_t *store) {

    cached_object_t *ptr;

    if (!store || !hashid) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if (!(_cache_flags & CACHE_PERM_READ)) {
        RET_ERROR_INT(ERR_UNSPEC, "no permission to read cache contents");
    }

    _lock_cache_store(store);
    ptr = store->head;

    // If the store is empty, don't worry; if not, see that we don't already exist.
    while (ptr) {

        // As we go along, evict any stale items.
        if (_evict_if_stale(&ptr)) {
            continue;
        }

        if (!memcmp(ptr->id, hashid, SHA_256_SIZE)) {
            _unlock_cache_store(store);
            return 1;
        }

        ptr = ptr->next;
    }

    _unlock_cache_store(store);

    return 0;
}


/**
 * @brief   Check to see if an object exists in a cached store by using a custom comparator function.
 * @note    This function is used when the simple unique object ID is not enough to identify an object.
 * @param   key a pointer to an object-specific key that will be passed to the comparison function.
 * @param   store   a pointer to the cached store in which to search for the cached object.
 * @param   cmpfn   a comparison function that will compare the key value to the objects in the
 *                      specified cached store, and determine whether the two objects match one another.
 * @return  1 if the object described was present in the cache, 0 if it was not, or -1 on error.
 */
int _cached_object_exists_cmp(const void *key, cached_store_t *store, cached_store_comparator_t cmpfn) {

    cached_object_t *ptr;

    if (!key || !store || !cmpfn) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if (!(_cache_flags & CACHE_PERM_READ)) {
        RET_ERROR_INT(ERR_UNSPEC, "no permission to read cache contents");
    }

    _lock_cache_store(store);
    ptr = store->head;

    // If the store is empty, don't worry; if not, see that we don't already exist.
    while (ptr) {

        // As we go along, evict any stale items.
        if (_evict_if_stale(&ptr)) {
            continue;
        }

        if (!cmpfn(ptr->data, key)) {
            _unlock_cache_store(store);
            return 1;
        }

        ptr = ptr->next;
    }

    _unlock_cache_store(store);

    return 0;
}


/**
 * @brief   Create and add a cached object to a cached store.
 * @param   id      a unique identifier to be associated with the cached object in the store.
 * @param   store       a pointer to the cached store that will hold the new cached object.
 * @param   ttl     an optional time-to-live value for the cached object, in seconds.
 * @param   expiration  an optional expiration date for the cached object, as a UTC time.
 * @param   data        a pointer to the object-specific data to be associated with the cache entry.
 * @param   persists    if set, the cached object will be persisted to disk when the cache is saved.
 * @param   relaxed     if set, use a relaxed cache policy; this ensures that even if the entry's TTL has
 *                              expired, if its absolute (UTC) expiration has not been reached, it will not be
 *                              evicted from the cache, but will delivery a refresh notification to the caller.
 * @return  a pointer to the newly allocated cached object for the specified data on success, or NULL on failure.
 */
cached_object_t *_add_cached_object(const char *id, cached_store_t *store, unsigned long ttl, time_t expiration, void *data, int persists, int relaxed) {

    cached_object_t *ptr, *entry, *optr, *result;
    void *odata;
    unsigned char hashid[SHA_256_SIZE];

    if (!id || !store) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (!(_cache_flags & CACHE_PERM_ADD)) {
        RET_ERROR_PTR(ERR_PERM, NULL);
    }

    if (_compute_sha_hash(256, (unsigned char *)id, strlen(id), hashid) < 0) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not compute SHA hash of new cache entry");
    }

    _lock_cache_store(store);
    ptr = store->head;

    // If the store is empty, don't worry; if not, see that we don't already exist.
    while (ptr) {

        // As we go along, evict any stale items.
        if (_evict_if_stale(&ptr)) {
            continue;
        }

        if (!memcmp(ptr->id, hashid, SHA_256_SIZE)) {
            _unlock_cache_store(store);
            RET_ERROR_PTR_FMT(ERR_UNSPEC, "could not add cached object to store because object id already exists: %s", id);
        }

        ptr = ptr->next;
    }

    // TODO: Can't we use _create_cached_object() here?
    if (!(entry = malloc(sizeof(cached_object_t)))) {
        PUSH_ERROR_SYSCALL("malloc");
        _unlock_cache_store(store);
        RET_ERROR_PTR(ERR_NOMEM, "could not allocate space for new cached object");
    }

    // Initialize the new object.
    memset(entry, 0, sizeof(cached_object_t));

    if (time(&(entry->timestamp)) == (time_t)-1) {
        PUSH_ERROR_SYSCALL("time");
        free(entry);
        _unlock_cache_store(store);
        RET_ERROR_PTR(ERR_UNSPEC, "could not calculate timestamp for new cached object");
    }

    memcpy(entry->id, hashid, SHA_256_SIZE);
    entry->dtype = store->dtype;
    entry->ttl = ttl;
    entry->expiration = expiration;
    entry->data = data;
    entry->persists = persists;
    entry->relaxed = relaxed;

    // Prepend it to the beginning of the linked list.
    optr = store->head;
    store->head = entry;
    entry->next = optr;

    if (optr) {
        optr->prev = entry;
    }

    result = _clone_cached_object(entry);

    // Protect against misbehaving callers who don't check this function return value, and continue to reference the original data address, instead of the returned value.
    if (result && (result->data != entry->data)) {
        odata = entry->data;
        entry->data = result->data;
        result->data = odata;
    }

    _unlock_cache_store(store);

    if (!result) {
        RET_ERROR_PTR(ERR_UNSPEC, NULL);
    }

    return result;
}


/**
 * @brief   Create and add an object to the cache, forcing the removal of any existing object with a clashing id.
 * @note    If an object with the same identifier already exists, it will be stored as the "shadow" data of the
 *              newly created cached entry. This allows for shadowed data to be saved to disk on a cache save, even
 *              if the over-shadowing data is not marked as persistent.
 * @param   id      a unique identifier to be associated with the cached object in the store.
 * @param   store       a pointer to the cached store that will hold the new cached object.
 * @param   ttl     an optional time-to-live value for the cached object, in seconds.
 * @param   expiration  an optional expiration date for the cached object, as a UTC time.
 * @param   data        a pointer to the object-specific data to be associated with the cache entry.
 * @param   persists    if set, the cached object will be persisted to disk when the cache is saved.
 * @param   relaxed     if set, use a relaxed cache policy; this ensures that even if the entry's TTL has
 *              expired, if its absolute (UTC) expiration has not been reached, it will not be
 *              evicted from the cache, but will delivery a refresh notification to the caller.
 * @return  a pointer to the newly allocated cached object for the specified data on success, or NULL on failure.
 */
cached_object_t *_add_cached_object_forced(const char *id, cached_store_t *store, unsigned long ttl, time_t expiration, void *data, int persists, int relaxed) {

    cached_object_t *found, *newobj, *result;
    void *odata;

    if (!id || !store) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (!(_cache_flags & CACHE_PERM_ADD)) {
        RET_ERROR_PTR(ERR_PERM, NULL);
    }

    // TODO: thread-safe code needs to be added here.
    // If we find a clashing object, remove it.
    if ((found = _find_cached_object(id, store))) {

        if (_verbose >= 1) {
            _dbgprint(1, "Forcibly overriding existing conflicting entry in cache: ");

            _dump_cache_data(stderr, found, 1);
            fprintf(stderr, "\n");

            if (_verbose >= 2) {
                _dump_cache_data(stderr, found, 0);
            }

        }

        // We create the new object to replace the old one.
        if (!(newobj = _create_cached_object(store->dtype, ttl, expiration, data, persists, relaxed))) {
            _destroy_cache_entry(found);
            RET_ERROR_PTR(ERR_UNSPEC, "unable to create new cached object");
        }

        // Call replace and make the old cache object our shadow.
        if (!_replace_object(found, newobj, 1)) {
            _destroy_cache_entry(newobj);
            RET_ERROR_PTR(ERR_UNSPEC, "unable to replace entry in cache");
        }

        result = _clone_cached_object(newobj);

        // Protect against misbehaving callers who don't check this function return value, and continue to reference the original data address, instead of the returned value.
        if (result && (result->data != newobj->data)) {
            odata = newobj->data;
            newobj->data = result->data;
            result->data = odata;
        }

    } else {
        // Otherwise we still have to add it normally.
        result = _add_cached_object(id, store, ttl, expiration, data, persists, relaxed);
    }

    if (!result) {
        RET_ERROR_PTR(ERR_UNSPEC, NULL);
    }

    return result;
}


/**
 * @brief   Create and add a cached object to a cached store using a custom comparator check for collisions.
 * @param   id      a unique identifier to be associated with the cached object in the store.
 * @param   key     a pointer to an object-specific key that will be passed to the custom comparison function.
 * @param   store       a pointer to the cached store that will hold the new cached object.
 * @param   ttl     an optional time-to-live value for the cached object, in seconds.
 * @param   expiration  an optional expiration date for the cached object, as a UTC time.
 * @param   data        a pointer to the object-specific data to be associated with the cache entry.
 * @param   persists    if set, the cached object will be persisted to disk when the cache is saved.
 * @param   relaxed     if set, use a relaxed cache policy; this ensures that even if the entry's TTL has
 *                              expired, if its absolute (UTC) expiration has not been reached, it will not be
 *                              evicted from the cache, but will delivery a refresh notification to the caller.
 * @param   cmpfn       a custom comparison function that will compare the key value to the objects in the
 *                              specified cache dstore and determine whether the two objects match one another.
 * @return  a pointer to the newly allocated cached object for the specified data on success, or NULL on failure.
 */
cached_object_t *_add_cached_object_cmp(const char *id, const void *key, cached_store_t *store, unsigned long ttl, time_t expiration, void *data, int persists, int relaxed, cached_store_comparator_t cmpfn) {

    cached_object_t *ptr, *entry, *optr, *result;
    void *odata;
    unsigned char hashid[SHA_256_SIZE];

    if (!id || !store || !cmpfn) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (!(_cache_flags & CACHE_PERM_ADD)) {
        RET_ERROR_PTR(ERR_PERM, NULL);
    }

    if (_compute_sha_hash(256, (unsigned char *)id, strlen(id), hashid) < 0) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not compute SHA hash of new cache entry");
    }

    _lock_cache_store(store);
    ptr = store->head;

    // If the store is empty, don't worry; if not, see that we don't already exist.
    while (ptr) {

        // As we go along, evict any stale items.
        if (_evict_if_stale(&ptr)) {
            continue;
        }

        // We don't want to have an entry that clashes in ID or that fails the comparator test.
        if (!memcmp(ptr->id, hashid, SHA_256_SIZE)) {
            _unlock_cache_store(store);
            RET_ERROR_PTR(ERR_UNSPEC, "could not add cached object to store because object ID already exists");
        } else if (!cmpfn(ptr->data, key)) {
            _unlock_cache_store(store);
            RET_ERROR_PTR(ERR_UNSPEC, "could not add cached object to store because a similar object already exists");
        }

        ptr = ptr->next;
    }

    if (!(entry = _create_cached_object(store->dtype, ttl, expiration, data, persists, relaxed))) {
        _unlock_cache_store(store);
        RET_ERROR_PTR(ERR_UNSPEC, "unable to create new cached object");
    }

    // The only additional field that needs to be set for the cached object is the hashed id.
    memcpy(entry->id, hashid, SHA_256_SIZE);

    // Prepend it to the beginning of the linked list.
    optr = store->head;
    store->head = entry;
    entry->next = optr;

    if (optr) {
        optr->prev = entry;
    }

    result = _clone_cached_object(entry);

    // Protect against misbehaving callers who don't check this function return value, and continue to reference the original data address, instead of the returned value.
    if (result && (result->data != entry->data)) {
        odata = entry->data;
        entry->data = result->data;
        result->data = odata;
    }

    _unlock_cache_store(store);

    if (!result) {
        RET_ERROR_PTR(ERR_UNSPEC, NULL);
    }

    return result;
}


/**
 * @brief   Create and add an object to the cache, forcing the removal of any existing object that matches the result
 *              of a custom comparison function.
 * @param   id      a unique identifier to be associated with the cached object in the store.
 * @param   key     a pointer to an object-specific key that will be passed to the custom comparison function.
 * @param   store       a pointer to the cached store that will hold the new cached object.
 * @param   ttl     an optional time-to-live value for the cached object, in seconds.
 * @param   expiration  an optional expiration date for the cached object, as a UTC time.
 * @param   data        a pointer to the object-specific data to be associated with the cache entry.
 * @param   persists    if set, the cached object will be persisted to disk when the cache is saved.
 * @param   relaxed     if set, use a relaxed cache policy; this ensures that even if the entry's TTL has
 *                              expired, if its absolute (UTC) expiration has not been reached, it will not be
 *                              evicted from the cache, but will delivery a refresh notification to the caller.
 * @param   cmpfn       a custom comparison function that will compare the key value to the objects in the
 *                              specified cache dstore and determine whether the two objects match one another.
 * @return  a pointer to the newly allocated cached object for the specified data on success, or NULL on failure.
 */
cached_object_t *_add_cached_object_cmp_forced(const char *id, const void *key, cached_store_t *store, unsigned long ttl, time_t expiration, void *data, int persists, int relaxed, cached_store_comparator_t cmpfn) {

    cached_object_t *found, *newobj, *result;
    void *odata;

    if (!id || !store) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (!(_cache_flags & CACHE_PERM_ADD)) {
        RET_ERROR_PTR(ERR_PERM, NULL);
    }

    // TODO: Needs to be made thread-safe.
    // If we find a clashing object, remove it.
    if ((found = _find_cached_object_cmp(key, store, cmpfn)) || (found = _find_cached_object(id, store))) {

        if (_verbose >= 1) {
            _dbgprint(1, "Forcibly overriding existing conflicting entry in cache: ");
            _dump_cache_data(stderr, found, 1);

            if (_verbose >= 2) {
                _dump_cache_data(stderr, found, 0);
            }

        }

        // We create the new object to replace the old one.
        if (!(newobj = _create_cached_object(store->dtype, ttl, expiration, data, persists, relaxed))) {
            _destroy_cache_entry(found);
            RET_ERROR_PTR(ERR_UNSPEC, "unable to create new cached object");
        }

        // Call replace and make the old cache object our shadow.
        if (!_replace_object(found, newobj, 1)) {
            _destroy_cache_entry(newobj);
            RET_ERROR_PTR(ERR_UNSPEC, "unable to replace entry in cache");
        }

        result = _clone_cached_object(newobj);

        // Protect against misbehaving callers who don't check this function return value,
        // and continue to reference the original data address, instead of the returned value.
        if (result && (result->data != newobj->data)) {
            odata = newobj->data;
            newobj->data = result->data;
            result->data = odata;
        }

    } else {
        result = _add_cached_object_cmp(id, key, store, ttl, expiration, data, persists, relaxed, cmpfn);
    }

    if (!result) {
        RET_ERROR_PTR(ERR_UNSPEC, NULL);
    }

    return result;
}


/**
 * @brief   Remove an object from a cached store by name.
 * @note    The caller is still responsible for freeing the cached object and its underlying data.
 * @param   oid the unique identifier of the cached object to be removed.
 * @param   store   a pointer to the cached store from which the specified object will be removed.
 * @return  1 if the object was successfully removed or 0 if it couldn't be found; -1 on general error.
 */
int _remove_cached_object(const char *oid, cached_store_t *store) {

    cached_object_t *ptr;
    unsigned char hashid[SHA_256_SIZE];

    if (!oid || !store) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if (!(_cache_flags & CACHE_PERM_DELETE)) {
        RET_ERROR_INT(ERR_PERM, NULL);
    }

    if (_compute_sha_hash(256, (unsigned char *)oid, strlen(oid), hashid) < 0) {
        RET_ERROR_INT(ERR_UNSPEC, "could not compute SHA hash of removed cache entry");
    }

    _lock_cache_store(store);
    ptr = store->head;

    // If we find the cached object, cut it from the store's linked list.
    while (ptr) {

        // As we go along, evict any stale items.
        if (_evict_if_stale(&ptr)) {
            continue;
        }

        if (!memcmp(ptr->id, hashid, SHA_256_SIZE)) {
            _unlink_object(ptr, 1, 0);
            _unlock_cache_store(store);
            return 1;
        }

        ptr = ptr->next;
    }

    _unlock_cache_store(store);

    return 0;
}
/**
 * @brief   Remove an object from a cached store using a custom comparator function.
 * @note    The caller is still responsible for freeing the cached object and its underlying data.
 * @param   key a pointer to an object-specific key that will be passed to the custom comparison function.
 * @param   store   a pointer to the cached store from which the specified object will be removed.
 * @param   cmpfn   a custom comparison function that will compare the key value to the objects in the
 *                      specified cached store and determine whether the two objects match one another.
 * @return  1 if the object was successfully removed or 0 if it couldn't be found; -1 on general error.
 */
int _remove_cached_object_cmp(const void *key, cached_store_t *store, cached_store_comparator_t cmpfn) {

    cached_object_t *ptr;

    if (!key || !store || !cmpfn) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if (!(_cache_flags & CACHE_PERM_DELETE)) {
        RET_ERROR_INT(ERR_PERM, NULL);
    }

    _lock_cache_store(store);
    ptr = store->head;

    // If we find the cached object, cut it from the store's linked list and free its underlying data, and then itself.
    while (ptr) {

        // As we go along, evict any stale items.
        if (_evict_if_stale(&ptr)) {
            continue;
        }

        if (!cmpfn(ptr->data, key)) {
            _unlink_object(ptr, 1, 0);
            _unlock_cache_store(store);
            return 1;
        }

        ptr = ptr->next;
    }

    _unlock_cache_store(store);

    return 0;
}


/**
 * @brief   Clone a deep copy of a cached object for user-safe retrieval.
 */
cached_object_t *_clone_cached_object(const cached_object_t *obj) {

    cached_store_t *store;
    cached_object_t *result;
    void *data;
    size_t dsize;

    if (!obj) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (!(store = _get_cached_store_by_type(obj->dtype))) {
        RET_ERROR_PTR(ERR_UNSPEC, "attempted to clone cached data of unrecognized type");
    } else if (!store->clone && !(store->serialize && store->deserialize)) {
        RET_ERROR_PTR(ERR_UNSPEC, "cached data store lacks proper serialization handler(s)");
    }

    // Internal stores know what they're doing. It's not necessary for data to be cloned.
    if (store->internal) {
        return ((cached_object_t *)obj);
    }

    if (!(result = malloc(sizeof(cached_object_t)))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_PTR(ERR_NOMEM, NULL);
    }

    memset(result, 0, sizeof(cached_object_t));

    // Only copy over a certain number of the fields that aren't for internal use only.
    result->timestamp = obj->timestamp;
    memcpy(result->id, obj->id, sizeof(result->id));
    result->dtype = obj->dtype;
    result->ttl = obj->ttl;
    result->expiration = obj->expiration;
    result->relaxed = obj->relaxed;
    result->persists = obj->persists;

    // If the object has a clone routine, then use it. Otherwise, simulate it with serialize/deserialize.
    if (obj->data && store->clone) {

        if (!(result->data = store->clone(obj->data))) {
            free(result);
            RET_ERROR_PTR(ERR_UNSPEC, "failed to clone object data");
        }


    } else {

        if (!(data = store->serialize(obj->data, &dsize))) {
            free(result);
            RET_ERROR_PTR(ERR_UNSPEC, "object serialization failed");
        }

        if (!(result->data = store->deserialize(data, dsize))) {
            free(result);
            free(data);
            RET_ERROR_PTR(ERR_UNSPEC, "object deserialization failed");
        }

        free(data);

    }

    return result;
}


/**
 * @brief   Dump the content of the specified data cached store(s) to the console.
 * @param   dtype       the type of the object cached store to be dumped, or cached_data_unknown for all stores.
 * @param   do_data     if non-zero, dump the type-specific data associated with the cached object.
 * @param   ephemeral   if non-zero, print out ephemeral as well as persistent cache data.
 */
void _dump_cache(cached_data_type_t dtype, int do_data, int ephemeral) {

    cached_object_t *ptr;
    char *tstr, ttlstr[64], *expstr;
    time_t now;
    size_t j, total;

    for(size_t i = 1; i < sizeof(cached_stores) / sizeof(cached_store_t); i++) {

        if ((dtype != cached_data_unknown) && (cached_stores[i].dtype != dtype)) {
            continue;
        }

        fprintf(stderr, "Dumping data cached store of type: %s ...\n", cached_stores[i].description);

        _lock_cache_store(&(cached_stores[i]));

        if (!(ptr = cached_stores[i].head)) {
            fprintf(stderr, "- Skipped empty store.\n");
            _unlock_cache_store(&(cached_stores[i]));
            continue;
        }

        total = j = 0;

        while (ptr) {
            total++;
            tstr = _get_chr_date(ptr->timestamp, 1);
            memset(ttlstr, 0, sizeof(ttlstr));

            if (!ephemeral && (!ptr->persists)) {
                ptr = ptr->next;
                continue;
            }

            if (time(&now) == (time_t)-1) {
                perror("time");
                fprintf(stderr, "Error: Could not get current time for TTL calculation.\n");
                snprintf(ttlstr, sizeof(ttlstr), "error [original = %lu]", (unsigned long)ptr->ttl);
            } else {

                if (!ptr->ttl) {
                    snprintf(ttlstr, sizeof(ttlstr), "[no ttl]");
                } else if ((time_t)(ptr->timestamp + ptr->ttl) > now) {
                    snprintf(ttlstr, sizeof(ttlstr), "%lu seconds", (unsigned long)(ptr->timestamp + ptr->ttl - now));
                } else {
                    snprintf(ttlstr, sizeof(ttlstr), "expired %lu seconds ago", (unsigned long)(now - (ptr->timestamp + ptr->ttl)));
                }

            }

            fprintf(stderr, "+++ [%u: ", (unsigned int)(j + 1));
            _dump_cache_data(stderr, ptr, 1);

            expstr = ptr->expiration ? _get_chr_date(ptr->expiration, 1) : strdup("[none]");
            fprintf(stderr, "] ttl = %s, expiration = %s, data = %s, timestamp = %s, persist = %s",
                    ttlstr, (expstr ? expstr : "[error]"), (ptr->data ? "yes" : "no"), (tstr ? tstr : "[unknown timestamp]"), (ptr->persists ? "yes" : "no"));

            if (ptr->relaxed) {
                fprintf(stderr, " [RELAXED]");
            }

            if (ptr->shadow) {
                fprintf(stderr, " [SHADOWED]\n");
            } else {
                fprintf(stderr, "\n");
            }

            free(tstr);
            free(expstr);

            if (do_data) {
                _dump_cache_data(stderr, ptr, 0);
            }

            ptr = ptr->next;
            j++;
        }

        if (total != j) {
            fprintf(stderr, "    Skipped a total of %zu non-persistent cache entries.\n", (total - j));
        }

        _unlock_cache_store(&(cached_stores[i]));
    }

}


/**
 * @brief   Dump information about a specified cached object to a file stream.
 * @param   fp  a pointer to the file stream across which the data should be dumped.
 * @param   obj a pointer to the cached object to have its details dumped.
 * @param   brief   if true, print a brief one-line description of the object; otherwise, dump full object information.
 */
void _dump_cache_data(FILE *fp, const cached_object_t *obj, int brief) {

    cached_store_t *store;

    if (!obj) {
        return;
    }

    if (!(store = _get_cached_store_by_type(obj->dtype))) {
        fprintf(stderr, "Error dumping object cache data: could not find associated cached store.\n");
        return;
    }

    if (!store->dump) {
        return;
    }

    store->dump(fp, obj->data, brief);

    // Preemptive cleanup in case there was an error.
    if (get_last_error()) {
        fprintf(stderr, "Errors were encountered while dumping cache store:\n");
        dump_error_stack();
        _clear_error_stack();
    }

}


/**
 * @brief   Persist the entire contents of the cache to local storage.
 * @return  1 if the cache was loaded successfully, 0 if it was created, or -1 on general failure.
 */
int _load_cache_contents(void) {

    cached_store_t *store;
    cached_object_t *obj;
    void *cdata = NULL, *udata, *reall_res = NULL;
    char *cfile;
    char ttlstr[64], *tstr, *expstr;
    unsigned char *uptr;
    time_t now;
    size_t clen = 0, chdr_size = 0;
    ssize_t nread;
    int cfd, res;
    uint32_t objlen;

    if (!(_cache_flags & CACHE_PERM_LOAD)) {
        RET_ERROR_INT(ERR_PERM, NULL);
    }

    // Get the name of the cache file to be opened for writing. It should be reset to empty, or created if it does not yet exist.
    if (!(cfile = _get_cache_location())) {
        RET_ERROR_INT(ERR_UNSPEC, "unable to open object cache file for reading");
    }

    // TODO: Make this thread-safe using file locking.
    if ((cfd = open(cfile, O_RDONLY)) < 0) {

        _dbgprint(4, "Cache file was not found... creating.\n");

        if ((cfd = creat(cfile, S_IRWXU)) < 0) {
            PUSH_ERROR_SYSCALL("creat");
            free(cfile);
            RET_ERROR_INT(ERR_UNSPEC, "unable to create cache file");
        }

        close(cfd);
        free(cfile);
        return 0;
    }
    free(cfile);

    // The file consists of a sequence of object chunk lengths and data.
    while (1) {

        if ((nread = read(cfd, &objlen, sizeof(objlen))) != sizeof(objlen)) {
            close(cfd);

            if (cdata) {
                free(cdata);
            }

            if (nread) {

                if (nread < 0) {
                    PUSH_ERROR_SYSCALL("read");
                }

                RET_ERROR_INT(ERR_UNSPEC, "reached unexpected end of cache data file");
            }

            // We can only reach this by reading in 0 bytes (EOF).
            return 1;
        }

        // If the objlen we read is 0, we skip forward to read the next length. This helps avoid the NULL pointer dereferencing,
        // which occurrs if cdata has not yet been allocated. Perhaps we should throw an error. TODO
        if(!objlen) {
            continue;
        }

        if (objlen > clen) {
            clen = objlen;

            if (!(reall_res = realloc(cdata, clen))) {
                PUSH_ERROR_SYSCALL("realloc");

                if(cdata) {
                    free(cdata);
                }

                close(cfd);
                RET_ERROR_INT(ERR_NOMEM, NULL);
            }

            cdata = reall_res;
            reall_res = NULL;
        }

        memset(cdata, 0, objlen);

        if (read(cfd, cdata, objlen) != objlen) {
            free(cdata);
            close(cfd);
            RET_ERROR_INT(ERR_UNSPEC, "unable to read contents of object cache");
        }

        obj = (cached_object_t *)cdata;

        // Make sure we're even able to handle this data type.
        if (!(store = _get_cached_store_by_type(obj->dtype))) {
            fprintf(stderr, "Error: read cached data of unrecognized type. Continuing...\n");
            continue;
        } else if (!store->deserialize) {
            fprintf(stderr, "Error: cached object did not have a deserialization handler. Continuing...\n");
            continue;
        }

        // The cached header on disk is equivalent to the cached object structure minus the last 3 fields (data and linked list pointers).
        if (!chdr_size) {
            chdr_size = (size_t)&(obj->data) - (size_t)obj;
        }

        // We must have read in at least the size of a cached object structure.
        if (objlen < chdr_size) {
            fprintf(stderr, "Error reading in cached object data; unexpected small entry size.\n");
            continue;
        }

        // Create a new cached object and copy the header information in from the file.
        if (!(obj = malloc(sizeof(cached_object_t)))) {
            PUSH_ERROR_SYSCALL("malloc");
            free(cdata);
            close(cfd);
            RET_ERROR_INT(ERR_NOMEM, NULL);
        }

        memset(obj, 0, sizeof(cached_object_t));
        memcpy(obj, cdata, chdr_size);

        // Everything that was loaded from the cache is automatically persisted again.
        obj->persists = 1;

        // Finally, attach the store-specific data to the cached object.
        if (objlen >= chdr_size) {
            uptr = (unsigned char *)cdata;
            uptr += chdr_size;

            if (!(udata = store->deserialize(uptr, (objlen - chdr_size)))) {
                fprintf(stderr, "Error: cached object could not be deserialized (continuing)...\n");
                dump_error_stack();
                _clear_error_stack();
                free(obj);
                continue;
            }

            obj->data = udata;
        }

        // TODO: This should leverage _dump_cache() functionality.
        tstr = _get_chr_date(obj->timestamp, 1);
        memset(ttlstr, 0, sizeof(ttlstr));

        if (time(&now) == (time_t)-1) {
            perror("time");
            fprintf(stderr, "Error: Could not get current time for TTL calculation.\n");
            snprintf(ttlstr, sizeof(ttlstr), "error [original = %lu]", (unsigned long)obj->ttl);
        } else {

            if ((time_t)(obj->timestamp + obj->ttl) > now) {
                snprintf(ttlstr, sizeof(ttlstr), "%lu seconds", (unsigned long)(obj->timestamp + obj->ttl - now));
            } else {
                snprintf(ttlstr, sizeof(ttlstr), "expired %lu seconds ago", (unsigned long)(now - (obj->timestamp + obj->ttl)));
            }

        }

        if (_verbose >= 5) {
            expstr = obj->expiration ? _get_chr_date(obj->expiration, 1) : strdup("[none]");
            fprintf(stderr, "+++ type = %u, ttl = %s, expiration = %s, data = %s, timestamp = %s\n",
                    (unsigned int)obj->dtype, ttlstr, (expstr ? expstr : "[error]"), (obj->data ? "yes" : "no"), (tstr ? tstr : "[unknown timestamp]"));
            _dump_cache_data(stderr, obj, 0);

            if (expstr) {
                free(expstr);
            }

        }

        if (tstr) {
            free(tstr);
        }

        // Finally store the object in the cache if it doesn't already exist.
        if ((res = _cached_object_exists(obj->id, store))) {
            fprintf(stderr, "Error: deserialized cached object was a duplicate:\n");
            _dump_cache_data(stderr, obj, 0);
            _destroy_cache_entry(obj);
            continue;
        } else if (res < 0) {
            fprintf(stderr, "Error: could not look up object in cache.\n");
            dump_error_stack();
            _clear_error_stack();
            _destroy_cache_entry(obj);
            continue;
        }

        // TODO: The locking block of code needs to be expanded to include the existence check.
        _lock_cache_store(store);

        // Insert into the head of the list.
        obj->next = store->head;
        store->head = obj;

        _unlock_cache_store(store);
    }

    if (cdata) {
        free(cdata);
    }

    close(cfd);

    return 1;
}

/**
 * @brief   Persist the entire contents of the cache to local storage.
 * @return  0 on success or -1 on failure.
 */
int _save_cache_contents(void) {

    cached_object_t *ptr, *towrite;
    void *cdata;
    char *cfile;
    size_t clen, chdr_size = 0;
    int cfd;
    uint32_t objlen;

    if (!(_cache_flags & CACHE_PERM_LOAD)) {
        RET_ERROR_INT(ERR_PERM, NULL);
    }

    // Get the name of the cache file to be opened for writing. It should be reset to empty, or created if it does not yet exist.
    if (!(cfile = _get_cache_location())) {
        RET_ERROR_INT(ERR_UNSPEC, "unable to determine cache file location");
    }

    if ((cfd = open(cfile, (O_CREAT | O_TRUNC | O_WRONLY), (S_IRWXU))) < 0) {
        PUSH_ERROR_SYSCALL("open");
        PUSH_ERROR_FMT(ERR_UNSPEC, "unable to open object cache file for writing: %s", cfile);
        free(cfile);
        return -1;
    }
    free(cfile);

    for(size_t i = 1; i < sizeof(cached_stores) / sizeof(cached_store_t); i++) {
        _dbgprint(4, "Persisting cache of type: %s ...\n", cached_stores[i].description);

        _lock_cache_store(&(cached_stores[i]));

        ptr = cached_stores[i].head;

        while (ptr) {
            // If we're shadowing a cached object, then the shadowed entry is the one that needs to be persisted.
            towrite = ptr->shadow ? ptr->shadow : ptr;

            // Only bother with the entries that need to be saved.
            if (!towrite->persists) {
                ptr = ptr->next;
                continue;
            }

            // The cached header on disk is equivalent to the cached object structure minus the last 3 fields (data and linked list pointers).
            if (!chdr_size) {
                chdr_size = (size_t)&(ptr->data) - (size_t)ptr;
            }

            if (cached_stores[i].serialize) {

                if (!(cdata = cached_stores[i].serialize(towrite->data, &clen))) {
                    fprintf(stderr, "Error serializing cached data for storage:\n");
                    dump_error_stack();
                    _clear_error_stack();
                } else {
                    objlen = chdr_size + clen;

                    if ((size_t)write(cfd, &objlen, sizeof(objlen)) != sizeof(objlen)
                        || (size_t)write(cfd, towrite, chdr_size) != chdr_size
                        || (size_t)write(cfd, cdata, clen) != clen) {
                        PUSH_ERROR_SYSCALL("write");
                        _unlock_cache_store(&(cached_stores[i]));
                        free(cdata);
                        close(cfd);
                        RET_ERROR_INT(ERR_UNSPEC, "error serializing cached data to file");
                    }

                    free(cdata);
                }

            }

            ptr = ptr->next;
        }

        _unlock_cache_store(&(cached_stores[i]));
    }

    close(cfd);

    return 0;
}


/**
 * @brief   Set the permissions flags for the object cache.
 * @param   flags
 * @return  -1 on error or 0 if the permissions were set successfully.
 */
int _set_cache_permissions(unsigned long flags) {

    if (flags & ~(CACHE_PERM_ALL_FLAGS)) {
        RET_ERROR_INT(ERR_BAD_PARAM, "invalid cache permision mask");
    }

    _cache_flags = flags;

    return 0;
}


/**
 * @brief       Append a variable-length chunk of data to a dynamically allocated buffer for serialization.
 * @param   buf a pointer to the address of the output buffer that will be resized to hold the result, or NULL to allocate one.
 * @param   blen    a pointer to a variable that holds the buffer's current size and will receive the
 *                      size of the newly resized output buffer.
 * @param   data    a pointer to a buffer holding the data that will be appended to the end of the output buffer.
 * @param   dlen    the size, in bytes, of the data buffer to be appended to the output buffer.
 * @return  the new size of the (re)allocated output buffer, or 0 on failure.
 */
size_t _mem_append_serialized(unsigned char **buf, size_t *blen, const unsigned char *data, size_t dlen) {

    int res;

    if (!buf || !blen) {
        RET_ERROR_UINT(ERR_BAD_PARAM, NULL);
    }

    if (!_mem_append(buf, blen, (unsigned char *)&dlen, sizeof(dlen))) {
        RET_ERROR_UINT(ERR_NOMEM, NULL);
    }

    if (!(res = _mem_append(buf, blen, data, dlen))) {
        RET_ERROR_UINT(ERR_NOMEM, NULL);
    }

    return res;
}


/**
 * @brief   Apped a null-terminated string to a dynamically allocated buffer for serialization.
 * @param   buf a pointer to the address of the output buffer that will be resized to hold the result, or NULL to allocate one.
 * @param   blen    a pointer to a variable that holds the buffer's current size and will receive the
 *                      size of the newly resized output buffer.
 * @param   string  a null-terminated string that will be appended to the end of the output buffer.
 * @return  the new size of the (re)allocated output buffer, or 0 on failure.
 */
size_t _mem_append_serialized_string(unsigned char **buf, size_t *blen, const char *string) {

    unsigned char null = 0;
    int res;

    if (!buf || !blen) {
        RET_ERROR_UINT(ERR_BAD_PARAM, NULL);
    }

    if (!string) {
        res = _mem_append(buf, blen, &null, sizeof(null));
    } else {
        res = _mem_append(buf, blen, (unsigned char *)string, strlen(string) + 1);
    }

    if (!res) {
        RET_ERROR_UINT(ERR_NOMEM, NULL);
    }

    return res;
}


/**
 * @brief   Append an array of fixed-size objects to a dynamically allocated buffer for serialization.
 * @param   buf     a pointer to the address of the output buffer that will be resized to hold the result, or NULL to allocate one.
 * @param   blen        a pointer to a variable that holds the buffer's current size and will receive the
 *                              size of the newly resized output buffer.
 * @param   array       an array of pointers to fixed-size objects (ending with a NULL pointer) to be
 *                              appended to the end of the output buffer.
 * @param   itemsize    the size, in bytes, of the array elements to be deserialized.
 * @return  the new size of the (re)allocated output buffer, or 0 on failure.
 */
size_t _mem_append_serialized_array(unsigned char **buf, size_t *blen, const unsigned char **array, size_t itemsize) {

    unsigned char *dataholder = NULL;
    const unsigned char **arptr = array;
    size_t holdlen = 0, result;

    if (!buf || !blen) {
        RET_ERROR_UINT(ERR_BAD_PARAM, NULL);
    }

    // If there's no data, then just write a null size.
    if (!array) {

        if (!(result = _mem_append(buf, blen, (unsigned char *)&holdlen, sizeof(holdlen)))) {
            RET_ERROR_UINT(ERR_NOMEM, NULL);
        }

        return result;
    }

    // First construct a buffer to hold all the data in the array together in one contiguous space.
    while (*arptr) {

        if (!_mem_append(&dataholder, &holdlen, *arptr, itemsize)) {

            if (dataholder) {
                free(dataholder);
            }

            RET_ERROR_UINT(ERR_NOMEM, "unable to allocate space for serialized data array");
        }

        arptr++;
    }

    result = _mem_append_serialized(buf, blen, dataholder, holdlen);
    free(dataholder);

    if (!result) {
        RET_ERROR_UINT(ERR_NOMEM, NULL);
    }

    return result;
}


/**
 * @brief   Append an array of null-terminated strings to a dynamically allocated buffer for serialization.
 * @param   buf a pointer to the address of the output buffer that will be resized to hold the result, or NULL to allocate one.
 * @param   blen    a pointer to a variable that holds the buffer's current size and will receive the
 *                      size of the newly resized output buffer.
 * @param   array   an array of pointers to null-terminated strings (ending with a NULL pointer) to be
 *                      appended to the end of the output buffer.
 * @return  the new size of the (re)allocated output buffer, or 0 on failure.
 */
size_t _mem_append_serialized_str_array(unsigned char **buf, size_t *blen, const char **array) {

    unsigned char *strholder = NULL;
    const char **arptr = array;
    size_t holdlen = 0, result;

    if (!buf || !blen) {
        RET_ERROR_UINT(ERR_BAD_PARAM, NULL);
    }

    // If there's no data, then just write a null size.
    if (!array) {

        if (!(result = _mem_append(buf, blen, (unsigned char *)&holdlen, sizeof(holdlen)))) {
            RET_ERROR_UINT(ERR_NOMEM, NULL);
        }

        return result;
    }

    // First construct a buffer to hold all the string data in the array together in one contiguous space.
    // This will consist of all strings end-to-end (along with their terminating null bytes).
    while (*arptr) {

        if (!_mem_append(&strholder, &holdlen, (unsigned char *)*arptr, strlen(*arptr) + 1)) {

            if (strholder) {
                free(strholder);
            }

            RET_ERROR_UINT(ERR_NOMEM, "unable to allocate space for serialized string array");
        }

        arptr++;
    }

    result = _mem_append_serialized(buf, blen, strholder, holdlen);
    free(strholder);

    if (!result) {
        RET_ERROR_UINT(ERR_NOMEM, NULL);
    }

    return result;
}


/**
 * @brief   Append an array of custom-formatted data to a dynamically allocated buffer for serialization.
 * @note    This function uses a custom serialization function in order to output the user data properly.
 * @param   buf a pointer to the address of the output buffer that will be resized to hold the result, or NULL to allocate one.
 * @param   blen    a pointer to a variable that holds the buffer's current size and will receive the
 *                      size of the newly resized output buffer.
 * @param   array   an array of pointers to custom-formatted data buffers (ending with a NULL pointer) to be
 *                      appended to the end of the output buffer using a custom serialization function.
 * @param   sfn a custom serialization function that will be used to serialize the user data into the output buffer.
 * @return  the new size of the (re)allocated output buffer, or 0 on failure.
 */
size_t _mem_append_serialized_array_cb(unsigned char **buf, size_t *blen, const char **array, custom_serializer_t sfn) {

    unsigned char *dataholder = NULL;
    const char **arptr = array;
    size_t holdlen = 0, result;

    if (!buf || !blen || !sfn) {
        RET_ERROR_UINT(ERR_BAD_PARAM, NULL);
    }

    // If there's no data, then just write a null size.
    if (!array) {

        if (!(result = _mem_append(buf, blen, (unsigned char *)&holdlen, sizeof(holdlen)))) {
            RET_ERROR_UINT(ERR_NOMEM, NULL);
        }

        return result;
    }

    // First construct a buffer to hold all the data in the array together in one contiguous space.
    while (*arptr) {

        if (!sfn(&dataholder, &holdlen, *arptr)) {

            if (dataholder) {
                free(dataholder);
            }

            RET_ERROR_UINT(ERR_NOMEM, "unable to allocate space to add serialized data array to object cache");
        }

        arptr++;
    }

    result = _mem_append_serialized(buf, blen, dataholder, holdlen);
    free(dataholder);

    if (!result) {
        RET_ERROR_UINT(ERR_NOMEM, NULL);
    }

    return result;
}


/**
 * @brief   Deserialize a fixed-sized chunk of data from an input buffer.
 * @note    This is the inverse function of _mem_append();
 *              dst must be at least len bytes long.
 * @param   dst     a pointer to the output buffer that will receive the deserialized data.
 * @param   bufptr      a pointer to the address of the input buffer holding the data to be deserialized,
 *                              which will be updated after the operation to point to the next available data.
 * @param   bufend      a pointer to the end of the input buffer for validation purposes.
 * @param   len     the size of the data chunk to be deserialized from the input buffer.
 * @return  1 on success or 0 on failure.
 */
unsigned int _deserialize_data(unsigned char *dst, unsigned char **bufptr, const unsigned char *bufend, size_t len) {

    size_t left;

    if (!dst || !bufptr || !bufend) {
        RET_ERROR_UINT(ERR_BAD_PARAM, NULL);
    }

    left = bufend - *bufptr;
    if (len > left) {
        RET_ERROR_UINT_FMT(ERR_UNSPEC, "could not deserialize data due to buffer underflow (%zu of %zu bytes)", len, left);
    }

    memcpy(dst, *bufptr, len);
    *bufptr += len;

    return 1;
}


/**
 * @brief   Deserialize a variable-length chunk of data from an input buffer.
 * @note    This is the inverse function of _mem_append_serialized.
 * @param   bufptr      a pointer to the address of the input buffer holding the data to be deserialized,
 *                              which will be updated after the operation to point to the next available data.
 * @param   bufend      a pointer to the end of the input buffer for validation purposes.
 * @param   olen        an optional pointer to receive the size of the deserialized data on success.
 * @return  a pointer to a newly allocated buffer containing the deserialized data on success, or NULL on failure.
 */
unsigned char *_deserialize_vardata(unsigned char **bufptr, const unsigned char *bufend, size_t *olen) {

    unsigned char *result;
    size_t dlen;

    if (!bufptr || !bufend) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    // First read in the data length.
    if (!_deserialize_data((unsigned char *)&dlen, bufptr, bufend, sizeof(dlen))) {
        RET_ERROR_UINT(ERR_UNSPEC, "could not read serialized data length");
    }

    if (!(result = malloc(dlen))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_PTR(ERR_NOMEM, "could not allocate space for deserialized data");
    }

    if (!_deserialize_data(result, bufptr, bufend, dlen)) {
        free(result);
        RET_ERROR_PTR(ERR_UNSPEC, "variable data deserialization failed");
    }

    if (olen) {
        *olen = dlen;
    }

    return result;
}


/**
 * @brief   Deserialize a null-terminated string from an input buffer.
 * @note    This is the inverse function of _mem_append_serialized_string().
 * @param   dst     a pointer to the address of a string that will receive a copy of the deserialized
 *                              data as a null-terminated string on success.
 * @param   bufptr      a pointer to the address of the input buffer holding the data to be deserialized,
 *                              which will be updated after the operation to point to the next available data.
 * @param   bufend      a pointer to the end of the input buffer for validation purposes.
 * @return  1 on success or 0 on failure.
 */
unsigned int _deserialize_string(char **dst, unsigned char **bufptr, const unsigned char *bufend) {

    unsigned char *bptr;
    size_t rsize;

    if (!dst || !bufptr || !*bufptr || !bufend) {
        RET_ERROR_UINT(ERR_BAD_PARAM, NULL);
    }

    // Empty string succeeds but returns a NULL pointer, not an empty string.
    if (!**bufptr) {
        *dst = NULL;
        *bufptr = *bufptr + 1;
        return 1;
    }

    bptr = *bufptr;
    while (*bptr && bptr < bufend) {
        bptr++;
    }

    if (bptr == bufend) {
        RET_ERROR_UINT(ERR_UNSPEC, "string deserialization failed due to buffer underflow");
    }

    rsize = bptr - *bufptr + 1;

    if (!(*dst = malloc(rsize))) {
        PUSH_ERROR_SYSCALL("malloc");
        RET_ERROR_UINT(ERR_UNSPEC, "could not deserialize string data due to memory allocation error");
    }

    memcpy(*dst, *bufptr, rsize);
    *bufptr = *bufptr + rsize;

    return 1;
}


/**
 * @brief   Deserialize an array of fixed-size objects from an input buffer.
 * @note    This is the inverse function of _mem_append_serialized_array().
 * @param   dst     a pointer to the address of an array of objects that will receive a copy of the deserialized
 *                              data as an array of (NULL pointer terminated) fixed-size objects on success.
 * @param   bufptr      a pointer to the address of the input buffer holding the data to be deserialized,
 *                              which will be updated after the operation to point to the next available data.
 * @param   bufend      a pointer to the end of the input buffer for validation purposes.
 * @param   itemsize    the size, in bytes, of the fixed-sized objects to be deserialized.
 * @return  1 on success or 0 on failure.
 */
unsigned int _deserialize_array(unsigned char ***dst, unsigned char **bufptr, const unsigned char *bufend, size_t itemsize) {

    unsigned char *nbufend, *objptr, **chain = NULL;
    size_t dlen;

    if (!dst || !bufptr || !bufend) {
        RET_ERROR_UINT(ERR_BAD_PARAM, NULL);
    }

    // First read in the data length.
    if (!_deserialize_data((unsigned char *)&dlen, bufptr, bufend, sizeof(dlen))) {
        RET_ERROR_UINT(ERR_UNSPEC, "could not read serialized data length");
    }

    // If it's a null size, then the record is empty.
    if (!dlen) {
        *dst = NULL;
        return 1;
    }

    // For the purpose of this function, we need to create a new bufend.
    nbufend = *bufptr + dlen;

    // Make sure it doesn't point too far into the distance.
    if (nbufend > bufend) {
        RET_ERROR_UINT(ERR_UNSPEC, "deserialization error: record stretched out of bounds");
    }

    // As long as we haven't reached the end of the buffer, keep trying to read more objects.
    while (*bufptr < nbufend) {

        if (!(objptr = malloc(itemsize))) {
            PUSH_ERROR_SYSCALL("malloc");

            if (chain) {
                _ptr_chain_free(chain);
            }

            RET_ERROR_UINT(ERR_NOMEM, "unable to allocate space for deserialized object");
        }

        if (!_deserialize_data(objptr, bufptr, nbufend, itemsize)) {
            free(objptr);

            if (chain) {
                _ptr_chain_free(chain);
            }

            RET_ERROR_UINT(ERR_UNSPEC, "data deserialization failed");
        }

        if (!(chain = _ptr_chain_add(chain, objptr))) {
            RET_ERROR_UINT(ERR_UNSPEC, "could not add deserialized data to chain");
        }


    }

    if (*bufptr != nbufend) {
        _ptr_chain_free(chain);
        RET_ERROR_UINT(ERR_UNSPEC, "reached unexpected end of data buffer");
    }

    *dst = chain;

    return 1;
}


/**
 * @brief   Deserialize an array of null-terminated string from an input buffer.
 * @note    This is the inverse function of _mem_append_serialized_str_array().
 * @param   dst     a pointer to the address of an array of strings that will receive a copy of the deserialized
 *                              data as an array of (NULL pointer terminated) null-terminated strings on success.
 * @param   bufptr      a pointer to the address of the input buffer holding the data to be deserialized,
 *                              which will be updated after the operation to point to the next available data.
 * @param   bufend      a pointer to the end of the input buffer for validation purposes.
 * @return  1 on success or 0 on failure.
 */
unsigned int _deserialize_str_array(char ***dst, unsigned char **bufptr, const unsigned char *bufend) {

    unsigned char *nbufend;
    char **chain = NULL, *strptr;
    size_t dlen;

    if (!dst || !bufptr || !bufend) {
        RET_ERROR_UINT(ERR_BAD_PARAM, NULL);
    }

    // First read in the data length.
    if (!_deserialize_data((unsigned char *)&dlen, bufptr, bufend, sizeof(dlen))) {
        RET_ERROR_UINT(ERR_UNSPEC, "could not read serialized data length");
    }

    // If it's a null size, then the record is empty.
    if (!dlen) {
        *dst = NULL;
        return 1;
    }

    // For the purpose of this function, we need to create a new bufend.
    nbufend = *bufptr + dlen;

    // Make sure it doesn't point too far into the distance.
    if (nbufend > bufend) {
        RET_ERROR_UINT(ERR_UNSPEC, "deserialization error: record stretched out of bounds");
    }

    // As long as we haven't reached the end of the buffer, keep trying to read more strings.
    while (*bufptr < nbufend) {

        if (!_deserialize_string(&strptr, bufptr, nbufend)) {

            if (chain) {
                _ptr_chain_free(chain);
            }

            RET_ERROR_UINT(ERR_UNSPEC, "string deserialization failed");
        }

        if (!(chain = _ptr_chain_add(chain, strptr))) {
            RET_ERROR_UINT(ERR_UNSPEC, "could not add deserialized string to chain");
        }


    }

    if (*bufptr != nbufend) {
        _ptr_chain_free(chain);
        RET_ERROR_UINT(ERR_UNSPEC, "reached unexpected end of data buffer");
    }

    *dst = chain;

    return 1;
}


/**
 * @brief   Deserialize an array of custom-formatted data objects from an input buffer using a deserialization function.
 * @note    This is the inverse function of _mem_append_serialized_array_cb().
 * @param   dst     a pointer to the address of an array of data buffers that will receive a copy of the deserialized
 *                              data as an array of (NULL pointer terminated) data buffers on success.
 * @param   bufptr      a pointer to the address of the input buffer holding the data to be deserialized,
 *                              which will be updated after the operation to point to the next available data.
 * @param   bufend      a pointer to the end of the input buffer for validation purposes.
 * @param   dfn     a custom deserialization function that will be used to deserialize the user data from the input buffer.
 * @return  1 on success or 0 on failure.
 */
unsigned int _deserialize_array_cb(unsigned char ***dst, unsigned char **bufptr, const unsigned char *bufend, custom_deserializer_t dfn) {

    unsigned char *nbufend, *dptr, **chain = NULL;
    size_t dlen;

    if (!dst || !bufptr || !bufend || !dfn) {
        RET_ERROR_UINT(ERR_BAD_PARAM, NULL);
    }

    // First read in the data length.
    if (!_deserialize_data((unsigned char *)&dlen, bufptr, bufend, sizeof(dlen))) {
        RET_ERROR_UINT(ERR_UNSPEC, "could not read serialized data length");
    }

    // If it's a null size, then the record is empty.
    if (!dlen) {
        *dst = NULL;
        return 1;
    }

    // For the purpose of this function, we need to create a new bufend.
    nbufend = *bufptr + dlen;

    // Make sure it doesn't point too far into the distance.
    if (nbufend > bufend) {
        RET_ERROR_UINT(ERR_UNSPEC, "deserialization error: record stretched out of bounds");
    }

    // As long as we haven't reached the end of the buffer, keep trying to read more strings.
    while (*bufptr < nbufend) {

        if (!dfn(&dptr, bufptr, nbufend)) {

            if (chain) {
                _ptr_chain_free(chain);
            }

            RET_ERROR_UINT(ERR_UNSPEC, "deserialization handler failed");
        }

        if (!(chain = _ptr_chain_add(chain, dptr))) {
            RET_ERROR_UINT(ERR_UNSPEC, "could not add deserialized data to chain");
        }


    }

    if (*bufptr != nbufend) {
        _ptr_chain_free(chain);
        RET_ERROR_UINT(ERR_UNSPEC, "reached unexpected end of data buffer");
    }

    *dst = chain;

    return 1;
}


/**
 * @brief   Check to see if a cached object needs to be evicted for exceeding its TTL or expiration.
 * @note    An object has "expired" and needs to be evicted from the cache if it has a non-zero TTL from its cache
 *              date that has been reached, or a non-zero absolute expiration date that has been exceeded. If the object's
 *               cache policy is relaxed, a TTL expiration advises a "refresh" if no absolute expiration has occurred.
 * @param   obj     a pointer to the cached object to have its age examined.
 * @param   refresh     an optional pointer to a value that, if the cached object has a relaxed expiration
 *                              policy, will be set to 1 if it needs to be refreshed or 0 if it does not.
 * @return  1 if the object has expired or 0 if it has not; -1 if a general error has occurred.
 */
int _is_object_expired(cached_object_t *obj, int *refresh) {

    time_t now;

    // This should never happen.
    if (!obj || !obj->timestamp) {
        RET_ERROR_INT(ERR_BAD_PARAM, NULL);
    }

    if (refresh) {
        *refresh = 0;
    }

    if (time(&now) == (time_t)-1) {
        PUSH_ERROR_SYSCALL("time");
        RET_ERROR_INT(ERR_UNSPEC, NULL);
    }

    if (obj->expiration && obj->expiration < now) {
        return 1;
    }

    // If our cache eviction is relaxed, that means that a TTL expiration is ignored if and only if
    // an expiration time for the object is set, and that time has not yet been reached.
    if (obj->ttl && (time_t)(obj->timestamp + obj->ttl) <= now) {

        if (!obj->relaxed || !obj->expiration) {
            return 1;
        }

        if (refresh) {
            *refresh = 1;
        }

    }

    return 0;
}


/**
 * @brief   Unlink a cached object from its doubly linked list.
 * @param   object      a pointer to the cached object to be delinked.
 * @param   destroy     if set, deallocate the specified cached object after unlinking.
 * @param   stale       if set, the cache removal was performed because of a stale entry.
 * @return  a pointer to the next cached object in the store, or NULL if at the end of the store.
 */
cached_object_t *_unlink_object(cached_object_t *object, int destroy, int stale) {

    cached_store_t *store;
    cached_object_t *next = NULL;

    // TODO: check the callers of this function and make sure they're operating in a thread-safe fashion.
    if (!object) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (!(store = _get_cached_store_by_type(object->dtype))) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not look up cached object store by type");
    }

    // If we're the only object in the store then reposition the head.
    if (store && (store->head == object)) {
        store->head = object->next;
    }

    if (object->prev) {
        object->prev->next = object->next;
    }

    if (object->next) {
        object->next->prev = object->prev;
        next = object->next;
    }

    if (stale && (_verbose >= 4) && store) {

        if (store->dump) {
            _dbgprint(4, "Evicting stale data from cache: ");
            _dump_cache_data(stderr, object, 0);
        } else {
            _dbgprint(4, "Evicting stale data from cache...\n");
        }

    }

    if (destroy) {

        if (store->destructor) {
            store->destructor(object->data);

            if (get_last_error()) {
                fprintf(stderr, "Cache destructor generated error(s):\n");
                dump_error_stack();
                _clear_error_stack();
            }

        }

        free(object);
    }

    return next;
}


/**
 * @brief   Replace one object in the cache with another.
 * @note    If shadow is not set, then the old cached object will be automatically destroyed.
 *              Otherwise this function also makes the old cached object the new cached object's "shadow" data.
 * @param   oobj    a pointer to the old cached object to be replaced in its cached store with the new object.
 * @param   nobj    a pointer to the new cached object that will replace the old object in the cached store.
 * @param   shadow  if set, preserve the old object as the "shadow" value of the new object.
 * @return  NULL on failure, or a pointer to the new cached object on success.
 */
cached_object_t *_replace_object(cached_object_t *oobj, cached_object_t *nobj, int shadow) {

    cached_store_t *store;

    // TODO: check the callers of this function and make sure they're operating in a thread-safe fashion.
    if (!oobj || !nobj) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if (oobj->dtype != nobj->dtype) {
        RET_ERROR_PTR(ERR_UNSPEC, "cached object replacement cannot be performed on different object types");
    }

    if (!(store = _get_cached_store_by_type(oobj->dtype))) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not look up cached object store by type");
    }

    // If we're the only object in the store then reposition the head.
    if (store && (store->head == oobj)) {
        store->head = nobj;
    }

    nobj->prev = oobj->prev;
    nobj->next = oobj->next;

    if (oobj->prev) {
        oobj->prev->next = nobj;
    }

    if (oobj->next) {
        oobj->next->prev = nobj;
    }

    // Finally, the two must have matching ids.
    memcpy(nobj->id, oobj->id, SHA_256_SIZE);

    oobj->prev = oobj->next = NULL;

    if (shadow) {
        nobj->shadow = oobj;
    } else {
        nobj->shadow = NULL;
        _destroy_cache_entry(oobj);
    }

    return nobj;
}


/**
 * @brief   Lock a cached store for thread-safe processing.
 * @param   store   a pointer to the cached store to be locked.
 */
void _lock_cache_store(cached_store_t *store) {

    if (pthread_mutex_lock(&(store->lock))) {
        perror("pthread_mutex_lock");
    }

}


/**
 * @brief   Unlock a cached storea for use by other callers.
 * @param   store   a pointer to the cached store to be unlocked.
 */
void _unlock_cache_store(cached_store_t *store) {

    if (pthread_mutex_unlock(&(store->lock))) {
        perror("pthread_mutex_unlock");
    }

}


/**
 * @brief  Evict an item from the object cache if it is stale (has expired).
 * @return 1 if the object was evicted for being stale or 0 if it was not.
 */
unsigned int _evict_if_stale(cached_object_t **objptr) {

    int res;

    if (!objptr || !*objptr) {
        return 0;
    }

    if ((res = _is_object_expired(*objptr, NULL)) > 0) {
        *objptr = _unlink_object(*objptr, 1, 1);

        if (get_last_error()) {
            fprintf(stderr, "Error: could not unlink expired item from cache:");
            dump_error_stack();
            _clear_error_stack();
        }

        return 1;
    } else if (res < 0) {
        _clear_error_stack();
    }

    return 0;
}


/* Signet callback functions */

/**
 * @brief   Deserializes signet from a void pointer.
 * @param   data    Pointer to serial signet data.
 * @param   len Length of serial signet data.
 * @return  Void pointer to a signet_t structure.
*/
void *_deserialize_signet_cb(void *data, size_t len) {

    signet_t *result;

    if(!data || !len) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if(!(result = dime_sgnt_signet_binary_deserialize(data, len))) {
        RET_ERROR_PTR(ERR_UNSPEC, NULL);
    }

    return result;
}


/**
 * @brief   Serializes a signet_t structure into a binary string.
 * @param   record  Void pointer to a signet_t structure to be serialized.
 * @param   outlen  Pointer to the length of the returned string.
 * @return  Pointer to a serialized signet.
*/
void *_serialize_signet_cb(void *record, size_t *outlen) {

    unsigned char *serial;
    void *result;
    uint32_t ssize;

    if(!record || !outlen) {
        RET_ERROR_PTR(ERR_BAD_PARAM, NULL);
    }

    if(!(serial = dime_sgnt_signet_binary_serialize(record, &ssize))) {
        RET_ERROR_PTR(ERR_UNSPEC, "could not serialize signet");
    }

    if(!(result = malloc(ssize))) {
        PUSH_ERROR_SYSCALL("malloc");
        free(serial);
        RET_ERROR_PTR(ERR_NOMEM, NULL);
    }

    *outlen = ssize;
    memcpy(result, serial, ssize);
    free(serial);

    return result;
}


/**
 * @brief   Dumps a signet from a signet_t structure.
 * @param   fp  File descriptor that specifies the output destination.
 * @param   record  Void pointer to a signet_t structure.
 * @param   brief   TODO
*/
void _dump_signet_cb(FILE *fp, void *record, int brief) {

    signet_t *sig = (signet_t *)record;

    if (!fp || !sig) {
        return;
    }

    if (brief) {
        fprintf(fp, "*** hashed ***");
        return;
    }

    dime_sgnt_signet_dump(fp, sig);

}


/**
 * @brief   Destroys a signet_t structure.
 * @param   record  Void pointer to a signet_t structure to be destroyed.
*/
void _destroy_signet_cb(void *record) {

    dime_sgnt_signet_destroy(record);

}
