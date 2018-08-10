
/**
 * @file /magma/src/core/host/signals.c
 *
 * @brief 	Functions to help handle signals.
 */

#include "magma.h"

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
}
