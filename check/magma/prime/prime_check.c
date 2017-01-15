
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

	if (status()) result = check_prime_ed25519_parameters_sthread(errmsg);
	if (status() && result) result = check_prime_ed25519_fixed_sthread(errmsg);
	if (status() && result) result = check_prime_ed25519_fuzz_lib_sthread(errmsg);
	if (status() && result) result = check_prime_ed25519_fuzz_provider_sthread(errmsg);

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

START_TEST (check_prime_signets_s) {

	log_disable();
	bool_t result = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (status()) result = check_prime_signets_org_sthread(errmsg);
	if (status() && result) result = check_prime_signets_user_sthread(errmsg);
	if (status() && result) result = check_prime_signets_parameters_sthread(errmsg);

	log_test("PRIME / SIGNETS / SINGLE THREADED:", errmsg);
	ck_assert_msg(result, st_char_get(errmsg));

}
END_TEST

START_TEST (check_prime_keys_s) {

	log_disable();
	bool_t result = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (status()) result = check_prime_keys_org_sthread(errmsg);
	if (status() && result) result = check_prime_keys_user_sthread(errmsg);
	if (status() && result) result = check_prime_keys_parameters_sthread(errmsg);

	log_test("PRIME / KEYS / SINGLE THREADED:", errmsg);
	ck_assert_msg(result, st_char_get(errmsg));

}
END_TEST

START_TEST (check_prime_primitives_s) {

	log_disable();
	bool_t result = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (status()) result = check_prime_writers_sthread(errmsg);
	if (status() && result) result = check_prime_unpacker_sthread(errmsg);
	if (status() && result) result = check_prime_armor_sthread(errmsg);

	log_test("PRIME / PRIMITIVES / SINGLE THREADED:", errmsg);
	ck_assert_msg(result, st_char_get(errmsg));

}
END_TEST

START_TEST (check_prime_chunk_ephemeral_s) {

	log_disable();
	bool_t result = true;
	ed25519_key_t *signing = NULL;
	secp256k1_key_t *encryption = NULL;
	prime_ephemeral_chunk_t *get = NULL, *set = NULL;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (status()) {

		if (!(signing = ed25519_generate()) || !(encryption = secp256k1_generate())) {
			st_sprint(errmsg, "Key generation failed.");

			result = false;
		}

		// Test chunk operations with just the encryption key.
		else if (!(get = ephemeral_chunk_get(NULL, encryption))) {
			st_sprint(errmsg, "Ephemeral chunk creation failed.");
			result = false;
		}
		else if (!(set = ephemeral_chunk_set(ephemeral_chunk_buffer(get)))) {
			st_sprint(errmsg, "Ephemeral chunk parsing failed.");
			result = false;
		}

		ephemeral_chunk_cleanup(get);
		ephemeral_chunk_cleanup(set);

		// Reset.
		get = set = NULL;

		// Test chunk operations with an encryption key and a signing key.
		if (result && !(get = ephemeral_chunk_get(signing, encryption))) {
			st_sprint(errmsg, "Ephemeral chunk creation failed.");
			result = false;
		}
		else if (result && !(set = ephemeral_chunk_set(ephemeral_chunk_buffer(get)))) {
			st_sprint(errmsg, "Ephemeral chunk parsing failed.");
			result = false;
		}

		if (signing) ed25519_free(signing);
		if (encryption) secp256k1_free(encryption);

		ephemeral_chunk_cleanup(get);
		ephemeral_chunk_cleanup(set);

	}

	log_test("PRIME / CHUNKS / EPHEMERAL / SINGLE THREADED:", errmsg);
	ck_assert_msg(result, st_char_get(errmsg));

}
END_TEST

START_TEST (check_prime_chunk_encrypted_s) {

	log_disable();
	bool_t result = true;
	ed25519_key_t *signing = NULL;
	secp256k1_key_t *encryption = NULL, *recipient = NULL;
	prime_encrypted_chunk_t *get = NULL, *set = NULL;
	stringer_t *errmsg = MANAGEDBUF(1024), *data = NULL;

	if (status()) {

		// We add spaces between the categories to increase the number of randomly placed whitespace.
		data = rand_choices("0123456789 abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ " \
			"+!@#$%^&*()_|\"?><{}>~`<[]/.,;'\\-=\n\t", 1024, MANAGEDBUF(1024));

		if (!(signing = ed25519_generate()) || !(encryption = secp256k1_generate()) || !(recipient = secp256k1_generate())) {
			st_sprint(errmsg, "Key generation failed.");
			result = false;
		}

		// Test chunk operations with just the encryption key.
//		else if (!(get = ephemeral_chunk_get(NULL, encryption))) {
//			st_sprint(errmsg, "Ephemeral chunk creation failed.");
//			result = false;
//		}
//		else if (!(set = ephemeral_chunk_set(ephemeral_chunk_buffer(get)))) {
//			st_sprint(errmsg, "Ephemeral chunk parsing failed.");
//			result = false;
//		}
//
//		ephemeral_chunk_cleanup(get);
//		ephemeral_chunk_cleanup(set);
//
//		// Reset.
//		get = set = NULL;
//
//		// Test chunk operations with an encryption key and a signing key.
//		if (result && !(get = ephemeral_chunk_get(signing, encryption))) {
//			st_sprint(errmsg, "Ephemeral chunk creation failed.");
//			result = false;
//		}
//		else if (result && !(set = ephemeral_chunk_set(ephemeral_chunk_buffer(get)))) {
//			st_sprint(errmsg, "Ephemeral chunk parsing failed.");
//			result = false;
//		}

		if (signing) ed25519_free(signing);
		if (encryption) secp256k1_free(encryption);
		if (recipient) secp256k1_free(recipient);

		encrypted_chunk_cleanup(get);
		encrypted_chunk_cleanup(set);

	}

	log_test("PRIME / CHUNKS / ENCRYPTED / SINGLE THREADED:", errmsg);
	ck_assert_msg(result, st_char_get(errmsg));

}
END_TEST

START_TEST (check_prime_chunk_signature_s) {

	log_disable();
	bool_t result = true;
	ed25519_key_t *signing = NULL;
	secp256k1_key_t *encryption = NULL;
	prime_signature_chunk_t *get = NULL, *set = NULL;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (status()) {

		if (!(signing = ed25519_generate()) || !(encryption = secp256k1_generate())) {
			st_sprint(errmsg, "Key generation failed.");

			result = false;
		}

		if (signing) ed25519_free(signing);
		if (encryption) secp256k1_free(encryption);

		signature_chunk_cleanup(get);
		signature_chunk_cleanup(set);

	}

	log_test("PRIME / CHUNKS / SIGNATURE / SINGLE THREADED:", errmsg);
	ck_assert_msg(result, st_char_get(errmsg));

}
END_TEST

START_TEST (check_prime_message_naked_s) {

	log_disable();
	bool_t result = true;
	prime_t *message = NULL;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (status()) {

		message = prime_message_encrypt(NULL, NULL, NULL, NULL, NULL);
		prime_cleanup(message);
	}

	log_test("PRIME / MESSAGES / NAKED / SINGLE THREADED:", errmsg);
	ck_assert_msg(result, st_char_get(errmsg));

}
END_TEST

START_TEST (check_prime_message_native_s) {

	log_disable();
	bool_t result = true;
	prime_t *message = NULL;
	stringer_t *errmsg = MANAGEDBUF(1024);

	if (status()) {

		message = prime_message_encrypt(NULL, NULL, NULL, NULL, NULL);
		prime_cleanup(message);
	}

	log_test("PRIME / MESSAGES / NATIVE / SINGLE THREADED:", errmsg);
	ck_assert_msg(result, st_char_get(errmsg));

}
END_TEST

Suite * suite_check_prime(void) {

	TCase *tc;
	Suite *s = suite_create("\tPRIME");

	testcase(s, tc, "STACIE/S", check_stacie_s);

	testcase(s, tc, "PRIME ed25519/S", check_prime_ed25519_s);
	testcase(s, tc, "PRIME secp256k1/S", check_prime_secp256k1_s);
	testcase(s, tc, "PRIME Primitives/S", check_prime_primitives_s);
	testcase(s, tc, "PRIME Keys/S", check_prime_keys_s);
	testcase(s, tc, "PRIME Signets/S", check_prime_signets_s);

	testcase(s, tc, "PRIME Ephemeral Chunks/S", check_prime_chunk_ephemeral_s);
	testcase(s, tc, "PRIME Encrypted Chunks/S", check_prime_chunk_encrypted_s);
	testcase(s, tc, "PRIME Signature Chunks/S", check_prime_chunk_signature_s);

	testcase(s, tc, "PRIME Naked Messages/S", check_prime_message_naked_s);
	testcase(s, tc, "PRIME Native Messages/S", check_prime_message_native_s);


	tcase_set_timeout(tc, 120);

	return s;
}

