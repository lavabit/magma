
/**
 * @file /magma/servers/imap/flags.c
 *
 * @brief 	Functions to handle IMAP commands/actions.
 */

#include "magma.h"

uint32_t imap_get_flag(stringer_t *string) {

	uint32_t result = 0;

	if (!string) {
		return 0;
	}

	// Loop and return the correct value.
	if (!st_cmp_ci_eq(string, PLACER("\\Recent", 7))) {
		result = MAIL_STATUS_RECENT;
	}
	else if (!st_cmp_ci_eq(string, PLACER("\\Seen", 5))) {
		result = MAIL_STATUS_SEEN;
	}
	else if (!st_cmp_ci_eq(string, PLACER("\\Answered", 9))) {
		result = MAIL_STATUS_ANSWERED;
	}
	else if (!st_cmp_ci_eq(string, PLACER("\\Flagged", 8))) {
		result = MAIL_STATUS_FLAGGED;
	}
	else if (!st_cmp_ci_eq(string, PLACER("\\Deleted", 8))) {
		result = MAIL_STATUS_DELETED;
	}
	else if (!st_cmp_ci_eq(string, PLACER("\\Draft", 5))) {
		result = MAIL_STATUS_DRAFT;
	}

	return result;
}

int_t imap_flag_action(stringer_t *string) {

	if (!st_cmp_ci_eq(string, PLACER("FLAGS", 5))) {
		return IMAP_FLAG_REPLACE;
	}
	else if (!st_cmp_ci_eq(string, PLACER("+FLAGS", 6))) {
		return IMAP_FLAG_ADD;
	}
	else if (!st_cmp_ci_eq(string, PLACER("-FLAGS", 6))) {
		return IMAP_FLAG_REMOVE;
	}
	else if (!st_cmp_ci_eq(string, PLACER("FLAGS.SILENT", 12))) {
		return IMAP_FLAG_REPLACE + IMAP_FLAG_SILENT;
	}
	else if (!st_cmp_ci_eq(string, PLACER("+FLAGS.SILENT", 13))) {
		return IMAP_FLAG_ADD + IMAP_FLAG_SILENT;
	}
	else if (!st_cmp_ci_eq(string, PLACER("-FLAGS.SILENT", 13))) {
		return IMAP_FLAG_REMOVE + IMAP_FLAG_SILENT;
	}

	return 0;
}

uint32_t imap_flag_parse(void *ptr, int_t type) {

	size_t number;
	imap_arguments_t *array;
	uint32_t current, flags = 0;

	// An empty () flag set was passed.
	if (ptr == NULL) {
		return MAIL_STATUS_EMPTY;
	}

	// A single flag was passed.
	if (type != IMAP_ARGUMENT_TYPE_ARRAY) {
		flags = imap_get_flag((stringer_t *)ptr);
	}
	// An array of flags was passed.
	else {
		array = (imap_arguments_t *)ptr;
		// Handle the special case where the flag empty set is passed in.
		if ((number = ar_length_get(array)) == 0) {
			flags = MAIL_STATUS_EMPTY;
		}

		for (size_t i = 0; i < number; i++) {

			// We should never get a nested array of flags.
			if (imap_get_type_ar(array, i) == IMAP_ARGUMENT_TYPE_ARRAY) {
				return MAIL_STATUS_EMPTY;
			}

			// Unrecognized flag.
			if ((current = imap_get_flag(imap_get_st_ar(array, i))) == 0) {
				return MAIL_STATUS_EMPTY;
			}

			// If we return the same flag twice.
			if ((current & flags) != 0) {
				return MAIL_STATUS_EMPTY;
			}

			flags += current;
		}
	}

	return flags;
}

void imap_update_flags(meta_user_t *user, inx_t *messages, uint64_t foldernum, int_t action, uint32_t flags) {

	inx_cursor_t *cursor;
	meta_message_t *active;

	uint32_t complete = (MAIL_STATUS_EMPTY | MAIL_STATUS_RECENT | MAIL_STATUS_SEEN | MAIL_STATUS_ANSWERED | MAIL_STATUS_FLAGGED | MAIL_STATUS_DELETED | MAIL_STATUS_DRAFT);

	if (user == NULL || messages == NULL || foldernum == 0 || action == 0 || flags == 0) {
		log_error("Sanity check failed, passed an invalid parameter.");
		return;
	}

	// Update the database first.
	if ((action & IMAP_FLAG_ADD) == IMAP_FLAG_ADD) {
		meta_data_flags_add(messages, user->usernum, foldernum, flags);
	}
	else if ((action & IMAP_FLAG_REMOVE) == IMAP_FLAG_REMOVE) {
		meta_data_flags_remove(messages, user->usernum, foldernum, flags);
	}
	else if ((action & IMAP_FLAG_REPLACE) == IMAP_FLAG_REPLACE) {
		meta_data_flags_replace(messages, user->usernum, foldernum, flags);
	}
	else {
		log_error("Invalid flag update action.");
		return;
	}

	// Update the messages structure.
	if (messages && (cursor = inx_cursor_alloc(messages))) {
		while ((active = inx_cursor_value_next(cursor))) {
			if (active->foldernum == foldernum) {
				if ((action & IMAP_FLAG_ADD) == IMAP_FLAG_ADD) {
					active->status = active->status | flags;
				}
				else if ((action & IMAP_FLAG_REMOVE) == IMAP_FLAG_REMOVE) {
					active->status = (active->status | flags) ^ flags;
				}
				else if ((action & IMAP_FLAG_REPLACE) == IMAP_FLAG_REPLACE) {
					active->status = ((active->status | complete) ^ complete) | flags;
				}
			}
		}

		inx_cursor_free(cursor);
	}

	return;
}
