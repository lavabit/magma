/**
 * @file /check/magma/camel/camel_check.c
 *
 * @brief Camelface test functions.
 */

#include "magma_check.h"

START_TEST (check_camel_auth_s) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (status() && !check_camel_auth_sthread(false, errmsg)) outcome = false;
	else if (status() && outcome && !check_camel_auth_sthread(true, errmsg)) outcome = false;

	log_test("HTTP / NETWORK / CAMEL / LOGIN / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_camel_register_s) {

	log_disable();
	json_error_t err;
	uint32_t length = 0;
	bool_t outcome = true;
	client_t *client = NULL;
	json_t *json_objs[1] = { NULL };
	const chr_t *json_values[1] = { NULL };
	stringer_t *post = NULL, *json = NULL, *username = NULL, *password = NULL, *errmsg = MANAGEDBUF(1024);
	chr_t *message = "POST /json HTTP/1.1\r\nHost: localhost:%u\r\nAccept: */*\r\n" \
		"Content-Length: %u\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\n{\"id\":%u,\"method\":\"register\"," \
		"\"params\":{\"username\":\"%.*s\",\"password\":\"%.*s\",\"password_verification\":\"%.*s\"}}\r\n\r\n";

	// HTTP system user registration attempt.
	if (status() && !(client = check_camel_connect(false))) {
		st_sprint(errmsg, "Failed to connect back to the HTTP server for a registration request.");
		outcome = false;
	}
	else if (status()) {

		// Generate a random username, using the pattern camel_user_X, which ensures the username is always unique.
		if (!(password = rand_choices("0123456789", 8, MANAGEDBUF(8))) || !(username = st_quick(MANAGEDBUF(64), "camel_user_%.*s",
			st_length_int(password), st_char_get(password)))) {
			st_sprint(errmsg, "An internal error occurred. Unable to generate a random username for the camelface registration test.");
			outcome = false;
		}

		// Create a random password and confirm the length of both values.
		else if (!(password = rand_choices("0123456789", 20, MANAGEDBUF(20)))) {
			st_sprint(errmsg, "An internal error occurred. Unable to generate a random password for the camelface registration test.");
			outcome = false;
		}

		// Validate the random values.
		else if (st_length_get(password) != 20 || st_length_get(username) != 19) {
			st_sprint(errmsg, "The random username/password values failed validation.");
			outcome = false;
		}

		// Construct the registration request.
		else {
			post = st_quick(MANAGEDBUF(1024), message, client->port, 157, 1, st_length_int(username), st_char_get(username),
				st_length_int(password), st_char_get(password), st_length_int(password), st_char_get(password));
		}

		// Send the request.
		if (outcome && client_write(client, post) != st_length_get(post)) {
			st_sprint(errmsg, "Failed to send the user registration request.");
			outcome = false;
		}

		// Read the response.
		else if (outcome && ((length = check_http_content_length_get(client)) <= 0 || !(json = check_camel_json_read(client, length)))) {
			st_sprint(errmsg, "Failed to read the user registration response.");
			outcome = false;
		}

		// Parse the returned JSON.
		else if (outcome && (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err))	||
			json_unpack_d(json_objs[0], "{s:{s:s}}", "result", "register", &json_values[0]) != 0)) {
			st_sprint(errmsg, "Failed to parse the registration response. { command = %.*s, json = %.*s, value = %s}",
				st_length_int(post), st_char_get(post), st_length_int(json), st_char_get(json), json_values[0]);
			if (json_objs[0]) json_decref_d(json_objs[0]);
			outcome = false;
		}

		// Otherwise we need to check the response value.
		else if (outcome && st_cmp_cs_eq(PLACER((chr_t * )json_values[0], ns_length_get(json_values[0])), PLACER("success", 7))) {
			st_sprint(errmsg, "The camelface registration attempt returned an error. { command = %.*s, json = %.*s }",
				st_length_int(post), st_char_get(post), st_length_int(json), st_char_get(json));
			json_decref_d(json_objs[0]);
			outcome = false;
		}

		// If we make it this far we need to cleanup the JSON object.
		else if (outcome) {
			json_decref_d(json_objs[0]);
		}

		// Cleanup.
		client_close(client);
		st_cleanup(json);
		json = NULL;

		// Confirm the user was created.
		if (outcome && sql_num_rows(st_quick(MANAGEDBUF(1024), "SELECT usernum, userid FROM Users WHERE userid = '%.*s';",
			st_length_int(username), st_char_get(username))) != 1) {
			st_sprint(errmsg, "Verification of the user table entry after registering a fully qualified user failed.");
			outcome = false;
		}
		else if (outcome && sql_num_rows(st_quick(MANAGEDBUF(1024), "SELECT usernum, address FROM Mailboxes WHERE address =  '%.*s@%.*s';",
			st_length_int(username), st_char_get(username), st_length_int(magma.system.domain), st_char_get(magma.system.domain))) != 1) {
			st_sprint(errmsg, "Verification of the mailbox table entry after registering a fully qualified user failed.");
			outcome = false;
		}

	}

	mark_point();

	// HTTPS system user registration attempt.
	if (status() && outcome && !(client = check_camel_connect(true))) {
		st_sprint(errmsg, "Failed to connect back to the HTTP server for a registration request.");
		outcome = false;
	}
	else if (status() && outcome) {

		// Generate a random username, using the pattern camel_user_X, which ensures the username is always unique.
		if (!(password = rand_choices("0123456789", 8, MANAGEDBUF(8))) || !(username = st_quick(MANAGEDBUF(64), "camel_user_%.*s",
			st_length_int(password), st_char_get(password)))) {
			st_sprint(errmsg,	"An internal error occurred. Unable to generate a random username for the camelface registration test.");
			outcome = false;
		}

		// Create a random password and confirm the length of both values.
		else if (!(password = rand_choices("0123456789", 20, MANAGEDBUF(20)))) {
			st_sprint(errmsg, "An internal error occurred. Unable to generate a random password for the camelface registration test.");
			outcome = false;
		}

		// Validate the random values.
		else if (st_length_get(password) != 20 || st_length_get(username) != 19) {
			st_sprint(errmsg, "The random username/password values failed validation.");
			outcome = false;
		}

		// Construct the registration request.
		else {
			post = st_quick(MANAGEDBUF(1024), message, client->port, 157, 1, st_length_int(username), st_char_get(username),
				st_length_int(password), st_char_get(password), st_length_int(password), st_char_get(password));
		}

		// Send the request.
		if (outcome && client_write(client, post) != st_length_get(post)) {
			st_sprint(errmsg, "Failed to send the user registration request.");
			outcome = false;
		}

		// Read the response.
		else if (outcome && ((length = check_http_content_length_get(client)) <= 0 || !(json = check_camel_json_read(client, length)))) {
			st_sprint(errmsg, "Failed to read the user registration response.");
			outcome = false;
		}

		// Parse the returned JSON.
		else if (outcome && (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err))	||
			json_unpack_d(json_objs[0], "{s:{s:s}}", "result", "register", &json_values[0]) != 0)) {
			st_sprint(errmsg, "Failed to parse the registration response. { command = %.*s, json = %.*s, value = %s}",
				st_length_int(post), st_char_get(post), st_length_int(json), st_char_get(json), json_values[0]);
			if (json_objs[0]) json_decref_d(json_objs[0]);
			outcome = false;
		}

		// Otherwise we need to check the response value.
		else if (outcome && st_cmp_cs_eq(PLACER((chr_t * )json_values[0], ns_length_get(json_values[0])), PLACER("success", 7))) {
			st_sprint(errmsg, "The camelface registration attempt returned an error. { command = %.*s, json = %.*s }",
				st_length_int(post), st_char_get(post), st_length_int(json), st_char_get(json));
			json_decref_d(json_objs[0]);
			outcome = false;
		}

		// If we make it this far we need to cleanup the JSON object.
		else if (outcome) {
			json_decref_d(json_objs[0]);
		}

		// Cleanup.
		client_close(client);
		st_cleanup(json);
		json = NULL;

		// Confirm the user was created.
		if (outcome && sql_num_rows(st_quick(MANAGEDBUF(1024), "SELECT usernum, userid FROM Users WHERE userid = '%.*s';",
			st_length_int(username), st_char_get(username))) != 1) {
			st_sprint(errmsg, "Verification of the user table entry after registering a fully qualified user failed.");
			outcome = false;
		}
		else if (outcome && sql_num_rows(st_quick(MANAGEDBUF(1024), "SELECT usernum, address FROM Mailboxes WHERE address =  '%.*s@%.*s';",
			st_length_int(username), st_char_get(username), st_length_int(magma.system.domain), st_char_get(magma.system.domain))) != 1) {
			st_sprint(errmsg, "Verification of the mailbox table entry after registering a fully qualified user failed.");
			outcome = false;
		}

	}

	mark_point();

	// HTTP corporate user registration attempt.
	if (status() && !(client = check_camel_connect(false))) {
		st_sprint(errmsg, "Failed to connect back to the HTTP server for a registration request.");
		outcome = false;
	}
	else if (status() && outcome) {

		// Generate a random username, using the pattern camel_user_X, which ensures the username is always unique.
		if (!(password = rand_choices("0123456789", 8, MANAGEDBUF(8))) || !(username = st_quick(MANAGEDBUF(64), "camel_user_%.*s@example.com",
			st_length_int(password), st_char_get(password)))) {
			st_sprint(errmsg,	"An internal error occurred. Unable to generate a random username for the camelface registration test.");
			outcome = false;
		}

		// Create a random password and confirm the length of both values.
		else if (!(password = rand_choices("0123456789", 20, MANAGEDBUF(20)))) {
			st_sprint(errmsg, "An internal error occurred. Unable to generate a random password for the camelface registration test.");
			outcome = false;
		}

		// Validate the random values.
		else if (st_length_get(password) != 20 || st_length_get(username) != 31) {
			st_sprint(errmsg, "The random username/password values failed validation.");
			outcome = false;
		}

		// Construct the registration request.
		else {
			post = st_quick(MANAGEDBUF(1024), message, client->port, 169, 1, st_length_int(username), st_char_get(username),
				st_length_int(password), st_char_get(password), st_length_int(password), st_char_get(password));
		}

		// Send the request.
		if (outcome && client_write(client, post) != st_length_get(post)) {
			st_sprint(errmsg, "Failed to send the user registration request.");
			outcome = false;
		}

		// Read the response.
		else if (outcome && ((length = check_http_content_length_get(client)) <= 0 || !(json = check_camel_json_read(client, length)))) {
			st_sprint(errmsg, "Failed to read the user registration response.");
			outcome = false;
		}

		// Parse the returned JSON.
		else if (outcome && (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err)) ||
			json_unpack_d(json_objs[0], "{s:{s:s}}", "result", "register", &json_values[0]) != 0)) {
			st_sprint(errmsg, "Failed to parse the registration response. { command = %.*s, json = %.*s, value = %s}",
				st_length_int(post), st_char_get(post), st_length_int(json), st_char_get(json), json_values[0]);
			if (json_objs[0]) json_decref_d(json_objs[0]);
			outcome = false;
		}

		// Otherwise we need to check the response value.
		else if (outcome && st_cmp_cs_eq(PLACER((chr_t * )json_values[0], ns_length_get(json_values[0])), PLACER("success", 7))) {
			st_sprint(errmsg, "The camelface registration attempt returned an error. { command = %.*s, json = %.*s }",
				st_length_int(post), st_char_get(post), st_length_int(json), st_char_get(json));
			json_decref_d(json_objs[0]);
			outcome = false;
		}

		// If we make it this far we need to cleanup the JSON object.
		else if (outcome) {
			json_decref_d(json_objs[0]);
		}

		// Cleanup.
		client_close(client);
		st_cleanup(json);
		json = NULL;

		// Confirm the user was created.
		if (outcome && sql_num_rows(st_quick(MANAGEDBUF(1024), "SELECT usernum, userid FROM Users WHERE userid = '%.*s';",
			st_length_int(username), st_char_get(username))) != 1) {
			st_sprint(errmsg, "Verification of the user table entry after registering a fully qualified user failed.");
			outcome = false;
		}
		else if (outcome && sql_num_rows(st_quick(MANAGEDBUF(1024), "SELECT usernum, address FROM Mailboxes WHERE address =  '%.*s';",
			st_length_int(username), st_char_get(username))) != 1) {
			st_sprint(errmsg, "Verification of the mailbox table entry after registering a fully qualified user failed.");
			outcome = false;
		}

	}

	mark_point();

	// HTTPS system user registration attempt.
	if (status() && outcome && !(client = check_camel_connect(true))) {
		st_sprint(errmsg, "Failed to connect back to the HTTP server for a registration request.");
		outcome = false;
	}
	else if (status() && outcome) {

		// Generate a random username, using the pattern camel_user_X, which ensures the username is always unique.
		if (!(password = rand_choices("0123456789", 8, MANAGEDBUF(8))) || !(username = st_quick(MANAGEDBUF(64), "camel_user_%.*s@example.com",
			st_length_int(password), st_char_get(password)))) {
			st_sprint(errmsg,	"An internal error occurred. Unable to generate a random username for the camelface registration test.");
			outcome = false;
		}

		// Create a random password and confirm the length of both values.
		else if (!(password = rand_choices("0123456789", 20, MANAGEDBUF(20)))) {
			st_sprint(errmsg, "An internal error occurred. Unable to generate a random password for the camelface registration test.");
			outcome = false;
		}

		// Validate the random values.
		else if (st_length_get(password) != 20 || st_length_get(username) != 31) {
			st_sprint(errmsg, "The random username/password values failed validation.");
			outcome = false;
		}

		// Construct the registration request.
		else {
			post = st_quick(MANAGEDBUF(1024), message, client->port, 169, 1, st_length_int(username), st_char_get(username),
				st_length_int(password), st_char_get(password), st_length_int(password), st_char_get(password));
		}

		// Send the request.
		if (outcome && client_write(client, post) != st_length_get(post)) {
			st_sprint(errmsg, "Failed to send the user registration request.");
			outcome = false;
		}

		// Read the response.
		else if (outcome && ((length = check_http_content_length_get(client)) <= 0 || !(json = check_camel_json_read(client, length)))) {
			st_sprint(errmsg, "Failed to read the user registration response.");
			outcome = false;
		}

		// Parse the returned JSON.
		else if (outcome && (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err))	||
			json_unpack_d(json_objs[0], "{s:{s:s}}", "result", "register", &json_values[0]) != 0)) {
			st_sprint(errmsg, "Failed to parse the registration response. { command = %.*s, json = %.*s, value = %s}",
				st_length_int(post), st_char_get(post), st_length_int(json), st_char_get(json), json_values[0]);
			if (json_objs[0]) json_decref_d(json_objs[0]);
			outcome = false;
		}

		// Otherwise we need to check the response value.
		else if (outcome && st_cmp_cs_eq(PLACER((chr_t * )json_values[0], ns_length_get(json_values[0])), PLACER("success", 7))) {
			st_sprint(errmsg, "The camelface registration attempt returned an error. { command = %.*s, json = %.*s }",
				st_length_int(post), st_char_get(post), st_length_int(json), st_char_get(json));
			json_decref_d(json_objs[0]);
			outcome = false;
		}

		// If we make it this far we need to cleanup the JSON object.
		else if (outcome) {
			json_decref_d(json_objs[0]);
		}

		// Cleanup.
		client_close(client);
		st_cleanup(json);
		json = NULL;

		// Confirm the user was created.
		if (outcome && sql_num_rows(st_quick(MANAGEDBUF(1024), "SELECT usernum, userid FROM Users WHERE userid = '%.*s';",
			st_length_int(username), st_char_get(username))) != 1) {
			st_sprint(errmsg, "Verification of the user table entry after registering a fully qualified user failed.");
			outcome = false;
		}
		else if (outcome && sql_num_rows(st_quick(MANAGEDBUF(1024), "SELECT usernum, address FROM Mailboxes WHERE address =  '%.*s';",
			st_length_int(username), st_char_get(username))) != 1) {
			st_sprint(errmsg, "Verification of the mailbox table entry after registering a fully qualified user failed.");
			outcome = false;
		}

	}

	mark_point();
	log_test("HTTP / NETWORK / CAMEL / REGISTER / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_camel_basic_s) {

	log_disable();
	int64_t result = 0;
	bool_t outcome = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	// Reset the alerts and make the insecure attempt.
	if (status() && (result = sql_write(PLACER("UPDATE `Alerts` SET `acknowledged` = NULL WHERE `usernum` < 10;", 63))) < 0) {
		st_sprint(errmsg, "Unable to configure the alerts for the basic camelface test. { result = %li }", result);
		outcome = false;
	}
	else if (status() && !check_camel_basic_sthread(false, errmsg)) {
		outcome = false;
	}

	// Reset the alerts again, and make the secure attempt.
	else if (status() && (result = sql_write(PLACER("UPDATE `Alerts` SET `acknowledged` = NULL WHERE `usernum` < 10;", 63))) < 0) {
		st_sprint(errmsg, "Unable to configure the alerts for the basic camelface test. { result = %li }", result);
		outcome = false;
	}
	else if (status() && outcome && !check_camel_basic_sthread(true, errmsg)) {
		outcome = false;
	}

	log_test("HTTP / NETWORK / CAMEL / BASIC / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST


Suite * suite_check_camel(void) {

	Suite *s = suite_create("\tCAMEL");

	suite_check_testcase(s, "HTTP CAMEL", "HTTP Network Camel Auth/S", check_camel_auth_s);
	suite_check_testcase(s, "HTTP CAMEL", "HTTP Network Camel Register/S", check_camel_register_s);

	// The Camel tests will occassionally timeout on slower systems, in part because of how many camelface requests
	// are made inside this single "testcase" and in part because the camelface makes heavy use of secure memory,
	// which has significantly more overhead for each allocate/free oepration. As a result we give the camelface
	// tests twice as much time to complete when applicable. (The timeout is disabled when a debugger, or profiler
	// is in use.)
	suite_check_testcase_timeout(s, "HTTP CAMEL", "HTTP Network Camel Basic/S", check_camel_basic_s, ( case_timeout * 2 ));

	return s;
}
