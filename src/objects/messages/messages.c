
/**
 * @file /magma/objects/messages/messages.c
 *
 * @brief	Mail message interface functions.
 */

#include "magma.h"

//	struct {
//		uint64_t num; /* Unique message number. */
//		uint64_t sequence; /* Position in the folder. */
//	} message;
//
//
//	struct {
//		array_t *tags; /* Any message tags. */
//		uint32_t flags; /* System flags. */
//	} status;
//
//	struct {
//		uint64_t num; /* Junk filter signature number. */
//		uint64_t key; /* Junk filter access key. */
//	} signature;
//
//	struct {
//		stringer_t *server; /* The storage node. */
//		stringer_t *header; /* The message header data. */
//		stringer_t *body; /* The message body data. */
//	} data;
//
// 	size_t total; /* The length of the raw message data. */
//	pthread_rwlock_t lock;

/**
 * @brief	Free a message object and its underlying data.
 * @return	This function returns no value.
 */
void message_free(message_t *message) {

	rwlock_destroy(&(message->lock));
	st_cleanup(message->data.header);
	st_cleanup(message->data.body);
	mm_cleanup(message);

	return;
}

/**
 * @brief	Allocate a new message object.
 * @param	messagenum	the numerical message id of the new message.
 * @param	created		the message creation timestamp.
 * @param	signature	the message's spam filter signature number.
 * @param	key			the message's spam filter access key.
 * @param	flags		the message's flags value.
 * @param	server		a managed string containing the name of the server where the message will be stored.
 * @param	size		the size, in bytes, of the raw message data.
 * @return	NULL on failure, or the newly allocated message object on success.
 */
message_t * message_alloc(uint64_t messagenum, uint64_t created, uint64_t signature, uint64_t key, uint64_t flags, stringer_t *server, size_t size) {

	message_t *result;

	if (!(result = mm_alloc(align(16, sizeof(message_t) + sizeof(placer_t)) + align(8, st_length_get(server) + 1)))) {
		log_pedantic("Unable to allocate %zu bytes for a message structure.", align(16, sizeof(message_t) + sizeof(placer_t)) + align(8, st_length_get(server) + 1));
		return NULL;
	}
	else if (rwlock_init(&(result->lock), NULL) != 0) {
		log_pedantic("Unable to initialize the message lock.");
		mm_free(result);
		return NULL;
	}

	result->message.num = messagenum;
	result->message.created = created;
	result->signature.num = signature;
	result->signature.key = key;
	result->status.flags = flags;
	result->size = size;

	// QUESTION: Messy?
	result->data.server = (placer_t *)((chr_t *)result + sizeof(message_t));
	((placer_t *)result->data.server)->opts = PLACER_T | JOINTED | STACK | FOREIGNDATA;
	((placer_t *)result->data.server)->length = st_length_get(server);
	((placer_t *)result->data.server)->data = (chr_t *)result + align(16, sizeof(message_t) + sizeof(placer_t));
	mm_copy(st_data_get(result->data.server), st_data_get(server), st_length_get(result->data.server));

	return result;
}

/**
 * @brief	Fetch all of a user's message folders and their child messages from the database.
 * @param	usernum		the numerical id of the target user.
 * @return	NULL on failure or an inx object holding all the retrieved and populated message folder objects on success.
 */
inx_t * messages_update(uint64_t usernum) {

	inx_t *folders;
	inx_cursor_t *cursor;
	message_folder_t *active;

	// If the fetch attempt fails, don't free the existing message folder index.
	if (!(folders = magma_folder_fetch(usernum, M_FOLDER_MESSAGES)) || !(cursor = inx_cursor_alloc(folders))) {
		inx_cleanup(folders);
		return NULL;
	}

	while ((active = inx_cursor_value_next(cursor))) {

		if (!meta_data_fetch_folder_messages(usernum, active)) {
			log_pedantic("Unable to load the user's messages. { usernum = %lu / folder = %lu }", usernum, active->foldernum);
			inx_cursor_free(cursor);
			inx_free(folders);
			return NULL;
		}

	}

	inx_cursor_free(cursor);

	return folders;
}

