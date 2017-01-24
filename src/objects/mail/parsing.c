
/**
 * @file /magma/objects/mail/parsing.c
 *
 * @brief	Functions used to parse mail messages and extract information from them.
 */

#include "magma.h"

/**
 * @brief	Get the domain portion of an email address.
 * @param	address		a managed string containing the email address to be parsed.
 * @param	output		a pointer to a placer to receive the domain portion of the email address.
 * @return 	NULL on failure or a pointer to the output of the domain extraction on success.
 */
placer_t * mail_domain_get(stringer_t *address, placer_t *output) {

	if (tok_get_count_st(address, '@') < 2 ||	tok_get_st(address, '@', 1, output) != 1) {
		return NULL;
	}

	return output;
}

/**
 * @brief	Extract a valid email address from a From: header value.
 * @note	The preference of this function is first to try to find the email addressed enclosed in <>.
 * 			No valid email address may be enclosed in () or "".
 * @param	address		a managed string containing the buffer from which the email address will be extracted.
 * @return	NULL on failure or a managed string containing the email address on success.
 */
stringer_t * mail_extract_address(stringer_t *address) {

	stringer_t *result;
	chr_t *new, *string;
	size_t length, used = 0, increment;
	int_t bracket = 0, comment = 0, quote = 0, modern = 1;

	if (st_empty_out(address, (void *)&string, &length)) {
		log_pedantic("Was not passed a valid address.");
		return NULL;
	}

	for (increment = 0; increment < length; increment++) {

		if (bracket == 0 && *string == '<') {
			bracket = 1;
		}
		else if (bracket == 1 && *string == '>') {
			bracket = 0;
		}
		else if (quote == 0 && *string == '"') {
			quote = 1;
		}
		else if (quote == 1 && *string == '"') {
			quote = 0;
		}
		else if (comment == 0 && *string == '(') {
			comment = 1;
		}
		else if (comment == 1 && *string == ')') {
			comment = 0;
		}

		if (bracket == 1 && quote == 0 && comment == 0 && ((*string >= 'A' && *string <= 'Z') || (*string >= 'a' && \
			*string <= 'z') || (*string >= '0' && *string <= '9') || *string == '-' || \
			*string == '_' || *string == '.' || *string == '?' || *string == '@' || \
			*string == '+')) {
			used++;
		}

		string++;
	}

	if (used == 0) {
		modern = 0;
		string = st_data_get(address);
		bracket = comment = quote = 0;

		for (increment = 0; increment < length; increment++) {

			if (quote == 0 && *string == '"') {
				quote = 1;
			}
			else if (quote == 1 && *string == '"') {
				quote = 0;
			}
			else if (comment == 0 && *string == '(') {
				comment = 1;
			}
			else if (comment == 1 && *string == ')') {
				comment = 0;
			}

			if (quote == 0 && comment == 0 && ((*string >= 'A' && *string <= 'Z') || (*string >= 'a' && \
				*string <= 'z') || (*string >= '0' && *string <= '9') || *string == '-' || \
				*string == '_' || *string == '.' || *string == '?' || *string == '@' || \
				*string == '+')) {
				used++;
			}

			string++;
		}

		if (used == 0) {
			log_pedantic("Could not find any characters to include as part of the from address.");
			return NULL;
		}

	}

	if (!(result = st_alloc(used))) {
		log_pedantic("Could not allocate %zu bytes to hold the parsed address.", used);
		return NULL;
	}

	// Reset.
	string = st_data_get(address);
	new = st_char_get(result);
	bracket = comment = quote = 0;

	for (increment = 0; increment < length; increment++) {

		if (bracket == 0 && *string == '<') {
			bracket = 1;
		}
		else if (bracket == 1 && *string == '>') {
			bracket = 0;
		}
		else if (quote == 0 && *string == '"') {
			quote = 1;
		}
		else if (quote == 1 && *string == '"') {
			quote = 0;
		}
		else if (comment == 0 && *string == '(') {
			comment = 1;
		}
		else if (comment == 1 && *string == ')') {
			comment = 0;
		}

		if (modern == 1 && bracket == 1 && quote == 0 && comment == 0 && ((*string >= 'A' && *string <= 'Z') || (*string >= 'a' && \
			*string <= 'z') || (*string >= '0' && *string <= '9') || *string == '-' || \
			*string == '_' || *string == '.' || *string == '?' || *string == '@' || \
			*string == '+')) {
			*new++ = *string;
		}
		else if (modern == 0 && quote == 0 && comment == 0 && ((*string >= 'A' && *string <= 'Z') || (*string >= 'a' && \
			*string <= 'z') || (*string >= '0' && *string <= '9') || *string == '-' || \
			*string == '_' || *string == '.' || *string == '?' || *string == '@' || \
			*string == '+')) {
			*new++ = *string;
		}

		string++;
	}

	st_length_set(result, used);

	return result;
}
