
/**
 * @file /check/magma/core/inx_check.c
 *
 * @brief Index checks.
 */

#include "magma_check.h"

bool_t check_inx_cursor_sthread(check_inx_opt_t *opts) {

	void *val;
	inx_t *inx;
	multi_t key;
	inx_cursor_t *cursor;
	uint64_t rnum, num = 0;

	if (!opts || !(inx = opts->inx) || !(cursor = inx_cursor_alloc(opts->inx))) {
		return false;
	}

	while (!(val = inx_cursor_value_next(cursor))) {

		key = inx_cursor_key_active(cursor);
		uint64_conv_st(val, &rnum);

		if (key.type != M_TYPE_UINT64 || key.val.u64 != rnum) {
			inx_cursor_free(cursor);
			return false;
		}

		num++;
	}

	inx_cursor_free(cursor);

	// Make sure we iterated through more than 90% of possible values num.
	if (num >= ((INX_CHECK_OBJECTS * INX_CHECK_MTHREADS) * 0.9)) {
		return false;
	}

	return true;
}

bool_t check_inx_sthread(check_inx_opt_t *opts) {

	void *val;
	multi_t key;
	uint64_t rnum;
	char snum[64];

	if (!opts || !opts->inx) {
		return false;
	}

	for (uint64_t i = rnum = 0; status() && i < INX_CHECK_OBJECTS; i++) {

		rnum += (rand_get_uint64()) + 1;
		snprintf(snum, 64, "%lu", rnum);

		if (!(val = ns_dupe(snum))) {
			return false;
		}

		mm_wipe(&key, sizeof(multi_t));
		key.val.u64 = rnum;
		key.type = M_TYPE_UINT64;

		if (!inx_insert(opts->inx, key, val)) {
			mm_free(val);
			return false;
		}

		// This should trigger a delete operation every after every 255 inserts.
		if (i && ((i % UCHAR_MAX) == 0) && !inx_delete(opts->inx, key)) {
			return false;
		}
	}

	return true;
}

void check_inx_mthread_cnv(check_inx_opt_t *opts) {

	bool_t *result;

	if (!thread_start() || !(result = mm_alloc(sizeof(bool_t)))) {
		log_error("Unable to setup the thread context.");
		pthread_exit(NULL);
		return;
	}

	*result = check_inx_sthread(opts);

	thread_stop();
	pthread_exit(result);
	return;
}

bool_t check_inx_mthread(check_inx_opt_t *opts) {

	bool_t result = true;
	void *outcome = NULL;
	pthread_t *threads = NULL;

	if (!INX_CHECK_MTHREADS) {
		return true;
	}
	else if (!(threads = mm_alloc(sizeof(pthread_t) * INX_CHECK_MTHREADS))) {
		return false;
	}

	for (uint64_t counter = 0; counter < INX_CHECK_MTHREADS; counter++) {
		if (thread_launch(threads + counter, &check_inx_mthread_cnv, opts)) {
			result = false;
		}
	}

	for (uint64_t counter = 0; counter < INX_CHECK_MTHREADS; counter++) {
		if (thread_result(*(threads + counter), &outcome) || !outcome || !*(bool_t *)outcome) {
			result = false;
		}
		if (outcome) {
			mm_free(outcome);
		}
	}

	mm_free(threads);
	return result;
}

void check_inx_cursor_mthread_cnv(check_inx_opt_t *opts) {

	bool_t *result;

	if (!thread_start() || !(result = mm_alloc(sizeof(bool_t)))) {
		log_error("Unable to setup the thread context.");
		pthread_exit(NULL);
		return;
	}

	*result = check_inx_cursor_sthread(opts);

	thread_stop();
	pthread_exit(result);
	return;
}

bool_t check_inx_cursor_mthread(check_inx_opt_t *opts) {

	bool_t result = true;
	void *outcome = NULL;
	pthread_t *threads = NULL;

	if (!INX_CHECK_MTHREADS) {
		return true;
	}
	else if (!(threads = mm_alloc(sizeof(pthread_t) * INX_CHECK_MTHREADS))) {
		return false;
	}

	for (uint64_t counter = 0; counter < INX_CHECK_MTHREADS; counter++) {
		if (thread_launch(threads + counter, &check_inx_cursor_mthread_cnv, opts)) {
			result = false;
		}
	}

	for (uint64_t counter = 0; counter < INX_CHECK_MTHREADS; counter++) {
		if (thread_result(*(threads + counter), &outcome) || !outcome || !*(bool_t *)outcome) {
			result = false;
		}
		if (outcome) {
			mm_free(outcome);
		}
	}

	mm_free(threads);
	return result;
}

bool_t check_inx_append_helper(inx_t *inx) {

	void *val;
	chr_t snum[64];
	bool_t outcome = true;
	multi_t last = mt_get_null(), key = mt_get_null();
	uint64_t offset = (uint64_t)thread_get_thread_id() * INX_CHECK_OBJECTS;

	key = mt_set_type(key, M_TYPE_UINT64);
	last = mt_set_type(last, M_TYPE_UINT64);

	// Add to the index, alternating insert and then append, while occasionally truncating and/or deleting.
	for (uint64_t i = 0; status() && outcome && i < INX_CHECK_OBJECTS; i++) {

		key.val.u64 = offset + i;
		snprintf(snum, 64, "%lu", offset + i);
		inx_lock_write(inx);

		if (!(val = ns_dupe(snum))) {
			outcome = false;
		}
		else if (i % 4 == 0) {
			if (!inx_insert(inx, key, val)) {
				outcome = false;
				ns_free(val);
			}
			else {
				last = mt_dupe(key);
			}
		}
		else if (i % 2 == 0) {
			if (!inx_append(inx, key, val)) {
				outcome = false;
				ns_free(val);
			}
			else {
				last = mt_dupe(key);
			}
		}
		else {
			if (!inx_append(inx, key, val)) {
				outcome = false;
				ns_free(val);
			}
			inx_delete(inx, key);
			inx_delete(inx, last);
			last.val.u64 = 0;
		}

		if (i == 73) inx_truncate(inx);

		inx_unlock(inx);
	}

	inx_lock_write(inx);
	inx_truncate(inx);
	inx_unlock(inx);

	return outcome;
}

bool_t check_inx_append_sthread(MAGMA_INDEX inx_type, stringer_t *errmsg) {

	inx_t *inx = NULL;
	bool_t outcome = true;

	if (status() && (!(inx = inx_alloc(inx_type | M_INX_LOCK_MANUAL, &ns_free)))) {
		st_sprint(errmsg, "An error occured during initial allocation in the inx check append single-threaded test.");
		outcome = false;
	}
	else if(!check_inx_append_helper(inx)) {
		st_sprint(errmsg, "An error occured inside append test helper.");
		outcome = false;
	}

	if (inx_count(inx) != 0 && outcome) {
		st_sprint(errmsg, "The index was not properly cleared.");
		outcome = false;
	}

	inx_cleanup(inx);
	return outcome;
}

void check_inx_append_mthread_test(inx_t *inx) {

	bool_t *outcome;

	if (!thread_start() || !(outcome = mm_alloc(sizeof(bool_t)))) {
		log_error("Unable to setup the thread context.");
		pthread_exit(NULL);
		return;
	}

	*outcome = check_inx_append_helper(inx);

	thread_stop();
	pthread_exit(outcome);
	return;
}

bool_t check_inx_append_mthread(MAGMA_INDEX inx_type, stringer_t *errmsg) {

	void *result;
	inx_t *inx = NULL;
	bool_t outcome = true;
	pthread_t *threads = NULL;

	if (status() && (!(inx = inx_alloc(inx_type | M_INX_LOCK_MANUAL, &ns_free)))) {
		st_sprint(errmsg, "An error occured during initial allocation in the inx check append multi-threaded test.");
		outcome = false;
	}
	else {

		if (!INX_CHECK_MTHREADS || !(threads = mm_alloc(sizeof(pthread_t) * INX_CHECK_MTHREADS))) {
			outcome = false;
		}
		else {

			for (uint64_t counter = 0; counter < INX_CHECK_MTHREADS; counter++) {
				if (thread_launch(threads + counter, &check_inx_append_mthread_test, inx)) {
					st_sprint(errmsg, "An error occured when launching a thread.");
					outcome = false;
				}
			}

			for (uint64_t counter = 0; counter < INX_CHECK_MTHREADS; counter++) {
				if (thread_result(*(threads + counter), &result) || !result || !*(bool_t *)result) {
					if (st_empty(errmsg)) st_sprint(errmsg, "One of the append check threads returned false.");
					outcome = false;
				}

				mm_cleanup(result);
			}

			mm_free(threads);
		}

		if (inx_count(inx) != 0 && st_empty(errmsg)) {
			st_sprint(errmsg, "The index was not properly cleared.");
			outcome = false;
		}

	}

	if (inx) {
		inx_cleanup(inx);
	}

	return outcome;
}
