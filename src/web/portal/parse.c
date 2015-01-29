
/**
 * @file /magma/web/portal/parse.c
 *
 * @brief	json-rpc request parameter parsers for the portal.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Parse a json string array into a linked list of managed strings.
 * @param	json	a json object containing an array of strings.
 * @param	nout	if not NULL, an optional pointer to a size_t to receive the item count of the json string array.
 * @return	NULL on failure, or a pointer to an inx holder containing the specified json array contents as a collection of managed strings.
 */
inx_t * portal_parse_json_str_array (json_t *json, size_t *nout) {

	inx_t *result;
	stringer_t *istr;
	const chr_t *str;
	size_t count;
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = 0 };

	if (!json) {
		log_pedantic("Portal cannot parse null json array.");
		return NULL;
	// The json object must be an array.
	} else if (!json_is_array(json)) {
		log_pedantic("Portal cannot parse mistyped json array.");
		return NULL;
	} else if (!(result = inx_alloc(M_INX_LINKED, st_free))) {
		log_error("Portal could not allocate space to parse json array.");
		return NULL;
	}

	// If it's an empty array, return right away.
	if (!(count = json_array_size_d(json))) {

		if (nout) {
			*nout = 0;
		}

		return result;
	}

	// Before starting a SQL transaction check that all of the array values are positive integer values.
	for (size_t i = 0; i < count; i++) {

		if (!json_is_string(json_array_get_d(json, i))) {
			log_pedantic("Portal cannot parse json string array with non-string element.");
			inx_free(result);
			return NULL;
		}

		if (!(str = json_string_value_d(json_array_get_d(json, i)))) {
			log_pedantic("Portal encountered json string array with NULL element.");
			inx_free(result);
			return NULL;
		}

		if (!(istr = st_import(str, ns_length_get(str)))) {
			log_error("Portal could not import string from json array.");
			inx_free(result);
			return NULL;
		}

		key.val.u64 = i+1;

		if (!inx_insert(result, key, istr)) {
			log_error("Portal could not prepare data from json array.");
			inx_free(result);
			return NULL;
		}

	}

	if (nout) {
		*nout = count;
	}

	return result;
}

/**
 * @brief	Parse the context of a requested folder.
 * @note	If no context is specified, the "mail" context is assumed (PORTAL_ENDPOINT_CONTEXT_MAIL).
 * @param	context		a managed string containing the context (supported values are "mail", "contacts", "settings", and "help").
 * @return	PORTAL_ENDPOINT_CONTEXT_INVALID on failure, or the flag of the determined context on success.
 */
int_t portal_parse_context(stringer_t *context) {

	int_t result = PORTAL_ENDPOINT_CONTEXT_MAIL;

	if (!context) {
		return result;
	}

	// Parse the context.
	if (!st_cmp_ci_eq(context, PLACER("mail", 4))) {
		result = PORTAL_ENDPOINT_CONTEXT_MAIL;
	}
	else if (!st_cmp_ci_eq(context, PLACER("contacts", 8))) {
		result = PORTAL_ENDPOINT_CONTEXT_CONTACTS;
	}
	else if (!st_cmp_ci_eq(context, PLACER("settings", 8))) {
		result = PORTAL_ENDPOINT_CONTEXT_SETTINGS;
	}
	else if (!st_cmp_ci_eq(context, PLACER("help", 4))) {
		result = PORTAL_ENDPOINT_CONTEXT_HELP;
	}
	else {
		result = PORTAL_ENDPOINT_CONTEXT_INVALID;
	}

	return result;
}

/**
 * @note	This function is currently not called anywhere in the code and may be subject to removal.
 */
int_t portal_parse_action(stringer_t *action) {

	int_t result = PORTAL_ENDPOINT_ACTION_INVALID;

	if (!action) {
		return result;
	}

	// Parse the action.
	if (!st_cmp_ci_eq(action, PLACER("add", 3))) {
		result = PORTAL_ENDPOINT_ACTION_ADD;
	}
	else if (!st_cmp_ci_eq(action, PLACER("remove", 6))) {
		result = PORTAL_ENDPOINT_ACTION_REMOVE;
	}
	else if (!st_cmp_ci_eq(action, PLACER("replace", 7))) {
		result = PORTAL_ENDPOINT_ACTION_REPLACE;
	}
	else if (!st_cmp_ci_eq(action, PLACER("list", 4))) {
		result = PORTAL_ENDPOINT_ACTION_LIST;
	}
	else {
		result = PORTAL_ENDPOINT_ACTION_INVALID;
	}

	return result;
}

/**
 * @brief	Parse the json-rpc messages.load parameter "section" from an array of strings into a bitmask of section flags.
 * @param	array		a pointer to the json object representing the "section" string array of the json request message.
 * @param	sections	a pointer to an unsigned 32 bit integer that will receive the translated section flags on success.
 * @return	< 0 on error (-3 for an internal server error, -2 if an unknown flag was received, or -1 on syntax error),
 * 			0 for an empty sections array, or 1 on success.
 */
// QUESTION: Should -3 be returned for !array, etc. ?
int_t portal_parse_sections(json_t *array, uint32_t *sections) {

	uint64_t count;
	chr_t *current;
	int_t result = 1;

	if (!array || !json_is_array(array) || !sections) {
		return -1;
	}

	// Clear the output just in case something goes wrong.
	*sections = 0;

	if (!(count = json_array_size_d(array))) {
		return 0;
	}

	for (uint64_t i = 0; i < count && result == 1; i++) {

		if (!json_is_string(json_array_get_d(array, i)) || !(current = (chr_t *)json_string_value_d(json_array_get_d(array, i)))) {
			return -1;
		}

		if (!st_cmp_ci_eq(NULLER(current), PLACER("meta", 4))) *sections |= PORTAL_ENDPOINT_MESSAGE_META;
		else if (!st_cmp_ci_eq(NULLER(current), PLACER("source", 6))) *sections |= PORTAL_ENDPOINT_MESSAGE_SOURCE;
		else if (!st_cmp_ci_eq(NULLER(current), PLACER("security", 8))) *sections |= PORTAL_ENDPOINT_MESSAGE_SECURITY;
		else if (!st_cmp_ci_eq(NULLER(current), PLACER("server", 6))) *sections |= PORTAL_ENDPOINT_MESSAGE_SERVER;
		else if (!st_cmp_ci_eq(NULLER(current), PLACER("header", 6))) *sections |= PORTAL_ENDPOINT_MESSAGE_HEADER;
		else if (!st_cmp_ci_eq(NULLER(current), PLACER("body", 4))) *sections |= PORTAL_ENDPOINT_MESSAGE_BODY;
		else if (!st_cmp_ci_eq(NULLER(current), PLACER("attachments", 11))) *sections |= PORTAL_ENDPOINT_MESSAGE_ATTACHMENTS;
		else if (!st_cmp_ci_eq(NULLER(current), PLACER("info",4))) *sections |= PORTAL_ENDPOINT_MESSAGE_INFO;
		else if (result == 1) result = -2;
	}

	return result;
}
