
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
	int64_t transaction = -1;
	stringer_t *errmsg = MANAGEDBUF(128), *username = NULL, *password = NULL;

	// If the check process hasn't been aborted, register new user account using a randomly generated userid/password.
	if (status()) {

		// Pass in a blank connection structure. This will be used to store the registration IP address.
		mm_wipe(&con, sizeof(connection_t));

		// Randomly select one of the available plans. Valid values are 1 through 4.
		plan = (rand_get_uint16() % 3) + 1;

		// Generate a random string of numbers as the password and then append the string of numbers to the username
		// pattern check_user_XYZ to create a username that should always be unique.
		if (!(password = rand_choices("0123456789", 20, NULL)) || !(username = st_aprint("check_user_%.*s", st_length_int(password), st_char_get(password)))) {
			st_sprint(errmsg, "An internal error occurred. Unable to generate a random username and password for registration.");
		}

		// Start the transaction.
		else if ((transaction = tran_start()) == -1) {
			st_sprint(errmsg, "An internal error occurred. Unable to start the transaction.");
		}

		// Database insert.
		else if (!register_data_insert_user(&con, plan, username, password, transaction, &usernum)) {
			st_sprint(errmsg, "User registration failed!.");
			tran_rollback(transaction);
		}

		// Were finally done.
		else {
			tran_commit(transaction);
		}

		st_cleanup(username, password);
	}

	log_test("USERS / REGISTER / SINGLE THREADED:", errmsg);
	if (st_populated(errmsg)) ck_abort_msg(st_char_get(errmsg));
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

		else if ((meta_get(auth->usernum, auth->username, auth->keys.master, auth->tokens.verification,
			META_PROTOCOL_POP, META_GET_MESSAGES | META_GET_KEYS, &(pop)))) {
			st_sprint(errmsg, "User meta login check failed. Get user metadata failure. { username =  %.*s / password = %.*s }",
				st_length_int(usernames[i]), st_char_get(usernames[i]), st_length_int(passwords[i]), st_char_get(passwords[i]));
			result = false;
		}

		else if ((meta_get(auth->usernum, auth->username, auth->keys.master, auth->tokens.verification,
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
	stringer_t *usernames[] = { NULLER("magma") }, *passwords[] = { NULLER("password") };

	for (int_t i = 0; i < (sizeof(usernames)/sizeof(stringer_t *)) && result && status(); i++) {

		if (auth_login(usernames[i], passwords[i], &auth)) {
			 st_sprint(errmsg, "User meta login check failed. Authentication failure. { username =  %.*s / password = %.*s }",
			 	 st_length_int(usernames[i]), st_char_get(usernames[i]), st_length_int(passwords[i]), st_char_get(passwords[i]));
			 result = false;
		}

		else if (!(meta_get(auth->usernum, auth->username, st_xor(auth->keys.master, auth->tokens.verification, MANAGEDBUF(64)),
			auth->tokens.verification, META_PROTOCOL_POP, META_GET_MESSAGES | META_GET_KEYS, &(user)))) {
			st_sprint(errmsg, "User meta login check failed. Get user metadata failure. { username =  %.*s / password = %.*s }",
				st_length_int(usernames[i]), st_char_get(usernames[i]), st_length_int(passwords[i]), st_char_get(passwords[i]));
			result = false;
		}

		else if (!(meta_get(auth->usernum, auth->username, auth->keys.master, st_xor(auth->keys.master, auth->tokens.verification,
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

	TCase *tc;
	Suite *s = suite_create("\tUsers");

	testcase(s, tc, "Auth Usernames/S", check_users_auth_username_s);
	testcase(s, tc, "Auth Addresses/S", check_users_auth_address_s);
	testcase(s, tc, "Auth Legacy/S", check_users_auth_legacy_s);
	testcase(s, tc, "Auth Stacie/S", check_users_auth_stacie_s);
	testcase(s, tc, "Auth Challenge/S", check_users_auth_challenge_s);
	testcase(s, tc, "Auth Response/S", check_users_auth_response_s);
	testcase(s, tc, "Auth Login/S", check_users_auth_login_s);

	testcase(s, tc, "Register/S", check_users_register_s);

	testcase(s, tc, "Meta Valid/S", check_users_meta_valid_s);
	testcase(s, tc, "Meta Invalid/S", check_users_meta_invalid_s);

	return s;
}
