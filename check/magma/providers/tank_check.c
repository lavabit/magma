
/**
 * @file /check/magma/providers/tank_check.c
 *
 * @brief A collection of functions designed to test the storage interface.
 */

#include "magma_check.h"

/**
 * Verifies that we can read all of the data out of the storage tank correctly.
 *
 * @param check_collection The collection of sums to verify.
 * @return Returns true only if all of the objects in the linked list match the expected hash value.
 */
bool_t check_tokyo_tank_cleanup(inx_t *check_collection) {

	inx_cursor_t *cursor;
	check_tank_obj_t *obj;

	if (!(cursor = inx_cursor_alloc(check_collection))) {
		return false;
	}

	while (status() && (obj = inx_cursor_value_next(cursor))) {

		if (!tank_delete(TANK_CHECK_DATA_HNUM, obj->tnum, TANK_CHECK_DATA_UNUM, obj->onum)) {
			log_unit("%lu - tank_delete error", obj->onum);
			inx_cursor_free(cursor);
			return false;
		}
	}

	inx_cursor_free(cursor);

	return true;
}

/**
 * Verifies that we can read all of the data out of the storage tank correctly.
 *
 * @param check_collection The collection of sums to verify.
 * @return Returns true only if all of the objects in the linked list match the expected hash value.
 */
bool_t check_tokyo_tank_verify(inx_t *check_collection) {

	stringer_t *data;
	check_tank_obj_t *obj;
	inx_cursor_t *cursor;

	if (!(cursor = inx_cursor_alloc(check_collection))) {
		return false;
	}

	while (status() && (obj = inx_cursor_value_next(cursor))) {

		if (!(data = tank_load(TANK_CHECK_DATA_HNUM, obj->tnum, TANK_CHECK_DATA_UNUM, obj->onum))) {
			log_unit("%lu - tank_get error", obj->onum);
			inx_cursor_free(cursor);
			return false;
		}

		if (obj->adler32 != hash_adler32(st_data_get(data), st_length_get(data))) {
			log_unit("%lu - adler32 error", obj->onum);
			inx_cursor_free(cursor);
			st_free(data);
			return false;
		}

		if (obj->fletcher32 != hash_fletcher32(st_data_get(data), st_length_get(data))) {
			log_unit("%lu - fletcher32 error", obj->onum);
			inx_cursor_free(cursor);
			st_free(data);
			return false;
		}

		if (obj->crc32 != crc32_checksum(st_data_get(data), st_length_get(data))) {
			log_unit("%lu - crc32 error", obj->onum);
			inx_cursor_free(cursor);
			st_free(data);
			return false;
		}

		if (obj->crc64 != crc64_checksum(st_data_get(data), st_length_get(data))) {
			log_unit("%lu - crc64 error", obj->onum);
			inx_cursor_free(cursor);
			st_free(data);
			return false;
		}

		if (obj->murmur32 != hash_murmur32(st_data_get(data), st_length_get(data))) {
			log_unit("%lu - murmur32 error", obj->onum);
			inx_cursor_free(cursor);
			st_free(data);
			return false;
		}

		if (obj->murmur64 != hash_murmur64(st_data_get(data), st_length_get(data))) {
			log_unit("%lu - murmur64 error", obj->onum);
			inx_cursor_free(cursor);
			st_free(data);
			return false;
		}

		st_free(data);
	}

	inx_cursor_free(cursor);
	return true;
}

/**
 * Recursively loads the files from a directory and stores them using the tank interface.
 *
 * @param location The directory path to search for files.
 * @return Returns false if an error occurs, otherwise true.
 */
bool_t check_tokyo_tank_load(inx_t *check_collection, check_tank_opt_t *opts) {

	multi_t key;
	check_tank_obj_t *obj;
	bool_t outcome = true;
	stringer_t *data = NULL;
	uint32_t max = check_message_max();

	for (uint32_t i = 0; i < max && outcome && status(); i++) {

		// Retrieve data for the current message.
		if (!(data = check_message_get(i))) {
			log_unit("Failed to get the message data. { message = %i }", i);
			outcome = false;
		}

		// Data used for verification.
		if (!(obj = mm_alloc(sizeof(check_tank_obj_t)))) {
			log_unit("Allocating check_tank_obj_t failed. { message = %i }", i);
			outcome = false;
		}
		else {
			obj->adler32 = hash_adler32(st_char_get(data), st_length_int(data));
			obj->fletcher32 = hash_fletcher32(st_char_get(data), st_length_int(data));
			obj->crc32 = crc32_checksum(st_char_get(data), st_length_int(data));
			obj->crc64 = crc64_checksum(st_char_get(data), st_length_int(data));
			obj->murmur32 = hash_murmur32(st_char_get(data), st_length_int(data));
			obj->murmur64 = hash_murmur64(st_char_get(data), st_length_int(data));

			// Request the next storage tank.
			obj->tnum = tank_cycle();

			// Try storing the file data.
			if (!(obj->onum = tank_store(TANK_CHECK_DATA_HNUM, obj->tnum, TANK_CHECK_DATA_UNUM, data, opts->engine))) {
				log_unit("The tank_store function failed. { message = %i }", i);
				outcome = false;
			}
		}

		st_cleanup(data);

		key = mt_set_type(key, M_TYPE_UINT64);
		key.val.u64 = obj->onum;

		if (!inx_insert(check_collection, key, obj)) {
			log_unit("The inx_insert function failed. { message = %i }", i);
			outcome = false;
		}
	}

	return outcome;
}

bool_t check_tokyo_tank(check_tank_opt_t *opts) {

	inx_t *check_collection = NULL;

	if (!(check_collection = inx_alloc(M_INX_LINKED, &mm_free))) {
		return false;
	}

	else if (!check_tokyo_tank_load(check_collection, opts)) {
		inx_free(check_collection);
		return false;
	}

	else if (!check_tokyo_tank_verify(check_collection)) {
		inx_free(check_collection);
		return false;
	}

	else if (TANK_CHECK_DATA_CLEANUP && !check_tokyo_tank_cleanup(check_collection)) {
		inx_free(check_collection);
		return false;
	}

	inx_free(check_collection);
	return true;
}

bool_t check_tokyo_tank_sthread(check_tank_opt_t *opts) {

	uint64_t local_objects = tank_count();

	if (!check_tokyo_tank(opts)) {
		return false;
	}
	else if (TANK_CHECK_DATA_CLEANUP && tank_count() != local_objects) {
		log_unit("The number of objects doesn't match what we started with. {start = %lu / finish = %lu}", local_objects, tank_count());
		return false;
	}

	return true;
}

void check_tokyo_tank_mthread_cnv(check_tank_opt_t *opts) {

	bool_t *result;

	if (!thread_start() || !(result = mm_alloc(sizeof(bool_t)))) {
		log_unit("Unable to setup the thread context.");
		pthread_exit(NULL);
		return;
	}

	*result = check_tokyo_tank(opts);

	thread_stop();
	pthread_exit(result);
	return;
}

bool_t check_tokyo_tank_mthread(check_tank_opt_t *opts) {

	bool_t result = true;
	void *outcome = NULL;
	pthread_t *threads = NULL;
	uint64_t local_objects = tank_count();

	if (!TANK_CHECK_DATA_MTHREADS) {
		return true;
	}
	else if (!(threads = mm_alloc(sizeof(pthread_t) * TANK_CHECK_DATA_MTHREADS))) {
		return false;
	}

	for (uint64_t counter = 0; counter < TANK_CHECK_DATA_MTHREADS; counter++) {
		if (thread_launch(threads + counter, &check_tokyo_tank_mthread_cnv, opts)) {
			result = false;
		}
	}

	for (uint64_t counter = 0; counter < TANK_CHECK_DATA_MTHREADS; counter++) {
		if (thread_result(*(threads + counter), &outcome) || !outcome || !*(bool_t *)outcome) {
			result = false;
		}
		if (outcome) {
			mm_free(outcome);
		}
	}

	mm_free(threads);

	if (TANK_CHECK_DATA_CLEANUP && tank_count() != local_objects) {
		log_unit("The number of objects doesn't match what we started with. {start = %lu / finish = %lu}", local_objects, tank_count());
		return false;
	}

	return result;
}
