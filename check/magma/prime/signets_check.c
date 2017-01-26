
/**
 * @file /check/magma/prime/signets_check.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 */

#include "magma_check.h"

bool_t check_prime_signets_org_sthread(stringer_t *errmsg) {

	prime_t *org = NULL, *signet = NULL;
	stringer_t *fingerprint = NULL, *binary = NULL, *armored = NULL, *signet2 = NULL, *signet3 = NULL;

	// Create an org key and then generate the corresponding signet.
	if (!(org = prime_key_generate(PRIME_ORG_KEY, NONE)) || !(signet = prime_signet_generate(org))) {
		st_sprint(errmsg, "Organizational signet/key creation failed.");
		prime_cleanup(org);
		return false;
	}

	// Serialize the org signet.
	else if (!(binary = prime_get(signet, BINARY, MANAGEDBUF(256)))) {
		st_sprint(errmsg, "Organizational signet serialization failed.");
		prime_free(signet);
		prime_free(org);
		return false;
	}

	// Serialize and armor the org signet.
	else if (!(armored = prime_get(signet, ARMORED, MANAGEDBUF(512)))) {
		st_sprint(errmsg, "Organizational signet armoring failed.");
		prime_free(signet);
		prime_free(org);
		return false;
	}
	else if (!(signet2 = prime_set(binary, BINARY,  NONE)) || !(signet3 = prime_set(armored, ARMORED,  NONE))) {
		st_sprint(errmsg, "Organizational signet parsing failed.");
		prime_cleanup(signet2);
		prime_free(signet);
		prime_free(org);
		return false;
	}

	// Validate the org signet.
	else if (!prime_signet_validate(signet, NULL)) {
		st_sprint(errmsg, "Organizational signet validation failed.");
		prime_free(signet3);
		prime_free(signet2);
		prime_free(signet);
		prime_free(org);
		return false;
	}

	// Fingerprint the org signet.
	else if (!(fingerprint = prime_signet_fingerprint(signet, MANAGEDBUF(64)))) {
		st_sprint(errmsg, "Organizational signet fingerprinting failed.");
		prime_free(signet3);
		prime_free(signet2);
		prime_free(signet);
		prime_free(org);
		return false;
	}

//  Hack to easily generate a new org identity.

	log_enable();
	log_pedantic("%.*s", st_length_int(armored), st_char_get(armored));
	fingerprint = prime_get(org, ARMORED, MANAGEDBUF(1024));
	log_pedantic("%.*s", st_length_int(fingerprint), st_char_get(fingerprint));
	log_disable();

	prime_free(signet3);
	prime_free(signet2);
	prime_free(signet);
	prime_free(org);

	return true;
}

bool_t check_prime_signets_user_sthread(stringer_t *errmsg) {

	prime_t *org = NULL, *verify = NULL, *user1 = NULL, *user2 = NULL, *request1 = NULL, *request2 = NULL,
		*signet1 = NULL, *signet2 = NULL, *signet3 = NULL, *signet4 = NULL;
	stringer_t *fingerprint = NULL, *binary = NULL, *armored = NULL;


	// Create an org key.
	if (!(org = prime_key_generate(PRIME_ORG_KEY, NONE)) || !(verify = prime_signet_generate(org))) {
		st_sprint(errmsg, "Organizational signet/key for user signing failed.");
		prime_cleanup(org);
		return false;
	}

	// Create a user key and then generate the corresponding signing request.
	else if (!(user1 = prime_key_generate(PRIME_USER_KEY, NONE)) || !(request1 = prime_request_generate(user1, NULL))) {
		st_sprint(errmsg, "User key/signing request creation failed.");
		prime_cleanup(user1);
		prime_free(verify);
		prime_free(org);
		return false;
	}

	// Sign the user request.
	else if (!(signet1 = prime_request_sign(request1, org))) {
		st_sprint(errmsg, "User key/signing request signing failed.");
		prime_free(request1);
		prime_free(verify);
		prime_free(user1);
		prime_free(org);
		return false;
	}


	// Serialize the user signet.
	else if (!(binary = prime_get(signet1, BINARY, MANAGEDBUF(512)))) {
		st_sprint(errmsg, "User signet serialization failed.");
		prime_free(request1);
		prime_free(signet1);
		prime_free(verify);
		prime_free(user1);
		prime_free(org);
		return false;
	}

	// Serialize and armor the user signet.
	else if (!(armored = prime_get(signet1, ARMORED, MANAGEDBUF(512)))) {
		st_sprint(errmsg, "User signet armoring failed.");
		prime_free(request1);
		prime_free(signet1);
		prime_free(verify);
		prime_free(user1);
		prime_free(org);
		return false;
	}

	else if (!(user2 = prime_key_generate(PRIME_USER_KEY, NONE)) || !(request2 = prime_request_generate(user2, user1)) ||
		!(signet2 = prime_request_sign(request2, org))) {
		st_sprint(errmsg, "User signet/key rotation failed.");
		prime_cleanup(request2);
		prime_cleanup(user2);
		prime_free(request1);
		prime_free(signet1);
		prime_free(verify);
		prime_free(user1);
		prime_free(org);
		return false;
	}

	else if (!(signet3 = prime_set(binary, BINARY,  NONE)) || !(signet4 = prime_set(armored, ARMORED,  NONE))) {
		st_sprint(errmsg, "User signet parsing failed.");
		prime_cleanup(signet3);
		prime_free(request2);
		prime_free(request1);
		prime_free(signet2);
		prime_free(signet1);
		prime_free(verify);
		prime_free(user2);
		prime_free(user1);
		prime_free(org);
		return false;
	}

	// Ensure we can generate cryptographic fingerprints.
	else if (!prime_signet_validate(request1, NULL) || !prime_signet_validate(signet1, NULL) || !prime_signet_validate(request2, NULL) ||
		!prime_signet_validate(signet2, NULL) || !prime_signet_validate(signet1, verify) || !prime_signet_validate(signet2, verify) ||
		!prime_signet_validate(request2, signet1) || !prime_signet_validate(signet2, signet1)) {
		st_sprint(errmsg, "User signet validation failed.");
		prime_free(request2);
		prime_free(request1);
		prime_free(signet4);
		prime_free(signet3);
		prime_free(signet2);
		prime_free(signet1);
		prime_free(verify);
		prime_free(user2);
		prime_free(user1);
		prime_free(org);
		return false;
	}

	// Ensure we can generate cryptographic fingerprints.
	else if (!(fingerprint = prime_signet_fingerprint(signet1, MANAGEDBUF(64))) || !(fingerprint = prime_signet_fingerprint(signet2, MANAGEDBUF(64)))) {
		st_sprint(errmsg, "User signet fingerprinting failed.");
		prime_free(request2);
		prime_free(request1);
		prime_free(signet4);
		prime_free(signet3);
		prime_free(signet2);
		prime_free(signet1);
		prime_free(verify);
		prime_free(user2);
		prime_free(user1);
		prime_free(org);
		return false;
	}

	prime_free(request2);
	prime_free(request1);
	prime_free(signet4);
	prime_free(signet3);
	prime_free(signet2);
	prime_free(signet1);
	prime_free(verify);
	prime_free(user2);
	prime_free(user1);
	prime_free(org);

	return true;
}

bool_t check_prime_signets_parameters_sthread(stringer_t *errmsg) {

	stringer_t *holder = NULL, *rand1 = MANAGEDBUF(32), *rand2 = MANAGEDBUF(128), *rand3 = MANAGEDBUF(64),
		*encrypted_key = NULL;
	prime_t *org_key = NULL, *org_signet = NULL, *user_key = NULL, *user_request = NULL, *user_signet = NULL,
		*rotation_key = NULL, *rotation_request = NULL, *rotation_signet = NULL, *check = NULL;

	// Create various PRIME types for use below.
	if (rand_write(rand1) != 32 || rand_write(rand2) != 128 || rand_write(rand3) != 64 ||
		!(org_key = prime_key_generate(PRIME_ORG_KEY, NONE)) || !(org_signet = prime_signet_generate(org_key)) ||
		!(user_key = prime_key_generate(PRIME_USER_KEY, NONE)) || !(user_request = prime_request_generate(user_key, NULL)) ||
		!(user_signet = prime_request_sign(user_request, org_key)) || !(rotation_key = prime_key_generate(PRIME_USER_KEY, NONE)) ||
		!(rotation_request = prime_request_generate(rotation_key, user_key)) || !(rotation_signet = prime_request_sign(rotation_request, org_key)) ||
		!(encrypted_key = prime_key_encrypt(rand3, user_key, ARMORED, MANAGEDBUF(512)))) {
		st_sprint(errmsg, "Signet/key creation for parameter testing failed.");
		prime_cleanup(org_key);
		prime_cleanup(org_signet);
		prime_cleanup(user_key);
		prime_cleanup(user_request);
		prime_cleanup(user_signet);
		prime_cleanup(rotation_key);
		prime_cleanup(rotation_request);
		prime_cleanup(rotation_signet);
		return false;
	}

	else if ((check = prime_signet_generate(user_key)) || (check = prime_signet_generate(user_request)) || (check = prime_signet_generate(user_signet)) ||
		(check = prime_request_generate(org_key, NULL)) || (check = prime_request_generate(org_signet, NULL)) || (check = prime_request_generate(user_request, NULL)) ||
		(check = prime_request_generate(user_signet, NULL)) || (check = prime_request_sign(org_key, user_key)) || (check = prime_request_sign(org_key, user_request)) ||
		(check = prime_request_sign(org_key, user_signet)) || (check = prime_request_sign(org_key, org_signet)) || (check = prime_request_sign(user_key, org_key)) ||
		(check = prime_request_sign(user_signet, org_key)) || prime_signet_validate(user_key, NULL) || prime_signet_validate(org_key, NULL) ||
		prime_signet_validate(org_signet, org_key) || prime_signet_validate(rotation_request, org_key) || prime_signet_validate(rotation_request, user_key) ||
		prime_signet_validate(org_signet, user_key) || prime_signet_validate(org_signet, user_signet) || prime_signet_validate(user_request, org_key) ||
		prime_signet_validate(user_request, user_key) || prime_signet_validate(user_request, org_signet) || prime_signet_validate(user_request, rotation_key) ||
		prime_signet_validate(user_signet, org_key) || prime_signet_validate(user_signet, user_key) || prime_signet_validate(user_signet, rotation_signet) ||
		prime_signet_validate(user_signet, user_request) || (check = prime_key_generate(PRIME_USER_SIGNET, NONE)) || (check = prime_key_generate(PRIME_ORG_SIGNET, NONE)) ||
		(check = prime_key_generate(PRIME_USER_SIGNING_REQUEST, NONE)) || (check = prime_get(NULL, BINARY, NULL)) ||
		(check = prime_get(NULL, BINARY, MANAGEDBUF(512))) || (check = prime_set(NULL, BINARY, NONE)) || (check = prime_set(NULL, ARMORED, NONE)) ||
		(holder = prime_signet_fingerprint(org_key, MANAGEDBUF(64))) || (holder = prime_signet_fingerprint(user_key, MANAGEDBUF(64))) ||
		(holder = prime_signet_fingerprint(user_request, MANAGEDBUF(64))) || (holder = prime_signet_fingerprint(org_signet, CONSTANT("TEST"))) ||
		(holder = prime_key_encrypt(NULL, org_key, BINARY, MANAGEDBUF(512))) || (holder = prime_key_encrypt(rand1, org_key, BINARY, MANAGEDBUF(512))) ||
		(holder = prime_key_encrypt(rand2, org_key, BINARY, MANAGEDBUF(512))) || (holder = prime_key_encrypt(rand3, org_signet, BINARY, MANAGEDBUF(512))) ||
		(holder = prime_key_encrypt(rand3, NULL, BINARY, MANAGEDBUF(512))) || (holder = prime_key_decrypt(NULL, encrypted_key, ARMORED, NONE)) ||
		(holder = prime_key_decrypt(rand3, NULL, ARMORED, NONE)) || (holder = prime_key_decrypt(rand1, encrypted_key, ARMORED, NONE)) ||
		(holder = prime_key_decrypt(rand2, encrypted_key, ARMORED, NONE)) || (holder = prime_key_decrypt(rand3, encrypted_key, BINARY, NONE)) ||
		(holder = prime_key_decrypt(rand3, prime_get(org_signet, BINARY, MANAGEDBUF(512)), BINARY, NONE)) ||
		(holder = prime_key_decrypt(rand3, prime_get(user_request, BINARY, MANAGEDBUF(512)), BINARY, NONE)) ||
		(holder = prime_key_decrypt(rand3, prime_get(user_signet, BINARY, MANAGEDBUF(512)), BINARY, NONE))) {

		st_sprint(errmsg, "Signet/key parameter checks failed.");
		prime_cleanup(check);
		prime_free(org_key);
		prime_free(org_signet);
		prime_free(user_key);
		prime_free(user_request);
		prime_free(user_signet);
		prime_free(rotation_key);
		prime_free(rotation_request);
		prime_free(rotation_signet);
	}

	prime_free(org_key);
	prime_free(org_signet);
	prime_free(user_key);
	prime_free(user_request);
	prime_free(user_signet);
	prime_free(rotation_key);
	prime_free(rotation_request);
	prime_free(rotation_signet);

	return true;
}
