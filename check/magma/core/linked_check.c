
/**
 * @file /check/magma/core/linked_check.c
 *
 * @brief Unit tests for linked lists.
 */

#include "magma_check.h"


bool_t check_indexes_linked_cursor_compare(uint64_t values[], inx_cursor_t *cursor) {

	void *val;
	multi_t key;
	uint64_t count = 0;
	bool_t found[] =	{	[0 ... LINKED_CURSORS_CHECK] = false };

	while (status() && (val = inx_cursor_value_next(cursor))) {

		key = inx_cursor_key_active(cursor);
		if (values[key.val.u64] != *((uint64_t *)val)) {
			return false;
		}

		found[key.val.u64] = true;
		count++;
	}

	if (count != LINKED_CURSORS_CHECK) {
		return false;
	}

	for (uint_t i = 0; i < LINKED_CURSORS_CHECK; i++) {
		if (found[i] != true) {
			return false;
		}

		found[i] = false;
	}

	count = 0;
	inx_cursor_reset(cursor);

	while (!mt_is_empty(key = inx_cursor_key_next(cursor))) {

		val = inx_cursor_value_active(cursor);
		if (values[key.val.u64] != *((uint64_t *)val)) {
			return false;
		}

		found[key.val.u64] = true;
		count++;
	}

	if (count != LINKED_CURSORS_CHECK) {
		return false;
	}

	for (uint_t i = 0; i < LINKED_CURSORS_CHECK; i++) {
		if (found[i] != true) {
			return false;
		}
			found[i] = false;
	}

	return true;
}

bool_t check_indexes_linked_cursor(char **errmsg) {

	void *val;
	inx_t *inx;
	multi_t key;
	inx_cursor_t *cursor;
	uint64_t values[LINKED_CURSORS_CHECK];

	for (uint64_t i = 0; status() && i < LINKED_CURSORS_CHECK; i++) {
		values[i] = rand_get_uint64();
	}

	// Insert random numbers into values array.
	if (!(inx = inx_alloc(M_INX_LINKED, mm_free))) {
		*errmsg = "index allocation failed";
		return false;
	}

	for (uint64_t i = 0; status() && i < LINKED_CURSORS_CHECK; i++) {

		if (!(val = mm_alloc(sizeof(uint64_t)))) {
			*errmsg = "value buffer allocation failed";
			inx_free(inx);
			return false;
		}

		mm_copy(val, &values[i], sizeof(uint64_t));
		mm_wipe(&key, sizeof(multi_t));
		key.type = M_TYPE_UINT64;
		key.val.u64 = i;

		if (!inx_insert(inx, key, val)) {
			*errmsg = "insert operation failed";
			inx_free(inx);
			mm_free(val);
			return false;
		}
	}

	if (!(cursor = inx_cursor_alloc(inx))) {
		*errmsg = "cursor allocation failed";
		inx_free(inx);
		return false;
	}

	if (!check_indexes_linked_cursor_compare(values, cursor)) {
		*errmsg = "cursor validation failed";
		inx_cursor_free(cursor);
		inx_free(inx);
		return false;
	}

	inx_cursor_free(cursor);
	inx_free(inx);

	return true;
}

bool_t check_indexes_linked_simple(char **errmsg) {

	void *val;
	inx_t *inx;
	multi_t key;
	uint64_t rnum;
	char snum[1024];

	if (!(inx = inx_alloc(M_INX_LINKED, mm_free))) {
		*errmsg = "index allocation failed";
		return false;
	}

	// Insert 128 random numbers into a linked list index.
	for (uint64_t i = 0; status() && i < LINKED_INSERTS_CHECK; i++) {
		rnum = rand_get_uint64();
		snprintf(snum, 1024, "%lu", rnum);

		if (!(val = mm_alloc(sizeof(uint64_t)))) {
			*errmsg = "value buffer allocation failed";
			inx_free(inx);
			return false;
		}

		mm_wipe(&key, sizeof(multi_t));
		key.val.ns = &snum[0];
		key.type = M_TYPE_NULLER;
		mm_copy(val, &rnum, sizeof(uint64_t));

		if (!inx_insert(inx, key, val)) {
			*errmsg = "insert operation failed";
			inx_free(inx);
			mm_free(val);
			return false;
		}
	}

	inx_free(inx);

	return true;
}


