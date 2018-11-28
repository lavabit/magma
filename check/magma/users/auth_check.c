
/**
 * @file /check/magma/users/auth_check.c
 *
 * @brief Ensure the STACIE and legacy objects used by the auth_t module calculate their results correctly.
 */

#include "magma_check.h"

struct {
	struct {
		stringer_t *username;
		stringer_t *password;
		stringer_t *salt;
		stringer_t *nonce;
	} creds;
	struct {
		stringer_t *key;
		stringer_t *token;
	} legacy;
	struct {
		stringer_t *master;
		stringer_t *ephemeral;
		stringer_t *verification;
	} stacie;
} auth_accounts = {
	{
		NULLER("magma"),
		NULLER("password"),
		NULLER("XjIw2JrWPUuJ4tlYR-58TMx7pVspxrTxrZ-LDZDaatyI5xtgPbv2-IaRRAl7mwGWuMEKRO-b5zM_ROjsn-OCNVdHKCp8JLEH7t0jzORxkDFDMdemHnHWxfwkFcML8CBamX46WeY0akGHT9xS9B8OrZuw1lYGAg_fGmWjcrWtJB4"),
		NULLER("T89aOfkFoll8GjldXCV0X1iMdP0MRSJRE6INDB-ufetwMQmxYQ24XVl_YDsZ5Jaj7xjUrPSqo_LF9IWulbUP6uC05tv0bqn8h9sCY9gQ4dmJM0ubFU2gksr-1lzV0d2qM_HMY1Ml2r8LXTvhyxXLoYbv9MqBIWJTL82eaWU_zao")
	}, {
		NULLER("VDLnzI9DMHUEvsCTaEO4fBqnfDMhI_95e7ecS1pUsdoOAzLQ-sKCF_fyV4HIgE9OdqWBPztzzY-45lo7ttLqSw"),
		NULLER("Qqfq1lUO5SNgIioOh4NkUhbDu0relatTG-E80wYNzdCF-1NOQk4Ge8Tt_ieocnfaqtZCj-5zezrcPFVgj7I-pQ")
	}, {
		NULLER("PfiXxnTrWAqeTs4VbZYTv6HMetm8FTkNbokW0jypIAOPhIFbjfLvUDb57mJwQGeOaGd9-l2CaBrjx2EO8RLRUA"),
		NULLER("LlgRSdjxOS_fBEGnSY-wSVpzcSuJotbRq-fhb2wxnJNpnh9gzlGA8LrSFyQuuTkkd3Hzf9dn4pip9xddWiIwzQ"),
		NULLER("wzRp634HPvvooyY-0YWy9FPAGvbBAIrnZYTK3lUZhN-t8jGazJzqqIja4p17aMpf0cQPHiDSJLuBfPY5t_tQ1Q")
	}
};

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

	log_disable();
	auth_t *auth = NULL;
	stringer_t *errmsg = NULL;

	// Valid Login Attempts

	if (!status()) {
		log_test("USERS / AUTH / LOGIN / SINGLE THREADED:", errmsg);
		return;
	}

	// Test a legacy account.
	if (auth_login(NULLER("magma"), NULLER("password"), &auth)) {
		 errmsg = st_aprint("Auth login failed.");
	}

	if (auth) {
		auth_free(auth);
		auth = NULL;
	}

	// Test a STACIE enabled account.
	if (auth_login(NULLER("stacie"), NULLER("password"), &auth)) {
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

START_TEST (check_users_auth_locked_s) {

	log_disable();
	auth_t *auth = NULL;
	int_t result, locks[] = { AUTH_LOCK_EXPIRED, AUTH_LOCK_ADMIN, AUTH_LOCK_ABUSE, AUTH_LOCK_USER, AUTH_LOCK_NONE,
		AUTH_LOCK_EXPIRED, AUTH_LOCK_ADMIN, AUTH_LOCK_ABUSE, AUTH_LOCK_USER, AUTH_LOCK_NONE };
	stringer_t *errmsg = NULL, *usernames[] = { PLACER("lock_expired", 12), PLACER("lock_admin", 10),
		PLACER("lock_abuse", 10), PLACER("lock_user", 9), PLACER("lock_none", 9), PLACER("lock_expired@lavabit.com", 24),
		PLACER("lock_admin@lavabit.com", 22), PLACER("lock_abuse@lavabit.com", 22), PLACER("lock_user@lavabit.com", 21),
		PLACER("lock_none@lavabit.com", 21) };

	if (!status()) {
		log_test("USERS / AUTH / LOCKED / SINGLE THREADED:", errmsg);
		return;
	}

	for (int_t i = 0; i < (sizeof(usernames)/sizeof(stringer_t *)) && !errmsg && status(); i++) {

		// Make sure the user can login first.
		if ((result = auth_login(usernames[i], PLACER("authenticate", 12), &auth))) {
			errmsg = st_aprint("The account locking test failed to log into the test account. { result = %i / username = %.*s }",
				result, st_length_int(usernames[i]), st_char_get(usernames[i]));
		}
		// The login succeeded, so now we make sure the account is properly locked.
		else if (auth && auth->status.locked != locks[i]) {
			errmsg = st_aprint("A locked account failed to return the correct locking value. { lock = %i / username = %.*s }",
				auth->status.locked, st_length_int(usernames[i]), st_char_get(usernames[i]));
		}

		// Cleanup the auth structure if the login succeeded.
		if (auth) {
			auth_free(auth);
			auth = NULL;
		}

	}

	log_test("USERS / AUTH / LOCKED / SINGLE THREADED:", errmsg);
	fail_unless(!errmsg, st_char_get(errmsg));
	st_cleanup(errmsg);

} END_TEST

START_TEST (check_users_auth_inactivity_s) {

	log_disable();
	int64_t result;
	auth_t *auth = NULL;
	stringer_t *errmsg = NULL;

	if (!status()) {
		log_test("USERS / AUTH / INACTIVITY / SINGLE THREADED:", errmsg);
		return;
	}

	// This query will ensure the lock_inactive account is locked for inactivity. Without it, repeat check runs would fail, as the
	// inactivity lock would have already been eliminated.
	if ((result = sql_write(PLACER("UPDATE `Users` SET `locked` = 1, `lock_expiration` = DATE(DATE_ADD(NOW(), INTERVAL 120 DAY)) " \
		"WHERE `userid` = 'lock_inactive';", 126))) < 0) {
		errmsg = st_aprint("Unable to configure the lock_inactive account for the inactivity test. { result = %li }", result);
	}

	// Make sure the user can login first.
	else if ((result = auth_login(PLACER("lock_inactive", 13), PLACER("authenticate", 12), &auth))) {
		errmsg = st_aprint("The inactivity lock test failed to log into the test account. { result = %li / username = lock_inactive }", result);
	}

	// The login succeeded, now we need to make sure the inactivity lock on the account has been cleared.
	else if (auth && auth->status.locked != AUTH_LOCK_NONE) {
		errmsg = st_aprint("A locked account failed to return the correct locking value. { lock = %i / username = lock_inactive }",	auth->status.locked);
	}

	// Cleanup the auth structure if the login succeeded.
	if (auth) {
		auth_free(auth);
		auth = NULL;
	}

	// The SQL query above, is designed to place an inactivity lock on the lock_inactive account, but it might not affect any rows, as the
	// account could already be locked. However, once the test completes, the same query must affect a row, or this test is a failure.
	if (!errmsg && (result = sql_write(PLACER("UPDATE `Users` SET `locked` = 1, `lock_expiration` = DATE(DATE_ADD(NOW(), INTERVAL 120 DAY)) " \
		"WHERE `userid` = 'lock_inactive';", 126))) != 1) {
		errmsg = st_aprint("The lock_inactive account update failed, indicating the lock was never removed, and the test is a " \
			"failure. { result = %li }", result);
	}

	log_test("USERS / AUTH / INACTIVITY / SINGLE THREADED:", errmsg);
	fail_unless(!errmsg, st_char_get(errmsg));
	st_cleanup(errmsg);

} END_TEST

START_TEST (check_users_auth_address_s) {

	char *errmsg = NULL;
	stringer_t *address = NULL;

	if (!status()) {
		log_test("USERS / AUTH / ADDRESS / SINGLE THREADED:", errmsg);
		return;
	}

	if (!errmsg && (!(address = auth_sanitize_address(CONSTANT("TEST@DOMAIN.COM"))) || st_cmp_cs_eq(address, CONSTANT("test@domain.com")))) {
		errmsg = "Address boiler failed [1].";
		//printf("r1 = %lx\n", (unsigned long) cred);	if (cred) printf("xxx: [%.*s] [%u]\n", (int)st_length_get(cred), st_char_get(cred), (int)st_length_get(cred));
	}

	st_cleanup(address);
	address = NULL;

	if (!errmsg && (!(address = auth_sanitize_address(CONSTANT("  TEST  @  DOMAIN.COM  "))) || st_cmp_cs_eq(address, CONSTANT("test@domain.com")))) {
		errmsg = "Address boiler failed [2].";
		//printf("r2 = %lx\n", (unsigned long) cred);	if (cred) printf("xxx: [%.*s] [%u]\n", (int)st_length_get(cred), st_char_get(cred), (int)st_length_get(cred));
	}

	st_cleanup(address);
	address = NULL;

	if (!errmsg && (!(address = auth_sanitize_address(CONSTANT("test.case+tag@sub.domain.com"))) || st_cmp_cs_eq(address, CONSTANT("test_case@sub.domain.com")))) {
		errmsg = "Address boiler failed [5].";
	}

	st_cleanup(address);
	address = NULL;

	if (!errmsg && (!(address = auth_sanitize_address(CONSTANT("TEST@DOMAIN.COM"))) || st_cmp_cs_eq(address, CONSTANT("test@domain.com")))) {
		errmsg = "Address boiler failed [2].";
		//printf("rx2 = %lx\n", (unsigned long) cred);	if (cred) printf("xxx: [%.*s] [%u]\n", (int)st_length_get(cred), st_char_get(cred), (int)st_length_get(cred));
	}

	st_cleanup(address);
	address = NULL;

	if (!errmsg && (!(address = auth_sanitize_address(CONSTANT("  TEST  @  DOMAIN.COM  "))) || st_cmp_cs_eq(address, CONSTANT("test@domain.com")))) {
		errmsg = "Address boiler failed [5].";
	}

	st_cleanup(address);
	address = NULL;

	if (!errmsg && (!(address = auth_sanitize_address(CONSTANT("test.case+tag@sub.domain.com"))) || st_cmp_cs_eq(address, CONSTANT("test_case@sub.domain.com")))) {
		errmsg = "Address boiler failed [11].";
	}

	st_cleanup(address);
	address = NULL;

	log_test("USERS / AUTH / ADDRESS / SINGLE THREADED:", errmsg);
	fail_unless(!errmsg, errmsg);
} END_TEST

START_TEST (check_users_auth_username_s) {

	bool_t result = true;
	stringer_t *username;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (!status()) {
		log_test("USERS / AUTH / USERNAME / SINGLE THREADED:", errmsg);
		return;
	}

	if (!(username = auth_sanitize_username(CONSTANT("TEST"))) || st_cmp_cs_eq(username, CONSTANT("test"))) {
		st_sprint(errmsg, "Username boiler failed [1].");
		result = false;
	}

	st_cleanup(username);
	username = NULL;

	if (result && (!(username = auth_sanitize_username(CONSTANT("TEST.CASE@LAVABIT.COM"))) || st_cmp_cs_eq(username, CONSTANT("test_case")))) {
		st_sprint(errmsg, "Username boiler failed [2].");
		result = false;
	}

	st_cleanup(username);
	username = NULL;

	if (result && (!(username = auth_sanitize_username(CONSTANT("test+tag@LAVABIT.com"))) ||
		st_cmp_cs_eq(username, CONSTANT("test")))) {
		st_sprint(errmsg, "Username boiler failed [3].");
		result = false;
	}

	st_cleanup(username);
	username = NULL;

	if (result && (!(username = auth_sanitize_username(CONSTANT("test.case+tag@sub.domain.com"))) ||
		st_cmp_cs_eq(username, CONSTANT("test_case@sub.domain.com")))) {
		st_sprint(errmsg, "Username boiler failed [4].");
		result = false;
	}

	st_cleanup(username);
	username = NULL;

	if (result && (!(username = auth_sanitize_username(CONSTANT("TEST@DOMAIN.COM"))) ||
		st_cmp_cs_eq(username, CONSTANT("test@domain.com")))) {
		st_sprint(errmsg, "Username boiler failed [5].");
		result = false;
	}

	st_cleanup(username);
	username = NULL;

	if (result && (!(username = auth_sanitize_username(CONSTANT("  TEST  "))) ||
		st_cmp_cs_eq(username, CONSTANT("test")))) {
		st_sprint(errmsg, "Username boiler failed [6].");
		result = false;
	}

	st_cleanup(username);
	username = NULL;

	if (result && (!(username = auth_sanitize_username(CONSTANT("  TEST  @  DOMAIN.COM  "))) ||
		st_cmp_cs_eq(username, CONSTANT("test@domain.com")))) {
		st_sprint(errmsg, "Username boiler failed [7].");
		result = false;
	}

	st_cleanup(username);
	username = NULL;

	if (result && (!(username = auth_sanitize_username(CONSTANT("TEST.CASE"))) ||
		st_cmp_cs_eq(username, CONSTANT("test_case")))) {
		st_sprint(errmsg, "Username boiler failed [8].");
		result = false;
	}

	st_cleanup(username);
	username = NULL;

	if (result && (!(username = auth_sanitize_username(CONSTANT("TEST.CASE@LAVABIT.COM"))) ||
		st_cmp_cs_eq(username, CONSTANT("test_case")))) {
		st_sprint(errmsg, "Username boiler failed [9].");
		result = false;
	}

	st_cleanup(username);
	username = NULL;

	if (result && (!(username = auth_sanitize_username(CONSTANT("TEST+tag"))) ||
		st_cmp_cs_eq(username, CONSTANT("test")))) {
		st_sprint(errmsg, "Username boiler failed [10].");
		result = false;
	}

	st_cleanup(username);
	username = NULL;

	if (result && (!(username = auth_sanitize_username(CONSTANT("test+tag@nerdshack.com"))) ||
		st_cmp_cs_eq(username, CONSTANT("test@nerdshack.com")))) {
		st_sprint(errmsg, "Username boiler failed [11].");
		result = false;
	}

	st_cleanup(username);
	username = NULL;

	log_test("USERS / AUTH / USERNAME / SINGLE THREADED:", errmsg);
	ck_assert_msg(result, st_char_get(errmsg));

} END_TEST
