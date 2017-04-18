/**
 * @file /check/magma/regression/regression_check_helpers.c
 *
 * @brief Functions that go with the regression test suite.
 */

#include "magma_check.h"

void check_regression_file_descriptors_leak_test(void) {

	stringer_t *m;
	bool_t *outcome;

	if (!thread_start() || !(outcome = mm_alloc(sizeof(bool_t)))) {
		log_error("Unable to setup the thread context.");
		pthread_exit(NULL);
		return;
	}

	if (!(m = st_alloc_opts(MAPPED_T | JOINTED | HEAP, 1024))) {
		*outcome = false;
	}
	else {
		st_free(m);
		*outcome = true;
	}

	thread_stop();
	pthread_exit(outcome);
	return;
}

/**
 * @brief	Reads lines from a client_t* until a line is reached containing token, and if a line is reached
 * 			that starts with a period, checks if the line starts with two periods.
 *
 * @param	client	The client_t* to read lines from.
 * @param	token	A chr_t*. When a line is reached that contains with this token, the function returns.
 * @return	True if all lines read until token that start with '.' start with '..', false otherwise.
 */
bool_t check_client_dot_stuff(client_t *client, chr_t *token) {

	while (client_read_line(client) > 0 && st_search_cs(&(client->line), NULLER(token), NULL)) {

		if (st_cmp_cs_starts(&(client->line), NULLER(".")) == 0 &&
			st_cmp_cs_starts(&(client->line), NULLER("..")) != 0) {

			return false;
		}
	}
	return true;
}

bool_t check_regression_smtp_dot_stuffing_sthread(stringer_t *errmsg) {

	client_t *client = NULL;
	server_t *server = NULL;
	uint64_t messagenum = 0;
	stringer_t *mailfrom = "magma@lavabit.com", *rcptto = NULLER("princess@example.com"),
		*message = NULLER("To: \"Magma\" <magma@lavabit.com>\r\n" \
		"From: \"Princess\" <princess@example.com>\r\n" \
		"Subject: Dot Stuffing Regression Test\r\n\r\n" \
		"This is an SMTP message whose body has a period at the start of a line\r\n" \
		". In fact, there are two instances of this in the body of this message\r\n" \
		". The SMTP client code should stuff an extra period after each of them.\r\n" \
		".\r\n");

	// First, send the message with periods at the beginning of lines in the body.
	if (!(client = smtp_client_connect(0))) {
		st_sprint(errmsg, "Failed to connect with the SMTP server.");
		return false;
	}
	else if (smtp_client_send_helo(client) != 1) {
		st_sprint(errmsg, "Failed to return successful state after HELO.");
		smtp_client_close(client);
		return false;
	}
	else if (smtp_client_send_mailfrom(client, mailfrom, 0) != 1) {
		st_sprint(errmsg, "Failed to return successful state after MAIL FROM.");
		smtp_client_close(client);
		return false;
	}
	else if (smtp_client_send_rcptto(client, rcptto) != 1) {
		st_sprint(errmsg, "Failed to return successful state after RCPT TO.");
		smtp_client_close(client);
		return false;
	}
	else if (smtp_client_send_data(client, message, false) != 1) {
		st_sprint(errmsg, "Failed to return successful state after DATA.");
		smtp_client_close(client);
		return false;
	}

	smtp_client_close(client);

	/// LOW: Split this unit test in the SMTP client dot stuff test, and the POP dot stuff test. In theory the latter should
	/// 	add a test message using mail_store_message() and then find the precise UID for said test message via POP.

	// Next, check if the entire message was sent to the recipient.
	if (!(server = servers_get_by_protocol(POP, false)) || !(client = client_connect("localhost", server->network.port)) ||
		!net_set_timeout(client->sockd, 20, 20) || client_read_line(client) <= 0 || client_status(client) != 1 ||
		st_cmp_cs_starts(&(client->line), NULLER("+OK"))) {

		st_sprint(errmsg, "Failed to connect to POP server.");
		client_close(client);
		return false;
	}
	else if (!check_pop_client_auth(client, "princess", "password", errmsg)) {

		client_close(client);
		return false;
	}
	else if (client_write(client, PLACER("LIST\r\n", 6)) != 6 || (messagenum = check_pop_client_read_list(client, errmsg)) == 0 ||
		client_status(client) != 1) {

		if (st_empty(errmsg)) st_sprint(errmsg, "Failed to return successful state after LIST.");
		client_close(client);
		return false;
	}
	else if (client_print(client, "TOP %lu 0\r\n", messagenum) != (uint16_digits(messagenum) + 8) || client_status(client) != 1) {

		st_sprint(errmsg, "Failed to return successful status after TOP.");
		client_close(client);
		return false;
	}
	else if (!check_client_dot_stuff(client, "Date:")) {

		st_sprint(errmsg, "The received message failed to be properly dot stuffed.");
		client_close(client);
		return false;
	}

	client_close(client);
	return true;
}

bool_t check_regression_imap_search_range_parsing_sthread(stringer_t *errmsg, uint32_t port) {

	client_t *client = NULL;

	// Enable logging to see the error. It should take the form of two repeating lines:
	//
	// Attempted a token count on a NULL string bugger.
	// Attempted token extraction from a NULL string buffer.
	log_enable();

	// Connect the client over TCP.
	if (!(client = client_connect("localhost", port)) || client_read_line(client) <= 0 || !net_set_timeout(client->sockd, 20, 20)) {

		st_sprint(errmsg, "Failed to connect with the IMAP server over TCP.");
		client_close(client);
		return false;
	}
	else if (!check_imap_client_login(client, "princess", "password", "A00", errmsg) ||
		!check_imap_client_select(client, "Inbox", "A01", errmsg)) {

		if (st_empty(errmsg)) st_sprint(errmsg, "The LOGIN or SELECT command failed.");
		client_close(client);
		return false;
	}
	else if (client_write(client, PLACER("A02 SEARCH UID 531870239:532870239 NOT DELETE\r\n", 47)) != 47 ||
		!check_imap_client_read_end(client, "A02") ||
		client_write(client, PLACER("A03 SEARCH UID 531870239:532870239, 1:10000 NOT DELETE\r\n", 56)) != 56 ||
		!check_imap_client_read_end(client, "A03")) {

		if (st_empty(errmsg)) st_sprint(errmsg, "A SEARCH UID command failed.");
		client_close(client);
		return false;
	}
	else if (!check_imap_client_close_logout(client, 4, errmsg)) {

		if (st_empty(errmsg)) st_sprint(errmsg, "A LOGOUT or CLOSE command failed.");
		client_close(client);
		return false;
	}

	// Disable logging again.
	log_disable();

	client_close(client);
	return true;
}
