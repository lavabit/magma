/**
 * @file /check/magma/servers/smtp/checkers_check.c
 *
 * @brief SMTP checkers test functions.
 */

#include "magma_check.h"

bool_t check_smtp_checkers_greylist_sthread(stringer_t *errmsg) {

	uint64_t now;
	connection_t con;
	server_t *server = NULL;
	client_t *client = NULL;
	smtp_inbound_prefs_t prefs;
	stringer_t *value = NULL, *addr = MANAGEDBUF(128), *key = MANAGEDBUF(256);

	mm_wipe(&con, sizeof(connection_t));
	mm_wipe(&prefs, sizeof(smtp_inbound_prefs_t));

	con.smtp.bypass = true;
	con.smtp.authenticated = true;
	con.smtp.mailfrom = NULLER("check@example.com");

	if (!(server = servers_get_by_protocol(HTTP, false))) {
		st_sprint(errmsg, "The SMTP greylist check couldn't find a valid SMTP server instance.");
		return false;
	}

	else if (!(client = client_connect("localhost", server->network.port))) {
		st_sprint(errmsg, "The SMTP greylist check couldn't setup a socket connection for testing address resolution.");
		return false;
	}

	// The connection needs a valid network socket or the address lookup will fail randomly.
	con.network.sockd = client->sockd;

	prefs.usernum = 1;
	prefs.greytime = 1;

	if (!(addr = con_addr_reversed(&con, addr)) ||
		st_sprint(key, "magma.greylist.%lu.%.*s", prefs.usernum, st_length_int(addr), st_char_get(addr)) <= 0) {
		st_sprint(errmsg, "The SMTP greylist check failed to create a valid lookup key.");
		client_close(client);
		return false;
	}

	// Delete the greylist key from the cache to ensure the test can be run in any order.
	cache_delete(key);

	if (smtp_check_greylist(&con, &prefs) != 1) {
		st_sprint(errmsg, "The SMTP greylist function failed to return 1 when bypass is enabled.");
		client_close(client);
		return false;
	}

	else if ((con.smtp.bypass = false) || smtp_check_greylist(&con, &prefs) != 0) {
		st_sprint(errmsg, "The SMTP greylist function failed to return 0 after the initial try.");
		client_close(client);
		return false;
	}

	else if (smtp_check_greylist(&con, &prefs) != 0) {
		st_sprint(errmsg, "The SMTP greylist check function failed to return 0 when resubmitted too fast.");
		client_close(client);
		return false;
	}

	// Manually set the timestamp value in the cache (steps taken from the smtp_check_greylist function).
	else if (!(now = time(NULL) - 100) || !(value = cache_get(key)) || !(*(((uint64_t *)st_data_get(value)) + 1) = now) ||
		(cache_set(key, value, 2592000) != 1)) {
		st_sprint(errmsg, "The SMTP check greylist function failed while attempting to update the cached greylist timestamp.");
		client_close(client);
		st_cleanup(value);
		return false;
	}

	else if (smtp_check_greylist(&con, &prefs) != 0) {
		st_sprint(errmsg, "The SMTP check greylist function failed to return 1 when the last timestamp is old enough.");
		client_close(client);
		st_free(value);
		return false;
	}

	client_close(client);
	st_free(value);

	return true;
}

bool_t check_smtp_checkers_filters_test(stringer_t *errmsg, int_t action, int_t expected) {

	size_t fields_num = 15;
	multi_t key = mt_get_null();
	smtp_inbound_prefs_t prefs;
	smtp_inbound_filter_t *filter = NULL;
	stringer_t *fields[15] = { NULL }, *values[15] = { NULL }, *combined = NULL, *body = NULL;

	mm_wipe(&prefs, sizeof(smtp_inbound_prefs_t));
	prefs.filters = inx_alloc(M_INX_LINKED, &mm_free);
	prefs.usernum = 2;
	prefs.mark = 0;

	if (!(filter = mm_alloc(sizeof(smtp_inbound_filter_t)))) return false;
	key = mt_set_type(key, M_TYPE_UINT64);
	key.val.u64 = rand_get_int64();
	inx_insert(prefs.filters, key, filter);

	// Set default fields and values;
	fields[0] 	= NULLER("From:");
	values[0] 	= NULLER(" Princess (princess@lavabit.com)\r\n");
	fields[1] 	= NULLER("Subject");
	values[1] 	= NULLER(": Unit Test: SMTP Filters\r\n");
	fields[2] 	= NULLER("Date");
	values[2] 	= NULLER(": March 7th, 2017 5:55:55 PM CST\r\n");
	fields[3] 	= NULLER("To");
	values[3] 	= NULLER(": ladar@lavabit.com\r\n");
	fields[4] 	= NULLER("Return-Path");
	values[4] 	= NULLER(": <princess@lavabit.com>\r\n");
	fields[5] 	= NULLER("Envelope-To");
	values[5] 	= NULLER(": ladar@lavabit.com\n");
	fields[6] 	= NULLER("Delivery-Date");
	values[6] 	= NULLER(": Tue, 7 Mar 2017 18:55:55-0600\r\n");
	fields[7] 	= NULLER("Received");
	values[7] 	= NULLER(": from unit.test.lavabit.com \r\n");
	fields[8] 	= NULLER("Dkim-Signature");
	values[8] 	= NULLER(": abcdefghijklm \r\n");
	fields[9] 	= NULLER("Domainkey-Signature");
	values[9] 	= NULLER(": nopqrstuvwxyz \r\n");
	fields[10] 	= NULLER("Message-Id");
	values[10]	= NULLER(": <111111111111111111>\r\n");
	fields[11] 	= NULLER("Mime-Version");
	values[11] 	= NULLER(": 1.0\r\n");
	fields[12] 	= NULLER("Content-Type");
	values[12] 	= NULLER(": multipart/alternative \r\n");
	fields[13] 	= NULLER("X-Spam-Status");
	values[13] 	= NULLER(": score=5\r\n");
	fields[14] 	= NULLER("X-Spam-Level");
	values[14] 	= NULLER(": **\r\n");

	body 		= NULLER("\r\n Hello World!\r\n");

	combined = st_merge("sssssssssssssssssssssssssssssss", fields[0], values[0], fields[1], values[1],
			fields[2], values[2], fields[3], values[3], fields[4], values[4], fields[5], values[5], fields[6],
			values[6], fields[7], values[7], fields[8], values[8], fields[9], values[9], fields[10], values[10],
			fields[11], values[11], fields[12], values[12], fields[13], values[13], fields[14], values[14], body);

	// Test if it returns 1 when no action is taken.
	filter->expression = NULLER("//g");
	filter->location = SMTP_FILTER_LOCATION_ENTIRE;
	if (status() && (smtp_check_filters(&prefs, &combined) != 1)) {
		st_sprint(errmsg, "Failed to return 1 when no action was taken.");
		return false;
	}

	// Test if it returns -1 on broken regex.
	filter->expression = NULLER("[this[is[not[valid[regex[");
	if (status() && (smtp_check_filters(&prefs, &combined) != -1)) {
		st_sprint(errmsg, "Failed to return -1 when regex is broken.");
		return false;
	}

	// First test the delete action.
	filter->action = action;

	// Test if it returns -2 when the message is supposed to be deleted and not when there is no match (header).
	filter->expression = NULLER("Princess");
	filter->location = SMTP_FILTER_LOCATION_HEADER;
	if (status() && (smtp_check_filters(&prefs, &combined) != expected)) {
		st_sprint(errmsg, "Failed to return -2 when the regex matches the header and the action is delete.");
		return false;
	}
	filter->expression = NULLER("This is not in the header.");
	if (status() && !(smtp_check_filters(&prefs, &combined) != expected)) {
		st_sprint(errmsg, "Failed to return -2 when the regex does not match the header and the action is delete.");
		return false;
	}

	// Test if it returns -2 when the message is supposed to be deleted and not when there is no match (body).
	filter->expression = NULLER("Hello World!");
	filter->location = SMTP_FILTER_LOCATION_BODY;
	if (status() && (smtp_check_filters(&prefs, &combined) != expected)) {
		st_sprint(errmsg, "Failed to return -2 when the regex matches the body and the action is delete.");
		return false;
	}
	filter->expression = NULLER("This is not in the body.");
	if (status() && !(smtp_check_filters(&prefs, &combined) != expected)) {
		st_sprint(errmsg, "Failed to not return -2 when the regex does not match the body and the action is delete.");
		return false;
	}

	// Test if it returns -2 when the message is supposed to be deleted and not when there is no match (entire).
	filter->expression = NULLER("March 7th");
	filter->location = SMTP_FILTER_LOCATION_ENTIRE;
	if (status() && (smtp_check_filters(&prefs, &combined) != expected)) {
		st_sprint(errmsg, "Failed to return -2 when the regex matches everything and the action is delete.");
		return false;
	}
	filter->expression = NULLER("This is not in the entire message.");
	if (status() && !(smtp_check_filters(&prefs, &combined) != expected)) {
		st_sprint(errmsg, "Failed to not return -2 when the regex does not match everything and the action is delete.");
		return false;
	}

	// Test if it returns -2 when the message is supposed to be deleted and not when there is no match (fields).
	filter->location = SMTP_FILTER_LOCATION_FIELD;
	for (size_t i = 0; i < fields_num; i++) {
		filter->expression = values[i];
		filter->field = fields[i];
		if (status() && (smtp_check_filters(&prefs, &combined) != expected)) {
			st_sprint(errmsg, "Failed to return -2 when the regex matches the field %s.", st_char_get(filter->field));
			return false;
		}
		filter->expression = NULLER("This is not in any of the fields.");
		if (status() && !(smtp_check_filters(&prefs, &combined) != expected)) {
			st_sprint(errmsg, "Failed to not return -2 when the regex does not match the field %s.", st_char_get(filter->field));
			return false;
		}
	}

	inx_cleanup(prefs.filters);
	st_cleanup(combined);

	return true;
}

bool_t check_smtp_checkers_filters_sthread(stringer_t *errmsg) {

	bool_t outcome = true;

	if (status()) outcome = check_smtp_checkers_filters_test(errmsg, SMTP_FILTER_ACTION_DELETE, -2);
	if (status() && outcome) outcome = check_smtp_checkers_filters_test(errmsg, SMTP_FILTER_ACTION_MOVE, 2);
	if (status() && outcome) outcome = check_smtp_checkers_filters_test(errmsg, SMTP_FILTER_ACTION_MARK_READ, 4);

	return outcome;

}
