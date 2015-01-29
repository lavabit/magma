
/**
 * @file /magma/objects/folders/folders.h
 *
 * @brief	Interface to managing user folders.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_OBJECTS_FOLDERS_H
#define MAGMA_OBJECTS_FOLDERS_H

#define FOLDER_LENGTH_LIMIT 16
#define FOLDER_RECURSION_LIMIT 16

/***
 * @note When defining a new folder class, add the alloc/free functions to magma_folder_funcs().
 */
enum {
	M_FOLDER_MESSAGES = 1,//!< M_FOLDER_MESSAGES
	M_FOLDER_CONTACTS = 2 //!< M_FOLDER_CONTACTS
};

typedef struct __attribute__((__packed__)) {
	inx_t *records;
	uint32_t order;
	stringer_t *name;
	pthread_rwlock_t lock;
	uint64_t foldernum, parent;
} magma_folder_t;

typedef magma_folder_t contact_folder_t;
typedef magma_folder_t message_folder_t;

/// messages.c
message_folder_t *  message_folder_alloc(uint64_t foldernum, uint64_t parent, uint32_t order, stringer_t *name);
int_t               message_folder_create(uint64_t usernum, uint64_t parent, stringer_t *name, inx_t *folders);
void                message_folder_free(message_folder_t *folder);
bool_t              message_folder_remove(uint64_t usernum, uint64_t foldernum, inx_t *folders);

/// contacts.c
contact_folder_t *  contact_folder_alloc(uint64_t foldernum, uint64_t parent, uint32_t order, stringer_t *name);
int_t               contact_folder_create(uint64_t usernum, uint64_t parent, stringer_t *name, inx_t *folders);
void                contact_folder_free(contact_folder_t *folder);
int_t               contact_folder_remove(uint64_t usernum, uint64_t foldernum, inx_t *folders);
bool_t              contact_folder_rename(uint64_t usernum, uint64_t foldernum, stringer_t *rename, inx_t *folders);

/// datatier.c
bool_t     magma_folder_delete(uint64_t usernum, uint64_t foldernum, uint_t type);
inx_t *    magma_folder_fetch(uint64_t usernum, uint_t type);
uint64_t   magma_folder_insert(uint64_t usernum, stringer_t *name, uint64_t parent, uint32_t order, uint_t type);
bool_t     magma_folder_rename(uint64_t usernum, uint64_t foldernum, uint_t type, stringer_t *rename);

/// find.c
magma_folder_t *  magma_folder_find_name(inx_t *folders, stringer_t *target, uint64_t parent, bool_t check_inbox);
magma_folder_t *  magma_folder_find_full_name(inx_t *folders, stringer_t *target, bool_t check_inbox);
magma_folder_t *  magma_folder_find_number(inx_t *folders, uint64_t target);

/// folders.c
magma_folder_t *  magma_folder_alloc(uint64_t foldernum, uint64_t parent, uint32_t order, stringer_t *name);
int_t             magma_folder_children(inx_t *folders, uint64_t target);
void              magma_folder_free(magma_folder_t *folder);
bool_t            magma_folder_funcs(uint_t type, magma_folder_t * (**folder_alloc)(uint64_t, uint64_t, uint32_t, stringer_t *), void (**folder_free)(magma_folder_t *));
stringer_t *      magma_folder_name(inx_t *folders, magma_folder_t *target);

#endif

