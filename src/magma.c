
/**
 * @file /magma/magma.c
 *
 * @brief	The main executable entry point.
 */

#include "magma.h"

extern FILE *log_descriptor;

int main(int argc, char *argv[]) {

	pid_t pid = 0;

	if ((pid = process_find_pid(PLACER("/usr/libexec/magmad", 19))) != 0) {
		log_error("Another instance of magma is already running. {pid = %i}", pid);
		exit(EXIT_FAILURE);
	}

	// Overwrites the default config file location with the value provided on the command line.
	if (!args_parse(argc, argv)) {
		exit(EXIT_FAILURE);
	}

	else if (!process_start()) {
		status_set(-1);
		process_stop();
		exit(EXIT_FAILURE);
	}

	// Do something for awhile.
	net_listen();

	process_stop();

	// We used a log file handle, we need to close it.
	if (log_descriptor) fclose(log_descriptor);

	// Close the console descriptors, if they are still valid.
	if ((errno = 0) || (fcntl(STDIN_FILENO, F_GETFL) != -1 && errno != EBADF)) close(STDIN_FILENO);
	if ((errno = 0) || (fcntl(STDOUT_FILENO, F_GETFL) != -1 && errno != EBADF)) close(STDOUT_FILENO);
	if ((errno = 0) || (fcntl(STDERR_FILENO, F_GETFL) != -1 && errno != EBADF)) close(STDERR_FILENO);

	exit(EXIT_SUCCESS);
}

