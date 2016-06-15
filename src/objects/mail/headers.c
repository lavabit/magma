
/**
 * @file /magma/objects/mail/headers.c
 *
 * @brief	Functions used to handle mail message header information.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Fetch the value of a specified line from a mail header, performing whitespace and line cleaning.
 * @note	This function handles mail header values that span new lines, although the output excludes line breaks and consecutive space characters.
 * @param	header	a managed string containing the full header of the target mail message.
 * @param	key		a managed string containing the name of the desired header name to be extracted, excluding the trailing ":"
 * @return	NULL on failure, or a managed string containing the cleaned value of the specified mail header line on success.
 */
stringer_t * mail_header_fetch_cleaned(stringer_t *header, stringer_t *key) {

	int_t found = 0;
	stringer_t *output;
	chr_t *start, *holder;
	size_t headlen, keylen, outlen = 0;

	if (st_empty(header) || st_empty(key)) {
		return NULL;
	}

	// Setup
	keylen = st_length_get(key);
	holder = st_char_get(header);
	headlen = st_length_get(header);

	while (!found && headlen > keylen) {

		// See if this line starts with our key.
		if (!st_cmp_ci_starts(PLACER(holder, headlen), key)) {
			holder += keylen;
			headlen -= keylen;

			// The header must also end with a colon
			if (headlen > 0 && *holder == ':') {
				holder++;
				headlen--;
				found = 1;
			}

		}

		// Advance to the next line.
		while (!found && headlen > 0 && *holder != '\n') {
			holder++;
			headlen--;
		}

		// Advance past the line break.
		if (!found && headlen > 0 && *holder == '\n') {
			holder++;
			headlen--;
		}

	}

	// We didn't find the header key.
	if (!found) {
		return NULL;
	}

	// Advance past any leading whitespace.
	while (headlen > 0 && (*holder == ' ' || *holder == '\t')) {
		holder++;
		headlen--;
	}

	// Store the beginning position.
	found = 0;
	start = holder;

	// Find the end of the header value. Values can span lines if the new line starts with a space.
	while (found != 2 && headlen > 0) {

		if (found == 0 && *holder == '\n') {
			found = 1;
			holder++;
			headlen--;
		}
		else if (found == 1 && *holder != '\t' && *holder != ' ') {
			found = 2;
		}
		else {
			found = 0;
			holder++;
			headlen--;
		}
	}

	// Make sure the header value exists.
	if (start == holder) {
		return NULL;
	}

	// Allocate a buffer for the return value.
	if (!(output = st_alloc(holder - start))) {
		return NULL;
	}

	// Setup for the copy.
	found = 0;
	headlen = holder - start;
	holder = st_char_get(output);

	// Copy the buffer. Exclude line breaks and consecutive spaces.
	while (headlen > 0) {

		if (*start >= '!' && *start <= '~') {
			outlen++;
			found = 0;
			*holder++ = *start++;
		}
		else if (found == 0 && (*start == ' ' || *start == '\t')) {
			start++;
			outlen++;
			found = 1;
			*holder++ = ' ';
		}
		else {
			start++;
		}

		headlen--;
	}

	// This will trim any trailing whitespace.
	if (outlen > 0 && *(holder - 1) == ' ') {
		st_length_set(output, outlen - 1);
	}
	else {
		st_length_set(output, outlen);
	}

	return output;
}

/**
 * @brief	Get all the values that match a named header line in a message header.
 * @param	header	a managed string containing the message header.
 * @param	key		a managed string containing the header name to be searched for.
 * @return	NULL on failure or a managed string containing all the matching header line values separated by newlines.
 */
stringer_t * mail_header_fetch_all(stringer_t *header, stringer_t *key) {

	int_t found = 0;
	size_t headlen, keylen;
	chr_t *start, *holder;
	stringer_t *output = NULL, *value;

	if (st_empty(header) || st_empty(key)) {
		return NULL;
	}

	// Setup
	keylen = st_length_get(key);
	holder = st_char_get(header);
	headlen = st_length_get(header);

	while (headlen > keylen) {

		found = 0;

		while (found == 0 && headlen > keylen) {

			// See if this line starts with our key.
			if (!mm_cmp_ci_eq(holder, key, keylen) && *(holder + keylen) == ':') {
				found = 1;
			}

			// Advance to the next line.
			while (found == 0 && headlen > 0 && *holder != '\n') {
				holder++;
				headlen--;
			}

			// Advance past the line break.
			if (found == 0 && headlen > 0 && *holder == '\n') {
				holder++;
				headlen--;
			}
		}

		// If we found the key.
		if (found != 0) {

			// Store the beginning position.
			found = 0;
			start = holder;
			holder += keylen;
			headlen -= keylen;

			// Find the end of the header value. Values can span lines if the new line starts with a space.
			while (found != 2 && headlen > 0) {

				if (found == 0 && *holder == '\n') {
					found = 1;
					holder++;
					headlen--;
				}
				else if (found == 1 && *holder != '\t' && *holder != ' ') {
					found = 2;
				}
				else {
					found = 0;
					holder++;
					headlen--;
				}
			}

			// Make sure the header value exists.
			if (!output) {
				output = st_import(start, holder - start);
			}
			else if ((value = st_merge("ss", output, PLACER(start, holder - start)))) {
				st_free(output);
				output = value;
			}

		}
	}

	return output;
}

/**
 * @brief	Pop the next header line from a mail message header.
 * @param	header		a managed string containing the entire mail message header.
 * @param	position	a pointer to a size_t that contains the tracker index into the entire header for the pop operation, and
 * 						will also receive the new value of the tracker for the subsequent call, when a new header line is located.
 * @return	pl_null() on error or a placer pointing to the next mail message subject line on success.
 */
placer_t mail_header_pop(stringer_t *header, size_t *position) {

	int_t found = 0;
	size_t headlen;
	chr_t *start, *holder;

	if (st_empty(header)) {
		return pl_null();
	}

	// Setup
	holder = st_char_get(header);
	headlen = st_length_get(header);

	if (*position >= headlen) {
		return pl_null();
	}

	start = holder += *position;
	headlen -= *position;

	// Allow for multi-line header values.
	while (found != 2 && headlen > 0) {

		if (found == 0 && *holder == '\n') {
			found = 1;
			holder++;
			headlen--;
		}
		else if (found == 1 && *holder != '\t' && *holder != ' ') {
			found = 2;
		}
		else {
			found = 0;
			holder++;
			headlen--;
		}

	}

	if (holder != start) {
		*position = (size_t)(holder - st_char_get(header));
		return pl_init(start, holder - start);
	}

	return pl_null();
}

/**
 * @brief	Return a placer pointing to the trimmed value of a mail header.
 * @note	This function merely marks a string residing between leading whitespace and a trailing \r or \n.
 * @param	stream	a pointer to a buffer containing the start of the mail header data.
 * @param	length	the length, in bytes, of the mail header field.
 * @retyrb	a placer containing the trimmed value of the mail header line.
 */
placer_t mail_store_header(chr_t *stream, size_t length) {

	size_t used = 0;
	size_t increment;

	// Skip leading whitespace.
	while (length > 0 && *stream == ' ') {
		length--;
		stream++;
	}

	for (increment = 0; increment < length; increment++) {
		if (*(stream + increment) == '\n' || *(stream + increment) == '\r') {
			used = increment;
			increment = length;
		}
	}

	return pl_init(stream, used);
}

/**
 * @brief	Parse an smtp message header and store the To, From, Date, and Subject header values.
 * @note	The found header fields will be stored inside the same smtp message object passed by the caller as input.
 * @param	message		a pointer to a partially populated smtp message object that contains the raw message data to be parsed.
 * @return	true on success or false on failure.
 */
bool_t mail_headers(smtp_message_t *message) {

	int_t next = 1;
	chr_t *stream;
	size_t length, increment;

	if (!message || !message->text) {
		log_pedantic("Invalid message passed in.");
		return false;
	}

	// Setup
	length = message->header_length;
	stream = st_char_get(message->text);

	if (length > st_length_get(message->text)) {
		log_pedantic("The header length is longer than the message.");
		return false;
	}
	else if (length != mail_header_end(message->text)) {
		log_pedantic("An invalid header length is stored in the structure.");
		return false;
	}

	// Increment through. When key headers are found, store the locations in placer's.
	for (increment = 0; increment < length; increment++) {

		// Locate the start of headers.
		if (next == 1 && *stream != '\n') {

			if (length - increment >= 3 && mm_cmp_ci_eq(stream, "To:", 3) == 0) {
				message->to = mail_store_header(stream + 3, length - increment - 3);
			}
			else if (length - increment >= 5 && mm_cmp_ci_eq(stream, "From:", 5) == 0) {
				message->from = mail_store_header(stream + 5, length - increment - 5);
			}
			else if (length - increment >= 5 && mm_cmp_ci_eq(stream, "Date:", 5) == 0) {
				message->date = mail_store_header(stream + 5, length - increment - 5);
			}
			else if (length - increment >= 8 && mm_cmp_ci_eq(stream, "Subject:", 8) == 0) {
				message->subject = mail_store_header(stream + 8, length - increment - 8);
			}

			next = 0;
		}

		// So we check the start of the next line for a key header.
		if (next == 0 && (*stream == '\n' || *stream == '\r')) {
			next = 1;
		}

		stream++;
	}

	return true;
}

/**
 * @brief	Prepend text to a Subject header line inside of a mail message.
 * @note	If no Subject line is found in the header of the message, one will be created and inserted with the specified label at the end of the header.
 * @param	message		a pointer to the address of a managed string that contains the mail message header or mail message body, and
 * 						will receive the value of the resulting managed string that will be allocated to hold the modified contents.
 * @param	label		a null-terminated string that will be prepended to the value of the subject header value.
 * @return	This function returns no value.
 */
void mail_mod_subject(stringer_t **message, chr_t *label) {

	int_t found = 0;
	stringer_t *result;
	size_t position = 0;
	placer_t line;
	stringer_t *header, *first = NULL, *second = NULL;

	if (!message || !*message || !label) {
		log_pedantic("Passed a NULL pointer.");
		return;
	}

	// Get the message header and see if the Subject line is present.
	header = PLACER(st_char_get(*message), mail_header_end(*message));

	while (found != 1 && !pl_empty((line = mail_header_pop(header, &position)))) {
		if (st_length_get(&line) >= 8) {
			found = st_cmp_ci_starts(&line, CONSTANT("Subject:")) == 0 ? 1 : 0;
		}
	}

	// Subject found.
	if (found == 1) {
		first = PLACER(st_data_get(*message), (st_char_get(&line) + 8) - st_char_get(*message));
		second = PLACER(st_data_get(&line) + 8, st_length_get(*message) - st_length_get(first));
	}
	// Subject not found.
	else {

		if (st_length_get(header) > 2) {
			position = st_length_get(header) - 2;
		}
		else {
			position = 0;
		}

		// If none exists, we will create our own Subject line and insert it right before the final trailing newlines of the subject.
		first = PLACER(st_char_get(*message), position);
		second = PLACER(st_char_get(*message) + position, st_length_get(*message) - position);
	}

	if (found == 1) {
		result = st_merge("snnns", first, " ", label, *st_char_get(second) != ' ' ? " " : NULL, second);
	}
	else {
		result = st_merge("snnns", first, "Subject: ", label, "\r\n", second);
	}

	if (!result) {
		log_pedantic("An error occurred while trying to label a message.");
		return;
	}

	st_free(*message);
	*message = result;

	return;
}

/**
 * @brief	Generate any missing required headers for an smtp message object.
 * @note	These required headers include To, From, Subject and Date, and if they are absent, they will be
 * 			generated from the specified connection's inbound or outbound preferences, accordingly.
 * @param	con			a pointer to the connection object across which the smtp message was received.
 * @param	message		the smtp message object to be updated for any missing required headers.
 * @return	true on success or false on failure.
 */
bool_t mail_add_required_headers(connection_t *con, smtp_message_t *message) {

	int_t state;
	time_t utime;
	struct tm ltime;
	size_t length = 0;
	smtp_recipients_t *outbound;
	smtp_inbound_prefs_t *inbound;
	stringer_t *to = NULL, *from = NULL, *date = NULL, *holder = NULL, *lines = NULL, *header, *body;
	static const chr_t *date_format = "Date: %a, %d %b %Y %H:%M:%S %z\r\n";

	// Generate a date.
	if (st_empty(&(message->date))) {

		// Store the current time.
		if ((utime = time(&utime)) == -1) {
			log_pedantic("Unable to retrieve the current time.");
			return false;
		}

		// Break the time up into its component parts.
		if (!localtime_r(&utime, &ltime)) {
			log_pedantic("Unable to break the current time up into its component parts.");
			return false;
		}

		if (!(date = st_alloc(1024))) {
			log_pedantic("Unable to allocate 1024 bytes for the date header.");
			return false;
		}

		// Print_t the time into a buffer according the RFC defined format.
		state = strftime(st_char_get(date), st_avail_get(date), date_format, &ltime);

		if (state <= 0) {
			log_pedantic("Unable to build the date header.");
			st_free(date);
			return false;
		}

		// Store the size of the date.
		st_length_set(date, state);
	}

	// Build the default from header.
	if (st_empty(&(message->from))) {

		if (!con->smtp.mailfrom) {
			from = st_import("From: \r\n", 8);
		}
		else {
			from = st_merge("nsn", "From: ", con->smtp.mailfrom, "\r\n");
		}

		if (!from) {
			log_pedantic("Unable to build the from header line.");
			st_cleanup(date);
			return false;
		}

	}

	// Build the default to header for inbound messages.
	if (st_empty(&(message->to)) && !con->smtp.authenticated) {

		if (!con->smtp.in_prefs || !con->smtp.in_prefs->rcptto) {
			to = st_import("To: \r\n", 6);
		}
		else {
			to = st_merge("ns", "To: ", con->smtp.in_prefs->rcptto);
			inbound = (smtp_inbound_prefs_t *) con->smtp.in_prefs->next;

			while (inbound) {
				holder = to;
				to = st_merge("sns", to, ", ", inbound->rcptto);

				if (to) {
					st_free(holder);
				}
				else {
					to = holder;
				}

				inbound = (smtp_inbound_prefs_t *)inbound->next;
			}

			holder = to;
			to = st_merge("sn", to, "\r\n");

			if (to) {
				st_free(holder);
			}
			else {
				to = holder;
			}

		}

	}
	// Build the default to header for outbound messages.
	if (st_empty(&(message->to)) && con->smtp.authenticated) {

		if (!con->smtp.out_prefs || !con->smtp.out_prefs->recipients || !con->smtp.out_prefs->recipients->address) {
			to = st_import("To: \r\n", 6);
		}
		else {
			to = st_merge("ns", "To: ", con->smtp.out_prefs->recipients->address);
			outbound = (smtp_recipients_t *) con->smtp.out_prefs->recipients->next;

			while (outbound) {
				holder = to;
				to = st_merge("sns", to, ", ", outbound->address);

				if (to) {
					st_free(holder);
				}
				else {
					to = holder;
				}

				outbound = (smtp_recipients_t *)outbound->next;
			}

			holder = to;
			to = st_merge("sn", to, "\r\n");

			if (to) {
				st_free(holder);
			}
			else {
				to = holder;
			}

		}

	}

	// Combine the required headers into a single string.
	if (date || from || to || st_empty(&(message->subject))) {
		lines = st_merge("ssns", date, from, st_empty(&(message->subject)) ? "Subject: \r\n" : NULL, to);
		st_cleanup(date);
		st_cleanup(from);
		st_cleanup(to);
	}

	// This will insert the new headers into the message body. If it fails, we just don't replace.
	if (lines) {

		if (message->header_length > 2) {
			length = message->header_length - 2;
		}
		else {
			length = 0;
		}

		header = PLACER(st_char_get(message->text), length);
		body = PLACER(st_char_get(message->text) + length, st_length_get(message->text) - length);
		holder = st_merge_opts(MAPPED_T | JOINTED | HEAP, "sss", header, lines, body);

		st_free(lines);

		if (!holder) {
			return false;
		}

		// Setup the structure with the new message.
		st_free(message->text);
		message->text = holder;
		message->header_length = mail_header_end(message->text);
		mail_headers(message);
	}

	return true;
}

/**
 * @brief	Prepend the Return-Path: and Received: headers to an inbound smtp message.
 * @param	con		the connection across which the inbound smtp message was accepted.
 * @param	prefs	the smtp inbound preferences corresponding to the connection, with the rcptto field populated.
 * @return	NULL on failure, or a managed string containing the message data preceded by the Return-Path and Received headers on success.
 */
stringer_t * mail_add_inbound_headers(connection_t *con, smtp_inbound_prefs_t *prefs) {

	int_t state;
	time_t utime;
	struct tm ltime;
	chr_t date_string[40];
	stringer_t *ip, *result, *reverse;

	if (!con) {
		log_pedantic("Passed a NULL pointer.");
		return NULL;
	}

	// Code to generate a proper timestamp.
	if ((utime = time(&utime)) == -1) {
		log_pedantic("Could not determine the proper time.");
		return NULL;
	}

	if (!localtime_r(&utime, &ltime)) {
		log_pedantic("Could not determine the local time.");
		return NULL;
	}

	if ((state = strftime(date_string, 40, "%a, %d %b %Y %H:%M:%S %z", &ltime)) <= 0) {
		log_pedantic("Could not build the date string.");
		return NULL;
	}

	// Build the IP string.
	else if (!(ip = con_addr_presentation(con, MANAGEDBUF(64)))) {
		log_pedantic("Could not convert the IP into a string.");
		return NULL;
	}

	// We need to make sure the reverse DNS lookup is complete.
	reverse = con_reverse_check(con, 20);

	if (!reverse || !st_cmp_ci_eq(reverse, con->smtp.helo))
	{
		// The reverse matches, or doesn't exist and there is no mailfrom or it's <>.
		if (!con->smtp.mailfrom || !st_cmp_cs_eq(con->smtp.mailfrom, CONSTANT("<>"))) {
			result =  st_merge_opts(MAPPED_T | JOINTED | HEAP, "nsnsnsnsnsnnns", "Return-Path: <>\r\nReceived: from ", con->smtp.helo, " (", ip, ")\r\n\tby ", con->server->domain,
				(con->smtp.esmtp == false) ? " with SMTP id " : " with ESMTP id ", con->smtp.message->id, "\r\n\tfor <", prefs->rcptto, ">; ",
				date_string, "\r\n", con->smtp.message->text);
		}
		// The reverse matches, or doesn't exist and there is a mailfrom to print.
		else {
			result = st_merge_opts(MAPPED_T | JOINTED | HEAP, "nsnsnsnsnsnsnnns", "Return-Path: <", con->smtp.mailfrom, ">\r\nReceived: from ", con->smtp.helo, " (", ip, ")\r\n\tby ",
				con->server->domain, (con->smtp.esmtp == false) ? " with SMTP id " : " with ESMTP id ", con->smtp.message->id, "\r\n\tfor <", prefs->rcptto, ">; ",
				date_string, "\r\n", con->smtp.message->text);
		}

	}
	// We need to print_t a reverse, but there is no mailfrom or it's <>.
	else if (!con->smtp.mailfrom || !st_cmp_cs_eq(con->smtp.mailfrom, CONSTANT("<>"))) {
			result = st_merge_opts(MAPPED_T | JOINTED | HEAP, "nsnsnsnsnsnsnnns", "Return-Path: <>\r\nReceived: from ", con->smtp.helo, " (", reverse, " [", ip, "])\r\n\tby ", con->server->domain,
				(con->smtp.esmtp == false) ? " with SMTP id " : " with ESMTP id ", con->smtp.message->id, "\r\n\tfor <", prefs->rcptto, ">; ",
				date_string, "\r\n", con->smtp.message->text);
	}
	// We need to print_t a reverse and the mailfrom.
	else {
		result = st_merge_opts(MAPPED_T | JOINTED | HEAP, "nsnsnsnsnsnsnsnnns", "Return-Path: <", con->smtp.mailfrom, ">\r\nReceived: from ", con->smtp.helo, " (", reverse,
			" [", ip, "])\r\n\tby ", con->server->domain,	(con->smtp.esmtp == false) ? " with SMTP id " : " with ESMTP id ", con->smtp.message->id,
			"\r\n\tfor <", prefs->rcptto, ">; ", date_string, "\r\n", con->smtp.message->text);
	}

	if (!result) {
		log_pedantic("Could not build the message with the spiffy new inbound headers.");
		return NULL;
	}

	return result;
}

/**
 * @brief	Add necessary headers to a forwarded mail message.
 * @note	This function will prepend tags to the Subject line like "JUNK", "INFECTED", "SPOOFED", etc.
 * 			The following headers will be stripped from the message: Sender, Return-Path, DKIM-Signature, and DomainKey-Signature.
 * 			This daemon will be reinserted into the headers as the origin of the Sender: header value.
 * 			If signum and sigkey are both set, then a spam training url will also be inserted into the message.
 *			Finally, if magma.dkim.enabled is set, a dkim signature will be added to the message.
 * @param	server		the server object of the web server where the teacher application is hosted.
 * @param	message		the address of a managed string containing the message data that will be set to the modified messagee data on success.
 * @param	id			a managed string containing the message id (for dkim).
 * @param	mark		a bitmask of marks to be tagged in the message subject (SMTP_MARK_SPAM, SMTP_MARK_VIRUS, SMTP_MARK_SPOOF,
 * 						SMTP_MARK_RBL, and SMTP_MARK_PHISH).
 * @param	signum		the optional spam signature number referenced by the teacher url.
 * @param	sigkey		the optional spam signature key for client verification in the teacher app.
 * @return	This function returns no value.
 */
// QUESTION: This prototype is stupid. We need to pass message as stringer_t * and return a stringer_t * as well.
void mail_add_forward_headers(server_t *server, stringer_t **message, stringer_t *id, int_t mark, uint64_t signum, uint64_t sigkey) {

	uint32_t status = 0;
	size_t position = 0;
	placer_t line;
	mail_message_t *modifier;
	stringer_t *result, *cleaned, *signature, *header, *first = NULL, *second = NULL;

	if (!server || !message || !*message) {
		log_pedantic("Passed a NULL parameter.");
		return;
	}

	if (!(result = st_dupe_opts(MAPPED_T | JOINTED | HEAP, *message))) {
		log_pedantic("Unable to duplicate message.");
		return;
	}

	// Label the message.
	if ((mark & SMTP_MARK_SPAM) == SMTP_MARK_SPAM) {
		mail_mod_subject(&result, "JUNK:");
		status += MAIL_MARK_JUNK;
	}
	else if ((mark & SMTP_MARK_VIRUS) == SMTP_MARK_VIRUS) {
		mail_mod_subject(&result, "INFECTED:");
	}
	else if ((mark & SMTP_MARK_SPOOF) == SMTP_MARK_SPOOF) {
		mail_mod_subject(&result, "SPOOFED:");
	}
	else if ((mark & SMTP_MARK_RBL) == SMTP_MARK_RBL) {
		mail_mod_subject(&result, "BLACKHOLED:");
	}
	else if ((mark & SMTP_MARK_PHISH) == SMTP_MARK_PHISH) {
		mail_mod_subject(&result, "PHISHING:");
	}

	// Remove unnecessary headers, including: Return-Path, Sender, DomainKey-Signature, DKIM-Signature.
	if ((cleaned = st_alloc_opts(MAPPED_T | JOINTED | HEAP, (st_length_get(result) * 2) + 52)) == NULL) {
		log_pedantic("Unable to setup a buffer for the cleaned message.");
		st_free(result);
		return;
	}

	header = PLACER(st_char_get(result), mail_header_end(result));

	while (!pl_empty(line = mail_header_pop(header, &position))) {

		if (st_cmp_ci_starts(&line, CONSTANT("Sender:")) != 0 &&
			st_cmp_ci_starts(&line, CONSTANT("Return-Path:")) != 0 &&
			st_cmp_ci_starts(&line, CONSTANT("DKIM-Signature:")) != 0 &&
			st_cmp_ci_starts(&line, CONSTANT("DomainKey-Signature:")) != 0) {
			cleaned = st_append_opts(1024, cleaned, &line);
		}

	}

	cleaned = st_append_opts(1024, cleaned, PLACER("Sender: Magma Mail Daemon <daemon@lavabit.com>\r\n\r\n", 52));
	cleaned = st_append_opts(1024, cleaned, PLACER(st_char_get(result) + st_length_get(header), st_length_get(result) - st_length_get(header)));

	st_cleanup(result);
	result = cleaned;

	// If necessary, insert a spam signature.
	if (signum && sigkey) {

		if (!(modifier = mail_message(result))) {
			log_pedantic("Unable to build the message structure.");
			st_free(result);
			return;
		}

		mail_signature_add(modifier, server, signum, sigkey, (status & MAIL_MARK_JUNK) == MAIL_MARK_JUNK ? 1 : 0);
		result = st_dupe(modifier->text);
		mail_destroy(modifier);
	}

	// Split the message apart.
	position = 0;
	header = PLACER(st_char_get(result), mail_header_end(result));
	while (!pl_empty(line = mail_header_pop(header, &position)) && st_length_get(&line) >= 9 && mm_cmp_ci_eq(st_data_get(&line), "Received:", 9) == 0);

	if (!pl_empty(line)) {
		first = PLACER(st_char_get(result), st_char_get(&line) - st_char_get(result));
		second = PLACER(st_data_get(&line), st_length_get(result) - st_length_get(first));
	}
	else {
		second = PLACER(st_char_get(result), st_length_get(result));
	}

	// Generate the message signature, and insert.
	if (magma.dkim.enabled && ((signature = dkim_create(id, result)))) {
		cleaned = st_merge_opts(MAPPED_T | JOINTED | HEAP, "sss", first, signature, second);
		st_free(signature);

		if (cleaned) {
			st_cleanup(result);
			result = cleaned;
		}

	}

	// Setup the output.
	if (result) {
		st_free(*message);
		*message = result;
	}

	return;
}

/**
 * @brief	Add a Received: header and dkim signature to an outbound relayed smtp message.
 * @note	If the message already has a Received header, the dkim signature will be inserted right before the first instance.
 * @param	con		the connection across which the outbound smtp message is being sent.
 * @return	NULL on failure, or a managed string containing the message data preceded by the Received header
 * 			and including the dkim signature on success.
 */
int_t mail_add_outbound_headers(connection_t *con) {

	int_t state;
	time_t utime;
	placer_t line;
	struct tm ltime;
	chr_t date_string[40];
	size_t position = 0;
	stringer_t *new, *ip, *reverse, *dk_signature = NULL, *first = NULL, *second = NULL, *header = NULL;

	if (!con || !con->smtp.message || !con->smtp.message->text) {
		log_pedantic("Passed a NULL pointer.");
		return -1;
	}

	// Code to generate a proper timestamp.
	if ((utime = time(&utime)) == -1) {
		log_pedantic("Could not determine the proper time.");
		return -1;
	}

	if (!localtime_r(&utime, &ltime)) {
		log_pedantic("Could not determine the local time.");
		return -1;
	}

	if ((state = strftime(date_string, 40, "%a, %d %b %Y %H:%M:%S %z", &ltime)) <= 0) {
		log_pedantic("Could not build the date string.");
		return -1;
	}

	// Build the IP string.
	else if (!(ip = con_addr_presentation(con, MANAGEDBUF(64)))) {
		log_pedantic("Could not convert the IP into a string.");
		return -1;
	}

	// Split the message.
	header = PLACER(st_char_get(con->smtp.message->text), mail_header_end(con->smtp.message->text));

	while (!pl_empty(line = mail_header_pop(header, &position)) && st_length_get(&line) >= 9 && !mm_cmp_ci_eq(st_char_get(&line), "Received:", 9));

	if (!st_empty(&line)) {
		first = PLACER(st_char_get(con->smtp.message->text), st_char_get(&line) - st_char_get(con->smtp.message->text));
		second = PLACER(st_data_get(&line), st_length_get(con->smtp.message->text) - st_length_get(first));
	}
	else {
		second = PLACER(st_char_get(con->smtp.message->text), st_length_get(con->smtp.message->text));
	}

	// Generate the message signature.
	if (magma.dkim.enabled) {
		dk_signature = dkim_create(con->smtp.message->id, con->smtp.message->text);
	}

	// We need to make sure the reverse DNS lookup is complete.
	reverse = con_reverse_check(con, 20);

	// Detect no HELO name.
	if (!con->smtp.helo && reverse) {
		con->smtp.helo = st_dupe(reverse);
	}
	else if (!con->smtp.helo) {
		con->smtp.helo = st_import("none", 4);
	}

	if (!reverse || !st_cmp_cs_eq(reverse, con->smtp.helo)) {

		if (con->smtp.num_recipients > 1) {
			new = st_merge_opts(MAPPED_T | JOINTED | HEAP, "nsnsnsnsnnnsss", "Received: from ", con->smtp.helo, " (", ip, ")\r\n\tby ", con->server->domain,
				(con->smtp.esmtp == false) ? " with SMTP id " : " with ESMTP id ", con->smtp.message->id, "; ", date_string, "\r\n", first, dk_signature, second);
		}
		else {
			new = st_merge_opts(MAPPED_T | JOINTED | HEAP, "nsnsnsnsnsnnnsss", "Received: from ", con->smtp.helo, " (", ip, ")\r\n\tby ", con->server->domain,
				(con->smtp.esmtp == false) ? " with SMTP id " : " with ESMTP id ", con->smtp.message->id, "\r\n\tfor <", con->smtp.out_prefs->recipients->address, ">; ",
				date_string, "\r\n", first, dk_signature, second);
		}

	}
	else if (con->smtp.num_recipients > 1) {
		new = st_merge_opts(MAPPED_T | JOINTED | HEAP, "nsnsnsnsnsnnnsss", "Received: from ", con->smtp.helo, " (", reverse, " [", ip, "])\r\n\tby ", con->server->domain,
			(con->smtp.esmtp == false) ? " with SMTP id " : " with ESMTP id ", con->smtp.message->id, "; ", date_string, "\r\n", first, dk_signature, second);
	}
	else {
		new = st_merge_opts(MAPPED_T | JOINTED | HEAP, "nsnsnsnsnsnsnnnsss", "Received: from ", con->smtp.helo, " (", reverse, " [", ip, "])\r\n\tby ", con->server->domain,
			(con->smtp.esmtp == false) ? " with SMTP id " : " with ESMTP id ", con->smtp.message->id, "\r\n\tfor <", con->smtp.out_prefs->recipients->address, ">; ",
			date_string, "\r\n", first, dk_signature, second);
	}

	st_cleanup(dk_signature);

	if (new) {
		st_free(con->smtp.message->text);
		con->smtp.message->text = new;
	}
	else {
		log_pedantic("Could not build the message with the spiffy new outbound headers.");
		return -1;
	}

	return 1;
}

