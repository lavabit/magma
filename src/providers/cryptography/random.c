
/**
 * @file /magma/providers/cryptography/random.c
 *
 * @brief A collection of functions for generating random data.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

// The thread specific random number generator context.
__thread uint_t rand_ctx = 0;

// Control version = "\t\r\n"
// Array version =  " !\"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\n"
// Print version =  " !\"#$%%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqrstuvwxyz{|}~\n"

/**
 * @brief	Get a random string of data of a specified size, populated with characters from a chosen set.
 * @param	choices		a pointer to a null-terminated string containing a pool of characters from which the contents of the random data will be selected.
 * @param	len			the length, in bytes, of the random string that will be returned to the caller.
 * @return	NULL on failure or a pointer to a managed string containing len bytes of random data on success.
 */
stringer_t * rand_choices(chr_t *choices, size_t len) {

	size_t clen;
	uchr_t *holder;
	stringer_t *result;

	if (len <= 0 || !choices || !(clen = ns_length_get(choices))) {
		log_pedantic("Passed an invalid parameter.");
		return NULL;
	}

	if (!(result = st_alloc(len)) || !(holder = st_data_get(result))) {
		log_pedantic("Could not allocate %zu bytes for the random string.", len);
		return NULL;
	}

	if (!RAND_bytes_d || RAND_bytes_d(holder, len) != 1) {
		log_pedantic("Could not generate a random string of bytes.");
		st_free(result);
		return NULL;
	}

	// Iterate through and replace the random bytes with characters from the choices buffer.
	for (size_t i = 0; i < len; i++) {
		*holder = *(choices + (*holder % clen));
		holder++;
	}

	st_length_set(result, len);

	return result;
}

/**
 * @brief	Fill a managed string with random data.
 * @note	This function generates random data using the cryptographically strong OpenSSL function RAND_bytes().
 * @param	s	the input managed string.
 * @return	0 on failure, or the total number of bytes written to the managed string.
 * @see		RAND_bytes()
 */
size_t rand_write(stringer_t *s) {

	size_t len = 0;
	uchr_t *p = NULL;
	uint32_t opts = 0;

	if (!s || !(p = st_data_get(s)) || !st_valid_destination((opts = *((uint32_t *)s)))) {
		log_pedantic("The supplied string does not have a buffer capable of being written to.");
		return 0;
	}

	// If the string type supports it, store the buffer length instead of using the data length.
	else if (st_valid_avail(opts)) {
		len = st_avail_get(s);
	}
	else {
		len = st_length_get(s);
	}

	if (!RAND_bytes_d || RAND_bytes_d(p, len) != 1) {
		log_pedantic("Could not generate a random string of bytes.");
		return 0;
	}

	if (st_valid_tracked(opts)) {
		st_length_set(s, len);
	}

	return len;
}

/**
 * @brief	Generate a random unsigned 64 bit number.
 * @note	This function attempts to generate random data securely, but falls back on the pseudo-random number generator.
 * @return	the newly generated unsigned 64 bit integer.
 * @see		RAND_bytes()
 */
uint64_t rand_get_uint64(void) {

	uint64_t result;

	if (!RAND_bytes_d || RAND_bytes_d((unsigned char *)&result, sizeof(uint64_t)) != 1) {

		log_pedantic("Entropy failure, falling back to backup entropy source. {error = %s}",
			RAND_bytes_d ? ssl_error_string(bufptr, buflen) : "RAND_bytes() isn't available");

		// Use system supplied pseudo random number generator if an error occurs.
		result = rand_r(&rand_ctx);
		result = result << 32;
		result = result | rand_r(&rand_ctx);
	}

	return result;
}

/**
 * @brief	Generate a random unsigned 32 bit number.
 * @note	This function attempts to generate random data securely, but falls back on the pseudo-random number generator.
 * @return	the newly generated unsigned 32 bit integer.
 * @see		RAND_bytes()
 */
uint32_t rand_get_uint32(void) {

	uint32_t result;

	if (!RAND_bytes_d || RAND_bytes_d((unsigned char *)&result, sizeof(uint32_t)) != 1) {

		log_pedantic("Entropy failure, falling back to backup entropy source. {error = %s}",
			RAND_bytes_d ? ssl_error_string(bufptr, buflen) : "RAND_bytes() isn't available");

		// Use system supplied pseudo random number generator if an error occurs.
		result = rand_r(&rand_ctx);
	}

	return result;
}

/**
 * @brief	Generate a random unsigned 16 bit number.
 * @note	This function attempts to generate random data securely, but falls back on the pseudo-random number generator.
 * @return	the newly generated unsigned 16 bit integer.
 * @see		RAND_bytes()
 */
uint16_t rand_get_uint16(void) {

	uint16_t result;

	if (!RAND_bytes_d || RAND_bytes_d((unsigned char *)&result, sizeof(uint16_t)) != 1) {

		log_pedantic("Entropy failure, falling back to backup entropy source. {error = %s}",
			RAND_bytes_d ? ssl_error_string(bufptr, buflen) : "RAND_bytes() isn't available");

		// Use system supplied pseudo random number generator if an error occurs.
		result = rand_r(&rand_ctx) % UINT16_MAX;
	}

	return result;
}

/**
 * @brief	Generate a random unsigned 8 bit number.
 * @note	This function attempts to generate random data securely, but falls back on the pseudo-random number generator.
 * @return	the newly generated unsigned 8 bit integer.
 * @see		RAND_bytes()
 */
uint8_t rand_get_uint8(void) {

	uint8_t result;

	if (!RAND_bytes_d || RAND_bytes_d((unsigned char *)&result, sizeof(uint8_t)) != 1) {

		log_pedantic("Entropy failure, falling back to backup entropy source. {error = %s}",
			RAND_bytes_d ? ssl_error_string(bufptr, buflen) : "RAND_bytes() isn't available");

		// Use system supplied pseudo random number generator if an error occurs.
		result = rand_r(&rand_ctx) % UINT8_MAX;
	}

	return result;
}

/**
 * @brief	Generate a random signed 64 bit number.
 * @note	This function attempts to generate random data securely, but falls back on the pseudo-random number generator.
 * @return	the newly generated signed 64 bit integer.
 * @see		RAND_bytes()
 */
// QUESTION: Why aren't we just generating unsigned random numbers and casting them to signed values?
int64_t rand_get_int64(void) {

	int64_t result;

	if (!RAND_bytes_d || RAND_bytes_d((unsigned char *)&result, sizeof(int64_t)) != 1) {

		log_pedantic("Entropy failure, falling back to backup entropy source. {error = %s}",
			RAND_bytes_d ? ssl_error_string(bufptr, buflen) : "RAND_bytes() isn't available");

		// Use system supplied pseudo random number generator if an error occurs.
		result = rand_r(&rand_ctx);
		result = result << 32;
		result = result | rand_r(&rand_ctx);

		if (rand_r(&rand_ctx) % 2) {
			result *= -1;
		}

	}

	return result;
}

int32_t rand_get_int32(void) {

	int32_t result;

	if (!RAND_bytes_d || RAND_bytes_d((unsigned char *)&result, sizeof(int32_t)) != 1) {

		log_pedantic("Entropy failure, falling back to backup entropy source. {error = %s}",
			RAND_bytes_d ? ssl_error_string(bufptr, buflen) : "RAND_bytes() isn't available");

		// Use system supplied pseudo random number generator if an error occurs.
		result = rand_r(&rand_ctx) % INT32_MAX;

		if (rand_r(&rand_ctx) % 2) {
			result *= -1;
		}

	}

	return result;
}

int16_t rand_get_int16(void) {

	int16_t result;

	if (!RAND_bytes_d || RAND_bytes_d((unsigned char *)&result, sizeof(int16_t)) != 1) {

		log_pedantic("Entropy failure, falling back to backup entropy source. {error = %s}",
			RAND_bytes_d ? ssl_error_string(bufptr, buflen) : "RAND_bytes() isn't available");

		// Use system supplied pseudo random number generator if an error occurs.
		result = rand_r(&rand_ctx) % INT16_MAX;

		if (rand_r(&rand_ctx) % 2) {
			result *= -1;
		}

	}

	return result;
}

int8_t rand_get_int8(void) {

	int8_t result;

	if (!RAND_bytes_d || RAND_bytes_d((unsigned char *)&result, sizeof(int8_t)) != 1) {

		log_pedantic("Entropy failure, falling back to backup entropy source. {error = %s}",
			RAND_bytes_d ? ssl_error_string(bufptr, buflen) : "RAND_bytes() isn't available");

		// Use system supplied pseudo random number generator if an error occurs.
		result = rand_r(&rand_ctx) % INT8_MAX;

		if (rand_r(&rand_ctx) % 2) {
			result *= -1;
		}

	}

	return result;
}

// LOW: Needs to be called from the thread startup function.
// Shouldn't this be replaced? rand_start() is better...
bool_t rand_thread_start(void) {

	rand_ctx = (uint_t)(time(NULL) | thread_get_thread_id());

	for (uint64_t i = 0; i < magma.iface.cryptography.seed_length; i++) {
		rand_r(&rand_ctx);
	}

	return true;
}

/**
 * @brief	Initialize random number generation services and seed the generator.
 * @note	The default seed source for cryptographically secure generation routines is the system device /dev/random.
 * @return	false on failure, true on success.
 */
bool_t rand_start(void) {

	uint_t seed = (time(NULL) | thread_get_thread_id());

	// Seed the random number generator.
	RAND_load_file_d("/dev/random", magma.iface.cryptography.seed_length);

	// Set the context to the current time, in case were unable to generate a secure random number for a seed.
	srand(rand_r(&seed));
	rand_ctx = rand_r(&seed);

	// Validate that the OpenSSL random number has been seeded properly and is ready for use.
	if (RAND_status_d() != 1) {
		return false;
	}

	return true;
}

void rand_stop(void) {

	RAND_cleanup_d();
	return;
}
