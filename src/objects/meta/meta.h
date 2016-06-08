
/**
 * @file /magma/src/objects/meta/meta.h
 *
 * @brief The interface for the new_meta objects.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_OBJECTS_NEW_META_H
#define MAGMA_OBJECTS_NEW_META_H

typedef struct {

	stringer_t *public;
	stringer_t *private;

} key_pair_t;

/// serials.c
bool_t     new_meta_user_serial_check(new_meta_user_t *user, uint64_t object);
uint64_t   new_meta_user_serial_get(new_meta_user_t *user, uint64_t object);
void       new_meta_user_serial_set(new_meta_user_t *user, uint64_t object, uint64_t serial);

/// updaters.c
int_t   meta_aliases_update(new_meta_user_t *user, META_LOCK_STATUS locked);
int_t   meta_message_folders_update(new_meta_user_t *user, META_LOCK_STATUS locked);
int_t   new_meta_contacts_update(new_meta_user_t *user, META_LOCK_STATUS locked);
int_t   new_meta_folders_update(new_meta_user_t *user, META_LOCK_STATUS locked);
int_t   new_meta_user_update(new_meta_user_t *user, META_LOCK_STATUS locked);

/// references.c
void       new_meta_user_ref_add(new_meta_user_t *user, META_PROTOCOL protocol);
void       new_meta_user_ref_dec(new_meta_user_t *user, META_PROTOCOL protocol);
uint64_t   new_meta_user_ref_protocol_total(new_meta_user_t *user, META_PROTOCOL protocol);
time_t     new_meta_user_ref_stamp(new_meta_user_t *user);
uint64_t   new_meta_user_ref_total(new_meta_user_t *user);

/// meta.c
new_meta_user_t *  new_meta_alloc(void);
void               new_meta_free(new_meta_user_t *user);
int_t              new_meta_get(uint64_t usernum, stringer_t *username, stringer_t *master, stringer_t *verification, META_PROTOCOL flags, META_GET get, new_meta_user_t **output);

/// datatier.c
bool_t   new_meta_data_fetch_folders(new_meta_user_t *user);
int_t    new_meta_data_fetch_keys(new_meta_user_t *user, key_pair_t *output);
int_t    new_meta_data_fetch_mailbox_aliases(new_meta_user_t *user);
int_t    new_meta_data_fetch_user(new_meta_user_t *user);
int_t    new_meta_data_insert_keys(new_meta_user_t *user, key_pair_t *input);
void     new_meta_data_update_log(new_meta_user_t *user, META_PROTOCOL prot);

/// locking.c
void   new_meta_user_rlock(new_meta_user_t *user);
void   new_meta_user_unlock(new_meta_user_t *user);
void   new_meta_user_wlock(new_meta_user_t *user);

/// indexes.c
new_meta_user_t *  new_meta_inx_find(uint64_t usernum, META_PROTOCOL flags);
void               new_meta_inx_remove(uint64_t usernum, META_PROTOCOL flags);

#endif

