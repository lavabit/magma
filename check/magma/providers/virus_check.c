
/**
 * @file /check/magma/providers/virus_check.c
 *
 * @brief Check the anti-virus provider.
 */

#include "magma_check.h"

chr_t * check_virus_sthread(void) {

	stringer_t *data = NULL;
	uint32_t max = check_message_max();

	for (uint32_t i = 0; i < max && status(); i++) {

		// Retrieve data for the current message.
		if (!(data = check_message_get(i))) {
			log_info("Failed to get the message data. { message = %i }", i);
			return "check_message_get() error";
		}

		if (virus_check(data) == -1) {
			log_info("There was a virus check error. { message = %i }", i);
			return "virus check error";
		}

		st_cleanup(data);
	}

	return NULL;
}
