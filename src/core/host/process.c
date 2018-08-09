
/**
 * @file /magma/core/host/process.c
 *
 * @brief	Functions for managing processes.
 */

#include "magma.h"

/**
 * @brief	Return the current process identifier using appropriate function for the current system.
 */
pid_t process_my_pid(void) {
	return getpid();
}

/**
 * @brief	Find the  process identifier for a named executable and return it.
 * @note	Only returns the first match, and will never return the process identifier for the currently running process.
 * @param	name	the name of the executable to find.
 * @return 0 if the process was not found, otherwise the a process identifier is returned.
 */
pid_t process_find_pid(stringer_t *name) {

	DIR *dir;
	struct dirent *entry;
	pid_t pid = 0, ret = 0;
	chr_t cmd[MAGMA_FILEPATH_MAX + 1];
	stringer_t *compare = MANAGEDBUF(1024);

	if (!(dir = opendir(MAGMA_PROC_PATH))) {
		log_pedantic("The system path could not be opened. { path = %s / %s }",
				MAGMA_PROC_PATH, strerror_r(errno, MEMORYBUF(1024), 1024));
		return 0;
	}

	while ((entry = readdir(dir)) && !ret) {
		if (entry->d_type == DT_DIR && chr_numeric((uchr_t)*(entry->d_name)) && int32_conv_ns(entry->d_name, &pid) && pid != process_my_pid()) {
			if (snprintf(cmd, MAGMA_FILEPATH_MAX + 1, "%s/%i/comm", MAGMA_PROC_PATH, pid) && file_read(cmd, compare) > 0) {
				st_trim(compare);
				if (!st_cmp_cs_eq(compare, name)) {
					ret = pid;
				}
			}

		}
	}

	closedir(dir);

	return ret;
}

/**
 * @brief	Kill a named process with the specified signal, and retry if necessary.
 * @param	name	the name of the process to kill (matches any process that starts with the specified name).
 * @param	signum	the signal number to be sent to the matching process.
 * @param	wait	the number of times to re-send the signal, with one second rest intervals in between.
 * @return -2 for a generic failure, -1 on timeout, 0 if process not found, and 1 if the process was successfully killed.
 */
int_t process_kill(stringer_t *name, int_t signum, int_t wait) {

	DIR *dir;
	int_t ret;
	struct dirent *entry;
	pid_t pid, killed[1024];
	//TODO
		chr_t cmd[MAGMA_FILEPATH_MAX + 1];
	uint_t matches = 0, exited = 0;
	stringer_t *compare = MANAGEDBUF(1024);

	if (!(dir = opendir(MAGMA_PROC_PATH))) {
		log_pedantic("The system path could not be opened. { path = %s / %s }",
				MAGMA_PROC_PATH, strerror_r(errno, MEMORYBUF(1024), 1024));
		return -2;
	}

	while ((entry = readdir(dir)) && (matches < (sizeof (killed) / sizeof (pid_t)))) {

		if (entry->d_type == DT_DIR && chr_numeric((uchr_t)*(entry->d_name)) && int32_conv_ns(entry->d_name, &pid) && pid != process_my_pid()) {

			// Since the cmdline file could contain the command arguments as a NULL separated array we have to wrap compare with a NULLER
			// to exclude those arguments.
			if (snprintf(cmd, MAGMA_FILEPATH_MAX + 1, "%s/%i/cmdline", MAGMA_PROC_PATH, pid) && file_read(cmd, compare) > 0 &&
				!st_cmp_cs_starts(st_swap(compare, '\0', ' '), name)) {
				if ((ret = kill(pid, signum))) {
					log_pedantic("The process could not be signaled. { signum = %i / %s }", signum, strerror_r(errno, MEMORYBUF(1024), 1024));
					return -2;
				}
				else {
					killed[matches++] = pid;
				}

			}
		}
	}

	closedir(dir);

	/***
	 * @warning Check to see if the victims are dead. Note that if a process ID is reused before we detect
	 * detect the victim's death the new process will be signaled inadvertently.
	 */
	for (uint_t i = 0; i < wait && matches != exited; i++) {
		for (uint_t j = 0; j < matches; j++) {
			if (killed[j] != -1 && kill(killed[j], 0) && errno == ESRCH) {
				log_pedantic("%.*s (%i) killed", st_length_int(name), st_char_get(name), killed[j]);
				killed[j] = -1;
				exited++;
			}
		}

		if (matches != exited) {
			sleep(1);
		}
	}

	// One of the victims didn't finish.
	if (wait > 0 && exited != matches) {
		for (uint_t j = 0; j < matches; j++) {
			if (killed[j] != -1) {
				log_pedantic("%.*s (%i) refused to die", st_length_int(name), st_char_get(name), killed[j]);
			}
		}
		return -1;
	}

	// Nothing found.
	if (!matches) {
		return 0;
	}

	return 1;
}
