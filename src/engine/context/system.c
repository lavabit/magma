
 /**
 * @file /magma/engine/context/system.c
 *
 * @brief	Functions used to interface with and configure the operating system.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Get the hard system limit for a specified resource.
 * @note	This function will never return a value greater than UINT64_MAX.
 * @see		getrlimit64()
 * @param	resource	the system rlimit resource identifier to be queried.
 * @return	-1 on failure, or the system hard limit of the specified resource identifier on success.
 */
uint64_t system_limit_max(int_t resource) {

	int_t ret;
	struct rlimit64 limits = { 0, 0 };

	if ((ret = getrlimit64(resource, &limits))) {
		log_info("Unable to retrieve the resource limit. {resource = %i / return = %i / error = %s}", resource, ret, strerror_r(errno, bufptr, buflen));
		return -1;
	}

	if (limits.rlim_max > UINT64_MAX) {
		log_pedantic("The requested resource has a maximum value that exceeds the range of possible of return values. Returning the maximum possible value instead. "
			"{resource = %i / actual = %lu / returning = %lu}", resource, limits.rlim_max, UINT64_MAX);
		return UINT64_MAX;
	}

	return (uint64_t)limits.rlim_max;
}

/**
 * @brief	Get the soft system limit for a specified resource.
 * @note	This function will never return a value greater than UINT64_MAX.
 * @see		getrlimit64()
 * @param	resource	the system rlimit resource identifier to be queried.
 * @return	-1 on failure, or the system soft limit of the specified resource identifier on success.
 */
uint64_t system_limit_cur(int_t resource) {

	int_t ret;
	struct rlimit64 limits = { 0, 0 };

	if ((ret = getrlimit64(resource, &limits))) {

		log_info("Unable to retrieve the resource limit. {resource = %i / return = %i / error = %s}", resource, ret, strerror_r(errno, bufptr, buflen));
		return -1;
	}

	if (limits.rlim_cur > UINT64_MAX) {
		log_pedantic("The requested resource is currently set to a value that exceeds the range of possible of return values. Returning the maximum possible value instead. "
			"{resource = %i / actual = %lu / returning = %lu}", resource, limits.rlim_cur, UINT64_MAX);
		return UINT64_MAX;
	}

	return (uint64_t)limits.rlim_cur;
}

/**
 * @brief	Perform a chroot() on the directory specified in the config option magma.system.root_directory, if it is set.
 * @return	true on success or false on failure.
 */
bool_t system_change_root_directory(void) {

	if (magma.system.root_directory && chroot(magma.system.root_directory)) {
		log_info("Could not jail the process inside %s. {%s}", magma.system.root_directory,
				strerror_r(errno, bufptr, buflen));
		return false;
	}

	return true;
}

/**
 * @brief	Daemonize into the background, if the magma.system.daemonize config option is set.
 * @return	true inside the child process, or false inside the parent process or if an error occurs.
 */
bool_t system_fork_daemon(void) {

	pid_t pid;

	// If requested, fork into different processes and release the console session.
	if (magma.system.daemonize) {

		if ((pid = fork()) == -1) {
			log_info("Could not fork a background daemon process.");
			return false;
		}

		// We are the parent process.
		if (pid != 0) {
			exit(0);
		}

		// Make this the session group leader.
		if (setsid() < 0) {
			log_info("Could not become the session group leader.");
			return false;
		}

		// If file based logging is not enabled we'll need to close stdout/stderr so we don't retain a link to the console session.
		if (freopen64("/dev/null", "a+", stdout) == NULL) {
			return false;
		}
		if (freopen64("/dev/null", "a+", stderr) == NULL) {
			fclose(stdout);
			return false;
		}
		fclose(stdin);

		// Since the process description may have changed we need to refresh it.
		status_process();
	}

	return true;
}

/**
 * @brief	Set the process umask for new file/dir creation to O_RDWR.
 * @return	true if the umask was set successfully, or false on failure.
 */
bool_t system_init_umask(void) {

	// Attempt to set the process mask.
	umask(O_RDWR);

	// Verify that the mask was set by calling the function again.
	if ((O_RDWR) != umask(O_RDWR)) {
		log_info("Could not set the process umask { error = %s }", strerror_r(errno, bufptr, buflen));
		return false;
	}

	return true;
}

/**
 * @brief	Set the magma core dump size rlimit, if magma.system.enable_core_dumps was enabled.
 * @return	true if core dumps were successfully enabled or false on failure.
 */
bool_t system_init_core_dumps(void) {

	struct rlimit64 limits = {
		.rlim_cur = magma.system.core_dump_size_limit,
		.rlim_max = magma.system.core_dump_size_limit
	};

	if (magma.system.enable_core_dumps && setrlimit64(RLIMIT_CORE, &limits)) {
		log_info("The system does not allow core dumps. {%s}", strerror_r(errno, bufptr, buflen));
		return false;
	}

	return true;
}

/**
 * @brief	Set process privileges to run as a specified user if magma.system.impersonate_user is set.
 * @note	This function will set the user id and group id to the specified user, and chdir() to their home directory.
 * @return	true on success or if magma.system.impersonate_user is not set; false on failure to change privileges.
 */
bool_t system_init_impersonation(void) {

	int err;
	char *pwnam;
	size_t pwnam_len;
	struct passwd user, *result;

	if (magma.system.impersonate_user) {

		mm_wipe(&user, sizeof(struct passwd));

		if ((pwnam_len = sysconf(_SC_GETPW_R_SIZE_MAX)) == -1) {
			log_info("Unable to determine the required buffer size for the getpwnam_r function. {%s}", strerror_r(errno, bufptr, buflen));
			return false;
		} else if (!(pwnam = mm_alloc(pwnam_len))) {
			log_info("Unable to allocate the buffer required for the getpwnam_r function.");
			return false;
		}
		// Pull the user information.
		else if ((err = getpwnam_r(magma.system.impersonate_user, &user, pwnam, pwnam_len, &result)) || !result) {

			if (!result) {
				log_info("The user account %s does not exist.", magma.system.impersonate_user);
			} else {
				log_info("Unable to retrieve information for the user account %s. {%s}", magma.system.impersonate_user,
					strerror_r(errno, bufptr, buflen));
			}

			mm_free(pwnam);
			return false;
		}
		// Change into the user's home directory.
		else if (chdir(user.pw_dir)) {
			log_info("Unable to change into the %s directory which is the home for the user %s. {%s}", user.pw_dir, magma.system.impersonate_user,
				strerror_r(errno, bufptr, buflen));
			mm_free(pwnam);
			return false;
		}
		// Assume the proper group permissions.
		else if (getgid() != user.pw_gid && setgid(user.pw_gid)) {
			log_info("Unable to assume the group id %i. {%s}", user.pw_gid,
				strerror_r(errno, bufptr, buflen));
			mm_free(pwnam);
			return false;
		}
		// Begin impersonating the user.
		else if (getuid() != user.pw_uid && setuid(user.pw_uid)) {
			log_info("Unable to begin impersonating the user %s. {%s}", magma.system.impersonate_user,
				strerror_r(errno, bufptr, buflen));
			mm_free(pwnam);
			return false;
		}

		mm_free(pwnam);

		// Since the process description may have changed we need to refresh it.
		status_process();
	}

	return true;
}

/**
 * @brief	Increase process resource limits, if magma.system.increase_resource_limits is set.
 * @note	Resource limits will be maximized for magma's virtual address space, data and stack segments, available file descriptors and
 * 			sub-processes, and allowed file sizes.
 * 			If output_resource_limits is enabled, the state of the process resource limits will be dumped to the log afterwards.
 * @return	This function always returns true (errors are logged).
 */
bool_t system_init_resource_limits(void) {

	struct rlimit64 limits = {
		.rlim_cur = RLIM64_INFINITY,
		.rlim_max = RLIM64_INFINITY
	};

	if (magma.system.increase_resource_limits) {

		// Address Space
		if (setrlimit64(RLIMIT_AS, &limits) && magma.config.output_resource_limits) {
			log_info("Unable to increase the address space limit. {%s}", strerror_r(errno, bufptr, buflen));
		}

		// Data Segment
		if (setrlimit64(RLIMIT_DATA, &limits) && magma.config.output_resource_limits) {
			log_info("Unable to increase the data segment limit. {%s}", strerror_r(errno, bufptr, buflen));
		}

		// Stack Size
		if (setrlimit64(RLIMIT_STACK, &limits) && magma.config.output_resource_limits) {
			log_info("Unable to increase the stack size limit. {%s}", strerror_r(errno, bufptr, buflen));
		}

		// File Size
		if (setrlimit64(RLIMIT_FSIZE, &limits) && magma.config.output_resource_limits) {
			log_info("Unable to increase the file size limit. {%s}", strerror_r(errno, bufptr, buflen));
		}

		// Number of Threads/Processes
		if (setrlimit64(RLIMIT_NPROC, &limits) && magma.config.output_resource_limits) {
			log_info("Unable to increase the thread limit. {%s}", strerror_r(errno, bufptr, buflen));
		}

		// File Descriptors
		// The command '/sbin/sysctl -a' is indicating a kernel max of 344021 (fs.file-max).
		limits.rlim_cur = limits.rlim_max = 262144;

		if (setrlimit64(RLIMIT_NOFILE, &limits) && magma.config.output_resource_limits) {
			log_info("Unable to increase the file descriptor limit. {%s}", strerror_r(errno, bufptr, buflen));
		}

	}

	if (magma.config.output_resource_limits) {

		log_info("\n\nResource Limits\n--------------------------------------------------------------------------------");

		// Core Dumps
		if (getrlimit64(RLIMIT_CORE, &limits)) {
			log_info("Unable to retrieve the core dump size limit. {%s}", strerror_r(errno, bufptr, buflen));
		} else if (limits.rlim_max == RLIM64_INFINITY) {
			log_info("RLIMIT_CORE = RLIM64_INFINITY");
		} else {
			log_info("RLIMIT_CORE = %ld", limits.rlim_max);
		}

		// Address Space
		if (getrlimit64(RLIMIT_AS, &limits)) {
			log_info("Unable to retrieve the address space limit. {%s}", strerror_r(errno, bufptr, buflen));
		} else if (limits.rlim_max == RLIM64_INFINITY) {
			log_info("RLIMIT_AS = RLIM64_INFINITY");
		} else {
			log_info("RLIMIT_AS = %ld", limits.rlim_max);
		}

		// Data Segment
		if (getrlimit64(RLIMIT_DATA, &limits)) {
			log_info("Unable to retrieve the data segment limit. {%s}", strerror_r(errno, bufptr, buflen));
		} else if (limits.rlim_max == RLIM64_INFINITY) {
			log_info("RLIMIT_DATA = RLIM64_INFINITY");
		} else {
			log_info("RLIMIT_DATA = %ld", limits.rlim_max);
		}

		// Stack Size
		if (getrlimit64(RLIMIT_STACK, &limits)) {
			log_info("Unable to retrieve the stack size limit. {%s}", strerror_r(errno, bufptr, buflen));
		} else if (limits.rlim_max == RLIM64_INFINITY) {
			log_info("RLIMIT_STACK = RLIM64_INFINITY");
		} else {
			log_info("RLIMIT_STACK = %ld", limits.rlim_max);
		}

		// File Size
		if (getrlimit64(RLIMIT_FSIZE, &limits)) {
			log_info("Unable to retrieve the file size limit. {%s}", strerror_r(errno, bufptr, buflen));
		} else if (limits.rlim_max == RLIM64_INFINITY) {
			log_info("RLIMIT_FSIZE = RLIM64_INFINITY");
		} else {
			log_info("RLIMIT_FSIZE = %ld", limits.rlim_max);
		}

		// Number of Threads/Processes
		if (getrlimit64(RLIMIT_NPROC, &limits)) {
			log_info("Unable to retrieve the thread limit. {%s}", strerror_r(errno, bufptr, buflen));
		} else if (limits.rlim_max == RLIM64_INFINITY) {
			log_info("RLIMIT_NPROC = RLIM64_INFINITY");
		} else {
			log_info("RLIMIT_NPROC = %ld", limits.rlim_max);
		}

		// File Descriptors
		if (getrlimit64(RLIMIT_NOFILE, &limits)) {
			log_info("Unable to retrieve the file descriptor limit. {%s}", strerror_r(errno, bufptr, buflen));
		} else if (limits.rlim_max == RLIM64_INFINITY) {
			log_info("RLIMIT_NOFILE = RLIM64_INFINITY");
		} else {
			log_info("RLIMIT_NOFILE = %ld", limits.rlim_max);
		}

	}

	return true;
}
