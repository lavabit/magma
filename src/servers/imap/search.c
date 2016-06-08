
/**
 * @file /magma/servers/imap/search.c
 *
 * @brief	Functions used to handle IMAP commands/actions.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

chr_t *MONTH_LOOKUP[] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN", "JUL", "AUG", "SEP", "OCT", "NOV", "DEC" };

int_t imap_search_messages_date_compare(stringer_t *one, stringer_t *two) {

	uint32_t numone, numtwo;
	placer_t partone, parttwo;

	if (one == NULL || two == NULL || tok_get_count_st(one, '-') != 3 || tok_get_count_st(two, '-') != 3) {
		return -1;
	}

	// Year
	if (tok_get_st(one, '-', 2, &partone) < 0 || tok_get_st(two, '-', 2, &parttwo) < 0) {
		return -1;
	}

	if (uint32_conv_st(&partone, &numone) != true || uint32_conv_st(&parttwo, &numtwo) != true) {
		return -1;
	}

	if (numone < numtwo) {
		return 0;
	}
	else if (numone > numtwo) {
		return 2;
	}

	// Month
	if (tok_get_st(one, '-', 1, &partone) < 0 || tok_get_st(two, '-', 1, &parttwo) < 0) {
		return -1;
	}

	numtwo = numone = 0;
	for (int_t i = 0; i < 12 && (numone == 0 || numtwo == 0); i++) {
		if (!st_cmp_ci_eq(&partone, PLACER(MONTH_LOOKUP[i], 3))) {
			numone = i + 1;
		}
		if (!st_cmp_ci_eq(&parttwo, PLACER(MONTH_LOOKUP[i], 3))) {
			numtwo = i + 1;
		}
	}

	if (numone == 0 || numtwo == 0) {
		return -1;
	}

	if (numone < numtwo) {
		return 0;
	}
	else if (numone > numtwo) {
		return 2;
	}

	// Day
	if (tok_get_st(one, '-', 0, &partone) < 0 || tok_get_st(two, '-', 0, &parttwo) < 0) {
		return -1;
	}

	if (uint32_conv_st(&partone, &numone) != true || uint32_conv_st(&parttwo, &numtwo) != true) {
		return -1;
	}

	if (numone < numtwo) {
		return 0;
	}
	else if (numone > numtwo) {
		return 2;
	}

	return 1;
}

int_t imap_search_messages_date(new_meta_user_t *user, mail_message_t **data, stringer_t **header, meta_message_t *active, stringer_t *date, int_t internal, int_t expected) {

	time_t utime;
	struct tm ltime;
	int_t compare = -1;
	chr_t date_string[40];
	placer_t area = pl_null(), front, mid, end;
	stringer_t *current = NULL, *line = NULL;

	// Use the internal date.
	if (internal == 1) {

		// Build a string with the current date.
		if ((utime = active->created) == 0 || localtime_r(&utime, &ltime) == NULL || strftime(date_string, 40, "%d-%b-%Y", &ltime) <= 0) {
			compare = -1;
		}
		else if ((current = st_import(date_string, ns_length_get(date_string))) == NULL) {
			compare = -1;
		}

		// Compare
		else {
			compare = imap_search_messages_date_compare(current, date);
		}
	}
	// Use the date field from the message header.
	else {

		// Load the message, if necessary.
		if (*data != NULL) {
			area = pl_init(st_char_get((*data)->text), (*data)->header_length);
		}
		else if (*header != NULL) {
			area = pl_init(st_char_get(*header), st_length_get(*header));
		}
		else if ((*header = mail_load_header(active, user)) != NULL){
			area = pl_init(st_char_get(*header), st_length_get(*header));
		}

		// Get the DATE line of the header.
		if (pl_empty(area) || (line = mail_header_fetch_cleaned(&area, PLACER("Date", 4))) == NULL) {
			compare = -1;
		}

		// Build the date string.
		else if (tok_get_count_st(line, ',') > 1 && tok_get_count_st(line, ' ') > 4 &&
			(tok_get_st(line, ' ', 1, &front) < 0 || tok_get_st(line, ' ', 2, &mid) < 0 || tok_get_st(line, ' ', 3, &end) < 0 ||
			!(current = st_merge("snsns", &front, "-", &mid, "-",	&end)))) {
			compare = -1;
		}
		else if (tok_get_count_st(line, ',') == 1 && tok_get_count_st(line, ' ') > 3 &&
			(tok_get_st(line, ' ', 0, &front) < 0 || tok_get_st(line, ' ', 1, &mid) < 0 || tok_get_st(line, ' ', 2, &end) < 0 ||
			!(current = st_merge("snsns", &front, "-", &mid, "-", &end)))) {
			compare = -1;
		}

		// Compare
		else {
			compare = imap_search_messages_date_compare(current, date);
		}
	}

	// Cleanup
	if (line != NULL) {
		st_free(line);
	}
	if (current != NULL) {
		st_free(current);
	}

	// Did we get the expected value.
	if (compare == expected) {
		return 1;
	}

	return -1;
}

// Search the header.
int_t imap_search_messages_header(new_meta_user_t *user, mail_message_t **data, stringer_t **header, meta_message_t *active, stringer_t *field, stringer_t *value) {

	size_t location;
	int_t compare = -1;
	placer_t area = pl_null();
	stringer_t *current = NULL;

	// Load the message, if necessary.
	if (*data != NULL) {
		area = pl_init(st_char_get((*data)->text), (*data)->header_length);
	}
	else if (*header != NULL) {
		area = pl_init(st_char_get(*header), st_length_get(*header));
	}
	else if ((*header = mail_load_header(active, user)) != NULL){
		area = pl_init(st_char_get(*header), st_length_get(*header));
	}

	// Get the header value.
	if (pl_empty(area) || (current = mail_header_fetch_all(&area, field)) == NULL) {
		compare = -1;
	}

	// Search for the value inside the header line.
	else if (st_search_ci(current, value, &location) == 1) {
		compare = 1;
	}

	// Cleanup
	if (current != NULL) {
		st_free(current);
	}

	return compare;
}

int_t imap_search_messages_body(new_meta_user_t *user, mail_message_t **data, meta_message_t *active, stringer_t *value) {

	size_t location;
	int_t compare = -1;
	stringer_t *current = NULL;

	// Load the message, if necessary.
	if (*data == NULL && ((*data = mail_load_message(active, user, NULL, 0)) == NULL || mail_mime_update(*data) == 0)) {
		compare = -1;
	}

	// Search for the value inside the body.
	else if (st_search_ci(&((*data)->mime->body), value, &location) == 1) {
		compare = 1;
	}

	// Cleanup
	if (current != NULL) {
		st_free(current);
	}

	return compare;
}

int_t imap_search_messages_text(new_meta_user_t *user, mail_message_t **data, meta_message_t *active, stringer_t *value) {

	size_t location;
	int_t compare = -1;
	stringer_t *current = NULL;

	// Load the message, if necessary.
	if (*data == NULL && ((*data = mail_load_message(active, user, NULL, 0)) == NULL || mail_mime_update(*data) == 0)) {
		compare = -1;
	}

	// Search for the value inside the header line.
	else if (st_search_ci((*data)->text, value, &location) == 1) {
		compare = 1;
	}

	// Cleanup
	if (current != NULL) {
		st_free(current);
	}

	return compare;
}

int_t imap_search_messages_size(meta_message_t *active, stringer_t *value, int_t expected) {

	uint64_t size;

	if (active == NULL) {
		return -1;
	}

	if (uint64_conv_st(value, &size) != true) {
		return -1;
	}
	else if (expected == 0 && active->size < size) {
		return 1;
	}
	else if (expected == 1 && active->size > size) {
		return 1;
	}

	return -1;
}

int_t imap_search_flag(uint32_t status, uint32_t flag, int_t has) {
	if (has == 1 && (status & flag) == flag) {
		return 1;
	}
	else if (has == 0 && (status & flag) != flag) {
		return 1;
	}
	return -1;
}

int_t imap_search_messages_range(meta_message_t *active, stringer_t *range, int_t uid) {

	int_t asterisk;
	uint64_t start, end, number;
	uint32_t commas, parts;
	placer_t sequence, start_token, end_token;

	if (!active || !range) {
		return -1;
	}

	// Count the commas.
	commas = tok_get_count_st(range, ',');

	// Break apart each sequence section.
	for (uint32_t i = 0; i <= commas && tok_get_st(range, ',', i, &sequence) >= 0; i++) {

		start = end = asterisk = 0;
		start_token = end_token = pl_null();
		parts = tok_get_count_st(&sequence, ':');
		tok_get_st(&sequence, ':', 0, &start_token);

		if (parts > 1) {
			tok_get_st(&sequence, ':', 1, &end_token);
		}

		// Parse the start.
		if (pl_empty(start_token)) {
			return -1;
		} else if (*(pl_char_get(start_token)) == '*') {
			asterisk = 1;
		} else if (!uint64_conv_st(&start_token, &start)) {
				return -1;
		}

		// Parse the end.
		if (!pl_empty(end_token) && *(pl_char_get(end_token)) == '*') {
			asterisk = 1;
		}
		else if (pl_empty(end_token)) {
			end = start;
		} else {

			if (!uint64_conv_st(&end_token, &end)) {
				return -1;
			}

		}

		// If necessary, swap the values.
		if (asterisk == 1 && end != 0) {
			start = end;
		}
		else if (asterisk == 0 && start > end) {
			number = start;
			start = end;
			end = number;
		}

		// Loop through and collect the messages.
		if (uid == 0 && active->sequencenum >= start && (asterisk == 1 || active->sequencenum <= end)) {
			return 1;
		}
		else if (uid == 1 && active->messagenum >= start && (asterisk == 1 || active->messagenum <= end)) {
			return 1;
		}

	}

	return -1;
}

int_t imap_search_messages_inner(new_meta_user_t *user, mail_message_t **message, stringer_t **header, meta_message_t *current, imap_arguments_t *array, unsigned recursion) {

	stringer_t *item;
	unsigned number, increment = 0;
	int_t eval = 0, output = 0, not = 0, or = 0;

	// Recursion limiter.
	if (recursion >= IMAP_SEARCH_RECURSION_LIMIT) {
		log_pedantic("Recursion limit hit.");
		return 0;
	}

	// NULL arrays don't remove any messages.
	if (array == NULL || (number = ar_length_get(array)) == 0) {
		return 1;
	}

	while (increment < number && current != NULL && output != -1) {

		// Handle nested arrays.
		if (imap_get_type_ar(array, increment) == IMAP_ARGUMENT_TYPE_ARRAY) {
			eval = imap_search_messages_inner(user, message, header, current, imap_get_ar_ar(array, increment++), recursion + 1);
		}
		else if ((item = imap_get_st_ar(array, increment++)) == NULL) {
			eval = -1;
		}

		// Flag checks.
		else if (!st_cmp_ci_eq(item, PLACER("ANSWERED", 8))) {
			eval = imap_search_flag(current->status, MAIL_STATUS_ANSWERED, 1);
		}
		else if (!st_cmp_ci_eq(item, PLACER("DELETED", 7))) {
			eval = imap_search_flag(current->status, MAIL_STATUS_DELETED, 1);
		}
		else if (!st_cmp_ci_eq(item, PLACER("DRAFT", 5))) {
			eval = imap_search_flag(current->status, MAIL_STATUS_DRAFT, 1);
		}
		else if (!st_cmp_ci_eq(item, PLACER("FLAGGED", 7))) {
			eval = imap_search_flag(current->status, MAIL_STATUS_FLAGGED, 1);
		}
		else if (!st_cmp_ci_eq(item, PLACER("RECENT", 6))) {
			eval = imap_search_flag(current->status, MAIL_STATUS_RECENT, 1);
		}
		else if (!st_cmp_ci_eq(item, PLACER("SEEN", 4))) {
			eval = imap_search_flag(current->status, MAIL_STATUS_SEEN, 1);
		}
		else if (!st_cmp_ci_eq(item, PLACER("OLD", 3))) {
			eval = imap_search_flag(current->status, MAIL_STATUS_RECENT, 0);
		}
		else if (!st_cmp_ci_eq(item, PLACER("UNANSWERED", 10))) {
			eval = imap_search_flag(current->status, MAIL_STATUS_ANSWERED, 0);
		}
		else if (!st_cmp_ci_eq(item, PLACER("UNDELETED", 9))) {
			eval = imap_search_flag(current->status, MAIL_STATUS_DELETED, 0);
		}
		else if (!st_cmp_ci_eq(item, PLACER("UNDRAFT", 7))) {
			eval = imap_search_flag(current->status, MAIL_STATUS_DRAFT, 0);
		}
		else if (!st_cmp_ci_eq(item, PLACER("UNFLAGGED", 9))) {
			eval = imap_search_flag(current->status, MAIL_STATUS_FLAGGED, 0);
		}
		else if (!st_cmp_ci_eq(item, PLACER("UNSEEN", 6))) {
			eval = imap_search_flag(current->status, MAIL_STATUS_SEEN, 0);
		}
		else if (!st_cmp_ci_eq(item, PLACER("NEW", 3))) {
			if (imap_search_flag(current->status, MAIL_STATUS_RECENT, 1) == 1 && imap_search_flag(current->status, MAIL_STATUS_SEEN, 0) == 1) {
				eval = 1;
			}
			else {
				eval = -1;
			}
		}

		// Date checks.
		else if (increment < number && !st_cmp_ci_eq(item, PLACER("BEFORE", 6)) && imap_get_type_ar(array, increment) != IMAP_ARGUMENT_TYPE_ARRAY) {
			eval = imap_search_messages_date(user, message, header, current, imap_get_st_ar(array, increment++), 1, 0);
		}
		else if (increment < number && !st_cmp_ci_eq(item, PLACER("ON", 2)) && imap_get_type_ar(array, increment) != IMAP_ARGUMENT_TYPE_ARRAY) {
			eval = imap_search_messages_date(user, message, header, current, imap_get_st_ar(array, increment++), 1, 1);
		}
		else if (increment < number && !st_cmp_ci_eq(item, PLACER("SINCE", 5)) && imap_get_type_ar(array, increment) != IMAP_ARGUMENT_TYPE_ARRAY) {
			eval = imap_search_messages_date(user, message, header, current, imap_get_st_ar(array, increment++), 1, 2);
		}
		else if (increment < number && !st_cmp_ci_eq(item, PLACER("SENTBEFORE", 10)) && imap_get_type_ar(array, increment) != IMAP_ARGUMENT_TYPE_ARRAY) {
			eval = imap_search_messages_date(user, message, header, current, imap_get_st_ar(array, increment++), 0, 0);
		}
		else if (increment < number && !st_cmp_ci_eq(item, PLACER("SENTON", 6)) && imap_get_type_ar(array, increment) != IMAP_ARGUMENT_TYPE_ARRAY) {
			eval = imap_search_messages_date(user, message, header, current, imap_get_st_ar(array, increment++), 0, 1);
		}
		else if (increment < number && !st_cmp_ci_eq(item, PLACER("SENTSINCE", 9)) && imap_get_type_ar(array, increment) != IMAP_ARGUMENT_TYPE_ARRAY) {
			eval = imap_search_messages_date(user, message, header, current, imap_get_st_ar(array, increment++), 0, 2);
		}

		// Header checks.
		else if (increment < number && !st_cmp_ci_eq(item, PLACER("BCC", 3)) && imap_get_type_ar(array, increment) != IMAP_ARGUMENT_TYPE_ARRAY) {
			eval = imap_search_messages_header(user, message, header, current, item, imap_get_st_ar(array, increment++));
		}
		else if (increment < number && !st_cmp_ci_eq(item, PLACER("CC", 2)) && imap_get_type_ar(array, increment) != IMAP_ARGUMENT_TYPE_ARRAY) {
			eval = imap_search_messages_header(user, message, header, current, item, imap_get_st_ar(array, increment++));
		}
		else if (increment < number && !st_cmp_ci_eq(item, PLACER("FROM", 4)) && imap_get_type_ar(array, increment) != IMAP_ARGUMENT_TYPE_ARRAY) {
			eval = imap_search_messages_header(user, message, header, current, item, imap_get_st_ar(array, increment++));
		}
		else if (increment < number && !st_cmp_ci_eq(item, PLACER("TO", 2)) && imap_get_type_ar(array, increment) != IMAP_ARGUMENT_TYPE_ARRAY) {
			eval = imap_search_messages_header(user, message, header, current, item, imap_get_st_ar(array, increment++));
		}
		else if (increment < number && !st_cmp_ci_eq(item, PLACER("SUBJECT", 7)) && imap_get_type_ar(array, increment) != IMAP_ARGUMENT_TYPE_ARRAY) {
			eval = imap_search_messages_header(user, message, header, current, item, imap_get_st_ar(array, increment++));
		}
		// This search term takes two parameters.
		else if (increment + 1 < number && !st_cmp_ci_eq(item, PLACER("HEADER", 6)) && imap_get_type_ar(array, increment) != IMAP_ARGUMENT_TYPE_ARRAY &&
			imap_get_type_ar(array, increment + 1) != IMAP_ARGUMENT_TYPE_ARRAY) {
			eval = imap_search_messages_header(user, message, header, current, imap_get_st_ar(array, increment), imap_get_st_ar(array, increment + 1));
			increment += 2;
		}

		// Body checks.
		else if (increment < number && !st_cmp_ci_eq(item, PLACER("BODY", 4)) && imap_get_type_ar(array, increment) != IMAP_ARGUMENT_TYPE_ARRAY) {
			eval = imap_search_messages_body(user, message, current, imap_get_st_ar(array, increment++));
		}

		// Full message checks.
		else if (increment < number && !st_cmp_ci_eq(item, PLACER("TEXT", 4)) && imap_get_type_ar(array, increment) != IMAP_ARGUMENT_TYPE_ARRAY) {
			eval = imap_search_messages_text(user, message, current, imap_get_st_ar(array, increment++));
		}

		// Size checks.
		else if (increment < number && !st_cmp_ci_eq(item, PLACER("LARGER", 6)) && imap_get_type_ar(array, increment) != IMAP_ARGUMENT_TYPE_ARRAY) {
			eval = imap_search_messages_size(current, imap_get_st_ar(array, increment++), 1);
		}
		else if (increment < number && !st_cmp_ci_eq(item, PLACER("SMALLER", 7)) && imap_get_type_ar(array, increment) != IMAP_ARGUMENT_TYPE_ARRAY) {
			eval = imap_search_messages_size(current, imap_get_st_ar(array, increment++), 0);
		}

		// Range checks.
		else if (increment < number && !st_cmp_ci_eq(item, PLACER("UID", 3)) && imap_get_type_ar(array, increment) != IMAP_ARGUMENT_TYPE_ARRAY
			&& imap_valid_sequence(imap_get_st_ar(array, increment)) == 1) {
			eval = imap_search_messages_range(current, imap_get_st_ar(array, increment++), 1);
		}
		// If the characters make up a valid sequence, try parsing it.
		else if (imap_valid_sequence(item) == 1) {
			eval = imap_search_messages_range(current, item, 0);
		}

		// All messages matches everything.
		else if (!st_cmp_ci_eq(item, PLACER("ALL", 3))) {
			eval = 1;
		}

		// Handle negation.
		else if (not == 0 && !st_cmp_ci_eq(item, PLACER("NOT", 3))) {
			not = 2;
		}

		// Handle the ors.
		else if (or == 0 && !st_cmp_ci_eq(item, PLACER("OR", 2))) {
			or = 3;
		}

		// We don't support KEYWORD, so just skip over its parameter, and since no messages will ever contain it, return an empty set.
		else if (!st_cmp_ci_eq(item, PLACER("KEYWORD", 7))) {
			eval = -1;
		}
		// We don't support UNKEYWORD, so just skip over its parameter.
		else if (!st_cmp_ci_eq(item, PLACER("UNKEYWORD", 9))) {
			eval = 1;
		}
		// We don't support searches that are aware of character set translations..
		else if (!st_cmp_ci_eq(item, PLACER("CHARSET", 7))) {
			increment++;
		}
		// Unrecognized search parameter.
		else {
			//log_pedantic("Unrecognized search keyword %.*s.", st_length_get(item), st_char_get(item));
			eval = -1;
		}

		// Change the value if not is enabled.
		if (not == 2) {
			not = 1;
		}
		else if (not == 1) {
			not = 0;
			if (eval == 1) {
				eval = -1;
			}
			else {
				eval = 1;
			}
		}

		// Handle the or'ing.
		if (or == 3) {
			or = 2;
			output = 0;
		}
		else if (or == 2) {
			if (eval == 1) {
				output = 1;
			}
			or--;
		}
		else if (or == 1) {
			if (output != 1 && eval == 1) {
				output = 1;
			}
			or--;
		}
		else {
			output = eval;
		}
	}

	return output;
}

inx_t * imap_search_messages(connection_t *con) {

	time_t start;
	inx_t *output;
	inx_cursor_t *cursor = NULL;
	stringer_t *header = NULL;
	mail_message_t *message = NULL;
	uint64_t finished = 0, uid = 0, count = 0;
	meta_message_t *duplicate, *active = NULL;
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = 0 };

	if (!con || !(output = inx_alloc(M_INX_LINKED, &meta_message_free))) {
		return NULL;
	}

	while (status() && !finished) {

		/// LOW: Is a read lock necessary now that were using index reference counters and thread safe iteration cursors?
		new_meta_user_rlock(con->imap.user);
		start = time(NULL);

		if (con->imap.user && con->imap.user->messages && (cursor = inx_cursor_alloc(con->imap.user->messages))) {
			while (uid && (active = inx_cursor_value_next(cursor)) && active->messagenum <= uid);
		} else {
			finished = 1;
		}

		while (!finished && time(NULL) != (start + 1) && (active = inx_cursor_value_next(cursor))) {

			// Check for a match.
			if (active->foldernum == con->imap.selected &&
					imap_search_messages_inner(con->imap.user, &message, &header, active, con->imap.arguments, 0) == 1 &&
					(key.val.u64 = active->messagenum) && (duplicate = meta_message_dupe(active)) &&
					inx_insert(output, key, duplicate) != true) {
				meta_message_free(duplicate);
			}

			// Cleanup, if needed.
			if (message != NULL) {
				mail_destroy(message);
				message = NULL;
			}
			// Cleanup, if needed.
			if (header != NULL) {
				mail_destroy_header(header);
				header = NULL;
			}

			uid = active->messagenum;
			count++;
		}

		new_meta_user_unlock(con->imap.user);
		inx_cursor_free(cursor);

		// If the search is defers it should also sleep to allowing other threads access.
		if (!active) {
			finished = 1;
		}
		else {
			usleep(10000);
		}

	}

	// We need to update the sequence numbers.
	if (!finished && con->imap.uid != 1 && con->imap.messages_checkpoint != con->imap.user->serials.messages && (cursor = inx_cursor_alloc(output))) {
		new_meta_user_rlock(con->imap.user);

		while ((active = inx_cursor_value_next(cursor)) && con->imap.user && con->imap.user->messages) {

			key = inx_cursor_key_active(cursor);
			duplicate = inx_find(con->imap.user->messages, key);

			// If the message wasn't found, set the sequence number to zero so its not included in the output.
			if (!duplicate || active->messagenum != duplicate->messagenum) {
				active->sequencenum = 0;
			}
			else {
				active->sequencenum = duplicate->sequencenum;
			}
		}

		new_meta_user_unlock(con->imap.user);
		inx_cursor_free(cursor);
	}

	return output;
}

/*
inx_t * imap_search_messages(connection_t *con) {

	time_t start;
	int_t counter;
	stringer_t *header = NULL;
	uint64_t uid = 0;
	inx_t *output; //, *local;
	mail_message_t *message = NULL;
	meta_message_t *duplicate, *active = NULL;

	if ((output = inx_alloc(M_INX_LINKED, &meta_message_free)) == NULL) {
		return NULL;
	}

	while (status()) {

		counter = 0;
		new_meta_user_rlock(con->imap.user);

		// Record the start time.
		start = time(NULL);

		// Check to see if another thread has used this structure.
		if (session != NULL && con->imap.user != NULL && con->imap.user->messages != NULL && (active = ll_peek(con->imap.user->messages)) && active->messagenum != uid) {
			ll_reset(con->imap.user->messages);
			while ((active = ll_pop(con->imap.user->messages)) != NULL && active->messagenum != uid);
			if (active == NULL) {
				new_meta_user_unlock(con->imap.user);
				return output;
			}
		}

		active = NULL;
		while (counter++ < 25 && session != NULL && con->imap.user != NULL && con->imap.user->messages != NULL && (active = ll_pop(con->imap.user->messages)) != NULL) {

			// Check for a match.
			if (active->foldernum == con->imap.selected && imap_search_messages_inner(con->imap.user, &message, &header, active, con->imap.arguments, 0) == 1 &&
				(duplicate = meta_message_dupe(active)) != NULL) {
				ll_add(output, duplicate);
			}

			// Cleanup, if needed.
			if (message != NULL) {
				mail_destroy(message);
				message = NULL;
			}
			// Cleanup, if needed.
			if (header != NULL) {
				mail_destroy_header(header);
				header = NULL;
			}

			// Only allow an unlock if we've held the structure for more than 3 seconds.
			if (counter == 25 && (int)difftime(time(NULL), start) <= 3) {
				counter = 0;
			}
		}

		if (active == NULL) {

			// If were going to return sequence numbers we need to
			if (con->imap.uid != 1 && con->imap.messages_checkpoint != con->imap.user->serials.messages) {

				// Unlock and then relock the mailbox in case any other threads are waiting.
				new_meta_user_unlock(con->imap.user);
				new_meta_user_rlock(con->imap.user);

				ll_reset(output);
				while ((active = ll_pop(output)) != NULL) {

					ll_reset(con->imap.user->messages);
					while ((duplicate = ll_pop(con->imap.user->messages)) != NULL && active->messagenum != duplicate->messagenum);

					// Copy the potentially new sequence number.
					if (duplicate != NULL) {
						active->sequencenum = duplicate->sequencenum;
					}
					// If the message has been deleted, set the sequence number to zero so its not included in the output.
					else {
						active->sequencenum = 0;
					}
				}
			}

			new_meta_user_unlock(con->imap.user);
			return output;
		}
		else {
			uid = active->messagenum;
		}

		new_meta_user_unlock(con->imap.user);
		log_pedantic("%lu deferring...", pthread_self());
	}

	return output;
}
*/
