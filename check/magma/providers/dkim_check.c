
/**
 * @file /magma/check/magma/providers/dkim_check.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma_check.h"

bool_t check_dkim_verify_sthread(stringer_t *errmsg) {

	/// LOW: Write a unit tests that will verify sample messages, using hard coded DKIM
	/// signatures, and keys, so they can perform a functional test without access to
	/// the internet.

	stringer_t *id = NULL, *data = NULL;
	uint32_t checked = 0, max = check_message_max();

	//bazinga._domainkey.magmadaemon.com descriptive text "v=DKIM1\; k=rsa\; p=MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAu0AH7Y9nzercUWi5Qqt4UUvKg8iRx1WJnGVVCCLYBQ2F4GgbFhs8w1tqGE7Ouaea/IH2v6K3bzM54/GYTmPLBX41krRX6AhTnMN66Qyc3RJRcmiHXB+DIrLbpja5inrlErt2PO4SWSsr0s2Az+rTr4AkXdE7+Lsbwgbr" "48QCGwCLVikgrTR9GSqDtWbLRWks7HPiExADpfpru4amaX0CWs5DaANbM/ujJvddXeZBAsV9zpGK+tLMoSrYzZ+TdHE5/2TuK9SlC+UAS1oUexrbt7d7hepVmmVoJ4g/Me3x8AASGhNIK55TCG4u6/jEUVXIpAlTZTdTKM/i+BB/Z22+3wIDAQAB"

	for (uint32_t i = 0; i < max; i++) {


		if (!(id = rand_choices("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789", 12))) {
			st_sprint(errmsg, "Failed to generate the message id.");
			st_free(id);
			return false;
		}
		else if (!(data = check_message_get(i))) {
			st_sprint(errmsg, "Failed to get the message data. { message = %i }", i);
			st_free(id);
			return false;
		}

		// We only attempt to generate signatures for messages without an existing signature.
		else if (check_message_dkim_verify(i) && dkim_signature_verify(id, data) != 1) {
			st_sprint(errmsg, "Failed to verify the domain keys message signature. { message = %i }", i);
			st_free(data);
			st_free(id);
			return false;
		}

		// We keep track of how many messages we actually checked, because we want the test to
		// fail if we ever end up without any suitable test data.
		else if (check_message_dkim_verify(i)) {
			checked++;
		}

		st_free(data);
		st_free(id);
	}

	// Ensure at least one message was verified.
	if (!checked) {
		st_sprint(errmsg, "None of the test messages were suitable candidates for domain key verification.");
		return false;
	}

	return true;
}

bool_t check_dkim_sign_sthread(stringer_t *errmsg) {

	uint32_t checked = 0, max = check_message_max();
	stringer_t *id = NULL, *data = NULL, *signature = NULL;

	for (uint32_t i = 0; i < max; i++) {

		if (!(id = rand_choices("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789", 12))) {
			st_sprint(errmsg, "Failed to generate the message id.");
			return false;
		}
		else if (!(data = check_message_get(i))) {
			st_sprint(errmsg, "Failed to get the message data. { message = %i }", i);
			st_free(id);
			return false;
		}

		// We only attempt to generate signatures for messages without an existing signature. Note we have to
		// run the input messages through the cleanup function, otherwise messages without a proper CRLF line
		// endings will fail.
		else if (check_message_dkim_sign(i) && mail_message_cleanup(&data) && !(signature = dkim_signature_create(id, data))) {
			st_sprint(errmsg, "Failed to generate the domain keys message signature. { message = %i }", i);
			st_free(data);
			st_free(id);
			return false;
		}

		// We keep track of how many messages we actually checked, because we want the test to
		// fail if we ever end up without any suitable test data. Also, we only need to free the signature
		// if we actually tried to sign the message. Otherwise a non-NULL pointer from a previous loop will
		// cause a double free.
		else if (check_message_dkim_sign(i)) {
			st_free(signature);
			checked++;
		}

		st_free(data);
		st_free(id);
	}

	// Ensure at least one message was signed.
	if (!checked) {
		st_sprint(errmsg, "None of the test messages were suitable candidates for signing with the domain key.");
		return false;
	}

	return true;
}
