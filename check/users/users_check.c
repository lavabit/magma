
/**
 * @file /magma.check/user/user_check.c
 *
 * @brief Checks the code used to handle user data.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma_check.h"

START_TEST (check_users_credentials_valid_s) {

	int_t state;
	stringer_t *errmsg = NULL;
	meta_user_t *user_check_data = NULL;
	credential_t *user_check_cred = NULL;

	// Valid Login Attempts
	log_unit("%-64.64s", "USERS / CREDENTIAL / VALID / SINGLE THREADED:");

		// Try the simplest use case. Login as the actual username.
		if (!(user_check_cred = credential_alloc_auth(CONSTANT("magma"), CONSTANT("test")))) {
			errmsg = st_import("Credential creation failed. { user = magma / password = test }", 63);
		}
		else if ((state = meta_get(user_check_cred, META_PROT_GENERIC,
			META_GET_MESSAGES | META_GET_FOLDERS | META_GET_CONTACTS, &(user_check_data))) != 1) {
			errmsg = st_aprint("Authentication should have succeeded. { user = magma / password = test / meta_get = %i }", state);
		}

		if (user_check_data) {
			meta_remove(user_check_cred->auth.username, META_PROT_GENERIC);
			if (meta_user_prune(user_check_cred->auth.username) < 1) {
				errmsg = st_import("An error occurred while trying to prune the user data from the object cache. { user = magma / password = test }", 112);
			}
			user_check_data = NULL;
		}

		if (user_check_cred) {
			credential_free(user_check_cred);
			user_check_cred = NULL;
		}

		// See if the domain name portion is correctly stripped off.
		if (!errmsg && !(user_check_cred = credential_alloc_auth(CONSTANT("magma@lavabit.com"), CONSTANT("test")))) {
			errmsg = st_import("Credential creation failed. { user = magma@lavabit.com / password = test }", 75);
		}
		else if (!errmsg && (state = meta_get(user_check_cred, META_PROT_GENERIC,
			META_GET_MESSAGES | META_GET_FOLDERS | META_GET_CONTACTS, &(user_check_data))) != 1) {
			errmsg = st_aprint("Authentication should have succeeded. { user = magma@lavabit.com / password = test / meta_get = %i }", state);
		}

		if (user_check_data) {
			meta_remove(user_check_cred, META_PROT_GENERIC);
			if (meta_user_prune(user_check_cred) < 1) {
				errmsg = st_import("An error occurred while trying to prune the user data from the object cache. { user = magma@lavabit.com / password = test }", 124);
			}
			user_check_data = NULL;
		}

		if (user_check_cred) {
			credential_free(user_check_cred);
			user_check_cred = NULL;
		}

		// Try including a label in the mailbox portion of the address.
		if (!errmsg && !(user_check_cred = credential_alloc_auth(CONSTANT("magma+label@lavabit.com"), CONSTANT("test")))) {
			errmsg = st_import("Credential creation failed. { user = magma+label@lavabit.com / password = test }", 81);
		}
		else if (!errmsg && (state = meta_get(user_check_cred, META_PROT_GENERIC,
			META_GET_MESSAGES | META_GET_FOLDERS | META_GET_CONTACTS, &(user_check_data))) != 1) {
			errmsg = st_aprint("Authentication should have succeeded. { user = magma+label@lavabit.com / password = test / meta_get = %i }", state);
		}

		if (user_check_data) {
			meta_remove(user_check_cred, META_PROT_GENERIC);
			if (meta_user_prune(user_check_cred) < 1) {
				errmsg = st_import("An error occurred while trying to prune the user data from the object cache. { user = magma+label@lavabit.com / password = test }", 130);
			}
			user_check_data = NULL;
		}

		if (user_check_cred) {
			credential_free(user_check_cred);
			user_check_cred = NULL;
		}

	log_unit("%10.10s\n", (!status() ? "SKIPPED" : !errmsg ? "PASSED" : "FAILED"));
	fail_unless(!errmsg, st_char_get(errmsg));

} END_TEST

START_TEST (check_users_credentials_invalid_s) {

	int_t state;
	stringer_t *errmsg = NULL;
	meta_user_t *user_check_data = NULL;
	credential_t *user_check_cred = NULL;
	stringer_t *username = NULL, *password = NULL;

	// Invalid Login Attempts
	log_unit("%-64.64s", "USERS / CREDENTIAL / INVALID / SINGLE THREADED:");

	// Try passing in various combinations of NULL.
	if ((user_check_cred = credential_alloc_auth(NULL, CONSTANT("test")))) {
		errmsg = st_import("Credential creation should have failed but succeeded instead. { user = NULL / password = test }", 96);
		credential_free(user_check_cred);
	}

	if (!errmsg && (user_check_cred = credential_alloc_auth(CONSTANT("magma"), NULL))) {
		errmsg = st_import("Credential creation should have failed but succeeded instead. { user = magma / password = NULL }", 97);
		credential_free(user_check_cred);
	}

	if (!errmsg && (user_check_cred = credential_alloc_auth(NULL, NULL))) {
		errmsg = st_import("Credential creation should have failed but succeeded instead. { user = NULL / password = NULL }", 96);
		credential_free(user_check_cred);
	}

	// Try the simplest use case. Login as the correct username but supply an incorrect password.
	if (!errmsg && !(user_check_cred = credential_alloc_auth(CONSTANT("magma"), CONSTANT("password")))) {
		errmsg = st_import("Credential creation failed. Authentication was not attempted. { user = magma / password = password }", 101);
	}
	else if (!errmsg && (state = meta_get(user_check_cred, META_PROT_GENERIC, META_GET_MESSAGES | META_GET_FOLDERS | META_GET_CONTACTS, &(user_check_data))) != 0) {
		errmsg = st_aprint("Authentication should have failed but succeeded instead. { user = magma / password = password / meta_get = %i }", state);
	}

	if (user_check_data) {
		meta_remove(user_check_cred->auth.username, META_PROT_GENERIC);
		user_check_data = NULL;
	}

	if (user_check_cred) {
		credential_free(user_check_cred);
		user_check_cred = NULL;
	}

	// See if the domain name portion is correctly stripped off.
	if (!errmsg && !(user_check_cred = credential_alloc_auth(CONSTANT("magma@lavabit.com"), CONSTANT("password")))) {
		errmsg = st_import("Credential creation failed. Authentication was not attempted. { user = magma@lavabit.com / password = password }", 113);
	}
	else if (!errmsg && (state = meta_get(user_check_cred, META_PROT_GENERIC, META_GET_MESSAGES | META_GET_FOLDERS | META_GET_CONTACTS, &(user_check_data))) != 0) {
		errmsg = st_aprint("Authentication should have failed but succeeded instead. { user = magma@lavabit.com / password = password / meta_get = %i }", state);
	}

	if (user_check_data) {
		meta_remove(user_check_cred->auth.username, META_PROT_GENERIC);
		user_check_data = NULL;
	}

	if (user_check_cred) {
		credential_free(user_check_cred);
		user_check_cred = NULL;
	}

	// Try including a label in the mailbox portion of the address.
	if (!errmsg && !(user_check_cred = credential_alloc_auth(CONSTANT("magma+label@lavabit.com"), CONSTANT("password")))) {
		errmsg = st_import("Credential creation failed. Authentication was not attempted. { user = magma+label@lavabit.com / password = password }", 119);
	}
	else if (!errmsg && (state = meta_get(user_check_cred, META_PROT_GENERIC, META_GET_MESSAGES | META_GET_FOLDERS | META_GET_CONTACTS, &(user_check_data))) != 0) {
		errmsg = st_aprint("Authentication should have failed but succeeded instead. { user = magma+label@lavabit.com / password = password / meta_get = %i }", state);
	}

	if (user_check_data) {
		meta_remove(user_check_cred->auth.username, META_PROT_GENERIC);
		user_check_data = NULL;
	}

	if (user_check_cred) {
		credential_free(user_check_cred);
		user_check_cred = NULL;
	}

	// Use an alternate email address and see if the credentials object is able to use the mailboxes table to map this email address to the correct username.
	// For
	if (!errmsg && !(user_check_cred = credential_alloc_auth(CONSTANT("magma@nerdshack.com"), CONSTANT("test")))) {
		errmsg = st_import("Credential creation failed. Authentication was not attempted. { user = magma@nerdshack.com / password = test }", 115);
	}
	else if (!errmsg && (state = meta_get(user_check_cred, META_PROT_GENERIC, META_GET_MESSAGES | META_GET_FOLDERS | META_GET_CONTACTS, &(user_check_data))) != 0) {
		errmsg = st_aprint("Authentication should have failed but succeeded instead. At least until an email address can be mapped to a username. { user = magma@nerdshack.com / password = test / meta_get = %i }", state);
	}

	if (user_check_data) {
		meta_remove(user_check_cred->auth.username, META_PROT_GENERIC);
		user_check_data = NULL;
	}

	if (user_check_cred) {
		credential_free(user_check_cred);
		user_check_cred = NULL;
	}

	// Attempt a credentials creation using a series of randomly generated, but valid usernames.
	for (uint_t i = 0; !errmsg && i < OBJECT_CHECK_ITERATIONS; i++) {
		if (!(username = rand_choices("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz_", (rand_get_uint8() % 128) + 1))) {
			errmsg = st_import("An error occurred while trying to generate a random username. { user = NULL }", 78);
		}
		else if (!(user_check_cred = credential_alloc_auth(username, CONSTANT("test")))) {
			errmsg = st_aprint("Credential creation failed. Authentication was not attempted. { user = %.*s / password = test }",	st_length_int(username), st_char_get(username));
		}
		else if ((state = meta_get(user_check_cred, META_PROT_GENERIC, META_GET_MESSAGES | META_GET_FOLDERS | META_GET_CONTACTS, &(user_check_data))) != 0) {
			errmsg = st_aprint("Authentication should have failed but succeeded instead. { user = RANDOM BINARY DATA / password = RANDOM BINARY DATA / meta_get = %i }", state);
		}

		if (user_check_data) {
			meta_remove(user_check_cred->auth.username, META_PROT_GENERIC);
			user_check_data = NULL;
		}

		if (user_check_cred) {
			credential_free(user_check_cred);
			user_check_cred = NULL;
		}

		st_cleanup(username);
	}

	// Attempt a credentials creation using a series of randomly generated binary usernames.
	for (uint_t i = 0; !errmsg && i < OBJECT_CHECK_ITERATIONS; i++) {

		username = st_alloc((rand_get_uint8() % 128) + 1);

		if (!username || !rand_write(username)) {
			errmsg = st_import("An error occurred while trying to generate a random username. { user = NULL }", 78);
		}
		else if ((user_check_cred = credential_alloc_auth(username, CONSTANT("test"))) && (state = meta_get(user_check_cred,
				META_PROT_GENERIC, META_GET_MESSAGES | META_GET_FOLDERS | META_GET_CONTACTS, &(user_check_data))) != 0) {
			errmsg = st_aprint("Authentication should have failed but succeeded instead. { user = RANDOM BINARY DATA / password = test / meta_get = %i }", state);
		}

		if (user_check_data) {
			meta_remove(user_check_cred->auth.username, META_PROT_GENERIC);
			user_check_data = NULL;
		}

		if (user_check_cred) {
			credential_free(user_check_cred);
			user_check_cred = NULL;
		}

		st_cleanup(username);
	}

	// Finally, try generating credentials using randomly generated binary data for the username and the password.
	for (uint_t i = 0; !errmsg && i < OBJECT_CHECK_ITERATIONS; i++) {

		username = st_alloc((rand_get_uint8() % 128) + 1);
		password = st_alloc((rand_get_uint8() % 255) + 1);

		if (!username || !rand_write(username) || !password || !rand_write(password)) {
			errmsg = st_import("Random username and password generation failed. { user = NULL / password = NULL }", 82);
		}
		else if ((user_check_cred = credential_alloc_auth(username, password)) && (state = meta_get(user_check_cred, META_PROT_GENERIC, META_GET_MESSAGES | META_GET_FOLDERS | META_GET_CONTACTS, &(user_check_data))) != 0) {
			errmsg = st_aprint("Authentication should have failed but succeeded instead. { user = RANDOM BINARY DATA / password = RANDOM BINARY DATA / meta_get = %i }", state);
		}

		if (user_check_data) {
			meta_remove(user_check_cred->auth.username, META_PROT_GENERIC);
			user_check_data = NULL;
		}

		if (user_check_cred) {
			credential_free(user_check_cred);
			user_check_cred = NULL;
		}

		st_cleanup(username);
		st_cleanup(password);
	}

	log_unit("%10.10s\n", (!status() ? "SKIPPED" : !errmsg ? "PASSED" : "FAILED"));
	fail_unless(!errmsg, st_char_get(errmsg));
	st_cleanup(errmsg);

} END_TEST

START_TEST (check_users_inbox_s) {

	char *errmsg = NULL;

	log_unit("%-64.64s", "USERS / INBOX / SINGLE THREADED:");
	errmsg = "User inbox test incomplete.";
	log_unit("%10.10s\n", "SKIPPED");

	//log_unit("%10.10s\n", (!status() ? "SKIPPED" : !errmsg ? "PASSED" : "FAILED"));
	//fail_unless(!errmsg, errmsg);
} END_TEST

START_TEST (check_users_message_s) {

	char *errmsg = NULL;

	log_unit("%-64.64s", "USERS / MESSAGE / SINGLE THREADED:");
	errmsg = "User message test incomplete.";
	log_unit("%10.10s\n", "SKIPPED");

	//log_unit("%10.10s\n", (!status() ? "SKIPPED" : !errmsg ? "PASSED" : "FAILED"));
	//fail_unless(!errmsg, errmsg);
} END_TEST

Suite * suite_check_users(void) {

	TCase *tc;
	Suite *s = suite_create("\tUsers");

	testcase(s, tc, "Auth Valid/S", check_users_credentials_valid_s);
	testcase(s, tc, "Auth Invalid/S", check_users_credentials_invalid_s);
	testcase(s, tc, "Inbox/S", check_users_inbox_s);
	testcase(s, tc, "Message/S", check_users_message_s);

	return s;
}
