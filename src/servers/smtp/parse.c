
/**
 * @file /magma/servers/smtp/parse.c
 *
 * @brief	Functions used to parse command parameters from SMTP clients.
 */

#include "magma.h"

/// LOW: The list of characters allowed in an email address needs to be verified against the RFC's.
/// LOW: The parser should use different lists of valid characters for the the local and domain portions of an email address.

/**
 * Extract the provided path from the command line
 * Reverse-path = Path
 * Forward-path = Path
 * Path = "<" [ A-d-l ":" ] Mailbox ">"
 * A-d-l = At-domain *( "," A-d-l )
 * At-domain = @ domain
 *
 * @param con A connection structure which presumably contains a the RCPT TO value inside the line buffer.
 * @return Returns a stringer with the path that was extracted or NULL to indicate a problem.
 */
stringer_t * smtp_parse_rcpt_to(connection_t *con) {

	size_t length;
	stringer_t *result;
	chr_t *input, *start;

	if (!con || pl_empty(con->network.line)) {
		log_pedantic("Invalid data was passed in for parsing.");
		return NULL;
	}
	else if (pl_length_get(con->network.line) <= 7) {
		log_pedantic("The RCPT TO line is too short. {%s = %.*s}", con->command->string, pl_length_int(pl_trim_end(con->network.line)),
				pl_char_get(pl_trim_end(con->network.line)));
		return NULL;
	}

	// This is a RCPT TO so we skip the first seven characters.
	input = pl_char_get(con->network.line) + 7;
	length = pl_length_get(con->network.line) - 7;

	// The RCPT TO should be immediately followed by a colon. Then possibly spaces.
	while (length && (*input == ':' || *input == ' ')) {
		input++;
		length--;
	}

	// Make sure something is still there.
	if (!length) {
		log_pedantic("Did not find any valid characters in the RCPT TO argument. {%s = %.*s}", con->command->string, pl_length_int(pl_trim_end(con->network.line)),
				pl_char_get(pl_trim_end(con->network.line)));
		return NULL;
	}

	else {

		while (length && (*input == '<')) {
			input++;
			length--;
		}

		// Make sure something is still there.
		if (!length) {
			log_pedantic("Did not find any valid characters in the RCPT TO argument. {%s = %.*s}", con->command->string, pl_length_int(pl_trim_end(con->network.line)),
				pl_char_get(pl_trim_end(con->network.line)));
			return NULL;
		}

		// Record the starting position.
		start = input;

		// Get all of the valid characters.
		// See RFC 5321 and 5621 for rules on what characters make up a valid email addresses.
		while (length && ((*input >= 'A' && *input <= 'Z') || (*input >= 'a' &&
			*input <= 'z') || (*input >= '0' && *input <= '9') || *input == '-' ||
			*input == '_' || *input == '.' || *input == '?' || *input == '@' ||
			*input == '+' || *input == '#' || *input == '=' || *input == '%' ||
			*input == '&' || *input == '$' || *input == '!' || *input == '\'')) {
			*input = lower_chr(*input);
			input++;
			length--;
		}

		// Make sure we still have something to process.
		if (input == start) {
			log_pedantic("Did not find any valid characters in the RCPT TO argument. {%s = %.*s}", con->command->string, pl_length_int(pl_trim_end(con->network.line)),
				pl_char_get(pl_trim_end(con->network.line)));
			return NULL;
		}

		// Import the RCPT TO argument into a stringer. Take into account the max length.
		if (input - start > magma.smtp.address_length_limit) result = st_import(start, magma.smtp.address_length_limit);
		else result = st_import(start, input - start);

		if (!result) {
			log_pedantic("Could not import the RCPT TO argument into a stringer. {%s = %.*s}", con->command->string, pl_length_int(pl_trim_end(con->network.line)),
				pl_char_get(pl_trim_end(con->network.line)));
			return NULL;
		}
	}

	return result;
}

/**
 * @brief	Parse the parameter specified to an SMTP AUTH PLAIN or SMTP AUTH LOGIN command.
 * @note	This function will stop reading input when an invalid base64-encoding character is encountered.
 * @param	con		the SMTP client connection issuing the AUTH command.
 * @return	a managed string containing the domain specified by the AUTH command, or NULL on failure.
 */
stringer_t * smtp_parse_auth(stringer_t *data) {

	size_t length;
	stringer_t *result;
	uchr_t *holder, *input;

	if (st_empty_out(data,&input, &length)) {
		log_pedantic("Invalid data was passed in for parsing.");
		return NULL;
	}

	// Skip leading spaces.
	while(length && *input == ' ') {
		input++;
		length--;
	}

	// Make sure something is still there.
	if (!length) {
		log_pedantic("Did not find any valid characters in the AUTH argument.");
		return NULL;
	}

	holder = input;

	// This will break when it hits the first non-base64 character.
	while (length && ((*holder >= 'A' && *holder <= 'Z') || (*holder >= 'a' && *holder <= 'z') ||
		(*holder >= '0' && *holder <= '9') || *holder == '+' || *holder == '/' || *holder == '=')) {
		holder++;
		length--;
	}

	// Make sure we still have something to process.
	if (holder == input) {
		return NULL;
	}

	if (!(result = st_import(input, holder - input))) {
		log_pedantic("Could not import the AUTH argument into a stringer.");
	}

	return result;
}

/**
 * Extract the provided path from the command line
 * Reverse-path = Path
 * Forward-path = Path
 * Path = "<" [ A-d-l ":" ] Mailbox ">"
 * A-d-l = At-domain *( "," A-d-l )
 * At-domain = @ domain
 *
 * @param con A connection structure which presumably contains a the MAIL FROM value inside the line buffer.
 * @return Returns a stringer with the path that was extracted or NULL to indicate a problem.
 */
stringer_t * smtp_parse_mail_from_path(connection_t *con) {

	placer_t token;
	stringer_t *result;
	chr_t *input, *start;
	size_t length, tokens;

	if (!con || pl_empty(con->network.line)) {
		log_pedantic("Invalid data was passed in for parsing.");
		return NULL;
	}
	else if (pl_length_get(con->network.line) <= 9) {
		log_pedantic("The MAIL FROM line is too short. {%s = %.*s}", con->command->string, pl_length_int(pl_trim_end(con->network.line)),
				pl_char_get(pl_trim_end(con->network.line)));
		return NULL;
	}

	// This is a MAIL FROM so we skip the first ten characters.
	input = pl_char_get(con->network.line) + 9;
	length = pl_length_get(con->network.line) - 9;

	// The MAIL FROM should be immediately followed by a colon. Then possibly spaces.
	while (length && (*input == ':' || *input == ' ')) {
		input++;
		length--;
	}

	// Make sure something is still there.
	if (!length) {
		log_pedantic("Did not find any valid characters in the MAIL FROM argument. {%s = %.*s}", con->command->string, pl_length_int(pl_trim_end(con->network.line)),
				pl_char_get(pl_trim_end(con->network.line)));
		return NULL;
	}

	// Look for the RFC defined null sender <>, Otherwise skip the brackets too.
	if (length >= 2 && !st_cmp_cs_eq(PLACER(input, 2), PLACER("<>", 2))) {
		result = st_import("<>", 2);
		input += 2;
		length -= 2;
	}
   // If there isn't a NULL sender, parse out the address.
	else {

		while (length && (*input == '<')) {
			input++;
			length--;
		}

		// Make sure something is still there.
		if (!length) {
			log_pedantic("Did not find any valid characters in the MAIL FROM argument. {%s = %.*s}", con->command->string, pl_length_int(pl_trim_end(con->network.line)),
				pl_char_get(pl_trim_end(con->network.line)));
			return NULL;
		}

		// Record the starting position.
		start = input;

		// Get all of the valid characters.
		// See RFC 5321 and 5621 for rules on what characters make up a valid email addresses.
		while (length && ((*input >= 'A' && *input <= 'Z') || (*input >= 'a' &&
			*input <= 'z') || (*input >= '0' && *input <= '9') || *input == '-' ||
			*input == '_' || *input == '.' || *input == '?' || *input == '@' ||
			*input == '+' || *input == '#' || *input == '=' || *input == '%' ||
			*input == '&' || *input == '$' || *input == '!' || *input == '\'')) {
			*input = lower_chr(*input);
			input++;
			length--;
		}

		// Make sure we still have something to process.
		if (input == start) {
			log_pedantic("Did not find any valid characters in the MAIL FROM argument. {%s = %.*s}", con->command->string, pl_length_int(pl_trim_end(con->network.line)),
				pl_char_get(pl_trim_end(con->network.line)));
			return NULL;
		}

		// Import the MAIL FROM argument into a stringer. Take into account the max length.
		if (input - start > magma.smtp.address_length_limit) result = st_import(start, magma.smtp.address_length_limit);
		else result = st_import(start, input - start);

		if (!result) {
			log_pedantic("Could not import the MAIL FROM argument into a stringer. {%s = %.*s}", con->command->string, pl_length_int(pl_trim_end(con->network.line)),
				pl_char_get(pl_trim_end(con->network.line)));
			return NULL;
		}
	}

	// Look for valid MAIL parameters. Currently we support the SIZE and BODY parameters.
	// First we need to skip the closing bracket, and any interim spaces.
	while (length && (*input == '>' || *input == ' ')) {
		input++;
		length--;
	}

	// Nothing left.
	if (!length) return result;

	tokens = tok_get_count_bl(input, length, ' ');
	for (size_t i = 0; i < tokens; i++) {

		if (tok_get_bl(input, length, ' ', i, &token) >= 0 && !pl_empty(token = pl_trim(token)) && pl_length_get(token) > 5) {

			// The BODY parameter provided by RFCs 1426 and 1652.
			if (!st_cmp_ci_starts(&token, CONSTANT("BODY=")) && tok_get_pl(token, '=', 1, &token) >= 0) {
				if (!st_cmp_ci_starts(&token, CONSTANT("7BIT"))) con->smtp.suggested_eight_bit = false;
				else if (!st_cmp_ci_starts(&token, CONSTANT("8BITMIME"))) con->smtp.suggested_eight_bit = true;
#ifdef MAGMA_PEDANTIC
				else {
					tok_get_bl(input, length, ' ', i, &token);
					log_pedantic("Unrecognized BODY parameter provided with the MAIL FROM. {%s PARAM = %.*s}", con->command->string, pl_length_int(pl_trim_end(con->network.line)),
							pl_char_get(pl_trim_end(con->network.line)));
				}
#endif
			}
			// The SIZE parameter provided by RFCs 1870.
			else if (!st_cmp_ci_starts(&token, CONSTANT("SIZE=")) && tok_get_pl(token, '=', 1, &token) >= 0) {
				if (!uint64_conv_pl(token, &(con->smtp.suggested_length))) {
					tok_get_bl(input, length, ' ', i, &token);
					log_pedantic("Invalid SIZE parameter, so we're ignoring it. {%s PARAM = %.*s}", con->command->string, pl_length_int(token), pl_char_get(token));
					con->smtp.suggested_length = 0;
				}
			}
#ifdef MAGMA_PEDANTIC
			else {
				tok_get_bl(input, length, ' ', i, &token);
				log_pedantic("Unrecognized MAIL FROM parameter. {%s PARAM = %.*s}", con->command->string, pl_length_int(token),	pl_char_get(token));
			}
#endif
		}
	}

	return result;
}

/**
 * @brief	Parse the domain specified as the parameter to an SMTP HELO or EHLO command.
 * @note	Valid domain names may only contain letters, numbers, periods and hyphens.
 *			This function will return a lowercase domain name, and stop reading input when an invalid character is encountered.
 * @param	con		the SMTP client connection issuing the command.
 * @return	a managed string containing the domain specified by the HELO command, or NULL on failure.
 */
stringer_t * smtp_parse_helo_domain(connection_t *con) {

	size_t length;
	stringer_t *result;
	chr_t *input, *start;

	if (!con || pl_empty(con->network.line)) {
		log_pedantic("Invalid data was passed in for parsing.");
		return NULL;
	}
	else if (pl_length_get(con->network.line) <= 4) {
		log_pedantic("The HELO line is too short. {%s = %.*s}", con->command->string, pl_length_int(pl_trim_end(con->network.line)),
				pl_char_get(pl_trim_end(con->network.line)));
		return NULL;
	}

	// This is a HELO or EHLO so we skip the first four characters.
	input = pl_char_get(con->network.line) + 4;
	length = pl_length_get(con->network.line) - 4;

	// Skip leading spaces, or < characters.
	while(length && (*input == ' ' || *input == '<' || *input == ':' || *input == '[')) {
		input++;
		length--;
	}

	// Make sure something is still there.
	if (!length) {
		log_pedantic("Did not find any valid characters in the HELO argument. {%s = %.*s}", con->command->string, pl_length_int(pl_trim_end(con->network.line)),
				pl_char_get(pl_trim_end(con->network.line)));
		return NULL;
	}

	start = input;

	// Cleanup the input. The rules say a domain name can only contain letters, numbers, periods and hyphens.
	// We also lower case the string.
	while (length && ((*input >= 'A' && *input <= 'Z') || (*input >= 'a' && *input <= 'z') ||
		(*input >= '0' && *input <= '9') || *input == '-' || *input == '.')) {
		*input = lower_chr(*input);
		input++;
		length--;
	}

	// Make sure we still have something to process.
	if (start == input) {
		log_pedantic("Did not find any valid characters in the HELO argument. {%s = %.*s}", con->command->string,  pl_length_int(pl_trim_end(con->network.line)),
				pl_char_get(pl_trim_end(con->network.line)));
		return NULL;
	}

	// Import the HELO argument into a stringer. Take into account the max length.
	if ((input - start) > magma.smtp.helo_length_limit) {
		result = st_import(start, magma.smtp.helo_length_limit);
	}
	else {
		result = st_import(start, input - start);
	}

	if (!result) {
		log_pedantic("Unable to allocate space for HELO command. {%s = %.*s}", con->command->string,  pl_length_int(pl_trim_end(con->network.line)),
			pl_char_get(pl_trim_end(con->network.line)));
	}

	return result;
}

