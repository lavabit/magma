
/**
 * @file /magma/objects/contacts/contacts.h
 *
 * @brief	Interface for managing user address book contacts.
 */

#ifndef MAGMA_OBJECTS_CONTACTS_H
#define MAGMA_OBJECTS_CONTACTS_H

enum {
	CONTACT_DETAIL_FLAG_NONE = 0,
	CONTACT_DETAIL_FLAG_CRITICAL = 1
};

typedef struct __attribute__ ((__packed__)) {
	uint64_t flags;
	stringer_t *key, *value;
} contact_detail_t;

typedef struct __attribute__ ((__packed__)) {
	inx_t *details;
	stringer_t *name;
	uint64_t contactnum;
} contact_t;

/// contacts.c
contact_t *         contact_alloc(uint64_t contactnum, stringer_t *name);
contact_t *         contact_create(uint64_t usernum, uint64_t foldernum, stringer_t *name);
contact_detail_t *  contact_detail_alloc(stringer_t *key, stringer_t *value, uint64_t flags);
void                contact_detail_free(contact_detail_t *detail);
int_t               contact_edit(contact_t *contact, uint64_t usernum, uint64_t foldernum, stringer_t *key, stringer_t *value);
void                contact_free(contact_t *contact);
int_t               contact_move(contact_folder_t *source, contact_folder_t *target, contact_t *contact, uint64_t usernum);
void                contact_name(contact_t *contact, stringer_t *name);
int_t               contact_remove(contact_t *contact, uint64_t usernum, uint64_t foldernum);
bool_t              contact_validate_name(stringer_t *name, chr_t **errmsg);
bool_t              contact_validate_detail(stringer_t *key, stringer_t *value, chr_t **errmsg);
inx_t *             contacts_update(uint64_t usernum);

/// datatier.c
int_t      contact_delete(uint64_t contactnum, uint64_t usernum, uint64_t foldernum);
int_t      contact_detail_delete(uint64_t contactnum, stringer_t *key);
int_t      contact_detail_upsert(uint64_t contactnum, stringer_t *key, stringer_t *value, uint64_t flags);
int_t      contact_details_fetch(contact_t *contact);
uint64_t   contact_insert(uint64_t usernum, uint64_t foldernum, stringer_t *name);
int_t      contact_update(uint64_t contactnum, uint64_t usernum, uint64_t cur_folder, uint64_t target_folder, stringer_t *name);
int_t      contact_update_stamp(uint64_t contactnum, uint64_t usernum, uint64_t foldernum);
int_t      contacts_fetch(uint64_t usernum, contact_folder_t *folder);

/// find.c
contact_t *  contact_find_detail(contact_folder_t *folder, stringer_t *key, stringer_t *target);
contact_t *  contact_find_name(contact_folder_t *folder, stringer_t *target);
contact_t *  contact_find_number(contact_folder_t *folder, uint64_t target);

#endif
