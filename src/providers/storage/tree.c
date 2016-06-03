
/**
 * @file /magma/providers/storage/tree.c
 *
 * @brief Use Tokyo Cabinet to provide a tree based index implementation for the generic index interface.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

typedef struct {
	inx_t *inx;
	void *tree;
	multi_t key;
	void *value;
}__attribute__ ((__packed__)) tree_cursor_t;

/**
 * The return value is positive if the former is big, negative if the latter is big, 0 if both are equivalent. If asked to search for a NULL key pointer
 * the function will always return 0. This allows us to match every record.
 *
 * @param	aptr	a pointer to the region of the first key.
 * @param	asize	the length, in bytes, of the region of the first key.
 * @param	bptr	a pointer to the region of the second key.
 * @param	bsiz	the length, in bytes, of the region of the first key.
 * @param	op		an optional pointer to an opaque object.
 * @return	0 if the two blocks are identical; otherwise, a signed integer indicating the difference.
 */
int tree_cmp(const char *aptr, int asiz, const char *bptr, int bsiz, void *op) {
	return cmp_mt_mt(*(multi_t *)aptr, *(multi_t *)bptr);
}

/**
 * @brief	Get the number of records in an in-memory tree.
 * @see		tcndbrnum()
 * @param	inx
 * @return	the record count.
 *
 *
 *
 */
uint64_t tree_count(void *inx) {
	inx_t *index = inx;
	return tcndbrnum_d(index->index);
}

void * tree_find(void *inx, multi_t key) {
	int len;
	void **value, *result = NULL;
	if ((value = tcndbget3_d(((inx_t *)inx)->index, &key, sizeof(multi_t), &len))) {
		result = *value;
		tcfree_d(value);
	}
	return result;
}

bool_t tree_delete(void *inx, multi_t key) {

	int ksp, vsp;
	multi_t dupe;
	void *vp, *kp;
	inx_t *index = inx;
	bool_t result = false;

	if (index == NULL || index->index == NULL || index->count == 0) {
		return false;
	}

	if (tcndbgetboth_d(index->index, &key, sizeof(multi_t), &kp, &ksp, &vp, &vsp)) {
		if (index->data_free && vp) index->data_free(*(void **)vp);
		if (kp) mm_copy(&dupe, kp, sizeof(multi_t));
		tcndbout_d(index->index, &key, sizeof(multi_t));
		if (kp) mt_free(dupe);
		index->count--;
		index->serial++;
		result = true;
	}

	return result;
}

/**
 * Makes a copy of the key and then attempts to add the new entry to the tree index. Note that because this is a tree index, duplicates are not
 * allowed, so if the key already exists, false is returned.
 *
 * @param inx The index were adding the entry too.
 * @param key The retrieval key expressed as a multi_t.
 * @param data The data buffer being stored.
 * @return Returns true if the entry was added, or false to indicate an existing duplicate key or an error.
 */
bool_t tree_insert(void *inx, multi_t key, void *data) {

	multi_t copy;
	inx_t *index = inx;

	if (mt_is_empty(copy = mt_dupe(key))) {
	log_info("Unable to make a copy of the key.");
		return false;
	}

	// This will fail if the tree index already contains the provided key.
	if (!tcndbputkeep_d(index->index, &copy, sizeof(multi_t), &data, sizeof(void **))) {
		log_info("Unable to store a new index record, possibly because it is a duplicate.");
		mt_free(copy);
		return false;
	}

	index->count++;
	index->serial++;
	return true;
}

bool_t tree_cursor_next(tree_cursor_t *cursor) {

	int len;
	multi_t *key;
	void **value;

	cursor->value = NULL;
	cursor->key = mt_get_null();

	// Request the next entry from the duplicate tree, then search for the value in the master tree. Skip any keys that are no longer found in the master tree.
	while (!cursor->value && (key = (multi_t *)tcndbiternext2_d(cursor->tree))) {

		if (cursor->inx && cursor->inx->index && (value = tcndbget3_d(cursor->inx->index, key, sizeof(multi_t), &len))) {
			mm_copy(&(cursor->key), key, sizeof(multi_t));
			cursor->value = *value;
			tcfree_d(value);
		}

		tcfree_d(key);
	}

	return true;
}

void * tree_cursor_value_next(tree_cursor_t *cursor) {
	if (tree_cursor_next(cursor)) {
		return cursor->value;
	}
	return NULL;
}

void * tree_cursor_value_active(tree_cursor_t *cursor) {
	return cursor->value;
}

multi_t tree_cursor_key_next(tree_cursor_t *cursor) {
	if (tree_cursor_next(cursor)) {
		return cursor->key;
	}
	return mt_get_null();
}

multi_t tree_cursor_key_active(tree_cursor_t *cursor) {
	return cursor->key;
}

void tree_cursor_reset(tree_cursor_t *cursor) {

	if (cursor && cursor->tree) {
		tcndbdel_d(cursor->tree);
		cursor->tree = NULL;
	}

	if (cursor && cursor->inx && cursor->inx->index && (cursor->tree = tcndbdup_d((TCNDB *)(cursor->inx->index)))) {
		tcndbiterinit_d(cursor->tree);
	}

	return;
}

void tree_cursor_free(tree_cursor_t *cursor) {

	if (cursor && cursor->tree) {
		tcndbdel_d(cursor->tree);
	}

	if (cursor) {
		mm_free(cursor);
	}

	return;
}

void * tree_cursor_alloc(inx_t *inx) {

	tree_cursor_t *cursor;

	if (!inx || !inx->index) {
		return NULL;
	}
	else if (!(cursor = mm_alloc(sizeof(tree_cursor_t)))) {
		log_pedantic("Failed to allocate %zu bytes for a tree index cursor.", sizeof(tree_cursor_t));
		return NULL;
	}

	cursor->inx = inx;

	if ((cursor->tree = tcndbdup_d((TCNDB *)(cursor->inx->index)))) {
		tcndbiterinit_d(cursor->tree);
	}

	return cursor;
}

/**
 * Truncates a tree based index.
 *
 * @param inx A pointer to the index that should be truncated.
 */
void tree_truncate(void *inx) {

	void *val;
	TCLIST *list;
	multi_t *key;
	int count, length;
	inx_t *index = inx;

	if (index == NULL || index->index == NULL) {
		return;
	}

	// Should return an array list containing every key, which we then get a count of.
	if (index->data_free) {

		list = tctreevals_d(((TCNDB *)index->index)->tree);
		count = tclistnum_d(list);

		for (int i = 0; i < count; i++) {
			if ((val = (void *)tclistval_d(list, i, &length))) {
				index->data_free(*(void **)val);
			}
		}

		tclistdel_d(list);
	}

	// Now free the keys.
	list = tctreekeys_d(((TCNDB *)index->index)->tree);
	count = tclistnum_d(list);

	for (int i = 0; i < count; i++) {
		key = (multi_t *)tclistval_d(list, i, &length);
		mt_free(*key);
	}

	tclistdel_d(list);
	return;
}

/**
 * Frees all of the resources used by a tree based index.
 *
 * @param inx A pointer to the index that should be freed.
 */
void tree_free(void *inx) {

	inx_t *index = inx;

	if (index == NULL || index->index == NULL) {
		return;
	}

	// Truncate the index.
	tree_truncate(inx);

	// A complete free also means we should destroy the tree context.
	tcndbdel_d(index->index);
	index->index = NULL;

	return;
}

/**
 * @brief	Create a new on-memory tree database.
 * @see		tcndbnew2()
 * @see
 *
 * @param options
 * @param data_free
 * @return
 */
inx_t * tree_alloc(uint64_t options, void *data_free) {

	inx_t *result;
	TCCMP cmp = tree_cmp;

	if (!(result = mm_alloc(sizeof(inx_t)))) {
		log_pedantic("Unable to allocate a new inx structure. {sizeof(inx_t) = %zu}", sizeof(inx_t));
		return NULL;
	} else if (!(result->index = tcndbnew2_d(cmp, NULL))) {
		log_pedantic("Unable to create a new tree index structure. {tcndbnew2 = NULL}");
		mm_free(result);
		return NULL;
	}

	result->options = options;
	result->data_free = data_free;
	result->index_free = tree_free;
	result->index_truncate = tree_truncate;

	result->find = tree_find;
	result->insert = tree_insert;
	result->delete = tree_delete;

	result->cursor_free = (void (*)(void *))&tree_cursor_free;
	result->cursor_reset = (void (*)(void *))&tree_cursor_reset;
	result->cursor_alloc = (void * (*)(void *))&tree_cursor_alloc;

	result->cursor_key_next = (multi_t (*)(void *))&tree_cursor_key_next;
	result->cursor_key_active = (multi_t (*)(void *))&tree_cursor_key_active;

	result->cursor_value_next = (void * (*)(void *))&tree_cursor_value_next;
	result->cursor_value_active = (void * (*)(void *))&tree_cursor_value_active;

	return result;
}
