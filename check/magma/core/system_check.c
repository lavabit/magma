
/**
 * @file /check/magma/core/system_check.c
 *
 * @brief System helper function checks.
 */

#include "magma_check.h"

bool_t check_system_signames(void) {

	chr_t buf[1024];
	bool_t result = true;

	if (!status()) {
		return result;
	}

	for (uint64_t i = 1; i < SIGUNUSED; i++) {
		if (st_cmp_cs_eq(NULLER(signal_name(i, buf, 1024)), NULLER(strsignal(i)))) {
			result = false;
		}
	}

	return result;
}

bool_t check_system_errnonames(void) {

	bool_t result = true;
	chr_t *buffer1 = MEMORYBUF(1024), *buffer2 = MEMORYBUF(1024);

	if (!status()) {
		return result;
	}

	for (uint64_t i = 1; i < _sys_nerr; i++) {

		// Errors 41 and 58 are aliases, and strerror will return unknown instead of the alias name..
		if (st_cmp_ci_starts(NULLER(strerror_r(i, buffer1, 1024)), PLACER("Unknown", 7)) &&
			st_cmp_cs_eq(NULLER(errno_name(i, buffer2, 1024)), NULLER(strerror_r(i, buffer2, 1024)))) {
			result = false;
		}
	}

	return result;
}
