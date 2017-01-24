
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
