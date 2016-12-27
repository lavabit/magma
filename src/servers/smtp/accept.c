
/**
 * @file /magma/servers/smtp/accept.c
 *
 * @brief	Functions used to handle SMTP commands/actions.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Store a received SMTP message as a generic mail message, both on disk and in the database.
 * @see		mail_store_messages()
 * @return	-1 on failure or 1 on success.
 */
int_t smtp_store_message(smtp_inbound_prefs_t *prefs, stringer_t **local) {

	uint32_t status = 0;
	uint64_t messagenum;

	if (prefs->usernum == 0 || local == NULL || prefs->foldernum == 0) {
		log_pedantic("An invalid message or session was passed in for storage.");
		return -1;
	}

	if ((prefs->mark & SMTP_MARK_READ) == SMTP_MARK_READ) {
		status = MAIL_STATUS_SEEN;
	}
	else {
		status = MAIL_STATUS_RECENT;
	}

	// Label the message.
	if ((prefs->mark & SMTP_MARK_VIRUS) == SMTP_MARK_VIRUS) {
		status += MAIL_MARK_INFECTED;
	}
	else if ((prefs->mark & SMTP_MARK_PHISH) == SMTP_MARK_PHISH) {
		status += MAIL_MARK_PHISHING;
	}
	else if ((prefs->mark & SMTP_MARK_SPOOF) == SMTP_MARK_SPOOF) {
		status += MAIL_MARK_SPOOFED;
	}
	else if ((prefs->mark & SMTP_MARK_RBL) == SMTP_MARK_RBL) {
		status += MAIL_MARK_BLACKHOLED;
	}
	else if ((prefs->mark & SMTP_MARK_SPAM) == SMTP_MARK_SPAM) {
		status += MAIL_MARK_JUNK;
	}

	// Begin the transaction.
	if (user_lock(prefs->usernum) != 1) {
		log_pedantic("Could not lock the user account %lu.", prefs->usernum);
		return -1;
	}

	messagenum = mail_store_message(prefs->usernum, prefs->pubkey, prefs->foldernum, &status, prefs->signum, prefs->spamkey, *local);
	user_unlock(prefs->usernum);

	// Error check.
	if (messagenum == 0) {
		log_error("Unable to store message.");
		return -1;
	}

	// Increment the messages checkpoint_t so connected clients know there are new messages waiting.
	serial_increment(OBJECT_MESSAGES, prefs->usernum);

	// Set the output values.
	prefs->messagenum = messagenum;
	return 1;
}

/**
 * @brief Delete the oldest mail message owned by a user until their storage usage falls below their storage quota.
 * @param	prefs	a pointer to the specified user's inbound mail preferences data.
 * @return	1 on success or < 0 on failure, where
 *         -1: An error occurred retrieving the rollout message list from the database.
 *         -2: The user lock could not be acquired.
 */
int_t smtp_rollout(smtp_inbound_prefs_t *prefs) {

	int_t state;
	unsigned size;
	row_t *row;
	stringer_t *server;
	table_t *messages;

	// Lock the account.
	state = user_lock(prefs->usernum);

	if (state != 1) {
		log_pedantic("Could not lock the user account %lu.", prefs->usernum);
		return -2;
	}

	// Get a list of messages ready to be deleted.
	if (!(messages = smtp_fetch_rollmessages(prefs->usernum))) {
		user_unlock(prefs->usernum);
		return -1;
	}

	// Loop until the storage size is under the quota.
	while (prefs->stor_size >= prefs->quota) {

		// Fetch a row. If no more rows exist, free the result set and try fetching another 20 messages.
		if (!(row = res_row_next(messages))) {
			res_table_free(messages);

			if (!(messages = smtp_fetch_rollmessages(prefs->usernum))) {
				user_unlock(prefs->usernum);
				return -1;
			}

			if (!(row = res_row_next(messages))) {
				user_unlock(prefs->usernum);
				res_table_free(messages);
				return -1;
			}

		}

		// Store the message size.
		size = res_field_uint32(row, 1);

		// Store the server.
		if (!(server = res_field_string(row, 2))) {
			user_unlock(prefs->usernum);
			res_table_free(messages);
			return -1;
		}

		// Delete the message.
		if (!mail_remove_message(prefs->usernum, res_field_uint64(row, 0), size, st_char_get(server))) {
			user_unlock(prefs->usernum);
			res_table_free(messages);
			st_free(server);
			return -1;
		}

		// We don't need the server anymore.
		st_free(server);

		// Update the storage size.
		prefs->stor_size -= size;
	}

	user_unlock(prefs->usernum);
	res_table_free(messages);

	// Increment the messages checkpoint_t so connected clients know there are new messages waiting.
	serial_increment(OBJECT_MESSAGES, prefs->usernum);

	return 1;
}

/**
 * @brief	Generate a random key for a spam signature and store it in the database.
 * @param	prefs	the user's smtp inbound preferences object, with the spam signature field set.
 * @param	spam	the dspam return code associated with the spam signature.
 * @return	true if the key was inserted into the database successfully, or false on failure.
 */
bool_t smtp_store_spamsig(smtp_inbound_prefs_t *prefs, int_t spam) {

	// Store the variables here.
	uint64_t key = 0;

	// Generate a random number to use as an authentication key.
	key = rand_get_uint64();

	while (uint64_digits(key) < 7) {
		key = (key * key);
	}

	// Try inserting the spam signature.
	if (!(prefs->signum = smtp_insert_spamsig(prefs, key, spam))) {
		log_pedantic("Could not insert the spam signature into the database. smtp_insert_spamsig = %lu", prefs->signum);
		return false;
	}

	prefs->spamkey = key;
	//log_pedantic("Statistical filter signature stored. {signature = %lu / key = %lu}", prefs->signum, prefs->spamkey);

	return true;
}

int_t smtp_accept_message(connection_t *con, smtp_inbound_prefs_t *prefs) {

	int_t state;
	stringer_t *local;

	if (con == NULL || prefs == NULL) {
		log_pedantic("Sanity check failed.");
		return SMTP_OUTCOME_PERM_FAILURE;
	}

	// If this message is a bounce, and we have bounces turned off.
	if (prefs->bounces == 0 && !st_cmp_cs_eq(con->smtp.mailfrom, PLACER("<>", 2))) {
		smtp_update_receive_stats(con, prefs);
		return SMTP_OUTCOME_SUCESS;
	}

	// If the message is larger than the user is allowed to accept.
	if (st_length_get(con->smtp.message->text) > prefs->recv_size_limit) {
		return SMTP_OUTCOME_TEMP_OVERQUOTA;
	}

	// Set the foldernum, so that if it doesn't change, the message ends up in the inbox.
	prefs->foldernum = prefs->inbox;

	// Check quota. If the user is overquota, and not forwarding, check whether rollout is enabled.
	// If rollout is not enabled, or an error occurred, send back the appropriate response.
	if (prefs->overquota == 1 && prefs->forwarded == NULL) {
		if (prefs->rollout == 1) {
			state = smtp_rollout(prefs);
			if (state == -2) {
				return SMTP_OUTCOME_TEMP_LOCKED;
			}
			else if (state != 1) {
				return SMTP_OUTCOME_PERM_FAILURE;
			}
		}
		// If the message is coming from a bypassed server and from a blessed address, allow it through.
		else if (!(con->smtp.bypass && ((magma.admin.contact && !st_cmp_ci_eq(con->smtp.mailfrom, magma.admin.contact)) ||
				(magma.admin.abuse && !st_cmp_ci_eq(con->smtp.mailfrom, magma.admin.abuse))))) {
			return SMTP_OUTCOME_TEMP_OVERQUOTA;
		}
	}

	// Check the message for a virus. If vscanned is equal to ten, then the message is virus free.
	if ((prefs->virus == 1 || prefs->phish == 1) && con->smtp.checked.virus != 1) {

		// The message hasn't been scanned, or encountered an error during the last attempt, so rescan it now.
		if (con->smtp.checked.virus == 0 || con->smtp.checked.virus == -1) {
			con->smtp.checked.virus = state = virus_check(con->smtp.message->text);
		}
		else {
			state = con->smtp.checked.virus;
		}

		// If a virus was found.
		if (prefs->virus == 1 && state == -2) {
			if (prefs->virusaction == SMTP_ACTION_MARK_READ) {
				prefs->mark = SMTP_MARK_READ | SMTP_MARK_VIRUS;
			}
			else if (prefs->virusaction == SMTP_ACTION_MARK) {
				prefs->mark = SMTP_MARK_VIRUS;
			}
			else if (prefs->virusaction == SMTP_ACTION_BOUNCE) {
				return SMTP_OUTCOME_BOUNCE_VIRUS;
			}
			else if (prefs->virusaction == SMTP_ACTION_DELETE) {
				return SMTP_OUTCOME_SUCESS;
			}
			else {
				log_error("The virus action specified is not supported. virusaction = %i", prefs->virusaction);
			}
		}
		// If a phishing e-mail was found.
		else if (prefs->phish == 1 && state == -3) {
			if (prefs->phishaction == SMTP_ACTION_MARK_READ) {
				prefs->mark = SMTP_MARK_READ | SMTP_MARK_PHISH;
			}
			else if (prefs->phishaction == SMTP_ACTION_MARK) {
				prefs->mark = SMTP_MARK_PHISH;
			}
			else if (prefs->phishaction == SMTP_ACTION_BOUNCE) {
				return SMTP_OUTCOME_BOUNCE_PHISH;
			}
			else if (prefs->phishaction == SMTP_ACTION_DELETE) {
				return SMTP_OUTCOME_SUCESS;
			}
			else {
				log_error("The phish action specified is not supported. phishaction = %i", prefs->phishaction);
			}
		}
	}

	// This will catch messages that failed SPF checks, but weren't deleted.
	if (!con->smtp.bypass && (prefs->mark == SMTP_MARK_NONE) && (prefs->spf == 1) && (con->smtp.checked.spf == -2)) {
		if (prefs->spfaction == SMTP_ACTION_MARK_READ) {
			prefs->mark = SMTP_MARK_READ | SMTP_MARK_SPOOF;
		}
		else if (prefs->spfaction == SMTP_ACTION_MARK) {
			prefs->mark = SMTP_MARK_SPOOF;
		}
		else if (prefs->spfaction == SMTP_ACTION_BOUNCE) {
			return SMTP_OUTCOME_BOUNCE_SPF;
		}
		else if (prefs->spfaction == SMTP_ACTION_DELETE) {
			return SMTP_OUTCOME_SUCESS;
		}
		else {
			log_error("The spf action specified is not supported. spfaction = %i", prefs->spfaction);
		}
	}

	// This will check messages domain key signatures.
	if (!con->smtp.bypass && (prefs->mark == SMTP_MARK_NONE) && (prefs->dkim == 1) && (con->smtp.checked.dkim == 0 || con->smtp.checked.dkim == -2)) {

		// This message hasn't been checked yet.
		if (con->smtp.checked.dkim == 0) {
			con->smtp.checked.dkim = dkim_signature_verify(con->smtp.message->id, con->smtp.message->text);
		}

		// What action.
		if (con->smtp.checked.dkim == -2 && prefs->dkimaction == SMTP_ACTION_MARK_READ) {
			prefs->mark = SMTP_MARK_READ | SMTP_MARK_SPOOF;
		}
		else if (con->smtp.checked.dkim == -2 && prefs->dkimaction == SMTP_ACTION_MARK) {
			prefs->mark = SMTP_MARK_SPOOF;
		}
		else if (con->smtp.checked.dkim == -2 && prefs->dkimaction == SMTP_ACTION_BOUNCE) {
			return SMTP_OUTCOME_BOUNCE_DKIM;
		}
		else if (con->smtp.checked.dkim == -2 && prefs->dkimaction == SMTP_ACTION_DELETE) {
			return SMTP_OUTCOME_SUCESS;
		}
		else if (con->smtp.checked.dkim == -2) {
			log_error("The dkim action specified is not supported. dkimaction = %i", prefs->dkimaction);
		}
	}

	// This will catch messages that failed the RBL checks, but weren't rejected.
	if (!con->smtp.bypass && (prefs->mark == SMTP_MARK_NONE) && (prefs->rbl == 1) && (con->smtp.checked.rbl < -1)) {
		if (prefs->rblaction == SMTP_ACTION_MARK_READ) {
			prefs->mark = SMTP_MARK_READ | SMTP_MARK_RBL;
		}
		else if (prefs->rblaction == SMTP_ACTION_MARK) {
			prefs->mark = SMTP_MARK_RBL;
		}
		else if (prefs->rblaction == SMTP_ACTION_BOUNCE) {
			return SMTP_OUTCOME_BOUNCE_RBL;
		}
		else if (prefs->rblaction == SMTP_ACTION_DELETE) {
			return SMTP_OUTCOME_SUCESS;
		}
		else {
			log_error("The rbl action specified is not supported. rblaction = %i", prefs->rblaction);
		}
	}

	// We add the user specific headers here. Because we want the spam filter to have the benefit of a received line.
	if ((local = mail_add_inbound_headers(con, prefs)) == NULL) {
		log_error("An error occurred while attempting to add the inbound headers.");
		return SMTP_OUTCOME_TEMP_SERVER;
	}

	if (!con->smtp.bypass && (prefs->mark == SMTP_MARK_NONE) && (prefs->spam == 1)) {
		if ((prefs->spam_checked = dspam_check(prefs->usernum, local, &(prefs->spamsig))) == -1) {
			st_free(local);
			return SMTP_OUTCOME_TEMP_SERVER;
		}

		// If the message is junk.
		if (prefs->spam_checked == -2) {
			if (prefs->spamaction == SMTP_ACTION_MARK_READ) {
				prefs->mark = SMTP_MARK_READ | SMTP_MARK_SPAM;
			}
			else if (prefs->spamaction == SMTP_ACTION_MARK) {
				prefs->mark = SMTP_MARK_SPAM;
			}
			else if (prefs->spamaction == SMTP_ACTION_BOUNCE) {
				st_free(local);
				return SMTP_OUTCOME_BOUNCE_SPAM;
			}
			else if (prefs->spamaction == SMTP_ACTION_DELETE) {
				st_free(local);
				return SMTP_OUTCOME_SUCESS;
			}
			else {
				log_error("The spam action specified is not supported. spamaction = %i", prefs->spamaction);
			}
		}

		// Store any spam signature associated with this message.
		if (prefs->spamsig && !smtp_store_spamsig(prefs, prefs->spam_checked)) {
			st_free(local);
			return SMTP_OUTCOME_TEMP_SERVER;
		}
	}

	// Auto replies.
	if (prefs->mark == SMTP_MARK_NONE && prefs->autoreply != 0) {
		smtp_reply(prefs->rcptto, con->smtp.mailfrom, prefs->usernum, prefs->autoreply, con->smtp.checked.spf, con->smtp.checked.dkim);
	}

	// If the message made it through SPF, RBL and SPAM Checks, apply the filters. A negative -2 indicates the message
	// should be deleted. That means cleanup and return success without saving the data.
	if (prefs->filters != NULL) {
		if (smtp_check_filters(prefs, &local) == -2) {
			st_free(local);
			return SMTP_OUTCOME_SUCESS;
		}
	}

	// Are we supposed to forward this message?
	if (prefs->forwarded != NULL) {

		// This function forwards the message.
		state = smtp_forward_message(con->server, prefs->forwarded, local, con->smtp.message->id, prefs->mark, prefs->signum, prefs->spamkey);
		st_free(local);

		// Check the forwarding result.
		if (state == 1) {
			smtp_update_receive_stats(con, prefs);
			return SMTP_OUTCOME_SUCESS;
		}
		return SMTP_OUTCOME_PERM_FAILURE;
	}

	// This function inserts the message into the database, then compresses, and in the future will encrypt.
	state = smtp_store_message(prefs, &local);
	st_free(local);
	if (state == -2) {
		return SMTP_OUTCOME_TEMP_LOCKED;
	}
	else if (state != 1) {
		return SMTP_OUTCOME_TEMP_SERVER;
	}

	// Update our various tables responsible for tracking receptions.
	smtp_update_receive_stats(con, prefs);
	return SMTP_OUTCOME_SUCESS;
}
