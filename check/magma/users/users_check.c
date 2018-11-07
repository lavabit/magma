
/**
 * @file /check/magma/users/users_check.c
 *
 * @brief Checks the code used to handle user data.
 */

#include "magma_check.h"

START_TEST (check_users_register_s) {

	log_disable();
	uint16_t plan;
	connection_t con;
	uint64_t usernum = 0;
	bool_t outcome = true;
	int64_t transaction = -1;
	stringer_t *errmsg = MANAGEDBUF(128), *username = NULL, *password = NULL;

	// If the check process hasn't been aborted, register a new user account using a randomly generated username/password.
	if (status()) {

		// Pass in a blank connection structure. This will be used to store the registration IP address.
		mm_wipe(&con, sizeof(connection_t));

		// Randomly select one of the available plans. Valid values are 1 through 6.
		plan = (rand_get_uint16() % 5) + 1;

		// Generate a random, 20 digit string of numbers to use as a unique suffix for the username, with the pattern
		// check_user_X, which ensures the username is always unique.
		if (!(password = rand_choices("0123456789", 8, MANAGEDBUF(8))) || !(username = st_quick(MANAGEDBUF(64), "check_user_%.*s", st_length_int(password), st_char_get(password)))) {
			st_sprint(errmsg, "An internal error occurred. Unable to generate a random username for the registration test.");
			outcome = false;
		}

		// We reuse the password buffer, which ensures the password is distinct from the username, and helps us confirm that supplying
		// an output buffer doesn't lead to a memory leak.
		else if (!(password = rand_choices("0123456789", 20, MANAGEDBUF(20))) || st_length_get(password) != 20 || st_length_get(username) != 19) {
			st_sprint(errmsg, "An internal error occurred. Unable to generate a random password for the registration test.");
			outcome = false;
		}

		// Start the transaction.
		else if ((transaction = tran_start()) == -1) {
			st_sprint(errmsg, "An internal error occurred. Unable to start the transaction.");
			outcome = false;
		}

		// Database insert.
		else if (register_data_insert_user(&con, plan, username, password, transaction, &usernum) != 0) {
			st_sprint(errmsg, "User registration failed!.");
			tran_rollback(transaction);
			outcome = false;
		}

		// Were finally done.
		else {
			tran_commit(transaction);
		}

		// Confirm the user was created.
		if (outcome && sql_num_rows(st_quick(MANAGEDBUF(1024), "SELECT usernum, userid FROM Users WHERE userid = '%.*s';", st_length_int(username), st_char_get(username))) != 1) {
			st_sprint(errmsg, "Verification of the user table entry after registering a system user failed.");
			outcome = false;
		}
		else if (outcome && sql_num_rows(st_quick(MANAGEDBUF(1024), "SELECT usernum, address FROM Mailboxes WHERE address = '%.*s@%.*s';",
			st_length_int(username), st_char_get(username), st_length_int(magma.system.domain), st_char_get(magma.system.domain))) != 1) {
			st_sprint(errmsg, "Verification of the mailbox table entry after registering a system user failed.");
			outcome = false;
		}

	}

	mark_point();

	// If the first test passed, try again with a fully qualified username.
	if (status() && outcome) {

		// Pass in a blank connection structure. This will be used to store the registration IP address.
		mm_wipe(&con, sizeof(connection_t));

		// Randomly select one of the available plans. Valid values are 1 through 6.
		plan = (rand_get_uint16() % 5) + 1;

		// Generate a random, 20 digit string of numbers to use as a unique suffix for the username, with the pattern
		// check_user_X@example.com, which ensures the username is always unique.
		if (!(password = rand_choices("0123456789", 8, MANAGEDBUF(8))) || !(username = st_aprint("check_user_%.*s@example.com", st_length_int(password), st_char_get(password)))) {
			st_sprint(errmsg, "An internal error occurred. Unable to generate a random username for the registration test.");
			outcome = false;
		}

		// We reuse the password buffer, which ensures the password is distinct from the username, and helps us confirm that supplying
		// an output buffer doesn't lead to a memory leak.
		else if (!(password = rand_choices("0123456789", 20, MANAGEDBUF(20))) || st_length_get(password) != 20 || st_length_get(username) != 31) {
			st_sprint(errmsg, "An internal error occurred. Unable to generate a random password for the registration test.");
			outcome = false;
		}

		// Start the transaction.
		else if ((transaction = tran_start()) == -1) {
			st_sprint(errmsg, "An internal error occurred. Unable to start the transaction.");
			outcome = false;
		}

		// Database insert.
		else if (register_data_insert_user(&con, plan, username, password, transaction, &usernum) != 0) {
			st_sprint(errmsg, "User registration failed!.");
			tran_rollback(transaction);
			outcome = false;
		}

		// Were finally done.
		else {
			tran_commit(transaction);
		}

		// Confirm the user was created.
		if (outcome && sql_num_rows(st_quick(MANAGEDBUF(1024), "SELECT usernum, userid FROM Users WHERE userid = '%.*s';", st_length_int(username), st_char_get(username))) != 1) {
			st_sprint(errmsg, "Verification of the user table entry after registering a fully qualified user failed.");
			outcome = false;
		}
		else if (outcome && sql_num_rows(st_quick(MANAGEDBUF(1024), "SELECT usernum, address FROM Mailboxes WHERE address = '%.*s';", st_length_int(username), st_char_get(username))) != 1) {
			st_sprint(errmsg, "Verification of the mailbox table entry after registering a fully qualified user failed.");
			outcome = false;
		}

	}

	log_test("USERS / REGISTER / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));

}
END_TEST

START_TEST (check_users_meta_valid_s) {

	log_disable();
	auth_t *auth = NULL;
	bool_t result = true;
	meta_user_t *pop = NULL, *imap = NULL;
	stringer_t *errmsg = MANAGEDBUF(1024);
	stringer_t *usernames[] = { PLACER("magma", 5) };
	stringer_t *passwords[] = { PLACER("password", 8) };

	// The registration check must be run frist, otherwise we won't have a user to check against.
//	if (status() && (!check_username || !check_password)) {
//		check_users_register_s(0);
//		usernames[1] = check_username;
//		passwords[1] = check_password;
//	}

	for (int_t i = 0; i < (sizeof(usernames)/sizeof(stringer_t *)) && result && status(); i++) {

		if (st_empty(usernames[i]) || st_empty(passwords[i])) {
			st_sprint(errmsg, "User meta login check failed. The username and password were invalid. { i = %i }", i);
			result = false;
		}

		else if (auth_login(usernames[i], passwords[i], &auth)) {
			st_sprint(errmsg, "User meta login check failed. Authentication failure. { username =  %.*s / password = %.*s }",
				st_length_int(usernames[i]), st_char_get(usernames[i]), st_length_int(passwords[i]), st_char_get(passwords[i]));
			result = false;
		}

		else if ((meta_get(auth->usernum, auth->username, auth->seasoning.salt, auth->keys.master, auth->tokens.verification,
			META_PROTOCOL_POP, META_GET_MESSAGES | META_GET_KEYS, &(pop)))) {
			st_sprint(errmsg, "User meta login check failed. Get user metadata failure. { username =  %.*s / password = %.*s }",
				st_length_int(usernames[i]), st_char_get(usernames[i]), st_length_int(passwords[i]), st_char_get(passwords[i]));
			result = false;
		}

		else if ((meta_get(auth->usernum, auth->username, auth->seasoning.salt, auth->keys.master, auth->tokens.verification,
			META_PROTOCOL_IMAP, META_GET_KEYS | META_GET_FOLDERS | META_GET_CONTACTS | META_GET_MESSAGES, &(imap)))) {
			st_sprint(errmsg, "User meta login check failed. Get user metadata failure. { username =  %.*s / password = %.*s }",
				st_length_int(usernames[i]), st_char_get(usernames[i]), st_length_int(passwords[i]), st_char_get(passwords[i]));
			result = false;
		}

		if (auth) auth_free(auth);
		if (pop) meta_inx_remove(pop->usernum, META_PROTOCOL_POP);
		if (imap) meta_inx_remove(imap->usernum, META_PROTOCOL_IMAP);

		auth = NULL;
		imap = pop = NULL;

	}

	log_test("USERS / META / VALID / SINGLE THREADED:", errmsg);
	ck_assert_msg(result, st_char_get(errmsg));
}
END_TEST

START_TEST (check_users_meta_invalid_s) {

	log_disable();
	auth_t *auth = NULL;
	bool_t result = true;
	meta_user_t *user = NULL;
	stringer_t *errmsg = MANAGEDBUF(1024);
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = 0 };
	stringer_t *usernames[] = { NULLER("magma") }, *passwords[] = { NULLER("password") };

	for (int_t i = 0; i < (sizeof(usernames)/sizeof(stringer_t *)) && result && status(); i++) {

		if (auth_login(usernames[i], passwords[i], &auth)) {
			 st_sprint(errmsg, "User meta login check failed. Authentication failure. { username =  %.*s / password = %.*s }",
			 	 st_length_int(usernames[i]), st_char_get(usernames[i]), st_length_int(passwords[i]), st_char_get(passwords[i]));
			 result = false;
		}
		else {

			// This will flush the object cache, so the magma user meta structure isn't loaded from cache. Without the cached structure,
			// the get function will need to perform the DIME key decryption, which should fail when we provide it with an invalid master key.
			inx_lock_read(objects.meta);
			key.val.u64 = auth->usernum;
			inx_delete(objects.meta, key);
			inx_unlock(objects.meta);
		}

		// The verification token is XOR'ed with the master key, which should result in a failure.
		if (result && auth && !(meta_get(auth->usernum, auth->username, auth->seasoning.salt, st_xor(auth->keys.master, auth->tokens.verification, MANAGEDBUF(64)),
			auth->tokens.verification, META_PROTOCOL_POP, META_GET_MESSAGES | META_GET_KEYS, &(user)))) {
			st_sprint(errmsg, "User meta login check failed. Get user metadata failure. { username =  %.*s / password = %.*s }",
				st_length_int(usernames[i]), st_char_get(usernames[i]), st_length_int(passwords[i]), st_char_get(passwords[i]));
			result = false;
		}

		else if (result && !(meta_get(auth->usernum, auth->username, auth->seasoning.salt, auth->keys.master, st_xor(auth->keys.master, auth->tokens.verification,
			MANAGEDBUF(64)), META_PROTOCOL_POP, META_GET_MESSAGES | META_GET_KEYS, &(user)))) {
			st_sprint(errmsg, "User meta login check failed. Get user metadata failure. { username =  %.*s / password = %.*s }",
				st_length_int(usernames[i]), st_char_get(usernames[i]), st_length_int(passwords[i]), st_char_get(passwords[i]));
			result = false;
		}

		if (auth) auth_free(auth);
		if (user) meta_inx_remove(user->usernum, META_PROTOCOL_POP);

		auth = NULL;
		user = NULL;
	}

	log_test("USERS / META / INVALID / SINGLE THREADED:", errmsg);
	ck_assert_msg(result, st_char_get(errmsg));
}
END_TEST

Suite * suite_check_users(void) {

	Suite *s = suite_create("\tUsers");

	suite_check_testcase(s, "USERS", "Auth Usernames/S", check_users_auth_username_s);
	suite_check_testcase(s, "USERS", "Auth Addresses/S", check_users_auth_address_s);
	suite_check_testcase(s, "USERS", "Auth Legacy/S", check_users_auth_legacy_s);
	suite_check_testcase(s, "USERS", "Auth Stacie/S", check_users_auth_stacie_s);
	suite_check_testcase(s, "USERS", "Auth Challenge/S", check_users_auth_challenge_s);
	suite_check_testcase(s, "USERS", "Auth Response/S", check_users_auth_response_s);
	suite_check_testcase(s, "USERS", "Auth Login/S", check_users_auth_login_s);

	suite_check_testcase(s, "USERS", "Auth Locked/S", check_users_auth_locked_s);
	suite_check_testcase(s, "USERS", "Auth Inactivity/S", check_users_auth_inactivity_s);

	suite_check_testcase(s, "USERS", "Register/S", check_users_register_s);

	suite_check_testcase(s, "USERS", "Meta Valid/S", check_users_meta_valid_s);
	suite_check_testcase(s, "USERS", "Meta Invalid/S", check_users_meta_invalid_s);

	return s;
}
