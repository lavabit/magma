
/**
 * @file /check/smtp/smtp_check.c
 *
 * @brief SMTP interface test functions.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma_check.h"

START_TEST (check_smtp_forced_s) {

	stringer_t *errmsg = NULL;

	if (!status()) {
		log_test("SERVER / SMTP / FORCED / SINGLE THREADED:", errmsg);
		return;
	}

	for (uint32_t i = 0; i < MAGMA_SERVER_INSTANCES; i++) {

		if (magma.servers[i] && magma.servers[i]->tls.forced && magma.servers[i]->protocol == SMTP) {

			// Check the SMTP server instances to make sure they are forcing the use of TLS when forced is set to true.

			// Since the test hasn't been written yet, we hard code a fail.
			errmsg = st_aprint("SMTP server instances failed to require transport security with the forced flag enabled.");

		}

	}

	log_test("SERVER / SMTP / FORCED / SINGLE THREADED:", errmsg);
	fail_unless(!errmsg, st_char_get(errmsg));
	st_cleanup(errmsg);
} END_TEST

START_TEST (check_pop_forced_s) {

	stringer_t *errmsg = NULL;

	if (!status()) {
		log_test("SERVER / POP / FORCED / SINGLE THREADED:", errmsg);
		return;
	}

	for (uint32_t i = 0; i < MAGMA_SERVER_INSTANCES; i++) {

		if (magma.servers[i] && magma.servers[i]->tls.forced && magma.servers[i]->protocol == POP) {

			// Check the POP server instances to make sure they are forcing the use of TLS when forced is set to true.

			// Since the test hasn't been written yet, we hard code a fail.
			errmsg = st_aprint("POP server instances failed to require transport security with the forced flag enabled.");

		}

	}

	log_test("SERVER / POP / FORCED / SINGLE THREADED:", errmsg);
	fail_unless(!errmsg, st_char_get(errmsg));
	st_cleanup(errmsg);
} END_TEST


START_TEST (check_imap_forced_s) {

	stringer_t *errmsg = NULL;

	if (!status()) {
		log_test("SERVER / IMAP / FORCED / SINGLE THREADED:", errmsg);
		return;
	}

	for (uint32_t i = 0; i < MAGMA_SERVER_INSTANCES; i++) {

		if (magma.servers[i] && magma.servers[i]->tls.forced && magma.servers[i]->protocol == IMAP) {

			// Check the IMAP server instances to make sure they are forcing the use of TLS when forced is set to true.

			// Since the test hasn't been written yet, we hard code a fail.
			errmsg = st_aprint("IMAP server instances failed to require transport security with the forced flag enabled.");

		}

	}

	log_test("SERVER / IMAP / FORCED / SINGLE THREADED:", errmsg);
	fail_unless(!errmsg, st_char_get(errmsg));
	st_cleanup(errmsg);
} END_TEST

START_TEST (check_submission_forced_s) {

	stringer_t *errmsg = NULL;

	if (!status()) {
		log_test("SERVER / SUBMISSION / FORCED / SINGLE THREADED:", errmsg);
		return;
	}

	for (uint32_t i = 0; i < MAGMA_SERVER_INSTANCES; i++) {

		if (magma.servers[i] && magma.servers[i]->tls.forced && magma.servers[i]->protocol == SUBMISSION) {

			// Check the submission server instances to make sure they are forcing the use of TLS when forced is set to true.

			// Since the test hasn't been written yet, we hard code a fail.
			errmsg = st_aprint("Submission server instances failed to require transport security with the forced flag enabled.");

		}

	}

	log_test("SERVER / SUBMISSION / FORCED / SINGLE THREADED:", errmsg);
	fail_unless(!errmsg, st_char_get(errmsg));
	st_cleanup(errmsg);
} END_TEST

Suite * suite_check_servers(void) {

	TCase *tc;
	Suite *s = suite_create("\tServers");

	testcase(s, tc, "Servers / SMTP / Force Transport Security/S", check_smtp_forced_s);
	testcase(s, tc, "Servers / POP / Force Transport Security/S", check_pop_forced_s);
	testcase(s, tc, "Servers / IMAP / Force Transport Security/S", check_imap_forced_s);
	testcase(s, tc, "Servers / SUBMISSION / Force Transport Security/S", check_submission_forced_s);

	return s;
}


