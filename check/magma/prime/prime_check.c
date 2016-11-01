
/**
 * @file /check/providers/provide_check.c
 *
 * @brief The heart of the suite of unit tests for the Magma provide module.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma_check.h"

//! STACIE Tests
START_TEST (check_stacie_s) {

	log_disable();
	bool_t result = true;
	stringer_t *errmsg = NULL;

	if (status() && !(result = check_stacie_parameters())) {
		errmsg = NULLER("STACIE parameter checks failed.");
	}
	else if (status() && result && !(result = check_stacie_determinism())) {
		errmsg = NULLER("STACIE checks to ensure a deterministic outcome failed.");
	}
	else if (status() && result && !(result = check_stacie_rounds())) {
		errmsg = NULLER("STACIE round calculation checks failed.");
	}
	else if (status() && result && !(result = check_stacie_simple())) {
		errmsg = NULLER("STACIE failed to produce the expected result using the hard coded input values.");
	}
	else if (status() && result && !(result = check_stacie_bitflip())) {
		errmsg = NULLER("The STACIE encryption scheme failed to detect tampering of an encrypted buffer.");
	}

	log_test("PRIME / STACIE / SINGLE THREADED:", errmsg);
	ck_assert_msg(result, st_char_get(errmsg));
}
END_TEST

//! PRIME Tests
START_TEST (check_prime_ed25519_s) {

	log_disable();
	bool_t result = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (status()) result = check_prime_ed25519_keys_sthread(errmsg);

	log_test("PRIME / ED25519 / SINGLE THREADED:", errmsg);
	ck_assert_msg(result, st_char_get(errmsg));

}
END_TEST

START_TEST (check_prime_secp256k1_s) {

	log_disable();
	bool_t result = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (status()) result = check_prime_secp256k1_parameters_sthread(errmsg);
	if (status() && result) result = check_prime_secp256k1_fixed_sthread(errmsg);
	if (status() && result) result = check_prime_secp256k1_keys_sthread(errmsg);

	log_test("PRIME / SECP256K1 / SINGLE THREADED:", errmsg);
	ck_assert_msg(result, st_char_get(errmsg));

}
END_TEST

START_TEST (check_prime_keys_s) {

	log_disable();
	bool_t result = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (status()) result = check_prime_keys_sthread(errmsg);

	log_test("PRIME / KEYS / SINGLE THREADED:", errmsg);
	ck_assert_msg(result, st_char_get(errmsg));

}
END_TEST

START_TEST (check_prime_writers_s) {

	log_disable();
	bool_t result = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (status()) result = check_prime_writers_sthread(errmsg);

	log_test("PRIME / WRITERS / SINGLE THREADED:", errmsg);
	ck_assert_msg(result, st_char_get(errmsg));

}
END_TEST

Suite * suite_check_prime(void) {

	TCase *tc;
	Suite *s = suite_create("\tPRIME");

	testcase(s, tc, "STACIE/S", check_stacie_s);

	testcase(s, tc, "PRIME ed25519/S", check_prime_ed25519_s);
	testcase(s, tc, "PRIME secp256k1/S", check_prime_secp256k1_s);
	testcase(s, tc, "PRIME Writers/S", check_prime_writers_s);
	testcase(s, tc, "PRIME Keys/S", check_prime_keys_s);

	tcase_set_timeout(tc, 120);

	return s;
}

