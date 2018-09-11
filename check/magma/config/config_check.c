/**
 * @file /check/magma/config/config_check.c
 *
 * @brief Check the magma config logic and related functions.
 */

#include "magma_check.h"

START_TEST (check_config_server_get_by_protocol_s) {

	log_disable();
	bool_t outcome = true;
	server_t *server = NULL;
	stringer_t *errmsg = NULL;
	uint32_t protocols[] = { HTTP, POP, IMAP, SMTP };

	// First we test the function with proper input.
	for (size_t i = 0; status() && outcome && i < sizeof(protocols)/sizeof(uint32_t); i++) {

		if (!(server = servers_get_by_protocol(protocols[i], false)) || server->network.type != TCP_PORT ||
				server->protocol != protocols[i]) {
			outcome = false;
			errmsg = NULLER("Failed to return a pointer to the correct server (TCP).");
		}

		else if (!(server = servers_get_by_protocol(protocols[i], true)) || server->network.type != TLS_PORT ||
				server->protocol != protocols[i]) {
			outcome = false;
			errmsg = NULLER("Failed to return a pointer to the correct server (TLS).");
		}
	}

	// Next we test the function with improper input.
	if ((server = servers_get_by_protocol(-1, false)) || (server = servers_get_by_protocol(-1, true))) {
		outcome = false;
		errmsg = NULLER("Failed to return NULL when given improper input for the protocol type.");
	}

	log_test("CONFIG / SERVER / PROTOCOL / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_config_minimum_password_length_s) {

	log_disable();
	connection_t con;
	uint16_t plan = 1;
	auth_t *auth = NULL;
	uint64_t usernum = 0;
	bool_t outcome = true;
	int64_t transaction = -1;
	uint32_t original_length_value = magma.secure.minimum_password_length;
	stringer_t *errmsg = MANAGEDBUF(1024), *username = MANAGEDBUF(64), *suffix = MANAGEDBUF(64);

	// Set the minimum length to 8, but attempt a value with only 5 characters.
	if (status()) {

		magma.secure.minimum_password_length = 8;

		if (auth_login(PLACER("magma", 5), PLACER("short", 5), &auth) != 1) {
			st_sprint(errmsg, "A password of insufficient length did not return the proper error code.");
			outcome = false;
		}

		if (auth) {
			auth_free(auth);
			auth = NULL;
		}

	}

	// Set the minimum length to 8, and attempt a value with precisely 8 characters.
	if (status() && outcome) {

		magma.secure.minimum_password_length = 8;

		if (auth_login(PLACER("magma", 5), PLACER("password", 8), &auth) != 0) {
			st_sprint(errmsg, "A password of 8 characters did not succeed with a minimum length of 8.");
			outcome = false;
		}

		if (auth) {
			auth_free(auth);
			auth = NULL;
		}

	}

	// Increase the minimum length to 9, and attempt to use the same, valid password value, with only 8 characters.
	if (status() && outcome) {

		magma.secure.minimum_password_length = 9;

		if (auth_login(PLACER("magma", 5), PLACER("password", 8), &auth) != 1) {
			st_sprint(errmsg, "The correct password, with 8 characters, was not rejected when the minimum length is set to 9.");
			outcome = false;
		}

		if (auth) {
			auth_free(auth);
			auth = NULL;
		}

	}

	// Attempt a password which is only 4 unicode characters, less than the minimum of 5, but is 12 bytes, which exceeds
	// the minimum.
	if (status() && outcome) {

		magma.secure.minimum_password_length = 5;

		if (auth_login(PLACER("magma", 5), hex_decode_st(NULLER("E5B890E58FB7E5AF86E7A081"), MANAGEDBUF(64)), &auth) != 1) {
			st_sprint(errmsg, "A unicode password with only 4 characters was accepted, despite a minimum length of 5.");
			outcome = false;
		}

		if (auth) {
			auth_free(auth);
			auth = NULL;
		}

	}

	// Create a user with this same password, doubled up, to make 8 unicode characters, and ensure it can authenticate.
	if (status() && outcome) {

		magma.secure.minimum_password_length = 8;

		// Pass in a blank connection structure. This will be used to store the registration IP address.
		mm_wipe(&con, sizeof(connection_t));


		// Randomly select one of the available plans. Valid values are 1 through 6.
		plan = (rand_get_uint16() % 5) + 1;

		// Generate a random username.
		if (st_empty(rand_choices("0123456789", 20, suffix)) || st_sprint(username, "check_user_%.*s", st_length_int(suffix), st_char_get(suffix)) != 31) {
			st_sprint(errmsg, "An internal error occurred. Unable to generate a random username for the password length check registration.");
			outcome = false;
		}

		// Start the transaction.
		else if ((transaction = tran_start()) == -1) {
			st_sprint(errmsg, "An internal error occurred. Unable to start the transaction.");
			outcome = false;
		}

		// This first database insert attempt should fail, because the password isn't long enough.
		else if (register_data_insert_user(&con, plan, username, hex_decode_st(NULLER("E5B890E58FB7E5AF86E7A081"), MANAGEDBUF(64)), transaction, &usernum) != 1) {
			st_sprint(errmsg, "The user registration function accepted a password with 4 unicode characters, despite a minimum password length setting of 8.");
			tran_rollback(transaction);
			outcome = false;
		}

		// Otherwise try again with a properly long password.
		else if (register_data_insert_user(&con, plan, username, hex_decode_st(NULLER("E5B890E58FB7E5AF86E7A081E5B890E58FB7E5AF86E7A081"), MANAGEDBUF(64)), transaction, &usernum) != 0) {
			st_sprint(errmsg, "The user registration function failed to accept a password, even though it was long enough.");
			tran_rollback(transaction);
			outcome = false;
		}

		// Were finally done.
		else {
			tran_commit(transaction);
		}

		// If the registration checks above don't fail, we make sure we can authenticate with those values here.
		if (outcome && auth_login(username, hex_decode_st(NULLER("E5B890E58FB7E5AF86E7A081E5B890E58FB7E5AF86E7A081"), MANAGEDBUF(64)), &auth) != 0) {
			st_sprint(errmsg, "A unicode password with 8 characters was rejected, despite a minimum length of 8.");
			outcome = false;
		}

		if (auth) {
			auth_free(auth);
			auth = NULL;
		}

	}

	magma.secure.minimum_password_length = original_length_value;

	log_test("CONFIG / SECURITY / PASSWORD LENGTH / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

Suite * suite_check_config(void) {

	Suite *s = suite_create("\tConfig");

	suite_check_testcase(s, "CONFIG", "Config / Protocol /S", check_config_server_get_by_protocol_s);
	suite_check_testcase(s, "CONFIG", "Config / Password Length /S", check_config_minimum_password_length_s);

	return s;
}
