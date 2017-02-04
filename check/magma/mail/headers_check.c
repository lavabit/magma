
/**
 * @file /magma/check/magma/mail/headers_check.c
 */

#include "magma_check.h"

bool_t check_mail_headers_sthread(stringer_t *errmsg) {

	bool_t result = true;
	uint32_t max = check_message_max();
	stringer_t *data = NULL;
	smtp_message_t *parsed = NULL;
	connection_t con;

	mm_wipe(&con, sizeof(connection_t));
	con.smtp.authenticated = true;
	con.smtp.mailfrom = NULLER("check@example.com");


	for (uint32_t i = 0; i < max && result && status(); i++) {

		if (!(data = check_message_get(i))) {
			st_sprint(errmsg, "Failed to get the message data. { message = %i }", i);
			result = false;
		}

		else if (!(parsed = mail_create_message(data))) {
			st_sprint(errmsg, "Mail message object creation failed. { message = %i }", i);
			result = false;
		}

		else if (!mail_headers(parsed)) {
			st_sprint(errmsg, "Mail message header parsing failed. { message = %i }", i);
			result = false;
		}

		else if (mail_add_required_headers(&con, parsed)) {
			st_sprint(errmsg, "Mail message header cleanup failed to add the required headers. { message = %i }", i);
			result = false;
		}

		if (parsed) mail_destroy_message(parsed);
		else st_cleanup(data);
		parsed = NULL;
	}


	return result;
}
