
/**
 * @file /magma/src/objects/auth/username.c
 *
 * @brief Functions used to parse and pre-process usernames.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Process an email address or username to ensure it only contains valid characters.
 * @param	username	a managed string containing the user supplied login name to be processed, it may contain an optional domain suffix.
 * @note	This function duplicates the input address string, with all characters converted to lowercase, and whitespace removed.
 * 			'.' and '-' are also converted to '_' in the username, and if there is a '+' in the local portion of an email
 * 			 address, all subsequent characters in that local part will be ignored, as they constitute a label.
 * @return	NULL on failure, or a managed string containing the sanitized address on success.
 */
stringer_t * auth_username_sanitizer(stringer_t *username) {

	size_t len;
	chr_t *p, *w, *tip = NULL;
	stringer_t *output, *handle, *domain = NULL;

	if (st_empty_out(username, (uchr_t **)&p, &len) || !(output = st_alloc(len + 1))) {
		return NULL;
	}

	w = st_char_get(output);

	for (size_t i = 0; i < len; i++) {

		// If an @ symbol is encountered, record its position
		if (!tip && *p == '@') {
			tip = w;
		}

		// Non white space characters are copied in lower case form
		if (!chr_whitespace(*p)) {
			*w++ = lower_chr(*p++);
		}

		// Just advance past white space
		else {
			p++;
		}

	}

	st_length_set(output, w - st_char_get(output));

	// If an @ symbol was encountered, save the handle and domain portions separately since they use different rules.
	if (tip) {
		handle = PLACER(st_char_get(output), tip - st_char_get(output));
		domain = PLACER(tip, st_length_get(output) - st_length_get(handle));
	}
	else {
		handle = PLACER(st_char_get(output), st_length_get(output));
	}

	p = st_char_get(handle);
	len = st_length_get(handle);
	tip = NULL;

	for (size_t i = 0; !tip && i < len; i++) {

		// Save the location of a plus sign (+) so it can be be stripped from the handle section.
		if (*p == '+') {
			tip = p;
		}
		// Translate periods and dashes to underscores allowing them to be used interchanged if the user desires.
		else if (*p == '.' || *p == '-') {
			*p++ = '_';
		}
		else {
			p++;
		}

	}

	// If a plus sign was found in the handle, trim the handle and if necessary shift the domain portion to accommodate.
	if (tip && domain) {
		w = st_char_get(domain);
		len = st_length_get(domain);

		for (size_t i = 0; i < len; i++) {
			*p++ = *w++;
		}

		st_length_set(output, (size_t)(p - st_char_get(output)));
	}
	else if (tip) {
		st_length_set(output, (size_t)(p - st_char_get(output)));
	}

	return output;
}

/**
 * @brief	Get the username portion of a fully qualified email address.
 * @param	username	a managed string containing the username to be processed, with an optional domain suffix.
 * @return	NULL on failure, or a managed string containing the credential username portion of the supplied address.
 */
stringer_t * auth_username(stringer_t *username) {

	size_t at;
	stringer_t *output, *domain = NULL;

	if (!(output = auth_username_sanitizer(username))) {
		return NULL;
	}

	// If an @ symbol was encountered, check the domain portion to see if it matches magma.system.domain and if so strip it off.
	if (st_search_cs(output, PLACER("@", 1), &at) && (st_length_get(output) - at) > 1) {
		domain = PLACER(st_char_get(output) + at + 1, st_length_get(output) - at - 1);
		if (!st_cmp_cs_eq(domain, magma.system.domain)) {
			st_length_set(output, at);
		}
	}

	return output;
}
