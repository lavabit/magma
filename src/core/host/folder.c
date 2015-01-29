
/**
 * @file /magma/core/host/folder.c
 *
 * @brief	Functions for folder operations.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
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
		if (!create || !(result = mkdir(st_char_get(path), S_IRWXU))) {
			result = -1;
		}
		{
			result = 1;
		}
	} else {
		closedir(d);
	}

	return result;
}
