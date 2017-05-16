
/**
 * @file /magma/servers/imap/sessions.c
 *
 * @brief IMAP session handlers.
 */

#include "magma.h"

/**
 * @brief Returns 0 if the selected folder wasn't modified, or 1 if things changed and the updated status should be sent to
 * the client, and a -1 is used to indicate the update check encountered a problem and should be retried later.
 */
int_t imap_session_update(connection_t *con) {

	int_t result = 0;
	inx_cursor_t *cursor;
	meta_message_t *active;
	uint64_t recent = 0, exists = 0, checkpoint;

	// Check for the right state.
	if (con->imap.session_state != 1 || con->imap.user == NULL || con->imap.selected == 0) {
		return -1;
	}

	if ((checkpoint = serial_get(OBJECT_USER, con->imap.user->usernum)) != con->imap.user_checkpoint) {
		meta_user_wlock(con->imap.user);

		// Update the user preferences.
		if (checkpoint != con->imap.user->serials.user) {
			meta_update_user(con->imap.user, META_LOCKED);
		}

		// Store the new checkpoint.
		con->imap.user_checkpoint = con->imap.user->serials.user;
		meta_user_unlock(con->imap.user);
	}

	if ((checkpoint = serial_get(OBJECT_FOLDERS, con->imap.user->usernum)) != con->imap.folders_checkpoint) {
		meta_user_wlock(con->imap.user);

	   // Update the list of folders.
		if (checkpoint != con->imap.user->serials.folders) {
			meta_update_folders(con->imap.user, META_LOCKED);
		}

		// Store the new checkpoint.
		con->imap.folders_checkpoint = con->imap.user->serials.folders;
		meta_user_unlock(con->imap.user);
	}

	if ((checkpoint = serial_get(OBJECT_MESSAGES, con->imap.user->usernum)) != con->imap.messages_checkpoint) {
		meta_user_wlock(con->imap.user);

		if (checkpoint != con->imap.user->serials.messages) {
			meta_messages_update(con->imap.user, META_LOCKED);
		}

		// If there is a selected folder, scan the status.
		if ((cursor = inx_cursor_alloc(con->imap.user->messages))) {

			while ((active = inx_cursor_value_next(cursor))) {

				if (active->foldernum == con->imap.selected && (active->status & MAIL_STATUS_RECENT) == MAIL_STATUS_RECENT) {
					recent++;
					exists++;
				}
				else if (active->foldernum == con->imap.selected) {
					exists++;
				}

			}

			inx_cursor_free(cursor);
		}

		// If the folder has changed, output the current status.
		if (con->imap.messages_recent != recent || con->imap.messages_total != exists) {
			con->imap.messages_recent = recent;
			con->imap.messages_total = exists;
			result = 1;
		}

		// Store the new checkpoint.
		con->imap.messages_checkpoint = con->imap.user->serials.messages;

		meta_user_unlock(con->imap.user);
	}

	return result;
}

void imap_session_destroy(connection_t *con) {

	inx_cursor_t *cursor;
	meta_message_t *active;

	meta_user_wlock(con->imap.user);

	// If a folder was selected, clear the recent flag before closing the mailbox.
	if (con->imap.session_state == 1 && con->imap.user && con->imap.selected && !con->imap.read_only &&
		(cursor = inx_cursor_alloc(con->imap.user->messages))) {

		while ((active = inx_cursor_value_next(cursor))) {

			if (active->foldernum == con->imap.selected && (active->status & MAIL_STATUS_RECENT) == MAIL_STATUS_RECENT) {
				active->status = (active->status | MAIL_STATUS_RECENT) ^ MAIL_STATUS_RECENT;
			}

		}

		inx_cursor_free(cursor);
	}

	meta_user_unlock(con->imap.user);

	// Is there a user session.
	if (con->imap.user && con->imap.usernum) {
		meta_inx_remove(con->imap.usernum, META_PROTOCOL_IMAP);
	}

	// Free the username, and reset the usernum.
	st_cleanup(con->imap.username);
	con->imap.username = NULL;
	con->imap.usernum = 0;

	// Free the tag.
	st_cleanup(con->imap.tag);
	con->imap.tag = NULL;

	// Free the command.
	st_cleanup(con->imap.command);
	con->imap.command = NULL;

	// Free the arguments array.
	if (con->imap.arguments) {
		ar_free(con->imap.arguments);
		con->imap.arguments = NULL;
	}

	mail_cache_reset();

	return;
}
