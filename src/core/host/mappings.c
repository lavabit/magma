
/**
 * @file /magma/core/host/mappings.c
 *
 * @brief	Map numeric system values to their string equivalents.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Translate an error number into a descriptive, human-readable message.
 * @param	error	the error number (errno) to be looked up.
 * @param	buffer	a pointer to a character buffer that will store the error description string.
 * @param	length	the length, in bytes, of the output buffer, which should be at minimum of 32 bytes.
 * @return	a pointer to a null-terminated string containing the descriptive error message.
 */
chr_t * errno_name(int errnum, char *buffer, size_t length) {

	if (!buffer || !length) {
		log_pedantic("Invalid output buffer supplied.");
	}
	else if (errnum < _sys_nerr && _sys_errlist[errnum]) {
		snprintf(buffer, length, "%s", _sys_errlist[errnum]);
	}
	else if (errnum == 41) {
		snprintf(buffer, length, "EWOULDBLOCK");
	}
	else if (errnum == 58) {
		snprintf(buffer, length, "EDEADLOCK");
	}
	else {
		log_pedantic("Asked to identify an error outside the legal range. { errno = %i / nerr = %i / name = UNKNOWN }", errnum, _sys_nerr);
		snprintf(buffer, length, "UNKNOWN");
	}

	return buffer;
}

/**
 * @brief	Translate a numeric signal into a human readable name.
 * @param	signal	the input signal number.
 * @param	buffer	a buffer to receive the signal description.
 * @param	length	the length of the buffer to receive the output, which should be greater than 32 bytes.
 * @return	a pointer to a null-terminated string containing the textual name of the specified signal.
 */
chr_t * signal_name(int signal, char *buffer, size_t length) {

	if (!buffer || !length) {
		log_pedantic("Invalid output buffer supplied.");
	}
	else if (signal < SIGUNUSED) {
		snprintf(buffer, length, "%s", sys_siglist[signal]);
	}
	else if (signal == SIGRTMIN) {
		snprintf(buffer, length, "SIGRT");
	}
	else if (signal > SIGRTMIN && signal <= SIGRTMAX) {
		snprintf(buffer, length, "SIGRT+%i", signal - SIGRTMIN);
	}
	else if (signal < NSIG) {
		snprintf(buffer, length, "SIGUNUSED");
	}
	else {
		log_pedantic("Asked to identify a signal outside legal range. { signal = %i / nsig = %i / name = UNKNOWN }", signal, SIGRTMIN);
		snprintf(buffer, length, "UNKNOWN");
	}

	return buffer;

#if 0
	switch (signal) {
	case (SIGHUP):
		snprintf(buffer, length, "%s", "SIGHUP");
		break;
	case (SIGINT):
		snprintf(buffer, length, "%s", "SIGINT");
		break;
	case (SIGQUIT):
		snprintf(buffer, length, "%s", "SIGQUIT");
		break;
	case (SIGILL):
		snprintf(buffer, length, "%s", "SIGILL");
		break;
	case (SIGTRAP):
		snprintf(buffer, length, "%s", "SIGTRAP");
		break;
	case (SIGABRT):
		snprintf(buffer, length, "%s", "SIGABRT");
		break;
	case (SIGBUS):
		snprintf(buffer, length, "%s", "SIGBUS");
		break;
	case (SIGFPE):
		snprintf(buffer, length, "%s", "SIGFPE");
		break;
	case (SIGKILL):
		snprintf(buffer, length, "%s", "SIGKILL");
		break;
	case (SIGUSR1):
		snprintf(buffer, length, "%s", "SIGUSR1");
		break;
	case (SIGSEGV):
		snprintf(buffer, length, "%s", "SIGSEGV");
		break;
	case (SIGUSR2):
		snprintf(buffer, length, "%s", "SIGUSR2");
		break;
	case (SIGPIPE):
		snprintf(buffer, length, "%s", "SIGPIPE");
		break;
	case (SIGALRM):
		snprintf(buffer, length, "%s", "SIGALRM");
		break;
	case (SIGTERM):
		snprintf(buffer, length, "%s", "SIGTERM");
		break;
	case (SIGSTKFLT):
		snprintf(buffer, length, "%s", "SIGSTKFLT");
		break;
	case (SIGCHLD):
		snprintf(buffer, length, "%s", "SIGCHLD");
		break;
	case (SIGCONT):
		snprintf(buffer, length, "%s", "SIGCONT");
		break;
	case (SIGSTOP):
		snprintf(buffer, length, "%s", "SIGSTOP");
		break;
	case (SIGTSTP):
		snprintf(buffer, length, "%s", "SIGTSTP");
		break;
	case (SIGTTIN):
		snprintf(buffer, length, "%s", "SIGTTIN");
		break;
	case (SIGTTOU):
		snprintf(buffer, length, "%s", "SIGTTOU");
		break;
	case (SIGURG):
		snprintf(buffer, length, "%s", "SIGURG");
		break;
	case (SIGXCPU):
		snprintf(buffer, length, "%s", "SIGXCPU");
		break;
	case (SIGXFSZ):
		snprintf(buffer, length, "%s", "SIGXFSZ");
		break;
	case (SIGVTALRM):
		snprintf(buffer, length, "%s", "SIGVTALRM");
		break;
	case (SIGPROF):
		snprintf(buffer, length, "%s", "SIGPROF");
		break;
	case (SIGWINCH):
		snprintf(buffer, length, "%s", "SIGWINCH");
		break;
	case (SIGIO):
		snprintf(buffer, length, "%s", "SIGIO");
		break;
	case (SIGPWR):
		snprintf(buffer, length, "%s", "SIGPWR");
		break;
	case (SIGSYS):
		snprintf(buffer, length, "%s", "SIGSYS");
		break;
	default:
		log_pedantic("Asked to name an unrecognized signal. { signal = %i / name = UNKNOWN }", signal);
		snprintf(buffer, length, "UNKNOWN");
		break;
	}

	return buffer;
#endif
}
