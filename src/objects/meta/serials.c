
/**
 * @file /magma/objects/meta/serials.c
 *
 * @brief Functions for retrieving and incrementing meta object serial numbers.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"


/**
 * @brief	Set an object serial number for a given meta user structure.
 *
 * @param	user	the meta user object to be adjusted.
 * @param	object	the object to receive the new serial number.
 * @param	serial	the new serial number to be set.
 *
 * @return	This function returns no value.
 */
void meta_user_serial_set(meta_user_t *user, uint64_t object, uint64_t serial) {

	if (!user || !serial) {
		return;
	}
	else if (object == OBJECT_USER) {
		user->serials.user = serial;
	}
	else if (object == OBJECT_FOLDERS) {
		user->serials.folders = serial;
	}
	else if (object == OBJECT_MESSAGES) {
		user->serials.messages = serial;
	}
	else if (object == OBJECT_CONTACTS) {
		user->serials.contacts = serial;
	}
	else if (object == OBJECT_ALIASES) {
		user->serials.aliases = serial;
	}

	return;
}

/**
 * @brief	Get an object serial number for a given meta object.
 *
 * @param	user	the meta user structure to be examined.
 * @param	object	the object to query for the serial number.
 *
 * @return	the serial number value of the specified object, or 0 on failure.
 */
/// LOW: Swap the object and user params so the interface matches the serial_get/set functions.
uint64_t meta_user_serial_get(meta_user_t *user, uint64_t object) {

	uint64_t serial = 0;

	if (!user) {
		return serial;
	}
	else if (object == OBJECT_USER) {
		serial = user->serials.user;
	}
	else if (object == OBJECT_FOLDERS) {
		serial = user->serials.folders;
	}
	else if (object == OBJECT_MESSAGES) {
		serial = user->serials.messages;
	}
	else if (object == OBJECT_CONTACTS) {
		serial = user->serials.contacts;
	}
	else if (object == OBJECT_ALIASES) {
		serial = user->serials.aliases;
	}

	return serial;
}

/**
 * @brief	Check an object's serial number to see if it is up-to-date.
 * @note	The object's serial number will be incremented regardless of whether it is consistent with the cache.
 * @param	user	the meta user object to whom the object belongs.
 * @param	object	the serial object to be checked for changes.
 * @return	0 if the object did not to be refreshed, or 1 if it doesn't match the internal checkpoint and should be updated.
 */
bool_t meta_user_serial_check(meta_user_t *user, uint64_t object) {

	int_t result = false;

	// If the serial number indicates no outside changes we can increment it without forcing a refresh.{
	if (user && meta_user_serial_get(user, object) == serial_get(object, user->usernum)) {
		meta_user_serial_set(user, object, serial_increment(object, user->usernum));
	}
	// Increment the reference counter and queue a session update.
	else if (user) {
		serial_increment(object, user->usernum);
		result = true;
	}

	return result;
}
