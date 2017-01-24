
/**
 * @file /magma/objects/warehouse/patterns.c
 *
 * @brief	Functions used to manage the list of spam patterns that are scanned for detection in outbound messages.
 */

#include "magma.h"

inx_t *patterns_list = NULL;
uint64_t patterns_stamp = 0;
pthread_mutex_t patterns_mutex = PTHREAD_MUTEX_INITIALIZER;

/**
 * @brief	Check to see if any of the entries in the patterns list are found in a body of text.
 * @param	message		a managed string containing the raw data of the text to be searched.
 * @return	-2 on pattern match, -1 if an error occurs, or 1 if none of the patterns in the patterns list were detected.
 */
int_t pattern_check(stringer_t *message) {

	size_t location;
	int_t result = 1;
	stringer_t *current;
	inx_cursor_t *cursor;

	stats_adjust_by_name("objects.patterns.checked", 1);

	mutex_lock(&patterns_mutex);
	cursor = inx_cursor_alloc(patterns_list);
	mutex_unlock(&patterns_mutex);

	if (!cursor) {
		result = -1;
	} else {

		while (result == 1 && (current = inx_cursor_value_next(cursor))) {

			if (st_search_ci(message, current, &location)) {
				result = -2;
			}

		}

		inx_cursor_free(cursor);
	}

	if (result == -2) {
		stats_adjust_by_name("objects.patterns.fail", 1);
	} else if (result == -1) {
		stats_adjust_by_name("objects.patterns.errors", 1);
	}else if (result == 1) {
		stats_adjust_by_name("objects.patterns.pass", 1);
	}

	return result;
}

/**
 * @brief	Update the patterns list from the database, but no more frequently than once daily.
 * @return	This function returns no value.
 */
void pattern_update(void) {

	inx_t *patterns_new, *patterns_old;

	// Refresh the list of user patterns whenever the date changes.
	if (patterns_stamp == time_datestamp()) {
		return;
	}

	patterns_stamp = time_datestamp();

	// Fetch the patterns and allocate a cursor for iterating.
	if (!(patterns_new = warehouse_fetch_patterns())) {
		return;
	}

	// Swap the old pointers for the new ones.
	mutex_lock(&patterns_mutex);
	patterns_old = patterns_list;
	patterns_list = patterns_new;
	mutex_unlock(&patterns_mutex);

	// If we replaced an existing index we can free it now.
	inx_cleanup(patterns_old);

	return;
}

/**
 * @brief	Destroy the patterns list.
 * @return	This function returns no value.
 */
void pattern_stop(void) {

	mutex_lock(&patterns_mutex);

	inx_cleanup(patterns_list);
	patterns_list = NULL;
	mutex_unlock(&patterns_mutex);

	return;
}

/**
 * @brief	Initialize the patterns list.
 * @return	This function will always return true.
 */
bool_t pattern_start(void) {

	pattern_update();
	return true;
}

