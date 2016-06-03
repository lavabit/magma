
/**
 * @file /magma/core/indexes/inx.c
 *
 * @brief	The generic index interface used to abstract away the underlying data structure used for storage.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Unlock an inx object.
 * @param	inx		a pointer to the inx object to be unlocked.
 * @return	This function returns no value.
 */
void inx_unlock(inx_t *inx) {
	if (!inx->automatic) {
		rwlock_unlock(&(inx->lock));
	}
	return;
}


void inx_auto_unlock(inx_t *inx) {
	if (inx->automatic) {
		rwlock_unlock(&(inx->lock));
	}
	return;
}

/**
 * @brief	Acquire a reader's lock for an inx object.
 * @param	inx		a pointer to the inx object to be locked.
 * @return	This function returns no value.
 */
void inx_lock_read(inx_t *inx) {
	if (!inx->automatic) {
		rwlock_lock_read(&(inx->lock));
	}
	return;
}

void inx_auto_read(inx_t *inx) {
	if (inx->automatic) {
		rwlock_lock_read(&(inx->lock));
	}
	return;
}
/**
 * @brief	Acquire a writer's lock for an inx object.
 * @param	inx		a pointer to the inx object to be locked.
 * @return	This function returns no value.
 */
void inx_lock_write(inx_t *inx) {
	if (!inx->automatic) {
		rwlock_lock_write(&(inx->lock));
	}
	return;
}


void inx_auto_write(inx_t *inx) {
	if (inx->automatic) {
		rwlock_lock_write(&(inx->lock));
	}
	return;
}

/**
 * @brief	Return the options value of an inx object.
 * @param	inx		a pointer to the inx object to be examined.
 * @return	0 on failure, or the options value of the inx object on success.
 */
uint64_t inx_options(inx_t *inx) {

	uint64_t options;

#ifdef MAGMA_PEDANTIC
	if (!inx) {
		log_pedantic("The index pointer is invalid.");
		return 0;
	}
#endif

	inx_auto_read(inx);
	options = inx->options;
	inx_auto_unlock(inx);

	return options;
}

/**
 * @brief	Return the total number of items held by an inx object.
 * @param	inx		a pointer to the inx object to be examined.
 * @return	the total number of items held by the inx object.
 */
uint64_t inx_count(inx_t *inx) {

	uint64_t count;

#ifdef MAGMA_PEDANTIC
	if (!inx) {
		log_pedantic("The index pointer is invalid.");
		return 0;
	}
#endif

	inx_auto_read(inx);
	count = inx->count;
	inx_auto_unlock(inx);

	return count;
}

uint64_t inx_serial(inx_t *inx) {

	uint64_t serial;

#ifdef MAGMA_PEDANTIC
	if (!inx) {
		log_pedantic("The index pointer is invalid.");
		return 0;
	}
#endif

	inx_auto_read(inx);
	serial = inx->count;
	inx_auto_unlock(inx);

	return serial;
}

/**
 * @brief	Insert a new record into an inx holder.
 * @param	inx		a pointer to the inx object that will hold the record.
 * @param	key		a multi-type value specifying the identifier of the record to be inserted.
 * @param	data	a pointer to the data associated with the new key.
 * @return	true if the new record was inserted successfully or false on failure.
 */
bool_t inx_insert(inx_t *inx, multi_t key, void *data) {

	bool_t result;

//#ifdef MAGMA_PEDANTIC
	if (!inx || !(inx->insert)) {
		log_pedantic("Invalid index or function pointer.");
		return false;
	}
//#endif

	inx_auto_write(inx);
	result = inx->insert(inx, key, data);
	inx_auto_unlock(inx);

	return result;
}

/**
 * @brief	Replace the value of a key in an inx holder.
 * @param	inx		a pointer to the inx object where the specified key will be replaced.
 * @param	key		a multi-type value specifying the identifier of the record to be replaced.
 * @param	data	a pointer to the new data to be associated with the specified key.
 * @return	true if the specified key's value was replaced successfully, or false on failure.
 */
bool_t inx_replace(inx_t *inx, multi_t key, void *data) {

	bool_t result;

#ifdef MAGMA_PEDANTIC
	if (!inx || !(inx->delete) || !(inx->insert)) {
		log_pedantic("Invalid index or function pointer.");
		return false;
	}
#endif

	inx_auto_write(inx);
	// Delete the existing record, if there is one.
	inx->delete(inx, key);
	// Insert the new record.
	result = inx->insert(inx, key, data);
	inx_auto_unlock(inx);

	return result;

}

/**
 * @brief	Delete a child of an inx object with a specified key.
 * @param	inx		a pointer to the inx object to be searched.
 * @param	key		the target key of the object to be deleted.
 * @return	true if the delete operation succeeded or false if it did not.
 */
bool_t inx_delete(inx_t *inx, multi_t key) {

	bool_t result;

#ifdef MAGMA_PEDANTIC
	if (!inx || !(inx->delete)) {
		log_pedantic("Invalid index or function pointer.");
		return false;
	}
#endif

	inx_auto_write(inx);
	result = inx->delete(inx, key);
	inx_auto_unlock(inx);

	return result;
}

/**
 * @brief	Find the value associated with a particular key within the children of an inx object.
 * @param	inx		a pointer to the inx object to be searched.
 * @param	key		the target key to be found.
 * @return	NULL on failure, or the value associated with the requested key on success.
 */
void * inx_find(inx_t *inx, multi_t key) {

	void *result;

#ifdef MAGMA_PEDANTIC
	if (!inx || !(inx->find)) {
		log_pedantic("Invalid index or function pointer.");
		return NULL;
	}
#endif

	inx_auto_read(inx);
	result = inx->find(inx, key);
	inx_auto_unlock(inx);

	return result;
}

void inx_free(inx_t *inx) {

	uint64_t refs;

#ifdef MAGMA_PEDANTIC
	if (!inx || !(inx->index_free)) {
		log_pedantic("The function pointer is invalid.");
		return;
	}
#endif

	inx_auto_write(inx);
	inx->references--;
	refs = inx->references;
	inx_auto_unlock(inx);

	if (!refs) {
		inx->index_free(inx);
		rwlock_destroy(&(inx->lock));
		mm_free(inx);
	}

	return;
}

void inx_truncate(inx_t *inx) {

#ifdef MAGMA_PEDANTIC
	if (!inx || !(inx->index_truncate)) {
		log_pedantic("The function pointer is invalid.");
		return;
	}
#endif

	inx_auto_write(inx);
	inx->index_truncate(inx);
	inx_auto_unlock(inx);

	return;
}

/**
 * @brief	Perform a checked free of an inx object.
 * @see		inx_free()
 * @param	inx		the inx instance to be freed.
 */
void inx_cleanup(inx_t *inx) {
	if (inx) {
		inx_free(inx);
	}
	return;
}

/**
 * @brief	Allocate a new inx instance.
 * @param	options	 	a value indicating the inx type. Can be M_INX_TREE for a binary tree, M_INX_LINKED for a linked list, or M_INX_HASHED for a hash tree.
 * @param	data_free	a function pointer to a routine to free the data associated with an inx record.
 * @return	NULL on failure or a pointer to the newly created inx object on success.
 */
inx_t * inx_alloc(uint64_t options, void *data_free) {

	inx_t *inx = NULL;

	switch (options & MAGMA_INDEX_TYPE) {
	case M_INX_TREE:
		inx = tree_alloc(options, data_free);
		break;
	case M_INX_LINKED:
		inx = linked_alloc(options, data_free);
		break;
	case M_INX_HASHED:
		inx = hashed_alloc(options, data_free);
		break;
	default:
		log_options(M_LOG_ERROR | M_LOG_STACK_TRACE, "Unsupported index type detected. {type = %lu}", options & MAGMA_INDEX_TYPE);
		break;
	};

	if (inx) {
		inx->automatic = options & M_INX_LOCK_MANUAL ? 0 : 1;
		rwlock_init(&(inx->lock), NULL);
		inx->references++;
	}

	return inx;
}

