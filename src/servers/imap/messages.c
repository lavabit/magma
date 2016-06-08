
/**
 * @file /magma/servers/imap/messages.c
 *
 * @brief	Functions used to handle IMAP commands and actions.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

int_t imap_append_message(connection_t *con, meta_folder_t *folder, uint32_t flags, stringer_t *message, uint64_t *outnum) {

	meta_message_t *new;
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = 0 };
	stringer_t *pubkey = ((con->imap.user->flags & META_USER_ENCRYPT_DATA) == META_USER_ENCRYPT_DATA) ? con->imap.user->keys.public : NULL;

	// Always add the recent and appended flags to these messages.
	flags = (flags | MAIL_STATUS_RECENT | MAIL_STATUS_APPENDED);

	if ((key.val.u64 = mail_store_message(con->imap.user->usernum, pubkey, folder->foldernum, &flags, 0, 0, message)) == 0) {
		log_pedantic("Unable to append a message of %zu bytes.", st_length_get(message));
		return 0;
	}

	if ((new = mm_alloc(sizeof(meta_message_t))) == NULL) {
		log_pedantic("Unable to allocate %zu bytes for a message structure.", sizeof(meta_message_t));
		return 0;
	}

	*outnum = new->messagenum = key.val.u64;
	new->status = flags;
	new->foldernum = folder->foldernum;
	new->created = time(NULL);
	new->size = st_length_get(message);
	snprintf(new->server, 33, "%.*s", st_length_int(magma.storage.active), st_char_get(magma.storage.active));

	if (inx_insert(con->imap.user->messages, key, new) != true) {
		mm_free(new);
	}

	meta_messages_update_sequences(con->imap.user->folders, con->imap.user->messages);

	// Update the checkpoint, so other connections know things have changed.
	if (con->imap.user->serials.messages != serial_get(OBJECT_MESSAGES, con->imap.user->usernum)) {
		con->imap.messages_checkpoint = con->imap.user->serials.messages = serial_increment(OBJECT_MESSAGES, con->imap.user->usernum) - 1;
	}
	else {
		con->imap.messages_checkpoint = con->imap.user->serials.messages = serial_increment(OBJECT_MESSAGES, con->imap.user->usernum);
	}

	return 1;
}

int_t imap_message_expunge(connection_t *con, meta_message_t *message) {

	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = (message && message->messagenum ? message->messagenum : 0) };

	if (!mail_remove_message(con->imap.user->usernum, message->messagenum, message->size, message->server)) {
		return 0;
	}

	inx_delete(con->imap.user->messages, key);
	return 1;
}

int_t imap_message_copier(connection_t *con, meta_message_t *message, uint64_t target, uint64_t *outnum) {

	uint32_t status;
	meta_message_t *new;
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = 0 };

	status = message->status;

	// Make sure the deleted/hidden/recent flags are not set.
	status = (status | MAIL_STATUS_DELETED | MAIL_STATUS_HIDDEN | MAIL_STATUS_RECENT) ^ (MAIL_STATUS_DELETED | MAIL_STATUS_HIDDEN | MAIL_STATUS_RECENT);

	if ((key.val.u64 = mail_copy_message(con->imap.user->usernum, message->messagenum, message->server, message->size, target, status, message->signum,
		message->sigkey, message->created)) == 0) {
		log_pedantic("Unable to copy message number %lu.", message->messagenum);
		return 0;
	}

	if (!(new = meta_message_dupe(message))) {
		log_pedantic("Unable to duplicate the message structure.");
		return 0;
	}

	*outnum = new->messagenum = key.val.u64;
	new->foldernum = target;

	// Messages added to a folder should be distinguished by having the recent flag.
	new->status = status | MAIL_STATUS_RECENT;

	if (message->size != new->size) {
		log_pedantic("orig->size = %zu && new->size = %zu", message->size, new->size);
	}

	if (inx_insert(con->imap.user->messages, key, new) != true) {
		mm_free(new);
	}

	meta_messages_update_sequences(con->imap.user->folders, con->imap.user->messages);

	// If the serial number indicates no outside changes we can increment the checkpoint and store the value. Otherwise we just increment it
	// so a full refresh will be triggered.
	if (con->imap.user->serials.messages == serial_get(OBJECT_MESSAGES, con->imap.user->usernum)) {
		con->imap.messages_checkpoint = con->imap.user->serials.messages = serial_increment(OBJECT_MESSAGES, con->imap.user->usernum);
	}
	// The context is already due for a refresh, but we increment the serial to let the rest of the cluster know about the change.
	else {
		serial_increment(OBJECT_MESSAGES, con->imap.user->usernum);
	}

	return 1;
}
