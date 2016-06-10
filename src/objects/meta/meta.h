
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

typedef struct {
	uint64_t count;
	stringer_t *tag;
} meta_stats_tag_t;

/// alerts.c
meta_alert_t * alert_alloc(uint64_t alertnum, stringer_t *type, stringer_t *message, uint64_t created);

/// folders.c
meta_stats_tag_t *  meta_folder_stats_tag_alloc(stringer_t *tag);
meta_folder_t *     meta_folders_by_name(inx_t *folders, stringer_t *name);
meta_folder_t *     meta_folders_by_number(inx_t *folders, uint64_t number);
int_t               meta_folders_children(inx_t *folders, uint64_t number);
stringer_t *        meta_folders_name(inx_t *list, meta_folder_t *folder);
inx_t *             meta_folders_stats_tags(inx_t *messages, uint64_t folder);

/// serials.c
bool_t     new_meta_user_serial_check(new_meta_user_t *user, uint64_t object);
uint64_t   new_meta_user_serial_get(new_meta_user_t *user, uint64_t object);
void       new_meta_user_serial_set(new_meta_user_t *user, uint64_t object, uint64_t serial);

/// updaters.c
int_t   meta_update_aliases(new_meta_user_t *user, META_LOCK_STATUS locked);
int_t   meta_update_keys(new_meta_user_t *user, stringer_t *master, META_LOCK_STATUS locked);
int_t   meta_update_message_folders(new_meta_user_t *user, META_LOCK_STATUS locked);
int_t   new_meta_update_contacts(new_meta_user_t *user, META_LOCK_STATUS locked);
int_t   new_meta_update_folders(new_meta_user_t *user, META_LOCK_STATUS locked);
int_t   new_meta_update_user(new_meta_user_t *user, META_LOCK_STATUS locked);

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
bool_t     meta_data_acknowledge_alert(uint64_t alertnum, uint64_t usernum, uint32_t transaction);
uint64_t   meta_data_delete_folder(uint64_t usernum, uint64_t foldernum);
int_t      meta_data_delete_tag(meta_message_t *message, stringer_t *tag);
inx_t *    meta_data_fetch_alerts(uint64_t usernum);
inx_t *    meta_data_fetch_all_tags(uint64_t usernum);
bool_t     meta_data_flags_add(inx_t *messages, uint64_t usernum, uint64_t foldernum, uint32_t flags);
bool_t     meta_data_flags_remove(inx_t *messages, uint64_t usernum, uint64_t foldernum, uint32_t flags);
bool_t     meta_data_flags_replace(inx_t *messages, uint64_t usernum, uint64_t foldernum, uint32_t flags);
uint64_t   meta_data_insert_folder(uint64_t usernum, stringer_t *name, uint64_t parent, uint32_t order);
int_t      meta_data_insert_tag(meta_message_t *message, stringer_t *tag);
int_t      meta_data_truncate_tags(meta_message_t *message);
uint64_t   meta_data_update_folder_name(uint64_t usernum, uint64_t foldernum, stringer_t *name, uint64_t parent, uint32_t order);
void       meta_data_update_lock(uint64_t usernum, uint8_t lock);
bool_t     new_meta_data_fetch_folders(new_meta_user_t *user);
int_t      new_meta_data_fetch_keys(new_meta_user_t *user, key_pair_t *output, int64_t transaction);
int_t      new_meta_data_fetch_mailbox_aliases(new_meta_user_t *user);
int_t      new_meta_data_fetch_user(new_meta_user_t *user);
int_t      new_meta_data_insert_keys(uint64_t usernum, stringer_t *username, key_pair_t *input, int64_t transaction);
void       new_meta_data_update_log(new_meta_user_t *user, META_PROTOCOL prot);

/// locking.c
void   new_meta_user_rlock(new_meta_user_t *user);
void   new_meta_user_unlock(new_meta_user_t *user);
void   new_meta_user_wlock(new_meta_user_t *user);

/// indexes.c
new_meta_user_t *  new_meta_inx_find(uint64_t usernum, META_PROTOCOL protocol);
void               new_meta_inx_remove(uint64_t usernum, META_PROTOCOL protocol);

/// crypto.c
int_t   meta_crypto_keys_create(uint64_t usernum, stringer_t *username, stringer_t *master, int64_t transaction);

/// alias.c
meta_alias_t *  alias_alloc(uint64_t aliasnum, stringer_t *address, stringer_t *display, int_t selected, uint64_t created);

#endif

