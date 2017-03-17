/**
 * @file /check/magma/servers/smtp/auth_check.c
 *
 * @brief SMTP auth test functions.
 */

#include "magma_check.h"

bool_t check_smtp_auth_from_field_sthread(stringer_t *errmsg) {

	client_t *client = NULL;

	// Connect the client.
	if (!(client = client_connect("localhost", port)) || client_read_line(client) <= 0 ||
		client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("220")) ||
		!st_search_cs(&(client->line), NULLER(" ESMTP "), &location)) {

		st_sprint(errmsg, "Failed to connect with the SMTP server.");
		client_close(client);
		return false;
	}
	// Issue EHLO.
	else if (client_print(client, "EHLO localhost\r\n") != 16 || !check_smtp_client_read_line_to_end(client) ||
		client_status(client) != 1 || st_cmp_cs_starts(&(client->line), NULLER("250"))) {

		st_sprint(errmsg, "Failed to return successful status after EHLO.");
		client_close(client);
		return false;
	}

	return true;
}
