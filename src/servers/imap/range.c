
/**
 * @file /magma/servers/imap/range.c
 *
 * @brief Functions used to handle IMAP commands/actions.
 */

#include "magma.h"

stringer_t * imap_range_build(size_t length, uint64_t *numbers) {

	chr_t buffer[128];
	uint64_t current = 0;
	size_t increment = 0, count = 1;
	stringer_t *result = NULL, *holder;

	if (length == 0 || numbers == 0) {
		return NULL;
	}

	while (length > increment) {

		// Pop a number, and see how many consecutive numbers there are.
		current = *(numbers + increment);
		while (length > (increment + count) && (current + count) == *(numbers + increment + count)) {
			count++;
		}

		// There's only one number.
		if (count == 1) {

			if (snprintf(buffer, 128, "%lu", current) <= 0) {
				if (result != NULL) {
					st_free(result);
				}
				log_pedantic("Could not build the range.");
				return NULL;
			}

			increment++;
		}
		// We need to print_t a range.
		else {

			if (snprintf(buffer, 128, "%lu:%lu", current, current + count - 1) <= 0) {
				if (result != NULL) {
					st_free(result);
				}
				log_pedantic("Could not build the range.");
				return NULL;
			}

			increment += count;
		}

		// Merge the strings.
		if ((holder = st_merge("snn", result, (result == NULL) ? NULL : ",", buffer)) != NULL) {
			if (result != NULL) {
				st_free(result);
			}
			result = holder;
		}

		count = 1;
	}

	return result;
}
