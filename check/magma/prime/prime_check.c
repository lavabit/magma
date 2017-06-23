
/**
 * @file /check/magma/prime/prime_check.c
 *
 * @brief The heart of the suite of unit tests for the Magma provide module.
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
	stringer_t *errmsg = MANAGEDBUF(1024);
	prime_ephemeral_chunk_t *get = NULL, *set = NULL;

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
	prime_encrypted_chunk_t *chunk = NULL;
	prime_chunk_keys_t encrypt_keys, decrypt_keys;
	ed25519_key_t *signing_pub = NULL, *signing_priv = NULL;
	prime_chunk_keks_t *encrypt_keks = NULL, *decrypt_keks = NULL;
	stringer_t *errmsg = MANAGEDBUF(1024), *data = NULL, *set = NULL;
	secp256k1_key_t *encryption_pub = NULL, *encryption_priv = NULL, *recipient_pub = NULL, *recipient_priv = NULL;

	if (status()) {

		// We add spaces between the categories to increase the number of randomly placed whitespace.
		data = rand_choices("0123456789 abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ " \
			"+!@#$%^&*()_|\"?><{}>~`<[]/.,;'\\-=\n\t", 1024, MANAGEDBUF(1024));

		if (!(signing_priv = ed25519_generate()) || !(encryption_priv = secp256k1_generate()) || !(recipient_priv = secp256k1_generate()) ||
			!(signing_pub = ed25519_public_set(ed25519_public_get(signing_priv, MANAGEDBUF(32)))) ||
			!(encryption_pub = secp256k1_public_set(secp256k1_public_get(encryption_priv, MANAGEDBUF(33)))) ||
			!(recipient_pub = secp256k1_public_set(secp256k1_public_get(recipient_priv, MANAGEDBUF(33))))) {
			st_sprint(errmsg, "Key generation failed.");
			result = false;
		}

		mm_wipe(&encrypt_keys, sizeof(prime_chunk_keys_t));
		mm_wipe(&decrypt_keys, sizeof(prime_chunk_keys_t));

		encrypt_keys.signing = signing_priv;
		encrypt_keys.encryption = encryption_priv;
		encrypt_keys.recipient = recipient_pub;

		decrypt_keys.signing = signing_pub;
		decrypt_keys.encryption = encryption_pub;
		decrypt_keys.recipient = recipient_priv;

		if (result && (!(encrypt_keks = keks_get(&encrypt_keys, NULL)) || !(decrypt_keks = keks_set(&decrypt_keys, NULL)) ||
			st_cmp_cs_eq(encrypt_keks->recipient, decrypt_keks->recipient))) {
			st_sprint(errmsg, "Encrypted chunk kek creation failed.");
			result = false;
		}

		// Test chunk creation using an ephemeral signing/encryption key, and a recipient public key.
		else if (result && !(chunk = encrypted_chunk_set(PRIME_CHUNK_COMMON, signing_priv, encrypt_keks, PRIME_CHUNK_FLAG_NONE, data))) {
			st_sprint(errmsg, "Encrypted chunk creation failed.");
			result = false;
		}
		else if (result && !(set = encrypted_chunk_get(signing_pub, decrypt_keks, encrypted_chunk_buffer(chunk), MANAGEDBUF(1024), NULL))) {
			st_sprint(errmsg, "Encrypted chunk parsing failed.");
			result = false;
		}
		else if (result && st_cmp_cs_eq(data, set)) {
			st_sprint(errmsg, "Encrypted chunk comparison failed. The output isn't identical to the input.");
			result = false;
		}

		// Cleanup and reset the pointers so we don't trigger a double free.
		encrypted_chunk_cleanup(chunk);
		chunk = NULL;

		// Randomize the test data.
		data = rand_choices("0123456789 abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ " \
			"+!@#$%^&*()_|\"?><{}>~`<[]/.,;'\\-=\n\t", 1024, MANAGEDBUF(1024));

		if (result && !(chunk = encrypted_chunk_set(PRIME_CHUNK_HEADERS, signing_priv, encrypt_keks, PRIME_CHUNK_FLAG_NONE, data))) {
			st_sprint(errmsg, "Encrypted chunk creation failed.");
			result = false;
		}
		else if (result && !(set = encrypted_chunk_get(signing_pub, decrypt_keks, encrypted_chunk_buffer(chunk), MANAGEDBUF(1024), NULL))) {
			st_sprint(errmsg, "Encrypted chunk parsing failed.");
			result = false;
		}
		else if (result && st_cmp_cs_eq(data, set)) {
			st_sprint(errmsg, "Encrypted chunk comparison failed. The output isn't identical to the input.");
			result = false;
		}

		// Cleanup and reset the pointers so we don't trigger a double free.
		encrypted_chunk_cleanup(chunk);
		chunk = NULL;


		// Randomize the test data.
		data = rand_choices("0123456789 abcdefghijklmnopqrstuvwxyz ABCDEFGHIJKLMNOPQRSTUVWXYZ " \
			"+!@#$%^&*()_|\"?><{}>~`<[]/.,;'\\-=\n\t", 1024, MANAGEDBUF(1024));

		if (result && !(chunk = encrypted_chunk_set(PRIME_CHUNK_BODY, signing_priv, encrypt_keks, PRIME_CHUNK_FLAG_NONE, data))) {
			st_sprint(errmsg, "Encrypted chunk creation failed.");
			result = false;
		}
		else if (result && !(set = encrypted_chunk_get(signing_pub, decrypt_keks, encrypted_chunk_buffer(chunk), MANAGEDBUF(1024), NULL))) {
			st_sprint(errmsg, "Encrypted chunk parsing failed.");
			result = false;
		}
		else if (result && st_cmp_cs_eq(data, set)) {
			st_sprint(errmsg, "Encrypted chunk comparison failed. The output isn't identical to the input.");
			result = false;
		}

		// Cleanup and reset the pointers so we don't trigger a double free.
		encrypted_chunk_cleanup(chunk);
		chunk = NULL;

		keks_cleanup(encrypt_keks);
		keks_cleanup(decrypt_keks);
		if (signing_pub) ed25519_free(signing_pub);
		if (signing_priv) ed25519_free(signing_priv);
		if (recipient_pub) secp256k1_free(recipient_pub);
		if (recipient_priv) secp256k1_free(recipient_priv);
		if (encryption_pub) secp256k1_free(encryption_pub);
		if (encryption_priv) secp256k1_free(encryption_priv);
	}

	log_test("PRIME / CHUNKS / ENCRYPTED / SINGLE THREADED:", errmsg);
	ck_assert_msg(result, st_char_get(errmsg));

}
END_TEST

START_TEST (check_prime_chunk_signature_s) {

	log_disable();
	size_t length = 0;
	bool_t result = true;
	prime_signature_tree_t *tree = NULL;
	prime_ephemeral_chunk_t *ephemeral = NULL;
	prime_chunk_keys_t encrypt_keys, decrypt_keys;
	ed25519_key_t *signing_pub = NULL, *signing_priv = NULL;
	prime_chunk_keks_t *encrypt_keks = NULL, *decrypt_keks = NULL;
	prime_encrypted_chunk_t *common = NULL, *headers = NULL, *body = NULL;
	secp256k1_key_t *encryption_pub = NULL, *encryption_priv = NULL, *recipient_pub = NULL, *recipient_priv = NULL;
	stringer_t *errmsg = MANAGEDBUF(1024), *payload = MANAGEDBUF(1024), *treesig = NULL, *data = NULL, *full = NULL;

	if (status()) {

		if (!(signing_priv = ed25519_generate()) || !(encryption_priv = secp256k1_generate()) || !(recipient_priv = secp256k1_generate()) ||
			!(signing_pub = ed25519_public_set(ed25519_public_get(signing_priv, MANAGEDBUF(32)))) ||
			!(encryption_pub = secp256k1_public_set(secp256k1_public_get(encryption_priv, MANAGEDBUF(33)))) ||
			!(recipient_pub = secp256k1_public_set(secp256k1_public_get(recipient_priv, MANAGEDBUF(33))))) {
			st_sprint(errmsg, "Key generation failed.");
			result = false;
		}

		mm_wipe(&encrypt_keys, sizeof(prime_chunk_keys_t));
		mm_wipe(&decrypt_keys, sizeof(prime_chunk_keys_t));

		encrypt_keys.signing = signing_priv;
		encrypt_keys.encryption = encryption_priv;
		encrypt_keys.recipient = recipient_pub;

		decrypt_keys.signing = signing_pub;
		decrypt_keys.encryption = encryption_pub;
		decrypt_keys.recipient = recipient_priv;

		if (result && (!(encrypt_keks = keks_get(&encrypt_keys, NULL)) || !(decrypt_keks = keks_set(&decrypt_keys, NULL)) ||
			st_cmp_cs_eq(encrypt_keks->recipient, decrypt_keks->recipient))) {
			st_sprint(errmsg, "Signature chunk kek creation failed.");
			result = false;
		}

		rand_write(payload);

		tree = signature_tree_alloc();

		ephemeral = ephemeral_chunk_get(signing_priv, encryption_priv);
		if (signature_tree_add(tree, ephemeral_chunk_buffer(ephemeral))) {
			st_sprint(errmsg, "Tree signature creation failed.");
			result = false;
		}

		common = encrypted_chunk_set(PRIME_CHUNK_COMMON, signing_priv, encrypt_keks, PRIME_CHUNK_FLAG_NONE, payload);
		if (signature_tree_add(tree, encrypted_chunk_buffer(common))) {
			st_sprint(errmsg, "Tree signature creation failed.");
			result = false;
		}

		headers = encrypted_chunk_set(PRIME_CHUNK_HEADERS, signing_priv, encrypt_keks, PRIME_CHUNK_FLAG_NONE, payload);
		if (signature_tree_add(tree, encrypted_chunk_buffer(headers))) {
			st_sprint(errmsg, "Tree signature creation failed.");
			result = false;
		}

		body = encrypted_chunk_set(PRIME_CHUNK_BODY, signing_priv, encrypt_keks, PRIME_CHUNK_FLAG_NONE, payload);
		if (signature_tree_add(tree, encrypted_chunk_buffer(body))) {
			st_sprint(errmsg, "Tree signature creation failed.");
			result = false;
		}

		if (!(treesig = signature_tree_get(signing_priv, tree, encrypt_keks))) {
			st_sprint(errmsg, "Tree signature retrieval failed.");
			result = false;
		}

		length = st_write(NULL, ephemeral_chunk_buffer(ephemeral), encrypted_chunk_buffer(common), encrypted_chunk_buffer(headers),
			encrypted_chunk_buffer(body));

		if (!(data = st_alloc(length + 512)) || st_write(data, ephemeral_chunk_buffer(ephemeral), encrypted_chunk_buffer(common), encrypted_chunk_buffer(headers),
			encrypted_chunk_buffer(body), treesig) != length + 161) {
			st_sprint(errmsg, "Serialized message creation failed.");
			result = false;
		}

		full = signature_full_get(PRIME_SIGNATURE_USER, signing_priv, encrypt_keks, data);
		st_append(data, full);
		st_cleanup(full);

		full = signature_full_get(PRIME_SIGNATURE_DESTINATION, signing_priv, encrypt_keks, data);
		st_append(data, full);

		ephemeral_chunk_cleanup(ephemeral);
		encrypted_chunk_cleanup(common);
		encrypted_chunk_cleanup(headers);
		encrypted_chunk_cleanup(body);
		signature_tree_cleanup(tree);
		st_cleanup(full, data, treesig);

		keks_cleanup(encrypt_keks);
		keks_cleanup(decrypt_keks);
		if (signing_pub) ed25519_free(signing_pub);
		if (signing_priv) ed25519_free(signing_priv);
		if (recipient_pub) secp256k1_free(recipient_pub);
		if (recipient_priv) secp256k1_free(recipient_priv);
		if (encryption_pub) secp256k1_free(encryption_pub);
		if (encryption_priv) secp256k1_free(encryption_priv);
	}

	log_test("PRIME / CHUNKS / SIGNATURE / SINGLE THREADED:", errmsg);
	ck_assert_msg(result, st_char_get(errmsg));

}
END_TEST

START_TEST (check_prime_chunk_spanning_s) {

	//log_disable();
	log_enable();

	bool_t result = true;
	prime_encrypted_chunk_t *chunk = NULL;
	prime_chunk_keys_t encrypt_keys, decrypt_keys;
	ed25519_key_t *signing_pub = NULL, *signing_priv = NULL;
	prime_chunk_keks_t *encrypt_keks = NULL, *decrypt_keks = NULL;
	stringer_t *errmsg = MANAGEDBUF(1024), *data = NULL, *buffer = NULL, *set = NULL;
	secp256k1_key_t *encryption_pub = NULL, *encryption_priv = NULL, *recipient_pub = NULL, *recipient_priv = NULL;

	if (status()) {

		if (!(signing_priv = ed25519_generate()) || !(encryption_priv = secp256k1_generate()) || !(recipient_priv = secp256k1_generate()) ||
			!(signing_pub = ed25519_public_set(ed25519_public_get(signing_priv, MANAGEDBUF(32)))) ||
			!(encryption_pub = secp256k1_public_set(secp256k1_public_get(encryption_priv, MANAGEDBUF(33)))) ||
			!(recipient_pub = secp256k1_public_set(secp256k1_public_get(recipient_priv, MANAGEDBUF(33))))) {
			st_sprint(errmsg, "Key generation failed.");
			result = false;
		}

		mm_wipe(&encrypt_keys, sizeof(prime_chunk_keys_t));
		mm_wipe(&decrypt_keys, sizeof(prime_chunk_keys_t));

		encrypt_keys.signing = signing_priv;
		encrypt_keys.encryption = encryption_priv;
		encrypt_keys.recipient = recipient_pub;

		decrypt_keys.signing = signing_pub;
		decrypt_keys.encryption = encryption_pub;
		decrypt_keys.recipient = recipient_priv;

		if (result && (!(encrypt_keks = keks_get(&encrypt_keys, NULL)) || !(decrypt_keks = keks_set(&decrypt_keys, NULL)) ||
			st_cmp_cs_eq(encrypt_keks->recipient, decrypt_keks->recipient))) {
			st_sprint(errmsg, "Encrypted chunk kek creation failed.");
			result = false;
		}

		// Generate a large (> 16mb) random buffer for the message body.
		else if (result && !(data = rand_choices("0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz", 20971520, NULL))) {
			st_sprint(errmsg, "Unable to generate a large random payload for testing spanning chunks.");
			result = false;

		}

		// Test spanning chunk creation using an ephemeral signing/encryption key, and a recipient public key.
		else if (result && !(chunk = part_encrypt(PRIME_CHUNK_BODY, signing_priv, encrypt_keks, data))) {
			st_sprint(errmsg, "Encrypted spanning chunk creation failed.");
			result = false;
		}

		// Serialize the spanning chunks into a single buffer.
		else if (result && !(buffer = part_buffer(chunk))) {
			st_sprint(errmsg, "Encrypted spanning chunk serialization failed.");
			result = false;
		}

		else if (result && !(set = part_decrypt(signing_pub, decrypt_keks, buffer, NULL, NULL))) {
			st_sprint(errmsg, "Encrypted spanning chunk parsing and decryption failed.");
			result = false;
		}
		else if (result && st_cmp_cs_eq(data, set)) {
			st_sprint(errmsg, "Encrypted spanning chunks failed with random alphabet data. The input doesn't match the output.");
			result = false;
		}

		// Clean up after the first test.
		encrypted_chunk_cleanup(chunk);
		st_cleanup(buffer, set);
		set = buffer = NULL;
		chunk = NULL;

		// Now generate a spanning chunk using random binary data.
		if (result && rand_write(data) != 20971520) {
			st_sprint(errmsg, "Unable to generate a large random payload filled with binary data for testing spanning binary chunks.");
			result = false;
		}

		// Test spanning chunk creation using an ephemeral signing/encryption key, and a recipient public key.
		else if (result && !(chunk = part_encrypt(PRIME_CHUNK_BODY, signing_priv, encrypt_keks, data))) {
			st_sprint(errmsg, "Encrypted spanning chunk creation failed.");
			result = false;
		}

		// Serialize the spanning chunks into a single buffer.
		else if (result && !(buffer = part_buffer(chunk))) {
			st_sprint(errmsg, "Encrypted spanning chunk serialization failed.");
			result = false;
		}

		else if (result && !(set = part_decrypt(signing_pub, decrypt_keks, buffer, NULL, NULL))) {
			st_sprint(errmsg, "Encrypted spanning chunk parsing and decryption failed.");
			result = false;
		}
		else if (result && st_cmp_cs_eq(data, set)) {
			st_sprint(errmsg, "Encrypted spanning chunks failed with random binary data. The input doesn't match the output.");
			result = false;
		}

		encrypted_chunk_cleanup(chunk);
		st_cleanup(data, buffer, set);
		set = data = buffer = NULL;
		chunk = NULL;

		keks_cleanup(encrypt_keks);
		keks_cleanup(decrypt_keks);
		if (signing_pub) ed25519_free(signing_pub);
		if (signing_priv) ed25519_free(signing_priv);
		if (recipient_pub) secp256k1_free(recipient_pub);
		if (recipient_priv) secp256k1_free(recipient_priv);
		if (encryption_pub) secp256k1_free(encryption_pub);
		if (encryption_priv) secp256k1_free(encryption_priv);
	}

	log_test("PRIME / CHUNKS / SPANNING / SINGLE THREADED:", errmsg);
	ck_assert_msg(result, st_char_get(errmsg));
}
END_TEST

START_TEST (check_prime_chunk_compressed_s) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	/// HIGH: Handle the different chunk flags properly. Specifically add support for compressed chunk payloads.

	log_test("PRIME / CHUNKS / COMPRESSED / SINGLE THREADED:", NULLER("SKIPPED"));
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_prime_chunk_padding_s) {

	log_disable();
	bool_t outcome = true;
	stringer_t *errmsg = MANAGEDBUF(1024);

	/// HIGH: Handle the different chunk flags properly. Specifically add support for the alternate padding algorithm.

	log_test("PRIME / CHUNKS / PADDING / SINGLE THREADED:", NULLER("SKIPPED"));
	ck_assert_msg(outcome, st_char_get(errmsg));
}
END_TEST

START_TEST (check_prime_message_naked_s) {

	//log_disable();
	log_enable();
	bool_t result = true;
	uint32_t max = check_message_max();
	stringer_t *raw = NULL, *message = NULL, *rebuilt = NULL, *errmsg = MANAGEDBUF(1024);
	prime_t *destination = NULL, *org = NULL, *recipient = NULL, *request = NULL, *signet = NULL;

	if (status()) {

		if (!(destination = prime_key_generate(PRIME_ORG_KEY, NONE)) || !(org = prime_signet_generate(destination)) ||
			!(recipient = prime_key_generate(PRIME_USER_KEY, NONE)) || !(request = prime_request_generate(recipient, NULL)) ||
			!(signet = prime_request_sign(request, destination))) {
			st_sprint(errmsg, "PRIME message test identity generation failed.");
			result = false;
		}

		for (uint32_t i = 0; i < max && result; i++) {

			if (!(raw = check_message_get(i)) || !(message = prime_message_encrypt(raw, NULL, NULL, destination, signet))) {
				st_sprint(errmsg, "PRIME message encryption test failed.");
				result = false;
			}
			else if (!(rebuilt = prime_message_decrypt(message, org, recipient))) {
				st_sprint(errmsg, "PRIME message decryption test failed.");
				result = false;
			}
			else if (st_cmp_cs_eq(raw, rebuilt)) {
				st_sprint(errmsg, "PRIME message encryption before and after comparison test failed.");
				result = false;
			}

			st_cleanup(message, rebuilt, raw);
			raw = rebuilt = message;
		}

		prime_cleanup(destination);
		prime_cleanup(recipient);
		prime_cleanup(request);
		prime_cleanup(signet);
		prime_cleanup(org);

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

	Suite *s = suite_create("\tPRIME");

	suite_check_testcase(s, "PRIME", "STACIE/S", check_stacie_s);

	suite_check_testcase(s, "PRIME", "PRIME ed25519/S", check_prime_ed25519_s);
	suite_check_testcase(s, "PRIME", "PRIME secp256k1/S", check_prime_secp256k1_s);
	suite_check_testcase(s, "PRIME", "PRIME Primitives/S", check_prime_primitives_s);
	suite_check_testcase(s, "PRIME", "PRIME Keys/S", check_prime_keys_s);
	suite_check_testcase(s, "PRIME", "PRIME Signets/S", check_prime_signets_s);

	suite_check_testcase(s, "PRIME", "PRIME Ephemeral Chunks/S", check_prime_chunk_ephemeral_s);
	suite_check_testcase(s, "PRIME", "PRIME Encrypted Chunks/S", check_prime_chunk_encrypted_s);
	suite_check_testcase(s, "PRIME", "PRIME Signature Chunks/S", check_prime_chunk_signature_s);
	suite_check_testcase(s, "PRIME", "PRIME Spanning Chunks/S", check_prime_chunk_spanning_s);

	suite_check_testcase(s, "PRIME", "PRIME Chunk Padding Algorithms/S", check_prime_chunk_padding_s);
	suite_check_testcase(s, "PRIME", "PRIME Chunk Compression/S", check_prime_chunk_compressed_s);

	suite_check_testcase(s, "PRIME", "PRIME Naked Messages/S", check_prime_message_naked_s);
	suite_check_testcase(s, "PRIME", "PRIME Native Messages/S", check_prime_message_native_s);

	return s;
}

