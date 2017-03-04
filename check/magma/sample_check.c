
/**
 * @file /check/magma/sample_check.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 */

#include "magma_check.h"

#define COMPONENT_CHECK_MTHREADS 2
#define COMPONENT_CHECK_ITERATIONS 16

bool_t check_component_test1_sthread(stringer_t *errmsg) {

	uchr_t y;
	stringer_t *s = NULL;

	for (uint64_t i = 0; status() && i < COMPONENT_CHECK_ITERATIONS; i++) {

		s = st_aprint("Hello world.");
		y = *((uchr_t *)st_data_get(s));
		if (!s || y != 'H') {
			st_cleanup(s);
			st_sprint(errmsg, "String check failed.");
			return false;
		}
		st_free(s);
	}

	return true;
}

void check_component_test2_wrap(void) {

	stringer_t *errmsg = MANAGEDBUF(128);

	if (!thread_start()) {
		log_unit("Unable to setup the thread context.");
		pthread_exit(st_dupe_opts(MANAGED_T | CONTIGUOUS | HEAP, NULLER("Thread startup error.")));
		return;
	}

	if (!check_component_test1_sthread(errmsg)) {
		thread_stop();
		pthread_exit(st_dupe_opts(MANAGED_T | CONTIGUOUS | HEAP, errmsg));
		return;
	}

	thread_stop();
	pthread_exit(NULL);
	return;
}

bool_t check_component_test2_mthread(stringer_t *errmsg) {

	void *outcome = NULL;
	bool_t result = true;
	pthread_t *threads = NULL;

	// Determines the number of threads spawned.
	if (!COMPONENT_CHECK_MTHREADS) {
		return true;
	}
	else if (!(threads = mm_alloc(sizeof(pthread_t) * COMPONENT_CHECK_MTHREADS))) {
		st_sprint(errmsg, "Thread allocation failed.");
		return false;
	}

	// Launch the threads.
	for (uint64_t counter = 0; counter < COMPONENT_CHECK_MTHREADS; counter++) {
		if (thread_launch(threads + counter, &check_component_test2_wrap, NULL)) {
			st_sprint(errmsg, "Thread launch failed.");
			result = false;
		}
	}

	// Wait for the threads to finish and check the output value for an error indication.
	for (uint64_t counter = 0; counter < COMPONENT_CHECK_MTHREADS; counter++) {
		if (thread_result(*(threads + counter), &outcome)) {
			if (!errmsg) st_sprint(errmsg, "Thread join error.");
			result = false;
		}
		else if ((threads + counter) && outcome) {
			if (!errmsg) st_sprint(errmsg, "Threaded test failed. {%.*s}", st_length_int((stringer_t *)outcome), st_char_get((stringer_t *)outcome));
			st_free(outcome);
			result = false;
		}
	}

	mm_free(threads);
	return result;
}

START_TEST (check_component_s) {
	bool_t result = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (status()) result = check_component_test1_sthread(errmsg);
	//if (status() && result) result = check_component_test2_sthread(errmsg);

	log_test("COMPONENT / INTERFACE / SINGLE THREADED:", errmsg);
	ck_assert_msg(result, st_char_get(errmsg));
}
END_TEST

START_TEST (check_component_m) {
	bool_t result = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (status()) result = check_component_test2_mthread(errmsg);
	//if (status() && result) result = check_component_test2_mthread(errmsg);

	log_test("COMPONENT / INTERFACE / MILTITHREADED:", errmsg);
	ck_assert_msg(result, st_char_get(errmsg));
}
END_TEST

Suite * suite_check_sample(void) {

	Suite *s = suite_create("\tSample");

	suite_check_testcase(s, "COMPONENT", "Component/S", check_component_s);
	suite_check_testcase(s, "COMPONENT", "Component/S", check_component_m);

	return s;

}
