
/**
 * @file /check/magma/providers/rand_check.c
 *
 * @brief Check out the random number functions.
 */

#include "magma_check.h"

stringer_t * check_rand_sthread(void) {

	size_t len;
	uint64_t num = 0;
	stringer_t *buffer;

	if (!(buffer = st_alloc(RAND_CHECK_SIZE_MAX))) {
		return st_dupe(NULLER("Buffer allocation error."));
	}

	for (int_t i = 0; status() && i < RAND_CHECK_ITERATIONS; i++) {

		num |= rand_get_int8();
		num |= rand_get_int16();
		num |= rand_get_int32();
		num |= rand_get_int64();

		num |= rand_get_uint8();
		num |= rand_get_uint16();
		num |= rand_get_uint32();
		num |= rand_get_uint64();

		// Pick a random length.
		len = (rand() % (RAND_CHECK_SIZE_MAX - RAND_CHECK_SIZE_MIN)) + RAND_CHECK_SIZE_MIN;

		if (rand_write(PLACER(st_char_get(buffer), len)) != len) {
			st_cleanup(buffer);
			return st_dupe(NULLER("Unable to fill the buffer with random data."));
		}
	}

	st_cleanup(buffer);

	// This time through we use the choices function since it will allocate its own output buffer.
	for (int_t i = 0; status() && i < RAND_CHECK_ITERATIONS; i++) {

		// Pick a random length.
		len = (rand() % (RAND_CHECK_SIZE_MAX - RAND_CHECK_SIZE_MIN)) + RAND_CHECK_SIZE_MIN;

		if (!(buffer = rand_choices("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", len, NULL))) {
			return st_dupe(NULLER("Unable to fill the buffer with random data."));
		}
		st_free(buffer);
	}

	return NULL;
}

void check_rand_mthread_wrap(void) {

	stringer_t *result = NULL;

	if (!thread_start()) {
		log_unit("Unable to setup the thread context.");
		pthread_exit(st_dupe(NULLER("Thread startup error.")));
		return;
	}

	result = check_rand_sthread();

	thread_stop();
	pthread_exit(result);
	return;
}

stringer_t * check_rand_mthread(void) {

	void *outcome = NULL;
	stringer_t *result = NULL;
	pthread_t *threads = NULL;

	if (!RAND_CHECK_MTHREADS) {
		return NULL;
	}
	else if (!(threads = mm_alloc(sizeof(pthread_t) * RAND_CHECK_MTHREADS))) {
		return st_dupe(NULLER("Thread allocation error."));
	}

	for (uint64_t counter = 0; counter < RAND_CHECK_MTHREADS; counter++) {
		if (thread_launch(threads + counter, &check_rand_mthread_wrap, NULL)) {
			result = false;
		}
	}

	for (uint64_t counter = 0; counter < RAND_CHECK_MTHREADS; counter++) {
		if (thread_result(*(threads + counter), &outcome)) {
			st_cleanup(result);
			result = st_dupe(NULLER("Thread join error."));
		}
		else if (outcome) {
			st_cleanup(result);
			result = outcome;
		}
	}

	mm_free(threads);
	return result;
}
