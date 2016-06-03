
/**
 * @file /magma/core/indexes/linked.c
 *
 * @brief	The linked list implementation functions utilized by the generic index interface.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

// Linked lists.
typedef struct {
	multi_t key;
	void **data;
	uint64_t size, count;
} linked_record_t;

typedef struct {
	linked_record_t *record;
	struct linked_node_t *next, *prev;
} linked_node_t;

typedef struct {
	inx_t *inx;
	linked_node_t *node;
	uint64_t serial, count, position;
}__attribute__ ((__packed__)) linked_cursor_t;

/**
 * @brief	Get the data associated with a linked list record.
 * @param	record		a pointer to the linked list record to be queried.
 * @param	element		the index of the data element to be retrieved. Must be set to zero.
 * @return	NULL on failure, or a pointer to the specified linked list record's data on success.
 */
void * linked_record_get_data(linked_record_t *record, size_t element) {

	#ifdef MAGMA_PEDANTIC
	if (!record) log_pedantic("The index pointer is invalid.");
	#endif

	if (element != 0)	return NULL;
	return record->data;
}

/**
 * @brief	Get the multi-type key of a linked list record.
 * @param	record	a pointer to the linked list record to be queried.
 * @return	an empty multi-type key on failure, or the specified record's multi-type key value on success.
 */
multi_t linked_record_get_key(linked_record_t *record) {

	#ifdef MAGMA_PEDANTIC
	if (!record) log_pedantic("The index pointer is invalid.");
	#endif

	if (!record)
		return mt_get_null();

	return record->key;
}

/**
 * @brief	Free a linked list record object and its underlying data.
 * @param	index	a pointer to the linked list containing the specified record.
 * @param	record	a pointer to the linked list record to be freed.
 * @return	This function returns no value.
 */
void linked_record_free(inx_t *index, linked_record_t *record) {

	#ifdef MAGMA_PEDANTIC
	if (!index) log_pedantic("The index pointer is invalid.");
	if (!record) log_pedantic("The record pointer is invalid.");
	//if (!(index->data_free))	log_pedantic("The function pointer is invalid.");
	#endif

	if (!record) return;
	if (index && index->data_free) index->data_free(record->data);
	mt_free(record->key);
	mm_free(record);
	return;
}

/**
 * @brief	Create a new linked list record object.
 * @param	key		the multi-type key value of the record to be used for data searches.
 * @param	data	a pointer to the data to be associated with the record.
 * @return	a pointer to the newly allocated and initialized linked record object.
 */
linked_record_t * linked_record_alloc(multi_t key, void *data) {

	linked_record_t *record;

	if ((record = mm_alloc(sizeof(linked_record_t))) == NULL) return NULL;

	record->key = mt_dupe(key);
	record->data = data;
	record->count = 1;

	return record;
}

/**
 * @brief	Find a record in a linked list by key.
 * @param	inx		a pointer to the linked list to be searched.
 * @param	key		a multi-type key value to be searched against the contents of the inx object.
 * @return	NULL on failure or if the record cannot be found, or a pointer to the data of the matching record on success.
 */
void * linked_find(void *inx, multi_t key) {

	inx_t *index = inx;
	linked_node_t *node;

	if (index == NULL || index->index == NULL || index->count == 0) {
		return NULL;
	}

	node = index->index;

	while (node != NULL && node->record != NULL && ident_mt_mt(node->record->key, key) != true) {
		node = (linked_node_t *)node->next;
	}

	// We didn't find the correct node, or the index is corrupted.
	if (node == NULL || node->record == NULL) {
		return NULL;
	}

	return linked_record_get_data(node->record, 0);
}

/**
 * @brief	Remove a record from a linked list and free it and its underlying data.
 * @param	inx		a pointer to the linked list to be searched for the specified key.
 * @param	key		a multi-type key value to lookup the record that will be deleted from the linked list.
 * @return	true on success or false on failure.
 */
// Find a record and remove it from the linked list.
bool_t linked_delete(void *inx, multi_t key) {

	inx_t *index = inx;
	linked_node_t *node;

	if (index == NULL || index->index == NULL || index->count == 0) {
		return false;
	}

	node = index->index;

	while (node != NULL && node->record != NULL && ident_mt_mt(node->record->key, key) != true) {
		node = (linked_node_t *)node->next;
	}

	// We didn't find the correct node, or the index is corrupted.
	if (node == NULL || node->record == NULL) {
		return false;
	}

	// Handle the special case where this is the first node.
	if (index->index == node) {
		index->index = node->next;
	}
	else if (node && node->prev) {
		((linked_node_t *)node->prev)->next = node->next;
	}

	if (node && node->next) {
		((linked_node_t *)node->next)->prev = node->prev;
	}

	linked_record_free(index, node->record);
	mm_free(node);
	index->count--;
	index->serial++;
	return true;
}

/**
 * @brief	Create and append a new record to the end of a linked list.
 * @param	inx		a pointer to the linked list that will store the new record.
 * @param	key		a multi-type key value that will be associated with the newly created record.
 * @param	data	a pointer to the data that will be associated with the new record.
 * @return	true on success or false on failure.
 */
// Append something to the linked list.
bool_t linked_insert(void *inx, multi_t key, void *data) {

	inx_t *index = inx;
	linked_node_t *holder, *node;

	if ((node = mm_alloc(sizeof(linked_node_t))) == NULL) {
		log_info("Unable to allocate %zu bytes for a linked node.", sizeof(linked_node_t));
		return false;
	}
	else if ((node->record = linked_record_alloc(key, data)) == NULL) {
		log_info("Unable to allocate an index record.");
		mm_free(node);
		return false;
	}

	if (index->index == NULL) {
		index->index = node;
	}
	else {
		holder = index->index;
		while (holder->next != NULL) {
			holder = (linked_node_t *)holder->next;
		}
		node->prev = (struct linked_node_t *)holder;
		holder->next = (struct linked_node_t *)node;
	}

	index->count++;
	index->serial++;
	return true;
}

/**
 * @brief	Get the current node pointed to by a linked list cursor.
 * @param	cursor	a pointer to the linked list cursor to be queried.
 *
 *
 *
 * @return	a pointer to the current node indicated by the specified linked list cursor.
 */
linked_node_t * linked_cursor_active(linked_cursor_t *cursor) {

	linked_node_t *node;
	uint64_t position = 0;

	if (cursor->serial == cursor->inx->serial) {
		return cursor->node;
	}

	if (cursor->node) {
		node = cursor->inx->index;
		while (node && node != cursor->node) {
			node = (linked_node_t *)node->next;
			position++;
		}

		// If the active node is still in the index, update its position and return.
		if (node == cursor->node) {
			cursor->position = position;
			cursor->count = cursor->inx->count;
			cursor->serial = cursor->inx->serial;
			return node;
		}
		// If the node was removed from the list we reduce the position counter. We have to assume the active node is the only one missing
		// and that reducing the position counter will trigger the selection of the previous node below.
		/// LOW: The logic used to find our place in an index that has been modified could be improved.
		///		- If the index we sort the index keys, we could look for the next highest key value.
		///   - We could store the previous node (or even the previous 10 nodes), and try searching for them instead.
		///   - We could also add a delete path through the cursor that would include positional updates.
		///   - We could simply make a deep copy of the entire list so that updates to the original index don't affect the iteration, or use the copy to reconcile.
		else if (cursor->position) {
			cursor->position--;
		}
	}

	// Use the counter to find our place if the data pointer wasn't found above.
	node = cursor->inx->index;
	position = cursor->position;
	while (node && position--) {
		node = (linked_node_t *)node->next;
	}

	if (position) {
		cursor->node = node;
		cursor->count = cursor->inx->count;
		cursor->serial = cursor->inx->serial;
	}

	return node;
}

/**
 * @brief	Get the next node of a linked list cursor.
 * @note	The cursor position will be reset if the end of the linked list has been reached.
 * @param	cursor	a pointer to the linked list cursor to be queried.
 * @return	a pointer to the next node of the linked list cursor.
 */
linked_node_t * linked_cursor_next(linked_cursor_t *cursor) {

	linked_node_t *node;

	if (cursor->node) {
		if ((node = linked_cursor_active(cursor))) {
			cursor->node = (node = (linked_node_t *)node->next);
			cursor->position++;
		}
	}
	else {
			cursor->node = node = cursor->inx->index;
	}

	return node;
}

/**
 * @brief	Get the data of a linked list cursor's next record, and update the cursor.
 * @param	cursor	a pointer to the linked list cursor to be queried.
 * @return	NULL on failure, or the data of the linked list cursor's next record on success.
 */
void * linked_cursor_value_next(linked_cursor_t *cursor) {

	linked_node_t *node;

	if ((node = linked_cursor_next(cursor))) {
		return linked_record_get_data(node->record, 0);
	}
	return NULL;
}

/**
 * @brief	Get the data of a linked list cursor's current record.
 * @param	cursor	a pointer to the linked list cursor to be queried.
 * @return	NULL on failure, or a pointer to the data of the linked list cursor's current record.
 */
void * linked_cursor_value_active(linked_cursor_t *cursor) {

	linked_node_t *node;

	if ((node = linked_cursor_active(cursor))) {
		return linked_record_get_data(node->record, 0);
	}
	return NULL;
}

/**
 * @brief	Get the multi-type key value of a linked list cursor's next record, and update the cursor.
 * @param	cursor	a pointer to the linked list cursor to be queried.
 * @return	an empty multi-type key on failure, or the linked list cursor's next record key on success.
 */
multi_t linked_cursor_key_next(linked_cursor_t *cursor) {

	linked_node_t *node;

	if ((node = linked_cursor_next(cursor))) {
		return linked_record_get_key(node->record);
	}

	return mt_get_null();
}

/**
 * @brief	Get the multi-type key value at the current linked list cursor position.
 * @param	cursor	a pointer to the linked list cursor to be queried.
 * @return	an empty multi-type key on failure, or the current linked list cursor's record key on success.
 */
multi_t linked_cursor_key_active(linked_cursor_t *cursor) {

	linked_node_t *node;

	if ((node = linked_cursor_active(cursor))) {
		return linked_record_get_key(node->record);
	}

	return mt_get_null();
}

/**
 * @brief	Reset the position of a linked list cursor.
 * @param	cursor	a pointer to the linked list cursor object to be reset.
 * @return	This function returns no value.
 */
void linked_cursor_reset(linked_cursor_t *cursor) {

	if (cursor) {
		cursor->node = NULL;
		cursor->serial = cursor->position = cursor->count = 0;
	}

	return;
}

/**
 * @brief	Free a linked list cursor.
 * @param	cursor	a pointer to the linked list cursor object to be freed.
 * @return	This function returns no value.
 */
void linked_cursor_free(linked_cursor_t *cursor) {

	if (cursor) {
		mm_free(cursor);
	}

	return;
}

/**
 * @brief	Allocate a cursor to traverse a linked list.
 * @param	inx		a pointer the linked list object to be traversed by the cursor.
 * @return	NULL on failure, or a cursor pointing to the head of the linked list on success.
 */
void * linked_cursor_alloc(inx_t *inx) {

	linked_cursor_t *cursor;

	if (!(cursor = mm_alloc(sizeof(linked_cursor_t)))) {
		log_pedantic("Failed to allocate %zu bytes for a linked list index cursor.", sizeof(linked_cursor_t));
		return NULL;
	}

	cursor->inx = inx;

	return cursor;
}

/**
 * @brief	Truncate all the nodes in a linked list, but do not free it.
 * @param	inx		a pointer to the linked list object to have all of its records truncated.
 * @return	This function returns no value.
 */
void linked_truncate(void *inx) {

	inx_t *index = inx;
	linked_node_t *node, *next;

	if (index == NULL || index->index == NULL) {
		return;
	}

	node = index->index;

	while (node != NULL) {
		next = (linked_node_t *)node->next;
		linked_record_free(index, node->record);
		mm_free(node);
		node = next;
	}

	index->index = NULL;
	return;
}

/**
 * @brief	Free a linked list object and all of its records.
 * @see		linked_truncate()
 * @param	inx		a pointer to the linked list object to be freed.
 * @return	This function returns no value.
 */
void linked_free(void *inx) {

	inx_t *index = inx;

	if (index == NULL || index->index == NULL) {
		return;
	}

	// For linked lists truncation involves the same steps as free.
	linked_truncate(inx);

	return;
}

/**
 * @brief	Allocate a new linked list instance.
 * @param	options		an options value for the newly created linked list object.
 * @param	data_free	a pointer to a function used to free linked list items.
 * @return	NULL on failure or a pointer to the newly allocated linked list object on success.
 */
inx_t * linked_alloc(uint64_t options, void *data_free) {

	inx_t *result;

	if ((result = mm_alloc(sizeof(inx_t))) == NULL) return NULL;

	result->options = options;
	result->data_free = data_free;
	result->index_free = linked_free;
	result->index_truncate = linked_truncate;

	result->find = linked_find;
	result->insert = linked_insert;
	result->delete = linked_delete;

	result->cursor_free = (void (*)(void *))&linked_cursor_free;
	result->cursor_reset = (void (*)(void *))&linked_cursor_reset;
	result->cursor_alloc = (void * (*)(void *))&linked_cursor_alloc;

	result->cursor_key_next = (multi_t (*)(void *))&linked_cursor_key_next;
	result->cursor_key_active = (multi_t (*)(void *))&linked_cursor_key_active;

	result->cursor_value_next = (void * (*)(void *))&linked_cursor_value_next;
	result->cursor_value_active = (void * (*)(void *))&linked_cursor_value_active;

	return result;
}
