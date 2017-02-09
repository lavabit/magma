
/**
 * @file /magma/servers/smtp/datatier.c
 *
 * @brief	Functions used to interface with and manage data needed by the SMTP protocol.
 */

#include "magma.h"


/**
 * @brief	Parse an smtp event action string from the Dispatch table.
 * @note	These values describe user-specified actions for events related to the spam filter, virus scanner, phishing detection,
 * 			SPF, DKIM, and RBL checks.
 * @param	string		a pointer to a null-terminated string containing the name of the action.
 * @param	length		the length, in bytes, of the action string.
 * @return	the code of the corresponding smtp event action, SMTP_ACTION_UNDEFINED if the action is not known, or SMTP_ACTION_ERROR on failure.
 */
int_t smtp_get_action(chr_t *string, size_t length) {

	if (!string || !length) {
		return SMTP_ACTION_ERROR;
	}
	else if (!st_cmp_cs_eq(PLACER(string, length), PLACER("DELETE", 6))) {
		return SMTP_ACTION_DELETE;
	}
	else if (!st_cmp_cs_eq(PLACER(string, length), PLACER("REJECT", 6))) {
		return SMTP_ACTION_REJECT;
	}
	else if (!st_cmp_cs_eq(PLACER(string, length), PLACER("BOUNCE", 6))) {
		return SMTP_ACTION_BOUNCE;
	}
	else if (!st_cmp_cs_eq(PLACER(string, length), PLACER("MARK", 4))) {
		return SMTP_ACTION_MARK;
	}
	else if (!st_cmp_cs_eq(PLACER(string, length), PLACER("MARK_READ", 9))) {
		return SMTP_ACTION_MARK_READ;
	}

	log_error("Undefined action detected. action = %.*s", (int_t)length, string);
	return SMTP_ACTION_UNDEFINED;
}

/**
 * @brief	Fetch a specified auto-reply message for a user.
 * @note	This function first checks the cache, and falls back to the database.
 * @param	autoreply	the numerical id of the auto-reply message in the database.
 * @param	usernum		the numerical id of the user to whom the auto-reply message belongs.
 * @return	NULL on failure, or a pointer to a managed string containing the user's auto-reply on success.
 */
stringer_t * smtp_fetch_autoreply(uint64_t autoreply, uint64_t usernum) {

	int_t keylen;
	chr_t key[1024];
	row_t *row;
	stringer_t *text;
	table_t *result;
	MYSQL_BIND parameters[2];

	// Check the cache.
	keylen = snprintf(key, 1024, "lavad.cache.autoreply.%lu.%lu", usernum, autoreply);

	if (keylen > 0 && (text = cache_get(PLACER(key, keylen))) != NULL) {
		return text;
	}

	mm_wipe(parameters, sizeof(parameters));

	// Autoreply
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &autoreply;
	parameters[0].is_unsigned = true;

	// Usernum
	parameters[1].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[1].buffer_length = sizeof(uint64_t);
	parameters[1].buffer = &usernum;
	parameters[1].is_unsigned = true;

	if ((result = stmt_get_result(stmts.select_autoreply, parameters)) == NULL) {
		log_pedantic("The autoreply %lu could not be fetched from the database.", autoreply);
		return NULL;
	}
	else if (!(row = res_row_next(result))) {
		log_pedantic("Could not fetch the SQL result row.");
		res_table_free(result);
		return NULL;
	}

	text = res_field_string(row, 0);
	res_table_free(result);

	// Set the cache for next time. Hold it for 7 days.
	if (keylen > 0 && text) {
		cache_set(PLACER(key, keylen), text, 604800);
	}

	return text;
}

/**
 * @brief	Fetch a user's smtp inbound preferences from the database.
 * @param	cred	a pointer to the credential object of a user with
 * @param	address
 *
 *
 * Returns -1 for errors, -2 for an admin lock, -3 for an inactivity lock, -4 for a user lock, -5 for an abuse lock, -6 if the domain isn't local
 * and 0 if the domain is local but the address wasn't found. If everything works, return 1 to indicate success.
 */
int_t smtp_fetch_inbound(stringer_t *address, smtp_inbound_prefs_t **output) {

	row_t *row;
	table_t *result;
	placer_t domain;
	MYSQL_BIND parameters[1];
	stringer_t *signet = NULL;
	int_t locked, filters, local = -1;
	smtp_inbound_prefs_t *inbound = NULL;
	smtp_inbound_filter_t *filter = NULL;
	multi_t key = {
		.type = M_TYPE_UINT64, .val.u64 = 0
	};

	if (st_empty(address) || !output) {
		return -1;
	}
	else if (mail_domain_get(address, &domain) == NULL) {
		return -1;
 	}

	*output = NULL;
	mm_wipe(parameters, sizeof(parameters));

	// Username.
	parameters[0].buffer_type = MYSQL_TYPE_STRING;
	parameters[0].buffer_length = st_length_get(address);
	parameters[0].buffer = st_char_get(address);

	// If the address isn't found locally, check whether the domain configuration indicates we should also perform a wildcard search.
	if ((result = stmt_get_result(stmts.select_prefs_inbound, parameters)) && res_row_count(result) == 0 && (local = domain_wildcard(&domain)) == 1) {

		res_table_free(result);
		mm_wipe(parameters, sizeof(parameters));

		parameters[0].buffer_type = MYSQL_TYPE_STRING;
		parameters[0].buffer_length = st_length_get(&domain);
		parameters[0].buffer = st_char_get(&domain);

		result = stmt_get_result(stmts.select_prefs_inbound, parameters);
	}

	// Server error.
	if (!result) {
		return -1;
	}

	// Results without any rows indicate the address didn't match a mailbox.
	if (!(row = res_row_next(result))) {
		res_table_free(result);

		// If the domain_wildcard search didn't find a domain record, the address can't be local.
		if (local == -1) {
			return -6;
		}

		// No matching mailboxes.
		return 0;
	}

	// Admin lock.
	if ((locked = res_field_int8(row, 1)) == 1) {
		res_table_free(result);
		return -2;
	}
	// Inactivity lock.
	else if (locked == 2) {
		res_table_free(result);
		return -3;
	}
	// Abuse lock.
	else if (locked == 3) {
		res_table_free(result);
		return -4;
	}
	// User lock.
	else if (locked == 4) {
		res_table_free(result);
		return -5;
	}

	if (!(inbound = mm_alloc(sizeof(smtp_inbound_prefs_t)))) {
		log_pedantic("Could not allocate %zu bytes for the inbound preferences.", sizeof(smtp_inbound_prefs_t));
		res_table_free(result);
		return -1;
	}

	// Store the result.
	if (!(inbound->usernum = res_field_uint64(row, 0))) {
		log_pedantic("Found a zero usernum for the address %.*s.", st_length_int(address), st_char_get(address));
		mm_free(inbound);
		res_table_free(result);
		return -1;
	}

	inbound->stor_size = res_field_uint64(row, 2);
	inbound->quota = res_field_uint64(row, 3);
	inbound->overquota = res_field_int8(row, 4);
	inbound->domain = res_field_string(row, 5);
	inbound->secure = res_field_int8(row, 6);
	inbound->bounces = res_field_int8(row, 7);
	inbound->forwarded = res_field_string(row, 8);
	inbound->rollout = res_field_int8(row, 9);
	inbound->spam = res_field_int8(row, 10);
	inbound->spamaction = smtp_get_action(res_field_block(row, 11), res_field_length(row, 11));
	inbound->virus = res_field_int8(row, 12);
	inbound->virusaction = smtp_get_action(res_field_block(row, 13), res_field_length(row, 13));
	inbound->phish = res_field_int8(row, 14);
	inbound->phishaction = smtp_get_action(res_field_block(row, 15), res_field_length(row, 15));
	inbound->autoreply = res_field_uint64(row, 16);
	inbound->inbox = res_field_uint64(row, 17);
	inbound->recv_size_limit = res_field_uint32(row, 18);
	inbound->daily_recv_limit = res_field_uint32(row, 19);
	inbound->daily_recv_limit_ip = res_field_uint32(row, 20);
	inbound->greylist = res_field_int8(row, 21);
	inbound->greytime = res_field_uint32(row, 22);
	inbound->spf = res_field_int8(row, 23);
	inbound->spfaction = smtp_get_action(res_field_block(row, 24), res_field_length(row, 24));
	inbound->dkim = res_field_int8(row, 25);
	inbound->dkimaction = smtp_get_action(res_field_block(row, 26), res_field_length(row, 26));
	inbound->rbl = res_field_int8(row, 27);
	inbound->rblaction = smtp_get_action(res_field_block(row, 28), res_field_length(row, 28));
	filters = res_field_int8(row, 29);

	// We should only decode and store the public key if the account has storage security enabled. Otherwise the
	// mail_store_message() function will interpret the presence of the key to mean encryption has been enabled.
	if (inbound->secure && res_field_length(row, 30)) {
		if (!(signet = base64_decode_mod(PLACER(res_field_block(row, 30), res_field_length(row, 30)), NULL)) ||
			!(inbound->signet = prime_set(signet, BINARY, NONE))) {
			log_pedantic("Unable to parse the signet for a secure account. { address = %.*s }", st_length_int(address),	st_char_get(address));
		}

		st_cleanup(signet);
	}
#ifdef MAGMA_SMTP_PEDANTIC
	else if (inbound->secure) {
		log_pedantic("Secure storage was enabled, but no public key was found for the account. { address = %.*s }", st_length_int(address),
			st_char_get(address));
	}
#endif

	// Free the memory.
	res_table_free(result);

	// Error checking.
	if (inbound->spamaction <= 0 || inbound->virusaction <= 0 || inbound->phishaction <= 0 || inbound->spfaction <= 0 || inbound->rblaction	<= 0 ||
		inbound->dkimaction <= 0) {
		log_pedantic("Found an invalid action field. {%.*s}", st_length_int(address), st_char_get(address));
		smtp_free_inbound(inbound);
		return -1;
	}

	// If there is no Inbox, then we better be forwarding this message.
	if ((inbound->inbox == 0 || inbound->quota == 0) && inbound->forwarded == NULL) {
		log_pedantic("Found an account with no Inbox and no forwarding address. {%.*s}", st_length_int(address), st_char_get(address));
		smtp_free_inbound(inbound);
		return -1;
	}

	// Initialize recipient and address parameters.
	if (!(inbound->rcptto = st_dupe(address)) || !(inbound->address = st_dupe_opts(MANAGED_T | CONTIGUOUS | HEAP, address))) {
		log_pedantic("Could not duplicate the recipient and address.");
		smtp_free_inbound(inbound);
		return -1;
	}

	// Set the output parameter.
	*output = inbound;

	if (filters == 1) {

		// Check for filters.
		mm_wipe(parameters, sizeof(parameters));

		// Usernum
		parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
		parameters[0].buffer_length = sizeof(uint64_t);
		parameters[0].buffer = &(inbound->usernum);
		parameters[0].is_unsigned = true;

		// Execute the query, and store the result.
		if ((result = stmt_get_result(stmts.select_filters, parameters)) != NULL) {

			// Allocate our linked list.
			if ((inbound->filters = inx_alloc(M_INX_LINKED, &smtp_list_free_filter)) == NULL) {
				log_error("Could not create a linked list for the filters.");
				res_table_free(result);
				return 1;
			}

			// This will build the filters linked list.
			while ((row = res_row_next(result)) != NULL) {

				if ((filter = mm_alloc(sizeof(smtp_inbound_filter_t))) == NULL) {
					log_error("Could not create allocate %zu bytes for an inbound filter.", sizeof(smtp_inbound_filter_t));
					res_table_free(result);
					return 1;
				}

				filter->rulenum = key.val.u64 = res_field_uint64(row, 0);
				filter->location = res_field_uint32(row, 1);
				filter->type = res_field_uint32(row, 2);
				filter->action = res_field_uint32(row, 3);
				filter->foldernum = res_field_uint64(row, 4);
				filter->field = res_field_string(row, 5);
				filter->label = res_field_string(row, 6);
				filter->expression = res_field_string(row, 7);

				// Make sure we get back a valid filter.
				if (((filter->action & SMTP_FILTER_ACTION_MOVE) == SMTP_FILTER_ACTION_MOVE && filter->foldernum == 0) || ((filter->action
					& SMTP_FILTER_ACTION_LABEL) == SMTP_FILTER_ACTION_LABEL && filter->label == NULL) || ((filter->location
					& SMTP_FILTER_LOCATION_FIELD) == SMTP_FILTER_LOCATION_FIELD && filter->field == NULL) || filter->expression == NULL
					|| filter->rulenum == 0) {
					smtp_list_free_filter(filter);
					log_error("Found an invalid filter for the user %lu.", inbound->usernum);
				}
				else if (!inx_insert(inbound->filters, key, filter)) {
					smtp_list_free_filter(filter);
				}
			}

			res_table_free(result);
		}
	}

	return 1;
}

/**
 * @brief	Retrieve a list, at a maximum of 20 entries, of the oldest messages owned by a user.
 * @param	usernum		the numerical id of the user whose messages are to be queried.
 * @return	NULL on failure, or a pointer to a sql results set containing the user's oldest messages on success.
 */
table_t * smtp_fetch_rollmessages(uint64_t usernum) {

	table_t *result;
	MYSQL_BIND parameters[1];

	mm_wipe(parameters, sizeof(parameters));

	// Usernum
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &usernum;
	parameters[0].is_unsigned = true;

	if (!(result = stmt_get_result(stmts.select_messages_rollout, parameters))) {
		log_pedantic("No messages are eligible for rollout.");
		return NULL;
	}

	return result;
}

/**
 * @brief	Update the receiving statistics and per-user log tables in the database for a successfully received smtp message.
 * @note	The Receiving table is updated with the subnet address from which the message was received;
 * 			the Log table for the user is updated to reflect the newly calculated totals of bounces or messages received.
 * @param	con		the connection across which the smtp message was received.
 * @param	prefs	a pointer to the user's smtp inbound mail preferences.
 * @return	This function returns no value.
 */
void smtp_update_receive_stats(connection_t *con, smtp_inbound_prefs_t *prefs) {

	stringer_t *substr;
	MYSQL_BIND parameters[3];
	int_t bounce = 0, message = 1;

	if (!(substr = con_addr_subnet(con, NULL))) {
		log_pedantic("Unable to perform subnet lookup of connection's remote address.");
		return;
	}

	mm_wipe(parameters, sizeof(parameters));

	// Usernum
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = (chr_t *)&(prefs->usernum);
	parameters[0].is_unsigned = true;

	// Subnet
	parameters[1].buffer_type = MYSQL_TYPE_STRING;
	parameters[1].buffer_length = st_length_get(substr);
	parameters[1].buffer = st_char_get(substr);

	if (!stmt_exec(stmts.insert_receiving, parameters)) {
		log_pedantic("Unable to insert a record into the receiving table.");
	}

	st_free(substr);

	mm_wipe(parameters, sizeof(parameters));

	if (!st_cmp_cs_eq(con->smtp.mailfrom, PLACER("<>", 2))) {
		bounce = 1;

		if (prefs->bounces == 0) {
			message = 0;
		}

	}

	// Received
	parameters[0].buffer_type = MYSQL_TYPE_LONG;
	parameters[0].buffer_length = sizeof(int32_t);
	parameters[0].buffer = &message;

	// Bounce
	parameters[1].buffer_type = MYSQL_TYPE_LONG;
	parameters[1].buffer_length = sizeof(int32_t);
	parameters[1].buffer = &bounce;

	// Usernum
	parameters[2].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[2].buffer_length = sizeof(uint64_t);
	parameters[2].buffer = &(prefs->usernum);
	parameters[2].is_unsigned = true;

	if (!stmt_exec(stmts.update_log_received, parameters)) {
		log_pedantic("Unable to update the log.");
	}

	return;
}

/**
 * @brief	Store a spam signature in the database.
 * @see		dspam_process()
 * @param	prefs	a pointer to the specified user's inbound mail preferences data.
 * @param	key		a randomly chosen authentication key for the signature.
 * @param	code	the dspam return code for the message from dspam_process()
 * @return	0 on failure, or the number of the newly inserted spam signature on success.
 */
uint64_t smtp_insert_spamsig(smtp_inbound_prefs_t *prefs, uint64_t key, int_t code) {

	uint64_t signum;
	MYSQL_BIND parameters[4];

	mm_wipe(parameters, sizeof(parameters));

	// DSPAM will return a code of -2 for junk mail, -1 for errors, and 1 for innocent messages. The logic below will map junk to 1, and everything else to 0 (aka innocent) in the database.
	if (code == -2) {
		code = 1;
	}
	else {
		code = 0;
	}

	// Usernum
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &(prefs->usernum);
	parameters[0].is_unsigned = true;

	// Key
	parameters[1].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[1].buffer_length = sizeof(uint64_t);
	parameters[1].buffer = &key;
	parameters[1].is_unsigned = true;

	// Junk
	parameters[2].buffer_type = MYSQL_TYPE_LONG;
	parameters[2].buffer_length = sizeof(int_t);
	parameters[2].buffer = &code;

	// Signature
	parameters[3].buffer_type = MYSQL_TYPE_STRING;
	parameters[3].buffer = st_char_get(prefs->spamsig);
	parameters[3].buffer_length = st_length_get(prefs->spamsig);

	if ((signum = stmt_insert(stmts.insert_signature, parameters)) == 0) {
		log_pedantic("Unable to insert the message signature.");
	}

	return signum;
}

/**
 * @brief	Update the transmission and per-user log tables in the database for a successfully sent smtp message.
 * @note	The Transmitting table is updated with the timestamp of this transaction;
 * 			the Log table for the user is updated to reflect the newly calculated total for messages sent.
 * @param	con		a pointer to the connection object across which the smtp message was sent.
 * @param	prefs	a pointer to the user's smtp inbound mail preferences.
 * @return	This function returns no value.
 */
void smtp_update_transmission_stats(connection_t *con) {

	MYSQL_BIND parameters[2];

	mm_wipe(parameters, sizeof(parameters));

	// Usernum
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = (chr_t *)&(con->smtp.out_prefs->usernum);
	parameters[0].is_unsigned = true;

	for (int_t ret = true, i = 0; ret && i < con->smtp.num_recipients; i++) {
		if (!(ret = stmt_exec(stmts.insert_transmitting, parameters))) {
			log_pedantic("Unable to insert a record into the transmitting table. {ret = %i}", ret);
		}
	}

	mm_wipe(parameters, sizeof(parameters));

	// Recipients
	parameters[0].buffer_type = MYSQL_TYPE_LONG;
	parameters[0].buffer_length = sizeof(int_t);
	parameters[0].buffer = &(con->smtp.num_recipients);

	// Usernum
	parameters[1].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[1].buffer_length = sizeof(uint64_t);
	parameters[1].buffer = &(con->smtp.out_prefs->usernum);
	parameters[1].is_unsigned = true;

	if (stmt_exec(stmts.update_log_sent, parameters) != true) {
		log_pedantic("Unable to update the log.");
	}

	return;
}

/**
 * @brief	Check to see if a user's current mail send request would push them over their daily transmission quota.
 * @note	This check is performed by querying the database to see how many messages a user has sent in the past 24 hour period, and by
 * 			adding the current number of recipients of the pending email request to that number to see if their quota would be exceeded.
 * @param	con		a pointer to the connection object of the user attempting to send mail.
 * @return	-1 on error, 0 if the send operation is permitted, or 1 if the send operation would result in a daily send quota overage.
 */
int_t smtp_check_transmit_quota(uint64_t usernum, size_t num_recipients, smtp_outbound_prefs_t *prefs) {

	row_t *row;
	table_t *result;
	MYSQL_BIND parameters[1];

	mm_wipe(parameters, sizeof(parameters));

	// Usernum
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &usernum;
	parameters[0].is_unsigned = true;

	if (!(result = stmt_get_result(stmts.select_transmitting, parameters))) {
		log_pedantic("Could not check the transmit quota.");
		return -1;
	}

	// Get the first row.
	if (!(row = res_row_next(result))) {
		log_pedantic("Could not fetch the first SQL result row.");
		res_table_free(result);
		return -1;
	}

	// Store the number of sent messages and free the result.
	prefs->sent_today = (uint32_t)res_field_uint64(row, 0);
	res_table_free(result);

	// If the user has exceeded their quota, return one so relay access is denied.
	if (prefs->sent_today + num_recipients > prefs->daily_send_limit) {
		return 1;
	}

	return 0;
}

/**
 * @brief	Check if a user is authorized to send messages, and retrieve the user's outbound smtp preferences.
 * @note	This function first checks if the account is locked in the Users table; next it populates the
 * 			user's outbound mail preferences with a combination of data from the Users and Dispatch tables.
 * 			The number of messages the user has sent in the past 24 hours is also computed from the Transmitting table.
 * @param	cred	a pointer to a credential object for the user, which must be of type CREDENTIAL_AUTH.
 * @param	output	a pointer to the address of an outbound smtp preferences object to receive the value of the lookup.
 * @return  1 on success or <= 0 on failure.
 * 		    0: authentication failure (invalid username/password combination).
 * 		   -1: general or database failure.
 * 		   -2: the account is subject to an administrative lock.
 *		   -3: the account is locked due to suspicion of abuse violations.
 *		   -4: the account has been locked at the request of the user.
 */
int_t smtp_fetch_authorization(stringer_t *username, stringer_t *verification, smtp_outbound_prefs_t **output) {

	row_t *row;
	int_t locked;
	table_t *result;
	MYSQL_BIND parameters[2];
	stringer_t *encoded = NULL;
	smtp_outbound_prefs_t *outbound;

	if (!st_populated(username, verification) || !output || !(encoded = base64_encode_mod(verification, NULL))) {
		return -1;
	}

	*output = NULL;
	mm_wipe(parameters, sizeof(parameters));

	// Parameters
	parameters[0].buffer_type = MYSQL_TYPE_STRING;
	parameters[0].buffer_length = st_length_get(username);
	parameters[0].buffer = st_char_get(username);

	parameters[1].buffer_type = MYSQL_TYPE_STRING;
	parameters[1].buffer_length = st_length_get(encoded);
	parameters[1].buffer = st_char_get(encoded);

	if (!(result = stmt_get_result(stmts.smtp_select_user_auth, parameters))) {
		log_pedantic("Authentication attempt failed by database error.");
		st_free(encoded);
		return -1;
	}

	// Free the encoded version of the verification token.
	st_free(encoded);

	// Get the first row.
	if (!(row = res_row_next(result))) {
		res_table_free(result);
		return 0;
	}

	// Admin lock.
	if ((locked = res_field_int8(row, 1)) == 1) {
		res_table_free(result);
		return -2;
	}
	// Abuse lock.
	else if (locked == 3) {
		res_table_free(result);
		return -3;
	}
	// User lock.
	else if (locked == 4) {
		res_table_free(result);
		return -4;
	}

	// Allocate storage for the prefs structure.
	if (!(outbound = mm_alloc(sizeof(smtp_outbound_prefs_t)))) {
		log_pedantic("Unable to allocate a block of %zu bytes to hold the outbound preferences.", sizeof(smtp_outbound_prefs_t));
		res_table_free(result);
		return -1;
	}

	// Store the result.
	if (!(outbound->usernum = res_field_uint64(row, 0))) {
		log_error("Invalid user number. { username = %.*s }", st_length_int(username), st_char_get(username));
		res_table_free(result);
		mm_free(outbound);
		return -1;
	}

	outbound->tls = res_field_int8(row, 2);
	outbound->domain = res_field_string(row, 3);
	outbound->send_size_limit = res_field_uint32(row, 4);
	outbound->daily_send_limit = res_field_uint32(row, 5);
	outbound->importance = res_field_int8(row, 6);

	// Check whether this account is locked as a result of inactivity. If so, unlock.
	if (locked == 2) {
		meta_data_update_lock(outbound->usernum, 0);
	}

	res_table_free(result);
	*output = outbound;

	// Now find out how many messages have been sent.
	mm_wipe(parameters, sizeof(parameters));

	// Usernum
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &(outbound->usernum);
	parameters[0].is_unsigned = true;

	if (!(result = stmt_get_result(stmts.select_transmitting, parameters))) {
		log_pedantic("Could not check the transmit quota.");
		return 1;
	}

	// Get the first row.
	else if (!(row = res_row_next(result))) {
		log_pedantic("Could not fetch the first SQL result row.");
		res_table_free(result);
		return 1;
	}

	// Store the number of sent messages and free the result.
	outbound->sent_today = (uint32_t)res_field_uint64(row, 0);
	res_table_free(result);

	return 1;
}

/**
 * @brief	Check the user's received statistics from the database to see if receiving a message would result in a quota overage.
 * @note	The sum total of all emails received by the user over the past 24 hours is calculated from the database, and these checks are made:
 *			1. The amount of mail messages received in the past 24 hours by the user does not exceed their daily mail received quota.
 *			2. The messages received in the past 24 hours by the user from this subnet does not exceed the user's daily per-subnet received quota.
 * @param	con		a pointer to the connection object over which the smtp message was received.
 * @param	prefs	a pointer to the user's smtp inbound mail preferences.
 * @return	0 if message receipt is permitted, 1 if the general daily receiving limit was exceeded, 2 if the sending subnet's transmission
 * 			limit was exceeded, or -1 if there was a general error.
 */
int_t smtp_check_receive_quota(connection_t *con, smtp_inbound_prefs_t *prefs) {

	row_t *row;
	table_t *result;
	uint64_t number;
	stringer_t *substr;
	MYSQL_BIND parameters[2];

	if (!(substr = con_addr_subnet(con, NULL))) {
		log_pedantic("Unable to perform subnet lookup of connection's remote address.");
		return -1;
	}

	mm_wipe(parameters, sizeof(parameters));

	// Subnet
	parameters[0].buffer_type = MYSQL_TYPE_STRING;
	parameters[0].buffer_length = st_length_get(substr);
	parameters[0].buffer = st_char_get(substr);

	// Usernum
	parameters[1].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[1].buffer_length = sizeof(uint64_t);
	parameters[1].buffer = &(prefs->usernum);
	parameters[1].is_unsigned = true;

	if (!(result = stmt_get_result(stmts.select_receiving, parameters))) {
		log_pedantic("Could not check the receive quota.");
		st_free(substr);
		return 0;
	}

	st_free(substr);

	// Get the first row.
	if (!(row = res_row_next(result))) {
		log_pedantic("Could not fetch the first SQL result row.");
		res_table_free(result);
		return -1;
	}

	// If the user has exceeded their limit, return one so we don't accept the message.
	if (res_field_uint64(row, 0) >= prefs->daily_recv_limit) {
		res_table_free(result);
		return 1;
	}
	else if ((res_field_block(row, 1) && (!uint64_conv_bl(res_field_block(row, 1), res_field_length(row, 1), &number) || number >= prefs->daily_recv_limit_ip)) ||
		(!res_field_block(row, 1) && !prefs->daily_recv_limit_ip)) {
		res_table_free(result);
		return 2;
	}

	res_table_free(result);

	return 0;
}

/**
 * @brief	Check to see if a user is permitted to send email originating from a specified email address.
 * @note	The authorization attempt first checks against the specified email address, and if unsuccessful, upon
 * 			the domain component of the email address if wildcards are enabled for that domain.
 * @param	usernum		the numerical id of the user attempting to send the mail message.
 * @param	address		a pointer to a managed string containing the From address value of the mail message to be sent.
 * @return	1 if the user is authorized to send email from the specified address, or 0 otherwise.
 */
int_t smtp_check_authorized_from(uint64_t usernum, stringer_t *address) {

	placer_t domain;
	table_t *result;
	MYSQL_BIND parameters[2];

	mm_wipe(parameters, sizeof(parameters));

	// Address
	parameters[0].buffer_type = MYSQL_TYPE_STRING;
	parameters[0].buffer_length = st_length_get(address);
	parameters[0].buffer = st_char_get(address);

	// Usernum
	parameters[1].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[1].buffer_length = sizeof(uint64_t);
	parameters[1].buffer = &usernum;
	parameters[1].is_unsigned = true;

	// Looks for a row in the Mailboxes table matching the address and user number. If the row exists then we let the message through.
	if ((result = stmt_get_result(stmts.select_mailbox_address, parameters)) && res_row_count(result)) {
		res_table_free(result);
		return 1;
	}
	else if (result) {
		res_table_free(result);
	}

	// An exact match wasn't found. Try the wildcards now.
	if (mail_domain_get(address, &domain) && domain_wildcard(&domain) == 1) {

		mm_wipe(parameters, sizeof(parameters));

		// Domain
		parameters[0].buffer_type = MYSQL_TYPE_STRING;
		parameters[0].buffer_length = st_length_get(&domain);
		parameters[0].buffer = st_char_get(&domain);

		// Usernum
		parameters[1].buffer_type = MYSQL_TYPE_LONGLONG;
		parameters[1].buffer_length = sizeof(uint64_t);
		parameters[1].buffer = &usernum;
		parameters[1].is_unsigned = true;

		if ((result = stmt_get_result(stmts.select_mailbox_address, parameters)) && res_row_count(result)) {
			res_table_free(result);
			return 1;
		}
		else if (result) {
			res_table_free(result);
		}
	}

	return 0;
}
