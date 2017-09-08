
/**
 * @file /magma/objects/mail/load_message.c
 *
 * @brief	Functions used to load mail messages.
 */

#include "magma.h"

/**
 * @brief	Load a stored mail message from disk.
 * @note	The mail message will always, at the very least, be compressed using the lzo algorithm; however, on-disk encryption may be enabled.
 			If parsing is enabled, a spam signature training link may be embedded in the message.
 * @param	meta	the meta message object of the message to be loaded from disk.
 * @param	user	the meta user object of the user that owns the requested message.
 * @param	server	the server object of the web server where the spam teacher application is hosted.
 * @param	parse	if true, the header's Subject line is branded with any applicable labels such as JUNK, INFECTED, SPOOFED, BLACKHOLED, PHISHING.
 * @return	NULL on failure or a a mail message object containing the retrieved mail message data on success.
 */
mail_message_t * mail_load_message(meta_message_t *meta, meta_user_t *user, server_t *server, bool_t parse) {

	int_t fd;
	chr_t *path;
	size_t data_len;
	struct stat file_info;
	compress_t *compressed;
	mail_message_t *result;
	message_header_t header;
	stringer_t *raw, *message;

	if (!meta || (parse && (!user || !server))) {
		log_pedantic("Invalid parameter combination passed in.");
		return NULL;
	}

	// Check the thread local message cache first.
	if ((message = mail_cache_get(meta->messagenum))) {

		if (!(result = mail_message(message))) {
			log_pedantic("Unable to build the message structure.");
			return NULL;
		}

		return result;
	}

	if (!(path = mail_message_path(meta->messagenum, meta->server))) {
		log_pedantic("Could not build the message path.");
		return NULL;
	}

	// Open the file.
	if ((fd = open(path, O_RDONLY)) < 0) {
		log_pedantic("Could not open a file descriptor for the message %s.", path);
		mail_db_hide_message(meta->messagenum);
		serial_increment(OBJECT_MESSAGES, user->usernum);
		ns_free(path);
		return NULL;
	}

	// Figure out how big the file is, and allocate memory for it.
	if (fstat(fd, &file_info) != 0) {
		log_pedantic("Could not fstat the file %s.", path);
		close(fd);
		ns_free(path);
		return NULL;
	}

	if (file_info.st_size < sizeof(message_header_t)) {
		log_pedantic("Mail message was missing full file header: { %s }", path);
		close(fd);
		ns_free(path);
		return NULL;
	}

	// Do some sanity checking on the message header
	data_len = file_info.st_size - sizeof(message_header_t);

	if (read(fd, &header, sizeof(header)) != sizeof(header)) {
		log_pedantic("Unable to read message file header: { %s }", path);
		close(fd);
		ns_free(path);
		return NULL;
	}

	if ((header.magic1 != FMESSAGE_MAGIC_1) || (header.magic2 != FMESSAGE_MAGIC_2)) {
		log_pedantic("Mail message had incorrect file format: { %s }", path);
		close(fd);
		ns_free(path);
		return NULL;
	}

	// Allocate a buffer big enough to hold the entire compressed file.
	if (!(raw = st_alloc(data_len))) {
		log_pedantic("Could not allocate a buffer of %li bytes to hold the message.", data_len);
		close(fd);
		ns_free(path);
		return NULL;
	}

	// Read the file in.
	if (read(fd, st_char_get(raw), data_len) != data_len) {
		log_pedantic("Could not read all %li bytes of the file %s.", data_len, path);
		ns_free(path);
		st_free(raw);
		close(fd);
		return NULL;
	}

	// Were done with the file.
	close(fd);

	// Tell the stringer how much data is there.
	st_length_set(raw, data_len);

	if (meta->status & MAIL_STATUS_ENCRYPTED) {

		if (!(header.flags & FMESSAGE_OPT_ENCRYPTED)) {
			log_pedantic("Message state mismatch: encrypted in database but unencrypted on disk. { user = %.*s / number = %lu }",
				st_length_int(user->username), st_char_get(user->username), meta->messagenum);
		}

		if (!(user->flags & META_USER_ENCRYPT_DATA)) {
			log_info("User with secure mode off requested encrypted message. { user = %.*s / number = %lu }",
				st_length_int(user->username), st_char_get(user->username), meta->messagenum);
		}

		if (!user->prime.key) {
			log_pedantic("User cannot read encrypted messages without a private key. { user = %.*s / number = %lu }",
				st_length_int(user->username), st_char_get(user->username), meta->messagenum);
			ns_free(path);
			st_free(raw);
			return NULL;
		}

		else if (!(message = prime_message_decrypt(raw, org_signet, user->prime.key))) {
			log_pedantic("Unable to decrypt mail message. { user = %.*s / number = %lu }",
				st_length_int(user->username), st_char_get(user->username), meta->messagenum);
			ns_free(path);
			st_free(raw);
			return NULL;
		}

		// Free the raw buffer, but keep the path around in case we need it for error messages.
		st_free(raw);
	}
	else if (header.flags & FMESSAGE_OPT_ENCRYPTED) {
		log_pedantic("Message state mismatch, a message marked encrypted in the was found in plain text on disk.");
		ns_free(path);
		st_free(raw);
		return NULL;
	}
	else if (header.flags & FMESSAGE_OPT_COMPRESSED) {

		// Convert the string buffer into a compression buffer.
		if (!(compressed = compress_import(raw))) {
			log_pedantic("Could not convert the stringer to a reducer.");
			ns_free(path);
			st_free(raw);
			return NULL;
		}

		// Decompress the message.
		message = decompress_lzo(compressed);

		// Free the raw buffer, but keep the path around in case we need it for error messages.
		st_free(raw);
	}

	// If were unable to uncompress the file, hide it.
	if (!message) {
		log_pedantic("Could not access a message. { user = %lu / number = %lu / path = %s }",
			user->usernum, meta->messagenum, path);
		mail_db_hide_message(meta->messagenum);
		serial_increment(OBJECT_MESSAGES, user->usernum);
		ns_free(path);
		return NULL;
	}

	// Finally free the path.
	ns_free(path);

	// Only modify and cache the message if parsing is enabled.
	if (parse) {

		// Modify the subject, if necessary.
		if ((meta->status & MAIL_MARK_JUNK) == MAIL_MARK_JUNK) {
			mail_mod_subject(&message, "JUNK:");
		}
		else if ((meta->status & MAIL_MARK_INFECTED) == MAIL_MARK_INFECTED) {
			mail_mod_subject(&message, "INFECTED:");
		}
		else if ((meta->status & MAIL_MARK_SPOOFED) == MAIL_MARK_SPOOFED) {
			mail_mod_subject(&message, "SPOOFED:");
		}
		else if ((meta->status & MAIL_MARK_BLACKHOLED) == MAIL_MARK_BLACKHOLED) {
			mail_mod_subject(&message, "BLACKHOLED:");
		}
		else if ((meta->status & MAIL_MARK_PHISHING) == MAIL_MARK_PHISHING) {
			mail_mod_subject(&message, "PHISHING:");
		}

		if (!(result = mail_message(message))) {
			log_pedantic("Unable to build the message structure.");
			st_free(message);
			return NULL;
		}

		// If we're supposed to parse the message.
		if (meta->signum && meta->sigkey) {
			mail_signature_add(result, server, meta->signum, meta->sigkey, (meta->status & MAIL_MARK_JUNK) == MAIL_MARK_JUNK ? 1 : 0);
		}

	}
	else if (!(result = mail_message(message))) {
		log_pedantic("Unable to build the message structure.");
		st_free(message);
		return NULL;
	}

	// Set thread cache. We use a thread cache since some IMAP clients like to pull messages in chunks leading to
	// lots of serialized requests for small pieces of the same message. Thread caching avoids having to process the
	// message repeatedly.
	mail_cache_set(meta->messagenum, result->text);

	return result;
}

/**
 * @brief	Get the header of a message, checking first in the cache and then on disk.
 * @note	The file data is first unencrypted and decompressed, according to the file header flags.
 * 			When extracted, the header's Subject line is branded with any applicable labels such as JUNK, INFECTED, SPOOFED, BLACKHOLED, PHISHING.
  * @param	meta	the meta message object of the message to be queried.
 * @param	user	the meta user object of the user that owns the message.
 * @return	NULL on failure or a managed string containing the message's header on success.
 */
stringer_t * mail_load_header(meta_message_t *meta, meta_user_t *user, server_t *server, bool_t parse) {

	stringer_t *header;
	mail_message_t *message;

	if (!meta || !user) {
		log_pedantic("Invalid parameter combination passed in.");
		return NULL;
	}

	else if (!(message = mail_load_message(meta, user, server, parse))) {
		log_pedantic("Could not find the end of the header.");
		return NULL;
	}
	else if (!message->header_length) {
		log_pedantic("Could not find the end of the message header.");
		mail_destroy(message);
		return NULL;
	}

	header = st_import(st_char_get(message->text), message->header_length);
	mail_destroy(message);

	if (!header) {
		log_pedantic("Could not import the message header.");
		return NULL;
	}

	return header;
}

/**
 * @brief	Get the top of a mail message, up to a specified maximum number of lines of content.
 * @see		mail_load_message()
 * @param	meta	the meta message object of the message to be loaded from disk.
 * @param	user	the meta user object of the user that owns the requested message.
 * @param	server	the server object of the web server where the spam teacher application is hosted.
 * @param	lines	the maximum number of lines to be retrieved from the loaded message.
 * @param	parse	sets the parse parameter passed to mail_load_message().
 * @return	NULL on failure or a a mail message object containing the retrieved mail message data (truncated if necessary) on success.
 */
mail_message_t * mail_load_message_top(meta_message_t *meta, meta_user_t *user, server_t *server, uint64_t lines, bool_t parse) {

	chr_t *stream;
	int_t header = 1;
	mail_message_t *result;
	size_t length, increment;

	if (!(result = mail_load_message(meta, user, server, parse))) {
		return NULL;
	}

	// Now that we've added the signature we iterate again.
	length = st_length_get(result->text);
	stream = st_char_get(result->text);

	// QUESTION: Does mail_header_end() already do this?
	for (increment = 0; increment < length && header != 3; increment++) {

		// Logic for detecting the end of the header.
		if (header == 0 && *stream == '\n') {
			header++;
		}
		else if (header == 1 && *stream == '\n') {
			header += 2;
		}
		else if (header == 1 && *stream == '\r') {
			header++;
		}
		else if (header == 2 && *stream == '\n') {
			header++;
		}
		else if (header != 0) {
			header = 0;
		}
		stream++;
	}

	// Now we need to advance X number of lines.
	while (lines && increment < length) {

		if (*stream == '\n') {
			lines--;
		}

		increment++;
		stream++;
	}

	// Use the stringer length parameter to restrict how much data is outputted.
	st_length_set(result->text, increment);

	return result;
}
