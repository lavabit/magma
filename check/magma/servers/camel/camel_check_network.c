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

bool_t check_camel_json_write(client_t *client, stringer_t *json, stringer_t *cookie, bool_t keep_alive) {

	chr_t *message = "POST /portal/camel HTTP/1.1\r\nHost: localhost:10000\r\nAccept: */*\r\nContent-Length: %u\r\n" \
		"Content-Type: application/x-www-form-urlencoded\r\nCookie: %.*s\r\nConnection: %s\r\n\r\n%.*s";

	if (client_print(client, message, st_length_get(json), st_length_int(cookie), st_char_get(cookie),
		(keep_alive ? "keep-alive" : "close"), st_char_get(json)) != (st_length_get(message) - 6 + st_length_get(json) +
		(keep_alive ? 10 : 5)) || client_status(client) != 1) {

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

		st_cleanup(json);
		if (result) mm_free(result);
		if (session) mm_free(session);
		if (parsed_json) mm_free(parsed_json);
		return false;
	}
	else if (cookie && st_sprint(cookie, "%s", json_string_value_d(session)) == -1) {

		mm_free(result);
		mm_free(session);
		st_cleanup(json);
		mm_free(parsed_json);
		return false;
	}

	st_cleanup(json);
	mm_free(result);
	mm_free(session);
	mm_free(parsed_json);

	return true;
}

// LOW: Test the four different ways of preserving a session token: Cookie, URL param, JSON param, Form post.
bool_t check_camel_auth_sthread(client_t *client, stringer_t *errmsg) {

	stringer_t *cookie = MANAGEDBUF(1024);

	if (!check_camel_login(client, 1, PLACER("princess", 8), PLACER("password", 8), cookie)) {

		st_sprint(errmsg, "Failed to return successful state after auth request.");
		return false;
	}

	client_close(client);
	return true;
}

bool_t check_camel_basic_sthread(client_t *client, stringer_t *errmsg) {

	uint32_t content_length = 0, folderid = 0, folderid_buff = 0;
	stringer_t *cookie = MANAGEDBUF(1024), *json = NULL, *commands[] = {
		NULLER("{\"id\":2,\"method\":\"config.edit\",\"params\":{\"key\":\"value\"}}"),
		NULLER("{\"id\":3,\"method\":\"config.load\"}"),
		NULLER("{\"id\":4,\"method\":\"config.edit\",\"params\":{\"key\":null}}"),
		NULLER("{\"id\":5,\"method\":\"config.load\"}"),
		NULLER("{\"id\":6,\"method\":\"config.edit\",\"params\":{\"key.3943\":\"18346\"}"),
		NULLER("{\"id\":7,\"method\":\"folders.add\",\"params\":{\"context\":\"contacts\",\"name\":\"Flight Crew\"}}"),
		NULLER("{\"id\":8,\"method\":\"folders.list\",\"params\":{\"context\":\"contacts\"}}"),
		NULLER("{\"id\":9,\"method\":\"contacts.add\",\"params\":{\"folderID\":37, \"contact\":{\"name\":\"Jenna\", \"email\":\"jenna@jameson.com\"}}}"),
		NULLER("{\"id\":10,\"method\":\"contacts.copy\",\"params\":{\"sourceFolderID\":37, \"targetFolderID\":37, \"contactID\": 1 }}"),
		NULLER("{\"id\":11,\"method\":\"contacts.list\",\"params\":{\"folderID\":37 }}"),
		NULLER("{\"id\":12,\"method\":\"contacts.edit\",\"params\":{\"folderID\":37, \"contactID\":1, \"contact\":{\"name\":\"Jenna Marie Massoli\", \"email\":\"jenna+private-chats@jameson.com\"}}}"),
		NULLER("{\"id\":13,\"method\":\"contacts.load\",\"params\":{\"folderID\":37, \"contactID\":2 }}"),
		NULLER("{\"id\":14,\"method\":\"contacts.edit\",\"params\":{\"folderID\":37, \"contactID\":1, \"contact\":{\"name\":\"Jenna\", \"email\":\"jenna@jameson.com\", \"phone\":\"2145551212\", \"notes\":\"The Tuesday night hottie!\"}}}"),
		NULLER("{\"id\":15,\"method\":\"contacts.load\",\"params\":{\"folderID\":37, \"contactID\":2 }}"),
		NULLER("{\"id\":16,\"method\":\"folders.add\",\"params\":{\"context\":\"contacts\",\"name\":\"Lovers\"}}"),
		NULLER("{\"id\":17,\"method\":\"contacts.move\",\"params\":{ \"contactID\":1, \"sourceFolderID\":37, \"targetFolderID\": }}"),
		NULLER("{\"id\":18,\"method\":\"contacts.list\",\"params\":{\"folderID\":37 }}"),
		NULLER("{\"id\":19,\"method\":\"contacts.list\",\"params\":{\"folderID\": }}"),
		NULLER("{\"id\":20,\"method\":\"contacts.remove\",\"params\":{\"folderID\":37, \"contactID\":1 }}"),
		NULLER("{\"id\":21,\"method\":\"contacts.remove\",\"params\":{\"folderID\":, \"contactID\": }}"),
		NULLER("{\"id\":22,\"method\":\"contacts.list\",\"params\":{\"folderID\": }}"),
		NULLER("{\"id\":23,\"method\":\"folders.remove\",\"params\":{\"context\":\"contacts\",\"folderID\":37 }}"),
		NULLER("{\"id\":24,\"method\":\"folders.remove\",\"params\":{\"context\":\"contacts\",\"folderID\": }}"),
		NULLER("{\"id\":25,\"method\":\"cookies\"}"),
		NULLER("{\"id\":26,\"method\":\"alert.list\"}"),
		NULLER("{\"id\":27,\"method\":\"alert.acknowledge\",\"params\":[1,7,13]}"),
		NULLER("{\"id\":28,\"method\":\"alert.list\"}"),
		NULLER("{\"id\":29,\"method\":\"folders.list\",\"params\":{\"context\":\"mail\"}}"),
		NULLER("{\"id\":30,\"method\":\"folders.list\",\"params\":{\"context\":\"settings\"}}"),
		NULLER("{\"id\":31,\"method\":\"folders.list\",\"params\":{\"context\":\"help\"}}"),
		NULLER("{\"id\":32,\"method\":\"folders.add\",\"params\":{\"context\":\"mail\",\"name\":\"Camel\"}}"),
		NULLER("{\"id\":33,\"method\":\"folders.add\",\"params\":{\"context\":\"mail\",\"parentID\":,\"name\":\"Toe\"}}"),
		NULLER("{\"id\":34,\"method\":\"folders.add\",\"params\":{\"context\":\"mail\",\"parentID\":,\"name\":\"Rocks\"}}"),
		NULLER("{\"id\":35,\"method\":\"folders.rename\",\"params\":{\"context\":\"mail\",\"folderID\":,\"name\":\"Dames.Rock\"}}"),
		NULLER("{\"id\":36,\"method\":\"folders.rename\",\"params\":{\"context\":\"mail\",\"folderID\":,\"name\":\"Clams\"}}"),
		NULLER("{\"id\":37,\"method\":\"folders.remove\",\"params\":{\"context\":\"mail\",\"folderID\": }}"),
		NULLER("{\"id\":38,\"method\":\"folders.remove\",\"params\":{\"context\":\"mail\",\"folderID\": }}"),
		NULLER("{\"id\":39,\"method\":\"folders.remove\",\"params\":{\"context\":\"mail\",\"folderID\": }}"),
		NULLER("{\"id\":40,\"method\":\"folders.remove\",\"params\":{\"context\":\"mail\",\"folderID\": }}"),
		NULLER("{\"id\":41,\"method\":\"aliases\"}"),
		NULLER("{\"id\":42,\"method\":\"folders.add\",\"params\":{\"context\":\"mail\",\"name\":\"Duplicate\"}}"),
		NULLER("{\"id\":43,\"method\":\"messages.copy\",\"params\":{\"messageIDs\": [], \"sourceFolderID\":1, \"targetFolderID\": }}"),
		NULLER("{\"id\":44,\"method\":\"messages.copy\",\"params\":{\"messageIDs\": [], \"sourceFolderID\":1, \"targetFolderID\": }}"),
		NULLER("{\"id\":45,\"method\":\"folders.remove\",\"params\":{\"context\":\"mail\",\"folderID\": }}"),
		NULLER("{\"id\":46,\"method\":\"folders.add\",\"params\":{\"context\":\"mail\",\"name\":\"Duplicate\"}}"),
		NULLER("{\"id\":47,\"method\":\"messages.load\",\"params\":{\"messageID\": , \"folderID\":1, \"sections\": [\"meta\", \"source\", \"security\", \"server\", \"header\", \"body\", \"attachments\" ]}}"),
		NULLER("{\"id\":48,\"method\":\"messages.copy\",\"params\":{\"messageIDs\": [], \"sourceFolderID\":1, \"targetFolderID\": }}"),
		NULLER("{\"id\":49,\"method\":\"folders.remove\",\"params\":{\"context\":\"mail\",\"folderID\": }}"),
		NULLER("{\"id\":50,\"method\":\"messages.flag\",\"params\":{\"action\":\"add\", \"flags\":[\"flagged\"], \"messageIDs\": [], \"folderID\":1 }}"),
		NULLER("{\"id\":51,\"method\":\"messages.tags\",\"params\":{\"action\":\"add\", \"tags\":[\"girlie\",\"girlie-6169\"], \"messageIDs\": [], \"folderID\":1 }}"),
		NULLER("{\"id\":52,\"method\":\"messages.flag\",\"params\":{\"action\":\"list\", \"messageIDs\": [], \"folderID\":1 }}"),
		NULLER("{\"id\":53,\"method\":\"messages.tags\",\"params\":{\"action\":\"list\", \"messageIDs\": [], \"folderID\":1 }}"),
		NULLER("{\"id\":54,\"method\":\"messages.list\",\"params\":{\"folderID\":1 }}"),
		NULLER("{\"id\":55,\"method\":\"folders.tags\",\"params\":{\"context\":\"mail\",\"folderID\":1 }}"),
		NULLER("{\"id\":56,\"method\":\"folders.add\",\"params\":{\"context\":\"mail\",\"name\":\"Mover\"}}"),
		NULLER("{\"id\":57,\"method\":\"messages.move\",\"params\":{\"messageIDs\": [], \"sourceFolderID\": 1, \"targetFolderID\": }}"),
		NULLER("{\"id\":58,\"method\":\"messages.remove\",\"params\":{\"folderID\":,\"messageIDs\":[]}}"),
		NULLER("{\"id\":59,\"method\":\"folders.remove\",\"params\":{\"context\":\"mail\",\"folderID\": }}"),
		NULLER("{\"id\":60,\"method\":\"logout\"}")
	};

	if (!check_camel_login(client, 1, PLACER("princess", 8), PLACER("password", 8), cookie)) {

		st_sprint(errmsg, "Failed to return successful response after auth request.");
		client_close(client);
		return false;
	}

	// Test config.edit { key = "key", value = "value" }
	if (!check_camel_json_write(client, st_char_get(commands[0]), cookie, true) || client_status(client) != 1 || !check_camel_status(client) ||
		!(content_length = check_http_content_length_get(client)) || !(json = check_camel_json_read(client, content_length))) {

		st_sprint(errmsg, "Failed to return successful response after config.edit.");
		client_close(client);
		return false;
	}

	st_free(json);

	// Test config.load
	if (!check_camel_json_write(client, st_char_get(commands[1]), cookie, true) || client_status(client) != 1 || !check_camel_status(client) ||
		!(content_length = check_http_content_length_get(client)) || !(json = check_camel_json_read(client, content_length))) {

		st_sprint(errmsg, "Failed to return successful response after config.load.");
		client_close(client);
		return false;
	}

	st_free(json);

	// Test config.edit { key = "key", "value" = null }
	if (!check_camel_json_write(client, st_char_get(commands[2]), cookie, true) || client_status(client) != 1 || !check_camel_status(client) ||
		!(content_length = check_http_content_length_get(client)) || !(json = check_camel_json_read(client, content_length))) {

		st_sprint(errmsg, "Failed to return successful response after config.edit.");
		client_close(client);
		return false;
	}

	st_free(json);

	// Test config.load
	if (!check_camel_json_write(client, st_char_get(commands[3]), cookie, true) || client_status(client) != 1 || !check_camel_status(client) ||
		!(content_length = check_http_content_length_get(client)) || !(json = check_camel_json_read(client, content_length))) {

		st_sprint(errmsg, "Failed to return successful response after config.load.");
		client_close(client);
		return false;
	}

	st_free(json);

	// Test config.edit { key = "key.3943", value = "18346" }
	if (!check_camel_json_write(client, st_char_get(commands[4]), cookie, true) || client_status(client) != 1 || !check_camel_status(client) ||
		!(content_length = check_http_content_length_get(client)) || !(json = check_camel_json_read(client, content_length))) {

		st_sprint(errmsg, "Failed to return successful response after config.edit.");
		client_close(client);
		return false;
	}

	st_free(json);

	// Test folders.add { context = "contacts", name = "Flight Crew" }
	if (!check_camel_json_write(client, st_char_get(commands[5]), cookie, true) || client_status(client) != 1 || !check_camel_status(client) ||
		!(content_length = check_http_content_length_get(client)) || !(json = check_camel_json_read(client, content_length)) ||
		json_unpack_d(json, "{s:{s:i}}", "result", "folderID", &folderid)) {

		st_sprint(errmsg, "Failed to return successful response after folders.add.");
		client_close(client);
		return false;
	}

	st_free(json);

	// Test folders.list { context = "contacts" }
	if (!check_camel_json_write(client, st_char_get(commands[6]), cookie, true) || client_status(client) != 1 || !check_camel_status(client) ||
		!(content_length = check_http_content_length_get(client)) || !(json = check_camel_json_read(client, content_length)) ||
		json_unpack_d(json, "{s:[{s:i}]}", "result", "folderID", &folderid_buff) || folderid != folderid_buff) {

		st_sprint(errmsg, "Failed to return successful response after folders.list.");
		client_close(client);
		return false;
	}

	st_free(json);
	client_close(client);

	return true;
}
