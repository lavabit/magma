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

bool_t check_smtp_checkers_filters_sthread(stringer_t *errmsg) {

	bool_t outcome = true;
	smtp_inbound_prefs_t prefs;
	stringer_t *from = NULL, *subject = NULL, *date = NULL, *to = NULL, *return_path = NULL, *envelope_to = NULL,
			*delivery_date = NULL, *received = NULL, *dkim_sig = NULL, *domainkey_sig = NULL, *message_id = NULL,
			*mime_version = NULL, *content_type = NULL, *x_spam_status = NULL, *x_spam_level = NULL,
			*message_body = NULL, *combined = NULL;

	mm_wipe(&prefs, sizeof(smtp_inbound_prefs_t));

	// Set default values.
	from 			= NULLER("From: Princess (princess@lavabit.com)\n");
	subject 		= NULLER("Subject: Unit Test: SMTP Filters\n");
	date 			= NULLER("Date: March 7th, 2017 5:55:55 PM CST\n");
	to 				= NULLER("To: ladar@lavabit.com\n");
	return_path 	= NULLER("Return-Path: <princess@lavabit.com>\n");
	envelope_to 	= NULLER("Envelope-To: ladar@lavabit.com\n");
	delivery_date 	= NULLER("Delivery-Date: Tue, 7 Mar 2017 18:55:55-0600\n");
	received 		= NULLER("Received: \n");
	dkim_sig 		= NULLER("Dkim-Signature: \n");
	domainkey_sig 	= NULLER("Domainkey-Signature: \n");
	message_id 		= NULLER("Message-Id: <>\n");
	mime_version 	= NULLER("Mime-Version: 1.0\n");
	content_type 	= NULLER("Content-Type: \n");
	x_spam_status 	= NULLER("X-Spam-Status: \n");
	x_spam_level 	= NULLER("X-Spam-Level: \n");
	message_body 	= NULLER("Message Body: Hello World!\n");

	// Test if it returns -1 on error.
	combined = st_merge("ssssssssssssssss", from, subject, date, to, return_path, envelope_to, delivery_date, received,
			dkim_sig, domainkey_sig, message_id, mime_version, content_type, x_spam_status, x_spam_level, message_body);

	if (status() && outcome && smtp_check_filters(&prefs, &combined) != 1) {
		st_sprint(errmsg, "Failed to return 1 when no action was taken.");
		outcome = false;
	}
	st_free(combined);

	// Test if it returns -2 when the message is supposed to be deleted.
	combined = st_merge("ssssssssssssssss", from, subject, date, to, return_path, envelope_to, delivery_date, received,
			dkim_sig, domainkey_sig, message_id, mime_version, content_type, x_spam_status, x_spam_level, message_body);

	if (status() && outcome && smtp_check_filters(&prefs, &combined) != 2) {
		st_sprint(errmsg, "Failed to return 1 when no action was taken.");
		outcome = false;
	}
	st_free(combined);

	// Test if it returns 1 when no action is taken.
	combined = st_merge("ssssssssssssssss", from, subject, date, to, return_path, envelope_to, delivery_date, received,
			dkim_sig, domainkey_sig, message_id, mime_version, content_type, x_spam_status, x_spam_level, message_body);

	if (status() && outcome && smtp_check_filters(&prefs, &combined) != 1) {
		st_sprint(errmsg, "Failed to return 1 when no action was taken.");
		outcome = false;
	}
	st_free(combined);

	// Test if it returns 2 when the message should go to a new folder.
	combined = st_merge("ssssssssssssssss", from, subject, date, to, return_path, envelope_to, delivery_date, received,
			dkim_sig, domainkey_sig, message_id, mime_version, content_type, x_spam_status, x_spam_level, message_body);

	if (status() && outcome && smtp_check_filters(&prefs, &combined) != 2) {
		st_sprint(errmsg, "Failed to return 1 when no action was taken.");
		outcome = false;
	}
	st_free(combined);

	// Test if it returns 3 when the message content is modified.
	combined = st_merge("ssssssssssssssss", from, subject, date, to, return_path, envelope_to, delivery_date, received,
			dkim_sig, domainkey_sig, message_id, mime_version, content_type, x_spam_status, x_spam_level, message_body);

	if (status() && outcome && smtp_check_filters(&prefs, &combined) != 3) {
		st_sprint(errmsg, "Failed to return 1 when no action was taken.");
		outcome = false;
	}
	st_free(combined);

	// Test if it returns 4 when the message is marked as read.
	combined = st_merge("ssssssssssssssss", from, subject, date, to, return_path, envelope_to, delivery_date, received,
			dkim_sig, domainkey_sig, message_id, mime_version, content_type, x_spam_status, x_spam_level, message_body);

	if (status() && outcome && smtp_check_filters(&prefs, &combined) != 4) {
		st_sprint(errmsg, "Failed to return 1 when no action was taken.");
		outcome = false;
	}
	st_free(combined);

	return outcome;

}
