
/**
 * @file /check/providers/tank_check.c
 *
 * @brief A collection of functions designed to test the storage interface.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma_check.h"

extern chr_t *tank_check_data_path;

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
			log_info("%lu - tank_delete error", obj->onum);
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
			log_info("%lu - tank_get error", obj->onum);
			inx_cursor_free(cursor);
			return false;
		}

		if (obj->adler32 != hash_adler32(st_data_get(data), st_length_get(data))) {
			log_info("%lu - adler32 error", obj->onum);
			inx_cursor_free(cursor);
			st_free(data);
			return false;
		}

		if (obj->fletcher32 != hash_fletcher32(st_data_get(data), st_length_get(data))) {
			log_info("%lu - fletcher32 error", obj->onum);
			inx_cursor_free(cursor);
			st_free(data);
			return false;
		}

		if (obj->crc32 != hash_crc32(st_data_get(data), st_length_get(data))) {
			log_info("%lu - crc32 error", obj->onum);
			inx_cursor_free(cursor);
			st_free(data);
			return false;
		}

		if (obj->crc64 != hash_crc64(st_data_get(data), st_length_get(data))) {
			log_info("%lu - crc64 error", obj->onum);
			inx_cursor_free(cursor);
			st_free(data);
			return false;
		}

		if (obj->murmur32 != hash_murmur32(st_data_get(data), st_length_get(data))) {
			log_info("%lu - murmur32 error", obj->onum);
			inx_cursor_free(cursor);
			st_free(data);
			return false;
		}

		if (obj->murmur64 != hash_murmur64(st_data_get(data), st_length_get(data))) {
			log_info("%lu - murmur64 error", obj->onum);
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
bool_t check_tokyo_tank_load(char *location, inx_t *check_collection, check_tank_opt_t *opts) {

	int fd;
	multi_t key;
	DIR *working;
	struct stat info;
	check_tank_obj_t *obj;
	struct dirent *entry;
	char file[1024], *buffer;

	if (!(working = opendir(location))) {
		log_info("Unable to open the data path. {location = %s}", location);
		return false;
	}

	while (status() && (entry = readdir(working))) {

		// Reset.
		errno = 0;
		bzero(file, 1024);
		bzero(&info, sizeof(struct stat));

		// Build an absolute path.
		snprintf(file, 1024, "%s%s%s", location, "/", entry->d_name);

		// If we hit a directory, recursively call the load function.
		if (entry->d_type == DT_DIR && *(entry->d_name) != '.') {
			if (!check_tokyo_tank_load(file, check_collection, opts)) {
				return false;
			}
		}
		// Otherwise if its a regular file try storing it.
		else if (entry->d_type == DT_REG && *(entry->d_name) != '.') {

			// Read the file.
			if ((fd = open(file, O_RDONLY)) < 0) {
				log_info("%s - open error", file);
				closedir(working);
				return false;
			}

			// How big is the file?
			if (fstat(fd, &info) != 0) {
				log_info("%s - stat error", file);
				closedir(working);
				close(fd);
				return false;
			}

			// Allocate a buffer.
			if (!(buffer = mm_alloc(info.st_size + 1))) {
				log_info("%s - malloc error", file);
				closedir(working);
				close(fd);
				return false;
			}

			// Clear the buffer.
			memset(buffer, 0, info.st_size + 1);

			// Read the file.
			if (read(fd, buffer, info.st_size) != info.st_size) {
				log_info("%s - read error", file);
				closedir(working);
				mm_free(buffer);
				close(fd);
				return false;
			}

			close(fd);

			// Data used for verification.
			if (!(obj = mm_alloc(sizeof(check_tank_obj_t)))) {
				log_info("check_tank allocation failed for the file %s", file);
				closedir(working);
				mm_free(buffer);
				return false;
			}

			obj->adler32 = hash_adler32(buffer, info.st_size);
			obj->fletcher32 = hash_fletcher32(buffer, info.st_size);
			obj->crc32 = hash_crc32(buffer, info.st_size);
			obj->crc64 = hash_crc64(buffer, info.st_size);
			obj->murmur32 = hash_murmur32(buffer, info.st_size);
			obj->murmur64 = hash_murmur64(buffer, info.st_size);

			// Request the next storage tank.
			obj->tnum = tank_cycle();

			// Try storing the file data.
			if (!(obj->onum = tank_store(TANK_CHECK_DATA_HNUM, obj->tnum, TANK_CHECK_DATA_UNUM, PLACER(buffer, info.st_size), opts->engine))) {
				log_info("tank_store failed for the file %s", file);
				closedir(working);
				mm_free(buffer);
				mm_free(obj);
				return false;
			}

			mm_free(buffer);

			key = mt_set_type(key, M_TYPE_UINT64);
			key.val.u64 = obj->onum;

			if (!inx_insert(check_collection, key, obj)) {
				log_info("inx_insert failed for the file %s", file);
				closedir(working);
				mm_free(obj);
				return false;
			}
		}
	}

	closedir(working);
	return true;
}

bool_t check_tokyo_tank(check_tank_opt_t *opts) {

	inx_t *check_collection = NULL;

	if (!(check_collection = inx_alloc(M_INX_LINKED, &mm_free))) {
		return false;
	}

	else if (!check_tokyo_tank_load(tank_check_data_path, check_collection, opts)) {
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
		log_info("The number of objects doesn't match what we started with. {start = %lu / finish = %lu}", local_objects, tank_count());
		return false;
	}

	return true;
}

void check_tokyo_tank_mthread_cnv(check_tank_opt_t *opts) {

	bool_t *result;

	if (!thread_start() || !(result = mm_alloc(sizeof(bool_t)))) {
		log_error("Unable to setup the thread context.");
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
		log_info("The number of objects doesn't match what we started with. {start = %lu / finish = %lu}", local_objects, tank_count());
		return false;
	}

	return result;
}
