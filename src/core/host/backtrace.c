
/**
 * @file /magma/src/core/host/backtrace.c
 *
 * @brief A simple function for dumping a stack trace to standard output.
 */

#include "magma.h"

/**
 * @brief	Print the current stack backtrace to stdout.
 * @note	This function was created because backtrace_symbols() can fail due to heap corruption.
 * @return	-1 if the backtrace failed, or 0 on success.
 */
int_t backtrace_print(void) {

#if (__linux__ && __GLIBC__) || __APPLE__
  int_t pipefds[2];
	char strbuf[1024];
	void *buffer[1024];
	int_t nbt, nread, nfound = 0, result = 0, i;

	nbt = backtrace (buffer, (sizeof (buffer) / sizeof (void *)));

	if (nbt < 0) {
		return -1;
	}

	// Create a pipe to read back the dumped line numbers.
	if (pipe(pipefds) < 0) {
		return -1;
	}

	backtrace_symbols_fd(buffer,nbt,pipefds[1]);

	if (write(STDOUT_FILENO, "   ", 3) != 3) {
		return -1;
	}

	while (nfound < nbt) {
		nread = read(pipefds[0], strbuf, sizeof(strbuf));

		if (nread < 0) {
			result = -1;
			break;
		}
		else if (!nread) {
			break;
		}

		for (i = 0; i < nread; i++) {
			if (write (STDOUT_FILENO, &strbuf[i], 1) != 1) {
				return -1;
			}

			if (strbuf[i] == '\n') {
				nfound++;

				if (nfound != nbt) {
					if (write(STDOUT_FILENO, "   ", 3) != 3) {
						return -1;
					}
				}
			}
		}
	}

	fsync(STDOUT_FILENO);
	close(pipefds[0]);
	close(pipefds[1]);

	return result;
#else
	return 0;
#endif

}
