
/**
 * @file /magma/core/indexes/indexes.h
 *
 * @brief	Function declarations and types for the generic index interface.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */


#ifndef MAGMA_CORE_INDEXES_H
#define MAGMA_CORE_INDEXES_H

/**
 * Index types and options.
 */
typedef enum {
	M_INX_TREE = 1, //!< M_INX_BTREE
	M_INX_HASHED = 2, //!< M_INX_HASHED
	M_INX_LINKED = 4, //!< M_INX_LINKED
	//M_INX_ALLOW_DUPE = 8, //!< M_INX_ALLOW_DUPE
	M_INX_LOCK_MANUAL = 16, //!< M_INX_LOCK_MANUAL

} MAGMA_INDEX;

/**
 * The different types of indexes.
 */
#define MAGMA_INDEX_TYPE (M_INX_TREE | M_INX_LINKED | M_INX_HASHED)

/**
 * The different index options.
 */
#define MAGMA_INDEX_OPTION (M_INX_INDEX_LOCK)

typedef struct {

	// Data and record count
	void *index;
	pthread_rwlock_t lock;
	uint64_t count, serial, automatic, options, references;

	// Index function pointers.
	void (*data_free)(void *data);
	void (*index_free)(void *index);
	void (*index_truncate)(void *index);

	bool_t (*delete)(void *index, multi_t envelope);
	bool_t (*insert)(void *index, multi_t envelope, void *data);

	void * (*find)(void *index, multi_t envelope);

	void (*cursor_free)(void *cursor);
	void (*cursor_reset)(void *cursor);
	void * (*cursor_alloc)(void *index);

	void * (*cursor_value_next)(void *cursor);
	void * (*cursor_value_active)(void *cursor);

	multi_t (*cursor_key_next)(void *cursor);
	multi_t (*cursor_key_active)(void *cursor);

} inx_t;

typedef struct {
	inx_t *inx;
} __attribute__((__packed__)) inx_cursor_t;

/// cursors.c
inx_cursor_t *  inx_cursor_alloc(inx_t *index);
void            inx_cursor_free(inx_cursor_t *cursor);
multi_t         inx_cursor_key_active(inx_cursor_t *cursor);
multi_t         inx_cursor_key_next(inx_cursor_t *cursor);
void            inx_cursor_reset(inx_cursor_t *cursor);
void *          inx_cursor_value_active(inx_cursor_t *cursor);
void *          inx_cursor_value_next(inx_cursor_t *cursor);

/// inx.c
inx_t *    inx_alloc(uint64_t options, void *data_free);
void       inx_auto_read(inx_t *inx);
void       inx_auto_unlock(inx_t *inx);
void       inx_auto_write(inx_t *inx);
void       inx_cleanup(inx_t *inx);
uint64_t   inx_count(inx_t *inx);
bool_t     inx_delete(inx_t *inx, multi_t key);
void *     inx_find(inx_t *inx, multi_t key);
void       inx_free(inx_t *inx);
bool_t     inx_insert(inx_t *inx, multi_t key, void *data);
void       inx_lock_read(inx_t *inx);
void       inx_lock_write(inx_t *inx);
uint64_t   inx_options(inx_t *inx);
bool_t     inx_replace(inx_t *inx, multi_t key, void *data);
uint64_t   inx_serial(inx_t *inx);
void       inx_truncate(inx_t *inx);
void       inx_unlock(inx_t *inx);

/// linked.c
inx_t * linked_alloc(uint64_t options, void *data_free);

/// hashed.c
inx_t * hashed_alloc(uint64_t options, void *data_free);

#endif
