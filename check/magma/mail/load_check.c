
/**
 * @file /magma/check/magma/mail/load_check.c
 */

#include "magma_check.h"


bool_t check_mail_load_sthread(stringer_t *errmsg) {

	auth_t *auth = NULL;
	bool_t result = true;
	meta_user_t *user = NULL;
	meta_folder_t *folder = NULL;
	inx_cursor_t *cursor;
	meta_message_t *active;
	mail_message_t *message;
	stringer_t *usernames[] = { PLACER("magma", 5) }, *passwords[] = { PLACER("password", 8) };

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

		else if (!(cursor = inx_cursor_alloc(user->messages))) {
			st_sprint(errmsg, "User message cursor failed to allocate.");
			result = false;
		}

		while (result && cursor && (active = inx_cursor_value_next(cursor))) {
			if (!(message = mail_load_message(active,  user,  NULL, false))) {
				st_sprint(errmsg, "User message failed to load properly. { usernum = %lu / message = %lu }",
					user->usernum, active->messagenum);
				result = false;
			}
			else {
				mail_destroy(message);
			}
		}

		if (cursor) inx_cursor_free(cursor);
		if (auth) auth_free(auth);
		if (user) meta_inx_remove(user->usernum, META_PROTOCOL_IMAP);

		auth = NULL;
		user = NULL;
	}

	return result;
}
