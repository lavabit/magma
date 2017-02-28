
/**
 * @file /check/magma/smtp/accept_check.c
 *
 * @brief SMTP accept test functions.
 */

#include "magma_check.h"

bool_t check_smtp_accept_store_message_sthread(stringer_t *errmsg) {

	prime_t *prime;
	bool_t outcome = true;
	stringer_t *data= NULL;
	smtp_inbound_prefs_t prefs;
	uint32_t max = check_message_max();
	uint64_t messagenums[max], messagesizes[max], fail_count = 0;
	uint64_t messages_checkpoint = serial_get(OBJECT_MESSAGES, 1);

	mm_wipe(&prefs, sizeof(smtp_inbound_prefs_t));

	if (!(prime = prime_alloc(PRIME_USER_SIGNET, NONE))) {
		outcome = false;
		st_sprint(errmsg, "Failed to allocate signet.");
	}

	else {
		prime->signet.user = user_signet_alloc();
		prime->signet.user->encryption = secp256k1_generate();
	}

	// try using improperly formed prefs or null data
	if (outcome && smtp_store_message(&prefs, &data) != -1) {
		outcome = false;
		st_sprint(errmsg, "Failed to return -1 when given improperly formed prefs or null data.");
	}
	else {
		prefs.usernum = 1;
		prefs.foldernum = 1;
		prefs.signum = 0;
		prefs.spamkey = 0;
	}

	for (uint32_t i = 0; outcome && status() && i < max-2; i+=2) {

		prefs.signet = NULL;

		if (!(data = check_message_get(i))) {
			outcome = false;
			st_sprint(errmsg, "Failed to get the message data. { message = %i }", i);
		}

		else if (outcome && (smtp_store_message(&prefs, &data) != 1)) {
			outcome = false;
			st_cleanup(data);
			st_sprint(errmsg, "Failed to store naked message.");
		}

		else if ((messagenums[i] = prefs.messagenum) && (messagesizes[i] = sizeof(*data)) &&
				serial_get(OBJECT_MESSAGES, prefs.usernum) != (messages_checkpoint += 1)) {
			outcome = false;
			st_cleanup(data);
			st_sprint(errmsg, "Failed to increment messages checkpoint value.");
		}

		if (outcome && !(data = check_message_get(i+1))) {
			outcome = false;
			st_cleanup(data);
			st_sprint(errmsg, "Failed to get the message data. { message = %i }", i+1);
		}

		else if ((prefs.signet = prime) && smtp_store_message(&prefs, &data) != 1) {
			outcome = false;
			st_cleanup(data);
			st_sprint(errmsg, "Failed to store encrypted message.");
		}

		else if ((messagenums[i+1] = prefs.messagenum) && (messagesizes[i+1] = sizeof(*data)) &&
				serial_get(OBJECT_MESSAGES, prefs.usernum) != (messages_checkpoint += 1)) {
			outcome = false;
			st_cleanup(data);
			st_sprint(errmsg, "Failed to increment messages checkpoint value.");
		}

		else {
			st_cleanup(data);
		}
	}

	// remove messages to see if they existed
	for (size_t i = 0; i < max-2; i++) {
		if (!(mail_remove_message(1, messagenums[i], messagesizes[i], NULL))) fail_count++;
	}

	if (fail_count) {
		outcome = false;
		st_sprint(errmsg, "Failed to remove message(s) that should exist. { fail_count = %lu }", fail_count);
	}

	if (prime) prime_cleanup(prime);

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

// TODO
bool_t check_smtp_accept_accept_message_sthread(stringer_t *errmsg) {
	return true;
}
































