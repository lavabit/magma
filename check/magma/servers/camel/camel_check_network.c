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

client_t * check_camel_connect(bool_t secure) {

	client_t *client = NULL;
	server_t *server = NULL;

	if (!(server = servers_get_by_protocol(HTTP, secure))) {
		return NULL;
	}
	else if (!(client = client_connect("localhost", server->network.port))) {
		return NULL;
	}
	else if (secure && client_secure(client) != 0) {
		client_close(client);
		return NULL;
	}

	return client;
}

bool_t check_camel_print(stringer_t *req, stringer_t *cookie, stringer_t *res, bool_t secure) {

	uint32_t length = 0;
	client_t *client = NULL;
	stringer_t *json = NULL;

	if (!req) return false;

	// Submit the command and check the status of the response.
	if (!(client = check_camel_connect(secure)) || !check_camel_json_write(client, req, cookie, secure) ||
		!check_camel_status(client) || !(length = check_http_content_length_get(client)) ||
		!(json = check_camel_json_read(client, length))) {

		client_close(client);
		st_cleanup(json);
		return false;
	}

	if (res) st_write(res, json, PLACER("\0",1));

	client_close(client);
	st_cleanup(json);
	return true;
}

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
		"Content-Type: application/x-www-form-urlencoded\r\nCookie: portal=%.*s;\r\nConnection: %s\r\n\r\n%.*s\r\n\r\n";

	if (client_print(client, message, st_length_get(json), st_length_int(cookie), (cookie ? st_char_get(cookie) : ""),
		(keep_alive ? "keep-alive" : "close"), st_length_int(json), st_char_get(json)) != (ns_length_get(message) - 12 +
		st_length_get(json) + uint32_digits(st_length_int(json)) + (cookie ? st_length_get(cookie) : 0) + (keep_alive ? 10 : 5))) {

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

	json_error_t err;
	size_t content_length = 0;
	json_t *json_root = NULL, *json_result = NULL, *json_key = NULL;
	uint32_t length = 62 + ns_length_get(user) + ns_length_get(pass) + uint32_digits(id);
	stringer_t *json = NULL, *message = NULLER("POST /portal/camel HTTP/1.1\r\nHost: localhost:10000\r\nAccept: */*\r\n" \
		"Content-Length: %u\r\nContent-Type: application/x-www-form-urlencoded\r\n\r\n{\"id\":%u,\"method\":\"auth\"," \
		"\"params\":{\"username\":\"%.*s\",\"password\":\"%.*s\"}}\r\n\r\n");

	if (client_print(client, st_char_get(message), length, id, st_length_int(user), st_char_get(user), st_length_int(pass),
		st_char_get(pass)) != ((st_length_get(message) - 12) + uint32_digits(length) + uint32_digits(id) + st_length_get(user) +
		st_length_get(pass)) || !check_camel_status(client) || !(content_length = check_http_content_length_get(client)) ||
		!(json = check_camel_json_read(client, content_length))) {

		return false;
	}
	else if (!(json_root = json_loads_d(st_char_get(json), 0, &err)) || !(json_result = json_object_get_d(json_root, "result")) ||
		!(json_key = json_object_get_d(json_result, "session"))) {

		st_free(json);
		mm_cleanup(json_root, json_result, json_key);

		return false;
	}
	else if (cookie && st_sprint(cookie, "%s", json_string_value_d(json_key)) == -1) {

		st_free(json);
		mm_cleanup(json_root, json_result, json_key);
		return false;
	}

	st_free(json);
	mm_cleanup(json_root, json_result, json_key);

	return true;
}

// LOW: Test the four different ways of preserving a session token: Cookie, URL param, JSON param, Form post.
bool_t check_camel_auth_sthread(bool_t secure, stringer_t *errmsg) {

	client_t *client = NULL;
	stringer_t *cookie = MANAGEDBUF(1024);

	if (!(client = check_camel_connect(secure))) {
		st_sprint(errmsg, "There were no HTTP servers available for %s connections.", (secure ? "TLS" : "TCP"));
		return false;
	}
	else if (!check_camel_login(client, 1, PLACER("princess", 8), PLACER("password", 8), cookie)) {

		st_sprint(errmsg, "Failed to return successful state after auth request.");
		return false;
	}

	client_close(client);
	return true;
}

bool_t check_camel_basic_sthread(bool_t secure, stringer_t *errmsg) {

	json_error_t err;
	client_t *client = NULL;
	json_t *json_objs[2] = { NULL, NULL };
	bool_t contains_entries[2] = { false, false };
	const chr_t *json_values[4] = { NULL, NULL, NULL, NULL };
	chr_t *choices = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz";
	int32_t folder_ids[2] = { -1, -1 }, contact_ids[3] = { -1, -1, -1 }, alert_ids[2] = { -1, -1 };
	stringer_t *cookie = MANAGEDBUF(1024), *command = MANAGEDBUF(8192), *json = MANAGEDBUF(8192),
		*rand_strs[4] = { MANAGEDBUF(64), MANAGEDBUF(64), MANAGEDBUF(64), MANAGEDBUF(64) };
	chr_t *commands[] = {
		"{\"id\":2,\"method\":\"config.edit\",\"params\":{\"%.*s\":\"%.*s\"}}",
		"{\"id\":3,\"method\":\"config.load\"}",
		"{\"id\":4,\"method\":\"config.edit\",\"params\":{\"%.*s\":null}}",
		"{\"id\":5,\"method\":\"config.load\"}",
		"{\"id\":6,\"method\":\"config.edit\",\"params\":{\"%.*s\":\"%.*s\"}}",
		"{\"id\":7,\"method\":\"folders.add\",\"params\":{\"context\":\"contacts\",\"name\":\"%.*s\"}}",
		"{\"id\":8,\"method\":\"folders.list\",\"params\":{\"context\":\"contacts\"}}",
		"{\"id\":9,\"method\":\"contacts.add\",\"params\":{\"folderID\":%u, \"contact\":{\"name\":\"%.*s\", \"email\":\"%.*s\"}}}",
		"{\"id\":10,\"method\":\"contacts.copy\",\"params\":{\"sourceFolderID\":%u, \"targetFolderID\":%u, \"contactID\":%u}}",
		"{\"id\":11,\"method\":\"contacts.list\",\"params\":{\"folderID\":%u}}",
		"{\"id\":12,\"method\":\"contacts.edit\",\"params\":{\"folderID\":%u, \"contactID\":%u, \"contact\":{\"name\":\"%.*s\", \"email\":\"%.*s\"}}}",
		"{\"id\":13,\"method\":\"contacts.load\",\"params\":{\"folderID\":%u, \"contactID\":%u }}",
		"{\"id\":14,\"method\":\"contacts.edit\",\"params\":{\"folderID\":%u, \"contactID\":%u, \"contact\":{\"name\":\"%.*s\", \"email\":\"%.*s\", \"phone\":\"%.*s\", \"notes\":\"%.*s\"}}}",
		"{\"id\":15,\"method\":\"contacts.load\",\"params\":{\"folderID\":%u, \"contactID\":%u}}",
		"{\"id\":16,\"method\":\"folders.add\",\"params\":{\"context\":\"contacts\",\"name\":\"%.*s\"}}",
		"{\"id\":17,\"method\":\"contacts.move\",\"params\":{ \"contactID\":%u, \"sourceFolderID\":%u, \"targetFolderID\":%u}}",
		"{\"id\":18,\"method\":\"contacts.list\",\"params\":{\"folderID\":%u}}",
		"{\"id\":19,\"method\":\"contacts.list\",\"params\":{\"folderID\":%u}}",
		"{\"id\":20,\"method\":\"contacts.remove\",\"params\":{\"folderID\":%u, \"contactID\":%u}}",
		"{\"id\":21,\"method\":\"contacts.remove\",\"params\":{\"folderID\":%u, \"contactID\":%u}}",
		"{\"id\":22,\"method\":\"contacts.list\",\"params\":{\"folderID\":%u}}",
		"{\"id\":23,\"method\":\"folders.remove\",\"params\":{\"context\":\"contacts\",\"folderID\":%u}}",
		"{\"id\":24,\"method\":\"folders.remove\",\"params\":{\"context\":\"contacts\",\"folderID\":%u}}",
		"{\"id\":25,\"method\":\"cookies\"}",
		"{\"id\":26,\"method\":\"alert.list\"}",
		"{\"id\":27,\"method\":\"alert.acknowledge\",\"params\":{\"alertIDs\":[%u]}}",
		"{\"id\":28,\"method\":\"alert.list\"}",
		"{\"id\":29,\"method\":\"folders.list\",\"params\":{\"context\":\"mail\"}}",
		"{\"id\":30,\"method\":\"folders.list\",\"params\":{\"context\":\"settings\"}}",
		"{\"id\":31,\"method\":\"folders.list\",\"params\":{\"context\":\"help\"}}",
		"{\"id\":32,\"method\":\"folders.add\",\"params\":{\"context\":\"mail\",\"name\":\"%.*s\"}}",
		"{\"id\":33,\"method\":\"folders.add\",\"params\":{\"context\":\"mail\",\"parentID\":%u,\"name\":\"%.*s\"}}",
		"{\"id\":34,\"method\":\"folders.add\",\"params\":{\"context\":\"mail\",\"parentID\":%u,\"name\":\"%.*s\"}}",
		"{\"id\":35,\"method\":\"folders.rename\",\"params\":{\"context\":\"mail\",\"folderID\":%u,\"name\":\"%.*s\"}}",
		"{\"id\":36,\"method\":\"folders.rename\",\"params\":{\"context\":\"mail\",\"folderID\":%u,\"name\":\"%.*s\"}}",
		"{\"id\":37,\"method\":\"folders.remove\",\"params\":{\"context\":\"mail\",\"folderID\":%u}}",
		"{\"id\":38,\"method\":\"folders.remove\",\"params\":{\"context\":\"mail\",\"folderID\":%u}}",
		"{\"id\":39,\"method\":\"folders.remove\",\"params\":{\"context\":\"mail\",\"folderID\":%u}}",
		"{\"id\":40,\"method\":\"folders.remove\",\"params\":{\"context\":\"mail\",\"folderID\":%u}}",
		"{\"id\":41,\"method\":\"aliases\"}",
		"{\"id\":42,\"method\":\"folders.add\",\"params\":{\"context\":\"mail\",\"name\":\"%.*s\"}}",
		"{\"id\":43,\"method\":\"messages.copy\",\"params\":{\"messageIDs\":[%u], \"sourceFolderID\":%u, \"targetFolderID\":%u}}",
		"{\"id\":44,\"method\":\"messages.copy\",\"params\":{\"messageIDs\":[%u], \"sourceFolderID\":%u, \"targetFolderID\":%u}}",
		"{\"id\":45,\"method\":\"folders.remove\",\"params\":{\"context\":\"mail\",\"folderID\":%u}}",
		"{\"id\":46,\"method\":\"folders.add\",\"params\":{\"context\":\"mail\",\"name\":\"%.*s\"}}",
		"{\"id\":47,\"method\":\"messages.load\",\"params\":{\"messageID\":%u, \"folderID\":%u, \"sections\": [\"meta\", \"source\", \"security\", \"server\", \"header\", \"body\", \"attachments\"]}}",
		"{\"id\":48,\"method\":\"messages.copy\",\"params\":{\"messageIDs\":[%u], \"sourceFolderID\":%u, \"targetFolderID\":%u}}",
		"{\"id\":49,\"method\":\"folders.remove\",\"params\":{\"context\":\"mail\",\"folderID\":%u}}",
		"{\"id\":50,\"method\":\"messages.flag\",\"params\":{\"action\":\"add\", \"flags\":[\"flagged\"], \"messageIDs\": [%u], \"folderID\":%u}}",
		"{\"id\":51,\"method\":\"messages.tags\",\"params\":{\"action\":\"add\", \"tags\":[\"girlie\",\"girlie-6169\"], \"messageIDs\": [%u], \"folderID\":%u}}",
		"{\"id\":52,\"method\":\"messages.flag\",\"params\":{\"action\":\"list\", \"messageIDs\":[%u], \"folderID\":%u}}",
		"{\"id\":53,\"method\":\"messages.tags\",\"params\":{\"action\":\"list\", \"messageIDs\":[%u], \"folderID\":%u}}",
		"{\"id\":54,\"method\":\"messages.list\",\"params\":{\"folderID\":%u}}",
		"{\"id\":55,\"method\":\"folders.tags\",\"params\":{\"context\":\"mail\",\"folderID\":%u}}",
		"{\"id\":56,\"method\":\"folders.add\",\"params\":{\"context\":\"mail\",\"name\":\"Mover\"}}",
		"{\"id\":57,\"method\":\"messages.move\",\"params\":{\"messageIDs\":[%u], \"sourceFolderID\":%u, \"targetFolderID\":%u}}",
		"{\"id\":58,\"method\":\"messages.remove\",\"params\":{\"folderID\":%u,\"messageIDs\":[%u]}}",
		"{\"id\":59,\"method\":\"folders.remove\",\"params\":{\"context\":\"mail\",\"folderID\":%u}}",
		"{\"id\":60,\"method\":\"logout\"}"
	};

	if (!(client = check_camel_connect(secure))) {
		st_sprint(errmsg, "There were no HTTP servers available for %s connections.", (secure ? "TLS" : "TCP"));
	}
	else if (!check_camel_login(client, 1, PLACER("princess", 8), PLACER("password", 8), cookie)) {

		st_sprint(errmsg, "Failed to return successful response after auth request.");
		client_close(client);
		return false;
	}

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Test config.edit 	: commands[0]
	// JSON Command			: {"id":2,"method":"config.edit","params":{<rand_strs[0]>:<rand_strs[1]>}}"
	// Expected Response 	: {"jsonrpc":"2.0","result":{"config.edit":"success"},"id":2}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Generate the random inputs for "key" and "value".
	if (!rand_choices(choices, 64, rand_strs[0]) || !rand_choices(choices, 64, rand_strs[1])) {

		st_sprint(errmsg, "Failed to create random inputs. { command = \"%s\" }", commands[0]);
		client_close(client);
		return false;
	}

	// Construct the command string.
	else if (!(st_sprint(command, commands[0], st_length_int(rand_strs[0]), st_char_get(rand_strs[0]), st_length_int(rand_strs[1]),
		st_char_get(rand_strs[1])))) {

		st_sprint(errmsg, "Failed to create command string. { command = \"%.*s\" }", st_length_int(command), st_char_get(command));
		client_close(client);
		return false;
	}

	// Submit the command and check the status of the response.
	else if (!check_camel_print(command, cookie, json, secure)) {

		st_sprint(errmsg, "Failed to return a successful HTTP response. { command = \"%.*s\" }", st_length_int(command),
			st_char_get(command));
		client_close(client);
		return false;
	}

	// Parse the returned JSON.
	else if (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err)) || json_unpack_d(json_objs[0], "{s:{s:s}}", "result",
		"config.edit", &json_values[0]) != 0) {

		st_sprint(errmsg, "Failed parsing the returned JSON. { command = \"%.*s\", json = \"%.*s\", value = \"%s\"}",
			st_length_int(command), st_char_get(command), st_length_int(json), st_char_get(json), json_values[0]);
		if (json_objs[0]) json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Check if the returned JSON is correct.
	else if (st_cmp_cs_eq(PLACER((chr_t *)json_values[0], ns_length_get(json_values[0])), PLACER("success", 7))) {

		st_sprint(errmsg, "Failed parsing the returned JSON. { command = \"%.*s\", json = \"%.*s\" }", st_length_int(command),
			st_char_get(command), st_length_int(json), st_char_get(json));
		json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Clean up before the next check.
	st_wipe(command);

	if (json_objs[0]) json_decref_d(json_objs[0]);

	json_objs[0] = NULL;
	json_values[0] = NULL;

	client_close(client);
	client = NULL;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Test config.edit 	: commands[1]
	// JSON Command			: {"id":3,"method":"config.load"}
	// Expected Response 	: {"jsonrpc":"2.0","result":{<rand_strs[0]>:{"value":<rand_strs[1]>, "flags":[]}, ...}, "id":3}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Submit the command and check the status of the response.
	if (!check_camel_print(NULLER(commands[1]), cookie, json, secure)) {

		st_sprint(errmsg, "Failed to return a successful HTTP response. { command = \"%s\" }", commands[1]);
		client_close(client);
		return false;
	}

	// Parse the returned JSON.
	else if (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err)) || json_unpack_d(json_objs[0], "{s:{s:{s:s}}}", "result",
		st_char_get(rand_strs[0]), "value", &json_values[0]) != 0) {

		st_sprint(errmsg, "Failed parsing the returned JSON. { command = \"%s\", json = \"%.*s\" }", commands[1],
			st_length_int(json), st_char_get(json));
		if (json_objs[0]) json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Check if the returned JSON is correct.
	else if (st_cmp_cs_eq(NULLER((chr_t *)json_values[0]), rand_strs[1])) {

		st_sprint(errmsg, "Failed to return a successful JSON response. { command = \"%s\", json = \"%.*s\" }", commands[0],
			st_length_int(json), st_char_get(json));
		json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Clean up before the next check.
	st_wipe(command);
	st_wipe(rand_strs[1]);

	if (json_objs[0]) json_decref_d(json_objs[0]);

	json_objs[0] = NULL;
	json_values[0] = NULL;

	client_close(client);
	client = NULL;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Test config.edit 	: commands[2]
	// JSON Command			: {"id":4,"method":"config.edit","params":{<rand_strs[0]>:null}}"
	// Expected Response 	: {"jsonrpc":"2.0","result":{"config.edit":"success"},"id":4}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Construct the command string.
	if (!(st_sprint(command, commands[2], st_length_int(rand_strs[0]), st_char_get(rand_strs[0])))) {

		st_sprint(errmsg, "Failed to create command string. { command = \"%s\" }", commands[2]);
		client_close(client);
		return false;
	}
	// Submit the command and check the status of the response.
	else if (!check_camel_print(command, cookie, json, secure)) {

		st_sprint(errmsg, "Failed to return a successful HTTP response. { command = \"%.*s\" }", st_length_int(command),
			st_char_get(command));
		client_close(client);
		return false;
	}

	// Parse the returned JSON.
	else if (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err)) || json_unpack_d(json_objs[0], "{s:{s:s}}",
		"result", "config.edit", &json_values[0]) != 0) {

		st_sprint(errmsg, "Failed parsing the returned JSON. { command = \"%.*s\", json = \"%.*s\", value = \"%s\", err = \"%s\" }",
			st_length_int(command), st_char_get(command), st_length_int(json), st_char_get(json), json_values[0], err.text);
		if (json_objs[0]) json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Check if the returned JSON is correct.
	else if (st_cmp_cs_eq(NULLER((chr_t *)json_values[0]), PLACER("success", 7))) {

		st_sprint(errmsg, "Failed to return a successful JSON response. { command = \"%.*s\", json = \"%.*s\" }",
			st_length_int(command), st_char_get(command), st_length_int(json), st_char_get(json));
		json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Clean up before the next check.
	st_wipe(command);

	if (json_objs[0]) json_decref_d(json_objs[0]);

	json_objs[0] = NULL;
	json_values[0] = NULL;

	client_close(client);
	client = NULL;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Test config.edit 	: commands[3]
	// JSON Command			: {"id":5,"method":"config.load"}
	// Expected Response 	: {"jsonrpc":"2.0","result":{ there should be no key matching <rand_strs[0]> }, "id":5}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Submit the command and check the status of the response.
	if (!check_camel_print(NULLER(commands[3]), cookie, json, secure)) {

		st_sprint(errmsg, "Failed to return a successful HTTP response. { command = \"%s\" }", commands[3]);
		client_close(client);
		return false;
	}

	// Parse the returned JSON.
	else if (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err)) || json_unpack_d(json_objs[0], "{s:{s:{s:s}}}", "result",
		st_char_get(rand_strs[0]), "value", &json_values[0]) == 0) {

		st_sprint(errmsg, "Failed to return a successful JSON response. { command = \"%s\", json = \"%.*s\" }", commands[3],
			st_length_int(json), st_char_get(json));
		if (json_objs[0]) json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Clean up before the next check.
	st_wipe(command);
	st_wipe(rand_strs[0]);

	if (json_objs[0]) json_decref_d(json_objs[0]);

	json_objs[0] = NULL;
	json_values[0] = NULL;

	client_close(client);
	client = NULL;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Test config.edit 	: commands[4]
	// JSON Command			: {"id":6,"method":"config.edit","params":{<rand_strs[0]>:<rand_strs[1]>}}"
	// Expected Response 	: {"jsonrpc":"2.0","result":{"config.edit":"success"},"id":6}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Generate the random inputs for "key" and "value".
	if (!rand_choices(choices, 64, rand_strs[0]) || !rand_choices(choices, 64, rand_strs[1])) {

		st_sprint(errmsg, "Failed to create random inputs. { command = \"%s\" }", commands[4]);
		client_close(client);
		return false;
	}

	// Construct the command string.
	else if (!(st_sprint(command, commands[4], st_length_int(rand_strs[0]), st_char_get(rand_strs[0]), st_length_int(rand_strs[1]),
		st_char_get(rand_strs[1])))) {

		st_sprint(errmsg, "Failed to create command string. { command = \"%.*s\" }", st_length_int(command), st_char_get(command));
		client_close(client);
		return false;
	}

	// Submit the command and check the status of the response.
	else if (!check_camel_print(command, cookie, json, secure)) {

		st_sprint(errmsg, "Failed to return a successful HTTP response. { command = \"%.*s\" }", st_length_int(command),
			st_char_get(command));
		client_close(client);
		return false;
	}

	// Parse the returned JSON.
	else if (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err)) || json_unpack_d(json_objs[0], "{s:{s:s}}", "result",
		"config.edit", &json_values[0]) != 0) {

		st_sprint(errmsg, "Failed parsing the returned JSON. { command = \"%.*s\", json = \"%.*s\", value = \"%s\"}",
			st_length_int(command), st_char_get(command), st_length_int(json), st_char_get(json), json_values[0]);
		if (json_objs[0]) json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Check if the returned JSON is correct.
	else if (st_cmp_cs_eq(PLACER((chr_t *)json_values[0], ns_length_get(json_values[0])), PLACER("success", 7))) {

		st_sprint(errmsg, "Failed parsing the returned JSON. { command = \"%.*s\", json = \"%.*s\" }", st_length_int(command),
			st_char_get(command), st_length_int(json), st_char_get(json));
		json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Clean up before the next check.
	st_wipe(command);

	if (json_objs[0]) json_decref_d(json_objs[0]);

	json_objs[0] = NULL;
	json_values[0] = NULL;

	client_close(client);
	client = NULL;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Test config.edit 	: commands[5]
	// JSON Command			: {"id":7,"method":"folders.add","params":{"context":"contacts","name:<rand_strs[0]>}}"
	// Expected Response 	: {"jsonrpc":"2.0","result":{"folderID":<folder_ids[0]>},"id":7}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Generate the random input for "name".
	if (!rand_choices(choices, 16, rand_strs[0])) {

		st_sprint(errmsg, "Failed to create random input. { command = \"%s\" }", commands[5]);
		client_close(client);
		return false;
	}

	// Construct the command string.
	else if (!(st_sprint(command, commands[5], st_length_int(rand_strs[0]), st_char_get(rand_strs[0])))) {

		st_sprint(errmsg, "Failed to create command string. { command = \"%s\", \"name\" = \"%.*s\"}", commands[5],
			st_length_int(rand_strs[0]), st_char_get(rand_strs[0]));
		client_close(client);
		return false;
	}

	// Submit the command and check the status of the response.
	else if (!check_camel_print(command, cookie, json, secure)) {

		st_sprint(errmsg, "Failed to return a successful HTTP response. { command = \"%.*s\" }", st_length_int(command),
			st_char_get(command));
		client_close(client);
		return false;
	}

	// Parse the returned JSON.
	else if (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err)) || (json_unpack_d(json_objs[0], "{s:{s:i}}", "result",
		"folderID", &folder_ids[0]) != 0)) {

		st_sprint(errmsg, "Failed to return a successful JSON response. { command = \"%.*s\", json = \"%.*s\" }", st_length_int(command),
			st_char_get(command), st_length_int(json), st_char_get(json));
		if (json_objs[0]) json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Clean up before the next check.
	st_wipe(command);

	if (json_objs[0]) json_decref_d(json_objs[0]);

	json_objs[0] = NULL;

	client_close(client);
	client = NULL;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Test config.edit 	: commands[6]
	// JSON Command			: {"id":8,"method":"folders.list","params":{"context":"contacts"}}
	// Expected Response 	: {"jsonrpc":"2.0","result":[{"context":"contacts","folderID":<folder_ids[0],"name":<rand_strs[0]>}], "id":8}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Submit the command and check the status of the response.
	if (!check_camel_print(NULLER(commands[6]), cookie, json, secure)) {

		st_sprint(errmsg, "Failed to return a successful HTTP response. { command = \"%s\" }", commands[6]);
		client_close(client);
		return false;
	}

	// Parse the returned JSON.
	else if (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err)) || !(json_objs[1] = json_object_get_d(json_objs[0],
		"result")) || !json_is_array(json_objs[1])) {

		st_sprint(errmsg, "Failed to return a successful JSON response. { command = \"%s\", json = \"%.*s\" }", commands[6],
			st_length_int(json), st_char_get(json));
		if (json_objs[0]) json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Search for the object describing the newly added folder.
	else {
		for (size_t i = 0; !contains_entries[0] && i < json_array_size_d(json_objs[1]); i++) {
			json_objs[2] = json_array_get_d(json_objs[1], i);
			if (json_unpack_d(json_objs[2], "{s:i,s:s}", "folderID", &folder_ids[1], "name", &json_values[0]) == 0 &&
				folder_ids[0] == folder_ids[1] && st_cmp_cs_eq(NULLER((chr_t *)json_values[0]), rand_strs[0]) == 0) {

				contains_entries[0] = true;
			}
		}
		if (!contains_entries[0]) {

			st_sprint(errmsg, "Failed to find folder entry in response. { command = \"%s\", json = \"%.*s\", folder_id = %d, folder_name = \"%.*s\" }",
				commands[6], st_length_int(json), st_char_get(json), folder_ids[0], st_length_int(rand_strs[0]), st_char_get(rand_strs[0]));
			json_decref_d(json_objs[0]);
			client_close(client);
			return false;
		}
	}

	// Clean up before the next check.
	st_wipe(command);
	st_wipe(rand_strs[0]);

	if (json_objs[0]) json_decref_d(json_objs[0]);

	contains_entries[0] = false;

	json_objs[0] = NULL;
	json_values[0] = NULL;

	client_close(client);
	client = NULL;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Test config.edit 	: commands[7]
	// JSON Command			: {"id\":9,"method":"contacts.add","params":{"folderID":<folder_ids[0]>,
	//							"contact":{"name":"<rand_strs[0]>", "email":"<rand_strs[1]>"}}}
	// Expected Response 	: {"jsonrpc":"2.0","result":{"contactID":<contact_ids[0]>},"id":9}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Generate the random input for "name" and "email".
	if (!rand_choices(choices, 64, rand_strs[0]) || !rand_choices(choices, 64, rand_strs[1])) {

		st_sprint(errmsg, "Failed to create random inputs. { command = \"%s\" }", commands[7]);
		client_close(client);
		return false;
	}

	// Construct the command string.
	else if (!(st_sprint(command, commands[7], folder_ids[0], st_length_int(rand_strs[0]), st_char_get(rand_strs[0]),
		st_length_int(rand_strs[1]), st_char_get(rand_strs[1])))) {

		st_sprint(errmsg, "Failed to create command string. { command = \"%s\", \"name\" = \"%.*s\"}", commands[7],
			st_length_int(rand_strs[0]), st_char_get(rand_strs[0]));
		client_close(client);
		return false;
	}

	// Submit the command and check the status of the response.
	else if (!check_camel_print(command, cookie, json, secure)) {

		st_sprint(errmsg, "Failed to return a successful HTTP response. { command = \"%.*s\" }", st_length_int(command),
			st_char_get(command));
		client_close(client);
		return false;
	}

	// Parse the returned JSON.
	else if (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err)) || (json_unpack_d(json_objs[0], "{s:{s:i}}", "result",
		"contactID", &contact_ids[0]) != 0)) {

		st_sprint(errmsg, "Failed to return a successful JSON response. { command = \"%.*s\", json = \"%.*s\" }", st_length_int(command),
			st_char_get(command), st_length_int(json), st_char_get(json));
		if (json_objs[0]) json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Clean up before the next check.
	st_wipe(command);

	if (json_objs[0]) json_decref_d(json_objs[0]);

	json_objs[0] = NULL;

	client_close(client);
	client = NULL;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Test config.edit 	: commands[8]
	// JSON Command			: {"id":10,"method":"contacts.copy","params":{"sourceFolderID":<folder_ids[0]>,
	//							"targetFolderID":folder_ids[0], "contactID":<contact_ids[0]>}}
	// Expected Response 	: {"jsonrpc":"2.0","result":{"contactID":<contact_ids[1]>},"id":10}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Construct the command string.
	if (!(st_sprint(command, commands[8], folder_ids[0], folder_ids[0], contact_ids[0]))) {

		st_sprint(errmsg, "Failed to create command string. { command = \"%s\"}", commands[8]);
		client_close(client);
		return false;
	}

	// Submit the command and check the status of the response.
	else if (!check_camel_print(command, cookie, json, secure)) {

		st_sprint(errmsg, "Failed to return a successful HTTP response. { command = \"%.*s\" }", st_length_int(command),
			st_char_get(command));
		client_close(client);
		return false;
	}

	// Parse the returned JSON.
	else if (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err)) || (json_unpack_d(json_objs[0], "{s:{s:i}}", "result",
		"contactID", &contact_ids[1]) != 0)) {

		st_sprint(errmsg, "Failed to return a successful JSON response. { command = \"%.*s\", json = \"%.*s\" }", st_length_int(command),
			st_char_get(command), st_length_int(json), st_char_get(json));
		if (json_objs[0]) json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Clean up before the next check.
	st_wipe(command);

	if (json_objs[0]) json_decref_d(json_objs[0]);

	json_objs[0] = NULL;

	client_close(client);
	client = NULL;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Test config.edit 	: commands[9]
	// JSON Command			: {"id":11,"method":"contacts.list","params":{"folderID":%u}}
	// Expected Response 	: {"jsonrpc":"2.0","result":[{"contactID":<contact_ids[0]>,"name":<rand_strs[0]>,email:<rand_strs[1]>},
	//							{same for contact_ids[1]}, ...],"id":11}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Construct the command string.
	if (!(st_sprint(command, commands[9], folder_ids[0]))) {

		st_sprint(errmsg, "Failed to create command string. { command = \"%.*s\" }", st_length_int(command), st_char_get(command));
		client_close(client);
		return false;
	}

	// Submit the command and check the status of the response.
	else if (!check_camel_print(command, cookie, json, secure)) {

		st_sprint(errmsg, "Failed to return a successful HTTP response. { command = \"%.*s\" }", st_length_int(command),
			st_char_get(command));
		client_close(client);
		return false;
	}

	// Parse the returned JSON.
	else if (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err)) || !(json_objs[1] = json_object_get_d(json_objs[0],
		"result")) || !json_is_array(json_objs[1])) {

		st_sprint(errmsg, "Failed to return a successful JSON response. { command = \"%s\", json = \"%.*s\" }", commands[6],
			st_length_int(json), st_char_get(json));
		if (json_objs[0]) json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Search for the objects describing the newly added contacts.
	else {
		for (size_t i = 0; i < json_array_size_d(json_objs[1]); i++) {
			json_objs[2] = json_array_get_d(json_objs[1], i);
			if (json_unpack_d(json_objs[2], "{s:i,s:s,s:s}", "contactID", &contact_ids[3], "name", &json_values[0], "email",
				&json_values[1]) == 0 && st_search_cs(NULLER((chr_t *)json_values[1]), rand_strs[1], NULL) &&
				st_search_cs(NULLER((chr_t*)json_values[0]), rand_strs[0], NULL)) {

				/// LOW: Figure out why st_cmp_* did not work between the name/email and their rand_strs counterparts.

				// Ensure that both of the contacts added by this test are present in the return set.
				if (contact_ids[3] == contact_ids[0]) contains_entries[0] = true;
				else if (contact_ids[3] == contact_ids[1]) contains_entries[1] = true;
			}
		}
		if (!contains_entries[0] || !contains_entries[1]) {

			st_sprint(errmsg, "Failed to find contact entry in response. { command = \"%s\", json = \"%.*s\", contact_id = %u, copy_id = %u}",
				commands[9], st_length_int(json), st_char_get(json), contact_ids[0], contact_ids[1]);
			json_decref_d(json_objs[0]);
			client_close(client);
			return false;
		}
	}

	// Clean up before the next check.
	st_wipe(command);

	if (json_objs[0]) json_decref_d(json_objs[0]);

	for (size_t i = 0; i < 2; i++) json_values[i] = NULL;

	contains_entries[0] = false;
	contains_entries[1] = false;

	json_objs[0] = NULL;

	client_close(client);
	client = NULL;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Test config.edit 	: commands[10]
	// JSON Command			: {"id":12,"method":"contacts.edit","params":{"folderID":<folder_ids[0]>, "contactID":<contact_ids[0]>,
	//							"contact":{"name":"<rand_strs[0]>","email":"rand_strs[1]"}}}
	// Expected Response 	: {"jsonrpc":"2.0","result":{"contacts.edit":"success"},"id":12}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Generate the new random inputs for "name" and "email".
	if (!rand_choices(choices, 64, rand_strs[0]) || !rand_choices(choices, 64, rand_strs[1])) {

		st_sprint(errmsg, "Failed to create random inputs. { command = \"%s\" }", commands[10]);
		client_close(client);
		return false;
	}

	// Construct the command string.
	else if (!(st_sprint(command, commands[10], folder_ids[0], contact_ids[0], st_length_int(rand_strs[0]), st_char_get(rand_strs[0]),
		st_length_int(rand_strs[1]), st_char_get(rand_strs[1])))) {

		st_sprint(errmsg, "Failed to create command string. { command = \"%.*s\" }", st_length_int(command), st_char_get(command));
		client_close(client);
		return false;
	}

	// Submit the command and check the status of the response.
	else if (!check_camel_print(command, cookie, json, secure)) {

		st_sprint(errmsg, "Failed to return a successful HTTP response. { command = \"%.*s\" }", st_length_int(command),
			st_char_get(command));
		client_close(client);
		return false;
	}

	// Parse the returned JSON.
	else if (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err)) || json_unpack_d(json_objs[0], "{s:{s:s}}", "result",
		"contacts.edit", &json_values[0]) != 0) {

		st_sprint(errmsg, "Failed parsing the returned JSON. { command = \"%.*s\", json = \"%.*s\", value = \"%s\"}",
			st_length_int(command), st_char_get(command), st_length_int(json), st_char_get(json), json_values[0]);
		if (json_objs[0]) json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Check if the returned JSON is correct.
	else if (st_cmp_cs_eq(NULLER((chr_t *)json_values[0]), PLACER("success", 7))) {

		st_sprint(errmsg, "Failed parsing the returned JSON. { command = \"%.*s\", json = \"%.*s\" }", st_length_int(command),
			st_char_get(command), st_length_int(json), st_char_get(json));
		json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Clean up before the next check.
	st_wipe(command);

	if (json_objs[0]) json_decref_d(json_objs[0]);

	json_objs[0] = NULL;
	json_values[0] = NULL;

	client_close(client);
	client = NULL;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Test config.edit 	: commands[11]
	// JSON Command			: {"id":13,"method\":"contacts.list","params":{"folderID":<folder_ids[0]>, "contactID":<contact_ids[0]>}}
	// Expected Response 	: {"jsonrpc":"2.0","result":{"contactID":<contact_ids[0]>, "name":{"value":<rand_strs[0]>, ...},
	//							"email":{"value":<rand_strs[1]>}}, "id":13}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Construct the command string.
	if (!(st_sprint(command, commands[11], folder_ids[0], contact_ids[0]))) {

		st_sprint(errmsg, "Failed to create command string. { command = \"%.*s\" }", st_length_int(command), st_char_get(command));
		client_close(client);
		return false;
	}

	// Submit the command and check the status of the response.
	else if (!check_camel_print(command, cookie, json, secure)) {

		st_sprint(errmsg, "Failed to return a successful HTTP response. { command = \"%.*s\" }", st_length_int(command),
			st_char_get(command));
		client_close(client);
		return false;
	}

	// Parse the returned JSON.
	else if (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err)) || json_unpack_d(json_objs[0], "{s:{s:i,s:{s:s},s:{s:s}}}",
		"result", "contactID", &contact_ids[2], "name", "value", &json_values[0], "email", "value", &json_values[1]) != 0) {

		st_sprint(errmsg, "Failed parsing the returned JSON. { command = \"%.*s\", json = \"%.*s\"}",
			st_length_int(command), st_char_get(command), st_length_int(json), st_char_get(json));
		if (json_objs[0]) json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Check if the returned JSON is correct.
	else if (st_cmp_cs_eq(NULLER((chr_t *)json_values[0]), rand_strs[0]) != 0 ||
			st_cmp_cs_eq(NULLER((chr_t *)json_values[1]), rand_strs[1]) != 0 ||
			contact_ids[2] != contact_ids[0]) {

		st_sprint(errmsg, "Failed parsing the returned JSON. { command = \"%.*s\", json = \"%.*s\" }", st_length_int(command),
			st_char_get(command), st_length_int(json), st_char_get(json));
		json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Clean up before the next check.
	st_wipe(command);
	st_wipe(rand_strs[0]);
	st_wipe(rand_strs[1]);

	if (json_objs[0]) json_decref_d(json_objs[0]);

	json_objs[0] = NULL;
	for (size_t i = 0; i < 2; i++) json_values[i] = NULL;

	client_close(client);
	client = NULL;

	/// LOW: This test triggers a bug in the contacts code. "Invalid String Options" at quit.

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Test config.edit 	: commands[12]
	// JSON Command			: {"id":14,"method":"contacts.edit","params":{"folderID":<folder_ids[0]>,"contactID":contact_ids[1],
	//							"contact":{"name":<rand_strs[0]>, "email":rand_strs[1],"phone":rand_strs[2],"notes":rand_strs[3}}}
	// Expected Response 	: {"jsonrpc":"2.0","result":{"contacts.edit":"success"},"id":14}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Generate the new random inputs for "name", "email", "phone", and "notes".
	if (!rand_choices(choices, 64, rand_strs[0]) || !rand_choices(choices, 64, rand_strs[1]) ||
		!rand_choices(choices, 64, rand_strs[2]) || !rand_choices(choices, 64, rand_strs[3])) {

		st_sprint(errmsg, "Failed to create random inputs. { command = \"%s\" }", commands[12]);
		client_close(client);
		return false;
	}

	// Construct the command string.
	else if (!(st_sprint(command, commands[12], folder_ids[0], contact_ids[1], st_length_int(rand_strs[0]), st_char_get(rand_strs[0]),
		st_length_int(rand_strs[1]), st_char_get(rand_strs[1]), st_length_int(rand_strs[2]), st_char_get(rand_strs[2]),
		st_length_int(rand_strs[3]), st_char_get(rand_strs[3])))) {

		st_sprint(errmsg, "Failed to create command string. { command = \"%.*s\" }", st_length_int(command), st_char_get(command));
		client_close(client);
		return false;
	}

	// Submit the command and check the status of the response.
	else if (!check_camel_print(command, cookie, json, secure)) {

		st_sprint(errmsg, "Failed to return a successful HTTP response. { command = \"%.*s\" }", st_length_int(command),
			st_char_get(command));
		client_close(client);
		return false;
	}

	// Parse the returned JSON.
	else if (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err)) || json_unpack_d(json_objs[0], "{s:{s:s}}", "result",
		"contacts.edit", &json_values[0]) != 0) {

		st_sprint(errmsg, "Failed parsing the returned JSON. { command = \"%.*s\", json = \"%.*s\", value = \"%s\"}",
			st_length_int(command), st_char_get(command), st_length_int(json), st_char_get(json), json_values[0]);
		if (json_objs[0]) json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Check if the returned JSON is correct.
	else if (st_cmp_cs_eq(NULLER((chr_t *)json_values[0]), PLACER("success", 7))) {

		st_sprint(errmsg, "Failed parsing the returned JSON. { command = \"%.*s\", json = \"%.*s\" }", st_length_int(command),
			st_char_get(command), st_length_int(json), st_char_get(json));
		json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Clean up before the next check.
	st_wipe(command);

	if (json_objs[0]) json_decref_d(json_objs[0]);

	json_objs[0] = NULL;
	json_values[0] = NULL;

	client_close(client);
	client = NULL;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Test config.edit 	: commands[13]
	// JSON Command			: {"id":15,"method":"contacts.load","params":{"folderID":<folder_ids[0]>, "contactID":<contact_ids[1]>}}
	// Expected Response 	: {"jsonrpc":"2.0","result":{"contactID":<contact_ids[1]>, "name":{"value":<rand_strs[0]>, ...},
	//							"email":{"value":<rand_strs[1]>, ...}, ... (for phone and notes)...}, "id":15}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Construct the command string.
	if (!(st_sprint(command, commands[13], folder_ids[0], contact_ids[1]))) {

		st_sprint(errmsg, "Failed to create command string. { command = \"%s\" }", commands[13]);
		client_close(client);
		return false;
	}

	// Submit the command and check the status of the response.
	else if (!check_camel_print(command, cookie, json, secure)) {

		st_sprint(errmsg, "Failed to return a successful HTTP response. { command = \"%.*s\" }", st_length_int(command),
			st_char_get(command));
		client_close(client);
		return false;
	}

	// Parse the returned JSON.
	else if (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err)) ||
		json_unpack_d(json_objs[0], "{s:{s:i,s:{s:s},s:{s:s},s:{s:s},s:{s:s}}}", "result", "contactID", &contact_ids[2],
		"name", "value", &json_values[0], "email", "value", &json_values[1], "phone", "value", &json_values[2], "notes",
		"value", &json_values[3]) != 0) {

		st_sprint(errmsg, "Failed parsing the returned JSON. { command = \"%.*s\", json = \"%.*s\"}",
			st_length_int(command), st_char_get(command), st_length_int(json), st_char_get(json));
		if (json_objs[0]) json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Check if the returned JSON is correct.
	else if (st_cmp_cs_eq(NULLER((chr_t *)json_values[0]), rand_strs[0]) != 0 ||
			st_cmp_cs_eq(NULLER((chr_t *)json_values[1]), rand_strs[1]) != 0 ||
			st_cmp_cs_eq(NULLER((chr_t *)json_values[2]), rand_strs[2]) != 0 ||
			st_cmp_cs_eq(NULLER((chr_t *)json_values[3]), rand_strs[3]) != 0 ||
			contact_ids[2] != contact_ids[1]) {

		st_sprint(errmsg, "Failed parsing the returned JSON. { command = \"%.*s\", json = \"%.*s\" }", st_length_int(command),
			st_char_get(command), st_length_int(json), st_char_get(json));
		json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Clean up before the next check.
	st_wipe(command);

	if (json_objs[0]) json_decref_d(json_objs[0]);

	json_objs[0] = NULL;

	for (size_t i = 0; i < 4; i++) json_values[i] = NULL;
	for (size_t i = 0; i < 4; i++) st_wipe(rand_strs[i]);

	client_close(client);
	client = NULL;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Test config.edit 	: commands[14]
	// JSON Command			: {"id":16,"method":"folders.add","params":{"context":"contacts","name:<rand_strs[0]>}}"
	// Expected Response 	: {"jsonrpc":"2.0","result":{"folderID":<folder_ids[2]>},"id":16}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Generate the random input for "name".
	if (!rand_choices(choices, 16, rand_strs[0])) {

		st_sprint(errmsg, "Failed to create random input. { command = \"%s\" }", commands[14]);
		client_close(client);
		return false;
	}

	// Construct the command string.
	else if (!(st_sprint(command, commands[5], st_length_int(rand_strs[0]), st_char_get(rand_strs[0])))) {

		st_sprint(errmsg, "Failed to create command string. { command = \"%s\", \"name\" = \"%.*s\"}", commands[14],
			st_length_int(rand_strs[0]), st_char_get(rand_strs[0]));
		client_close(client);
		return false;
	}

	// Submit the command and check the status of the response.
	else if (!check_camel_print(command, cookie, json, secure)) {

		st_sprint(errmsg, "Failed to return a successful HTTP response. { command = \"%.*s\" }", st_length_int(command),
			st_char_get(command));
		client_close(client);
		return false;
	}

	// Parse the returned JSON.
	else if (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err)) || (json_unpack_d(json_objs[0], "{s:{s:i}}", "result",
		"folderID", &folder_ids[2]) != 0)) {

		st_sprint(errmsg, "Failed to return a successful JSON response. { command = \"%.*s\", json = \"%.*s\" }", st_length_int(command),
			st_char_get(command), st_length_int(json), st_char_get(json));
		if (json_objs[0]) json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Clean up before the next check.
	st_wipe(command);

	if (json_objs[0]) json_decref_d(json_objs[0]);

	json_objs[0] = NULL;

	client_close(client);
	client = NULL;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Test config.edit 	: commands[15]
	// JSON Command			: {"id":17,"method":"contacts.move","params":{"contactID":contact_ids[0], "sourceFolderID":<folder_ids[0]>,
	//							"targetFolderID":<folder_ids[2]}}
	// Expected Response 	: {"jsonrpc":"2.0","result":{"contactID":<contact_ids[1]>},"id":15}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Construct the command string.
	if (!(st_sprint(command, commands[15], contact_ids[0], folder_ids[0], folder_ids[2]))) {

		st_sprint(errmsg, "Failed to create command string. { command = \"%s\"}", commands[15]);
		client_close(client);
		return false;
	}

	// Submit the command and check the status of the response.
	else if (!check_camel_print(command, cookie, json, secure)) {

		st_sprint(errmsg, "Failed to return a successful HTTP response. { command = \"%.*s\" }", st_length_int(command),
			st_char_get(command));
		client_close(client);
		return false;
	}

	// Parse the returned JSON.
	else if (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err)) || json_unpack_d(json_objs[0], "{s:{s:s}}", "result",
		"contacts.move", &json_values[0]) != 0) {

		st_sprint(errmsg, "Failed parsing the returned JSON. { command = \"%.*s\", json = \"%.*s\" }", st_length_int(command),
			st_char_get(command), st_length_int(json), st_char_get(json));
		if (json_objs[0]) json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Check if the returned JSON is correct.
	else if (st_cmp_cs_eq(NULLER((chr_t *)json_values[0]), PLACER("success", 7))) {

		st_sprint(errmsg, "Failed parsing the returned JSON. { command = \"%.*s\", json = \"%.*s\" }", st_length_int(command),
			st_char_get(command), st_length_int(json), st_char_get(json));
		json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Clean up before the next check.
	st_wipe(command);

	if (json_objs[0]) json_decref_d(json_objs[0]);

	json_objs[0] = NULL;
	json_values[0] = NULL;

	client_close(client);
	client = NULL;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Test config.edit 	: commands[16]
	// JSON Command			: {"id":18,"method\":"contacts.list","params":{"folderID":<folder_ids[0]>}}
	// Expected Response 	: {"jsonrpc":"2.0","result":{ { no object matching contact_ids[0] }, ...}, "id":18}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Construct the command string.
	if (!(st_sprint(command, commands[16], folder_ids[0]))) {

		st_sprint(errmsg, "Failed to create command string. { command = \"%.*s\" }", st_length_int(command), st_char_get(command));
		client_close(client);
		return false;
	}

	// Submit the command and check the status of the response.
	else if (!check_camel_print(command, cookie, json, secure)) {

		st_sprint(errmsg, "Failed to return a successful HTTP response. { command = \"%.*s\" }", st_length_int(command),
			st_char_get(command));
		client_close(client);
		return false;
	}

	// Parse the returned JSON.
	else if (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err)) || !(json_objs[1] = json_object_get_d(json_objs[0],
		"result")) || !json_is_array(json_objs[1])) {

		st_sprint(errmsg, "Failed to return a successful JSON response. { command = \"%s\", json = \"%.*s\" }", commands[16],
			st_length_int(json), st_char_get(json));
		if (json_objs[0]) json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}
	else {
		// Search for the object describing the newly added folder.
		for (size_t i = 0; !contains_entries[0] && i < json_array_size_d(json_objs[1]); i++) {
			json_objs[2] = json_array_get_d(json_objs[1], i);
			if (json_unpack_d(json_objs[2], "{s:i}", "contactID", &contact_ids[2]) == 0 && contact_ids[0] == contact_ids[2]) {

				contains_entries[0] = true;
			}
		}
		if (contains_entries[0]) {

			st_sprint(errmsg, "The contact was not moved between folders. { command = \"%s\", json = \"%.*s\", contact_ids[0] = <%u> }",
				commands[16], st_length_int(json), st_char_get(json), contact_ids[0]);
			json_decref_d(json_objs[0]);
			client_close(client);
			return false;
		}
	}

	// Clean up before the next check.
	st_wipe(command);
	st_wipe(rand_strs[0]);
	st_wipe(rand_strs[1]);

	if (json_objs[0]) json_decref_d(json_objs[0]);

	json_objs[0] = NULL;
	for (size_t i = 0; i < 2; i++) json_values[i] = NULL;

	client_close(client);
	client = NULL;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Test config.edit 	: commands[17]
	// JSON Command			: {"id":19,"method\":"contacts.list","params":{"folderID":<folder_ids[2]>}}
	// Expected Response 	: {"jsonrpc":"2.0","result":{ { an object matching contact_ids[0] }, ...}, "id":19}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Construct the command string.
	if (!(st_sprint(command, commands[17], folder_ids[2]))) {

		st_sprint(errmsg, "Failed to create command string. { command = \"%.*s\" }", st_length_int(command), st_char_get(command));
		client_close(client);
		return false;
	}

	// Submit the command and check the status of the response.
	else if (!check_camel_print(command, cookie, json, secure)) {

		st_sprint(errmsg, "Failed to return a successful HTTP response. { command = \"%.*s\" }", st_length_int(command),
			st_char_get(command));
		client_close(client);
		return false;
	}

	// Parse the returned JSON.
	else if (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err)) || !(json_objs[1] = json_object_get_d(json_objs[0],
		"result")) || !json_is_array(json_objs[1])) {

		st_sprint(errmsg, "Failed to return a successful JSON response. { command = \"%s\", json = \"%.*s\" }", commands[16],
			st_length_int(json), st_char_get(json));
		if (json_objs[0]) json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}
	else {
		// Search for the object describing the newly added folder.
		for (size_t i = 0; !contains_entries[0] && i < json_array_size_d(json_objs[1]); i++) {
			json_objs[2] = json_array_get_d(json_objs[1], i);
			if (json_unpack_d(json_objs[2], "{s:i}", "contactID", &contact_ids[2]) == 0 && contact_ids[0] == contact_ids[2]) {

				contains_entries[0] = true;
			}
		}
		if (!contains_entries[0]) {

			st_sprint(errmsg, "Failed to find the moved contact. { command = \"%s\", json = \"%.*s\", contact_ids[0] = <%u> }",
				commands[16], st_length_int(json), st_char_get(json), contact_ids[0]);
			json_decref_d(json_objs[0]);
			client_close(client);
			return false;
		}
	}

	// Clean up before the next check.
	st_wipe(command);
	st_wipe(rand_strs[0]);
	st_wipe(rand_strs[1]);

	contains_entries[0] = false;

	if (json_objs[0]) json_decref_d(json_objs[0]);

	json_objs[0] = NULL;
	for (size_t i = 0; i < 2; i++) json_values[i] = NULL;

	client_close(client);
	client = NULL;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Test config.edit 	: commands[18]
	// JSON Command			: {"id":20,"method":"contacts.remove","params":{"folderID":<folder_ids[2]>, "contactID":<contact_ids[0]>}}
	// Expected Response 	: {"jsonrpc":"2.0","result":{"contacts.remove":"success"},"id":19}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Construct the command string.
	if (!(st_sprint(command, commands[18], folder_ids[2], contact_ids[0]))) {

		st_sprint(errmsg, "Failed to create command string. { command = \"%s\"}", commands[15]);
		client_close(client);
		return false;
	}

	// Submit the command and check the status of the response.
	else if (!check_camel_print(command, cookie, json, secure)) {

		st_sprint(errmsg, "Failed to return a successful HTTP response. { command = \"%.*s\" }", st_length_int(command),
			st_char_get(command));
		client_close(client);
		return false;
	}

	// Parse the returned JSON.
	else if (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err)) || json_unpack_d(json_objs[0], "{s:{s:s}}", "result",
		"contacts.remove", &json_values[0]) != 0) {

		st_sprint(errmsg, "Failed parsing the returned JSON. { command = \"%.*s\", json = \"%.*s\" }", st_length_int(command),
			st_char_get(command), st_length_int(json), st_char_get(json));
		if (json_objs[0]) json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Check if the returned JSON is correct.
	else if (st_cmp_cs_eq(NULLER((chr_t *)json_values[0]), PLACER("success", 7))) {

		st_sprint(errmsg, "Failed parsing the returned JSON. { command = \"%.*s\", json = \"%.*s\" }", st_length_int(command),
			st_char_get(command), st_length_int(json), st_char_get(json));
		json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Clean up before the next check.
	st_wipe(command);

	if (json_objs[0]) json_decref_d(json_objs[0]);

	json_objs[0] = NULL;
	json_values[0] = NULL;

	client_close(client);
	client = NULL;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Test config.edit 	: commands[19]
	// JSON Command			: {"id":21,"method":"contacts.remove","params":{"folderID":<folder_ids[0]>, "contactID":<contact_ids[1]>}}
	// Expected Response 	: {"jsonrpc":"2.0","result":{"contacts.remove":"success"},"id":19}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Construct the command string.
	if (!(st_sprint(command, commands[19], folder_ids[0], contact_ids[1]))) {

		st_sprint(errmsg, "Failed to create command string. { command = \"%s\"}", commands[19]);
		client_close(client);
		return false;
	}

	// Submit the command and check the status of the response.
	else if (!check_camel_print(command, cookie, json, secure)) {

		st_sprint(errmsg, "Failed to return a successful HTTP response. { command = \"%.*s\" }", st_length_int(command),
			st_char_get(command));
		client_close(client);
		return false;
	}

	// Parse the returned JSON.
	else if (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err)) || json_unpack_d(json_objs[0], "{s:{s:s}}", "result",
		"contacts.remove", &json_values[0]) != 0) {

		st_sprint(errmsg, "Failed parsing the returned JSON. { command = \"%.*s\", json = \"%.*s\" }", st_length_int(command),
			st_char_get(command), st_length_int(json), st_char_get(json));
		if (json_objs[0]) json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Check if the returned JSON is correct.
	else if (st_cmp_cs_eq(NULLER((chr_t *)json_values[0]), PLACER("success", 7))) {

		st_sprint(errmsg, "Failed parsing the returned JSON. { command = \"%.*s\", json = \"%.*s\" }", st_length_int(command),
			st_char_get(command), st_length_int(json), st_char_get(json));
		json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Clean up before the next check.
	st_wipe(command);

	if (json_objs[0]) json_decref_d(json_objs[0]);

	json_objs[0] = NULL;
	json_values[0] = NULL;

	client_close(client);
	client = NULL;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Test config.edit 	: commands[20]
	// JSON Command			: {"id":22,"method":"contacts.list","params":{"folderID":<folder_ids[2]>}}
	// Expected Response 	: {"jsonrpc":"2.0","result":[{ no objects matching contact_ids[0] }, ...],"id":22}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Construct the command string.
	if (!(st_sprint(command, commands[20], folder_ids[2]))) {

		st_sprint(errmsg, "Failed to create command string. { command = \"%.*s\" }", st_length_int(command), st_char_get(command));
		client_close(client);
		return false;
	}

	// Submit the command and check the status of the response.
	else if (!check_camel_print(command, cookie, json, secure)) {

		st_sprint(errmsg, "Failed to return a successful HTTP response. { command = \"%.*s\" }", st_length_int(command),
			st_char_get(command));
		client_close(client);
		return false;
	}

	// Parse the returned JSON.
	else if (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err)) || !(json_objs[1] = json_object_get_d(json_objs[0],
		"result")) || !json_is_array(json_objs[1])) {

		st_sprint(errmsg, "Failed to return a successful JSON response. { command = \"%s\", json = \"%.*s\" }", commands[20],
			st_length_int(json), st_char_get(json));
		if (json_objs[0]) json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}
	else {
		// Search for the object describing the newly added folder.
		for (size_t i = 0; !contains_entries[0] && i < json_array_size_d(json_objs[1]); i++) {

			json_objs[2] = json_array_get_d(json_objs[1], i);
			if (json_unpack_d(json_objs[2], "{s:i}", "contactID", &contact_ids[2]) == 0 && contact_ids[0] == contact_ids[2]) {

				contains_entries[0] = true;
			}
		}
		if (contains_entries[0]) {

			st_sprint(errmsg, "Found a contact that should have been removed. { command = \"%s\", json = \"%.*s\", contact_ids[0] = <%u> }",
				commands[16], st_length_int(json), st_char_get(json), contact_ids[0]);
			json_decref_d(json_objs[0]);
			client_close(client);
			return false;
		}
	}

	// Clean up before the next check.
	st_wipe(command);

	if (json_objs[0]) json_decref_d(json_objs[0]);

	for (size_t i = 0; i < 2; i++) json_values[i] = NULL;

	contains_entries[0] = false;

	json_objs[0] = NULL;

	client_close(client);
	client = NULL;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Test config.edit 	: commands[21]
	// JSON Command			: {"id":23,"method":"folders.remove","params":{"context":"contacts","folderID":<folder_ids[2]>}}
	// Expected Response 	: {"jsonrpc":"2.0","result":{"folder.remove":"success"},"id":23}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Construct the command string.
	if (!(st_sprint(command, commands[21], folder_ids[2]))) {

		st_sprint(errmsg, "Failed to create command string. { command = \"%s\"}", commands[21]);
		client_close(client);
		return false;
	}

	// Submit the command and check the status of the response.
	else if (!check_camel_print(command, cookie, json, secure)) {

		st_sprint(errmsg, "Failed to return a successful HTTP response. { command = \"%.*s\" }", st_length_int(command),
			st_char_get(command));
		client_close(client);
		return false;
	}

	// Parse the returned JSON.
	else if (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err)) || json_unpack_d(json_objs[0], "{s:{s:s}}", "result",
		"folder.remove", &json_values[0]) != 0) {

		st_sprint(errmsg, "Failed parsing the returned JSON. { command = \"%.*s\", json = \"%.*s\" }", st_length_int(command),
			st_char_get(command), st_length_int(json), st_char_get(json));
		if (json_objs[0]) json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Check if the returned JSON is correct.
	else if (st_cmp_cs_eq(NULLER((chr_t *)json_values[0]), PLACER("success", 7))) {

		st_sprint(errmsg, "Failed parsing the returned JSON. { command = \"%.*s\", json = \"%.*s\" }", st_length_int(command),
			st_char_get(command), st_length_int(json), st_char_get(json));
		json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Clean up before the next check.
	st_wipe(command);

	if (json_objs[0]) json_decref_d(json_objs[0]);

	json_objs[0] = NULL;
	json_values[0] = NULL;

	client_close(client);
	client = NULL;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Test config.edit 	: commands[22]
	// JSON Command			: {"id":24,"method":"folders.remove","params":{"context":"contacts","folderID":<folder_ids[0]>}}",
	// Expected Response 	: {"jsonrpc":"2.0","result":{"folder.remove":"success"},"id":24}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Construct the command string.
	if (!(st_sprint(command, commands[22], folder_ids[0]))) {

		st_sprint(errmsg, "Failed to create command string. { command = \"%s\"}", commands[22]);
		client_close(client);
		return false;
	}

	// Submit the command and check the status of the response.
	else if (!check_camel_print(command, cookie, json, secure)) {

		st_sprint(errmsg, "Failed to return a successful HTTP response. { command = \"%.*s\" }", st_length_int(command),
			st_char_get(command));
		client_close(client);
		return false;
	}

	/// LOW: Why is the key "folder.remove" (singular) in the response, but "folders.remove" (plural) in the request?

	// Parse the returned JSON.
	else if (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err)) || json_unpack_d(json_objs[0], "{s:{s:s}}", "result",
		"folder.remove", &json_values[0]) != 0) {

		st_sprint(errmsg, "Failed parsing the returned JSON. { command = \"%.*s\", json = \"%.*s\" }", st_length_int(command),
			st_char_get(command), st_length_int(json), st_char_get(json));
		if (json_objs[0]) json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Check if the returned JSON is correct.
	else if (st_cmp_cs_eq(NULLER((chr_t *)json_values[0]), PLACER("success", 7))) {

		st_sprint(errmsg, "Failed parsing the returned JSON. { command = \"%.*s\", json = \"%.*s\" }", st_length_int(command),
			st_char_get(command), st_length_int(json), st_char_get(json));
		json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Clean up before the next check.
	st_wipe(command);

	if (json_objs[0]) json_decref_d(json_objs[0]);

	json_objs[0] = NULL;
	json_values[0] = NULL;

	client_close(client);
	client = NULL;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Test config.edit 	: commands[23]
	// JSON Command			: {"id":25,"method":"cookies"}
	// Expected Response 	: {"jsonrpc":"2.0","result":{"cookies":"enabled"},"id":25}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Submit the command and check the status of the response.
	if (!check_camel_print(PLACER(commands[23], ns_length_get(commands[23])), cookie, json, secure)) {

		st_sprint(errmsg, "Failed to return a successful HTTP response. { command = \"%.*s\" }", st_length_int(command),
			st_char_get(command));
		client_close(client);
		return false;
	}

	// Parse the returned JSON.
	else if (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err)) || json_unpack_d(json_objs[0], "{s:{s:s}}", "result",
		"cookies", &json_values[0]) != 0) {

		st_sprint(errmsg, "Failed parsing the returned JSON. { command = \"%.*s\", json = \"%.*s\" }", st_length_int(command),
			st_char_get(command), st_length_int(json), st_char_get(json));
		if (json_objs[0]) json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Check if the returned JSON is correct.
	else if (st_cmp_cs_eq(NULLER((chr_t *)json_values[0]), PLACER("enabled", 7))) {

		st_sprint(errmsg, "Failed parsing the returned JSON. { command = \"%.*s\", json = \"%.*s\" }", st_length_int(command),
			st_char_get(command), st_length_int(json), st_char_get(json));
		json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Clean up before the next check.
	st_wipe(command);

	if (json_objs[0]) json_decref_d(json_objs[0]);

	json_objs[0] = NULL;
	json_values[0] = NULL;

	client_close(client);
	client = NULL;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Test config.edit 	: commands[24]
	// JSON Command			: {"id":26,"method":"alert.list"}
	// Expected Response 	: {"jsonrpc":"2.0","result":{"cookies":"enabled"},"id":26}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Submit the command and check the status of the response.
	if (!check_camel_print(PLACER(commands[24], ns_length_get(commands[24])), cookie, json, secure)) {

		st_sprint(errmsg, "Failed to return a successful HTTP response. { command = \"%.*s\" }", st_length_int(command),
			st_char_get(command));
		client_close(client);
		return false;
	}

	// Parse the returned JSON.
	else if (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err)) || !(json_objs[1] = json_object_get_d(json_objs[0],
		"result")) || !json_is_array(json_objs[1])) {

		st_sprint(errmsg, "Failed to return a successful JSON response. { command = \"%s\", json = \"%.*s\" }", commands[24],
			st_length_int(json), st_char_get(json));
		if (json_objs[0]) json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Check if there is an alert message. If there is, save the id and use it in the next check.
	else if (json_array_size_d(json_objs[1]) > 0 && (!(json_objs[2] = json_array_get_d(json_objs[1], 0)) ||
		json_unpack_d(json_objs[2], "{s:i}", "alertID", &alert_ids[0]) != 0)) {

		st_sprint(errmsg, "Unable to parse the alertID. { command = \"%s\", json = \"%.*s\"}", commands[24],
			st_length_int(json), st_char_get(json));
		json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Clean up before the next check.
	st_wipe(command);

	if (json_objs[0]) json_decref_d(json_objs[0]);

	json_objs[0] = NULL;
	json_values[0] = NULL;

	client_close(client);
	client = NULL;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Test config.edit 	: commands[25]
	// JSON Command			: {"id":27,"method":"alert.acknowledge","params":[<alert_ids[0]>]}
	// Expected Response 	: {"jsonrpc":"2.0","result":{"alert.acknowledge":"success"},"id":24}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Construct the command string.
	if (!(st_sprint(command, commands[25], alert_ids[0]))) {

		st_sprint(errmsg, "Failed to create command string. { command = \"%s\"}", commands[25]);
		client_close(client);
		return false;
	}

	// Submit the command and check the status of the response.
	else if (!check_camel_print(command, cookie, json, secure)) {

		st_sprint(errmsg, "Failed to return a successful HTTP response. { command = \"%.*s\" }", st_length_int(command),
			st_char_get(command));
		client_close(client);
		return false;
	}

	/// LOW: Why is the key "folder.remove" (singular) in the response, but "folders.remove" (plural) in the request?

	// Parse the returned JSON.
	else if (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err)) || json_unpack_d(json_objs[0], "{s:{s:s}}", "result",
		"alert.acknowledge", &json_values[0]) != 0) {

		st_sprint(errmsg, "Failed parsing the returned JSON. { command = \"%.*s\", json = \"%.*s\" }", st_length_int(command),
			st_char_get(command), st_length_int(json), st_char_get(json));
		if (json_objs[0]) json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	/// LOW: The alert.acknowledge returns success when IDs for non-existent alerts are passed in.

	// Check if the returned JSON is correct.
	else if (st_cmp_cs_eq(NULLER((chr_t *)json_values[0]), PLACER("success", 7))) {

		st_sprint(errmsg, "Failed parsing the returned JSON. { command = \"%.*s\", json = \"%.*s\" }", st_length_int(command),
			st_char_get(command), st_length_int(json), st_char_get(json));
		json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Clean up before the next check.
	st_wipe(command);

	if (json_objs[0]) json_decref_d(json_objs[0]);

	json_objs[0] = NULL;
	json_values[0] = NULL;

	client_close(client);
	client = NULL;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Test config.edit 	: commands[26]
	// JSON Command			: {"id":28,"method":"alert.list"}
	// Expected Response 	: {"jsonrpc":"2.0","result":[ {if alert_ids[0] was set previously, expect to not find a matching alert},
	//							...], "id":26}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Submit the command and check the status of the response.
	if (!check_camel_print(PLACER(commands[26], ns_length_get(commands[26])), cookie, json, secure)) {

		st_sprint(errmsg, "Failed to return a successful HTTP response. { command = \"%.*s\" }", st_length_int(command),
			st_char_get(command));
		client_close(client);
		return false;
	}

	// Parse the returned JSON.
	else if (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err)) || !(json_objs[1] = json_object_get_d(json_objs[0],
		"result")) || !json_is_array(json_objs[1])) {

		st_sprint(errmsg, "Failed to return a successful JSON response. { command = \"%s\", json = \"%.*s\" }", commands[20],
			st_length_int(json), st_char_get(json));
		if (json_objs[0]) json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}
	else {
		// Search for the object describing the newly added folder.
		for (size_t i = 0; !contains_entries[0] && i < json_array_size_d(json_objs[1]); i++) {

			json_objs[2] = json_array_get_d(json_objs[1], i);
			if (json_unpack_d(json_objs[2], "{s:i}", "alertID", &alert_ids[1]) == 0 && alert_ids[0] == alert_ids[0]) {

				contains_entries[0] = true;
			}
		}
		if (contains_entries[0]) {

			st_sprint(errmsg, "Found an alert that should have been ack. { command = \"%s\", json = \"%.*s\", contact_ids[0] = <%u> }",
				commands[16], st_length_int(json), st_char_get(json), contact_ids[0]);
			json_decref_d(json_objs[0]);
			client_close(client);
			return false;
		}
	}

	// Clean up before the next check.
	st_wipe(command);

	if (json_objs[0]) json_decref_d(json_objs[0]);

	contains_entries[0] = false;

	json_objs[0] = NULL;
	json_values[0] = NULL;

	client_close(client);
	client = NULL;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Test config.edit 	: commands[27]
	// JSON Command			: "{\"id\":29,\"method\":\"folders.list\",\"params\":{\"context\":\"mail\"}}",
	// Expected Response 	: {"jsonrpc":"2.0","result"{ ... },"id":29}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Submit the command and check the status of the response.
	if (!check_camel_print(PLACER(commands[27], ns_length_get(commands[27])), cookie, json, secure)) {

		st_sprint(errmsg, "Failed to return a successful HTTP response. { command = \"%s\" }", commands[27]);
		client_close(client);
		return false;
	}

	// Parse the returned JSON.
	else if (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err)) || !(json_objs[1] = json_object_get_d(json_objs[0],
		"result")) || !json_is_array(json_objs[1])) {

		st_sprint(errmsg, "Failed to return a successful JSON response. { command = \"%s\", json = \"%.*s\" }", commands[27],
			st_length_int(json), st_char_get(json));
		if (json_objs[0]) json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Clean up before the next check.
	st_wipe(command);

	if (json_objs[0]) json_decref_d(json_objs[0]);

	json_objs[0] = NULL;
	json_values[0] = NULL;

	client_close(client);
	client = NULL;

	/// LOW: The following two checks to not work. When submitting "settings" or "help" as the "context" value
	///			for the method "folders.list", an error is thrown and the message "Context not supported" is
	///			returned.

	/*

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Test config.edit 	: commands[28]
	// JSON Command			: "{\"id\":30,\"method\":\"folders.list\",\"params\":{\"context\":\"settings\"}}",
	// Expected Response 	: {"jsonrpc":"2.0","result"{ ... },"id":30}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Submit the command and check the status of the response.
	if (!check_camel_print(PLACER(commands[28], ns_length_get(commands[28])), cookie, json, secure)) {

		st_sprint(errmsg, "Failed to return a successful HTTP response. { command = \"%s\" }", commands[28]);
		client_close(client);
		return false;
	}

	// Parse the returned JSON.
	else if (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err)) || !(json_objs[1] = json_object_get_d(json_objs[0],
		"result")) || !json_is_array(json_objs[1])) {

		st_sprint(errmsg, "Failed to return a successful JSON response. { command = \"%s\", json = \"%.*s\" }", commands[28],
			st_length_int(json), st_char_get(json));
		if (json_objs[0]) json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Clean up before the next check.
	st_wipe(command);

	if (json_objs[0]) json_decref_d(json_objs[0]);

	json_objs[0] = NULL;
	json_values[0] = NULL;

	client_close(client);
	client = NULL;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Test config.edit 	: commands[29]
	// JSON Command			: "{\"id\":31,\"method\":\"folders.list\",\"params\":{\"context\":\"help\"}}",
	// Expected Response 	: {"jsonrpc":"2.0","result"{ ... },"id":31}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Submit the command and check the status of the response.
	if (!check_camel_print(PLACER(commands[29], ns_length_get(commands[29])), cookie, json, secure)) {

		st_sprint(errmsg, "Failed to return a successful HTTP response. { command = \"%s\" }", commands[29]);
		client_close(client);
		return false;
	}

	// Parse the returned JSON.
	else if (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err)) || !(json_objs[1] = json_object_get_d(json_objs[0],
		"result")) || !json_is_array(json_objs[1])) {

		st_sprint(errmsg, "Failed to return a successful JSON response. { command = \"%s\", json = \"%.*s\" }", commands[29],
			st_length_int(json), st_char_get(json));
		if (json_objs[0]) json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Clean up before the next check.
	st_wipe(command);

	if (json_objs[0]) json_decref_d(json_objs[0]);

	json_objs[0] = NULL;
	json_values[0] = NULL;

	client_close(client);
	client = NULL;

	*/

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Test config.edit 	: commands[30]
	// JSON Command			: {"id":32,"method":"folders.add","params":{"context":"mail","name":"%.*s"}}",
	// Expected Response 	: {"jsonrpc":"2.0","result":{"folderID":<folder_ids[0]>},"id":32}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Generate the random input for "name".
	if (!rand_choices(choices, 16, rand_strs[0])) {

		st_sprint(errmsg, "Failed to create random input. { command = \"%s\" }", commands[30]);
		client_close(client);
		return false;
	}

	// Construct the command string.
	else if (!(st_sprint(command, commands[30], st_length_int(rand_strs[0]), st_char_get(rand_strs[0])))) {

		st_sprint(errmsg, "Failed to create command string. { command = \"%s\", \"name\" = \"%.*s\"}", commands[30],
			st_length_int(rand_strs[0]), st_char_get(rand_strs[0]));
		client_close(client);
		return false;
	}

	// Submit the command and check the status of the response.
	else if (!check_camel_print(command, cookie, json, secure)) {

		st_sprint(errmsg, "Failed to return a successful HTTP response. { command = \"%.*s\" }", st_length_int(command),
			st_char_get(command));
		client_close(client);
		return false;
	}

	// Parse the returned JSON.
	else if (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err)) || (json_unpack_d(json_objs[0], "{s:{s:i}}", "result",
		"folderID", &folder_ids[0]) != 0)) {

		st_sprint(errmsg, "Failed to return a successful JSON response. { command = \"%.*s\", json = \"%.*s\" }", st_length_int(command),
			st_char_get(command), st_length_int(json), st_char_get(json));
		if (json_objs[0]) json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Clean up before the next check.
	st_wipe(command);

	if (json_objs[0]) json_decref_d(json_objs[0]);

	json_objs[0] = NULL;

	client_close(client);
	client = NULL;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Test config.edit 	: commands[31]
	// JSON Command			: {"id":33,"method":"folders.add","params":{"context":"mail","parentID":<folder_ids[0]>,
	//							"name":<rand_strs[0]>}}
	// Expected Response 	: {"jsonrpc":"2.0","result":{"folderID":<folder_ids[1]>},"id":33}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Generate the random input for "name".
	if (!rand_choices(choices, 16, rand_strs[0])) {

		st_sprint(errmsg, "Failed to create random input. { command = \"%s\" }", commands[31]);
		client_close(client);
		return false;
	}

	// Construct the command string.
	else if (!(st_sprint(command, commands[31], folder_ids[0], st_length_int(rand_strs[0]), st_char_get(rand_strs[0])))) {

		st_sprint(errmsg, "Failed to create command string. { command = \"%s\", \"name\" = \"%.*s\"}", commands[31],
			st_length_int(rand_strs[0]), st_char_get(rand_strs[0]));
		client_close(client);
		return false;
	}

	// Submit the command and check the status of the response.
	else if (!check_camel_print(command, cookie, json, secure)) {

		st_sprint(errmsg, "Failed to return a successful HTTP response. { command = \"%.*s\" }", st_length_int(command),
			st_char_get(command));
		client_close(client);
		return false;
	}

	// Parse the returned JSON.
	else if (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err)) || (json_unpack_d(json_objs[0], "{s:{s:i}}", "result",
		"folderID", &folder_ids[1]) != 0)) {

		st_sprint(errmsg, "Failed to return a successful JSON response. { command = \"%.*s\", json = \"%.*s\" }", st_length_int(command),
			st_char_get(command), st_length_int(json), st_char_get(json));
		if (json_objs[0]) json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Clean up before the next check.
	st_wipe(command);

	if (json_objs[0]) json_decref_d(json_objs[0]);

	json_objs[0] = NULL;

	client_close(client);
	client = NULL;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Test config.edit 	: commands[32]
	// JSON Command			: {"id":34,"method":"folders.add","params":{"context":"mail","parentID":<folder_ids[1]>,
	//							"name":<rand_strs[0]>}}
	// Expected Response 	: {"jsonrpc":"2.0","result":{"folderID":<folder_ids[2]>},"id":34}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Generate the random input for "name".
	if (!rand_choices(choices, 16, rand_strs[0])) {

		st_sprint(errmsg, "Failed to create random input. { command = \"%s\" }", commands[32]);
		client_close(client);
		return false;
	}

	// Construct the command string.
	else if (!(st_sprint(command, commands[32], folder_ids[1], st_length_int(rand_strs[0]), st_char_get(rand_strs[0])))) {

		st_sprint(errmsg, "Failed to create command string. { command = \"%s\", \"name\" = \"%.*s\"}", commands[32],
			st_length_int(rand_strs[0]), st_char_get(rand_strs[0]));
		client_close(client);
		return false;
	}

	// Submit the command and check the status of the response.
	else if (!check_camel_print(command, cookie, json, secure)) {

		st_sprint(errmsg, "Failed to return a successful HTTP response. { command = \"%.*s\" }", st_length_int(command),
			st_char_get(command));
		client_close(client);
		return false;
	}

	// Parse the returned JSON.
	else if (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err)) || (json_unpack_d(json_objs[0], "{s:{s:i}}", "result",
		"folderID", &folder_ids[2]) != 0)) {

		st_sprint(errmsg, "Failed to return a successful JSON response. { command = \"%.*s\", json = \"%.*s\" }", st_length_int(command),
			st_char_get(command), st_length_int(json), st_char_get(json));
		if (json_objs[0]) json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Clean up before the next check.
	st_wipe(command);

	if (json_objs[0]) json_decref_d(json_objs[0]);

	json_objs[0] = NULL;

	client_close(client);
	client = NULL;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Test config.edit 	: commands[33]
	// JSON Command			: {"id":35,"method":"folders.rename","params":{"context":"mail","folderID": <folder_ids[0]>,
	//							"name":<rand_strs[0]>}}
	// Expected Response 	: {"jsonrpc":"2.0","result":{"folder.rename":"success"},"id":35}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Generate the random input for "name".
	if (!rand_choices(choices, 16, rand_strs[0])) {

		st_sprint(errmsg, "Failed to create random input. { command = \"%s\" }", commands[33]);
		client_close(client);
		return false;
	}

	// Construct the command string.
	else if (!(st_sprint(command, commands[33], folder_ids[0], st_length_int(rand_strs[0]), st_char_get(rand_strs[0])))) {

		st_sprint(errmsg, "Failed to create command string. { command = \"%s\", \"name\" = \"%.*s\"}", commands[33],
			st_length_int(rand_strs[0]), st_char_get(rand_strs[0]));
		client_close(client);
		return false;
	}

	// Submit the command and check the status of the response.
	else if (!check_camel_print(command, cookie, json, secure)) {

		st_sprint(errmsg, "Failed to return a successful HTTP response. { command = \"%.*s\" }", st_length_int(command),
			st_char_get(command));
		client_close(client);
		return false;
	}

	// Parse the returned JSON.
	else if (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err)) || json_unpack_d(json_objs[0], "{s:{s:s}}", "result",
		"folder.rename", &json_values[0]) != 0) {

		st_sprint(errmsg, "Failed parsing the returned JSON. { command = \"%.*s\", json = \"%.*s\" }", st_length_int(command),
			st_char_get(command), st_length_int(json), st_char_get(json));
		if (json_objs[0]) json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Check if the returned JSON is correct.
	else if (st_cmp_cs_eq(NULLER((chr_t *)json_values[0]), PLACER("success", 7))) {

		st_sprint(errmsg, "Failed parsing the returned JSON. { command = \"%.*s\", json = \"%.*s\" }", st_length_int(command),
			st_char_get(command), st_length_int(json), st_char_get(json));
		json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Clean up before the next check.
	st_wipe(command);

	if (json_objs[0]) json_decref_d(json_objs[0]);

	json_objs[0] = NULL;

	client_close(client);
	client = NULL;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Test config.edit 	: commands[34]
	// JSON Command			: {"id":36,"method":"folders.rename","params":{"context":"mail","folderID": <folder_ids[1]>,
	//							"name":<rand_strs[0]>}}
	// Expected Response 	: {"jsonrpc":"2.0","result":{"folder.rename":"success"},"id":36}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Generate the random input for "name".
	if (!rand_choices(choices, 16, rand_strs[0])) {

		st_sprint(errmsg, "Failed to create random input. { command = \"%s\" }", commands[33]);
		client_close(client);
		return false;
	}

	// Construct the command string.
	else if (!(st_sprint(command, commands[34], folder_ids[1], st_length_int(rand_strs[0]), st_char_get(rand_strs[0])))) {

		st_sprint(errmsg, "Failed to create command string. { command = \"%s\", \"name\" = \"%.*s\"}", commands[34],
			st_length_int(rand_strs[0]), st_char_get(rand_strs[0]));
		client_close(client);
		return false;
	}

	// Submit the command and check the status of the response.
	else if (!check_camel_print(command, cookie, json, secure)) {

		st_sprint(errmsg, "Failed to return a successful HTTP response. { command = \"%.*s\" }", st_length_int(command),
			st_char_get(command));
		client_close(client);
		return false;
	}

	// Parse the returned JSON.
	else if (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err)) || json_unpack_d(json_objs[0], "{s:{s:s}}", "result",
		"folder.rename", &json_values[0]) != 0) {

		st_sprint(errmsg, "Failed parsing the returned JSON. { command = \"%.*s\", json = \"%.*s\" }", st_length_int(command),
			st_char_get(command), st_length_int(json), st_char_get(json));
		if (json_objs[0]) json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Check if the returned JSON is correct.
	else if (st_cmp_cs_eq(NULLER((chr_t *)json_values[0]), PLACER("success", 7))) {

		st_sprint(errmsg, "Failed parsing the returned JSON. { command = \"%.*s\", json = \"%.*s\" }", st_length_int(command),
			st_char_get(command), st_length_int(json), st_char_get(json));
		json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Clean up before the next check.
	st_wipe(command);

	if (json_objs[0]) json_decref_d(json_objs[0]);

	json_objs[0] = NULL;

	client_close(client);
	client = NULL;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Test config.edit 	: commands[35]
	// JSON Command			: {"id":37,"method":"folders.remove","params":{"context":"contacts","folderID":<folder_ids[0]>}}
	// Expected Response 	: {"jsonrpc":"2.0","result":{"folder.remove":"success"},"id":37}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Construct the command string.
	if (!(st_sprint(command, commands[36], folder_ids[0]))) {

		st_sprint(errmsg, "Failed to create command string. { command = \"%s\"}", commands[36]);
		client_close(client);
		return false;
	}

	// Submit the command and check the status of the response.
	else if (!check_camel_print(command, cookie, json, secure)) {

		st_sprint(errmsg, "Failed to return a successful HTTP response. { command = \"%.*s\" }", st_length_int(command),
			st_char_get(command));
		client_close(client);
		return false;
	}

	// Parse the returned JSON.
	else if (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err)) || json_unpack_d(json_objs[0], "{s:{s:s}}", "result",
		"folder.remove", &json_values[0]) != 0) {

		st_sprint(errmsg, "Failed parsing the returned JSON. { command = \"%.*s\", json = \"%.*s\" }", st_length_int(command),
			st_char_get(command), st_length_int(json), st_char_get(json));
		if (json_objs[0]) json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Check if the returned JSON is correct.
	else if (st_cmp_cs_eq(NULLER((chr_t *)json_values[0]), PLACER("success", 7))) {

		st_sprint(errmsg, "Failed parsing the returned JSON. { command = \"%.*s\", json = \"%.*s\" }", st_length_int(command),
			st_char_get(command), st_length_int(json), st_char_get(json));
		json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Clean up before the next check.
	st_wipe(command);

	if (json_objs[0]) json_decref_d(json_objs[0]);

	json_objs[0] = NULL;
	json_values[0] = NULL;

	client_close(client);
	client = NULL;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Test config.edit 	: commands[36]
	// JSON Command			: {"id":38,"method":"folders.remove","params":{"context":"contacts","folderID":<folder_ids[1]>}}
	// Expected Response 	: {"jsonrpc":"2.0","result":{"folder.remove":"success"},"id":38}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Construct the command string.
	if (!(st_sprint(command, commands[38], folder_ids[1]))) {

		st_sprint(errmsg, "Failed to create command string. { command = \"%s\"}", commands[38]);
		client_close(client);
		return false;
	}

	// Submit the command and check the status of the response.
	else if (!check_camel_print(command, cookie, json, secure)) {

		st_sprint(errmsg, "Failed to return a successful HTTP response. { command = \"%.*s\" }", st_length_int(command),
			st_char_get(command));
		client_close(client);
		return false;
	}

	// Parse the returned JSON.
	else if (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err)) || json_unpack_d(json_objs[0], "{s:{s:s}}", "result",
		"folder.remove", &json_values[0]) != 0) {

		st_sprint(errmsg, "Failed parsing the returned JSON. { command = \"%.*s\", json = \"%.*s\" }", st_length_int(command),
			st_char_get(command), st_length_int(json), st_char_get(json));
		if (json_objs[0]) json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Check if the returned JSON is correct.
	else if (st_cmp_cs_eq(NULLER((chr_t *)json_values[0]), PLACER("success", 7))) {

		st_sprint(errmsg, "Failed parsing the returned JSON. { command = \"%.*s\", json = \"%.*s\" }", st_length_int(command),
			st_char_get(command), st_length_int(json), st_char_get(json));
		json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Clean up before the next check.
	st_wipe(command);

	if (json_objs[0]) json_decref_d(json_objs[0]);

	json_objs[0] = NULL;
	json_values[0] = NULL;

	client_close(client);
	client = NULL;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Test config.edit 	: commands[37]
	// JSON Command			: {"id":39,"method":"folders.remove","params":{"context":"contacts","folderID":<folder_ids[2]>}}
	// Expected Response 	: {"jsonrpc":"2.0","result":{"folder.remove":"success"},"id":39}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Construct the command string.
	if (!(st_sprint(command, commands[37], folder_ids[2]))) {

		st_sprint(errmsg, "Failed to create command string. { command = \"%s\"}", commands[37]);
		client_close(client);
		return false;
	}

	// Submit the command and check the status of the response.
	else if (!check_camel_print(command, cookie, json, secure)) {

		st_sprint(errmsg, "Failed to return a successful HTTP response. { command = \"%.*s\" }", st_length_int(command),
			st_char_get(command));
		client_close(client);
		return false;
	}

	// Parse the returned JSON.
	else if (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err)) || json_unpack_d(json_objs[0], "{s:{s:s}}", "result",
		"folder.remove", &json_values[0]) != 0) {

		st_sprint(errmsg, "Failed parsing the returned JSON. { command = \"%.*s\", json = \"%.*s\" }", st_length_int(command),
			st_char_get(command), st_length_int(json), st_char_get(json));
		if (json_objs[0]) json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Check if the returned JSON is correct.
	else if (st_cmp_cs_eq(NULLER((chr_t *)json_values[0]), PLACER("success", 7))) {

		st_sprint(errmsg, "Failed parsing the returned JSON. { command = \"%.*s\", json = \"%.*s\" }", st_length_int(command),
			st_char_get(command), st_length_int(json), st_char_get(json));
		json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Clean up before the next check.
	st_wipe(command);

	if (json_objs[0]) json_decref_d(json_objs[0]);

	json_objs[0] = NULL;
	json_values[0] = NULL;

	client_close(client);
	client = NULL;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Test config.edit 	: commands[38]
	// JSON Command			: {"id":40,"method":"folders.remove","params":{"context":"contacts","folderID":<folder_ids[4]>}}
	// Expected Response 	: {"jsonrpc":"2.0","result":{"folder.remove":"success"},"id":40}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Test config.edit 	: commands[39]
	// JSON Command			: {"id":41,"method":"aliases"}
	// Expected Response 	: {"jsonrpc":"2.0","result":[...],"id":41}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Submit the command and check the status of the response.
	if (!check_camel_print(PLACER(commands[39], ns_length_get(commands[39])), cookie, json, secure)) {

		st_sprint(errmsg, "Failed to return a successful HTTP response. { command = \"%.*s\" }", st_length_int(command),
			st_char_get(command));
		client_close(client);
		return false;
	}

	// Parse the returned JSON.
	else if (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err)) || !(json_objs[1] = json_object_get_d(json_objs[0],
		"result")) || !json_is_array(json_objs[1])) {

		st_sprint(errmsg, "Failed to return a successful JSON response. { command = \"%s\", json = \"%.*s\" }", commands[39],
			st_length_int(json), st_char_get(json));
		if (json_objs[0]) json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Clean up before the next check.
	st_wipe(command);

	if (json_objs[0]) json_decref_d(json_objs[0]);

	json_objs[0] = NULL;
	json_values[0] = NULL;

	client_close(client);
	client = NULL;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Test config.edit 	: commands[40]
	// JSON Command			: {"id":42,"method":"folders.add","params":{"context":"mail","name":<rand_strs[0]>}}
	// Expected Response 	: {"jsonrpc":"2.0","result":{"folderID":<folder_ids[3]>},"id":42}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Generate the random input for "name".
	if (!rand_choices(choices, 16, rand_strs[0])) {

		st_sprint(errmsg, "Failed to create random input. { command = \"%s\" }", commands[40]);
		client_close(client);
		return false;
	}

	// Construct the command string.
	else if (!(st_sprint(command, commands[40], st_length_int(rand_strs[0]), st_char_get(rand_strs[0])))) {

		st_sprint(errmsg, "Failed to create command string. { command = \"%s\", \"name\" = \"%.*s\"}", commands[40],
			st_length_int(rand_strs[0]), st_char_get(rand_strs[0]));
		client_close(client);
		return false;
	}

	// Submit the command and check the status of the response.
	else if (!check_camel_print(command, cookie, json, secure)) {

		st_sprint(errmsg, "Failed to return a successful HTTP response. { command = \"%.*s\" }", st_length_int(command),
			st_char_get(command));
		client_close(client);
		return false;
	}

	// Parse the returned JSON.
	else if (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err)) || (json_unpack_d(json_objs[0], "{s:{s:i}}", "result",
		"folderID", &folder_ids[3]) != 0)) {

		st_sprint(errmsg, "Failed to return a successful JSON response. { command = \"%.*s\", json = \"%.*s\" }", st_length_int(command),
			st_char_get(command), st_length_int(json), st_char_get(json));
		if (json_objs[0]) json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Clean up before the next check.
	st_wipe(command);

	if (json_objs[0]) json_decref_d(json_objs[0]);

	json_objs[0] = NULL;

	client_close(client);
	client = NULL;

	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
	// Test config.edit 	: commands[58]
	// JSON Command			: "{\"id\":60,\"method\":\"logout\"}"
	// Expected Response 	: {"jsonrpc":"2.0","result":{"logout":"success},"id":60}
	///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

	// Submit the command and check the status of the response.
	if (!check_camel_print(PLACER(commands[58], ns_length_get(commands[58])), cookie, json, secure)) {

		st_sprint(errmsg, "Failed to return a successful HTTP response. { command = \"%.*s\" }", st_length_int(command),
			st_char_get(command));
		client_close(client);
		return false;
	}

	// Parse the returned JSON.
	else if (!(json_objs[0] = json_loads_d(st_char_get(json), 0, &err)) || json_unpack_d(json_objs[0], "{s:{s:s}}", "result",
		"logout", &json_values[0]) != 0) {

		st_sprint(errmsg, "Failed parsing the returned JSON. { command = \"%.*s\", json = \"%.*s\" }", st_length_int(command),
			st_char_get(command), st_length_int(json), st_char_get(json));
		if (json_objs[0]) json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Check if the returned JSON is correct.
	else if (st_cmp_cs_eq(NULLER((chr_t *)json_values[0]), PLACER("success", 7))) {

		st_sprint(errmsg, "Failed parsing the returned JSON. { command = \"%.*s\", json = \"%.*s\" }", st_length_int(command),
			st_char_get(command), st_length_int(json), st_char_get(json));
		json_decref_d(json_objs[0]);
		client_close(client);
		return false;
	}

	// Clean up before the next check.
	st_wipe(command);

	if (json_objs[0]) json_decref_d(json_objs[0]);

	json_objs[0] = NULL;
	json_values[0] = NULL;

	client_close(client);
	client = NULL;

	return true;


}
