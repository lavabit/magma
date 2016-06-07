
/**
 * @file /magma/network/meta.h
 *
 * @brief	Meta information structures/types for users, folders, messages, etc.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_NETWORK_META_H
#define MAGMA_NETWORK_META_H


typedef enum {
	META_GET_NONE = 0,
	META_GET_MESSAGES = 1,
	META_GET_FOLDERS = 2,
	META_GET_CONTACTS = 4
} META_GET;

typedef enum {
	META_NEED_LOCK = 0,
	META_LOCKED = 1
} META_LOCK_STATUS;

typedef enum {
	META_CHECKPOINT_MESSAGES = 0,
	META_CHECKPOINT_FOLDERS = 1,
	META_CHECKPOINT_USER = 2
} META_CHECKPOINT;

typedef enum {
	META_USER_NONE = 0,
	META_USER_SSL = 1,
	META_USER_ADVERTISING = 2,
	META_USER_OVERQUOTA = 4,
	META_USER_ENCRYPT_DATA = 8
} META_USER_FLAGS;

typedef enum {
	META_PROT_NONE = 0,
	META_PROT_SMTP = 1,
	META_PROT_POP = 2,
	META_PROT_IMAP = 4,
	META_PROT_WEB = 8,
	META_PROT_GENERIC = 16,
	META_PROT_JSON = 32
} META_PROT;

enum {
	CREDENTIAL_MAIL = 0,
	CREDENTIAL_AUTH = 1
};

typedef struct {
	uint64_t alertnum, created;
	stringer_t *type, *message;
} __attribute__((__packed__)) meta_alert_t;

/***
 * @struct meta_alias_t
 * @brief	Describes user mailboxes and provides the display name for each address, while also indicating which address is the preferred default.
 */
typedef struct {
	bool_t selected;
	uint64_t aliasnum, created;
	stringer_t *display, *address;
} __attribute__((__packed__)) meta_alias_t;

typedef struct {
	size_t size;
	array_t *tags;
	chr_t server[33];
	uint32_t status, updated;
	uint64_t messagenum, foldernum, sequencenum, signum, sigkey, created;
} meta_message_t;

typedef struct {
	chr_t name[128]; // Even though we limit folder names to 16 characters, with modified UTF-7 escaping, the string could be longer.
	uint32_t order;
	uint64_t parent, foldernum;
} meta_folder_t;

// All of a user's information is stored using this structure.
typedef struct {
	META_USER_FLAGS flags;
	///NEXT: There is an undiscovered bug with read/write locks which can cause instability. To promote short term stability switch back
	// to using a mutex. This will mean only one thread may read or write an individual user's mailbox at any given time.
	// pthread_rwlock_t lock;
	pthread_mutex_t lock;
	stringer_t *username, *passhash /* passhash is old */, *verification, *storage_privkey, *storage_pubkey;
	inx_t *aliases, *messages, *folders, *message_folders, *ads, *contacts;
	struct {
		uint64_t user, messages, folders, contacts;
	} serials;
	struct {
		time_t stamp;
		uint64_t smtp, pop, imap, web, generic;
		pthread_mutex_t lock;
	} refs;
	uint64_t usernum, lock_status;
} meta_user_t;

#endif

