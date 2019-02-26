
/**
 * @file /magma/core/host/folder.c
 *
 * @brief	Functions for folder operations.
 */

#include "magma.h"

/**
 * @brief	Check to see if a specified directory exists, or if specified, create it if it doesn't exist.
 * @param	path	a managed string containing the ful pathname of the directory.
 * @param	create	if true, attempt to create the directory if it does not already exist.
 * @return	-1 if the directory doesn't exist or couldn't be created, 0 if the directory exists, or 1 if the directory was created.
 */
int_t folder_exists(stringer_t *path, bool_t create) {

	DIR *d;
	int_t result = 0;

	if (!(d = opendir(st_char_get(path)))) {
		if (!create || (result = mkdir(st_char_get(path), S_IRWXU))) {
			result = -1;
		}
		else {
			result = 1;
		}
	}  
	else {
		closedir(d);
	}

	return result;
}

/**
 * @brief	Count the number of files in a folder.
 */
int_t folder_count(stringer_t *path, bool_t recursive, bool_t strict) {

	DIR *dir;
	stringer_t *node;
	struct dirent *entry;
	int_t count = 0, subdir = 0;

	if (st_empty(path) || !(dir = opendir(st_char_get(path)))) {
		log_info("Unable to access the requested directory. { directory = \"%s\" / error = %s }",
			(st_empty(path) ? "NULL" : st_char_get(path)), errno_string(errno, MEMORYBUF(1024), 1024));
		return -1;
	}

	// Loop through.
	while ((entry = readdir(dir))) {

		if (st_cmp_cs_eq(NULLER(entry->d_name), PLACER(".", 1)) && st_cmp_cs_eq(NULLER(entry->d_name), PLACER("..", 2))) {

			// Build the path.
			if (!(node = st_merge("nnn", st_char_get(path), !st_cmp_cs_ends(path, PLACER("/", 1)) ? "" : "/", entry->d_name))) {
				closedir(dir);
				return false;
			}

			// Recursively count the sub directory.
			else if (entry->d_type == DT_DIR && recursive && (subdir = folder_count(node, true, strict)) <= 0) {
				st_free(node);
				closedir(dir);
				return -1;
			}
			// If we hit a directory, and didn't get an error while counting it, add the subdir count to the overall count.
			else if (entry->d_type == DT_DIR) {
				count += subdir;
			}

			// Regular file.
			else if (strict && entry->d_type == DT_REG) {
				count++;
			}
			else {
				count++;
			}

			st_free(node);
		}
	}

	closedir(dir);

	return count;
}
