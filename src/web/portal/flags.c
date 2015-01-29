
/**
 * @file /magma/web/portal/flags.c
 *
 * @brief Functions for handling message flags and tags.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/// TODO: The messages.load method uses the flags/tags helper functions, but the messages.list and the messages.tags/flags methods
/// still need to be updated.

/**
 * @brief	Return a json array of flag descriptions for a mail message's flags bitmask.
 * @param	meta	a pointer to the meta message object of the mail message to have its flags parsed.
 * @return	NULL on failure, or a pointer to the json object of the specified mail message's flags.
 */
json_t * portal_message_flags_array(meta_message_t *meta) {

	json_t *flags;

	// If the status is variable is zero or set to MAIL_STATUS_EMPTY, skip the individual checks.
	if ((flags = json_array_d()) && meta && (meta->status | MAIL_STATUS_EMPTY) != MAIL_STATUS_EMPTY) {
		if (meta->status & MAIL_STATUS_RECENT) json_array_append_new_d(flags, json_string_d("recent"));
		if (meta->status & MAIL_STATUS_SEEN) json_array_append_new_d(flags, json_string_d("seen"));
		if (meta->status & MAIL_STATUS_ANSWERED) json_array_append_new_d(flags, json_string_d("answered"));
		if (meta->status & MAIL_STATUS_FLAGGED) json_array_append_new_d(flags, json_string_d("flagged"));
		if (meta->status & MAIL_STATUS_DELETED) json_array_append_new_d(flags, json_string_d("deleted"));
		if (meta->status & MAIL_STATUS_DRAFT) json_array_append_new_d(flags, json_string_d("draft"));
		if (meta->status & MAIL_STATUS_APPENDED) json_array_append_new_d(flags, json_string_d("appended"));
		if (meta->status & MAIL_MARK_JUNK) json_array_append_new_d(flags, json_string_d("junk"));
		if (meta->status & MAIL_MARK_INFECTED) json_array_append_new_d(flags, json_string_d("infected"));
		if (meta->status & MAIL_MARK_SPOOFED) json_array_append_new_d(flags, json_string_d("spoofed"));
		if (meta->status & MAIL_MARK_BLACKHOLED) json_array_append_new_d(flags, json_string_d("blackholed"));
		if (meta->status & MAIL_MARK_PHISHING) json_array_append_new_d(flags, json_string_d("phishing"));
		if (meta->status & MAIL_STATUS_ENCRYPTED) json_array_append_new_d(flags, json_string_d("encrypted"));
	}

	return flags;
}

/***
 * Returns 1 if the array of flags checks out. Returns 0 if the array was empty. Returns -1 if there is a syntax/structure
 * problem. Returns -2 if an unrecognized flag was specified in the array.
 */
int_t portal_parse_flags(json_t *array, uint32_t *flags) {

	uint64_t count;
	chr_t *current;
	int_t result = 1;

	if (!array || !json_is_array(array) || !flags) {
		return -1;
	}

	*flags = 0;

	if (!(count = json_array_size_d(array))) {
		return 0;
	}

	for (uint64_t i = 0; i < count && result == 1; i++) {

		if (!json_is_string(json_array_get_d(array, i)) || !(current = (chr_t *)json_string_value_d(json_array_get_d(array, i)))) {
			return -1;
		}

		if (!st_cmp_ci_eq(NULLER(current), PLACER("recent", 6))) *flags |= MAIL_STATUS_RECENT;
		else if (!st_cmp_ci_eq(NULLER(current), PLACER("seen", 4))) *flags |= MAIL_STATUS_SEEN;
		else if (!st_cmp_ci_eq(NULLER(current), PLACER("answered", 8))) *flags |= MAIL_STATUS_ANSWERED;
		else if (!st_cmp_ci_eq(NULLER(current), PLACER("flagged", 7))) *flags |= MAIL_STATUS_FLAGGED;
		else if (!st_cmp_ci_eq(NULLER(current), PLACER("deleted", 7))) *flags |= MAIL_STATUS_DELETED;
		else if (!st_cmp_ci_eq(NULLER(current), PLACER("draft", 5))) *flags |= MAIL_STATUS_DRAFT;
		else if (!st_cmp_ci_eq(NULLER(current), PLACER("appended", 8))) *flags |= MAIL_STATUS_APPENDED;
		else if (!st_cmp_ci_eq(NULLER(current), PLACER("junk", 4))) *flags |= MAIL_MARK_JUNK;
		else if (!st_cmp_ci_eq(NULLER(current), PLACER("infected", 8))) *flags |= MAIL_MARK_INFECTED;
		else if (!st_cmp_ci_eq(NULLER(current), PLACER("spoofed", 7))) *flags |= MAIL_MARK_SPOOFED;
		else if (!st_cmp_ci_eq(NULLER(current), PLACER("blackholed", 10))) *flags |= MAIL_MARK_BLACKHOLED;
		else if (!st_cmp_ci_eq(NULLER(current), PLACER("phishing", 8))) *flags |= MAIL_MARK_PHISHING;
		else if (result == 1) result = -2;
	}

	return result;
}

json_t * portal_message_tags_array(meta_message_t *meta) {

	json_t *tags;
	size_t count;

	if ((tags = json_array_d()) && meta && meta->tags && (count = ar_length_get(meta->tags))) {
		for (uint64_t i = 0; i < count; i++) {
			json_array_append_new_d(tags, json_string_d(st_char_get(ar_field_st(meta->tags, i))));
		}
	}

	return tags;
}
