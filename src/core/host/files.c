
/**
 * @file /magma/core/host/files.c
 *
 * @brief	Generic system file I/O operations.
 *
 * $Author$
 * $Author$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Get the contents of a file on disk.
 * @see		file_load()
 * @note 	This is similar in function to file_load() but there is no read() length checking, so it can be used on non-regular files.
 * @param	name	a character pointer to the full pathname of the file to be opened.
 * @param	output	a managed string where the results of the file read operation will be stored.
 * @return	-1 on failure or the number of bytes read from the file on success.
 */
int_t file_read(char *name, stringer_t *output) {

	int_t fd, result;

	if (!output) {
		log_pedantic("File read operation performed with NULL output buffer.");
		return -1;
	}

	// FTM: returns the new file descriptor, or -1 if an error occurred (in which case, errno is set appropriately).
	if ((fd = open(name, O_RDONLY)) == -1) {
		log_info("Could not open the file %s for reading. {errno = %i & strerror = %s}", name, errno, strerror_r(errno, MEMORYBUF(1024), 1024));
		return -1;
	}

	// FTM: On success, the number of bytes read is returned (zero indicates end of file), and the file position is
	// advanced by this number. On error, -1 is returned, and errno is set appropriately.
	// LOW: Were loading files using a blocking read call.
	if ((result = read(fd, st_data_get(output), st_avail_get(output))) >= 0) {
		st_length_set(output, result);
	}
	else {
		log_info("Could not read the file %s. {errno = %i & strerror = %s}", name, errno, strerror_r(errno, MEMORYBUF(1024), 1024));
	}

	close(fd);
	return result;
}

/**
 * @brief	Get the contents of a file on disk.
 * @param	name	a character pointer to the full pathname of the file to be opened.
 * @return	NULL on failure, or a managed string containing all the data in the file.
 */
stringer_t * file_load(char *name) {

	int fd;
	stringer_t *result;
	struct stat info;
	char estring[1024];

	// FTM: returns the new file descriptor, or -1 if an error occurred (in which case, errno is set appropriately).
	if ((fd = open(name, O_RDONLY)) == -1) {
		log_info("Could not open the file %s for reading. {errno = %i & strerror = %s}", name, errno,
				(strerror_r(errno, estring, 1024) == 0 ? estring : "Unknown error"));
		return NULL;
	}
	// FTM: On success, zero is returned.  On error, -1 is returned, and errno is set appropriately.
	else if (fstat(fd, &info) == -1) {
		log_info("Could not fstat the file %s. {errno = %i & strerror = %s}", name, errno,
				(strerror_r(errno, estring, 1024) == 0 ? estring : "Unknown error"));
		close(fd);
		return NULL;
	} else if ((result = st_alloc(info.st_size)) == NULL) {
		log_info("Could not create a buffer big enough to hold the file %s.", name);
		close(fd);
		return NULL;

	}
	// FTM: On success, the number of bytes read is returned (zero indicates end of file), and the file position is
	// advanced by this number. On error, -1 is returned, and errno is set appropriately.
	// LOW: Were loading files using a blocking read call.
	// QUESTION: We also probably shouldn't count on read reading all the bytes either
	else if (read(fd, st_data_get(result), st_avail_get(result)) != info.st_size) {
		log_info("Could not read the entire file %s. {errno = %i & strerror = %s}", name, errno,
				(strerror_r(errno, estring, 1024) == 0 ? estring : "Unknown error"));
		close(fd);
		return NULL;
	}

	st_length_set(result, info.st_size);
	close(fd);

	return result;
}

/**
 * @brief	Get a file handle to a temporary file created in a specified directory.
 * @param	pdir		the parent directory in which to create the temporary file, or NULL for the default spool dir.
 * @param	tmpname		an optional pointer to a managed string to receive the name of the created temp file.
 * @return	-1 on failure or the new temporary file's file descriptor on success.
 */
int_t get_temp_file_handle(chr_t *pdir, stringer_t **tmpname) {

	int_t result;
	stringer_t *opath, *path, *sp = NULL;

	if (!pdir && !(sp = spool_path(MAGMA_SPOOL_BASE))) {
		log_pedantic("Failed to retrieve spool directory to create temp file.");
		return -1;
	}

	if (sp) {
		opath = sp;
	} else {
		opath = st_import(pdir,ns_length_get(pdir));
	}

	if (!opath) {
		log_pedantic("Failed to create temp file with invalid parent directory.");
		return -1;
	}

	// We have to do this because spool_path() doesn't return the type of string we need for appending.
	if (!(path = st_dupe_opts(MANAGED_T | JOINTED | HEAP, opath))) {
			log_pedantic("Could not allocate a buffer for temp file name.");
			st_free(opath);
			return -1;
		}

	st_free(opath);

	if (!(path = st_append(path, CONSTANT("/XXXXXX")))) {
		log_pedantic("Error allocating space for temporary file name.");
		st_free(path);
		return -1;
	}

	if ((result = mkstemp(st_char_get(path))) < 0) {
		log_pedantic("Failed to acquire temporary file handle with mkstemp. { %s }.", st_char_get(path));
		st_free(path);
		return -1;
	}

	if (tmpname) {
		*tmpname = st_dupe(path);
	}

	st_free(path);
	return result;
}


/**
 * @brief	Determine whether a given filename or directory path is accessible by a user.
 * @param	path	a null-terminated string with the name of the filename or directory to be checked for existence/access.
 * @return	true if the pathname exists and is readable, or false otherwise.
 */
bool_t file_accessible(const chr_t *path) {

	return (access(path, R_OK) == 0);
}

/**
 * @brief	Determine whether a given filename or directory path is readable and writable by a user.
 * @param	path	a null-terminated string with the name of the filename or directory to be checked for permissions.
 * @return	true if the pathname exists and is readable and writable, or false otherwise.
 */
bool_t file_readwritable(const chr_t *path) {

	return (access(path, R_OK|W_OK) == 0);
}

/**
 * @brief	Determine whether a given filename or directory path is readable or writable by other users.
 * @param	path	a null-terminated string with the name of the filename or directory to be checked for world permissions.
 * @return	true if the pathname exists and is readable or writable by others, or false otherwise.
 */
bool_t file_world_accessible(const chr_t *path) {
	struct stat sb;

	if (stat(path, &sb)) {
		return false;
	}

	return ((sb.st_mode & (S_IROTH|S_IWOTH)) != 0);
}
