
/**
 * @file /check/magma/smtp/smtp_check.c
 *
 * @brief SMTP interface test functions.
 */

#include "magma_check.h"

START_TEST (check_smtp_network_simple_s) {

	log_disable();

//	FILE *buf_d;
	bool_t outcome = true;
//	const int port = 7000;
//	int socket_descriptor;
//	struct sockaddr_in pin;
	stringer_t *errmsg = NULL;
//	struct linger linger_timeout;
//	struct hostent *server_host_name;
//
//	server_host_name = gethostbyname("localhost");
//
//	mm_wipe(&pin, sizeof(pin));
//	pin.sin_family = AF_INET;
//	pin.sin_addr.s_addr = htonl(INADDR_ANY);
//	pin.sin_addr.s_addr = ((struct in_addr *) (server_host_name->h_addr))->s_addr;
//	pin.sin_port = htons(port);
//
//	if ((socket_descriptor = socket(AF_INET,SOCK_STREAM, 0)) == -1) {
//		outcome = false;
//		errmsg = NULLER("Failed to open socket.");
//	}
//
//	else if ((connect(socket_descriptor, (void *) &pin, sizeof(pin))) == -1) {
//		outcome = false;
//		errmsg = NULLER("Failed to connect to socket.");
//	}
//
//	else if (!(linger_timeout.l_onoff = 1) || !(linger_timeout.l_linger = 1) ||
//			setsockopt(socket_descriptor, SOL_SOCKET, SO_LINGER,
//			&linger_timeout, sizeof(linger_timeout)) != 0) {
//		outcome = false;
//		errmsg = NULLER("Failed to set main socket timeout.");
//	}
//
//	else {
//		buf_d = fdopen(socket_descriptor, "a+");
//		outcome = (buf_d, errmsg);
//		fclose(buf_d);
//		fflush(stdout);
//	}

	log_test("SMTP / NETWORK / SIMPLE CHECK:", NULLER("SKIPPED"));
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_smtp_accept_store_message_s) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = MANAGEDBUF(2048);

	outcome = check_smtp_accept_store_message_sthread(errmsg);

	log_test("SMTP / ACCEPT / STORAGE / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_smtp_accept_rollout_s) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	outcome = check_smtp_accept_rollout_sthread(errmsg);

	log_test("SMTP / ACCEPT / ROLLOUT / SINGLE THREADED:", NULLER("SKIPPED"));
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_smtp_accept_store_spamsig_s) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	outcome = check_smtp_accept_store_spamsig_sthread(errmsg);

	log_test("SMTP / ACCEPT / SIGNATURES / SINGLE THREADED:", NULLER("SKIPPED"));
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_smtp_checkers_greylist_s) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	outcome = check_smtp_checkers_greylist_sthread(errmsg);

	log_test("SMTP / GREYLIST / SINGLE THREADED:", errmsg);
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

Suite * suite_check_smtp(void) {

	TCase *tc;
	Suite *s = suite_create("\tSMTP");

	testcase(s, tc, "SMTP / Accept/S", check_smtp_accept_store_message_s);
	testcase(s, tc, "SMTP / Rollout/S", check_smtp_accept_rollout_s);
	testcase(s, tc, "SMTP / Signatres/S", check_smtp_accept_store_spamsig_s);
	testcase(s, tc, "SMTP / Greylist/S", check_smtp_checkers_greylist_s);
	return s;
}


