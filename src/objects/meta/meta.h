
/**
 * @file /magma/objects/meta/meta.h
 *
 * @brief The interface for the meta objects.
 */

#ifndef MAGMA_OBJECTS_META_H
#define MAGMA_OBJECTS_META_H

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
bool_t     meta_user_serial_check(meta_user_t *user, uint64_t object);
uint64_t   meta_user_serial_get(meta_user_t *user, uint64_t object);
void       meta_user_serial_set(meta_user_t *user, uint64_t object, uint64_t serial);

/// updaters.c
int_t   meta_update_aliases(meta_user_t *user, META_LOCK_STATUS locked);
int_t   meta_update_contacts(meta_user_t *user, META_LOCK_STATUS locked);
int_t   meta_update_folders(meta_user_t *user, META_LOCK_STATUS locked);
int_t   meta_update_keys(meta_user_t *user, META_LOCK_STATUS locked);
int_t   meta_update_message_folders(meta_user_t *user, META_LOCK_STATUS locked);
int_t   meta_update_realms(meta_user_t *user, stringer_t *salt, stringer_t *master, META_LOCK_STATUS locked);
int_t   meta_update_user(meta_user_t *user, META_LOCK_STATUS locked);

/// references.c
void       meta_user_ref_add(meta_user_t *user, META_PROTOCOL protocol);
void       meta_user_ref_dec(meta_user_t *user, META_PROTOCOL protocol);
uint64_t   meta_user_ref_protocol_total(meta_user_t *user, META_PROTOCOL protocol);
time_t     meta_user_ref_stamp(meta_user_t *user);
uint64_t   meta_user_ref_total(meta_user_t *user);

/// meta.c
meta_user_t *  meta_alloc(void);
void           meta_free(meta_user_t *user);
int_t          meta_get(uint64_t usernum, stringer_t *username, stringer_t *salt, stringer_t *master, stringer_t *verification, META_PROTOCOL protocol, META_GET get, meta_user_t **output);

/// datatier.c
bool_t     meta_data_acknowledge_alert(uint64_t alertnum, uint64_t usernum, uint32_t transaction);
uint64_t   meta_data_delete_folder(uint64_t usernum, uint64_t foldernum);
int_t      meta_data_delete_tag(meta_message_t *message, stringer_t *tag);
inx_t *    meta_data_fetch_alerts(uint64_t usernum);
inx_t *    meta_data_fetch_all_tags(uint64_t usernum);
bool_t     meta_data_fetch_folders(meta_user_t *user);
int_t      meta_data_fetch_keys(meta_user_t *user, key_pair_t *output, int64_t transaction);
int_t      meta_data_fetch_mailbox_aliases(meta_user_t *user);
int_t      meta_data_fetch_shard(uint64_t usernum, uint16_t serial, stringer_t *label, stringer_t *output, uint_t *rotated, int64_t transaction);
int_t      meta_data_fetch_user(meta_user_t *user);
bool_t     meta_data_flags_add(inx_t *messages, uint64_t usernum, uint64_t foldernum, uint32_t flags);
bool_t     meta_data_flags_remove(inx_t *messages, uint64_t usernum, uint64_t foldernum, uint32_t flags);
bool_t     meta_data_flags_replace(inx_t *messages, uint64_t usernum, uint64_t foldernum, uint32_t flags);
uint64_t   meta_data_insert_folder(uint64_t usernum, stringer_t *name, uint64_t parent, uint32_t order);
int_t      meta_data_insert_keys(uint64_t usernum, stringer_t *username, key_pair_t *input, int64_t transaction);
int_t      meta_data_insert_shard(uint64_t usernum, uint16_t serial, stringer_t *label, stringer_t *shard, int64_t transaction);
int_t      meta_data_insert_tag(meta_message_t *message, stringer_t *tag);
int_t      meta_data_truncate_tags(meta_message_t *message);
uint64_t   meta_data_update_folder_name(uint64_t usernum, uint64_t foldernum, stringer_t *name, uint64_t parent, uint32_t order);
void       meta_data_update_lock(uint64_t usernum, uint8_t lock);
void       meta_data_update_log(meta_user_t *user, META_PROTOCOL prot);

/// locking.c
void   meta_user_rlock(meta_user_t *user);
void   meta_user_unlock(meta_user_t *user);
void   meta_user_wlock(meta_user_t *user);

/// indexes.c
meta_user_t *  meta_inx_find(uint64_t usernum, META_PROTOCOL protocol);
void           meta_inx_remove(uint64_t usernum, META_PROTOCOL protocol);

/// crypto.c
int_t   meta_crypto_keys_create(uint64_t usernum, stringer_t *username, stringer_t *realm, int64_t transaction);

/// alias.c
meta_alias_t *  alias_alloc(uint64_t aliasnum, stringer_t *address, stringer_t *display, int_t selected, uint64_t created);

#endif

