
/**
 * @file /magma/objects/serials.c
 *
 * @brief	Functions used for track and update the object checkpoints.
 */

#include "magma.h"

stringer_t *serial_prefix_strings[] = {
	 CONSTANT("user"),
	 CONSTANT("config"),
	 CONSTANT("folders"),
	 CONSTANT("messages"),
	 CONSTANT("contacts"),
	 CONSTANT("aliases")
};

/**
 * @brief	Return a textual prefix describing an object type.
 * @param	type	the serial object type: OBJECT_USER, OBJECT_CONFIG, OBJECT_FOLDERS, OBJECT_MESSAGES, or OBJECT_CONTACTS.
 * @return	a managed string containing a description of the object type, or NULL on failure.
 */
stringer_t * serial_prefix(uint64_t type) {

	stringer_t *prefix = NULL;

	switch (type) {
		case (OBJECT_USER):
			prefix = serial_prefix_strings[0];
			break;
		case (OBJECT_CONFIG):
			prefix = serial_prefix_strings[1];
			break;
		case (OBJECT_FOLDERS):
			prefix = serial_prefix_strings[2];
			break;
		case (OBJECT_MESSAGES):
			prefix = serial_prefix_strings[3];
			break;
		case (OBJECT_CONTACTS):
			prefix = serial_prefix_strings[4];
			break;
		case (OBJECT_ALIASES):
			prefix = serial_prefix_strings[5];
			break;
		default:
			log_pedantic("Unrecognized object type. { type = %lu }", type);
			prefix = NULL;
			break;
	}

	return prefix;
}

/**
 * @brief	Get the serial number (checkpoint value) for an object from memcached.
 * @param	type	the serial type to be queried (OBJECT_USER, OBJECT_CONFIG, OBJECT_FOLDERS, OBJECT_MESSAGES, or OBJECT_CONTACTS).
 * @param	num		the specific object identifier.
 * @return	0 on failure or the serial number of the requested object.
 */
uint64_t serial_get(uint64_t type, uint64_t num) {

	uint64_t result = 0;
	stringer_t *key, *prefix;

	// Build retrieval key.
	if (!(prefix = serial_prefix(type)) || !(key = st_aprint("magma.%.*s.%lu", st_length_int(prefix), st_char_get(prefix), num))) {
		log_pedantic("Unable to build %.*s serial key.", st_length_int(prefix), st_char_get(prefix));
		return 0;
	}

	// Get the key value. The increment functions store the value in binary form, so we must use them to access the value, even if we aren't incrementing the value.
	result = cache_increment(key, 0, 0, 2592000);
	st_free(key);

	return result;
}

/**
 * @brief	Increment the serial number for an object in memcached.
 * @param	type	the serial type to be queried (OBJECT_USER, OBJECT_CONFIG, OBJECT_FOLDERS, OBJECT_MESSAGES, or OBJECT_CONTACTS).
 * @param	num		the specific object identifier.
 * @return	0 on failure or the new serial number of the requested object.
 */
uint64_t serial_increment(uint64_t type, uint64_t num) {

	uint64_t result = 0;
	stringer_t *key, *prefix;

	// Build retrieval key.
	if (!(prefix = serial_prefix(type)) || !(key = st_aprint("magma.%.*s.%lu", st_length_int(prefix), st_char_get(prefix), num))) {
		log_pedantic("Unable to build %.*s serial key.", st_length_int(prefix), st_char_get(prefix));
		return 0;
	}

	// Increment the key.
	result = cache_increment(key, 1, 1, 2592000);
	st_free(key);

	return result;
}

/**
 * @brief	Reset the serial number to 1 for an object in memcached.
 * @param	type	the serial type to be queried (OBJECT_USER, OBJECT_CONFIG, OBJECT_FOLDERS, OBJECT_MESSAGES, or OBJECT_CONTACTS).
 * @param	num		the specific object identifier.
 * @return	0 on failure or the new value of the cached serial number (1) on success.
 */
uint64_t serial_reset(uint64_t type, uint64_t num) {

	uint64_t result = 0;
	stringer_t *key, *prefix;

	// Build key.
	if (!(prefix = serial_prefix(type)) || !(key = st_aprint("magma.%.*s.%lu", st_length_int(prefix), st_char_get(prefix), num))) {
		log_pedantic("Unable to build %.*s serial key.", st_length_int(prefix), st_char_get(prefix));
		return 0;
	}

	// If were able to set the key, return the value one.
	if (cache_set(key, CONSTANT("1"), 2592000) == 1) {
		result = 1;
	}

	st_free(key);

	return result;
}

