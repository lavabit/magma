
/**
 * @file /magma/magma.c
 *
 * @brief	The main executable entry point.
 */

#include "magma.h"

int main(int argc, char *argv[]) {

	pid_t pid = 0;

	if ((pid = process_pid(PLACER("/usr/libexec/magmad", 19))) != 0) {
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
	exit(EXIT_SUCCESS);
}

