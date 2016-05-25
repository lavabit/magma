
/**
 * @file /magma/objects/users/users.h
 *
 * @brief	Functions to interface with and manage user data.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_OBJECTS_USERS_H
#define MAGMA_OBJECTS_USERS_H

// Message marking & status codes.
enum {
	MAIL_STATUS_NONE = 0,
	MAIL_STATUS_EMPTY = 1,
	MAIL_STATUS_RECENT = 2,
	MAIL_STATUS_SEEN = 4,
	MAIL_STATUS_ANSWERED = 8,
	MAIL_STATUS_FLAGGED = 16,
	MAIL_STATUS_DELETED = 32,
	MAIL_STATUS_DRAFT = 64,

	MAIL_STATUS_SECURE = 128,
	MAIL_STATUS_APPENDED = 256, // Use to indicate uploaded messages. These don't get ads, and don't show up in POP sessions.
	MAIL_STATUS_HIDDEN = 512, // Used to indicate the message has been deleted by a POP session.

	MAIL_MARK_JUNK = 1024,
	MAIL_MARK_INFECTED = 2048,
	MAIL_MARK_SPOOFED = 4096,
	MAIL_MARK_BLACKHOLED = 8192,
	MAIL_MARK_PHISHING = 16384,

	MAIL_STATUS_TAGGED = 32768,

	MAIL_STATUS_ENCRYPTED = 65536
};

// The flags typically controlled by the user.
#define MAIL_STATUS_USER_FLAGS (MAIL_STATUS_SEEN | MAIL_STATUS_ANSWERED | MAIL_STATUS_FLAGGED | MAIL_STATUS_DELETED | MAIL_STATUS_DRAFT)

// The set of flags used exclusively by the system. User attempts to manipulate these flags should generate an error.
#define MAIL_STATUS_SYSTEM_FLAGS (MAIL_STATUS_EMPTY | MAIL_STATUS_RECENT | MAIL_STATUS_SECURE | MAIL_STATUS_APPENDED | MAIL_STATUS_HIDDEN | \
	MAIL_MARK_JUNK | MAIL_MARK_INFECTED | MAIL_MARK_SPOOFED | MAIL_MARK_BLACKHOLED | MAIL_MARK_PHISHING | MAIL_STATUS_TAGGED | MAIL_STATUS_ENCRYPTED)

// The complete collection.
#define MAIL_STATUS_ALL_FLAGS (MAIL_STATUS_USER_FLAGS | MAIL_STATUS_SYSTEM_FLAGS)

// Email message types.
#define MESSAGE_TYPE_UNKNOWN 0
#define MESSAGE_TYPE_PLAIN 1
#define MESSAGE_TYPE_HTML 2
#define MESSAGE_TYPE_MULTI_ALTERNATIVE 3
#define MESSAGE_TYPE_MULTI_MIXED 4
#define MESSAGE_TYPE_MULTI_RELATED 5
#define MESSAGE_TYPE_MULTI_RFC822 5
#define MESSAGE_TYPE_MULTI_UNKOWN 6

// Email message encodings.
#define MESSAGE_ENCODING_UNKNOWN 0
#define MESSAGE_ENCODING_QUOTED_PRINTABLE 1
#define MESSAGE_ENCODING_BASE64 2
#define MESSAGE_ENCODING_7BIT 3
#define MESSAGE_ENCODING_8BIT 4

typedef struct {
	uint64_t count;
	stringer_t *tag;
} meta_stats_tag_t;

/// messages.c
int_t   meta_message_folders_update(meta_user_t *user, META_LOCK_STATUS locked);

/// folders.c
meta_stats_tag_t *  meta_folder_stats_tag_alloc(stringer_t *tag);
meta_folder_t *     meta_folders_by_name(inx_t *folders, stringer_t *name);
meta_folder_t *     meta_folders_by_number(inx_t *folders, uint64_t number);
int_t               meta_folders_children(inx_t *folders, uint64_t number);
stringer_t *        meta_folders_name(inx_t *list, meta_folder_t *folder);
inx_t *             meta_folders_stats_tags(inx_t *messages, uint64_t folder);
int_t               meta_folders_update(meta_user_t *user, META_LOCK_STATUS locked);

/// datatier.c
bool_t     meta_data_acknowledge_alert(uint64_t alertnum, uint64_t usernum, uint32_t transaction);
uint64_t   meta_data_delete_folder(uint64_t usernum, uint64_t foldernum);
int_t      meta_data_delete_tag(meta_message_t *message, stringer_t *tag);
inx_t *    meta_data_fetch_alerts(uint64_t usernum);
bool_t     meta_data_fetch_folders(meta_user_t *user);
bool_t     meta_data_fetch_mailbox_aliases(meta_user_t *user);
bool_t     meta_data_check_mailbox(stringer_t *address);
bool_t     meta_data_fetch_messages(meta_user_t *user);
int_t      meta_check_message_encryption(meta_user_t *user);
inx_t *    meta_data_fetch_all_tags(uint64_t usernum);
void       meta_data_fetch_message_tags(meta_message_t *message);
bool_t     meta_data_fetch_user(meta_user_t *user);
bool_t     meta_data_flags_add(inx_t *messages, uint64_t usernum, uint64_t foldernum, uint32_t flags);
bool_t     meta_data_flags_remove(inx_t *messages, uint64_t usernum, uint64_t foldernum, uint32_t flags);
bool_t     meta_data_flags_replace(inx_t *messages, uint64_t usernum, uint64_t foldernum, uint32_t flags);
uint64_t   meta_data_insert_folder(uint64_t usernum, stringer_t *name, uint64_t parent, uint32_t order);
int_t      meta_data_insert_tag(meta_message_t *message, stringer_t *tag);
int_t      meta_data_truncate_tags(meta_message_t *message);
uint64_t   meta_data_update_folder_name(uint64_t usernum, uint64_t foldernum, stringer_t *name, uint64_t parent, uint32_t order);
void       meta_data_update_lock(uint64_t usernum, uint8_t lock);
void       meta_data_update_log(meta_user_t *user, META_PROT prot);
int_t      meta_data_user_build(meta_user_t *user, credential_t *cred);
int_t      meta_data_user_build_storage_keys(uint64_t usernum, stringer_t *passkey, stringer_t **priv_out, stringer_t **pub_out, bool_t dont_create, bool_t do_trans, uint32_t tid);
int_t      meta_data_user_save_storage_keys(uint64_t usernum, stringer_t *passkey, stringer_t *pubkey, stringer_t *privkey, bool_t do_trans, uint32_t tid);

/// alerts.c
meta_alert_t *  alert_alloc(uint64_t alertnum, stringer_t *type, stringer_t *message, uint64_t created);

/// aliases.c
meta_alias_t *  alias_alloc(uint64_t aliasnum, stringer_t *address, stringer_t *display, int_t selected, uint64_t created);

/// contacts.c
int_t   meta_contacts_update(meta_user_t *user, META_LOCK_STATUS locked);

/// users.c
int_t          meta_get(credential_t *cred, META_PROT flags, META_GET get, meta_user_t **output);
void           meta_remove(stringer_t *username, META_PROT flags);
int_t          meta_user_build(meta_user_t *user, credential_t *cred, META_LOCK_STATUS locked);
meta_user_t *  meta_user_create(void);
void           meta_user_destroy(meta_user_t *user);
int_t          meta_user_prune(stringer_t *username);
void           meta_user_ref_add(meta_user_t *user, META_PROT protocol);
void           meta_user_ref_dec(meta_user_t *user, META_PROT protocol);
time_t         meta_user_ref_stamp(meta_user_t *user);
uint64_t       meta_user_ref_total(meta_user_t *user);
void           meta_user_rlock(meta_user_t *user);
bool_t         meta_user_serial_check(meta_user_t *user, uint64_t object);
uint64_t       meta_user_serial_get(meta_user_t *user, uint64_t object);
void           meta_user_serial_set(meta_user_t *user, uint64_t object, uint64_t serial);
void           meta_user_unlock(meta_user_t *user);
int_t          meta_user_update(meta_user_t *user, META_LOCK_STATUS locked);
void           meta_user_wlock(meta_user_t *user);

#endif
