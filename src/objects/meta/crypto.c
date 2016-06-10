
/**
 * @file /magma/src/objects/meta/crypto.c
 *
 * @brief The meta module interfaces for handling cryptographic functionality.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief Creates a new ECIES key pair and inserts it into the database.
 *
 * @param user
 * @param master
 *
 * @return -1 if an error occurs, 0 if successful, and 1 if the insertion fails because a key already exists.
 */
int_t meta_crypto_keys_create(uint64_t usernum, stringer_t *username, stringer_t *master, int64_t transaction) {

	size_t length = 0;
	EC_KEY *ecies_key = NULL;
	scramble_t *scramble = NULL;
	uchr_t *public = NULL, *private = NULL;
	key_pair_t pair = {
		NULL, NULL
	};

	// Create the ECIES key pair first and extract the public and private keys, wrapping them in managed strings.
	if (!(ecies_key = ecies_key_create())) {
		log_pedantic("Unable to create a user key pair. { username = %.*s }",
			st_length_int(username), st_char_get(username));
		return -1;
	}

	// Extract the public portion in binary form.
	else if (!(public = ecies_key_public_bin(ecies_key, &length)) || length <= 0 || !(pair.public = st_import(public, length))) {

		log_pedantic("Unable to extract the ECIES public key from the user key pair. { username = %.*s }",
			st_length_int(username), st_char_get(username));

		ecies_key_free(ecies_key);
		mm_cleanup(public);

		return -1;
	}

	// Extract the private portion of the key pair in binary form.
	else if (!(private = ecies_key_private_bin(ecies_key, &length)) || length <= 0
		|| !(scramble = scramble_encrypt(master, PLACER(private, length)))
		|| !(pair.private = st_import(scramble, scramble_total_length(scramble)))) {

		log_pedantic("Unable to extract and scamble the private portion of the user key pair. { username = %.*s }",
			st_length_int(username), st_char_get(username));

		scramble_cleanup(scramble);
		ecies_key_free(ecies_key);
		mm_sec_cleanup(private);
		st_cleanup(pair.public);
		mm_cleanup(public);

		return -1;
	}

	scramble_cleanup(scramble);
	ecies_key_free(ecies_key);
	mm_sec_cleanup(private);
	mm_cleanup(public);

	// Try storing the keys in the database. If 0 is returned, the new pair was stored, otherwise if a 1 is returned
	// its possible another process created the keys already, in which case they will be retrieved below.
	if (meta_data_insert_keys(usernum, username, &pair, transaction) < 0) {
		log_pedantic("Unable to store the user key pair. { username = %.*s }", st_length_int(username), st_char_get(username));
		st_cleanup(pair.private, pair.public);
		return 1;
	}

	log_info("Created user storage keys. { username = %.*s }", st_length_int(username), st_char_get(username));
	st_cleanup(pair.private, pair.public);

	return 0;
}

/**
 * @brief	Adjust the encrypted status of a message, in accordance with the user's secure flag.
 * @param	user		a pointer to the meta user object owning the specified message.
 * @param	message		a pointer to the meta message that should have its on-disk data updated accordingly.
 * @param	oprivkey	if re-encryption is specified (user's secure flag is set and the message is already encrypted),
 * 						this is a pointer to a managed string that contains the user's original private key in binary form.
 * @return	true on success or false on failure.
 */
/*bool_t adjuset_message_encryption(meta_user_t *user, meta_message_t *message, stringer_t *oprivkey) {

	inx_t *mholder;
	multi_t nkey;
	stringer_t *fcontents, *ftmpname;
	message_fheader_t *fheader, new_fheader;
	cryptex_t *enc_data = NULL;
	uint32_t transaction;
	size_t data_length, mdatalen;
	uchr_t *mdataptr;
	chr_t *msgpath, *write_data;
	int_t fd;
	bool_t do_encrypt = ((user->flags & META_USER_ENCRYPT_DATA) == META_USER_ENCRYPT_DATA);
	bool_t message_encrypted = ((message->status & MAIL_STATUS_ENCRYPTED) == MAIL_STATUS_ENCRYPTED);

	// Nothing to do if encryption is off and message is already decrypted
	if (!do_encrypt && !message_encrypted) {
		return true;
	}

	if (!(msgpath = mail_message_path(message->messagenum, message->server))) {
		log_pedantic("Unable to get file path of mail message.");
		return false;
	}

	if (!(fcontents = file_load(msgpath))) {
		log_pedantic("Unable to retrieve contents of old message data for encryption change operation.");
		ns_free(msgpath);
		return false;
	}

	if (st_length_get(fcontents) < sizeof(message_fheader_t)) {
		log_pedantic("Mail message was missing full file header: { %s }", msgpath);
		ns_free(msgpath);
		return false;
	}

	// Do some sanity checking on the message header
	fheader = (message_fheader_t *) st_data_get (fcontents);

	if ((fheader->magic1 != FMESSAGE_MAGIC_1) || (fheader->magic2 != FMESSAGE_MAGIC_2)) {
		log_pedantic("Mail message had incorrect file format: { %s }", msgpath);
		ns_free(msgpath);
		return false;
	}

	mdataptr = (uchr_t *) fheader;
	mdataptr += sizeof(message_fheader_t);
	mdatalen = st_length_get(fcontents) - sizeof(message_fheader_t);

	// Prepare the new message file header to be written
	new_fheader.magic1 = FMESSAGE_MAGIC_1;
	new_fheader.magic2 = FMESSAGE_MAGIC_2;
	new_fheader.reserved = 0;
	new_fheader.flags = fheader->flags;

	// We are left with 3 possible cases:

	// If encryption on and the message isn't encrypted, encrypt it.
	if (do_encrypt && !message_encrypted) {

		if (fheader->flags & FMESSAGE_OPT_ENCRYPTED) {
			log_pedantic("Message state mismatch: unencrypted in database but encrypted on disk.");
		}

		if (!(enc_data = ecies_encrypt(user->storage_pubkey, ECIES_PUBLIC_BINARY, mdataptr, mdatalen))) {
			log_pedantic("Unable to encrypt contents of user's message.");
			ns_free(msgpath);
			ns_free(fcontents);
			return false;
		}

		data_length = (size_t) cryptex_total_length(enc_data);

		if (!(write_data = ns_import(enc_data, data_length))) {
			log_pedantic("Unable to allocate buffer for user's encrypted message.");
			ns_free(msgpath);
			ns_free(fcontents);
			cryptex_free(enc_data);
			return false;
		}

		new_fheader.flags |= FMESSAGE_OPT_ENCRYPTED;
		cryptex_free(enc_data);
	// If encryption is off and the message is encrypted, decrypt it.
	} else if (!do_encrypt && message_encrypted) {

		if (!(fheader->flags & FMESSAGE_OPT_ENCRYPTED)) {
			log_pedantic("Message state mismatch: encrypted in database but unencrypted on disk.");
		}

		if (!(write_data = (chr_t *) ecies_decrypt(user->storage_privkey, ECIES_PRIVATE_BINARY, (cryptex_t *) mdataptr, &data_length))) {
			log_pedantic("Unable to decrypt contents of user's message.");
			ns_free(msgpath);
			ns_free(fcontents);
			return false;
		}

		new_fheader.flags &= ~FMESSAGE_OPT_ENCRYPTED;
	}

	ns_free(fcontents);

	if ((fd = get_temp_file_handle(NULL,&ftmpname)) < 0) {
		log_pedantic("Unable to get file descriptor for temp file.");
		ns_free(msgpath);
		ns_free(write_data);
		return false;
	}

	if ((write(fd, &new_fheader, sizeof(new_fheader)) != sizeof(new_fheader)) || (write(fd, write_data, data_length) != data_length)) {
		log_pedantic("Write of message data to temp file failed.");
		ns_free(msgpath);
		close(fd);
		unlink(st_char_get(ftmpname));
		return false;
	}

	fsync(fd);
	close(fd);
	ns_free(write_data);

	 We have the transformed contents of a message in a temp file. Now make an atomic transaction of updating the file,
	 * along with updating the message's flags in the database.

	 * Construct a dummy holder for our message so we can update the flags in the database
	 * We don't need to free them since they were already allocated for us by the caller.
	if (!(mholder = inx_alloc(M_INX_LINKED, NULL))) {
		log_pedantic("Could not allocate holder for encrypted user message.");
		ns_free(msgpath);
		unlink(st_char_get(ftmpname));
		return false;
	}

	nkey.type = M_TYPE_UINT64;
	nkey.val.u64 = message->messagenum;

	// Add this single message to the structure.
	if (!inx_insert(mholder, nkey, message)) {
		log_pedantic("Could not prepare encrypted message for update in database.");
		ns_free(msgpath);
		unlink(st_char_get(ftmpname));
		inx_free(mholder);
		return false;
	}

	if ((transaction = tran_start()) < 0) {
		log_pedantic("Unable to start transaction for user message encryption.");
		ns_free(msgpath);
		unlink(st_char_get(ftmpname));
		inx_free(mholder);
		return false;
	}

	// If we're encrypting, add the encrypted flag to the message
	if (do_encrypt && !message_encrypted && !(meta_data_flags_add(mholder, user->usernum, message->foldernum, MAIL_STATUS_ENCRYPTED))) {
		log_pedantic("Unable to set encryption flag for message in database.");
		ns_free(msgpath);
		unlink(st_char_get(ftmpname));
		inx_free(mholder);
		tran_rollback(transaction);
		return false;
	// or if we're decrypting, remove the encrypted flag from the message.
	} else if (!do_encrypt && message_encrypted && !(meta_data_flags_remove(mholder, user->usernum, message->foldernum, MAIL_STATUS_ENCRYPTED))) {
		log_pedantic("Unable to clear encryption flag for message in database.");
		ns_free(msgpath);
		unlink(st_char_get(ftmpname));
		inx_free(mholder);
		tran_rollback(transaction);
		return false;
	}

	inx_free(mholder);

	// Update the message flags in-memory.
	if (do_encrypt) {
		message->status |= MAIL_STATUS_ENCRYPTED;

	} else {
		message->status &= ~MAIL_STATUS_ENCRYPTED;
	}

	if (rename(st_char_get(ftmpname), msgpath) < 0) {
		//if (rename(msgpath,st_char_get(ftmpname)) <  0) {
		log_pedantic("Rename of encrypted temp file failed.");
		ns_free(msgpath);
		unlink(st_char_get(ftmpname));
		tran_rollback(transaction);
		return false;
	}

	// QUESTION: What do we do if rename() succeeds but the transaction fails?
	if (tran_commit(transaction)) {
		log_pedantic("Transaction commit for file encryption failed.");
	}

	ns_free(msgpath);
	unlink(st_char_get(ftmpname));

	return true;
}*/

/**
 * @brief	Encrypt all of a user's messages that aren't tagged as encrypted already.
 * @param	user	the meta user object to have its messages processed.
 * @return	This function returns no value.
 */
/*
void encrypt_user_messages(meta_user_t *user) {

	inx_cursor_t *cursor;
	meta_message_t *message;
	int_t pending = 0;

	if ((cursor = inx_cursor_alloc(user->messages))) {

		// Cycle through all the messages and decrypt only the ones that need to be decrypted
		while ((message = inx_cursor_value_next(cursor))) {

			if (message->status & MAIL_STATUS_ENCRYPTED)
				continue;

			if (!adjust_message_encryption(user,message,NULL)) {
				log_pedantic("Message encryption operation failed.");
				pending++;
			}

		}

		inx_cursor_free(cursor);
	}
	else {
		log_pedantic("Unable to allocate cursor for batch encryption routine.");
	}

	if (pending) {
		log_info("Failed to encrypt entire queued message batch. Submitting for sleep + reprocessing.");
		// QUESTION: Is this how we do it?
		enqueue(encrypt_user_messages, user);
	} else {
		log_info("Message encryption batch successfully completed for user: %s", st_char_get(user->username));
		meta_user_ref_dec(user, META_PROTOCOL_GENERIC);
	}

	return;
}
*/

/**
 * @brief	Decrypt all of a user's messages that aren't tagged as unencrypted already.
 * @param	user	the meta user object to have its messages processed.
 * @return	This function returns no value.
 */
/*
void decrypt_user_messages(meta_user_t *user) {

	inx_cursor_t *cursor;
	meta_message_t *message;
	int_t pending = 0;

	if ((cursor = inx_cursor_alloc(user->messages))) {

		// Cycle through all the messages and decrypt only the ones that need to be decrypted
		while ((message = inx_cursor_value_next(cursor))) {

			if (!(message->status & MAIL_STATUS_ENCRYPTED))
				continue;

			if (!adjust_message_encryption(user,message,NULL)) {
				log_pedantic("Message decryption operation failed.");
				pending++;
			}

		}

		inx_cursor_free(cursor);
	}
	else {
		log_pedantic("Unable to allocate cursor for batch decryption routine.");
	}

	if (pending) {
		log_info("Failed to decrypt entire queued message batch. Submitting for sleep + reprocessing.");
		// QUESTION: Is this how we do it?
		enqueue(encrypt_user_messages, user);
	} else {
		log_info("Message decryption batch successfully completed for user: %s", st_char_get(user->username));
		meta_user_ref_dec(user, META_PROTOCOL_GENERIC);
	}

	return;
}
*/
