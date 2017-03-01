/**
 * @file /check/magma/config/config_check.c
 *
 * @brief Check the magma config logic and related functions.
 */

#include "magma_check.h"

START_TEST (check_config_server_get_by_protocol_s) {

	log_disable();
	bool_t outcome = true;
	server_t *server = NULL;
	stringer_t *errmsg = NULL;
	uint32_t protocols[] = { HTTP, POP, IMAP, SMTP };

	for (size_t i = 0; status() && outcome && i < sizeof(protocols)/sizeof(uint32_t); i++) {
		if (!(server = servers_get_by_protocol(protocols[i], false)) || server->network.type != TCP_PORT ||
				server->protocol != protocols[i]) {
			outcome = false;
			errmsg = NULLER("Failed to return a pointer to the correct server (TCP).");
		}

		else if (!(server = servers_get_by_protocol(protocols[i], true)) || server->network.type != TLS_PORT ||
				server->protocol != protocols[i]) {
			outcome = false;
			errmsg = NULLER("Failed to return a pointer to the correct server (TLS).");
		}
	}

	log_test("CONFIG / SERVER / PROTOCOL / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

Suite * suite_check_config(void) {

	TCase *tc;
	Suite *s = suite_create("\tConfig");

	testcase(s, tc, "Config / Protocol /S", check_config_server_get_by_protocol_s);

	return s;
}
