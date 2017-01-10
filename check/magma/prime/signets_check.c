
/**
 * @file /magma/check/magma/prime/signets_check.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma_check.h"

/**
 * @file /magma/check/magma/prime/keys_check.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma_check.h"

bool_t check_prime_signets_org_sthread(stringer_t *errmsg) {

	prime_t *org = NULL, *signet = NULL, *binary = NULL, *armored = NULL;

	// Create an org key and then generate the corresponding signet.
	if (!(org = prime_key_generate(PRIME_ORG_KEY)) || !(signet = prime_signet_generate(org))) {
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

	log_unit("%.*s", st_length_int(armored), st_char_get(armored));

	prime_free(signet);
	prime_free(org);

	return true;
}

bool_t check_prime_signets_user_sthread(stringer_t *errmsg) {

	prime_t *org = NULL, *verify = NULL, *user1 = NULL, *user2 = NULL, *request1 = NULL, *request2 = NULL,
		*signet1 = NULL, *signet2 = NULL, *binary = NULL, *armored = NULL;

	// Create an org key.
	if (!(org = prime_key_generate(PRIME_ORG_KEY)) || !(verify = prime_signet_generate(org))) {
		st_sprint(errmsg, "Organizational signet/key for user signing failed.");
		prime_cleanup(org);
		return false;
	}

	// Create a user key and then generate the corresponding signing request.
	else if (!(user1 = prime_key_generate(PRIME_USER_KEY)) || !(request1 = prime_request_generate(user1, NULL))) {
		st_sprint(errmsg, "User key/signing request creation failed.");
		prime_cleanup(user1);
		prime_free(verify);
		prime_free(org);
		return false;
	}

	// Signing the user request.
	else if (!(signet1 = prime_request_sign(request1, org))) {
		st_sprint(errmsg, "User key/signing request creation failed.");
		prime_free(request1);
		prime_free(verify);
		prime_free(user1);
		prime_free(org);
		return false;
	}


	// Serialize the org signet.
	else if (!(binary = prime_get(signet1, BINARY, MANAGEDBUF(256)))) {
		st_sprint(errmsg, "User signet serialization failed.");
		prime_free(request1);
		prime_free(signet1);
		prime_free(verify);
		prime_free(user1);
		prime_free(org);
		return false;
	}

	// Serialize and armor the org signet.
	else if (!(armored = prime_get(signet1, ARMORED, MANAGEDBUF(512)))) {
		st_sprint(errmsg, "User signet armoring failed.");
		prime_free(request1);
		prime_free(signet1);
		prime_free(verify);
		prime_free(user1);
		prime_free(org);
		return false;
	}

	else if (!(user2 = prime_key_generate(PRIME_USER_KEY)) || !(request2 = prime_request_generate(user2, user1)) ||
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

	log_unit("%.*s", st_length_int(armored), st_char_get(armored));

	prime_free(request2);
	prime_free(request1);
	prime_free(signet2);
	prime_free(signet1);
	prime_free(verify);
	prime_free(user2);
	prime_free(user1);
	prime_free(org);

	return true;
}

bool_t check_prime_signets_parameters_sthread(stringer_t *errmsg) {
//#error "Write unit tests. Ensure we test that 0 length payloads will work."
	return true;

}
