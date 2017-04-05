/**
 * @file /magma/check/magma/servers/camel/camel_check_network.c
 *
 * @brief Functions used to test the Camelface.
 *
 */

#include "magma_check.h"

/**
 * @brief	Reads lines from the client until the HTTP response status code is found, which it checks.
 *
 * @param	client	A client_t* to read lines from. An HTTP request should have been submitted
 * 			from the client before this function is called.
 *
 * @return	True if the HTTP status code of the response begins with a '2', false otherwise.
 */
bool_t check_camel_status(client_t *client) {

	while (st_cmp_cs_starts(&(client->line), NULLER("HTTP/1.1"))) {
		if (client_read_line(client) <= 2) return false;
	}

	return ((*(pl_char_get(client->line) + 9) == '2') ? true : false);
}

// Combine submit and read, because we now need to handle the connection being closed between requests.

/**
 * @brief	Reads lines from the client until the end of the HTTP response is reached.
 *
 * @param	client	A client_t* to read lines from. An HTTP request should have been submitted
 * 			from the client before this function is called.
 *
 * @return	True if the end of the HTTP response was reached, false if client_read_line reads
 * 			a 0 length line before the last line is reached.
 */
stringer_t * check_camel_json_read(client_t *client, size_t length) {

	stringer_t *json = NULL;
	uint32_t content_read = 0;

	while (st_cmp_cs_eq(&(client->line), PLACER("\r\n", 2))) {
		if (client_read_line(client) <= 0) return NULL;
	}

	while (content_read < length) {
		content_read += client_read(client);
		json = st_append_opts(8192, json, client->buffer);
	}

	if (st_empty(json)) {
		st_free(json);
		return NULL;
	}

	return json;
}

bool_t check_camel_json_submit(client_t *client, stringer_t *json, bool_t keep_alive) {

	chr_t *message = "POST /portal/camel HTTP/1.1\r\nHost: localhost:10000\r\nAccept: */*\r\n" \
		"Content-Length: %u\r\nContent-Type: application/x-www-form-urlencoded\r\nConnection: %s\r\n\r\n%s";

	if (client_print(client, message, st_length_get(json), (keep_alive ? "keep-alive" : "close"), st_char_get(json)) !=
		(st_length_get(message) - 6 + st_length_get(json) + (keep_alive ? 10 : 5)) || client_status(client) != 1) {

		return false;
	}

	return true;
}

/**
 * @brief	Submits an auth request to /portal/camel, setting *cookie to the session cookie in the response.
 *
 * @param	client	should be connected to an HTTP server.
 * @param	id		the value to place in the "id" field of the json request.
 * @param	user	the username of the account to issue the auth request for.
 * @param	pass	the password of the account to issue the auth request for.
 * @param	cookie	if not NULL, will be set to the value of Set-Cookie in the response.
 *
 * @return	True if the request was successful, false otherwise.
 */
bool_t check_camel_login(client_t *client, uint32_t id, stringer_t *user, stringer_t *pass, stringer_t *cookie) {

	json_error_t json_err;
	size_t content_length = 0;
	json_t *parsed_json = NULL, *result = NULL, *session = NULL;
	uint32_t length = 62 + ns_length_get(user) + ns_length_get(pass) + uint32_digits(id);
	stringer_t *json = NULL, *message = NULLER("POST /portal/camel HTTP/1.1\r\nHost: localhost:10000\r\nAccept: */*\r\n" \
		"Content-Length: %u\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\n{\"id\":%u,\"method\":\"auth\"," \
		"\"params\":{\"username\":\"%.*s\",\"password\":\"%.*s\"}}\r\n\r\n");

	if (client_print(client, st_char_get(message), length, id, st_length_int(user), st_char_get(user), st_length_int(pass),
		st_char_get(pass)) != ((st_length_get(message) - 12) + uint32_digits(length) + uint32_digits(id) + st_length_get(user) +
		st_length_get(pass)) || client_status(client) != 1 || !check_camel_status(client) ||
		!(content_length = check_http_content_length_get(client)) || !(json = check_camel_json_read(client, content_length))) {

		return false;
	}
	else if (!(parsed_json = json_loads_d(st_char_get(json), 0, &json_err)) || !(result = json_object_get_d(parsed_json, "result")) ||
		!(session = json_object_get_d(result, "session"))) {

		return false;
	}
	else if (cookie && st_sprint(cookie, "%s", json_string_value_d(session)) == -1) {
		return false;
	}

	st_cleanup(json);
	if (result) mm_free(result);
	if (session) mm_free(session);
	if (parsed_json) mm_free(parsed_json);

	return true;
}

// LOW: Test the four different ways of preserving a session token: Cookie, URL param, JSON param, Form post.
bool_t check_camel_auth_sthread(client_t *client, stringer_t *errmsg) {

	stringer_t *cookie = MANAGEDBUF(1024);

	if (!check_camel_login(client, 1, PLACER("princess", 8), PLACER("password", 8), cookie)) {

		st_sprint(errmsg, "Failed to return successful state after auth request.");
		return false;
	}

	st_cleanup(cookie);
	client_close(client);

	return true;
}

bool_t check_camel_basic_sthread(client_t *client, stringer_t *errmsg) {

//	chr_t *commands = {
//		"{\"id\":%u,\"method\":\"config.edit\",\"params\":{\"key\":\"value\"}}",
//		"{\"id\":%u,\"method\":\"config.load\"}",
//		"{\"id\":%u,\"method\":\"config.edit\",\"params\":{\"key\":null}}",
//		"{\"id\":%u,\"method\":\"folders.add\",\"params\":{\"context\":\"contacts\",\"name\":\"Flight Crew\"}}",
//		"{\"id\":%u,\"method\":\"folders.list\",\"params\":{\"context\":\"contacts\"}}",
//		"{\"id\":%u,\"method\":\"cookies\"}",
//		"{\"id\":%u,\"method\":\"alert.list\"}",
//		"{\"id\":%u,\"method\":\"alert.acknowledge\",\"params\":[1,7,13]}",
//		"{\"id\":%u,\"method\":\"folders.rename\",\"params\":{\"context\":\"contacts\",\"folderID\":1,\"name\"Camel\"}}",
//		"{\"id\":%u,\"method\":\"folders.remove\",\"params\":{\"context\":\"contacts\",\"folderID\":1}}",
//	};
//
//	for (size_t i = 0; i < sizeof(commands)/sizeof(chr_t*); i++) {
//		if (client_print(client, commands[i], i) != ns_length_get(commands[i]) - 2 + uint32_digits(i)) {
//
//		}
//	}

	return true;
}
