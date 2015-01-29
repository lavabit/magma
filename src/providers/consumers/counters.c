/*
 *
 * @file /magma/providers/consumers/counters.c
 *
 * @brief Distributed cache interface.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

int stamp_counter_check(char *key, size_t keylen, unsigned long interval) {

	time_t now;
	placer_t value;
	stringer_t *object;
	unsigned long stamp;
	int length, count = 0, token = 0;

	// No object, means no registrations.
	if ((object = cache_get(PLACER(key, keylen))) == NULL) {
		return -1;
	}

	// Store the current time.
	if ((now = time(NULL)) == ((time_t)-1)) {
		log_pedantic("Unable to determine the current time. Returning zero.");
		st_free(object);
		return -1;
	}

	length = tok_get_count_st(object, ';');
	while (token < length && tok_get_st(object, ';', token++, &value) == 0 && uint64_conv_pl(value, &stamp) == true) {
		 if ((now - stamp) <= interval) {
			 count++;
		 }
	}

	st_free(object);
	return count;
}

void stamp_counter_increment(char *key, size_t keylen) {

	time_t now;
	int objlen;
	char object[64];

	// Store the current time.
	if ((now = time(NULL)) == ((time_t)-1)) {
		log_pedantic("Unable to determine the current time.");
		return;
	}

	// Build the object.
	objlen = snprintf(object, 64, "%li;", now);

	if (cache_append(PLACER(key, keylen), PLACER(object, objlen), 604800) != 1) {
		log_pedantic("Unable to increment the timestamp counter.");
	}

	return;
}
