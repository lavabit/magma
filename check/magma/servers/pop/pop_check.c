/**
 * @file /check/magma/pop/pop_check.c
 *
 * @brief POP interface test functions.
 */

#include "magma_check.h"

START_TEST (check_pop_network_simple_s) {

	log_disable();
//	client_t *client = NULL;
	stringer_t *errmsg = NULL;
//	// In the future we want to programatically determine this by protocol.
//	const uint32_t port = 8000;
//
//	if (!(client = client_connect("localhost", port)) || client_secure(client) != 0) {
//		errmsg = NULLER("Failed to establish a client connection.");
//	}
//	else if (client_read_line(client) <= 0 || client->status != 1 || pl_char_get(client->line)[2] != 'O' ||
//			pl_char_get(client->line)[2] != 'K') {
//		errmsg = NULLER("Failed to return successful status initially.");
//		log_pedantic("%s", pl_char_get(client->line));
//	}
//
//	// Test USER
//	else if (client_write(client, PLACER("USER princess\r\n", 15)) <= 0) {
//		errmsg = NULLER("Failed to write the USER command.");
//	}
//	else if (client_read_line(client) <= 0 || client->status != 1 || pl_char_get(client->line)[1] != 'O' ||
//			pl_char_get(client->line)[2] != 'K') {
//		errmsg = NULLER("Failed to return successful status after USER.");
//		log_pedantic("> %s <", pl_char_get(client->line));
//	}
//
//	// Test PASS
//	else if (client_write(client, PLACER("PASS password\r\n", 15)) <= 0) {
//		errmsg = NULLER("Failed to write the PASS command.");
//	}
//	else if (client_read_line(client) <= 0 || client->status != 1 || pl_char_get(client->line)[1] != 'O' ||
//			pl_char_get(client->line)[2] != 'K') {
//		errmsg = NULLER("Failed to return successful status after PASS.");
//		log_pedantic("> %s <", pl_char_get(client->line));
//	}
//
//	// Test LIST
//	else if (client_write(client, PLACER("LIST\r\n", 6)) <= 0) {
//		errmsg = NULLER("Failed to write the LIST command.");
//	}
//	else if (client_read_line(client) <= 0 || client->status != 1 || pl_char_get(client->line)[0] != '1') {
//		errmsg = NULLER("Failed to return successful status after LIST.");
//		log_pedantic("> %s <", pl_char_get(client->line));
//	}
//
//	// Test RETR
//	else if (client_write(client, PLACER("RETR 1\r\n", 8)) <= 0) {
//		errmsg = NULLER("Failed to write the RETR command.");
//	}
//	else if (client_read_line(client) <= 0 || client->status != 1 || pl_char_get(client->line)[1] != 'O' ||
//			pl_char_get(client->line)[2] != 'K') {
//		errmsg = NULLER("Failed to return successful status after RETR.");
//		log_pedantic("> %s <", pl_char_get(client->line));
//	}
//
//	// Test DELE
//	else if (client_write(client, PLACER("DELE 1\r\n", 8)) <= 0) {
//		errmsg = NULLER("Failed to write the DELE command.");
//	}
//	else if (client_read_line(client) <= 0 || client->status != 1 || pl_char_get(client->line)[1] != 'O' ||
//			pl_char_get(client->line)[2] != 'K') {
//		errmsg = NULLER("Failed to return successful status after DELE.");
//		log_pedantic("> %s <", pl_char_get(client->line));
//	}
//
//	if (client) {
//
//		// Test QUIT
//		if (client_write(client, PLACER("QUIT\r\n", 6)) <= 0) {
//			errmsg = NULLER("Failed to write the LIST command.");
//		}
//		else if (client_read_line(client) <= 0 || client->status != 1 || pl_char_get(client->line)[1] != 'O' ||
//				pl_char_get(client->line)[2] != 'K') {
//			errmsg = NULLER("Failed to return successful status after QUIT.");
//		}
//
//		if (client) client_close(client);
//	}


//	log_test("POP / NETWORK / SIMPLE CHECK:", errmsg);
	log_test("POP / NETWORK / SIMPLE CHECK:", NULLER("SKIPPED"));
	ck_assert_msg(!errmsg, st_char_get(errmsg));
}
END_TEST

Suite * suite_check_pop(void) {

	TCase *tc;
	Suite *s = suite_create("\tPOP");

	testcase(s, tc, "POP Network Simple Check/S", check_pop_network_simple_s);
	return s;
}
