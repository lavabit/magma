
/**
 * @file /check/magma/providers/dspam_check.c
 *
 * @brief DSPAM unit tests.
 */

#include "magma_check.h"

bool_t check_dspam_binary_sthread(void) {

	size_t len;
	int_t result;
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
		if ((result = dspam_check(DSPAM_CHECK_DATA_UNUM, buffer, &signature)) != -1 && signature && !dspam_train(DSPAM_CHECK_DATA_UNUM, result == 1 ? 1 : 0, signature)) {
			st_free(signature);
			st_free(buffer);
			return false;
		}

		st_cleanup(signature);
		st_free(buffer);
	}

	return true;
}

bool_t check_dspam_mail_sthread(void) {

	int_t outcome;
	uint32_t max = check_message_max();
	stringer_t *signature, *data = NULL;

	//	uint64_t class = 0, train = 0;

	for (uint32_t i = 0; status() && i < max; i++) {

		// Reset.
		signature = NULL;

		// Retrieve data for the current message.
		if (!(data = check_message_get(i))) {
			log_unit("Failed to get the message data. { message = %i }", i);
			return false;
		}

		// Process the email message.
		if ((outcome = dspam_check(DSPAM_CHECK_DATA_UNUM, data, &signature)) == -1 || !signature) {
			log_unit("There was a dspam_check error. { message = %i }", i);
			st_cleanup(signature);
			st_cleanup(data);
			return false;
		}

		// Every 16 or so messages tell the library it made a mistake.
		if ((rand_get_uint8() % 16) == 0 && !dspam_train(DSPAM_CHECK_DATA_UNUM, outcome == 1 ? 1 : 0, signature)) {
			log_unit("There was a dspam_training error. { message = %i }", i);
			st_free(signature);
			st_cleanup(data);
			return false;
		}

//			if ((rand_get_uint8() % 16) == 0) {
//
//				if (dspam_train(DSPAM_CHECK_DATA_UNUM, outcome == 1 ? 1 : 0, signature) != 1) {
//					log_unit("dspam_training error");
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
//				log_unit("CLASSIFIED = %lu / TRAINED = %lu", class, train);
//			}

		st_cleanup(data);
		st_free(signature);
	}

	return true;
}
