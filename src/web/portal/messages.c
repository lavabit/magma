
/**
 * @file /magma/web/portal/messages.c
 *
 * @brief	Functions to help assemble mail message for output.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/***
 * Build the meta response section, or return NULL if an error occurs.
 */
json_t * portal_message_meta(meta_message_t *meta) {

	json_t *section;
	json_error_t err;

	if (!(section = json_pack_ex_d(&err, JSON_ENSURE_ASCII, "{s:I, s:I, s:o, s:o}", "messageID", meta->messagenum,
		"folderID", meta->foldernum, "flags", portal_message_flags_array(meta), "tags", portal_message_tags_array(meta)))) {
		log_pedantic("Unable to generate the meta section. {%s}", err.text);
		return NULL;
	}

	return section;
}

json_t * portal_message_source(meta_message_t *meta) {

	json_t *section;
	json_error_t err;

	/// TODO: Replace hard coded values with actual data.
	if (!(section = json_pack_ex_d(&err, JSON_ENSURE_ASCII, "{s:s, s:s, s:s}", "ip", "127.0.0.1",
		"dns", "magma.lavabit.com", "reputation", rand_get_int64() % 2 ? "Good" : rand_get_int64() % 2 ? "Neutral" : "Malicious"))) {
		log_pedantic("Unable to generate the source section. {%s}", err.text);
		return NULL;
	}

	return section;
}

json_t * portal_message_security(meta_message_t *meta) {

	json_t *section;
	json_error_t err;

	/// TODO: Replace hard coded values with actual data.
	if (!(section = json_pack_ex_d(&err, JSON_ENSURE_ASCII, "{s:b, s:b, s:b}", "secure", false,	"spf", false, "dkim", false))) {
		log_pedantic("Unable to generate the security section. {%s}", err.text);
		return NULL;
	}

	return section;
}

json_t * portal_message_server(meta_message_t *meta) {

	json_t *section;
	json_error_t err;

	/// TODO: Replace hard coded values with actual data.
	if (!(section = json_pack_ex_d(&err, JSON_ENSURE_ASCII, "{s:I, s:b, s:s}", "utc", meta->created, "images", false, "warnings",
		"This message is spoofed! The signature it carries does not validate!"))) {
		log_pedantic("Unable to generate the server section. {%s}", err.text);
		return NULL;
	}

	return section;
}


/**
 * @brief	Build the "header" section response to a rpc-json "messages.load" request.
 * @note	The following header fields will be returned: "To, CC, BCC, From, Replyto, Sender, Return-Path, Subject, Date, and Size.
 * @param	meta	a pointer to the meta message object of the requested message.
 * @param	data	a pointer to the mail message object containing the requested message's data.
 * @return	a pointer to a json object containing the header fields of the requested message.
 */
json_t * portal_message_header(meta_message_t *meta, mail_message_t *data) {

	json_t *section;
	json_error_t err;
	stringer_t *headers[9];

	mm_wipe(headers, sizeof(headers));

	if (!(section = json_pack_ex_d(&err, JSON_ENSURE_ASCII, "{s:S, s:S, s:S, s:S, s:S, s:S, s:S, s:S, s:S, s:I}",
		"to", st_char_get(headers[0] = mail_header_fetch_cleaned(PLACER(data->text, data->header_length), PLACER("To", 2))),
		"cc", st_char_get(headers[1] = mail_header_fetch_cleaned(PLACER(data->text, data->header_length), PLACER("Cc", 2))),
		"bcc", st_char_get(headers[2] = mail_header_fetch_cleaned(PLACER(data->text, data->header_length), PLACER("Bcc", 3))),
		"from", st_char_get(headers[3] = mail_header_fetch_cleaned(PLACER(data->text, data->header_length), PLACER("From", 4))),
		"replyto", st_char_get(headers[4] = mail_header_fetch_cleaned(PLACER(data->text, data->header_length), PLACER("Reply-To", 8))),
		"sender", st_char_get(headers[5] = mail_header_fetch_cleaned(PLACER(data->text, data->header_length), PLACER("Sender", 6))),
		"return-path", st_char_get(headers[6] = mail_header_fetch_cleaned(PLACER(data->text, data->header_length), PLACER("Return-Path", 11))),
		"subject", st_char_get(headers[7] = mail_header_fetch_cleaned(PLACER(data->text, data->header_length), PLACER("Subject", 7))),
		"date", st_char_get(headers[8] = mail_header_fetch_cleaned(PLACER(data->text, data->header_length), PLACER("Date", 4))),
		"size", meta->size))) {
		log_pedantic("Unable to generate the header section. {%s}", err.text);
	}

	for(size_t i = 0; i < (sizeof(headers) / sizeof(stringer_t *)); i++) {
		st_cleanup(headers[i]);
	}

	return section;
}

json_t * portal_message_body(meta_message_t *meta, mail_message_t *data) {

	json_t *section;
	json_error_t err;
	stringer_t *body = NULL, *type = NULL;
	mail_mime_t *active = NULL, *quarry = NULL;

	if (!data || !(active = data->mime)) {
		log_pedantic("Passed an invalid message context.");
		return NULL;
	}

	for (size_t i = 0; !quarry && i < MAIL_MIME_RECURSION_LIMIT; i++) {

		// Keep diving through multipart messages until we find our quary.
		if (active && active->children && (active->type == MESSAGE_TYPE_MULTI_RELATED || active->type == MESSAGE_TYPE_MULTI_MIXED ||
			active->type == MESSAGE_TYPE_HTML || active->type == MESSAGE_TYPE_MULTI_ALTERNATIVE || active->type == MESSAGE_TYPE_MULTI_UNKOWN)) {
			active = ar_field_ptr(active->children, 0);
		}

		if (active && (active->type == MESSAGE_TYPE_HTML || active->type == MESSAGE_TYPE_PLAIN)) {
			quarry = active;
		}
	}

	// While the logic above needs work, the goal is to eventually discover message content that is either HTML or plain text.
	if (active && active->type == MESSAGE_TYPE_HTML) {
		type = st_dupe_opts(NULLER_T | CONTIGUOUS | HEAP, PLACER("html", 4));
		//body = st_dupe_opts(NULLER_T | CONTIGUOUS | HEAP, &(active->body));
		body = st_nullify(pl_char_get(active->body), pl_length_get(active->body));
	}
	else if (active && active->type == MESSAGE_TYPE_PLAIN) {
		type = st_dupe_opts(NULLER_T | CONTIGUOUS | HEAP, PLACER("text", 4));
		//body = st_dupe_opts(NULLER_T | CONTIGUOUS | HEAP, &(active->body));
		body = st_nullify(pl_char_get(active->body), pl_length_get(active->body));
	}

	/// TODO: I consider it a miracle whenever the above logic actually manages to select the message content. But if it doesn't we fall through to this
	/// fixed error message, at least until we can improve the selection process.
	else {
		type = st_dupe_opts(NULLER_T | CONTIGUOUS | HEAP, PLACER("text", 4));
		body = st_dupe_opts(NULLER_T | CONTIGUOUS | HEAP, PLACER("Unable to parse the message content.", 36));
	}

	/// HIGH: Because JSON wants NULLERS, and were using PLACEHOLDERS, its printing out the entire message, not just the section of interest.
	// Note that we don't return here because doing so leaves open the possibility that the type or body variables will end up leaking memory.
	if (!(section = json_pack_ex_d(&err, 0, "{s:S}", st_char_get(type), st_char_get(body)))) {
		log_pedantic("Unable to generate the body section. {%s}", err.text);
	}

	st_cleanup(type);
	st_cleanup(body);

	return section;
}

json_t * portal_message_attachments(meta_message_t *meta, mail_message_t *data) {

	size_t count;
	json_error_t err;
	mail_mime_t *active;
	json_t *array, *attachment;
	stringer_t *group, *subtype, *type, *name;

	/// LOW: We also need to detect messages that have no readable content so the entire body is just a blob.
	if (!(array = json_array_d()) || !data || !data->mime) {
		return array;
	}

	if (data->mime->type == MESSAGE_TYPE_MULTI_MIXED && (count = ar_length_get(data->mime->children))) {

		// Start at one so we skip the first MIME part. Presumably because the first entry is the display data.
		for (size_t i = 1; i < count; i++) {

			if ((active = ar_field_ptr(data->mime->children, i)) && (i != 0 || (active->type != MESSAGE_TYPE_HTML && active->type != MESSAGE_TYPE_PLAIN))) {

				/// Create a function to search for the filename. Common locations are:
				/// Content-Type: image/png; name="webmail-php-download.png"
				/// Content-Disposition: attachment; filename="webmail-php-download.png"
				name = st_aprint("filename_%zu.dat", i);
				type = st_merge("sss", (group = mail_mime_type_group(active->header)), PLACER("/", 1), (subtype = mail_mime_type_sub(active->header)));

				if (!(attachment = json_pack_ex_d(&err, JSON_ENSURE_ASCII, "{s:I, s:I, s:s, s:s}", "attachmentID", i, "size", st_length_get(&(active->body)), "type",
					st_char_get(type), "name", st_char_get(name)))) {
					log_pedantic("Unable to generate the attachments section. {%s}", err.text);
				}
				else if (json_array_append_new_d(array, attachment)) {
					log_pedantic("Unable to append the attachments object to the array.");
					json_decref_d(attachment);
				}

				st_cleanup(name);
				st_cleanup(type);
				st_cleanup(group);
				st_cleanup(subtype);
			}
		}
	}

	/*
	json_t *section;
	json_error_t err;
	stringer_t *body = NULL, *type = NULL;
	mail_mime_t *active = NULL, *quarry = NULL;

	if (!data || !(active = data->mime)) {
		log_pedantic("Passed an invalid message context.");
		return NULL;
	}

	for (size_t i = 0; !quarry && i < MAIL_MIME_RECURSION_LIMIT; i++) {

		// Keep diving through multipart messages until we find our quarry.
		if (active && active->children && (active->type == MESSAGE_TYPE_MULTI_RELATED || active->type == MESSAGE_TYPE_MULTI_MIXED ||
			active->type == MESSAGE_TYPE_HTML || active->type == MESSAGE_TYPE_MULTI_ALTERNATIVE || active->type == MESSAGE_TYPE_MULTI_UNKOWN)) {
			active = ar_field_ptr(active->children, 0);
		}

		if (active && (active->type == MESSAGE_TYPE_HTML || active->type == MESSAGE_TYPE_PLAIN)) {
			quarry = active;
		}

	}

	// While the logic above needs work, the goal is to eventually discover message content that is either HTML or plain text.
	if (active && active->type == MESSAGE_TYPE_HTML) {
		type = st_dupe_opts(NULLER_T | CONTIGUOUS | HEAP, PLACER("html", 4));
		body = st_dupe_opts(NULLER_T | CONTIGUOUS | HEAP, &(active->body));
	}
	else if (active && active->type == MESSAGE_TYPE_PLAIN) {
		type = st_dupe_opts(NULLER_T | CONTIGUOUS | HEAP, PLACER("text", 4));
		body = st_dupe_opts(NULLER_T | CONTIGUOUS | HEAP, &(active->body));
	}

	/// TODO: I consider it a miracle whenever the above logic actually manages to select the message content. But if it doesn't we fall through to this
	/// fixed error message, at least until we can improve the selection process.
	else {
		type = st_dupe_opts(NULLER_T | CONTIGUOUS | HEAP, PLACER("text", 4));
		body = st_dupe_opts(NULLER_T | CONTIGUOUS | HEAP, PLACER("Unable to parse the message content.", 36));
	}

	/// HIGH: Because json wants NULLERS, and were using PLACEHOLDERS, its printing out the entire message, not just the section of interest.
	if (!(section = json_pack_ex_d(&err, 0, "{s:S}", st_char_get(type), st_char_get(body)))) {
		log_pedantic("Unable to generate the body section. {%s}", err.text);
		return NULL;
	}

	st_cleanup(type);
	st_cleanup(body);

	return section;*/

	return array;
}


/**
 * @brief	Build the "info" section response to a rpc-json "messages.load" request.
 * @param	meta	a pointer to the meta message object of the requested message.
 * @return	a pointer to a json object containing the appropriate information about the requested message.
 */

json_t * portal_message_info(meta_message_t *meta) {

	json_t *section;
	json_error_t err;
/*	if (!(section = json_pack_ex_d(&err, JSON_ENSURE_ASCII, "{s:{s:s, s:s, s:s}, s:{s:b, s:b, s:b}, s:{s:I, s:b, s:s}}", "source", "ip", "1.2.3.4",
		"dns", "dns.lavabit.com", "reputation", "Good", "security", "secure", 1, "spf", 1, "dkim", 0, "server", "utc", 1003029300349, "images", 1,
		"warnings", ""))) { */


	//if (!(section = json_pack_ex_d(&err, JSON_ENSURE_ASCII, "{s:{s:s, s:s, s:s}}", "source", "ip", "1.2.3.4", "dns", "dns.lavabit.com", "reputation", "Good"))) {
	//if (!(section = json_pack_ex_d(&err, JSON_ENSURE_ASCII, "{s:{s:s, s:s, s:s}, s:s}", "source", "ip", "1.2.3.4", "dns", "dns.lavabit.com", "reputation", "Good",
	/*if (!(section = json_pack_ex_d(&err, JSON_ENSURE_ASCII, "{s:{s:s, s:s, s:s}, s:{s:b, s:b, s:b}, s:s}", "source", "ip", "1.2.3.4", "dns", "dns.lavabit.com", "reputation", "Good",
		"security", "secure", true, "spf", true, "dkim", false, "server", "utc"))) { */
	if (!(section = json_pack_ex_d(&err, JSON_ENSURE_ASCII, "{s:{s:s, s:s, s:s}, s:{s:b, s:b, s:b}, s:{s:I, s:b, s:s}}", "source", "ip", "1.2.3.4", "dns", "dns.lavabit.com", "reputation", "Good",
			"security", "secure", true, "spf", true, "dkim", false, "server", "utc", 1003029300349, "images", true, "warnings", "This message is spoofed! The signature it carries does not validate!"))) {
		log_pedantic("Unable to generate the info section. {%s}", err.text);
	return NULL;
	}

	return section;
}
