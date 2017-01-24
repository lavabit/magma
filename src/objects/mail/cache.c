
/**
 * @file /magma/objects/mail/cache.c
 *
 * @brief	Functions used to cache a mail message in its parsed form.
 */

#include "magma.h"

static pthread_key_t mail_cache;

/**
 * @brief	Free a cached mail message.
 * @param	holder	a pointer to the cached mail message object to be freed.
 * @return	This function returns no value.
 */
void mail_cache_destroy(void *holder) {

	mail_cache_t *message = (mail_cache_t *)holder;

	 if (message) {
		 st_cleanup(message->text);
		 mm_free(message);
	 }

	 return;
}

/**
 * @brief	Initialize the thread-specific data key used for the mail message cache.
 * @return	true on success or false on failure.
 */
bool_t mail_cache_start(void) {

	 if (pthread_key_create(&mail_cache, mail_cache_destroy) != 0) {
		 log_pedantic("Unable to create the thread local message cache.");
		 return false;
	 }

	 return true;
}

/**
 * @brief	Destroy the thread-specific data key used for the mail message cache.
 * @return	This function returns no value.
 */
void mail_cache_stop(void) {

	if (pthread_key_delete(mail_cache) != 0) {
		log_pedantic("Unable to delete the thread local message cache.");
	}

	return;
}

/**
 * @brief	Free the thread-specific data for the mail message cache.
 * @return	This function returns no value.
 */
void mail_cache_thread_stop(void) {

	mail_cache_t *message;

	if ((message = pthread_getspecific(mail_cache))) {
		mail_cache_destroy(message);
		pthread_setspecific(mail_cache, NULL);
	}

	return;
}

/**
 * @brief	Attempt to retrieve the contents of a message from the thread's cached data.
 * @note	The thread's cache only has the capacity to store a single message.
 * @param	messagenum		the id of the message to be retrieved.
 * @return	NULL on failure or a managed string containing the message data on success.
 */
stringer_t * mail_cache_get(uint64_t messagenum) {

	mail_cache_t *message;

	if (!(message = pthread_getspecific(mail_cache))) {
		return NULL;
	}

	if (message->messagenum == messagenum) {
		return st_dupe(message->text);
	}

	return NULL;
}

/**
 * @brief	Reset the thread's mail cache and free any held message.
 * @return	This function returns no value.
 */
void mail_cache_reset(void) {

	mail_cache_t *message;

	if ((message = pthread_getspecific(mail_cache))) {
		st_cleanup(message->text);
		message->text = NULL;
		message->messagenum = 0;
	}

	return;
}

/**
 * @brief	Set the contents of the thread's mail cache.
 * @param	messagenum	the numerical id of the message to be cached.
 * @text	a managed string containing the contents of the specified message to be cached.
 * @return	This function returns no value.
 */
void mail_cache_set(uint64_t messagenum, stringer_t *text) {

	mail_cache_t *message;

	if (!(message = pthread_getspecific(mail_cache))) {

		if (!(message = mm_alloc(sizeof(mail_cache_t)))) {
			return;
		}

		if (pthread_setspecific(mail_cache, message) != 0) {
			log_pedantic("Unable to setup thread message cache.");
			mm_free(message);
			return;
		}
	}
	else {
		st_cleanup(message->text);
	}

	message->messagenum = messagenum;
	message->text = st_dupe_opts(MANAGED_T | HEAP | CONTIGUOUS, text);

	return;
}
