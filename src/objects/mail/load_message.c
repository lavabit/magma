
/**
 * @file /magma/objects/mail/load_message.c
 *
 * @brief	Functions used to load mail messages.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Get the header of a message, checking first in the cache and then on disk.
 * @note	The file data is first unencrypted and decompressed, according to the file header flags.
 * 			When extracted, the header's Subject line is branded with any applicable labels such as JUNK, INFECTED, SPOOFED, BLACKHOLED, PHISHING.
  * @param	meta	the meta message object of the message to be queried.
 * @param	user	the meta user object of the user that owns the message.
 * @return	NULL on failure or a managed string containing the message's header on success.
 */
stringer_t * mail_load_header(meta_message_t *meta, meta_user_t *user) {

	struct stat file_info;
	message_fheader_t fheader;
	mail_message_t *message;
	stringer_t *uncompressed;
	uint32_t total, taken = 0;
	uchr_t *unencrypted;
	chr_t *path, key[128], *raw;
	off_t offset = sizeof(compress_head_t) + sizeof(message_fheader_t);
	size_t dec_len, data_len;
	int_t fd, keylen, block_len = compress_block_length() + offset;

	// QUESTION: Do we check for user secure flag here, or user->storage_privkey != NULL ?

	if (!meta) {
		log_pedantic("Invalid parameter combination passed in.");
		return NULL;
	}

	// Create the cache key.
	keylen = snprintf(key, 128, "magma.message.header.%lu", meta->messagenum);

	if (!(uncompressed = cache_get(PLACER(key, keylen)))) {

		// Read the header from the storage server.
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

		// Figure out the size of the file
		if (fstat(fd, &file_info) != 0) {
			log_pedantic("Could not fstat file %s.", path);
			close(fd);
			ns_free(path);
			return NULL;
		}

		if (file_info.st_size < sizeof(message_fheader_t)) {
			log_pedantic("Mail message was missing full file header: { %s }", path);
			close(fd);
			ns_free(path);
			return NULL;
		}

		// Do some sanity checking on the message header
		data_len = file_info.st_size - sizeof(message_fheader_t);

		if (read(fd, &fheader, sizeof(fheader)) != sizeof(fheader)) {
			log_pedantic("Unable to read message file header: { %s }", path);
			close(fd);
			ns_free(path);
			return NULL;
		}

		if ((fheader.magic1 != FMESSAGE_MAGIC_1) || (fheader.magic2 != FMESSAGE_MAGIC_2)) {
			log_pedantic("Mail message had incorrect file format: { %s }", path);
			close(fd);
			ns_free(path);
			return NULL;
		}

		// QUESTION: What exactly should we do here?
		// We can fail... or we can just trust the file header. It's unclear which is best.
		if ((meta->status & MAIL_STATUS_ENCRYPTED) && !(fheader.flags & FMESSAGE_OPT_ENCRYPTED)) {
			log_pedantic("Message state mismatch: encrypted in database but unencrypted on disk.");
			close(fd);
			ns_free(path);
			return NULL;
		} else if (!(meta->status & MAIL_STATUS_ENCRYPTED) && (fheader.flags & FMESSAGE_OPT_ENCRYPTED)) {
			log_pedantic("Message state mismatch: unencrypted in database but encrypted on disk.");
			close(fd);
			ns_free(path);
			return NULL;
		}

		// If the message is encrypted we have to read in the entire buffer
		if (meta->status & MAIL_STATUS_ENCRYPTED) {
			total = data_len;
		// but if it's not, we don't need to decompress all of it.
		} else
		{

			// Seek to a position past the compression header.
			if (lseek(fd, offset, SEEK_SET) != offset) {
				log_pedantic("Could not fstat or lseek the file %s.", path);
				close(fd);
				ns_free(path);
				return NULL;
			}

			// If the file is smaller than our block size, read the whole file.
			total = ((file_info.st_size - offset) < block_len ? (file_info.st_size - offset) : block_len);
		}

		// Allocate a buffer to hold the compressed and/or encrypted data.
		if (!(raw = mm_alloc(total))) {
			log_pedantic("Could not allocate a block of %u bytes to hold the message header buffer.", block_len);
			close(fd);
			ns_free(path);
			return NULL;
		}

		// Read the file.
		if ((taken = read(fd, raw, total)) != total) {
			log_pedantic("Could not read all %i bytes of the file %s.", total, path);
			close(fd);
			ns_free(path);
			ns_free(raw);
			return NULL;
		}

		ns_free(path);
		close(fd);

		// If encrypted, we must decrypt the message first.
		if (meta->status & MAIL_STATUS_ENCRYPTED) {

			// First read in the cryptex disk header.

			if (!(unencrypted = ecies_decrypt(user->storage_privkey, ECIES_PRIVATE_BINARY, (cryptex_t *) raw, &dec_len))) {
				log_pedantic("Failed to decrypt message mail header.");
				ns_free(raw);
				return NULL;
			}

			ns_free(raw);
			raw = (chr_t *) unencrypted;
			taken = dec_len;
			uncompressed = decompress_lzo((compress_t *)raw);
		}
		// Otherwise go straight to decompression.
		else {
			uncompressed = decompress_block_lzo(PLACER(raw, taken));
		}

		ns_free(raw);

		// Check whether decompression succeeded and then look for the end of the header.
		if (!uncompressed) {
			log_pedantic("Could not uncompress the header data.");
			return NULL;
		}
		// Workaround for messages with headers longer than 8192 characters. If that happens we try loading the entire message and then searching for the end of the header.
		else if ((fd = mail_header_end(uncompressed)) < 0 || fd == st_length_get(uncompressed)) {

			if (!(message = mail_load_message(meta, user, NULL, false))) {
				log_pedantic("Could not find the end of the header.");
				st_free(uncompressed);
				return NULL;
			} else if (!message->header_length) {
				log_pedantic("Could not find the end of the header.");
				st_free(uncompressed);
				mail_destroy(message);
				return NULL;
			}

			st_free(uncompressed);
			uncompressed = st_import(st_char_get(message->text), message->header_length);
			mail_destroy(message);

			if (!uncompressed) {
				log_pedantic("Could not find the end of the header.");
				return NULL;
			}

		}
		else {
			st_length_set(uncompressed, fd);
		}

		// Modify the subject, if necessary.
		if ((meta->status & MAIL_MARK_JUNK) == MAIL_MARK_JUNK) {
			mail_mod_subject(&uncompressed, "JUNK:");
		}
		else if ((meta->status & MAIL_MARK_INFECTED) == MAIL_MARK_INFECTED) {
			mail_mod_subject(&uncompressed, "INFECTED:");
		}
		else if ((meta->status & MAIL_MARK_SPOOFED) == MAIL_MARK_SPOOFED) {
			mail_mod_subject(&uncompressed, "SPOOFED:");
		}
		else if ((meta->status & MAIL_MARK_BLACKHOLED) == MAIL_MARK_BLACKHOLED) {
			mail_mod_subject(&uncompressed, "BLACKHOLED:");
		}
		else if ((meta->status & MAIL_MARK_PHISHING) == MAIL_MARK_PHISHING) {
			mail_mod_subject(&uncompressed, "PHISHING:");
		}

		// Cache the header for next time.
		cache_add(PLACER(key, keylen), uncompressed, 3600);
	}

	return uncompressed;
}

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

	int_t fd, keylen;
	chr_t *path, key[128];
	message_fheader_t fheader;
	compress_t *compressed;
	stringer_t *raw, *uncompressed;
	uchr_t *unencrypted;
	mail_message_t *result;
	struct stat file_info;
	size_t data_len, plain_len;

	if (!meta || (parse && (!user || !server))) {
		log_pedantic("Invalid parameter combination passed in.");
		return NULL;
	}

	// Check the message cache first.
	// Only parsed messages hit the mail cache because there is a lot more work to be performed for them.
	if (parse && (uncompressed = mail_cache_get(meta->messagenum))) {

		if (!(result = mail_message(uncompressed))) {
			log_pedantic("Unable to build the message structure.");
			return NULL;
		}

		return result;
	}

	if (!(path = mail_message_path(meta->messagenum, meta->server))) {
		log_pedantic("Could not build the message path.");
		return NULL;
	}

	// Create the cache key.
	keylen = snprintf(key, 128, "magma.message.%lu", meta->messagenum);

	if (!(raw = cache_get(PLACER(key, keylen)))) {

		// Open the file.LZO1X_1_MEM_COMPRESS
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

		if (file_info.st_size < sizeof(message_fheader_t)) {
			log_pedantic("Mail message was missing full file header: { %s }", path);
			close(fd);
			ns_free(path);
			return NULL;
		}

		// Do some sanity checking on the message header
		data_len = file_info.st_size - sizeof(message_fheader_t);

		if (read(fd, &fheader, sizeof(fheader)) != sizeof(fheader)) {
			log_pedantic("Unable to read message file header: { %s }", path);
			close(fd);
			ns_free(path);
			return NULL;
		}

		if ((fheader.magic1 != FMESSAGE_MAGIC_1) || (fheader.magic2 != FMESSAGE_MAGIC_2)) {
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
			close(fd);
			ns_free(path);
			st_free(raw);
			return NULL;
		}

		// Were done with the file.
		close(fd);

		// Tell the stringer how much data is there.
		st_length_set(raw, data_len);

		if (meta->status & MAIL_STATUS_ENCRYPTED) {

			if (!(fheader.flags & FMESSAGE_OPT_ENCRYPTED)) {
				log_pedantic("Message state mismatch: encrypted in database but unencrypted on disk.");
			}

			if (!(user->flags & META_USER_ENCRYPT_DATA)) {
				log_info("User with secure mode off requested encrypted message.");
			}

			if (!user->storage_privkey) {
				log_pedantic("User cannot read encrypted message without a private key!");
				ns_free(path);
				st_free(raw);
				return NULL;
			}

			if (!(unencrypted = ecies_decrypt(user->storage_privkey, ECIES_PRIVATE_BINARY, (cryptex_t *)st_data_get(raw), &plain_len))) {
				log_pedantic("Unable to decrypt mail message.");
				ns_free(path);
				st_free(raw);
				return NULL;
			}

			st_free(raw);

			if (!(raw = st_import(unencrypted, plain_len))) {
				log_pedantic("Unable to copy decrypted mail message buffer.");
				ns_free(path);
				mm_free(unencrypted);
				return NULL;
			}

			mm_free(unencrypted);
		} else if (fheader.flags & FMESSAGE_OPT_ENCRYPTED) {
				log_pedantic("Message state mismatch: unencrypted in database but encrypted on disk.");
		}

		// Store the compressed data.
		/*if (st_length_get(compressed) <= 65536) {
			cache_add_ns(key, keylen, st_char_get(compressed), st_length_get(compressed), 3600);
		}
		 Memcached can currently only store objects less than 1 megabyte.
		else if (st_length_get(compressed) <= 2097152) {
			cache_add_ns(key, keylen, st_char_get(compressed), st_length_get(compressed), 86400);
		}
		else {
			cache_add_ns(key, keylen, st_char_get(compressed), st_length_get(compressed), 7200);
		}*/
	}

	// QUESTION: Compress then decompress???
	// Convert the string buffer into a compression buffer.
	if (!(compressed = compress_import(raw))) {
		log_pedantic("Could not convert the stringer to a reducer.");
		ns_free(path);
		st_free(raw);
		return NULL;
	}

	// Decompress the message.
	uncompressed = decompress_lzo(compressed);

	st_free(raw);

	// If were unable to uncompress the file, hide it.
	if (!uncompressed) {
		log_pedantic("Could not uncompress the file %s.", path);
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
			mail_mod_subject(&uncompressed, "JUNK:");
		}
		else if ((meta->status & MAIL_MARK_INFECTED) == MAIL_MARK_INFECTED) {
			mail_mod_subject(&uncompressed, "INFECTED:");
		}
		else if ((meta->status & MAIL_MARK_SPOOFED) == MAIL_MARK_SPOOFED) {
			mail_mod_subject(&uncompressed, "SPOOFED:");
		}
		else if ((meta->status & MAIL_MARK_BLACKHOLED) == MAIL_MARK_BLACKHOLED) {
			mail_mod_subject(&uncompressed, "BLACKHOLED:");
		}
		else if ((meta->status & MAIL_MARK_PHISHING) == MAIL_MARK_PHISHING) {
			mail_mod_subject(&uncompressed, "PHISHING:");
		}

		if (!(result = mail_message(uncompressed))) {
			log_pedantic("Unable to build the message structure.");
			st_free(uncompressed);
			return NULL;
		}

		// If we're supposed to parse the message.
		if (meta->signum && meta->sigkey) {
			mail_signature_add(result, server, meta->signum, meta->sigkey, (meta->status & MAIL_MARK_JUNK) == MAIL_MARK_JUNK ? 1 : 0);
		}

		// Set thread cache. We use a thread cache since some IMAP clients like to pull messages in chunks leading to
		// lots of requests for small amounts of data.
		mail_cache_set(meta->messagenum, result->text);

	}
	else if (!(result = mail_message(uncompressed))) {
		log_pedantic("Unable to build the message structure.");
		st_free(uncompressed);
		return NULL;
	}
	// Cache the header as well.
	// QUESTION: How is this ever going to be reached? if (parse) is evaluated twice...
	else if (parse) {
		keylen = snprintf(key, 128, "magma.message.header.%lu", meta->messagenum);
		cache_add(PLACER(key, keylen), PLACER(st_char_get(result->text), result->header_length), 3600);
	}

	return result;
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
