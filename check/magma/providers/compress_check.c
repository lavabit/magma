
/**
 * @file /check/providers/compress_check.c
 *
 * @brief The heart of the suite of unit tests for the Magma provide module.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma_check.h"

bool_t check_compress_sthread(check_compress_opt_t *opts) {

	size_t rlen;
	stringer_t *output;
	compress_t *compress;
	unsigned char *original;

	for (uint64_t i = 0; status() && i < COMPRESS_CHECK_ITERATIONS; i++) {

		// Pick a random length.
		do {
			rlen = (rand() % (COMPRESS_CHECK_SIZE_MAX - COMPRESS_CHECK_SIZE_MIN)) + COMPRESS_CHECK_SIZE_MIN;
		} while (rlen < COMPRESS_CHECK_SIZE_MIN);

		if (!(original = mm_alloc(rlen))) {
			return false;
		}

		// Fill it with random data.
		for (uint64_t j = 0; j < rlen; j++) {
			original[j] = rand() % 256;
		}

		// Compress the data block.
		if (opts->engine == COMPRESS_ENGINE_LZO && !(compress = compress_lzo(PLACER(original, rlen)))) {
			mm_free(original);
			return false;
		} else if (opts->engine == COMPRESS_ENGINE_ZLIB && !(compress = compress_zlib(PLACER(original, rlen)))) {
			mm_free(original);
			return false;
		} else if (opts->engine == COMPRESS_ENGINE_BZIP && !(compress = compress_bzip(PLACER(original, rlen)))) {
			mm_free(original);
			return false;
		}

		// Decompress the data block and verify.
		if (opts->engine == COMPRESS_ENGINE_LZO && !(output = decompress_lzo(compress))) {
			compress_free(compress);
			mm_free(original);
			return false;
		} else if (opts->engine == COMPRESS_ENGINE_ZLIB && !(output = decompress_zlib(compress))) {
			compress_free(compress);
			mm_free(original);
			return false;
		} else if (opts->engine == COMPRESS_ENGINE_BZIP && !(output = decompress_bzip(compress))) {
			compress_free(compress);
			mm_free(original);
			return false;
		}

		// Verify the output is identical to the input.
		if (st_length_get(output) != rlen || memcmp(st_data_get(output), original, rlen)) {
			compress_free(compress);
			mm_free(original);
			st_free(output);
			return false;
		}

		compress_free(compress);
		mm_free(original);
		st_free(output);
	}

	return true;
}

void check_compress_mthread_cnv(check_compress_opt_t *opts) {

	bool_t *result;

	if (!thread_start() || !(result = mm_alloc(sizeof(bool_t)))) {
		log_error("Unable to setup the thread context.");
		pthread_exit(NULL);
		return;
	}

	*result = check_compress_sthread(opts);

	thread_stop();
	pthread_exit(result);
	return;
}

bool_t check_compress_mthread(check_compress_opt_t *opts) {

	bool_t result = true;
	void *outcome = NULL;
	pthread_t *threads = NULL;

	if (!COMPRESS_CHECK_MTHREADS) {
		return true;
	}
	else if (!(threads = mm_alloc(sizeof(pthread_t) * COMPRESS_CHECK_MTHREADS))) {
		return false;
	}

	for (uint64_t counter = 0; counter < COMPRESS_CHECK_MTHREADS; counter++) {
		if (thread_launch(threads + counter, &check_compress_mthread_cnv, opts)) {
			result = false;
		}
	}

	for (uint64_t counter = 0; counter < COMPRESS_CHECK_MTHREADS; counter++) {
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
