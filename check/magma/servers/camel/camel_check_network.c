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
bool_t check_camel_read_end(client_t *client) {

	while (client_read_line(client) >= 0) {
		if (st_cmp_cs_starts(&(client->line), NULLER("{\"jsonrpc\":\"2.0\",")) == 0) return true;
	}
	return false;
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

	return ((*(pl_char_get(client->line)+9) == '2') ? true : false);
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

	uint32_t content_length = 62 + ns_length_get(user) + ns_length_get(pass) + uint32_digits(id);
	chr_t *message = "POST /portal/camel HTTP/1.1\r\n" \
		"Host: localhost:10000\r\n" \
		"Accept: */*\r\n" \
		"Content-Length: %u\r\n" \
		"Content-Type: application/x-www-form-urlencoded\r\n" \
		"\r\n"
		"{\"id\":%u,\"method\":\"auth\",\"params\":{\"username\":\"%s\",\"password\":\"%s\"}}\r\n"
		"\r\n";

	if (client_print(client, message, content_length, id, user, pass) != (ns_length_get(message) + uint32_digits(id) + ns_length_get(user) + ns_length_get(pass)) || client_status(client) != 1 || !check_camel_status(client) ||
		!check_camel_read_end(client)) {

		return false;
	}

	chr_t *foo = pl_char_get(client->line);
	(void)foo;
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
