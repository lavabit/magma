
/**
 * @file /magma/check/magma/mail/store_check.c
 */

#include "magma_check.h"

bool_t check_mail_store_plaintext_sthread(stringer_t *errmsg) {

	uint32_t flags = 0;
	auth_t *auth = NULL;
	bool_t result = true;
	meta_user_t *user = NULL;
	meta_folder_t *folder = NULL;
	uint32_t max = check_message_max();
	stringer_t *usernames[] = { PLACER("magma", 5) }, *passwords[] = { PLACER("password", 8) }, *data = NULL;

	// The registration check must be run frist, otherwise we won't have a user to check against.
//	if (status() && (!check_username || !check_password)) {
//		check_users_register_s(0);
//		usernames[1] = check_username;
//		passwords[1] = check_password;
//	}

	for (int_t i = 0; i < (sizeof(usernames)/sizeof(stringer_t *)) && result && status(); i++) {

		if (st_empty(usernames[i]) || st_empty(passwords[i])) {
			st_sprint(errmsg, "User meta login check failed. The username and password were invalid.");
			result = false;
		}

		else if (auth_login(usernames[i], passwords[i], &auth)) {
			st_sprint(errmsg, "User meta login check failed. Authentication failure. { username =  %.*s / password = %.*s }",
				st_length_int(usernames[i]), st_char_get(usernames[i]), st_length_int(passwords[i]), st_char_get(passwords[i]));
			result = false;
		}

		else if (meta_get(auth->usernum, auth->username, auth->keys.master, auth->tokens.verification,
			META_PROTOCOL_IMAP, META_GET_KEYS | META_GET_ALIASES | META_GET_FOLDERS | META_GET_CONTACTS | META_GET_MESSAGES, &(user))) {
			st_sprint(errmsg, "User meta login check failed. Get user metadata failure. { username =  %.*s / password = %.*s }",
				st_length_int(usernames[i]), st_char_get(usernames[i]), st_length_int(passwords[i]), st_char_get(passwords[i]));
			result = false;
		}
		else if (!(folder = meta_folders_by_name(user->folders, NULLER("Inbox")))) {
			st_sprint(errmsg, "User Inbox appears to be missing. { username =  %.*s / password = %.*s }",
				st_length_int(usernames[i]), st_char_get(usernames[i]), st_length_int(passwords[i]), st_char_get(passwords[i]));
			result = false;
		}

		for (uint32_t j = 0; j < max && result && status(); j++) {

			if (!(data = check_message_get(j))) {
				st_sprint(errmsg, "Failed to get the message data. { message = %i }", j);
				result = false;
			}

			else if (mail_store_message(user->usernum, NULL, folder->foldernum, &flags, 0, 0, data) == 0) {
				st_sprint(errmsg, "Failed to store the plaintext message data. { message = %i }", j);
				result = false;
			}

			st_cleanup(data);
			flags = 0;
		}

		if (auth) auth_free(auth);
		if (user) meta_inx_remove(user->usernum, META_PROTOCOL_IMAP);

		auth = NULL;
		user = NULL;

	}

	return result;
}

bool_t check_mail_store_encrypted_sthread(stringer_t *errmsg) {

	uint32_t flags = 0;
	auth_t *auth = NULL;
	bool_t result = true;
	meta_user_t *user = NULL;
	meta_folder_t *folder = NULL;
	uint32_t max = check_message_max();
	stringer_t *usernames[] = { PLACER("magma", 5) }, *passwords[] = { PLACER("password", 8) }, *data = NULL;

	// The registration check must be run frist, otherwise we won't have a user to check against.
//	if (status() && (!check_username || !check_password)) {
//		check_users_register_s(0);
//		usernames[1] = check_username;
//		passwords[1] = check_password;
//	}

	for (int_t i = 0; i < (sizeof(usernames)/sizeof(stringer_t *)) && result && status(); i++) {

		if (st_empty(usernames[i]) || st_empty(passwords[i])) {
			st_sprint(errmsg, "User meta login check failed. The username and password were invalid.");
			result = false;
		}

		else if (auth_login(usernames[i], passwords[i], &auth)) {
			st_sprint(errmsg, "User meta login check failed. Authentication failure. { username =  %.*s / password = %.*s }",
				st_length_int(usernames[i]), st_char_get(usernames[i]), st_length_int(passwords[i]), st_char_get(passwords[i]));
			result = false;
		}

		else if (meta_get(auth->usernum, auth->username, auth->keys.master, auth->tokens.verification,
			META_PROTOCOL_IMAP, META_GET_KEYS | META_GET_ALIASES | META_GET_FOLDERS | META_GET_CONTACTS | META_GET_MESSAGES, &(user))) {
			st_sprint(errmsg, "User meta login check failed. Get user metadata failure. { username =  %.*s / password = %.*s }",
				st_length_int(usernames[i]), st_char_get(usernames[i]), st_length_int(passwords[i]), st_char_get(passwords[i]));
			result = false;
		}
		else if (!(folder = meta_folders_by_name(user->folders, NULLER("Inbox")))) {
			st_sprint(errmsg, "User Inbox appears to be missing. { username =  %.*s / password = %.*s }",
				st_length_int(usernames[i]), st_char_get(usernames[i]), st_length_int(passwords[i]), st_char_get(passwords[i]));
			result = false;
		}

		for (uint32_t j = 0; j < max && result && status(); j++) {

			if (!(data = check_message_get(j))) {
				st_sprint(errmsg, "Failed to get the message data. { message = %i }", j);
				result = false;
			}

			else if (mail_store_message(user->usernum, user->prime.signet, folder->foldernum, &flags, 0, 0, data) == 0) {
				st_sprint(errmsg, "Failed to store the encrypted message data. { message = %i }", j);
				result = false;
			}

			st_cleanup(data);
			flags = 0;
		}

		if (auth) auth_free(auth);
		if (user) meta_inx_remove(user->usernum, META_PROTOCOL_IMAP);

		auth = NULL;
		user = NULL;

	}

	return result;
}
