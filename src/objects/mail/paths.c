
/**
 * @file /magma/objects/mail/paths.c
 *
 * @brief	Functions for mapping mail messages to their persistent file storage paths.
 */

#include "magma.h"

/**
 * @note	This function is not called from anywhere in the code and may be subject to removal.
 */
int_t mail_path_finder(chr_t *string) {

	int_t length = ns_length_get(string);

	while (length) {

		if (*(string + length) == '/') {
			return length + 1;
		}

		length--;
	}

	return 0;
}

/**
 * @brief	Return the fully qualified local file path of a stored mail message for a specified message number and server.
 * @param	number		the mail message id.
 * @param	server		the hostname of the server where the message data resides or if NULL, the default server.
 * @return	NULL on failure, or a pointer to a null-terminated string containing the absolute file path of the specified message.
 */
chr_t * mail_message_path(uint64_t number, chr_t *server) {

	chr_t *result;

	if (!(result = ns_alloc(1024))) {
		log_pedantic("Unable to allocate a buffer of %i bytes for the storage path.", 1024);
		return NULL;
	}

	// The default storage server.
	if (!server) {
		server = st_char_get(magma.storage.active);
	}

	// Build the message path.
	if ((snprintf(result, 1024, "%.*s/%s/%lu/%lu/%lu/%lu/%lu", st_length_int(magma.storage.root), st_char_get(magma.storage.root),
		server, number / 32768 / 32768 / 32768 / 32768, number / 32768 / 32768 / 32768 , number / 32768 / 32768,  number / 32768, number)) <= 0) {
		log_pedantic("Unable to create the message path.");
		ns_free(result);
		return NULL;
	}

	return result;
}

/**
 * @brief	Create the on-disk directory structure necessary to hold a given message's file data.
 * @param	number		the mail message id.
 * @param	server		the hostname of the server where the message data resides or if NULL, the default server.
 * @return	true on success or false on failure.
 */
bool_t mail_create_directory(uint64_t number, chr_t *server) {

	DIR *dir;
	chr_t dirpath[1024];

	if (!number) {
		return false;
	}

	// The default storage server.
	if (!server) {
		server = st_char_get(magma.storage.active);
	}

	// Check for the base NFS directory.
	snprintf(dirpath, 1024, "%.*s/%s",  st_length_int(magma.storage.root), st_char_get(magma.storage.root), server);

	if (!(dir = opendir(dirpath))) {
		log_error("It appears that the path %s is invalid.", dirpath);
		return false;
	}
	else {
		closedir(dir);
	}

	// Check the first level.
	snprintf(dirpath, 1024, "%.*s/%s/%lu", st_length_int(magma.storage.root), st_char_get(magma.storage.root), server,
		number / 32768 / 32768 / 32768 / 32768);

	if (!(dir = opendir(dirpath))) {

		if (mkdir(dirpath, S_IRWXU) != 0) {
			log_error("An error occurred while attempting to create the directory %s.", dirpath);
			return false;
		}
	}
	else {
		closedir(dir);
	}

	// Check the second level.
	snprintf(dirpath, 1024, "%.*s/%s/%lu/%lu", st_length_int(magma.storage.root), st_char_get(magma.storage.root), server,
		number / 32768 / 32768 / 32768 / 32768, number / 32768 / 32768 / 32768);

	if (!(dir = opendir(dirpath))) {

		if (mkdir(dirpath, S_IRWXU) != 0) {
			log_error("An error occurred while attempting to create the directory %s.", dirpath);
			return false;
		}
	}
	else {
		closedir(dir);
	}

	// Check the third level.
	snprintf(dirpath, 1024, "%.*s/%s/%lu/%lu/%lu", st_length_int(magma.storage.root), st_char_get(magma.storage.root), server,
		number / 32768 / 32768 / 32768 / 32768, number / 32768 / 32768 / 32768, number / 32768 / 32768 );

	if (!(dir = opendir(dirpath))) {

		if (mkdir(dirpath, S_IRWXU) != 0) {
			log_error("An error occurred while attempting to create the directory %s.", dirpath);
			return false;
		}
	}
	else {
		closedir(dir);
	}

	// Check the fourth level.
	snprintf(dirpath, 1024, "%.*s/%s/%lu/%lu/%lu/%lu", st_length_int(magma.storage.root), st_char_get(magma.storage.root), server,
		number / 32768 / 32768 / 32768 / 32768, number / 32768 / 32768 / 32768, number / 32768 / 32768,  number / 32768 );

	if (!(dir = opendir(dirpath))) {

		if (mkdir(dirpath, S_IRWXU) != 0) {
			log_error("An error occurred while attempting to create the directory %s.", dirpath);
			return false;
		}
	}
	else {
		closedir(dir);
	}

	return true;
}
