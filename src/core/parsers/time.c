
/**
 * @file /magma/core/parsers/time.c
 *
 * @brief	Functions used to parse time.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Get the number of seconds until midnight.
 * @return	an unsigned 64-bit integer containing the number of seconds until midnight, or 86400 on failure.
 */
uint64_t time_till_midnight(void) {

	time_t utime;
	struct tm now;
	uint64_t result = 86400;

	// Get the local time.
	if ((utime = time(NULL)) == ((time_t)-1) || !(localtime_r(&utime, &now))) {
		return result;
	}

	result -= (now.tm_hour * 3600);
	result -= (now.tm_min * 60);
	result -= (now.tm_sec);

	return result;
}

/**
 * @brief	Get the current date as an integer of the form YYYYMMDD.
 * @return	0 on failure or a 64-bit unsigned integer containing the formatted current date on success.
 */
uint64_t time_datestamp(void) {

	time_t utime;
	struct tm now;
	uint64_t result = 0;

	// Get the local time.
	if ((utime = time(NULL)) == ((time_t)-1) || !(localtime_r(&utime, &now))) {
		return 0;
	}

	result += now.tm_mday;
	result += (now.tm_mon + 1) * 100;
	result += (now.tm_year + 1900) * 10000;

	return result;
}

/**
 * @brief	Get a specified time as a formatted string.
 * @param	s	a managed string where the formatted time is to be stored.
 * @param	format	a null-terminated string containing the format of the time string.
 * @param	moment	the specified time to be formatted, as a UNIX time value.
 * @return	NULL on failure, or a pointer to the managed string containing the result on success.
 */
stringer_t * time_print_local(stringer_t *s, chr_t *format, time_t moment) {

	size_t len;
	struct tm localtime;

	mm_wipe(&localtime, sizeof(struct tm));

	if (!s || (moment = time(NULL)) == ((time_t) -1) || !localtime_r(&moment, &localtime)) {
		log_pedantic("Could not determine the proper time.");
		return NULL;
	}

	else if ((len = strftime(st_char_get(s), st_avail_get(s), format, &localtime)) <= 0) {
		log_pedantic("Could not build the date string.");
		return NULL;
	}

	st_length_set(s, len);
	return s;
}

/***
 * Convert the localized moment to GMT time and prints the formtted string.
 */
/**
 * @brief	Get a specified time as a formatted string converted to GMT.
 * @param	s	a managed string where the formatted time is to be stored.
 * @param	format	a null-terminated string containing the format of the time string.
 * @param	moment	the specified time to be formatted, as a UNIX time value.
 * @return	NULL on failure, or a pointer to the managed string containing the result on success.
 */
stringer_t * time_print_gmt(stringer_t *s, chr_t *format, time_t moment) {

	size_t len;
	struct tm gmt;

	mm_wipe(&gmt, sizeof(struct tm));

	if (!s || !gmtime_r(&moment, &gmt)) {
		log_pedantic("Could not determine the proper time.");
		return NULL;
	}

	else if ((len = strftime(st_char_get(s), st_avail_get(s), format, &gmt)) <= 0) {
		log_pedantic("Could not build the date string.");
		return NULL;
	}

	st_length_set(s, len);
	return s;

}
