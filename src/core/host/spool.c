
/**
 * @file /magma/core/host/spool.c
 *
 * @brief	Functions for checking, creating, maintaining and using the spool.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @note	We have to track errors locally so these functions can be used during startup and shutdown when the global statistics system may not be available.
 */
static stringer_t *spool_base = NULL;
static uint64_t spool_files_cleaned = 0, spool_errors = 0;
static time_t spool_check_failure = 0, spool_creation_failure = 0;
static pthread_rwlock_t spool_creation_lock = PTHREAD_RWLOCK_INITIALIZER;
static pthread_mutex_t spool_error_lock = PTHREAD_MUTEX_INITIALIZER,
spool_check_lock = PTHREAD_MUTEX_INITIALIZER;


/**
 * @brief	Get the number of spool errors encountered.
 * @return	the total number of spool errors.
 */
// LOW: Add files created, and include files cleaned.
uint64_t spool_error_stats(void) {
	uint64_t result;
	mutex_get_lock(&spool_error_lock);
	result = spool_errors;
	mutex_unlock(&spool_error_lock);
	return result;
}

/**
 * @brief	Get the full path to a requested spool directory.
 * @param	the spool id (MAGMA_SPOOL_BASE, MAGMA_SPOOL_DATA, or MAGMA_SPOOL_SCAN); defaults to MAGMA_SPOOL_DATA.
 * @return	NULL on failure, or a managed string pointing to the requested path inside the magma spool parent directory,
 * 			or /tmp/magma if the spool isn't configured.
 */
stringer_t * spool_path(int_t spool) {

	chr_t *folder;
	stringer_t *result;

	// Use the spool value to determine the folder.
	if (spool == MAGMA_SPOOL_BASE) {
		folder = "";
	}
	else if (spool == MAGMA_SPOOL_SCAN) {
		folder = "scan/";
	}
	else {
		folder = "data/";
	}

	// If the spool directory isn't configured, fall back to using /tmp/magma/ instead.
	if (magma.spool) {
		result = st_merge_opts(NULLER_T | CONTIGUOUS | HEAP, "nnn", magma.spool, (*(magma.spool + ns_length_get(magma.spool) - 1) == '/' ? "" : "/"), folder);
	}
	else {
		result = st_merge_opts(NULLER_T | CONTIGUOUS | HEAP, "nn", "/tmp/magma/", folder);
	}

	return result;
}

/**
 * @brief	Check to see if the specified spool directory exists.
 * @param	path	a managed string with the spool directory to be checked.
 * @return	0 if the specified spool path exists, 1 if it was created, or -1 on error.
 */
int_t spool_check(stringer_t *path) {

	time_t now;
	int_t result = 0;

	mutex_get_lock(&spool_check_lock);
	if (folder_exists(path, false) < 0) {

		// We only record errors once per hour, otherwise we'd fill the log file up.
		if ((result = folder_exists(path, true)) < 0 && ((now = time(NULL)) - spool_check_failure) > 3600) {
			log_critical("Unable to access the spool directory. {mkdir = %i / path = %.*s}", result, st_length_int(path), st_char_get(path));
			spool_check_failure = now;
		}
#ifdef MAGMA_PEDANTIC
		else if (result == 1) {
			log_info("The spool directory was created successfully. {path = %.*s}", st_length_int(path), st_char_get(path));
		}
#endif
	}
	mutex_unlock(&spool_check_lock);

	// Error tracking - this depends on the function having a single return point.
	if (result) {
		mutex_get_lock(&spool_error_lock);
		spool_errors++;
		mutex_unlock(&spool_error_lock);
	}

	return result;
}

/**
 * @brief	Create a temporary file in a specified spool directory.
 * @note	The temp file is automatically unlinked as soon as it is created.
 * @param	spool	the spool directory id in which the temp file will be stored (MAGMA_SPOOL_BASE, MAGMA_SPOOL_DATA, or MAGMA_SPOOL_SCAN).
 * @param	prefix	an optional prefix for the temp file name ("magma" will be used if prefix is NULL0.
 * @return	-1 on failure or the file descriptor to the newly created temp file on success.
 */
int_t spool_mktemp(int_t spool, chr_t *prefix) {

	time_t now;
	int_t fd, err_info;
	stringer_t *path, *template, *base = NULL;

	if (!prefix) prefix = "magma";

	// We use the creation lock to allow simultaneous temporary file creation, but prevent the cleanup function from running
	// during the short window between when a file is created and we unlink it ourselves.
	rwlock_lock_read(&spool_creation_lock);

	// Build the a template that includes the thread id and a random number to make the resulting file path harder to predict and try creating the temporary file handle.
	// The O_EXCL+O_CREAT flags ensure we create the file or the call fails, O_SYNC indicates synchronous IO, and O_NOATIME eliminates access time tracking.
	if ((path = spool_path(spool)) && (template = st_aprint("%.*s%s_%lu_%lu_XXXXXX", st_length_int(path), st_char_get(path), prefix, thread_get_thread_id(), rand_get_uint64()))
		&& (fd = mkostemp(st_char_get(template), O_EXCL | O_CREAT | O_RDWR | O_SYNC | O_NOATIME)) < 0) {

		// Verify that the spool directory directory tree is valid. If any of the directories are missing, this will try and create them.
		if ((base = spool_path(MAGMA_SPOOL_BASE)) && !spool_check(base) && !spool_check(path)) {

			// We need to generate a new file template since the first mkostemp may have overwritten the required X characters.
			st_free(template);

			if ((template = st_aprint("%.*s%s_%lu_%lu_XXXXXX", st_length_int(path), st_char_get(path), prefix, thread_get_thread_id(), rand_get_uint64()))
				&& (fd = mkostemp(st_char_get(template), O_EXCL | O_CREAT | O_RDWR | O_SYNC | O_NOATIME)) < 0) {

				// Store the errno.
				err_info = errno;

				// If the path is valid, but were still failing, record an error message in the log file, but only once an hour.
				mutex_get_lock(&spool_error_lock);
				if (((now = time(NULL)) - spool_creation_failure) > 3600) {
					log_critical("Temporary file creation failed. {mkostemp = %i / errno = %i / strerror = %s / file = %.*s}", fd, err_info, strerror_r(err_info, MEMORYBUF(1024), 1024),
						st_length_int(template), st_char_get(template));
					spool_creation_failure = now;
				}
				mutex_unlock(&spool_error_lock);
			}
		}
	}

	// LOW: Place an advisory/mandatory using fcntl(F_SETLK).
	// LOW: Acquire a file lease, so we get notified if another process ignores our advisory lock and opens the file. Use fcntl(F_SETLEASE, F_WRLCK).

	// We need to explicitly unlink the file or the data will remain after the application exits.
	if (fd >= 0 && unlink(st_char_get(template)) != 0) {
		log_pedantic("A temporary file has been created, but could not be unlinked. {unlink = %i / file = %.*s}", errno, st_length_int(template), st_char_get(template));
	}

	rwlock_unlock(&spool_creation_lock);

	if (template) st_free(template);
	if (path) st_free(path);
	if (base) st_free(base);

	// Error tracking - this depends on the function having a single return point.
	if (fd < 0) {
		mutex_get_lock(&spool_error_lock);
		spool_errors++;
		mutex_unlock(&spool_error_lock);
	}

	return fd;
}

/**
 * @brief	An internal function used by the ftw() function to cleanup the file contents of a spool directory.
 * @param	file	a pointer to a null-terminated string containing the pathname of the spool file.
 * @param	info	a pointer to a stat object containing the filesystem info of the specified file.
 * @param	the ftw type flag of the specified file (only FTW_F is handled).
 * @return	This function always returns 0.
 */
int_t spool_check_file(const char *file, const struct stat *info, int type) {

	// Development builds should overlook the ".empty" files used to force Mercurial into creating the spool directory structure.
#ifdef MAGMA_PEDANTIC
	if (type == FTW_F && !st_cmp_cs_eq(NULLER(basename(file)), PLACER(".empty", 6)) && info->st_size == 0) {
		return 0;
	}
#endif

	if (type == FTW_F) {
		if (unlink(file)) {
			log_error("An error occurred while trying to unlink a temporary file inside the spool. {%s / %s}", strerror_r(errno, bufptr, buflen), file);
			mutex_get_lock(&spool_error_lock);
			spool_errors++;
			mutex_unlock(&spool_error_lock);
		}
		else {
			spool_files_cleaned++;
		}
	}

	return 0;
}

/**
 * @brief	Clean up the file contents in the spool base directory.
 * @return	-1 on error or the number of files that were purged from the spool base directory.
 */
int_t spool_cleanup(void) {

	int_t result;
	stringer_t *base;
	uint64_t before = spool_files_cleaned;

	if (!(base = spool_path(MAGMA_SPOOL_BASE))) {
		log_error("Unable to build the spool path.");
		return -1;
	}

	rwlock_lock_write(&spool_creation_lock);
	result = ftw(st_char_get(base), spool_check_file, 32);
	rwlock_unlock(&spool_creation_lock);

	// Non-zero return values from ftw trigger the return value -1, otherwise we calculate the number of files cleaned and return that value instead.
	if (result) {
		log_error("Unable to traverse the spool directory. {path = %.*s}", st_length_int(base), st_char_get(base));
		result = -1;
	}
#ifdef MAGMA_PEDANTIC
	else if ((result = spool_files_cleaned - before)) {
		log_pedantic("%i files needed to be purged from the spool.", result);
	}
#else
	else {
		result = spool_files_cleaned - before;
	}
#endif

	st_free(base);
	return result;
}

/**
 * @brief	Clean up the file contents in the spool base directory.
 * @note	This is like a fast version of spool_cleanup().
 * @return	This function returns no value.
 */
void spool_stop(void) {

	uint64_t before = spool_files_cleaned;

	// Since spool_cleanup pulls the base path from the current config, we skip straight to the traversal logic using the base path we stored during startup.
	if (spool_base && ftw(st_char_get(spool_base), spool_check_file, 32)) {
		log_pedantic("Unable to clean the spool directory. {path = %.*s}", st_length_int(spool_base), st_char_get(spool_base));
	}

	if (spool_base) {
		st_free(spool_base);
		spool_base = false;
	}

#ifdef MAGMA_PEDANTIC
	if (spool_files_cleaned - before > 0) log_pedantic("%lu files needed to be purged from the spool.", spool_files_cleaned - before);
#endif

	return;
}

/**
 * @brief	Create and check the spool directories, and clean them for use.
 * @return	true on success or false on failure.
 */
bool_t spool_start(void) {

	stringer_t *path;
	bool_t result = true;

	// Make sure the spool folder path is valid. If necessary, create any subfolders that are missing.
	for (int_t i = 0; i < 3 && result == true; i++) {

		// Generate the appropriate path string.
		// QUESTION: Why not just path = spool_path(i) ?
		if (i == MAGMA_SPOOL_BASE)
			path = spool_path(MAGMA_SPOOL_BASE);
		else if (i == MAGMA_SPOOL_DATA)
			path = spool_path(MAGMA_SPOOL_DATA);
		else if (i == MAGMA_SPOOL_SCAN)
			path = spool_path(MAGMA_SPOOL_SCAN);

		if (path) {
			if (spool_check(path)) {
				log_critical("Spool path is invalid. {path = %.*s}", st_length_int(path), st_char_get(path));
				result = false;
			}

			st_free(path);
			path = NULL;
		}
		else {
			log_critical("Spool location is invalid.");
			result = false;
		}
	}

	// We store the base location outside of the config structure since spool_stop needs to be called after the config is freed. And we
	if (result && !(spool_base = spool_path(MAGMA_SPOOL_BASE))) {
		log_critical("Unable to remove stale files found inside the spool directory.");
		result = false;
	}
	// Note that if the cleanup function fails spool_base is freed and set to NULL which will prevent spool_stop from attempting to cleanup the spool during shutdown.
	else if (spool_cleanup() < 0) {
		log_critical("Unable to remove stale files found inside the spool directory. {path = %.*s}", st_length_int(spool_base), st_char_get(spool_base));
		st_free(spool_base);
		spool_base = NULL;
		result = false;
	}

	// Error tracking - this depends on the function having a single return point.
	if (!result) {
		mutex_get_lock(&spool_error_lock);
		spool_errors++;
		mutex_unlock(&spool_error_lock);
	}

	return result;
}
