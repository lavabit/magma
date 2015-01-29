
/**
 * @file /magma/servers/pop/mailbox.c
 *
 * @brief	Utility functions used to retrieve POP3 message-related statistics.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Get the number of messages available to a POP3 user.
 * @note	This function only counts messages that aren't deleted or hidden, and weren't created by the IMAP APPEND command.
 * @param	messages	an inx holder containing a collection of messages to be analyzed.
 * @return	the total number of messages available to the POP3 user, or 0 on failure.
 */
uint64_t pop_total_messages(inx_t *messages) {

	uint64_t result = 0;
	inx_cursor_t *cursor;
	meta_message_t *active;

	if (!messages || !(cursor = inx_cursor_alloc(messages))) {
		return 0;
	}

	while ((active = inx_cursor_value_next(cursor))) {

		if (!(active->status & (MAIL_STATUS_APPENDED | MAIL_STATUS_HIDDEN))) {
			result++;
		}

	}

	inx_cursor_free(cursor);

	return result;
}

/**
 * @brief	Get the total size of all messages available to a POP3 user.
 * @note	This function only counts messages that aren't deleted or hidden, and weren't created by the IMAP APPEND command.
 * @param	messages	an inx holder containing a collection of messages to be analyzed.
 * @return	the total size, in bytes, of all messages available to the POP3 user, or 0 on failure.
 */
uint64_t pop_total_size(inx_t *messages) {

	uint64_t result = 0;
	inx_cursor_t *cursor;
	meta_message_t *active;

	if (!messages || !(cursor = inx_cursor_alloc(messages))) {
		return 0;
	}

	while ((active = inx_cursor_value_next(cursor))) {

		if (!(active->status & (MAIL_STATUS_APPENDED | MAIL_STATUS_HIDDEN))) {
			result += active->size;
		}

	}

	inx_cursor_free(cursor);

	return result;
}

/**
 * @brief	Get the POP3 sequence number of the last message that isn't flagged as recent.
 * @note	This function only counts messages that aren't deleted or hidden, and weren't created by the IMAP APPEND command.
 * @param	messages	an inx holder containing a collection of messages to be analyzed.
 * @return	the sequence number of the last message that isn't flagged as recent, or 0 on failure.
 */
uint64_t pop_get_last(inx_t *messages) {

	uint64_t result = 1;
	inx_cursor_t *cursor;
	meta_message_t *active;

	if (!messages || !(cursor = inx_cursor_alloc(messages))) {
		return 0;
	}

	while ((active = inx_cursor_value_next(cursor))) {

		// Only count messages that aren't appened.
		if (!(active->status & MAIL_STATUS_APPENDED)) {

			// If the message has the recent flag, but doesn't have the hidden flag.
			if ((active->status & (MAIL_STATUS_RECENT | MAIL_STATUS_HIDDEN)) == MAIL_STATUS_RECENT) {
				inx_cursor_free(cursor);
				return result;
			}
			else {
				result++;
			}

		}

	}

	inx_cursor_free(cursor);

	return result;
}

/**
 * @brief	Get a message by its pop sequence number.
 * @param	messages	an inx holder containing the collection of the user's messages to be traversed.
 * @param	number		the zero-based pop sequence number of the message to be retrieved.
 * @return	NULL on failure or the meta message object of the message if it was found.
 */
meta_message_t * pop_get_message(inx_t *messages, uint64_t get) {

	uint64_t count = 1;
	inx_cursor_t *cursor;
	meta_message_t *active;

	if (!messages || !(cursor = inx_cursor_alloc(messages))) {
		return NULL;
	}

	while ((active = inx_cursor_value_next(cursor))) {

		if (active->status & MAIL_STATUS_APPENDED) {
			continue;
		}

		if (count == get) {
			inx_cursor_free(cursor);
			return active;
		}

		count++;
	}

	inx_cursor_free(cursor);

	return NULL;
}
