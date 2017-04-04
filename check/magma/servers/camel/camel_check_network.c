/**
 * @file /magma/check/magma/servers/camel/camel_check_network.c
 *
 * @brief Functions used to test the Camelface.
 *
 */

#include "magma_check.h"

/**
 * @brief	Reads lines from the client until the end of the HTTP response is reached.
 *
 * @param	client	A client_t* to read lines from. An HTTP request should have been submitted
 * 			from the client before this function is called.
 *
 * @return	True if the end of the HTTP response was reached, false if client_read_line reads
 * 			a 0 length line before the last line is reached.
 */
stringer_t* check_camel_read_json(client_t *client, size_t length) {

	stringer_t *json = st_alloc(length);

	while (st_cmp_cs_eq(&(client->line), PLACER("\r\n", 2))) client_read_line(client);
	recv(client->sockd, st_char_get(json), length, 0);

	chr_t *foo = st_char_get(json);
	(void)foo;


	return json;
}

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
		if (client_read_line(client) <= 0) return false;
	}

	return ((*(pl_char_get(client->line) + 9) == '2') ? true : false);
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
bool_t check_camel_login(client_t *client, uint32_t id, chr_t *user, chr_t *pass, stringer_t *cookie) {

	size_t content_length = 0;
	uint32_t length = 62 + ns_length_get(user) + ns_length_get(pass) + uint32_digits(id);
	stringer_t *json = NULL, *message = "POST /portal/camel HTTP/1.1\r\n" \
		"Host: localhost:10000\r\n" \
		"Accept: */*\r\n" \
		"Content-Length: %u\r\n" \
		"Content-Type: application/x-www-form-urlencoded\r\n" \
		"\r\n"
		"{\"id\":%u,\"method\":\"auth\",\"params\":{\"username\":\"%s\",\"password\":\"%s\"}}\r\n"
		"\r\n";

	if (client_print(client, message, length, id, user, pass) != ((ns_length_get(message) - 8) + uint32_digits(length) +
		uint32_digits(id) + ns_length_get(user) + ns_length_get(pass)) || client_status(client) != 1 || !check_camel_status(client) ||
		!(content_length = check_http_content_length_get(client)) || !(json = check_camel_read_json(client, content_length))) {

		return false;
	}

	chr_t *foo = st_char_get(json);
	(void)foo;
	st_free(json);

	return true;
}

bool_t check_camel_login_sthread(client_t *client, stringer_t *errmsg) {

	if (!check_camel_login(client, 1, "princess", "password", NULL)) {

		st_sprint(errmsg, "Failed to return successful state after auth request.");
		return false;
	}

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
//		if (client_print(commands[i], i) != ns_length_get(commands[i]) - 2 + uint32_digits(i)) {
//
//		}
//	}

	return true;
}
