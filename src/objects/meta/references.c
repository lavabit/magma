
/**
 * @file /magma/objects/meta/references.c
 *
 * @brief Functions for handling the meta object reference counters.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Increment a meta user's reference counter for a specified protocol and update the activity timestamp.
 *
 * @note	META_PROTOCOL_GENERIC can be specified for non-specific, protocol independent accounting purposes.
 *
 * @param	user		a pointer to the meta user object to be adjusted.
 * @param	protocol	the protocol identifier for the session.
 *
 * @return	This function returns no value.
 */
void meta_user_ref_add(meta_user_t *user, META_PROTOCOL protocol) {

	if (user) {

		// Acquire the reference counter lock.
		mutex_lock(&(user->refs.lock));

		// Increment the right counter.
		if ((protocol & META_PROTOCOL_WEB) == META_PROTOCOL_WEB) user->refs.web++;
		else if ((protocol & META_PROTOCOL_IMAP) == META_PROTOCOL_IMAP) user->refs.imap++;
		else if ((protocol & META_PROTOCOL_POP) == META_PROTOCOL_POP) user->refs.pop++;
		else if ((protocol & META_PROTOCOL_SMTP) == META_PROTOCOL_SMTP) user->refs.smtp++;
		else if ((protocol & META_PROTOCOL_GENERIC) == META_PROTOCOL_GENERIC) user->refs.generic++;
#ifdef MAGMA_PEDANTIC
		else {
			log_pedantic("The protocol enumerator doesn't have a reference counter. { protocol = %u }", protocol);
		}
#endif

		// Update the activity time stamp.
		 user->refs.stamp = time(NULL);

		// Release the reference counter lock.
		mutex_unlock(&(user->refs.lock));

	}

	return;
}

/**
 * @brief	Decrement a user's reference counter for the specified protocol and update the activity timestamp.
 *
 * @note	META_PROTOCOL_GENERIC can be specified for non-specific, protocol independent accounting purposes.
 *
 * @param	user		a pointer to the meta user object to be adjusted.
 * @param	protocol	the protocol identifier for the session using the META_PROTOCOL enumerator.
 *
 * @return	This function returns no value.
 */
void meta_user_ref_dec(meta_user_t *user, META_PROTOCOL protocol) {

	if (user) {

		// Acquire the reference counter lock.
		mutex_lock(&(user->refs.lock));

		// Decrement the right counter.
		if ((protocol & META_PROTOCOL_WEB) == META_PROTOCOL_WEB) user->refs.web--;
		else if ((protocol & META_PROTOCOL_IMAP) == META_PROTOCOL_IMAP) user->refs.imap--;
		else if ((protocol & META_PROTOCOL_POP) == META_PROTOCOL_POP) user->refs.pop--;
		else if ((protocol & META_PROTOCOL_SMTP) == META_PROTOCOL_SMTP) user->refs.smtp--;
		else if ((protocol & META_PROTOCOL_GENERIC) == META_PROTOCOL_GENERIC) user->refs.generic--;
#ifdef MAGMA_PEDANTIC
		else {
			log_pedantic("The protocol enumerator doesn't have a reference counter. { protocol = %u }", protocol);
		}
#endif

		// Update the activity time stamp.
		user->refs.stamp = time(NULL);

		// Release the reference counter lock.
		mutex_unlock(&(user->refs.lock));

	}

	return;
}

/**
 * @brief	Get the total reference count for the user object over the specified protocol.
 *
 * @param	user		a pointer to the meta user object to be examined.
 * @param	protocol	the protocol identifier for the session using the META_PROTOCOL enumerator.
 *
 * @return	the total number of references held by the user.
 */
uint64_t meta_user_ref_protocol_total(meta_user_t *user, META_PROTOCOL protocol) {

	uint64_t result = 0;

	if (user) {

		// Acquire the reference counter lock.
		mutex_lock(&(user->refs.lock));

		// Decrement the right counter.
		if ((protocol & META_PROTOCOL_WEB) == META_PROTOCOL_WEB) result = user->refs.web;
		else if ((protocol & META_PROTOCOL_IMAP) == META_PROTOCOL_IMAP) result = user->refs.imap;
		else if ((protocol & META_PROTOCOL_POP) == META_PROTOCOL_POP) result = user->refs.pop;
		else if ((protocol & META_PROTOCOL_SMTP) == META_PROTOCOL_SMTP) result = user->refs.smtp;
		else if ((protocol & META_PROTOCOL_GENERIC) == META_PROTOCOL_GENERIC) result = user->refs.generic;
#ifdef MAGMA_PEDANTIC
		else {
			log_pedantic("The protocol enumerator doesn't have a reference counter. { protocol = %u }", protocol);
		}
#endif

		// Release the reference counter lock.
		mutex_unlock(&(user->refs.lock));

	}

	return result;
}

/**
 * @brief	Get the total reference count for a user object, over all of the supported protocols.
 *
 * @param	user		a pointer to the meta user object to be examined.
 *
 * @return	the total number of references held by the user.
 */
uint64_t meta_user_ref_total(meta_user_t *user) {

	uint64_t result = 0;

	if (user) {

		// Acquire the reference counter lock.
		mutex_lock(&(user->refs.lock));

		// Sum the total.
		result = user->refs.web + user->refs.imap + user->refs.pop + user->refs.smtp + user->refs.generic;

		// Release the reference counter lock.
		mutex_unlock(&(user->refs.lock));

	}

	return result;
}

/**
 * @brief	Get the activity timestamp for a meta user object.
 *
 * @param	user	a pointer to the meta user object to be examined.
 *
 * @return	a timestamp containing the last time the meta user object's reference count changed.
 */
time_t meta_user_ref_stamp(meta_user_t *user) {

	time_t stamp = 0;

	if (user) {

		// Acquire the reference counter lock.
		mutex_lock(&(user->refs.lock));

		// Grab the object time stamp.
		stamp = user->refs.stamp;

		// Release the reference counter lock.
		mutex_unlock(&(user->refs.lock));

	}

	return stamp;
}
