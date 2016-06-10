
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

START_TEST (check_users_auth_legacy_s) {

	auth_legacy_t *legacy = NULL;
	stringer_t *errmsg = NULL, *buffer = MANAGEDBUF(64);

	if (!status()) {
		log_test("USERS / AUTH / LEGACY / SINGLE THREADED:", errmsg);
		return;
	}

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
	log_test("USERS / AUTH / LEGACY / SINGLE THREADED:", errmsg);
	fail_unless(!errmsg, st_char_get(errmsg));
	st_cleanup(errmsg);

} END_TEST

START_TEST (check_users_auth_stacie_s) {

	auth_stacie_t *stacie = NULL;
	stringer_t *errmsg = NULL, *buffer = MANAGEDBUF(128);

	if (!status()) {
		log_test("USERS / AUTH / STACIE / SINGLE THREADED:", errmsg);
		return;
	}

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
	log_test("USERS / AUTH / STACIE / SINGLE THREADED:", errmsg);
	fail_unless(!errmsg, st_char_get(errmsg));
	st_cleanup(errmsg);

} END_TEST

START_TEST (check_users_auth_challenge_s) {

	auth_t *auth = NULL;
	stringer_t *errmsg = NULL;

	if (!status()) {
		log_test("USERS / AUTH / CHALLENGE / SINGLE THREADED:", errmsg);
		return;
	}

	// Valid Login Attempts
	log_disable();

	// Test a legacy account.
	if (status() && !(auth = auth_challenge(NULLER("princess")))) {
		 errmsg = st_aprint("Auth allocation failed.");
	}

	if (auth) {
		auth_free(auth);
		auth = NULL;
	}

	// Test a STACIE enabled account.
	if (status() && !(auth = auth_challenge(NULLER("stacie")))) {
		 errmsg = st_aprint("Auth allocation failed.");
	}

	if (auth) {
		auth_free(auth);
		auth = NULL;
	}

	log_test("USERS / AUTH / CHALLENGE / SINGLE THREADED:", errmsg);
	fail_unless(!errmsg, st_char_get(errmsg));
	st_cleanup(errmsg);

} END_TEST

START_TEST (check_users_auth_response_s) {

	// We setup an auth_t structure with predefined values for comparison. Note the usernum and bonus values are set
	// to avoid complaints by various tools. They aren't being used for anything, although bonus is technically passed
	// by the response to the STACIE layer.
	auth_t auth = {
		.usernum = 0,
		.username = auth_accounts.creds.username,
		.seasoning = {
			.bonus = 0,
			.salt = base64_decode_mod(auth_accounts.creds.salt, MANAGEDBUF(128)),
			.nonce = base64_decode_mod(auth_accounts.creds.nonce, MANAGEDBUF(128))
		},
		.tokens = {
			.verification = base64_decode_mod(auth_accounts.stacie.verification, MANAGEDBUF(64))
		}
	};
	stringer_t *errmsg = NULL, *token = MANAGEDBUF(64);

	if (!status()) {
		log_test("USERS / AUTH / RESPONSE / SINGLE THREADED:", errmsg);
		return;
	}

	// Valid Login Attempts
	log_disable();

	// Make sure we can validate an ephemeral value. This will also replace the nonce value, so we'll have to free it below.
	if (status() && auth_response(&auth, base64_decode_mod(auth_accounts.stacie.ephemeral, MANAGEDBUF(64)))) {
		 errmsg = st_aprint("The auth response function failed to validate the predefined ephemeral value.");
	}

	// Please note that because the ephemeral variable is random, it could end up randomly picking a value that
	// validates. The odds of this happening are approximately 2^64, or 1 in every 18,446,744,073,709,551,616
	// attempts. If this happens to you, go buy a lottery ticket. Now. Good luck!
	else if (status() && rand_write(token) == 64 && auth_response(&auth, token) != 1) {
		errmsg = st_aprint("The auth response function failed to reject a random, and likely invalid ephemeral value.");
	}

	log_test("USERS / AUTH / RESPONSE / SINGLE THREADED:", errmsg);
	fail_unless(!errmsg, st_char_get(errmsg));
	st_cleanup(errmsg);

} END_TEST

START_TEST (check_users_auth_login_s) {

	auth_t *auth = NULL;
	stringer_t *errmsg = NULL;

	// Valid Login Attempts
	log_disable();

	if (!status()) {
		log_test("USERS / AUTH / LOGIN / SINGLE THREADED:", errmsg);
		return;
	}

	// Test a legacy account.
	if (auth_login(NULLER("magma"), NULLER("test"), &auth)) {
		 errmsg = st_aprint("Auth login failed.");
	}

	if (auth) {
		auth_free(auth);
		auth = NULL;
	}

	// Test a STACIE enabled account.
	if (auth_login(NULLER("stacie"), NULLER("StacieJohnson"), &auth)) {
		 errmsg = st_aprint("Auth login failed.");
	}

	if (auth) {
		auth_free(auth);
		auth = NULL;
	}

	log_test("USERS / AUTH / LOGIN / SINGLE THREADED:", errmsg);
	fail_unless(!errmsg, st_char_get(errmsg));
	st_cleanup(errmsg);

} END_TEST

START_TEST (check_users_auth_address_s) {

	stringer_t *cred;
	char *errmsg = NULL;

	if (!status()) {
		log_test("USERS / AUTH / ADDRESS / SINGLE THREADED:", errmsg);
		return;
	}

	if (!errmsg && (!(cred = auth_sanitize_address(CONSTANT("TEST@DOMAIN.COM"))) || st_cmp_ci_eq(cred, CONSTANT("test@domain.com")))) {
		errmsg = "Address boiler failed [1].";
		//printf("r1 = %lx\n", (unsigned long) cred);	if (cred) printf("xxx: [%.*s] [%u]\n", (int)st_length_get(cred), st_char_get(cred), (int)st_length_get(cred));
	}

	st_cleanup(cred);
	cred = NULL;

	if (!errmsg && (!(cred = auth_sanitize_address(CONSTANT("  TEST  @  DOMAIN.COM  "))) || st_cmp_ci_eq(cred, CONSTANT("test@domain.com")))) {
		errmsg = "Address boiler failed [2].";
		//printf("r2 = %lx\n", (unsigned long) cred);	if (cred) printf("xxx: [%.*s] [%u]\n", (int)st_length_get(cred), st_char_get(cred), (int)st_length_get(cred));
	}

	st_cleanup(cred);
	cred = NULL;

	if (!errmsg && (!(cred = auth_sanitize_address(CONSTANT("test.case+tag@sub.domain.com"))) || st_cmp_ci_eq(cred, CONSTANT("test_case@sub.domain.com")))) {
		errmsg = "Address boiler failed [5].";
	}

	st_cleanup(cred);
	cred = NULL;

	if (!errmsg && (!(cred = auth_sanitize_address(CONSTANT("TEST@DOMAIN.COM"))) || st_cmp_ci_eq(cred, CONSTANT("test@domain.com")))) {
		errmsg = "Address boiler failed [2].";
		//printf("rx2 = %lx\n", (unsigned long) cred);	if (cred) printf("xxx: [%.*s] [%u]\n", (int)st_length_get(cred), st_char_get(cred), (int)st_length_get(cred));
	}

	st_cleanup(cred);
	cred = NULL;

	if (!errmsg && (!(cred = auth_sanitize_address(CONSTANT("  TEST  @  DOMAIN.COM  "))) || st_cmp_ci_eq(cred, CONSTANT("test@domain.com")))) {
		errmsg = "Address boiler failed [5].";
	}

	st_cleanup(cred);
	cred = NULL;

	if (!errmsg && (!(cred = auth_sanitize_address(CONSTANT("test.case+tag@sub.domain.com"))) || st_cmp_ci_eq(cred, CONSTANT("test_case@sub.domain.com")))) {
		errmsg = "Address boiler failed [11].";
	}

	st_cleanup(cred);
	cred = NULL;

	log_test("USERS / AUTH / ADDRESS / SINGLE THREADED:", errmsg);
	fail_unless(!errmsg, errmsg);
} END_TEST

START_TEST (check_users_auth_username_s) {

	stringer_t *cred;
	char *errmsg = NULL;

	if (!status()) {
		log_test("USERS / AUTH / USERNAME / SINGLE THREADED:", errmsg);
		return;
	}

	if (!(cred = auth_sanitize_username(CONSTANT("TEST"))) || st_cmp_ci_eq(cred, CONSTANT("test"))) {
		errmsg = "Username boiler failed [1].";
		//printf("rx1 = %lx\n", (unsigned long) cred);	if (cred) printf("xxx: [%.*s] [%u]\n", (int)st_length_get(cred), st_char_get(cred), (int)st_length_get(cred));
	}

	st_cleanup(cred);
	cred = NULL;

	if (!errmsg && (!(cred = auth_sanitize_username(CONSTANT("TEST.CASE@LAVABIT.COM"))) || st_cmp_ci_eq(cred, CONSTANT("test_case")))) {
		errmsg = "Username boiler failed [3].";
	}

	st_cleanup(cred);
	cred = NULL;

	if (!errmsg && (!(cred = auth_sanitize_username(CONSTANT("test+tag@nerdshack.com"))) || !st_cmp_ci_eq(cred, CONSTANT("test")) ||
		st_cmp_ci_eq(cred, CONSTANT("test@nerdshack.com")))) {
		errmsg = "Username boiler failed [4].";
	}

	st_cleanup(cred);
	cred = NULL;

	if (!errmsg && (!(cred = auth_sanitize_username(CONSTANT("test.case+tag@sub.domain.com"))) || !st_cmp_ci_eq(cred, CONSTANT("test_case")) ||
		st_cmp_ci_eq(cred, CONSTANT("test_case@sub.domain.com")))) {
		errmsg = "Username boiler failed [6].";
	}

	st_cleanup(cred);
	cred = NULL;

	if (!errmsg && (!(cred = auth_sanitize_username(CONSTANT("TEST@DOMAIN.COM"))) || !st_cmp_ci_eq(cred, CONSTANT("test")) || st_cmp_ci_eq(cred, CONSTANT("test@domain.com")))) {
			errmsg = "Username boiler failed [3].";
			//printf("rx2 = %lx\n", (unsigned long) cred);	if (cred) printf("xxx: [%.*s] [%u]\n", (int)st_length_get(cred), st_char_get(cred), (int)st_length_get(cred));
		}

		st_cleanup(cred);
		cred = NULL;

	if (!errmsg && (!(cred = auth_sanitize_username(CONSTANT("  TEST  "))) || st_cmp_ci_eq(cred, CONSTANT("test")))) {
		errmsg = "Username boiler failed [4].";
	}

	st_cleanup(cred);
	cred = NULL;

	if (!errmsg && (!(cred = auth_sanitize_username(CONSTANT("  TEST  @  DOMAIN.COM  "))) || !st_cmp_ci_eq(cred, CONSTANT("test")) || st_cmp_ci_eq(cred, CONSTANT("test@domain.com")))) {
			errmsg = "Username boiler failed [6].";
		}

		st_cleanup(cred);
		cred = NULL;

	if (!errmsg && (!(cred = auth_sanitize_username(CONSTANT("TEST.CASE"))) || st_cmp_ci_eq(cred, CONSTANT("test_case")))) {
		errmsg = "Username boiler failed [7].";
	}

	st_cleanup(cred);
	cred = NULL;

	if (!errmsg && (!(cred = auth_sanitize_username(CONSTANT("TEST.CASE@LAVABIT.COM"))) || st_cmp_ci_eq(cred, CONSTANT("test_case")) || !st_cmp_ci_eq(cred, CONSTANT("test_case@lavabit.com")))) {
		errmsg = "Username boiler failed [8].";
	}

	st_cleanup(cred);
	cred = NULL;

	if (!errmsg && (!(cred = auth_sanitize_username(CONSTANT("test+tag"))) || st_cmp_ci_eq(cred, CONSTANT("test")))) {
		errmsg = "Username boiler failed [9].";
	}

	st_cleanup(cred);
	cred = NULL;

	if (!errmsg && (!(cred = auth_sanitize_username(CONSTANT("test+tag@nerdshack.com"))) || !st_cmp_ci_eq(cred, CONSTANT("test")) || st_cmp_ci_eq(cred, CONSTANT("test@nerdshack.com")))) {
		errmsg = "Username boiler failed [10].";
	}

	st_cleanup(cred);
	cred = NULL;

	log_test("USERS / AUTH / USERNAME / SINGLE THREADED:", errmsg);
	fail_unless(!errmsg, errmsg);
} END_TEST
