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
 * 				from the client before this function is called.
 * @return	True if the end of the HTTP response was reached, false if client_read_line reads
 * 				a 0 length line before the last line is reached.
 */
bool_t check_camel_response_read_end(client_t *client) {

	while (client_read_line(client) >= 0) {
		if (st_cmp_cs_starts(&(client->line), NULLER("\r\n")) == 0) return true;
	}
	return false;
}

/**
 * @brief	Reads lines from the client until the HTTP response status code is found, which it checks.
 *
 * @param	client	A client_t* to read lines from. An HTTP request should have been submitted
 * 				from the client before this function is called.
 * @return	True if the HTTP status code of the response begins with a '2', false otherwise.
 */
bool_t check_camel_response_status(client_t *client) {

	while (st_cmp_cs_starts(&(client->line), NULLER("HTTP/1.1"))) {
		if (client_read_line(client) <= 0) return false;
	}

	return ((*(pl_char_get(client->line)+9) == '2') ? true : false);
}

bool_t check_camel_login_sthread(client_t *client, stringer_t *errmsg) {

	chr_t *message = \
		"POST /json HTTP/1.1\r\n"\
		"Host: localhost:10000\r\n"\
		"Accept: */*\r\n"\
		"Content-Length: 79\r\n"\
		"Content-Type: application/x-www-form-urlencoded\r\n\r\n"\
		"{\"id\":1,\"method\":\"auth\",\"params\":{\"username\":\"princess\",\"password\":\"password\"}}\r\n";

	if (client_write(client, PLACER(message, ns_length_get(message))) != ns_length_get(message) || client_status(client) != 1) {

		st_sprint(errmsg, "The client failed to have a successful status after printing the request.");
		client_close(client);
		return false;
	}
	else if (!check_camel_response_status(client) || !check_camel_response_read_end(client)) {

		st_sprint(errmsg, "Failed to return successful response to login request.");
		client_close(client);
		return false;
	}

	client_close(client);

	return true;
}
