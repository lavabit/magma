
/**
 * @file /magma/check/users/auth_check.c
 *
 * @brief Ensure the STACIE and legacy objects used by the auth_t module calculate their results correctly.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma_check.h"

void log_test(stringer_t *test, stringer_t *error) {
	log_enable();
	log_unit("%-64.64s%10.10s\n", st_char_get(test), (!status() ? "SKIPPED" : !error ? "PASSED" : "FAILED"));
	return;
}

START_TEST (check_users_auth_legacy_s) {

	stringer_t *errmsg = NULL;

	// Status output.
	log_disable();

	// Run the actual tests here.

	// Result output.
	log_test(NULLER("USERS / AUTH / LEGACY / SINGLE THREADED:"), errmsg);
	fail_unless(!errmsg, st_char_get(errmsg));
	st_cleanup(errmsg);

} END_TEST

START_TEST (check_users_auth_stacie_s) {

	stringer_t *errmsg = NULL;

	// Status output.
	log_disable();

	// Run the actual tests here.

	// Result output.
	log_test(NULLER("USERS / AUTH / STACIE / SINGLE THREADED:"), errmsg);
	fail_unless(!errmsg, st_char_get(errmsg));
	st_cleanup(errmsg);

} END_TEST
