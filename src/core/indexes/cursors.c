
/**
 * @file /magma/core/indexes/cursors.c
 *
 * @brief	The generic index interface for handling cursors.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Reset the position of an inx cursor.
 * @param	cursor	a pointer to the inx cursor to be reset.
 * @return	This function returns no value.
 */
void inx_cursor_reset(inx_cursor_t *cursor) {

	if (cursor && cursor->inx && cursor->inx->cursor_reset) {
		cursor->inx->cursor_reset(cursor);
	}

	return;
}

/**
 * @brief	Free an inx cursor and the inx object it points to, if the inx reference count hits zero.
 * @param	cursor	the inx cursor to be freed.
 * @return	This function returns no value.
 */
void inx_cursor_free(inx_cursor_t *cursor) {

	inx_t *index;

	if (cursor && cursor->inx && cursor->inx->cursor_free) {
		index = cursor->inx;

		inx_auto_write(index);
		cursor->inx->cursor_free(cursor);
		inx_auto_unlock(index);

		// This will decrement the reference counter and if the counter hits zero release the index.
		inx_free(index);
	}

	return;
}

/**
 * @brief	Create a new cursor to iterate through an inx object.
 * @param	index	a pointer to the inx object to be traversed.
 * @return	NULL on failure, or a pointer to a new inx cursor for the specified inx object on success.
 */
inx_cursor_t * inx_cursor_alloc(inx_t *index) {

	inx_cursor_t *cursor = NULL;

	if (index && index->cursor_alloc) {
		inx_auto_write(index);

		if ((cursor = index->cursor_alloc(index))) {
			index->references++;
		}

		inx_auto_unlock(index);
	}

	return cursor;
}

/**
 * @brief	Get the key at the next inx cursor position.
 * @param	cursor	the inx cursor to be examined.
 * @return	NULL on failure, or a multi-type data object containing the key of the next inx cursor position.
 */
multi_t inx_cursor_key_next(inx_cursor_t *cursor) {

	multi_t key = mt_get_null();

	if (cursor && cursor->inx && cursor->inx->cursor_key_next) {
		inx_auto_read(cursor->inx);
		key =  cursor->inx->cursor_key_next(cursor);
		inx_auto_unlock(cursor->inx);
	}

	return key;
}

/**
 * @brief	Get the key at the current inx cursor position.
 * @param	cursor	the inx cursor to be examined.
 * @return	NULL on failure, or a multi-type data object containing the key of the specified inx cursor.
 */
multi_t inx_cursor_key_active(inx_cursor_t *cursor) {

	multi_t key = mt_get_null();

	if (cursor && cursor->inx && cursor->inx->cursor_key_active) {
		inx_auto_read(cursor->inx);
		key =  cursor->inx->cursor_key_active(cursor);
		inx_auto_unlock(cursor->inx);
	}

	return key;
}

/**
 * @brief	Get the key at the next inx cursor position.
 * @param	cursor	the inx cursor to be examined.
 * @return	NULL on failure, or a multi-type data object containing the key of the next inx cursor position.
 */
void * inx_cursor_value_next(inx_cursor_t *cursor) {

	void *value = NULL;

	if (cursor && cursor->inx && cursor->inx->cursor_value_next) {
		inx_auto_read(cursor->inx);
		value = cursor->inx->cursor_value_next(cursor);
		inx_auto_unlock(cursor->inx);
	}

	return value;
}

/**
 * @brief	Get the value at the current inx cursor position.
 * @param	cursor	the inx cursor to be examined.
 * @return	NULL on failure, or the value at the current position of the specified inx cursor.
 */
void * inx_cursor_value_active(inx_cursor_t *cursor) {

	void *value = NULL;

	if (cursor && cursor->inx && cursor->inx->cursor_value_active) {
		inx_auto_read(cursor->inx);
		value =  cursor->inx->cursor_value_active(cursor);
		inx_auto_unlock(cursor->inx);
	}

	return value;
}
