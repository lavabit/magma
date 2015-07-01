
/**
 * @file /magma/engine/context/signal.c
 *
 * @brief	A collection of functions used to register and handle signals.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

pthread_mutex_t sig_hup_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief	Handle signals that indicate the program was killed because of an invalid operation (including SIGSEGV).
 * @note	The signals handled by this function are: SIGSEGV, SIGFPE, SIGBUS, SIGSYS, and SIGABRT.
 * 			The handler will respond by logging a stack backtrace, and terminating the program with abort().
 * @param	signal	the number of the signal that was received.
 * @return	This function returns no value.
 */
void signal_segfault(int signal) {

	char signame[32];
	struct sigaction clear;

	// Log it.
	log_options(M_LOG_CRITICAL | M_LOG_STACK_TRACE, "Memory corruption has been detected. Attempting to print a back trace and exit. {signal = %s}",
			signal_name(signal, signame, 32));

	// Configure the structure to return a signal to its default handler via the sigaction call below.
	mm_wipe(&clear, sizeof(struct sigaction));
	sigemptyset(&clear.sa_mask);
	clear.sa_handler = SIG_DFL;
	clear.sa_flags = 0;

	// Return the SIGABRT handler to its default value. This allows us to call abort and trigger a core dump without creating a recursive loop.
	sigaction(SIGABRT, &clear, NULL);

	// The abort function should trigger the creation of a core dump file if core dumps have been enabled.
	abort();
}

/**
 * @brief	A function to handle receipt of shutdown signals and allow for graceful exits.
 * @note	This function handles the following signals: SIGINT, SIGQUIT, and SIGTERM.
 * 			The shutdown procedure is as follows:
 * 			1. Set status to -1 and sleep for .1 second to allow for normal daemon termination.
 * 			2. Signal all worker threads to wake up blocked threads, and sleep one more second.
 * 			3. Loop through all possible file descriptors and if the descriptor matches a socket that is bound to a magma server, close it.
 *
 * @param	signal	the number of the signal delivered to the process.
 * @return	This function returns no value.
 */
void signal_shutdown(int signal) {

	//int fd = 0;
	ip_t ip;
	char working[64];
	struct stat64 info;
	struct rlimit64 limits;
	struct sockaddr_in6 saddr;
	socklen_t len = sizeof(struct sockaddr_in6);
	const struct timespec split = { .tv_sec = 0, .tv_nsec = 100000000 }, single = { .tv_sec = 1, .tv_nsec = 0 };

	// Set the status flag so all the worker threads exit nicely.
	status_set(-1);

	// We assume the server is being shutdown for a good reason.
	log_critical("Signal received. The Magma daemon is attempting a graceful exit. {signal = %s}", signal_name(signal, working, 32));

	// We give threads 0.1 seconds to terminate normally before forcefully closing the socket connections.
	nanosleep(&split, NULL);

	// Signals the worker threads. Since the client socket connections were closed above, the signal should unblock any stuck worker threads.
	queue_signal();

	// Sleep for one second before forcibly shutting down the client connections.
	nanosleep(&single, NULL);

	// Now go through and shutdown all client connections.
	if (getrlimit64(RLIMIT_NOFILE, &limits)) {
		log_critical("Unable to determine the maximum legal file descriptor.");
		return;
	}

	// Loop through and check all of the potentially valid file descriptors.
	for (int fd = 0; fd <= limits.rlim_max; fd++) {

		mm_wipe(&info, sizeof(struct stat64));
		mm_wipe(&saddr, sizeof(struct sockaddr_in6));

		// Look for socket descriptors using ports assigned to server instances and close them.
		if (!fstat64(fd, &info) && S_ISSOCK(info.st_mode) && !getsockname(fd, &saddr, &len)) {

			if (len == sizeof(struct sockaddr_in6) && saddr.sin6_family == AF_INET6 && servers_get_count_using_port(ntohs(saddr.sin6_port))) {
				mm_copy(&(ip.ip6), &(saddr.sin6_addr), sizeof(struct in6_addr));
				ip.family = AF_INET6;
				log_info("%s:%u is being shutdown.", st_char_get(ip_presentation(&ip, PLACER(working, 64))), ntohs(saddr.sin6_port));
				close(fd);
			}
			else if (len == sizeof(struct sockaddr_in) && ((struct sockaddr_in *)&saddr)->sin_family == AF_INET && servers_get_count_using_port(ntohs(((struct sockaddr_in *)&saddr)->sin_port))) {
				mm_copy(&(ip.ip4), &(((struct sockaddr_in *)&saddr)->sin_addr), sizeof(struct in_addr));
				ip.family = AF_INET;
				log_info("%s:%u is being shutdown.", st_char_get(ip_presentation(&ip, PLACER(working, 64))), ntohs(((struct sockaddr_in *)&saddr)->sin_port));
				close(fd);
			}
		}
	}

	return;
}

/**
 * @brief	A generic worker thread signal handler entry point.
 * @note	If the target thread is not in the process of shutting down, display the name of the caught signal.
 * @param	signal	the number of the signal caught by the signal handler.
 * @return	This function returns no value.
 */
void signal_status(int signal) {
	chr_t signame[32];
	if (status()) {
		log_info("This worker thread was signaled but the status function doesn't indicate a shutdown is in progress. {signal = %s}",
			signal_name(signal, signame, 32));
	}
	return;
}

/**
 * @brief	A function to handle receipt of SIGHUP and refresh all web server content.
 * @param	signal	the number of the delivered signal (should only be SIGHUP).
 * @return	This function returns no value.
 */
void signal_refresh(int signal) {

	mutex_get_lock (&sig_hup_mutex);

	if (!http_content_refresh()) {
		log_error("An error occurred while trying to refresh disk based content.");
	}
	else {
		log_info("Disk content refreshed.");
	}

	mutex_unlock (&sig_hup_mutex);

	return;
}

/**
 * @brief	Bind a SIGALRM handler for the calling thread.
 * @see		signal_status()
 * @return	false on failure or true on success.
 */
bool_t signal_thread_start(void) {

	struct sigaction status;

	// Zero out the signal structure and setup the normal shutdown handler.
	mm_wipe(&status, sizeof(struct sigaction));
	status.sa_handler = signal_status;
	status.sa_flags = 0;
	if (sigemptyset(&status.sa_mask) || sigaction(SIGALRM, &status, NULL)) {
		log_info("Could not setup the thread status signal handler.");
		return false;
	}

	return true;
}

/**
 * @brief	Setup signal masks and register signal handlers for handling shutdowns, program termination, and reloads.
 * @note	The following handlers are established:
 * 			signal_shutdown:	SIGINT, SIGQUIT, SIGTERM, SIGHUP
 * 			signal_segfault:	SIGSEGV, SIGFPE, SIGBUS, SIGSYS
 * 			signal_refresh:		SIGHUP
 * 			[ignored]:			SIGPIPE
 * @return	true if all siginal handlers were successfully registered, or false on failure.
 */
bool_t signal_start(void) {

	struct sigaction hup;
	struct sigaction normal;
	struct sigaction ignored;
	struct sigaction segfault;

	// Zero out the signal structure and setup the normal shutdown handler.
	mm_wipe(&normal, sizeof(struct sigaction));
	normal.sa_handler = signal_shutdown;
	normal.sa_flags = 0;
	// QUESTION: Why is this being handled here and below?
	if (sigemptyset(&normal.sa_mask) || sigaction(SIGINT, &normal, NULL) || sigaction(SIGQUIT, &normal, NULL) || sigaction(SIGTERM, &normal, NULL) || sigaction(SIGHUP, &normal, NULL)) {
		log_info("Could not setup the shutdown signal handler.");
		return false;
	}

	// Ignore these signals.
	mm_wipe(&ignored, sizeof(struct sigaction));
	ignored.sa_handler = SIG_IGN;
	ignored.sa_flags = 0;
	if (sigemptyset(&ignored.sa_mask) || sigaction(SIGPIPE, &ignored, NULL)) {
		log_info("Could not setup the ignore signal handler");
		return false;
	}

	// Segmentation fault handler.
	mm_wipe(&segfault, sizeof(struct sigaction));
	segfault.sa_handler = signal_segfault;

	// We restore the default handler for SIGABRT before calling the abort function, but the SA_RESETHAND flag
	// should provide further protection against the creation of an endless recursive loop by restoring the
	// default handlers after receiving any of the signals below.
	segfault.sa_flags = SA_RESETHAND;
	if (sigemptyset(&segfault.sa_mask) || sigaction(SIGSEGV, &segfault, NULL) || sigaction(SIGFPE, &segfault, NULL) || sigaction(SIGBUS, &segfault, NULL) || sigaction(SIGSYS, &segfault, NULL) || sigaction(SIGABRT, &segfault, NULL)) {
		log_info("Could not setup the shutdown signal handler.");
		return false;
	}

	// Zero out the signal structure and setup the SIGHUP handler.
	mm_wipe(&hup, sizeof(struct sigaction));
	hup.sa_handler = signal_refresh;
	hup.sa_flags = 0;
	if (sigemptyset(&hup.sa_mask) || sigaction(SIGHUP, &hup, NULL)) {
		log_info("Could not setup the thread status signal handler.");
		return false;
	}

	return true;
}
