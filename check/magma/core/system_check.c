
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
	chr_t *buffer = MEMORYBUF(1024);

	if (!status()) {
		return result;
	}

	for (uint64_t i = 1; i < 33; i++) {

		// No error code between 1 and 32 should be "Unknown" or "Out of range."
		if (!st_cmp_ci_starts(NULLER(strerror_r(i, buffer, 1024)), PLACER("Unknown", 7)) ||
			!st_cmp_cs_eq(NULLER(errno_name(i)), NULLER("UNKNOWN")) || !st_cmp_cs_eq(NULLER(errno_name(i)), NULLER("ERANGE"))) {
			result = false;
		}
	}

	return result;
}
