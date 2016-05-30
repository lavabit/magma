
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
#include "auth_check.h"

void log_test(stringer_t *test, stringer_t *error) {
	log_enable();
	log_unit("%-64.64s%10.10s\n", st_char_get(test), (!status() ? "SKIPPED" : !error ? "PASSED" : "FAILED"));
	return;
}

START_TEST (check_users_auth_legacy_s) {

	auth_legacy_t *legacy = NULL;
	stringer_t *errmsg = NULL, *buffer = MANAGEDBUF(64);

	// Status output.
	log_disable();

	// Authenticate using the test account and compare the results.
	if (status() && !(legacy = auth_legacy(auth_accounts.creds.username, auth_accounts.creds.password))) {
		errmsg = st_aprint("Failed to generate the legacy authentication credentials.");
	}
	else if (status() && st_cmp_cs_eq(legacy->key, base64_decode_mod(auth_accounts.legacy.key, buffer))) {
		errmsg = st_aprint("The legacy key failed to match the provided test value.");
	}
	else if (status() && st_cmp_cs_eq(legacy->token, base64_decode_mod(auth_accounts.legacy.token, buffer))) {
		errmsg = st_aprint("The legacy authentication token failed to match the provided test value.");
	}

	// Cleanup.
	if (legacy) {
		auth_legacy_free(legacy);
	}

	// Result output.
	log_test(NULLER("USERS / AUTH / LEGACY / SINGLE THREADED:"), errmsg);
	fail_unless(!errmsg, st_char_get(errmsg));
	st_cleanup(errmsg);

} END_TEST

START_TEST (check_users_auth_stacie_s) {

	auth_stacie_t *stacie = NULL;
	stringer_t *errmsg = NULL, *buffer = MANAGEDBUF(128);

	// Status output.
	log_disable();

	// Authenticate using the test account and compare the results.
	if (status() && !(stacie = auth_stacie(0, auth_accounts.creds.username, auth_accounts.creds.password,
		base64_decode_mod(auth_accounts.creds.salt, buffer), NULL, NULL))) {
		errmsg = st_aprint("Failed to generate the legacy authentication credentials.");
	}
	else if (status() && st_cmp_cs_eq(stacie->keys.master, base64_decode_mod(auth_accounts.stacie.master, buffer))) {
		errmsg = st_aprint("The legacy key failed to match the provided test value.");
	}
	else if (status() && st_cmp_cs_eq(stacie->tokens.verification, base64_decode_mod(auth_accounts.stacie.verification, buffer))) {
		errmsg = st_aprint("The legacy authentication token failed to match the provided test value.");
	}

	// Cleanup.
	if (stacie) {
		auth_stacie_free(stacie);
	}

	// Result output.
	log_test(NULLER("USERS / AUTH / STACIE / SINGLE THREADED:"), errmsg);
	fail_unless(!errmsg, st_char_get(errmsg));
	st_cleanup(errmsg);

} END_TEST
