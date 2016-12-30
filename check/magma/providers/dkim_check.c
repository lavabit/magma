
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

extern DKIM_LIB *dkim_engine;

bool_t check_dkim_verify_sthread(stringer_t *errmsg) {

	/// LOW: Write a unit tests that will verify sample messages, using hard coded DKIM
	/// signatures, and keys, so they can perform a functional test without access to
	/// the internet.

	DKIM *context;
	DKIM_STAT status;
	struct dkim_test_dns_data *td;
	stringer_t *id = NULL, *data = NULL;
	uint32_t checked = 0, max = check_message_max();

	//bazinga._domainkey.magmadaemon.com descriptive text

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

		// We only attempt to verify signatures for messages with the verify flag set.
		else if (check_message_dkim_verify(i)) {

			// Create a new handle to verify the signed message.
			if (!(context = dkim_verify_d(dkim_engine, st_data_get(id), NULL, &status)) || status != DKIM_STAT_OK) {
				st_sprint(errmsg, "Allocation of the DKIM verification context failed. { %sstatus = %s }",
					context ? "" : "dkim_verify = NULL / ", dkim_getresultstr_d(status));

				if (context) dkim_free_d(context);
				st_free(data);
				st_free(id);

				return false;
			}

			// Push the following public key onto the stack for verification purposes. Note that the public key passed in
			// must match the message being checked.
			if (dkim_test_dns_put_d(context, C_IN, T_TXT, 0, (uchr_t *)"bazinga._domainkey.magmadaemon.com",
				(uchr_t *)"v=DKIM1; k=rsa; p=MIIBIjANBgkqhkiG9w0BAQEFAAOCAQ8AMIIBCgKCAQEAu0AH7Y9nzercUWi5Qqt"
				"4UUvKg8iRx1WJnGVVCCLYBQ2F4GgbFhs8w1tqGE7\\Ouaea/IH2v6K3bzM54/GYTmPLBX41krRX6AhTnMN66Qyc3RJR"
				"cmiHXB+DIrLbpja5inrlErt2PO4SWSsr0s2Az+rTr4AkXdE7+Lsbwg br48QCGwCLVikgrTR9GSqDtWbLRWks7HPiEx"
				"ADpfpru4amaX0CWs5DaANbM/ujJvddXeZBAsV9zpGK+tLMoSrYzZ+TdHE5/2TuK9SlC+UAS1oUexrbt7d7hepVmmVoJ"
				"4g/Me3x8AASGhNIK55TCG4u6/jEUVXIpAlTZTdTKM/i+BB/Z22+3wIDAQAB") != DKIM_STAT_OK) {

				st_sprint(errmsg, "Unable to push the DKIM public key onto the DNS resolver stack.");
				dkim_free_d(context);
				st_free(data);
				st_free(id);

				return false;
			}

			// Handle the message as a chunk, then finalize the input by passing in a NULL chunk and calling the
			// end-of-message function.
			if ((status = dkim_chunk_d(context, st_data_get(data), st_length_get(data))) == DKIM_STAT_OK
				&& (status = dkim_chunk_d(context, NULL, 0)) == DKIM_STAT_OK) {
				status = dkim_eom(context, NULL);
			}


//			if (context->dkim_dnstestt->dns_query) {
//				DKIM_FREE(context, context->dkim_dnstestt->dns_query);
//				context->dkim_dnstestt->dns_query = NULL;
//			}

			dkim_free_d(context);

			if (status != DKIM_STAT_OK) {
				st_sprint(errmsg, "Found a DKIM signature but verification of its validity failed. { status = %s }",
					dkim_getresultstr_d(status));
				st_free(data);
				st_free(id);
				return false;
			}

			// We keep track of how many messages we actually checked, because we want the test to
			// fail if we ever end up without any suitable test data.
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
