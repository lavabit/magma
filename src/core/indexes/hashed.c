
/**
 * @file /magma/core/indexes/hashed.c
 *
 * @brief	Function declarations and types for the hashed list.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

#define MAGMA_HASHED_BUCKETS 1024

// Hashed lists.
typedef struct {
	void *data;
	multi_t key;
	struct hashed_bucket_t *next;
} hashed_bucket_t;

typedef struct {
	inx_t *inx;
	hashed_bucket_t *bucket;
	uint64_t serial, slot, count;
} __attribute__((__packed__)) hashed_cursor_t;

typedef struct {
	uint32_t buckets;
} __attribute__((__packed__)) hashed_index_t;

// TODO: Finish implementing the hashed iterators.
// TODO: Respect dupe/lock flags.
// TODO: Move locking calls to the inx functions.

/**
 * @brief	Get the hash bucket for a key.
 * @note	If the key is passed as a string, it will be mapped to a bucket using the Fletcher32 hash.
 * @param	buckets		the total number of buckets for grouping items.
 * @param	key			a multi-type key with the value to be looked up; numbers and strings are supported.
 * @return	the number of the hash bucket corresponding to the specified key.
 */
uint32_t hashed_bucket(uint32_t buckets, multi_t key) {

	uint32_t result = 0, count;

	if (mt_is_number(key)) {
		if ((result = mt_get_number(key)) < buckets) {

			// If my math is right, the bit count should never exceed the value of result. That also means the expression should never return a negative number.
			log_check((count = bits_count(result)) > result);

			result = (((count = bits_count(result)) % 2) ? result + count : result - count);
			result = (buckets <= result ? (buckets - 1) : result);
		}
		else {
			result = (result % buckets);
		}
	}
	else {
		result = hash_fletcher32(mt_get_char(key), mt_get_length(key)) % buckets;
	}

	return result;
}

/**
 * @brief	Allocate and initialize hashed bucket object.
 * @param	key		a multi-type key that will be hashed on lookup.
 * @param	data	a pointer to a data buffer associated with the key.
 * @return	NULL on failure, or a pointer to the newly allocated hashed bucket object on success.
 */
hashed_bucket_t * hashed_bucket_alloc(multi_t key, void *data) {

	hashed_bucket_t *bucket;

	if ((bucket = mm_alloc(sizeof(hashed_bucket_t))) == NULL) {
		//lavalog(LOG_DEBUG, LOG_FRAMEWORK, "Unable to allocate %i bytes for a hash bucket.", sizeof(hashed_bucket_t));
		return NULL;
	}

	bucket->key = mt_dupe(key);
	bucket->data = data;
	return bucket;
}

// Gets the bucket from the list.
hashed_bucket_t * hashed_bucket_get_ptr(hashed_index_t *hashed, uint32_t num) {

	hashed_bucket_t *result = NULL;

	if (num < hashed->buckets) {
		result = *(hashed_bucket_t **)((chr_t *)hashed + sizeof(hashed_index_t) + (sizeof(hashed_bucket_t *) * num));
	}

#ifdef MAGMA_PEDANTIC
	else {
		log_pedantic("Invalid hash bucket requested.");
	}
#endif

	return result;
}

// Sets the bucket on the list.
void hashed_bucket_set_ptr(hashed_index_t *hashed, uint32_t num, hashed_bucket_t *bucket) {

	if (num < hashed->buckets) {
		*(hashed_bucket_t **)((chr_t *)hashed + sizeof(hashed_index_t) + (sizeof(hashed_bucket_t *) * num)) = bucket;
	}

#ifdef MAGMA_PEDANTIC
	else {
		log_pedantic("Invalid hash bucket requested.");
	}
#endif

	return;
}

// Tacks on a bucket at the end.
void hashed_bucket_add(hashed_bucket_t *bucket, hashed_bucket_t *new) {

	if (bucket == NULL) {
		//lavalog(LOG_DEBUG, LOG_FRAMEWORK, "Passed a NULL pointer.");
		return;
	}

	// Find the end.
	while (bucket->next != NULL) {
		bucket = (hashed_bucket_t *)bucket->next;
	}

	bucket->next = (struct hashed_bucket_t *)new;
	return;
}

// Add a data item to the list.
bool_t hashed_insert(void *inx, multi_t key, void *data) {

	uint32_t num;
	inx_t *index = inx;
	hashed_index_t *hashed;
	hashed_bucket_t *bucket, *holder;

	if (index == NULL || index->index == NULL) {
		return false;
	}

	hashed = index->index;

	// Create the new bucket.
	if ((holder = hashed_bucket_alloc(key, data)) == NULL) {
		return false;
	}

	// Figure out the bucket number.
	num = hashed_bucket(hashed->buckets, key);

	// Get the bucket.
	bucket = hashed_bucket_get_ptr(hashed, num);

	// We need to create a bucket.
	if (bucket == NULL) {
		hashed_bucket_set_ptr(hashed, num, holder);
	} else {
		hashed_bucket_add(bucket, holder);
	}

	index->count++;
	index->serial++;
	return true;
}

// Searches a bucket list for the key.
void * hashed_bucket_find_key(hashed_bucket_t *bucket, multi_t key) {

	void *data = NULL;

	while (bucket != NULL && data == NULL) {
		if (ident_mt_mt(bucket->key, key) == true) {
			data = bucket->data;
		}
		bucket = (hashed_bucket_t *)bucket->next;
	}

	return data;
}

// Gets a data item, or returns NULL.
void * hashed_find(void *inx, multi_t key) {

	uint32_t num;
	void *data = NULL;
	inx_t *index = inx;
	hashed_index_t *hashed;
	hashed_bucket_t *bucket;

	if (index == NULL || index->index == NULL) {
		return NULL;
	}

	hashed = index->index;

	// Figure out the bucket number.
	num = hashed_bucket(hashed->buckets, key);

	// Get the bucket.
	bucket = hashed_bucket_get_ptr(hashed, num);

	// We need to search the bucket for the key.
	if (bucket != NULL) {
		data = hashed_bucket_find_key(bucket, key);
	}

	return data;
}

bool_t hashed_delete(void *inx, multi_t key) {

	uint32_t num;
	inx_t *index = inx;
	bool_t found = false;
	hashed_index_t *hashed;
	hashed_bucket_t *bucket, *holder = NULL;

	if (index == NULL || index->index == NULL) {
		return false;
	}

	hashed = index->index;

	// Figure out the bucket number.
	num = hashed_bucket(hashed->buckets, key);

	// Get the bucket.
	bucket = hashed_bucket_get_ptr(hashed, num);

	// There is only one bucket.
	while (bucket && !found) {

		// This is the bucket to free.
		if ((found = ident_mt_mt(bucket->key, key)) == true) {

			// Its the first bucket.
			if (holder == NULL) {
				hashed_bucket_set_ptr(hashed, num, (hashed_bucket_t *)bucket->next);
			}
			// Takes this bucket out of the list.
			else {
				holder->next = (struct hashed_bucket_t *)bucket->next;
			}

			if (bucket->data && index->data_free) {
				index->data_free(bucket->data);
			}

			mt_free(bucket->key);
			mm_free(bucket);

		} else {
			holder = bucket;
			bucket = (hashed_bucket_t *)bucket->next;
		}
	}

	if (found) {
		index->count--;
		index->serial++;
	}

	return found;
}

/// BUG: It's a little unclear what's happening, but when this function gets called, the serials don't match, and the count is greater than the number of available
/// data buckets, because a bucket has been removed, the cursor will get stuck returning the last item endlessly (I think).
hashed_bucket_t * hashed_cursor_active(hashed_cursor_t *cursor) {

	uint64_t count = 0;
	hashed_bucket_t *bucket = NULL, *loop;

	// If the serial numbers match, return the bucket pointer.
	if (cursor->serial == cursor->inx->serial) {
		bucket = (hashed_bucket_t *)cursor->bucket;
	}

	// Otherwise pull the bucket pointer and count off the items till we have a match.
	if (!bucket && cursor->slot < ((hashed_index_t *)cursor->inx->index)->buckets) {

		if ((loop = hashed_bucket_get_ptr(cursor->inx->index, cursor->slot))) {

			count++;

			// Advance to the active node.
			while (loop && count != cursor->count) {
				loop = (hashed_bucket_t *)loop->next;
				count++;
			}

			if (loop && count == cursor->count) {
				cursor->bucket = bucket = loop;
				cursor->count = 1;
			}
		}
	}

	// The bucket above didn't contain enough items to satisfy the counter, so all we need to do is look for the next slot with a valid entry.
	if (!bucket && cursor->slot < ((hashed_index_t *)cursor->inx->index)->buckets) {
		for (count = cursor->slot; !bucket && count < ((hashed_index_t *)cursor->inx->index)->buckets; count++) {
			bucket = hashed_bucket_get_ptr(cursor->inx->index, count);
		}

		if (bucket) {
			cursor->count = 1;
			cursor->slot = count - 1;
			cursor->bucket = bucket;
			cursor->serial = cursor->inx->serial;
		}

	}

	return bucket;
}

hashed_bucket_t * hashed_cursor_next(hashed_cursor_t *cursor) {

	hashed_bucket_t *bucket = NULL;

	// If the serials match up, see if the bucket pointer will lead us to the next entry.
	if (cursor->serial == cursor->inx->serial && cursor->bucket && (bucket = (hashed_bucket_t *)(cursor->bucket->next))) {
		cursor->count++;
		cursor->bucket = bucket;
	}
	// Otherwise if the serials match, advance the slot number.
	else if (!bucket && cursor->serial == cursor->inx->serial) {
		cursor->slot++;
		cursor->count = 0;
		cursor->bucket = NULL;
		bucket = hashed_cursor_active(cursor);
	}
	// The serials don't match so we'll be counting out the items in the current bucket. So we just need to increase our counter.
	else if (cursor->slot < ((hashed_index_t *)cursor->inx->index)->buckets){
		bucket = hashed_cursor_active(cursor);
	}

	return bucket;
}

void * hashed_cursor_value_next(hashed_cursor_t *cursor) {

	hashed_bucket_t *bucket;

	if ((bucket = hashed_cursor_next(cursor))) {
		return bucket->data;
	}
	return NULL;

}

void * hashed_cursor_value_active(hashed_cursor_t *cursor) {

	hashed_bucket_t *bucket;

	if ((bucket = hashed_cursor_active(cursor))) {
		return bucket->data;
	}
	return NULL;
}

multi_t hashed_cursor_key_next(hashed_cursor_t *cursor) {

	hashed_bucket_t *bucket;

	if ((bucket = hashed_cursor_next(cursor))) {
		return bucket->key;
	}
	return mt_get_null();
}

multi_t hashed_cursor_key_active(hashed_cursor_t *cursor) {

	hashed_bucket_t *bucket;

	if ((bucket = hashed_cursor_active(cursor))) {
		return bucket->key;
	}
	return mt_get_null();
}

void hashed_cursor_reset(hashed_cursor_t *cursor) {

	if (cursor) {
		cursor->bucket = NULL;
		cursor->serial = cursor->count = cursor->slot = 0;
	}

	return;
}

void hashed_cursor_free(hashed_cursor_t *cursor) {

	if (cursor) {
		mm_free(cursor);
	}

	return;
}

void * hashed_cursor_alloc(inx_t *inx) {

	hashed_cursor_t *cursor;

	if (!(cursor = mm_alloc(sizeof(hashed_cursor_t)))) {
		log_pedantic("Failed to allocate %zu bytes for a hash index cursor.", sizeof(hashed_cursor_t));
		return NULL;
	}

	cursor->inx = inx;

	return cursor;
}

void hashed_free(void *inx) {

	uint32_t num;
	inx_t *index = inx;
	hashed_index_t *hashed;
	hashed_bucket_t *bucket, *holder;

	if (index == NULL || index->index == NULL) {
		return;
	}

	hashed = index->index;

	for (num = 0; num < hashed->buckets; num++) {

		bucket = hashed_bucket_get_ptr(hashed, num);
		while (bucket) {
			holder = bucket;
			bucket = (hashed_bucket_t *)holder->next;
			if (holder->data && index->data_free) {
				index->data_free(holder->data);
			}
			mt_free(holder->key);
			mm_free(holder);
		}

	}

	mm_free(index->index);
	index->index = NULL;
	return;
}

void hashed_truncate(void *inx) {

	uint32_t num;
	inx_t *index = inx;
	hashed_index_t *hashed;
	hashed_bucket_t *bucket, *holder;

	if (index == NULL || index->index == NULL) {
		return;
	}

	hashed = index->index;

	for (num = 0; num < hashed->buckets; num++) {
		if ((bucket = hashed_bucket_get_ptr(hashed, num))) {
			while (bucket) {
				holder = bucket;
				bucket = (hashed_bucket_t *)holder->next;
				if (holder->data && index->data_free) {
					index->data_free(holder->data);
				}
				mt_free(holder->key);
				mm_free(holder);
			}
			hashed_bucket_set_ptr(hashed, num, NULL);
		}
	}

	return;
}

/**
 * @brief	Allocate a new hash table.
 * @param	options		an options value for the hash table.
 * @param	data_free	a pointer to a function
 *
 *
 *
 * @return	NULL on failure, or a pointer to the newly allocated hash table object on success.
 */
inx_t * hashed_alloc(uint64_t options, void *data_free) {

	inx_t *result;

	if ((result = mm_alloc(sizeof(inx_t))) == NULL) {
		return NULL;
	} else if (!(result->index = mm_alloc(sizeof(hashed_index_t) + (sizeof(hashed_bucket_t *) * MAGMA_HASHED_BUCKETS)))) {
		mm_free(result);
		return NULL;
	}

	result->options = options;
	result->data_free = data_free;
	result->index_free = hashed_free;
	result->index_truncate = hashed_truncate;
	((hashed_index_t *)(result->index))->buckets = MAGMA_HASHED_BUCKETS;

	result->find = hashed_find;
	result->insert = hashed_insert;
	result->delete = hashed_delete;

	result->cursor_free = (void (*)(void *))&hashed_cursor_free;
	result->cursor_reset = (void (*)(void *))&hashed_cursor_reset;
	result->cursor_alloc = (void * (*)(void *))&hashed_cursor_alloc;

	result->cursor_key_next = (multi_t (*)(void *))&hashed_cursor_key_next;
	result->cursor_key_active = (multi_t (*)(void *))&hashed_cursor_key_active;

	result->cursor_value_next = (void * (*)(void *))&hashed_cursor_value_next;
	result->cursor_value_active = (void * (*)(void *))&hashed_cursor_value_active;

	return result;
}
