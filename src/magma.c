
/**
 * @file /magma/magma.c
 *
 * @brief	The main executable entry point.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

int main(int argc, char *argv[]) {

	/*if (process_kill(PLACER("magmad", 6), SIGTERM, 10) < 0) {
		log_error("Another instance of the Magma Daemon is already running and refuses to die.");
		exit(EXIT_FAILURE);
	}*/

	// Updates the location of the config file if it was specified on the command line.
	if (!args_parse(argc, argv)) {
		exit(EXIT_FAILURE);
	}

	else if (!process_start()) {
		status_set(-1);
		process_stop();
		exit(EXIT_FAILURE);
	}

#ifdef MAGMA_PEDANTIC
	// Only during development...
	cache_flush();
#endif

	// Do something for awhile.
	net_listen();

	process_stop();
	exit(EXIT_SUCCESS);
	return EXIT_SUCCESS;
}

