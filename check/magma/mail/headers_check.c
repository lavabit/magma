
/**
 * @file /magma/check/magma/mail/headers_check.c
 */

#include "magma_check.h"

bool_t mail_tls_header(smtp_message_t *parsed) {
	return strstr(st_char_get(parsed->text), "(version=");
}

bool_t check_mail_headers_sthread(stringer_t *errmsg) {

	connection_t con;
	bool_t result = true;
	stringer_t *data = NULL;
	smtp_message_t *parsed = NULL;
	uint32_t max = check_message_max();

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

		else if (!mail_add_required_headers(&con, parsed)) {
			st_sprint(errmsg, "Mail message header cleanup failed to add the required headers. { message = %i }", i);
			result = false;
		}

		else if(con_secure(con.network.tls) == 1 && !mail_tls_header(parsed)) {
			st_sprint(errmsg, "Mail message header doesn't have the TLS signature when using TLS", mail_tls_header(parsed));
			result = false;
		}

		if (parsed) mail_destroy_message(parsed);
		else st_cleanup(data);
		parsed = NULL;
	}


	return result;
}
