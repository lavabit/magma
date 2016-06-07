
/**
 * @file /magma/servers/pop/sessions.c
 *
 * @brief	Functions for managing POP3 sessions.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Reset a POP3 session by guaranteeing that no messages are flagged to be deleted.
 * @param	con		the POP3 client connection issuing the command.
 * @return	-1 on general error, 0 if the connection hasn't been authenticated, or 1 if all messages have had their statuses successfully reset.
 */
int_t pop_session_reset(connection_t *con) {

	inx_cursor_t *cursor;
	meta_message_t *active;

	if (con->pop.session_state != 1 || !con->pop.user) {
		return 0;
	}

	new_meta_user_wlock(con->pop.user);

	if (con->pop.user->messages && (cursor = inx_cursor_alloc(con->pop.user->messages))) {

		// Loop through and remove the deleted flag.
		while ((active = inx_cursor_value_next(cursor))) {

			if ((active->status & MAIL_STATUS_HIDDEN) == MAIL_STATUS_HIDDEN) {
				active->status -= MAIL_STATUS_HIDDEN;
			}

		}

		inx_cursor_free(cursor);
	}

	new_meta_user_unlock(con->pop.user);

	return 1;
}

/**
 * @brief	Destroy a POP3 session and delete any flagged messages belonging to the user.
 * @param	con		the POP3 client connection issuing the command.
 * @brief	This function returns no value.
 */
void pop_session_destroy(connection_t *con) {

	bool_t deleted = false;
	inx_cursor_t *cursor;
	meta_message_t *active;
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = 0 };

	// Is there a user session?
	if (con->pop.user) {

		// If it was a clean exit, and this is the only pop session, delete messages marked for deletion.
		if (con->pop.expunge) {

			new_meta_user_wlock(con->pop.user);

			if (con->pop.user->messages && (cursor = inx_cursor_alloc(con->pop.user->messages))) {

				while ((active = inx_cursor_value_next(cursor))) {

					if ((active->status & MAIL_STATUS_HIDDEN) == MAIL_STATUS_HIDDEN) {
						mail_remove_message(con->pop.user->usernum, active->messagenum, active->size, active->server);
						key.val.u64 = active->messagenum;
						inx_delete(con->pop.user->messages, key);
						deleted = true;
					}

				}

				if (deleted) {
					meta_messages_update_sequences(con->pop.user->folders, con->pop.user->messages);
					con->pop.user->serials.messages = serial_increment(OBJECT_MESSAGES, con->pop.user->usernum);
				}

				inx_cursor_free(cursor);
			}

			new_meta_user_unlock(con->pop.user);
		}

		// Check whether we're the last reference to this object.
		if (con->pop.usernum) {
			new_meta_inx_remove(con->pop.usernum, META_PROTOCOL_POP);
		}

	}

	st_cleanup(con->pop.username);
	con->pop.usernum = 0;
	mail_cache_reset();
	return;
}
