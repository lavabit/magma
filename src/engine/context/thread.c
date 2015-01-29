
/**
 * @file /magma/engine/context/thread.c
 *
 * @brief	The thread start and stop functions.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Prepare a thread to exit by destroying its mysql and openssl-specific and associated mail cache data.
 * @return	This function returns no value.
 */
void thread_stop(void) {

	sql_thread_stop();
	ssl_thread_stop();
	mail_cache_thread_stop();

	return;
}

/**
 * @brief	Setup a new thread by setting up signal handling and preparing it for mysql processing.
 * @return	true on success or false on failure.
 */
bool_t thread_start(void) {

	if (!signal_thread_start()) {
		return false;
	}
	else if (!sql_thread_start()) {
		return false;
	}

	return true;
}
