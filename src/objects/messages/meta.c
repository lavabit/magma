
/**
 * @file /magma/objects/messages/meta.c
 *
 * @brief	Functions to interface with the deprecated meta_message_t structure.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"


/**
 * @brief	Free a meta message object (and its tags).
 * @return	This function returns no value.
 */
void meta_message_free(meta_message_t *message) {

	if (message) {

		if (message->tags) {
			ar_free(message->tags);
		}

		mm_free(message);
	}

	return;
}

/**
 * @brief	Duplicate a meta message object (along with a deep copy of its tags).
 * @param	message		the meta message object to be cloned.
 * @return	NULL on failure or a pointer to the duplicated meta message object on success.
 */
meta_message_t * meta_message_dupe(meta_message_t *message) {

	meta_message_t *result = NULL;

	if (message && (result = mm_dupe(message, sizeof(meta_message_t)))) {

		if (message->tags) {
			result->tags = ar_dupe(message->tags);
		}

	}

	return result;
}

/**
 * @brief	Retrieve a message by number.
 * @param	messages	a pointer to the messages collection to be searched.
 * @param	number		the number of the message to be retrieved.
 * @return	NULL on failure, or a pointer to the meta message object of the matching mail message.
 */
meta_message_t * meta_message_by_number(inx_t *messages, uint64_t number) {

	inx_cursor_t *cursor;
	meta_message_t *result = NULL, *active;

	if (!(cursor = inx_cursor_alloc(messages))) {
		return NULL;
	}

	while ((active = inx_cursor_value_next(cursor)) && !result) {

		if (active->messagenum == number) {
			result = active;
		}

	}

	inx_cursor_free(cursor);

	return result;
}

/**
 * @brief	Update the sequence numbers of a series of messages, each re-indexed by their containing folder.
 * @note	All messages will be sequenced incrementally per folder, starting with a value of 1.
 * @param	folders		an inx holder containing all of the user's meta folder records.
 * @param	messages	an inx folder containing all the messages to be re-sequenced.
 * @return	This function returns no value.
 */
void meta_messages_update_sequences(inx_t *folders, inx_t *messages) {

	uint64_t sequence;
	meta_folder_t *folder;
	meta_message_t *message;
	inx_cursor_t *cursor_messages, *cursor_folders;

	if (!folders || !messages || !(cursor_folders = inx_cursor_alloc(folders))) {
		return;
	}

	// Iterate through all of the messages for each folder and set their sequence number.
	while ((folder = inx_cursor_value_next(cursor_folders)) && (sequence = 1)) {

		if ((cursor_messages = inx_cursor_alloc(messages))) {

			while ((message = inx_cursor_value_next(cursor_messages))) {

				if (message->foldernum == folder->foldernum) {
					message->sequencenum = sequence++;
				}

			}

			inx_cursor_free(cursor_messages);
		}

	}

	inx_cursor_free(cursor_folders);

	return;
}

/**
 * @brief	Build a user's messages collection if it is empty, or needs to be refreshed (see note).
 * @note	This function will fetch and sequence the user's messages from the database if the meta user object has no messages,
 * 			or if the user has one or fewer pop sessions open and the messages serial number was stale.
 * @param	user	a pointer to the meta user object of the user requesting the messages update.
 * @param	locked	if set to META_NEED_LOCK, lock the specified meta user object for the duration of the request.
 * @return	true if no update was necessary or if the update was successful, or false otherwise.
 */
bool_t meta_messages_login_update(meta_user_t *user, META_LOCK_STATUS locked) {

	bool_t output = true;
	uint64_t checkpoint;

	if (!user) {
		return -1;
	}

	// Do we need a lock.
	if (locked == META_NEED_LOCK) {
		meta_user_wlock(user);
	}

	// This function will check if there is only one POP session.
	if (user->refs.pop <= 1 && user->messages && (checkpoint = serial_get(OBJECT_MESSAGES, user->usernum)) != user->serials.messages) {

		if (!(user->serials.messages = checkpoint)) {
			user->serials.messages = serial_increment(OBJECT_MESSAGES, user->usernum);
		}

		if ((output = meta_data_fetch_messages(user)) && user->folders) {
			meta_messages_update_sequences(user->folders, user->messages);
		}
	}

	// We need to build the messages table.
	else if (!user->messages) {

		if (!(user->serials.messages = serial_get(OBJECT_MESSAGES, user->usernum))) {
			user->serials.messages = serial_increment(OBJECT_MESSAGES, user->usernum);
		}

		if ((output = meta_data_fetch_messages(user)) && user->folders) {
			meta_messages_update_sequences(user->folders, user->messages);
		}
	}

	// Do we need to clear the lock.
	if (locked == META_NEED_LOCK) {
		meta_user_unlock(user);
	}

	return output;
}

/**
 * @brief	Refresh and resquence a user's message collection if it is stale.
 * @note	The user's messages will only be updated if they are empty or if they are out of sync and the user has no open pop sessions.
 * @see		meta_data_fetch_messages()
 * @param	user	a pointer to the meta user object requesting the messages update.
 * @param	locked	if set to META_NEED_LOCK, lock the specified meta user object for the duration of the request.
 * @return	-1 on failure, or 1 on success.
 */
int_t meta_messages_update(meta_user_t *user, META_LOCK_STATUS locked) {

	short output = 0;
	uint64_t checkpoint;

	if (!user) {
		return -1;
	}

	// Do we need a lock.
	if (locked == META_NEED_LOCK) {
		meta_user_wlock(user);
	}

	// If there are no POP sessions, the checkpoint is more than 60 seconds old, and the memcache checkpoint is newer, refresh.
	if (!user->refs.pop && user->messages && (checkpoint = serial_get(OBJECT_MESSAGES, user->usernum)) != user->serials.messages) {

		if ((user->serials.messages = checkpoint) == 0) {
			user->serials.messages = serial_increment(OBJECT_MESSAGES, user->usernum);
		}

		if ((output = meta_data_fetch_messages(user)) && user->folders) {
			meta_messages_update_sequences(user->folders, user->messages);
		}
	}

	// We need to build the messages table.
	else if (!user->messages) {

		if ((user->serials.messages = serial_get(OBJECT_MESSAGES, user->usernum)) == 0) {
			user->serials.messages = serial_increment(OBJECT_MESSAGES, user->usernum);
		}

		if ((output = meta_data_fetch_messages(user)) && user->folders) {
			meta_messages_update_sequences(user->folders, user->messages);
		}

	}

	// Do we need to clear the lock.
	if (locked == META_NEED_LOCK) {
		meta_user_unlock(user);
	}

	return output;
}

/**
 * @param	Make a copy of a mail message, in the database and in memory.
 * @see		mail_copy_message()
 * @note	The message copy will not have the deleted, hidden, or recent flags set in the database, but it will have the recent flag set in memory.
 * @param	user		a pointer to the meta user object to whom the message belong.
 * @param	message		the meta message object of the message to be copied.
 * @param	target		the numerical id of the folder to which the message will be copied.
 * @param	outnum		a pointer to an unsigned 64-bit integer that will receive the value of the numerical id of the message copy.
 * @param	sequences	if true, resequence the message folders after the copy has been made.
 * @param	locked		if set to META_NEED_LOCK, lock the specified meta user object for the duration of the request.
 * @return	true on success or false on failure.
 */
bool_t meta_messages_copier(meta_user_t *user, meta_message_t *message, uint64_t target, uint64_t *outnum, bool_t sequences, META_LOCK_STATUS locked) {

	uint32_t status;
	meta_message_t *new;
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = 0 };

	if (!user || !message) {
		return false;
	}

	// Make sure the deleted/hidden/recent flags are not set.
	status = (message->status | MAIL_STATUS_DELETED | MAIL_STATUS_HIDDEN | MAIL_STATUS_RECENT) ^ (MAIL_STATUS_DELETED | MAIL_STATUS_HIDDEN | MAIL_STATUS_RECENT);

	// Do we need a lock.
	if (locked == META_NEED_LOCK) {
		meta_user_wlock(user);
	}

	if (!(key.val.u64 = mail_copy_message(user->usernum, message->messagenum, message->server, message->size, target, status, message->signum,
		message->sigkey, message->created))) {
		log_pedantic("Unable to copy message number %lu.", message->messagenum);

		if (locked == META_NEED_LOCK) {
			meta_user_unlock(user);
		}

		return false;
	}

	if (!(new = meta_message_dupe(message))) {
		log_pedantic("Unable to duplicate the message structure.");

		if (locked == META_NEED_LOCK) {
			meta_user_unlock(user);
		}

		return false;
	}

	*outnum = new->messagenum = key.val.u64;
	new->foldernum = target;

	// Messages added to a folder should be distinguished by having the recent flag.
	new->status = status | MAIL_STATUS_RECENT;

	if (message->size != new->size) {
		log_pedantic("orig->size = %zu && new->size = %zu", message->size, new->size);
	}

	if (!inx_insert(user->messages, key, new)) {
		log_error("Failed to insert message copy into user's messages.");
		mm_free(new);
	}

	// If this operation is part of a much larger one we might want to wait until the end to update the message sequence numbers.
	if (sequences) {
		meta_messages_update_sequences(user->folders, user->messages);
	}

	if (locked == META_NEED_LOCK) {
		meta_user_unlock(user);
	}

	return true;
}

/**
 * @param	Move a mail message to another folder.
 * @see		mail_move_message()
 * @note	The message will receive a copy of the recent flag in memory.
 * @param	user		a pointer to the meta user object to whom the message belong.
 * @param	message		the meta message object of the message to be moved.
 * @param	target		the numerical id of the folder to which the message will be copied.
 * @param	lookup		if set, verify the existence of the moved message in the user's messages.
 * @param	sequences	if true, resequence the message folders after the copy has been made.
 * @param	locked		if set to META_NEED_LOCK, lock the specified meta user object for the duration of the request.
 * @return	true on success or false on failure.
 */
int_t meta_messages_mover(meta_user_t *user, meta_message_t *message, uint64_t target, bool_t lookup, bool_t sequences, META_LOCK_STATUS locked) {

	int_t result = -1;
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = 0 };

	if (!user || !message) {
		return -1;
	}

	// Do we need a lock.
	if (locked == META_NEED_LOCK) {
		meta_user_wlock(user);
	}

	/// BUG: Currently we preserve the original message number which means when the target folder sequences are updated the moved message
	/// can appear anywhere in the list (since its based on message number). Inserting messages into the sequence list in this fashion will
	/// probably break clients. We may want to duplicate the row  which would generate a new message number causing the newly added message to
	/// appear at the end of the folder listing. On the other hand POP ignores folders completely, so maybe it won't be quite so bad.
	if (!(key.val.u64 = message->messagenum) || (result = mail_move_message(user->usernum, message->messagenum, message->foldernum, target)) != 1) {
		log_pedantic("Unable to move message number. { message = %lu }", key.val.u64);

		if (locked == META_NEED_LOCK) {
			meta_user_unlock(user);
		}

		return result;
	}

	if (lookup && !(message = inx_find(user->messages, key))) {
		log_pedantic("Unable to lookup the master message meta context. { message = %lu }", key.val.u64);

		if (locked == META_NEED_LOCK) {
			meta_user_unlock(user);
		}

		return -1;
	}

	// Update the message context so it uses the new folder.
	message->foldernum = target;

	// New messages in a folder should be distinguished by the recent flag.
	message->status |= MAIL_STATUS_RECENT;

	// If this operation is part of a much larger one we might want to wait until the end to update the message sequence numbers.
	if (sequences) {
		meta_messages_update_sequences(user->folders, user->messages);
	}

	if (locked == META_NEED_LOCK) {
		meta_user_unlock(user);
	}

	return 1;
}
