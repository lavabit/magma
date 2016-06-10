
/**
 * @file /magma/objects/objects.c
 *
 * @brief	Functions used for managing objects.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

object_cache_t objects = {
	.meta = NULL,
	.sessions = NULL
};

/**
 * @brief	Initialize the object cache for all active user objects and web sessions.
 * @return	true on success or false on failure.
 */
bool_t obj_cache_start(void) {

	if (!(objects.meta = inx_alloc(M_INX_HASHED | M_INX_LOCK_MANUAL, &meta_free))) {
		log_critical("Unable to initialize the meta information cache.");
		return false;
	}

	if (!(objects.sessions = inx_alloc(M_INX_HASHED | M_INX_LOCK_MANUAL, &sess_destroy))) {
		log_critical("Unable to initialize the session cache.");
		return false;
	}

	return true;
}

/**
 * @brief	Stop the object cache and free all active user and web session objects.
 * @return	This function returns no value.
 */
void obj_cache_stop(void) {

	// Since web sessions can contain user objects; we need to free the sessions first, otherwise we'll have memory access errors.
	if (objects.sessions) {
		inx_free(objects.sessions);
		objects.sessions = NULL;
	}

	if (objects.meta) {
		inx_free(objects.meta);
		objects.meta = NULL;
	}


	return;
}

/**
 * @brief	The prune function runs every few minutes and scans the object cache and removes any stale objects it finds.
 *
 * @note	If the number of entries is over 4,096, then the prune function will remove entries candidates which have been unused more
 * 			then 5 minutes. If the index holds more than 2,048, entries older than 30 minutes are pruned, otherwise if the index holds
 * 			fewer than 2,048 entries, only those objects older than 1 hour are removed. Also, note that the precise interval between
 * 			scans is somewhat random, because the background thread responsible for running the prune function goes to sleep for a
 * 			random number of seconds.
 */
void obj_cache_prune(void) {

	time_t now;
	double_t gap;
	session_t *sess;
	inx_cursor_t *cursor;
	meta_user_t *meta;
	uint64_t count, expired;

	if ((now = time(NULL)) == (time_t)(-1)) {
		return;
	}

	if (objects.meta && (cursor = inx_cursor_alloc(objects.meta))) {

		expired = 0;

		inx_lock_read(objects.meta);

		// If were currently holding more than 4,096 meta objects, prune those older than 5 minutes.
		if ((count = inx_count(objects.meta)) > 4096) {
			gap = 300;
		}
		// If the count is above 2,048, prune entries older than 30 minutes.
		else if (count > 2048) {
			gap = 1800;
		}
		// Otherwise only prune those older than 1 hour.
		else  {
			gap = 3600;
		}

		inx_unlock(objects.meta);
		inx_lock_write(objects.meta);

		meta = inx_cursor_value_next(cursor);

		while (meta) {
			if (difftime(now, meta_user_ref_stamp(meta)) > gap && !meta_user_ref_total(meta)) {
				inx_delete(objects.meta, inx_cursor_key_active(cursor));
				inx_cursor_reset(cursor);
				expired++;
			}
			meta = inx_cursor_value_next(cursor);
		}

		// Record the total so we can update the statistics variable.
		count = inx_count(objects.meta);
		inx_unlock(objects.meta);
		inx_cursor_free(cursor);

		stats_set_by_name("objects.meta.total", count);
		stats_adjust_by_name("objects.meta.expired", expired);
	}

	if (objects.sessions && (cursor = inx_cursor_alloc(objects.sessions))) {

		expired = 0;

		inx_lock_read(objects.sessions);

		// If were currently holding more than 4,096 sessions, prune those older than 5 minutes.
		if ((count = inx_count(objects.sessions)) > 4096) {
			gap = 300;
		}
		// If the count is above 2,048, prune entries older than 30 minutes.
		else if (count > 2048) {
			gap = 1800;
		}
		// Otherwise only prune those older than 1 hour.
		else  {
			gap = 3600;
		}

		inx_unlock(objects.sessions);
		inx_lock_write(objects.sessions);

		sess = inx_cursor_value_next(cursor);

		while (sess) {
			if (difftime(now, sess_ref_stamp(sess)) > gap && !sess_ref_total(sess)) {
				inx_delete(objects.sessions, inx_cursor_key_active(cursor));
				inx_cursor_reset(cursor);
				expired++;
			}
			sess = inx_cursor_value_next(cursor);
		}

		// Record the total so we can update the statistics variable.
		count = inx_count(objects.sessions);
		inx_unlock(objects.sessions);
		inx_cursor_free(cursor);

		stats_set_by_name("objects.sessions.total", count);
		stats_adjust_by_name("objects.sessions.expired", expired);
	}


	return;
}

