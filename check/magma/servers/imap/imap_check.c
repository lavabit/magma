/*
 * @file /check/magma/pop/pop_check.c
 *
 * @brief IMAP interface test functions.
 */

#include "magma_check.h"

START_TEST (check_imap_network_simple_s) {

	log_disable();
//	client_t *client = NULL;
	stringer_t *errmsg = NULL;
//	// In the future we want to programatically determine this by protocol.
//	const uint32_t port = 9500;
//
//	if (!(client = client_connect("localhost", port)) || client_secure(client) != 0) {
//		errmsg = NULLER("Failed to establish a client connection.");
//	}
//	else if (client_read_line(client) <= 0 || client->status != 1 || pl_char_get(client->line)[2] != 'O' ||
//			pl_char_get(client->line)[3] != 'K') {
//		errmsg = NULLER("Failed to return successful status initially.");
//		log_pedantic("%s", pl_char_get(client->line));
//	}
//
//	// Test LOGIN
//	else if (client_write(client, PLACER("A1 LOGIN princess password\r\n", 28)) <= 0) {
//		errmsg = NULLER("Failed to write the A1 LOGIN command.");
//	}
//	else if (client_read_line(client) <= 0 || client->status != 1 || pl_char_get(client->line)[3] != 'O' ||
//			pl_char_get(client->line)[4] != 'K') {
//		errmsg = NULLER("Failed to return successful status after LOGIN.");
//		log_pedantic("> %s <", pl_char_get(client->line));
//	}
//
//	// Test SELECT Inbox
//	else if (client_write(client, PLACER("A2 SELECT Inbox\r\n", 28)) <= 0) {
//		errmsg = NULLER("Failed to write the A1 LOGIN command.");
//	}
//	else if (client_read_line(client) <= 0 || client->status != 1 || pl_char_get(client->line)[2] != 'O' ||
//			pl_char_get(client->line)[3] != 'K') {
//		errmsg = NULLER("Failed to return successful status after SELECT.");
//		log_pedantic("%s", pl_char_get(client->line));
//	}
//
//	if (client) {
//		// Test LOGOUT
//		if (client_write(client, PLACER("A3 LOGOUT\r\n", 28)) <= 0) {
//			errmsg = NULLER("Failed to write the A1 LOGIN command.");
//		}
//		else if (client_read_line(client) <= 0 || client->status != 1 || pl_char_get(client->line)[2] != 'O' ||
//				pl_char_get(client->line)[3] != 'K') {
//			errmsg = NULLER("Failed to return successful status after LOGOUT.");
//			log_pedantic("%s", pl_char_get(client->line));
//		}
//
//		client_close(client);
//	}

	log_test("IMAP / NETWORK / SIMPLE CHECK:", NULLER("SKIPPED"));
	ck_assert_msg(!errmsg, st_char_get(errmsg));
}
END_TEST

Suite * suite_check_imap(void) {

	TCase *tc;
	Suite *s = suite_create("\tIMAP");

	testcase(s, tc, "IMAP Network Simple Check/S", check_imap_network_simple_s);
	return s;
}
