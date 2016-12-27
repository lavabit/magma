
/**
 * @file /magma/servers/smtp/transmit.c
 *
 * @brief Handle replies.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Relay an outbound smtp message for a user.
 * @note	The following process occurs before the message will be sent:
 * 			1. Necessary outbound headers are attached to the message.*
 * 			2. An outbound connection to a mail relay server is established (with a premium or normal server pool).
 * 			3. Once the connection is negotiated, an RCPT TO command is issued for each of the message's recipients.
 * 			4. The mail message data is sent and the client connection is closed.
 * @param	con		a pointer to the connection object across which the outbound mail was attempted to be sent.
 * @param	result	a pointer to the address of a managed string that will receive the server's last response to the mail send attempt,
 * 			regardless of whether or not it was successful.
 * @return	1 if the message was successfully sent or -1 on failure.
 */
int_t smtp_relay_message(connection_t *con, stringer_t **result) {

	int_t state;
	smtp_recipients_t *holder;
	client_t *client;

	if (!result || !con || !con->smtp.message || !con->smtp.message->text || !con->smtp.out_prefs->recipients) {
		log_pedantic("Passed a NULL pointer.");
		return -1;
	}

	// In case we return early.
	*result = NULL;

	if (mail_add_outbound_headers(con) != 1) {
		log_pedantic("Could not add the outbound headers.");
		return -1;
	}

	// Open the connection to the SMTP server.
	if (!(client = smtp_client_connect(con->smtp.out_prefs->importance))) {
		log_pedantic("Could not relay the message.");
		return -1;
	}

	// Send HELO.
	if (smtp_client_send_helo(client) != 1) {
		log_pedantic("An error occurred while trying to say hello.");
		smtp_client_close(client);
		return -1;
	}

	// Send MAIL FROM.
	if ((state = smtp_client_send_mailfrom(client, con->smtp.mailfrom, 0)) == -2) {
		log_pedantic("An error occurred while trying to send the mail from.");
		*result = st_dupe_opts(MANAGED_T | CONTIGUOUS | HEAP, &(client->line));
		smtp_client_close(client);
		return -1;
	}
	else if (state != 1) {
		log_pedantic("An error occurred while trying to send the mail from.");
		smtp_client_close(client);
		return -1;
	}

	// Send the RCPT TO command.
	holder = con->smtp.out_prefs->recipients;

	while (holder) {

		if ((state = smtp_client_send_rcptto(client, holder->address)) == -2) {
			log_pedantic("An error occurred while trying to send a rcpt to command.");
			*result = st_dupe_opts(MANAGED_T | CONTIGUOUS | HEAP, &(client->line));
			smtp_client_close(client);
			return -1;
		}
		else if (state != 1) {
			log_pedantic("An error occurred while trying to send a rcpt to command.");
			smtp_client_close(client);
			return -1;
		}

		holder = (smtp_recipients_t *)holder->next;
	}

	// Send the the message.
	state = smtp_client_send_data(client, con->smtp.message->text);

	if (state == -2) {
		log_pedantic("An error occurred while trying to send the message.");
		*result = st_dupe_opts(MANAGED_T | CONTIGUOUS | HEAP, &(client->line));
		smtp_client_close(client);
		return -1;
	}
	else if (state != 1) {
		log_pedantic("An error occurred while trying to send the message.");
		smtp_client_close(client);
		return -1;
	}

	// Store the result.
	*result = st_dupe_opts(MANAGED_T | CONTIGUOUS | HEAP, &(client->line));
	smtp_client_close(client);

	return 1;
}

// Forwards an inbound message to the user's remote e-mail.
int_t smtp_forward_message(server_t *server, stringer_t *address, stringer_t *message, stringer_t *id, int_t mark, uint64_t signum, uint64_t sigkey) {

	int_t state;
	stringer_t *new;
	client_t *client;

	if (!address || !message) {
		log_pedantic("Passed a NULL pointer.");
		return -2;
	}
	else if (!(new = st_dupe(message))) {
		log_pedantic("Could not duplicate the message.");
		return -1;
	}

	mail_add_forward_headers(server, &new, id, mark, signum, sigkey);

	// Open the connection to the SMTP server. Always use the default servers for forwards.
	if (!(client = smtp_client_connect(0))) {
		log_pedantic("Could not relay the message.");
		smtp_client_close(client);
		st_free(new);
		return -1;
	}

	// Send HELO.
	if (smtp_client_send_helo(client)!= 1) {
		log_pedantic("An error occurred while trying to say hello.");
		smtp_client_close(client);
		st_free(new);
		return -1;
	}

	// Send MAIL FROM.
	if (smtp_client_send_nullfrom(client) != 1) {
		log_pedantic("An error occurred while trying to send the mail from.");
		smtp_client_close(client);
		st_free(new);
		return -1;
	}

	// Send the RCPT TO command. If this fails, use a permanent failure code.
	if ((state = smtp_client_send_rcptto(client, address)) != 1) {
		log_pedantic("An error occurred while trying to send a rcpt to command.");
		smtp_client_close(client);
		st_free(new);
		return -2;
	}

	// Send the the message.
	if ((state = smtp_client_send_data(client, new)) != 1) {
		log_pedantic("An error occurred while trying to send the message.");
		smtp_client_close(client);
		st_free(new);
		return -1;
	}

	// Store the result.
	smtp_client_close(client);
	st_free(new);

	return 1;
}

int_t smtp_bounce(connection_t *con) {

	time_t utime;
	struct tm ltime;
	uint32_t number = 0;
	chr_t date_buffer[1024];
	smtp_inbound_prefs_t *prefs;
	client_t *client;
	int_t explanations = SMTP_OUTCOME_SUCESS;
	static const chr_t *date_format = "Date: %a, %d %b %Y %H:%M:%S %z\r\n";
	stringer_t *message = NULL, *holder = NULL, *explain = NULL, *bounces = NULL, *signature = NULL, *id = MANAGEDBUF(16);

	// Only send bounces if the SPF and DKIM checks didn't explicitly fail, and the return path isn't empty.
	if (!con || con->smtp.checked.spf == -2 || con->smtp.checked.dkim == -2 || !st_cmp_cs_eq(con->smtp.mailfrom, "<>")) {
		return 0;
	}

	// Build the date string. Otherwise null the buffer so it doesn't get appended to the header.
	if ((utime = time(&utime)) == -1 || !localtime_r(&utime, &ltime) || strftime(date_buffer, 1024, date_format, &ltime) <= 0) {
		log_pedantic("Unable to retrieve the current time.");
		date_buffer[0] = '\0';
	}

		// Generate the ID string. If the random method fails, use a hash of the current time.
	if ((holder = rand_choices("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789", 12))) {
		st_sprint(id, "%.*s", st_length_int(holder), st_char_get(holder));
		st_free(holder);
		holder = NULL;
	}
	else {
		st_sprint(id, "%lu", hash_crc64(&utime, sizeof(time_t)));
	}

	// Open the connection to the SMTP server. Always use the default servers for forwards.
	if (!(client = smtp_client_connect(0))) {
		log_pedantic("Could not relay the message.");
		smtp_client_close(client);
		return 0;
	}

	// Send HELO.
	if (smtp_client_send_helo(client)!= 1) {
		log_pedantic("An error occurred while trying to say hello.");
		smtp_client_close(client);
		return 0;
	}

	// Send NULL MAIL FROM.
	if (smtp_client_send_nullfrom(client) != 1) {
		log_pedantic("An error occurred while trying to send the mail from.");
		smtp_client_close(client);
		return 0;
	}

	// Send the RCPT TO command. If this fails, use a permanent failure code.
	if (smtp_client_send_rcptto(client, con->smtp.mailfrom) != 1) {
		log_pedantic("An error occurred while trying to send a rcpt to command.");
		smtp_client_close(client);
		return 0;
	}

	// Loop through and build the bounce summary.
	prefs = con->smtp.in_prefs;

	while (prefs) {

		if ((prefs->outcome & SMTP_OUTCOME_PERM_FAILURE) == SMTP_OUTCOME_PERM_FAILURE) {
			number++;
			explanations = (explanations | SMTP_OUTCOME_PERM_FAILURE);
			holder = st_merge("snsn", bounces, "    ", prefs->rcptto, " - Permanent server error.\r\n");
		}
		else if ((prefs->outcome & SMTP_OUTCOME_TEMP_SERVER) == SMTP_OUTCOME_TEMP_SERVER) {
			number++;
			explanations = (explanations | SMTP_OUTCOME_TEMP_SERVER);
			holder = st_merge("snsn", bounces, "    ", prefs->rcptto, " - Temporary server error.\r\n");
		}
		else if ((prefs->outcome & SMTP_OUTCOME_TEMP_OVERQUOTA) == SMTP_OUTCOME_TEMP_OVERQUOTA) {
			number++;
			explanations = (explanations | SMTP_OUTCOME_TEMP_OVERQUOTA);
			holder = st_merge("snsn", bounces, "    ", prefs->rcptto, " - Mailbox is over assigned storage quota.\r\n");
		}
		else if ((prefs->outcome & SMTP_OUTCOME_TEMP_LOCKED) == SMTP_OUTCOME_TEMP_LOCKED) {
			number++;
			explanations = (explanations | SMTP_OUTCOME_TEMP_LOCKED);
			holder = st_merge("snsn", bounces, "    ", prefs->rcptto, " - Mailbox is locked and unavailable for delivery.\r\n");
		}
		else if ((prefs->outcome & SMTP_OUTCOME_BOUNCE_SPF) == SMTP_OUTCOME_BOUNCE_SPF) {
			number++;
			explanations = (explanations | SMTP_OUTCOME_BOUNCE_SPF);
			holder = st_merge("snsn", bounces, "    ", prefs->rcptto, " - The return path failed a Sender Policy Framework (SPF) check.\r\n");
		}
		else if ((prefs->outcome & SMTP_OUTCOME_BOUNCE_DKIM) == SMTP_OUTCOME_BOUNCE_DKIM) {
			number++;
			explanations = (explanations | SMTP_OUTCOME_BOUNCE_DKIM);
			holder = st_merge("snsn", bounces, "    ", prefs->rcptto, " - The message failed a Domain Key Identified Mail (DKIM) check.\r\n");
		}
		else if ((prefs->outcome & SMTP_OUTCOME_BOUNCE_VIRUS) == SMTP_OUTCOME_BOUNCE_VIRUS) {
			number++;
			explanations = (explanations | SMTP_OUTCOME_BOUNCE_VIRUS);
			holder = st_merge("snsn", bounces, "    ", prefs->rcptto, " - The message appears to contain a virus.\r\n");
		}
		else if ((prefs->outcome & SMTP_OUTCOME_BOUNCE_PHISH) == SMTP_OUTCOME_BOUNCE_PHISH) {
			number++;
			explanations = (explanations | SMTP_OUTCOME_BOUNCE_PHISH);
			holder = st_merge("snsn", bounces, "    ", prefs->rcptto, " - The message appears to be a phishing attempt.\r\n");
		}
		else if ((prefs->outcome & SMTP_OUTCOME_BOUNCE_SPAM) == SMTP_OUTCOME_BOUNCE_SPAM) {
			number++;
			explanations = (explanations | SMTP_OUTCOME_BOUNCE_SPAM);
			holder = st_merge("snsn", bounces, "    ", prefs->rcptto, " - The message appears to be spam.\r\n");
		}
		else if ((prefs->outcome & SMTP_OUTCOME_BOUNCE_RBL) == SMTP_OUTCOME_BOUNCE_RBL) {
			number++;
			explanations = (explanations | SMTP_OUTCOME_BOUNCE_RBL);
			holder = st_merge("snsn", bounces, "    ", prefs->rcptto, " - The sending server's IP address is on a blocklist.\r\n");
		}

		if (holder) {
			st_cleanup(bounces);
			bounces = holder;
			holder = NULL;
		}

		prefs = (smtp_inbound_prefs_t *)prefs->next;
	}

	// Loop through and build the bounce summary.
	explain = st_merge("nnnnnnnnnn", (explanations & SMTP_OUTCOME_PERM_FAILURE) == SMTP_OUTCOME_PERM_FAILURE ? "A permanent error occurred while attempting delivery. Either the message was malformed "
		"or perhaps the forwarding address is invalid. Either way the server couldn't deliver the message to the user." : NULL, (explanations & SMTP_OUTCOME_TEMP_SERVER) == SMTP_OUTCOME_TEMP_SERVER ?
		"A temporary server error can occur if needed resources are unavailable to store the message right away. Typically resending the message will succeed.\r\n\r\n" : NULL,
		(explanations & SMTP_OUTCOME_TEMP_OVERQUOTA) == SMTP_OUTCOME_TEMP_OVERQUOTA ? "One or more of the users you attempted to e-mail is over their storage quota. Once they delete some of "
		"their e-mail off the server you will be able to e-mail them.\r\n\r\n" : NULL,
		(explanations & SMTP_OUTCOME_TEMP_LOCKED) == SMTP_OUTCOME_TEMP_LOCKED ? "The server was unable to exclusively lock one or more of the mailboxes needed for delivery. "
		"While rare, this error can occur if the mailbox is very busy. Generally resending the message will succeed.\r\n\r\n" : NULL, (explanations & SMTP_OUTCOME_BOUNCE_SPF) ==
		SMTP_OUTCOME_BOUNCE_SPF ? "The return e-mail address provided during the SMTP session failed a Sender Policy Framework (SPF) check. SPF is used to authenticate e-mail "
		"addresses and prevent spoofing. If this is a mistake consult your local system administrator and make sure you are sending your e-mail through a server authorized "
		"in the SPF record.\r\n\r\n" : NULL, (explanations & SMTP_OUTCOME_BOUNCE_DKIM) == SMTP_OUTCOME_BOUNCE_DKIM ? "The message failed a Domain Key Identified Mail (DKIM) check. "
		"DKIM is used to make sure e-mail messages arrive from mail servers authorized to represent a domain and are not modified while in transit. If you feel this is a mistake, consult "
		"your local system administrator and make sure you are sending e-mail through a server configured to properly sign outgoing mail for your domain.\r\n\r\n" : NULL,
		(explanations & SMTP_OUTCOME_BOUNCE_VIRUS) == SMTP_OUTCOME_BOUNCE_VIRUS ? "The message appears to contain mailware (typically a virus, Internet worm, or trojan). It is "
		"suggested that you run a malware scan on your computer to identify the problem. Once the problem has been identified and removed you can resend your message." : NULL,
		(explanations & SMTP_OUTCOME_BOUNCE_PHISH) == SMTP_OUTCOME_BOUNCE_PHISH ? "The message appears to be an attempt at gaining private information from the user. "
		"This practice, sometimes called \"phishing\", occurs when a message imitates an organization in which the user has a confidential relationship. If you feel this is a mistake, "
		"please modify the message below to remove any corporate references and try sending it again.\r\n\r\n" : NULL, (explanations & SMTP_OUTCOME_BOUNCE_SPAM) == SMTP_OUTCOME_BOUNCE_SPAM ?
		"The message appears to be unsolicited bulk e-mail. This user's filter has been configured to block such messages. Please modify the message, or contact the user through some other "
		"means.\r\n\r\n" : NULL, (explanations & SMTP_OUTCOME_BOUNCE_RBL) == SMTP_OUTCOME_BOUNCE_RBL ? "The sending server's IP address is on a realtime blocklist. This can occur "
		"if the sending server has recently been responsible for sending out unsolicited bulk mail. Consult your local system administrator and have them correct the issue." : NULL);


	// Build the bounce message.
	message = st_merge("nsnnnsnsnsnsnsn", "From: Magma Mail Daemon <daemon@", magma.system.domain, ">\r\nSubject: Bounce Notification\r\n", date_buffer, "To: ", con->smtp.mailfrom, "\r\n\r\n"
		"This is the Magma Mail Daemon faithfully reporting a bounced message. A message sent by you (", con->smtp.mailfrom, ") could not be delivered to all of its recipients. "
		"Please direct any questions or comments you may have to the support address, thank you.\r\n\r\n\r\n          ------- Summary -------\r\n\r\n\r\n", bounces,
		"\r\n\r\n          ------- Explanations -------\r\n\r\n\r\n", explain, "\r\n\r\n          ------- Original Message -------\r\n\r\n\r\n",
		(con->smtp.message != NULL) ? con->smtp.message->text : NULL, "\r\n\r\n");

	// Add a DKIM signature.
	if (!message) {
		log_pedantic("Unable to build the bounce message.");
	}
	else if ((signature = dkim_signature_create(id, message))) {

		if ((holder = st_merge("ss", signature, message))) {
			st_free(message);
			message = holder;
		}

	}

	// Transmit the bounce.
	if (message && smtp_client_send_data(client, message) != 1) {
		log_pedantic("An error occurred while trying to send the message.");
	}

	// Close the connection and cleanup.
	smtp_client_close(client);
	st_cleanup(bounces);
	st_cleanup(explain);
	st_cleanup(signature);
	st_cleanup(message);

	return 1;
}

int_t smtp_reply(stringer_t *from, stringer_t *to, uint64_t usernum, uint64_t autoreply, int_t spf, int_t dkim) {

	time_t utime;
	struct tm ltime;
	client_t *client;
	chr_t buffer[1024];
	static const chr_t *date_format = "Date: %a, %d %b %Y %H:%M:%S %z\r\n";
	stringer_t *message = NULL, *holder = NULL, *signature = NULL, *text = NULL, *key = NULL, *id = MANAGEDBUF(16);

	// Only send an automated reply if SPF/DKIM didn't explicitly fail.
	if (spf == -2 || dkim == -2) {
		return 0;
	}

	// Build the key.
	if (snprintf(buffer, 1024, "magma.reply.replies.%lu.%lu.", usernum, autoreply) <= 0 || (key = st_merge("ns", buffer, to)) == NULL) {
		log_pedantic("Unable to build the key.");
		return 0;
	}


	// Establish a lock, so only one bounce is set.
	if (lock_get(key) != 1) {
		log_pedantic("Unable to get a reply lock.");
		st_free(key);
		return 0;
	}

	// Make sure a reply hasn't been sent in the last 24 hours.
	if ((time(NULL) - cache_get_u64(key)) <= 86400) {
		lock_release(key);
		st_free(key);
		return 0;
	}

	// Fetch the autoreply.
	if ((text = smtp_fetch_autoreply(autoreply, usernum)) == NULL) {
		lock_release(key);
		st_free(key);
		return 0;
	}

	// Build the date string. Otherwise null the buffer so it doesn't get appended to the header.
	if ((utime = time(&utime)) == -1 || !localtime_r(&utime, &ltime) || strftime(buffer, 1024, date_format, &ltime) <= 0) {
		log_pedantic("Unable to retrieve the current time.");
		buffer[0] = '\0';
	}

	// Generate the ID string. If the random method fails, use a hash of the current time.
	if ((holder = rand_choices("ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789", 12))) {
		st_sprint(id, "%.*s", st_length_int(holder), st_char_get(holder));
		st_free(holder);
	}
	else {
		st_sprint(id, "%lu", hash_crc64(&utime, sizeof(time_t)));
	}

	// Open the connection to the SMTP server. Always use the default servers for forwards.
	if (!(client = smtp_client_connect(0))) {
		log_pedantic("Could not relay the message.");
		smtp_client_close(client);
		lock_release(key);
		st_free(text);
		st_free(key);
		return 0;
	}

	// Send HELO.
	if (smtp_client_send_helo(client) != 1) {
		log_pedantic("An error occurred while trying to say hello.");
		smtp_client_close(client);
		lock_release(key);
		st_free(text);
		st_free(key);
		return 0;
	}

	// Send NULL MAIL FROM.
	if (smtp_client_send_nullfrom(client) != 1) {
		log_pedantic("An error occurred while trying to send the mail from.");
		smtp_client_close(client);
		lock_release(key);
		st_free(text);
		st_free(key);
		return 0;
	}

	// Send the RCPT TO command. If this fails, use a permanent failure code.
	if (smtp_client_send_rcptto(client, to) != 1) {
		log_pedantic("An error occurred while trying to send a rcpt to command.");
		smtp_client_close(client);
		lock_release(key);
		st_free(text);
		st_free(key);
		return 0;
	}

	// Build the autoreply message.
	message = st_merge("nsnnnsnsn", "From: ", from, "\r\nSubject: Autoreply\r\n", buffer, "To: ", to, "\r\n\r\n", text, "\r\n\r\n");

	// Add a DKIM signature.
	if (!message) {
		log_pedantic("Unable to build the bounce message.");
	}
	else if ((signature = dkim_signature_create(id, message)) != NULL) {
		if ((holder = st_merge("ss", signature, message)) != NULL) {
			st_free(message);
			message = holder;
		}
	}

	// Transmit the auto reply and set the timestamp.
	if (message != NULL && smtp_client_send_data(client, message) == 1) {
		cache_set_u64(key, time(NULL), 86460);
	}
	else if (message != NULL) {
		log_pedantic("An error occurred while trying to send the message.");
	}

	// Close the connection and cleanup.
	smtp_client_close(client);

	// Release the lock.
	lock_release(key);

	st_cleanup(signature);
	st_cleanup(message);
	st_free(text);
	st_free(key);

	return 1;
}

/**
 * @brief	Relay an outbound smtp message for the user.
 * @param	to		a managed string containing the name of the mail recipient.
 * @param	from	a managed string containing the address from which the email is being sent.
 * @param	message	a managed string containing the raw body of the mail message.
 * @return	-1 on error or 1 on success.
 */
int_t smtp_send_message(stringer_t *to, stringer_t *from, stringer_t *message) {

	client_t *client;

	if (!to || !from || !message) {
		log_pedantic("Passed a NULL pointer.");
		return -1;
	}

	// Open the connection to the SMTP server.
	if (!(client = smtp_client_connect(1))) {
		log_pedantic("Could not relay the message.");
		smtp_client_close(client);
		return -1;
	}

	// Send HELO.
	if (smtp_client_send_helo(client)!= 1) {
		log_pedantic("An error occurred while trying to say hello.");
		smtp_client_close(client);
		return -1;
	}

	// Send MAIL FROM.
	if (smtp_client_send_mailfrom(client, from, 0) != 1) {
		log_pedantic("An error occurred while trying to send the mail from.");
		smtp_client_close(client);
		return -1;
	}

	// Send the RCPT TO command.
	if (smtp_client_send_rcptto(client, to) != 1) {
		log_pedantic("An error occurred while trying to send a rcpt to command.");
		smtp_client_close(client);
		return -1;
	}

	// Send the the message.
	if (smtp_client_send_data(client, message) != 1) {
		log_pedantic("An error occurred while trying to send the message.");
		smtp_client_close(client);
		return -1;
	}

	// Store the result.
	smtp_client_close(client);

	return 1;
}
