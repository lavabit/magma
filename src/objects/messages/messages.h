
/**
 * @file /magma/objects/messages/messages.h
 *
 * @brief	Mail message operations.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_OBJECTS_MESSAGES_H
#define MAGMA_OBJECTS_MESSAGES_H


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


/// LOW: Update the Messages table columns so they match the tank mail message header fields. Store individual header/body lengths
/// and the compressed/uncompressed hash values; then update the message type to store and use the new information.

typedef struct __attribute__ ((__packed__)) {

	struct {
		uint64_t num; /* Unique message number. */
		uint64_t created; /* When the message record was created. */
		uint64_t sequence; /* Position in the folder. */
	} message;

	struct {
		array_t *tags; /* Any message tags. */
		uint32_t flags; /* System flags. */
	} status;

	struct {
		uint64_t num; /* Junk filter signature number. */
		uint64_t key; /* Junk filter access key. */
	} signature;

	struct {
		stringer_t *server; /* The storage node. */
		stringer_t *header; /* The message header data. */
		stringer_t *body; /* The message body data. */
	} data;

	//struct {
		//size_t header; /* The size of the header data. */
		//size_t body; /* The size of the body data. */
	//} len;

	size_t size; /* The length of the raw message data. */
	pthread_rwlock_t lock; /* Control access to the message. */

} message_t;

#define FMESSAGE_MAGIC_1		0x17
#define FMESSAGE_MAGIC_2		0x76

#define FMESSAGE_OPT_COMPRESSED	0x1
#define FMESSAGE_OPT_ENCRYPTED	0x2

typedef struct __attribute__ ((packed)) {
	uint8_t magic1;		// first magic byte: 0x17
	uint8_t magic2;		// second magic byte: 0x76
	uint8_t reserved;
	uint8_t flags;
}  message_fheader_t;

/// messages.c
message_t *  message_alloc(uint64_t messagenum, uint64_t created, uint64_t signature, uint64_t key, uint64_t flags, stringer_t *server, size_t size);
void         message_free(message_t *message);
inx_t *      messages_update(uint64_t usernum);

/// meta.c
meta_message_t *  meta_message_by_number(inx_t *messages, uint64_t number);
meta_message_t *  meta_message_dupe(meta_message_t *message);
void              meta_message_free(meta_message_t *message);
bool_t            meta_messages_copier(meta_user_t *user, meta_message_t *message, uint64_t target, uint64_t *outnum, bool_t sequences, META_LOCK_STATUS locked);
bool_t            meta_messages_login_update(meta_user_t *user, META_LOCK_STATUS locked);
int_t             meta_messages_mover(meta_user_t *user, meta_message_t *message, uint64_t target, bool_t lookup, bool_t sequences, META_LOCK_STATUS locked);
int_t             meta_messages_update(meta_user_t *user, META_LOCK_STATUS locked);
void              meta_messages_update_sequences(inx_t *folders, inx_t *messages);

/// datatier.c
bool_t   meta_data_fetch_folder_messages(uint64_t usernum, message_folder_t *folder);
void     meta_data_fetch_message_tags(meta_message_t *message);
bool_t   meta_data_fetch_messages(meta_user_t *user);

#endif

