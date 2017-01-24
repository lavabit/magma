
/**
 * @file /magma/objects/meta/folders.c
 *
 * @brief  Functions used for handling message folders.
 */

#include "magma.h"

/**
 * @brief	Allocate a new meta tag stat object for tracking message tags.
 * @param	tag		a managed string containing the name of the message tag.
 * @return	NULL on failure, or a newly allocated and initialized meta tag stat object on success.
 */
meta_stats_tag_t * meta_folder_stats_tag_alloc(stringer_t *tag) {

	meta_stats_tag_t *result;

	if (!(result = mm_alloc(align(16, sizeof(meta_stats_tag_t) + sizeof(placer_t)) + align(8, st_length_get(tag) + 1)))) {
		log_pedantic("Unable to allocate %zu bytes for a tag statistics tracker.", align(16, sizeof(meta_stats_tag_t) + sizeof(placer_t)) + align(8, st_length_get(tag) + 1));
		return NULL;
	}

	result->tag = (placer_t *)((chr_t *)result + sizeof(meta_stats_tag_t));
	((placer_t *)result->tag)->opts = PLACER_T | JOINTED | STACK | FOREIGNDATA;
	((placer_t *)result->tag)->length = st_length_get(tag);
	((placer_t *)result->tag)->data = (chr_t *)result + align(16, sizeof(meta_stats_tag_t) + sizeof(placer_t));
	mm_copy(st_data_get(result->tag), st_data_get(tag), st_length_get(tag));

	return result;
}

/**
 * @brief	Create a collection of meta tag stats for a set of messages.
 * @note	Each meta tag stat will contain the name of a message tag, along with the number of messages with which it was associated.
 * @param	messages	an inx holder containing the list of messages to be examined.
 * @param	folder		the numerical id of the parent folder that contains all target messages.
 * @return	NULL on failure, or an inx holder containing all of the messages' meta tag stats on success.
 */
inx_t * meta_folders_stats_tags(inx_t *messages, uint64_t folder) {

	size_t len;
	inx_t *result;
	inx_cursor_t *cursor;
	meta_message_t *active;
	meta_stats_tag_t *track;
	multi_t multi = { .type = M_TYPE_STRINGER, .val.st = NULL };

	if (!(result = inx_alloc(M_INX_HASHED, &mm_free))) {
		log_pedantic("Unable to scan the folder and collection tag statistics.");
		return NULL;
	}

	/// LOW: This is a very inefficient method for counting the number of times each tag appears.
	else if ((cursor = inx_cursor_alloc(messages))) {

		// Search for an existing tag context. If none is found allocate a new one and append it to the index.
		while ((active = inx_cursor_value_next(cursor))) {

			if (active->foldernum == folder && active->tags && (len = ar_length_get(active->tags))) {

				for (size_t i = 0; i < len && (multi.val.st = ar_field_st(active->tags, i)); i++) {

					if ((track = inx_find(result, multi))) {
						track->count++;
					}

					// If the search fails, allocate a new context and insert it. But if the insert fails free the memory.
					else if ((track = meta_folder_stats_tag_alloc(multi.val.st))) {

						if (inx_insert(result, multi, track)) {
							track->count++;
						}
						else {
							mm_free(track);
						}

					}

				}

			}

		}

		inx_cursor_free(cursor);
	}

	return result;
}

/**
 * @brief	Get a folder's fully qualified name.
 * @note	This function will prepend the entire ancestor path of a folder to its name, with each level delimited by periods.
 * @param	list	a pointer to an inx holder containing a list of folders to be searched for parent folders.
 * @param	folder	a pointer to the meta folder object that is the leaf node of the folder path.
 * @return	NULL on failure or a managed string containing the fully qualified folder name on success.
 */
stringer_t * meta_folders_name(inx_t *list, meta_folder_t *folder) {

	int_t recursion = 0;
	stringer_t *result, *holder;

	if (!list || !folder) {
		log_pedantic("We were passed a NULL pointer.");
		return NULL;
	}

	if (!(result = st_import(folder->name, ns_length_get(folder->name)))) {
		log_pedantic("We were unable to import the folder name.");
		return NULL;
	}

	while (folder && folder->parent && recursion++ < FOLDER_RECURSION_LIMIT) {

		// Get the parent folder.
		if (!(folder = meta_folders_by_number(list, folder->parent))) {
			log_pedantic("There appears to be a folder with an invalid parent.");
			return result;
		}

		// Merge the strings.
		if ((holder = st_merge("nns", folder->name, ".", result))) {
			st_free(result);
			result = holder;
		}
	}

	return result;
}

/**
 * @brief	Get a folder by its fully qualified name.
 * @param	folders		a pointer to an inx holder containing a list of folders to be traversed in the search.
 * @param	folder		a pointer to the meta folder object that is the leaf node of the folder path.
 * @return	NULL on failure or a managed string containing the fully qualified folder name on success.
 */
meta_folder_t * meta_folders_by_name(inx_t *folders, stringer_t *name) {

	int_t inbox = 0;
	stringer_t *current;
	inx_cursor_t *cursor;
	meta_folder_t *result = NULL, *active = NULL;

	if (!st_cmp_ci_eq(name, CONSTANT("Inbox"))) {
		inbox = 1;
	}

	if (!(cursor = inx_cursor_alloc(folders))) {
		return NULL;
	}

	// Get the name and compare.
	while (!result && (active = inx_cursor_value_next(cursor))) {

		if ((current = meta_folders_name(folders, active)) != NULL && !st_cmp_cs_eq(current, name)) {
			result = active;
		}
		else if (current != NULL && inbox == 1 && !st_cmp_ci_eq(current, name)) {
			result = active;
		}

		if (current != NULL) {
			st_free(current);
		}
	}

	inx_cursor_free(cursor);

	return result;
}

/**
 * @brief	Get a folder by number.
 * @param	folders		a pointer to an inx holder containing the folders to be searched.
 * @param	number		the numerical id of the folder to be retrieved.
 * @return	NULL on failure, or a pointer to the found meta folder object of the specified folder on success.
 */
meta_folder_t * meta_folders_by_number(inx_t *folders, uint64_t number) {

	inx_cursor_t *cursor;
	meta_folder_t *result = NULL, *active;

	if (!(cursor = inx_cursor_alloc(folders))) {
		return NULL;
	}

	while ((active = inx_cursor_value_next(cursor)) && !result) {

		if (active->foldernum == number) {
			result = active;
		}

	}

	inx_cursor_free(cursor);

	return result;
}

/**
 * @brief	Get the number of direct child folders of a specified parent folder.
 * @param	folders		a pointer to an inx holder containing the the collection of folders to be traversed.
 * @param	number		the folder id of the parent folder to be scanned for children.
 * @return	the number of children folders in the specified parent folder, or 0 on failure.
 */
int_t meta_folders_children(inx_t *folders, uint64_t number) {

	int_t result = 0;
	inx_cursor_t *cursor;
	meta_folder_t *active = NULL;

	if (!number || !(cursor = inx_cursor_alloc(folders))) {
		return 0;
	}

	while ((active = inx_cursor_value_next(cursor)) && !result) {

		if (active->parent == number) {
			result++;
		}

	}

	inx_cursor_free(cursor);

	return result;
}

