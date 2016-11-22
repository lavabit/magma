
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

	/// LOW: Write a unit test that will verify sample, hard coded DKIM signatures without
	/// relying on the DNS system for a public key record (and thus work without an internet
	/// connection, etc).
	return true;
}

bool_t check_dkim_sign_sthread(stringer_t *errmsg) {

	uint32_t max = check_message_max();
	stringer_t *id = NULL, *data = NULL, *signature = NULL;

	for (uint32_t i = 0; i < max; i++) {

		if (!(id = rand_choices("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789", 12))) {
			st_sprint(errmsg, "Failed to generate the message id.");
			return false;
		}
		else if (!(data = check_message_get(i))) {
			st_sprint(errmsg, "Failed to get the message data.");
			st_free(id);
			return false;
		}
		else if (!(signature = dkim_create(id, data))) {
			st_sprint(errmsg, "Failed to generate the message signature.");
			st_free(data);
			st_free(id);
			return false;
		}

		st_free(signature);
		st_free(data);
		st_free(id);
	}

	return true;
}
