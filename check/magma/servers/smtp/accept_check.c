
/**
 * @file /check/magma/smtp/accept_check.c
 *
 * @brief SMTP accept test functions.
 */

#include "magma_check.h"

bool_t check_smtp_accept_store_message_sthread(stringer_t *errmsg) {

	bool_t outcome = true;
	stringer_t *data = NULL;
	uint32_t max = check_message_max();
	prime_t *key = NULL, *request = NULL, *signet = NULL;
	uint64_t messagenums[max], messagesizes[max], fail_count = 0;
	uint64_t messages_checkpoint = serial_get(OBJECT_MESSAGES, 1);
	smtp_inbound_prefs_t prefs;

	mm_wipe(&messagenums, sizeof(messagenums));
	mm_wipe(&prefs, sizeof(smtp_inbound_prefs_t));

	prefs.usernum = 1;
	prefs.foldernum = 1;

	// Try using improperly formed prefs or NULL data.
	if (smtp_store_message(&prefs, &data) != -1) {
		st_sprint(errmsg, "Failed to return -1 when given improperly formed prefs or null data.");
		return false;
	}

	// Generate an ephemeral signet for use in this test case.
	if (!(key = prime_key_generate(PRIME_USER_KEY, NONE)) || !(request = prime_request_generate(key, NULL)) ||
			!(signet = prime_request_sign(request, org_key))) {
		st_sprint(errmsg, "Failed to allocate signet.");
		prime_cleanup(request);
		prime_cleanup(signet);
		prime_cleanup(key);
		return false;
	}

	for (uint32_t i = 0; outcome && status() && i < max; i++) {

		// For odd message numbers, use encryption.
		if (i % 2) {
			prefs.signet = signet;
		}
		else {
			prefs.signet = NULL;
		}

		// Now grab the message data and store it.
		if (!(data = check_message_get(i))) {
			st_sprint(errmsg, "Failed to get the message data. { message = %i }", i);
			outcome = false;
		}

		else if (smtp_store_message(&prefs, &data) != 1) {
			st_sprint(errmsg, "Failed to store naked message.");
			outcome = false;
		}
		else if ((messagenums[i] = prefs.messagenum) && (messagesizes[i] = sizeof(*data)) &&
				serial_get(OBJECT_MESSAGES, prefs.usernum) != (messages_checkpoint += 1)) {
			st_sprint(errmsg, "Failed to increment messages checkpoint value.");
			outcome = false;
		}

		st_cleanup(data);
		data = NULL;
	}

	// remove messages to see if they existed
	for (size_t i = 0; i < max; i++) {
		if (messagenums[i] != 0 && !(mail_remove_message(1, messagenums[i], messagesizes[i], NULL))) fail_count++;
	}

	if (fail_count) {
		st_sprint(errmsg, "Failed to remove message(s) that should exist. { fail_count = %lu }", fail_count);
		outcome = false;
	}

	prime_free(request);
	prime_free(signet);
	prime_free(key);

	return outcome;
}

// TODO
bool_t check_smtp_accept_rollout_sthread(stringer_t *errmsg) {
	return true;
}

// TODO
bool_t check_smtp_accept_store_spamsig_sthread(stringer_t *errmsg) {
	return true;
}
































