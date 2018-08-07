
/**
 * @file /magma/web/portal/mail.c
 *
 * @brief	Functions for various smtp-level functionality for use in conjunction with the portal.
 */

#include "magma.h"

/**
 * @brief	Perform a series of security-related checks on an outbound email message in a transport-independent manner.
 * @see		smtp_data_outbound()
 * @note	The checks include: Pattern matching for junk mail, authorization for the user to use the email address or domain
 * 			specified in the From address, and finally, a virus check.
 * @param	cred		a credential structure obtained for the authenticated user's session.
 * @param	usernum		the numerical id of the user attempting to send the message.
 * @param	from		a managed string containing the email address specified as the From address.
 * @param	nrecipients	the number of recipients that will receive the sent message.
 * @param	body_plain	a managed string containing the plain text body of the message.
 * @param	body_html	a managed string containing the html body of the message.
 * @param	errmsg	the address of a pointer to a null-terminated string that will be set to a descriptive error message on failure.
 * @return	true if all security checks were passed or false otherwise.
 */
bool_t portal_outbound_checks(uint64_t usernum, stringer_t *username, stringer_t *verification, stringer_t *from, size_t num_recipients, stringer_t *body_plain, stringer_t *body_html, chr_t **errmsg) {

	smtp_outbound_prefs_t *prefs;
	int_t state, nstate;

	if (!usernum || !from || !num_recipients || (!body_plain && !body_html) || !errmsg) {
		*errmsg = "Unexpected internal failure occurred while sending message.";
		return false;
	}

	// Check the outbound blocker list.
	if ((pattern_check(body_plain) == -2) || (pattern_check(body_html) == -2)) {
		*errmsg = "Message was blocked on suspicion of being junk mail.";
		return false;
	}

	// We need to grab the user's outbound credentials before we can check the transmit quota.
	if (smtp_fetch_authorization(username, verification, &prefs)) {
		*errmsg = "User failed to meet authorization check.";
		return false;
	}

	// Check the transmit quota one more time before we actually send the message.
	else if (smtp_check_transmit_quota(usernum, num_recipients, prefs) == 1) {
		*errmsg = "User is over maximum 24 hour send limit.";
		//con_print(con, "451 DATA BLOCKED - THIS USER ACCOUNT IS ONLY ALLOWED TO SEND %u %s IN A TWENTY-FOUR HOUR PERIOD - PLEASE TRY AGAIN LATER\r\n",
		//	con->smtp.out_prefs->daily_send_limit, (con->smtp.out_prefs->daily_send_limit != 1) ? "MESSAGES" : "MESSAGE");
		return false;
	}

	// Now check the from address.
	else if (!(state = smtp_check_authorized_from(usernum, from))) {
		*errmsg = "Account was not authorized to send emails from this address.";
		return false;
	}
	else if (state < 0) {
		*errmsg = "An unexpected error occurred while authorizing this email to be sent.";
		return false;
	}

	// Make sure this message does not contain a virus.
	else if (((state = virus_check(body_plain)) == -2) || ((nstate = virus_check(body_html)) == -2)) {
		*errmsg = "Email message was blocked because it failed virus scan.";
		return false;
	} else if (state == -3 || nstate == -3) {
		*errmsg = "Email message was blocked because it was detected as a phishing attempt.";
		return false;
	}

	return true;
}

/**
 * @brief	Send (relay) a message composed by a user via a portal session.
 * @see	smtp_relay_message() - a lot of logic borrowed from here.
 * @param	from		a pointer to a managed string containing the email address specified as the From address.
 * @param	to			a pointer to a managed string containing the destination email address of the message.
 * @param	data		a pointer to a managed string containing the raw data of the mail message.
 * @param	send_size	if greater than 0, specify the optional SIZE parameter to the MAIL FROM command.
 * @param	errmsg		the address of a pointer to a null-terminated string that will be set to a descriptive error message on failure.
 * @return	true if the mail message was sent successfully, or false otherwise.
 */
bool_t portal_smtp_relay_message(stringer_t *from, inx_t *to, stringer_t *data, size_t send_size, chr_t **errmsg) {

	inx_cursor_t *cursor;
	stringer_t *to_address;
	client_t *client;
	int_t state, nsentto = 0;

	if (!from || !to || !data || !errmsg) {
		*errmsg = "Unexpected internal failure occurred while sending message.";
		return false;
	}

	/*if (mail_add_outbound_headers(con) != 1) {
		log_pedantic("Could not add the outbound headers.");
		return false;
	}*/

	// Open the connection to the SMTP server.
	if (!(client = smtp_client_connect(0))) {
		*errmsg = "Encountered transport error with relay server.";
		return false;
	}

	// Send HELO.
	if (smtp_client_send_helo(client)!= 1) {
		*errmsg = "Handshake with relay server failed.";
		smtp_client_close(client);
		return false;
	}

	// Send MAIL FROM.
	if ((state = smtp_client_send_mailfrom(client, from, send_size)) != 1) {
		*errmsg = "Error occurred while sending From field to email server.";
		smtp_client_close(client);
		return false;
	}

	// Send the RCPT TO command.
	if (!(cursor = inx_cursor_alloc(to))) {
		*errmsg = "Internal occurred while expanding recipient list.";
		smtp_client_close(client);
		return false;
	}

	while ((to_address = inx_cursor_value_next(cursor))) {

		if ((state = smtp_client_send_rcptto(client, to_address)) != 1) {
			*errmsg = "Error occurred while specifying recipient to email server.";
			smtp_client_close(client);
			inx_cursor_free(cursor);
			return false;
		}

		nsentto++;
	}

	inx_cursor_free(cursor);

	if (!nsentto) {
		*errmsg = "Mail message could not be sent without recipient.";
		smtp_client_close(client);
		return false;
	}

	// Send the the message.
	if ((state = smtp_client_send_data(client, data, true)) != 1) {
		*errmsg = "Error occurred while sending email message body.";
		smtp_client_close(client);
		return false;
	}

	// Store the result.
	//*result = st_dupe_opts(MANAGED_T | CONTIGUOUS | HEAP, &(client->line));
	smtp_client_close(client);

	// TODO: smtp_update_transmission_stats() needs to be called here.
	return true;
}

/**
 * @brief	Create the data of an outbound smtp message that will be specified with the smtp DATA command.
 * @param	attachments		an optional inx holder containing a list of attachments to be included in the message.
 * @param	from			a managed string containing the email address sending the message.
 * @param	to				an inx holder containing a list of To: email recipients as managed strings.
 * @param	cc				an inx holder containing a list of 0 or more CC: recipients as managed strings.
 * @param	bcc				an inx holder containing a list of 0 or more BCC: recipients as managed strings.
 * @param	subject			a managed string containing the subject of the message.
 * @param	body_plain		an optional managed string containing the plain text body of the message.
 * @param	body_html		an optional managed string containing the html-formatted body of the message.
 * @return	NULL on failure or a managed string containing the packaged outbound smtp message data on success.
 */
stringer_t * portal_smtp_create_data(inx_t *attachments, stringer_t *from, inx_t *to, inx_t *cc, inx_t *bcc, stringer_t *subject, stringer_t *body_plain, stringer_t *body_html) {

		inx_cursor_t *cursor = NULL;
		array_t *all_attachments = NULL;
		attachment_t *attachment;
		stringer_t *result = NULL, *boundary = NULL, *mime_data, *tmp;
		size_t nattached = 0;

		if (attachments && (!(cursor = inx_cursor_alloc(attachments)))) {
			log_pedantic("Unable to read message attachments.");
			return NULL;
		} else if (attachments) {

			while ((attachment = inx_cursor_value_next(cursor))) {

					// We are creating an array of all the attachment data. It needs to be ARRAY_TYPE_POINTER so deallocating the array doesn't free the underlying data.
					if (attachment->filedata && !ar_append(&all_attachments, ARRAY_TYPE_POINTER, attachment->filedata)) {
						log_pedantic("Unable to parse message attachments.");
						inx_cursor_free(cursor);
						if (all_attachments) ar_free(all_attachments);
						return NULL;
					}

			}

			if (all_attachments) {
				nattached = ar_length_get(all_attachments);
			}

		}

		if (!nattached) {

			if (cursor) {
				inx_cursor_free(cursor);
			}

			if (!(result = mail_mime_get_smtp_envelope(from, to, cc, bcc, subject, boundary, false))) {
						log_pedantic("Unable to generate smtp envelope for outbound mail.");
						return NULL;
			}

		} else {

			// TODO: This should really happen after encoding is performed, but the chances of a collision are still infinitesimal.
			// Now that we have all the attachment data in an array, we can generate a unique boundary string.
			if (!(boundary = mail_mime_generate_boundary(all_attachments))) {
				log_pedantic("Unable to generate boundary for MIME attachments.");
				inx_cursor_free(cursor);
				ar_free(all_attachments);
				return NULL;
			}

			ar_free(all_attachments);

			// Get the envelope for a message that has attachments.
			if (!(result = mail_mime_get_smtp_envelope(from, to, cc, bcc, subject, boundary, true))) {
				log_pedantic("Unable to generate smtp envelope for outbound mail.");
				inx_cursor_free(cursor);
				st_free(boundary);
				return NULL;
			}

			// Now that we have the envelope, the first content is the actual email body.
			if (!(tmp = st_merge("snsnsn", result, "--------------", boundary, "\r\nContent-Type: text-plain\r\nContent-Transfer-Encoding: 7bit\r\n\r\n", body_plain, "\r\n"))) {
				log_pedantic("Unable to pack message body into outbound message.");
				inx_cursor_free(cursor);
				st_free(boundary);
				return NULL;
			}

			st_free(result);
			result = tmp;

			// Now go through each attachment, encode it, and append it to the envelope.
			inx_cursor_reset(cursor);

			while ((attachment = inx_cursor_value_next(cursor))) {

				if (!(mime_data = mail_mime_encode_part(attachment->filedata, attachment->filename, boundary))) {
					log_pedantic("Unable to mime encode part for message attachment.");
					inx_cursor_free(cursor);
					st_free(boundary);
					return NULL;
				}

				tmp = st_merge("ss", result, mime_data);
				st_free(mime_data);
				st_free(result);

				if (!tmp) {
					log_pedantic("Unable to allocate space for portal smtp message.");
					inx_cursor_free(cursor);
					return NULL;
				}

				result = tmp;
			}

			inx_cursor_free(cursor);

			// One final boundary at the end.
			tmp = st_merge("snsn", result, "\r\n--------------", boundary, "--");
			st_free(result);

			if (!tmp) {
				log_pedantic("Unable to allocate space for portal smtp message.");
				return NULL;
			}

			result = tmp;
		}

		return result;
}


/**
 * @brief	Merge a list of smtp message headers into a single string, preceded by the leading text and followed by the trailing text.
 * @param	headers		an inx holder containing a collection of header string data to be merged together.
 * @param	leading		a managed string containing text that will lead each header line.
 * @param	trailing	a managed string containing text that will trail each header line.
 * @return	NULL on failure or a managed string containing the merged headers on success.
 */
stringer_t * portal_smtp_merge_headers(inx_t *headers, stringer_t *leading, stringer_t *trailing) {

	stringer_t *result = NULL, *tmp, *current;
	inx_cursor_t *cursor;

	if (!headers || !leading || !trailing) {
		return NULL;
	}

	if (!(cursor = inx_cursor_alloc(headers))) {
		return NULL;
	}

	while ((current = inx_cursor_value_next(cursor))) {

		if (!(tmp = st_merge("ssss", result, leading, current, trailing))) {
			inx_cursor_free(cursor);
			st_cleanup(result);
			return NULL;
		}

		result = tmp;
	}

	inx_cursor_free(cursor);

	// We should at least return an empty managed string if we have a valid inx holder but no data in it.
	if (!result) {
		result = st_alloc(0);
	}

	return result;
}
