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

bool_t check_smtp_checkers_regex_sthread(stringer_t *errmsg) {

	chr_t *expressions[] = {
			"http\\:\\/\\/www\\.lavabit\\.com\\ ",
			"lavabit\\ ",
			"expr\\ with\\ spaces",
			"expr\\ with\\ trailing\\ backslash\\ ",
			"\\ levabit\\ ",
			"\\*\\*\\ regex\\ with\\ wildcards\\ \\*",
			"regex\\ \\*\\ with\\ \\*\\ wildcards"
	};
	struct re_pattern_buffer regbuff;
	mm_wipe(&regbuff, sizeof(struct re_pattern_buffer));

	for (size_t i = 0; i < (sizeof(expressions)/sizeof(chr_t*)); i++) {
		if (regcomp(&regbuff, expressions[i], REG_ICASE) != 0) {
			st_sprint(errmsg, "Regular expression compilation failed. { expression = %s }", expressions[i]);
			return false;;
		}
	}

	return true;
}

bool_t check_smtp_checkers_filters_sthread(stringer_t *errmsg, int_t action, int_t expected) {

	multi_t key = mt_get_null();
	smtp_inbound_prefs_t prefs;
	smtp_inbound_filter_t *filter = NULL;
	stringer_t *message = NULLER( \
			"From: Princess (princess@example.com\r\n" \
			"To: ladar@lavabit.com\r\n" \
			"Date: March 7th, 2017 5:55:55 PM CST\r\n" \
			"Subject: SMTP Filters Unit Test\r\n" \
			"\r\n" \
			"This is the message body.\r\n" \
			".\r\n");
	chr_t *fields[] = { "From", "To", "Date", "Subject" }, *exprs[] = { "Princess", "ladar", "March", "Filters" };

	mm_wipe(&prefs, sizeof(smtp_inbound_prefs_t));
	prefs.filters = inx_alloc(M_INX_LINKED, &mm_free);
	prefs.usernum = 2;
	prefs.mark = 0;

	if (!(filter = mm_alloc(sizeof(smtp_inbound_filter_t)))) return false;
	key = mt_set_type(key, M_TYPE_UINT64);
	key.val.u64 = rand_get_int64();
	inx_insert(prefs.filters, key, filter);
	filter->action = action;
	filter->foldernum = 1;
	filter->label = NULLER("JUNK:");

	// Test if it returns 1 when no action is taken.
	filter->location = SMTP_FILTER_LOCATION_ENTIRE;
	filter->expression = NULLER("//");
	if (status() && (smtp_check_filters(&prefs, &message) != 1)) {
		st_sprint(errmsg, "Failed to return 1 when no action was taken.");
		return false;
	}

	// Test if it returns -1 on broken regex.
	filter->expression = NULLER("[this[is[not[valid[regex[");
	if (status() && (smtp_check_filters(&prefs, &message) != -1)) {
		st_sprint(errmsg, "Failed to return -1 when regex is broken.");
		return false;
	}

	// Test if it returns -2 when the message is supposed to be deleted and not when there is no match (header).
	filter->expression = NULLER("Princess");
	filter->location = SMTP_FILTER_LOCATION_HEADER;
	if (status() && (smtp_check_filters(&prefs, &message) != expected)) {
		st_sprint(errmsg, "Failed to return -2 when the regex matches the header. { action = %x }", action);
		return false;
	}
	filter->expression = NULLER("This is not in the header.");
	if (status() && !(smtp_check_filters(&prefs, &message) != expected)) {
		st_sprint(errmsg, "Failed to return -2 when the regex does not match the header. { action = %x }", action);
		return false;
	}

	// Test if it returns -2 when the message is supposed to be deleted and not when there is no match (body).
	filter->expression = NULLER("the message body");
	filter->location = SMTP_FILTER_LOCATION_BODY;
	if (status() && (smtp_check_filters(&prefs, &message) != expected)) {
		st_sprint(errmsg, "Failed to return -2 when the regex matches the body. { action = %x }", action);
		return false;
	}
	filter->expression = NULLER("This is not in the body.");
	if (status() && !(smtp_check_filters(&prefs, &message) != expected)) {
		st_sprint(errmsg, "Failed to not return -2 when the regex does not match the body. { action = %x }", action);
		return false;
	}

	// Test if it returns -2 when the message is supposed to be deleted and not when there is no match (entire).
	filter->expression = NULLER("March 7th");
	filter->location = SMTP_FILTER_LOCATION_ENTIRE;
	if (status() && (smtp_check_filters(&prefs, &message) != expected)) {
		st_sprint(errmsg, "Failed to return -2 when the regex matches everything. { action = %x }", action);
		return false;
	}
	filter->expression = NULLER("This is not in the entire message.");
	if (status() && !(smtp_check_filters(&prefs, &message) != expected)) {
		st_sprint(errmsg, "Failed to not return -2 when the regex does not match anything. { action = %x }", action);
		return false;
	}

	// Test if it returns -2 when the message is supposed to be deleted and not when there is no match (fields).
	filter->location = SMTP_FILTER_LOCATION_FIELD;
	for (size_t i = 0; i < (sizeof(fields)/sizeof(chr_t*)); i++) {

		filter->field = NULLER(fields[i]);
		filter->expression = NULLER(exprs[i]);

		if (status() && (smtp_check_filters(&prefs, &message) != expected)) {
			st_sprint(errmsg, "Failed to return -2 when the regex matches. { field = \"%s\" }", fields[i]);
			return false;
		}
		filter->expression = NULLER("This is not in any of the fields.");
		if (status() && !(smtp_check_filters(&prefs, &message) != expected)) {
			st_sprint(errmsg, "Failed to not return -2 when the regex does not match. { field = \"%s\" }", fields[i]);
			return false;
		}
	}

	inx_cleanup(prefs.filters);

	return true;
}
