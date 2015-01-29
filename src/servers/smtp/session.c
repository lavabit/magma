
/**
 * @file /magma/servers/smtp/session.c
 *
 * @brief	Functions used to handle SMTP sessions.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Reset an SMTP session to its initialized state.
 * @param	con		the SMTP client connection to be reset.
 * @return	This function returns no value.
 */
void smtp_session_reset(connection_t *con) {

	st_cleanup(con->smtp.mailfrom);
	con->smtp.mailfrom = NULL;

	if (con->smtp.message) {
		mail_destroy_message(con->smtp.message);
		con->smtp.message = NULL;
	}

	if (con->smtp.in_prefs) {
		smtp_free_inbound(con->smtp.in_prefs);
		con->smtp.in_prefs = NULL;
	}

	if (con->smtp.out_prefs && con->smtp.out_prefs->recipients) {
		smtp_free_recipients(con->smtp.out_prefs->recipients);
		con->smtp.out_prefs->recipients = NULL;
	}

	// For unauthenticated sessions, reset the max length to zero and then adjust it when a recipient is provided.
	if (!con->smtp.out_prefs) {
		con->smtp.max_length = 0;
	}

	con->smtp.checked.rbl = 0;
	con->smtp.checked.spf = 0;
	con->smtp.checked.dkim = 0;
	con->smtp.checked.virus = 0;

	con->smtp.suggested_eight_bit = false;
	con->smtp.suggested_length = 0;
	con->smtp.num_recipients = 0;

	return;
}

/**
 * @brief	Destroy the data associated with an SMTP session.
 * @param	con		the SMTP client connection to be destroyed.
 * @return	This function returns no value.
 */
void smtp_session_destroy(connection_t *con) {

	st_cleanup(con->smtp.helo);
	st_cleanup(con->smtp.mailfrom);

	if (con->smtp.message) {
		mail_destroy_message(con->smtp.message);
	}

	if (con->smtp.in_prefs) {
		smtp_free_inbound(con->smtp.in_prefs);
	}

	if (con->smtp.out_prefs) {
		smtp_free_outbound(con->smtp.out_prefs);
	}

	return;
}

// HIGH: Wtf is this?
// Check whether the usernum on the inbound structure has already been used.
/**
 *
 */
bool_t smtp_check_duplicate_recipient(connection_t *con, uint64_t usernum) {

	int_t result = 0;
	smtp_inbound_prefs_t *holder;

	if (con || usernum || !(holder = con->smtp.in_prefs)) {
		return false;
	}

	do {

		if (holder->usernum == usernum) {
			result = true;
		}

	} while (!result && (holder = (smtp_inbound_prefs_t *)holder->next));

	return result;
}

/**
 * @brief	Add an entry to an SMTP session's recipients list for outbound/relayed mail.
 * @note	If the recipients list does not exist, it will be created automatically.
 * @param	con		the SMTP client connection specifying the added recipient.
 * @param	address	a managed string containing the recipient's email address.
 * @return	true if the requested recipient was added successfully to the recipients list, or false on failure.
 */
bool_t smtp_add_recipient(connection_t *con, stringer_t *address) {

	smtp_recipients_t *append;
	smtp_recipients_t *holder;

	if (!con->smtp.out_prefs) {
		log_pedantic("A NULL outbound preferences pointer found.");
		return false;
	}
	else if (!(append = mm_alloc(sizeof(smtp_recipients_t)))) {
		log_pedantic("Unable to allocate %zu bytes.", sizeof(smtp_recipients_t));
		return false;
	}
	else if (!(append->address = st_dupe(address))) {
		log_pedantic("Unable to duplicate the address.");
		mm_free(append);
		return false;
	}

	if (!con->smtp.out_prefs->recipients) {
		con->smtp.out_prefs->recipients = append;
	}
	else {
		holder = con->smtp.out_prefs->recipients;

		while (holder->next) {
			holder = (smtp_recipients_t *)holder->next;
		}

		holder->next = (struct smtp_recipients_t *)append;
	}

	con->smtp.num_recipients++;

	return true;
}

/**
 * @brief	Add an entry to an SMTP session's inbound preferences list for local delivery.
 * @param	con			the SMTP client connection specifying the added local recipient.
 * @param	inbound		a pointer to the SMTP inbound preferences object to be added to the SMTP session's inbound preferences list.
 * @return	true if the requested inbound preferences object was added successfully to the inbound preferences list, or false on failure.
 */
void smtp_add_inbound(connection_t *con, smtp_inbound_prefs_t *inbound) {

	smtp_inbound_prefs_t *holder;

	// Figure out where to attach the structure.
	if (!con->smtp.in_prefs) {
		con->smtp.in_prefs = inbound;
	}
	else {
		holder = con->smtp.in_prefs;

 		while (holder->next) {
			holder = (smtp_inbound_prefs_t *)holder->next;
		}

		holder->next = (struct smtp_inbound_prefs_t *)inbound;
	}

	con->smtp.num_recipients++;

	return;
}

/**
 * @brief	Attach a set of SMTP outbound mail preferences to an SMTP client connection.
 * @param	con			the SMTP client connection to which the outbound mail preferences should be attached.
 * @param	outbound	a pointer to the SMTP outbound mail preferences to be set for the specified connection.
 * @return	This function returns no value.
 */
void smtp_add_outbound(connection_t *con, smtp_outbound_prefs_t *outbound) {

	con->smtp.out_prefs = outbound;
	return;
}

// TODO: This appears to be ancient code. Seems like an inx * would do better.
/**
 * @brief	Free a list of SMTP recipients and its underlying data.
 * @param	recipients	a pointer to the head of the recipients list to be destroyed.
 * @return	This function returns no value.
 */
void smtp_free_recipients(smtp_recipients_t *recipients) {

	smtp_recipients_t *holder;

	while (recipients) {
		st_cleanup(recipients->address);
		holder = recipients;
		recipients = (smtp_recipients_t *)holder->next;
		mm_free(holder);
	}

	return;
}

/**
 * @brief	Free a set of SMTP outbound mail preferences and its underlying data.
 * @param	outbound	a pointer to the SMTP outbound mail preferences set to be destroyed.
 * @return	This function returns no value.
 */
void smtp_free_outbound(smtp_outbound_prefs_t *outbound) {

	st_cleanup(outbound->domain);
	
	if (outbound->recipients) {
		smtp_free_recipients(outbound->recipients);
	}

	mm_free(outbound);

	return;
}

/**
 * @brief	Free a list of SMTP inbound mail preferences and its underlying data.
 * @param	inbound		a pointer to the head of the SMTP inbound mail preferences list to be destroyed.
 * @return	This function returns no value.
 */
void smtp_free_inbound(smtp_inbound_prefs_t *inbound) {

	smtp_inbound_prefs_t *holder;

	while (inbound) {
		st_cleanup(inbound->rcptto);
		st_cleanup(inbound->address);
		st_cleanup(inbound->domain);
		st_cleanup(inbound->forwarded);
		st_cleanup(inbound->spamsig);
		inx_cleanup(inbound->filters);
		holder = inbound;
		inbound = (smtp_inbound_prefs_t *)holder->next;
		mm_free(holder);
	}

	return;
}

/**
 * @brief	Free an SMTP inbound filter and its underlying data.
 * @param	filter		a pointer to the SMTP inbound filter to be destroyed.
 * @return	This function returns no value.
 */
void smtp_list_free_filter(smtp_inbound_filter_t *filter) {

	st_cleanup(filter->label);
	st_cleanup(filter->field);
	st_cleanup(filter->expression);
	mm_free(filter);

	return;
}

