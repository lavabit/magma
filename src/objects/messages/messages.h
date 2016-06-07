
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
bool_t            meta_messages_copier(new_meta_user_t *user, meta_message_t *message, uint64_t target, uint64_t *outnum, bool_t sequences, META_LOCK_STATUS locked);
bool_t            meta_messages_login_update(new_meta_user_t *user, META_LOCK_STATUS locked);
int_t             meta_messages_mover(new_meta_user_t *user, meta_message_t *message, uint64_t target, bool_t lookup, bool_t sequences, META_LOCK_STATUS locked);
int_t             meta_messages_update(new_meta_user_t *user, META_LOCK_STATUS locked);
void              meta_messages_update_sequences(inx_t *folders, inx_t *messages);

/// datatier.c
bool_t   messages_fetch(uint64_t usernum, message_folder_t *folder);
void     meta_data_fetch_message_tags(meta_message_t *message);
bool_t   meta_data_fetch_messages(new_meta_user_t *user);

#endif

