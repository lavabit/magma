
/**
 * @file /check/providers/dspam_check.c
 *
 * @brief DSPAM unit tests.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma_check.h"

extern chr_t *dspam_check_data_path;

bool_t check_dspam_binary_sthread(chr_t *location) {

	size_t len;
	int_t outcome;
	stringer_t *buffer, *signature;

	for (uint_t i = 0; status() && i < DSPAM_CHECK_ITERATIONS; i++) {

		signature = NULL;

		do {
			len = (rand() % (DSPAM_CHECK_SIZE_MAX - DSPAM_CHECK_SIZE_MIN)) + DSPAM_CHECK_SIZE_MIN;
		} while (len < DSPAM_CHECK_SIZE_MIN);

		if (!(buffer = st_alloc(len)) || rand_write(buffer) != len) {
			st_cleanup(buffer);
			return false;
		}

		// Since DSPAN uses NULL terminated strings we have to any NULL bytes in the buffer.
		st_replace(&buffer, PLACER("\0", 1), PLACER("\255", 1));

		// Process the email message. Ignore process errors since were feeding in random data. But if the buffer is processed, assume the signature is valid or error.
		if ((outcome = dspam_check(DSPAM_CHECK_DATA_UNUM, buffer, &signature)) != -1 && signature && !dspam_train(DSPAM_CHECK_DATA_UNUM, outcome == 1 ? 1 : 0, signature)) {
			st_free(signature);
			st_free(buffer);
			return false;
		}

		st_cleanup(signature);
		st_free(buffer);
	}

	return true;
}

bool_t check_dspam_mail_sthread(chr_t *location) {

	DIR *working;
	int_t outcome;
	struct dirent *entry;
	stringer_t *buffer, *signature;
	char path[MAGMA_FILEPATH_MAX + 1];

//	uint64_t class = 0, train = 0;

	if (!(working = opendir(location ? location : dspam_check_data_path))) {
		log_info("Unable to open the data path. { location = %s / errno = %i / strerror = %s }", location ? location : dspam_check_data_path, errno, strerror_r(errno, MEMORYBUF(1024), 1024));
		return false;
	}

	while (status() && (entry = readdir(working))) {

		// Reset.
		signature = NULL;
		mm_wipe(path, sizeof(path));

		// Build an absolute path.
		snprintf(path, 1024, "%s%s%s", location ? location : dspam_check_data_path, "/", entry->d_name);

		// If we hit a directory, recursively call the load function.
		if (entry->d_type == DT_DIR && *(entry->d_name) != '.') {
			if (!check_dspam_mail_sthread(path)) {
				return false;
			}
		}
		// Otherwise if its a regular file try storing it.
		else if (entry->d_type == DT_REG && *(entry->d_name) != '.') {

			if (!(buffer = file_load(path))) {
				log_info("%s - read error", path);
				closedir(working);
				return false;
			}

			// Process the email message.
			if ((outcome = dspam_check(DSPAM_CHECK_DATA_UNUM, buffer, &signature)) == -1 || !signature) {
				log_info("dspam_check error");
				st_cleanup(signature);
				closedir(working);
				st_free(buffer);
				return false;
			}

			// Every 16 or so messages tell the library it made a mistake.
			if ((rand_get_uint8() % 16) == 0 && !dspam_train(DSPAM_CHECK_DATA_UNUM, outcome == 1 ? 1 : 0, signature)) {
				log_info("dspam_training error");
				st_free(signature);
				closedir(working);
				st_free(buffer);
				return false;
			}

//			if ((rand_get_uint8() % 16) == 0) {
//
//				if (dspam_train(DSPAM_CHECK_DATA_UNUM, outcome == 1 ? 1 : 0, signature) != 1) {
//					log_info("dspam_training error");
//					st_free(signature);
//					closedir(working);
//					st_free(buffer);
//					return false;
//				}
//
//				class++;
//				train++;
//			}
//			else {
//				class++;
//			}
//
//			if ((class % 100) == 0) {
//				log_pedantic("CLASSIFIED = %lu / TRAINED = %lu", class, train);
//			}

			st_free(signature);
			st_free(buffer);
		}
	}

	closedir(working);
	return true;
}
