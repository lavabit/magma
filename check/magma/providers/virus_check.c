
/**
 * @file /check/magma/providers/virus_check.c
 *
 * @brief Check the anti-virus provider.
 */

#include "magma_check.h"

 bool_t check_virus_sthread(stringer_t *errmsg) {

	stringer_t *data = NULL;
	uint32_t max = check_message_max();

	for (uint32_t i = 0; i < max && status(); i++) {

		// Retrieve data for the current message.
		if (!(data = check_message_get(i))) {
			st_sprint(errmsg, "Failed to get the message data. { message = %i }", i);
			return false;
		}

		else if (virus_check(data) == -1) {
			st_sprint(errmsg, "The virus checker returned an error. { message = %i }", i);
			return false;
		}

		st_cleanup(data);
	}

	return true;
}
