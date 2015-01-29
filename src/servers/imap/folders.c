
/**
 * @file /magma/servers/imap/folders.c
 *
 * @brief	Functions used to handle IMAP commands/actions.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/// TODO: Since the Portal needs access to some of these folder manipulation functions the common logic should be extracted
/// and moved into the folder object interface.

/**
 * @brief	Get an imap folder's escaped name.
 * @param	folders		an inx holder containing the ancestor folders of the specified imap folder.
 * @param	active		a pointer to the meta folder object of the folder to have its name escaped.
 * @return	NULL on failure, or a pointer to a managed string containing the escaped name of the specified imap folder on success.
 */
stringer_t * imap_folder_name_escaped(inx_t *folders, meta_folder_t *active) {

	stringer_t *name = meta_folders_name(folders, active), *string;

	if (!name) {
		return NULL;
	}

	st_replace(&name, PLACER("\"", 1), PLACER("\\\"", 2));
	string = st_merge("nsn", "\"", name, "\"");
	st_free(name);
	return string;
	/****

	We should detect and escape non printable characters with
	 modified UTF-7, but that requires code in other places.

	size_t size;
	int_t escape = 0;
	chr_t *data, buffer[128];

	data = st_char_get(name);
	size = st_length_get(name);
	while (size-- && escape == 0) {
		if (*data == '"') escape = 1;
		data++;
	}

	if (escape == 0) {
		string = st_merge("nsn", "\"", name, "\"");
		st_free(name);
	}
	else {
		snprintf(buffer, 128, "{%u}\r\n", st_length_get(name));
		string = st_merge("ns", buffer, name);
		st_free(name);
	}
	return string;
	****/
}

/**
 * @brief	Get the next order value for an imap folder.
 * @note	The next order value is the current highest order value of any child folder in the specified directory, incremented by one.
 * @param	folder	an inx holder containing the collection of imap folders to be scanned.
 * @param	parent	the numerical id of the folder to be queried.
 * @return	0 on failure, or the next order value of the specified folder on success.
 */
uint64_t imap_next_folder_order(inx_t *folders, uint64_t parent) {

	uint64_t order = 0;
	inx_cursor_t *cursor = NULL;
	meta_folder_t *active = NULL;

	if (folders && (cursor = inx_cursor_alloc(folders))) {

		// Advance through the structure.
		while ((active = inx_cursor_value_next(cursor))) {

			if (active->parent == parent && active->order > order) {
				order = (active->order + 1);
			}

		}

		inx_cursor_free(cursor);
	}

	return order;
}

// Compares a mailbox with wildcards to a name. Returns 1 for matches, 0 for nots.
int_t imap_folder_compare(stringer_t *name, stringer_t *compare) {

	chr_t seek, *left, *right;
	size_t left_length, right_length;

	// Handles the special case of the Inbox being case insensitive.
	if (!st_cmp_ci_eq(compare, PLACER("Inbox", 5)) && !st_cmp_ci_eq(name, PLACER("Inbox", 5))) {
		return 1;
	}

	// Get setup.
	left = st_char_get(name);
	left_length = st_length_get(name);
	right = st_char_get(compare);
	right_length = st_length_get(compare);

	while (right_length != 0) {

		seek = *right;
		right++;
		right_length--;

		// If there is an asterik, then we should match anything.
		if (seek == '*') {
			return 1;
		}
		// Match anything that isn't a heirarchy delimeter.
		else if (seek == '%') {

			while (*left != '.' && left_length != 0) {
				left++;
				left_length--;
			}

			if (left_length == 0 && right_length == 0) {
				return 1;
			}
			else if (left_length == 0 || right_length == 0) {
				return 0;
			}
			else if (*left == '.' && *right == '.') {
				right++;
				right_length--;
			}
		}
		else if (seek != *left) {
			return 0;
		}

		left++;
		left_length--;
	}

	return 1;
}

/**
 * @brief	Check to see if a name is a valid imap folder name.
 * @note	The following naming checks are enforced:
 * 			1. The name can't be empty or begin with a period.
 * 			2. It cannot contain a non-printable character.
 * 			3. Trailing periods will be removed.
 * 			4. The folder name cannot contain consecutive periods.
 * @param	name	a managed string containing the imap folder name to be validated.
 * @return	true on success or false on failure.
 */
bool_t imap_valid_folder_name(stringer_t *name) {

	size_t len;
	chr_t *stream;
	int_t period = 0;

	// Generic failure.
	if (st_empty_out(name, (uchr_t **)&stream, &len)) {
		log_pedantic("Invalid imap folder name was empty.");
		return false;
	}

	// Folders can't start with a period.
	if (*stream == '.') {
		log_pedantic("Invalid imap folder name began with a period.");
		return false;
	}

	while (len) {

		// Look for non printable characters.
		if (*stream < ' ' || *stream > '~') {
			log_pedantic("Invalid imap folder name contained a non-printable character.");
			return false;
		}
		else if (period == 0 && *stream == '.') {
			period = 1;

			// If a folder ends in a period, remove it by shortening the string by one character. This is for backwards compatibility.
			if (len == 1 && st_length_get(name) == 1) {
				log_pedantic("Invalid imap folder name consisted of one period.");
				return false;
			}
			else if (len == 1) {
				log_pedantic("Trailing period was removed from imap folder name.");
				st_length_set(name, st_length_get(name) - 1);
			}

		}
		// Look for consecutive periods.
		else if (period == 1 && *stream == '.') {
			log_pedantic("Invalid imap folder name contained consecutive periods.");
			return false;
		}
		else if (period == 1) {
			period = 0;
		}

		stream++;
		len--;
	}

	return true;
}

/**
 * @brief	Get the highest node level indicated by an imap folder name.
 * @note	The smallest possible node level value is 1.
 * @param	name	a managed string containing the name of the imap folder.
 * @return	0 on failure, or the number of node levels indicated by the specified imap folder name, on success.
 */
int_t imap_count_folder_levels(stringer_t *name) {

	size_t len;
	chr_t *stream;
	int_t levels = 1;

	// Generic failure.
	if (st_empty_out(name, (uchr_t **)&stream, &len)) {
		log_pedantic("We were passed in invalid folder name for checking.");
		return 0;
	}

	while (len) {

		// Count the periods.
		if (*stream == '.') {
			levels++;
		}

		if (levels > IMAP_FOLDER_RECURSION_LMIIT) {
			log_pedantic("We detected more than %i folder levels. Stopping the count.", IMAP_FOLDER_RECURSION_LMIIT);
			return levels;
		}

		len--;
		stream++;
	}

	return levels;

}

/**
 * @brief	Create a new imap folder.
 * @note	If they don't already exist, any parent folders specified in the fully qualified folder name will automatically be created first.
 * @param	usernum		the numerical id of the user to whom the new imap folder will belong.
 * @param	folders		an inx holder containing the set of folders into which the new imap folder will be inserted.
 * @param	name		a managed string containing the fully qualified name of the new imap folder to be created.
 * @return	1 on success or <= 0 on failure.
 *          0:	An invalid parameter was passed to the function, or an internal failure occurred.
 *         -1:	The specified new folder name was invalid.
 *         -2:	The node depth of the new folder is too large (imap folder recursion limit reached [IMAP_FOLDER_RECURSION_LMIIT]).
 *         -3:	Special folder "Inbox" cannot contain subfolders.
 *         -4:	Part of the folder path name exceeds 16 characters (FOLDER_LENGTH_LIMIT) after being unescaped for quotation marks.
 *         -5:	The specified folder name already exists.
 */
int_t imap_folder_create(uint64_t usernum, inx_t *folders, stringer_t *name) {

	uint32_t order;
	int_t levels;
	meta_folder_t *active;
	uint64_t parent = 0;
	placer_t current, fragment;
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = 0 };
	stringer_t *working = NULL, *holder, *duplicate;

	if (!folders || !name || !usernum) {
		log_pedantic("We were passed an invalid pointer.");
		return 0;
	}

	// Make sure we have a valid folder name.
	if (!imap_valid_folder_name(name)) {
		return -1;
	}

	// We hard code the maximum number of sub folders.
	else if ((levels = imap_count_folder_levels(name)) > IMAP_FOLDER_RECURSION_LMIIT) {
		return -2;
	}

	// Make sure the passed in folder is not the inbox.
	tok_get_st(name, '.', 0, &fragment);

	if (!st_cmp_ci_eq(&fragment, PLACER("Inbox", 5))) {
		return -3;
	}

	// Convert back the quotes for length calcs.
	if (!(duplicate = st_dupe(name))) {
		return 0;
	}

	st_replace(&duplicate, PLACER("&ACI-", 5), PLACER("\"", 1));

	// Iterate through each segment and make sure its less than fifteen characters.
	for (int_t level = 0; level < levels; level++) {

		if (tok_get_st(duplicate, '.', level, &current) < 0) {
			log_pedantic("We were unable to extract the folder name token.");
			st_free(duplicate);
			return 0;
		}
		else if (pl_length_get(current) > FOLDER_LENGTH_LIMIT) {
			st_free(duplicate);
			return -4;
		}

	}

	st_free(duplicate);

	// Check to see if the folder already exists.
	if (meta_folders_by_name(folders, name)) {
		return -5;
	}

	// Hack to detect quotes being used in the name. This escapes the quotes using
	// modified UTF-7. In theory all non-printable characters should be escaped this way.
	// Note that by changing the folder name, some IMAP clients won't detect the new folder
	// until they reconnect.
	if (!(duplicate = st_dupe(name))) {
		return 0;
	}

	st_replace(&duplicate, PLACER("\"", 1), PLACER("&ACI-", 5));

	// Iterate through each folder level and create the folder, if necessary.
	for (int_t level = 0; level < levels; level++) {

		// Get the current level.
		if (tok_get_st(duplicate, '.', level, &current) < 0) {
			st_cleanup(working);
			st_free(duplicate);
			return 0;
		}

		// Build a stringer with the current working level.
		if (!working) {
			working = st_dupe_opts(MANAGED_T | HEAP | CONTIGUOUS, &current);
		}
		else {

			if (!(holder = st_merge("sns", working, ".", &current))) {
				st_cleanup(working);
				st_free(duplicate);
				return 0;
			}

			st_free(working);
			working = holder;
		}

		// Check whether this folder already exists.
		if (!(active = meta_folders_by_name(folders, working))) {

			// Create it in the database.
			order = imap_next_folder_order(folders, parent);

			if (!(key.val.u64 = meta_data_insert_folder(usernum, &current, parent, order))) {
				st_free(working);
				st_free(duplicate);
				return 0;
			}

			// Save the folder information.
			active = mm_alloc(sizeof(meta_folder_t));
			active->foldernum = key.val.u64;
			mm_copy(active->name, pl_data_get(current), pl_length_get(current));
			active->order = order;
			active->parent = parent;

			// Add the new folder to the structure.
			if (!inx_insert(folders, key, active)) {
				mm_free(active);
				st_free(working);
				st_free(duplicate);
				return 0;
			}
		}

		parent = active->foldernum;
	}

	st_free(duplicate);
	st_free(working);

	return 1;
}

/**
 * @brief	Remove an imap folder, both in memory and on the database.
 * @note	Any child messages of the specified folder will be deleted, but if it has child folders, the folder will not be deleted.
 * 			The function also protects the special "Inbox" folder from deletion.
 * @param	usernum		the numerical id of the user that owns the imap folder to be deleted.
 * @param	folders		an inx holder containing a collection of folders for looking up the specified imap folder by name.
 * @param	messages	an inx holder containing a collection of messages for looking up the contents of the specified imap folder.
 * @param	name		a managed string containing the name of the imap folder to be deleted.
 * @return	<= 0 on failure, or 1 on success.
 * 			 0:	General failure or if there was an error removing the folder from the database.
 * 			-1: The specified folder name was invalid.
 * 			-2:	Removal failed because the "Inbox" folder was specified.
 * 			-3:	The specified folder could not be found.
 */
int_t imap_folder_remove(uint64_t usernum, inx_t *folders, inx_t *messages, stringer_t *name) {

	placer_t fragment;
	meta_folder_t *active;
	inx_cursor_t *cursor;
	meta_message_t *message;
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = 0 };

	if (!folders || !name || !usernum) {
		log_pedantic("We were passed an invalid pointer.");
		return 0;
	}

	// Make sure we have a valid folder name.
	if (!imap_valid_folder_name(name)) {
		return -1;
	}

	// Make sure the passed in folder is not the inbox.
	tok_get_st(name, '.', 0, &fragment);

	if (!st_cmp_ci_eq(&fragment, PLACER("Inbox", 5))) {
		return -2;
	}

	// Make sure the folder exists, and find the structure.
	if (!(active = meta_folders_by_name(folders, name))) {
		return -3;
	}

	// Delete all of the messages in this folder.
	if ((cursor = inx_cursor_alloc(messages))) {

		while ((message = inx_cursor_value_next(cursor))) {

			if (message->foldernum == active->foldernum && mail_remove_message(usernum, message->messagenum, message->size, message->server)) {
				key.val.u64 = message->messagenum;
				inx_delete(messages, key);
			}

		}

		inx_cursor_free(cursor);
	}

	// If the folder has children, don't delete it.
	if (!meta_folders_children(folders, active->foldernum)) {

		if (meta_data_delete_folder(usernum, active->foldernum) != 1) {
			log_error("Unable to delete the folder %lu.", active->foldernum);
			return 0;
		}

		key.val.u64 = active->foldernum;
		inx_delete(folders, key);
	}

	return 1;
}

/**
 * @brief	Rename an imap folder.
 * @note	Any parent folder components of the folder's fully qualified name will be created if they do not already exist.
 * @param	usernum		the numerical id of the user requesting the folder renaming.
 * @param	folders		an inx holder containing the  folders to be searched for the folder specified to be renamed.
 * @param	original	a managed string containing the original name of the folder to be renamed.
 * @param	rename		a managed string containing the new name of the specified folder.
 * @return  1 on success or 0 or less on failure.
 * 			 0:	One of the parameters passed to the function was invalid, or there was a memory allocation failure.
 * 			-1:	Either the original or new folder name was invalid.
 * 			-2: Was unable to rename the "Inbox" folder.
 * 			-3: The specified imap folder did not exist.
 * 			-4: Either the original or new name exceeded the imap folder recursion limit.
 * 			-5: A folder already exists with the new folder name.
 * 			-6: A segment of the folder name was larger than FOLDER_LENGTH_LIMIT (16 bytes).
 *
 *
 *
 *
 */
int_t imap_folder_rename(uint64_t usernum, inx_t *folders, stringer_t *original, stringer_t *rename) {

	uint64_t order;
	meta_folder_t *active;
	uint64_t parent = 0;
	placer_t current, origfrag, renfrag;
	int_t orig_levels, rename_levels;
	stringer_t *working = NULL, *holder, *duplicate;
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = 0 };

	if (!folders || !original || !rename || !usernum) {
		log_pedantic("We were passed an invalid pointer.");
		return 0;
	}

	// Make sure we have a valid folder name.
	if (!imap_valid_folder_name(original) || !imap_valid_folder_name(rename)) {
		return -1;
	}

	tok_get_st(original, '.', 0, &origfrag);
	tok_get_st(rename, '.', 0, &renfrag);

	// Were not allowed to delete the inbox.
	if (!st_cmp_ci_eq(&origfrag, PLACER("Inbox", 5)) || !st_cmp_ci_eq(&renfrag, PLACER("Inbox", 5)) ) {
		return -2;
	}

	// Make sure the folder exists, and find the structure.
	if (!meta_folders_by_name(folders, original)) {
		return -3;
	}

	// We hard code the maximum number of sub folders.
	else if ((rename_levels = imap_count_folder_levels(rename)) > IMAP_FOLDER_RECURSION_LMIIT || (orig_levels = imap_count_folder_levels(original)) > IMAP_FOLDER_RECURSION_LMIIT) {
		return -4;
	}

	// Make sure the target folder does not exist.
	if (meta_folders_by_name(folders, rename)) {
		return -5;
	}

	// Convert back the quotes for length calcs.
	if (!(duplicate = st_dupe(rename))) {
		return 0;
	}
	st_replace(&duplicate, PLACER("&ACI-", 5), PLACER("\"", 1));

	// Iterate through each segment and make sure its less than fifteen characters.
	for (int_t level = 0; level < rename_levels; level++) {

		if (tok_get_st(duplicate, '.', level, &current) < 0) {
			log_pedantic("We were unable to extract the folder name token.");
			st_free(duplicate);
			return 0;
		}
		else if (pl_length_get(current) > FOLDER_LENGTH_LIMIT) {
			st_free(duplicate);
			return -6;
		}

	}

	st_free(duplicate);


	// Hack to detect quotes being used in the name. This escapes the quotes using
	// modified UTF-7. In theory all non-printable characters should be escaped this way.
	// Note that by changing the folder name, some IMAP clients won't detect the new folder
	// until they reconnect.
	if (!(duplicate = st_dupe(rename))) {
		return 0;
	}
	st_replace(&duplicate, PLACER("\"", 1), PLACER("&ACI-", 5));

	// Iterate through each folder level and create any parent folders, if necessary. In the loop above we check every level, but
	// here we check all but the last level since that will be an update operation and not an insert.
	for (int_t level = 0; level < rename_levels -1; level++) {

		// Get the current name.
		if (tok_get_st(duplicate, '.', level, &current) < 0) {
			st_cleanup(working);
			st_free(duplicate);
			return 0;
		}

		// Build a stringer of the full working folder name.
		if (!working) {
			working = st_dupe_opts(MANAGED_T | HEAP | CONTIGUOUS, &current);
		}
		else {

			if (!(holder = st_merge("sns", working, ".", &current))) {
				st_cleanup(working);
				st_free(duplicate);
				return 0;
			}

			st_free(working);
			working = holder;
		}

		// Check whether this folder already exists.
		if (!(active = meta_folders_by_name(folders, working))) {

			// Create it in the database.
			order = imap_next_folder_order(folders, parent);

			if (!(key.val.u64 = meta_data_insert_folder(usernum, &current, parent, order))) {
				st_free(working);
				st_free(duplicate);
				return 0;
			}

			// Save the folder information.
			active = mm_alloc(sizeof(meta_folder_t));
			active->foldernum = key.val.u64;
			mm_copy(active->name, pl_data_get(current), pl_length_get(current));
			active->order = order;
			active->parent = parent;

			// Add the new folder to the structure.
			if (!inx_insert(folders, key, active)) {
				mm_free(active);
				st_free(working);
				st_free(duplicate);
				return 0;
			}

		}

		parent = active->foldernum;
	}

	// Cleanup if necessary.
	st_cleanup(working);

	// Get the folder.
	if (!(active = meta_folders_by_name(folders, original))) {
		st_free(duplicate);
		return -3;
	}

	// A folder cannot become a child of itself.
	if (active->foldernum == parent) {
		st_free(duplicate);
		return -7;
	}

	// If the parent is changing, find out the new order.
	if (active->parent != parent) {
		order = imap_next_folder_order(folders, parent);
	}
	else {
		order = active->order;
	}

	// Get the new folder name.
	tok_get_st(duplicate, '.', rename_levels - 1, &current);

	// Change the folder name in the database.
	if (meta_data_update_folder_name(usernum, active->foldernum, &current, parent, order) != 1) {
		log_error("Unable to update the name on folder %lu.", active->foldernum);
		st_free(duplicate);
		return 0;
	}
	mm_wipe(active->name, 32);
	mm_copy(active->name, pl_data_get(current), pl_length_get(current));
	active->order = order;
	active->parent = parent;

	st_free(duplicate);

	return 1;
}

/**
 * @brief	Get the status of a folder.
 * @note	This function will count the number of messages in a folder, as well as the number of messages marked recent or unseen,
 * 			as well as the numerical id of the first message in the folder and the UIDNEXT of the specified folder.
 * @param	folders		an inx holder containing a list of folders to be searched for the specified folder.
 * @param	messages	an inx holder containing a complete list of a user's messages to be examined for gathering statistics.
 * @param	name		a managed string containing the name of the imap folder to be queried.
 * @param	status		a pointer to an imap folder status object to receive the folder's status information.
 * @return	1 on success or <= 0 on failure.
 *          0:	General failure.
 *         -1:	The specified folder name was invalid.
 *         -2:	The folder did not exist.
 */
int_t imap_folder_status(inx_t *folders, inx_t *messages, stringer_t *name, imap_folder_status_t *status) {

	meta_folder_t *folder;
	inx_cursor_t *cursor;
	meta_message_t *message;

	if (!folders || !name || !status) {
		log_pedantic("We were passed an invalid pointer.");
		return 0;
	}

	// Reset the folder structure.
	mm_wipe(status, sizeof(imap_folder_status_t));

	// Make sure we have a valid folder name.
	if (!imap_valid_folder_name(name)) {
		return -1;
	}

	// Make sure the folder exists, and find the structure.
	if (!(folder = meta_folders_by_name(folders, name))) {
		return -2;
	}

	// Store the folder number.
	status->foldernum = folder->foldernum;

	// Iterate through the messages structure and collect status information.
	if ((cursor = inx_cursor_alloc(messages))) {

		while ((message = inx_cursor_value_next(cursor))) {

			if (message->foldernum == folder->foldernum) {
				status->messages++;

				if ((message->status & MAIL_STATUS_RECENT) == MAIL_STATUS_RECENT) {
					status->recent++;
				}

				if ((message->status & MAIL_STATUS_SEEN) != MAIL_STATUS_SEEN) {
					status->unseen++;

					if (!status->first) {
						status->first = status->messages;
					}

				}

			}

			if (message->messagenum > status->uidnext) {
				status->uidnext = message->messagenum;
			}

		}

		inx_cursor_free(cursor);
	}

	status->uidnext += 1;

	return 1;
}

// Will take a reference pattern and return a linked list of the folders it refers to.
inx_t * imap_narrow_folders(inx_t *folders, stringer_t *reference, stringer_t *mailbox) {

	inx_t *result = NULL;
	inx_cursor_t *cursor = NULL;
	stringer_t *name, *compare;
	meta_folder_t *active, *holder;
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = 0 };

	// Combine the reference and the name.
	if (!(compare = st_merge("sns", reference, (reference && st_length_get(reference) && mailbox && st_length_get(mailbox)) ? "." : "", mailbox))) {
		log_pedantic("Unable to merge the reference and the mailbox.");
		return NULL;
	}


	if (folders && (cursor = inx_cursor_alloc(folders))) {

		// Advance through the structure.
		while ((active = inx_cursor_value_next(cursor))) {

			if ((name = meta_folders_name(folders, active)) && imap_folder_compare(name, compare) == 1) {

					// If we don't have a linked list, create one.
					if (result == NULL) {
						result = inx_alloc(M_INX_LINKED, &mm_free);
					}

					// Duplicate the folder and add it.
					if (result != NULL && (key.val.u64 = active->foldernum) != 0 &&	(holder = mm_dupe(active, sizeof(meta_folder_t))) != NULL &&
						inx_insert(result, key, holder) != true) {
							mm_free(holder);
					}
			}

			// If necessary, cleanup.
			if (name) st_free(name);

		}

		inx_cursor_free(cursor);
	}

	st_free(compare);
	return result;
}
