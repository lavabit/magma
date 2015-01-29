
/**
 * @file /magma/web/contact/abuse.c
 *
 * @brief	Functions for detecting abuse of the contact form.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Increment the contact abuse history counter for an IP address.
 * @param	con		a pointer to the connection object of the remote host making the contact request.
 * @return	This function returns no value.
 */
void contact_abuse_increment_history(connection_t *con) {

	stringer_t *key = MANAGEDBUF(64), *ip = NULL;

	// Build the key.
	if (!(ip = con_addr_presentation(con, ip)) && st_sprint(key, "magma.web.contact.history.%.*s", st_length_int(ip), st_char_get(ip)) > 0) {
		cache_increment(key, 1, 1, 86400);
	}

	return;
}

/**
 * @brief	Check to see that a client from a given IP address hasn't exceeded its daily quota of contact requests.
 * @note	Each IP address will be limited to at most 2 contact requests in any 24-hour period.
 * @param	con		a pointer to the connection object of the remote host making the contact request.
 * @param	branch	a null-terminated string specifying where the contact request was directed ("Abuse" or "Contact").
 * @return	true if the specified connection failed the abuse check or false if it did not.
 */
bool_t contact_abuse_checks(connection_t *con, chr_t *branch) {

	stringer_t *key = MANAGEDBUF(64), *ip = NULL;

	// Build the key.
	if (!(ip = con_addr_presentation(con, ip)) && st_sprint(key, "magma.web.contact.history.%.*s", st_length_int(ip), st_char_get(ip)) > 0 && cache_get_u64(key) >= 2) {
		contact_print_message(con, branch, "To prevent abuse, our system only allows you to submit two letters to our team in a twenty-four hour period. If you need "
			"to submit another message please return in twenty-four hours and try again");
		return true;
	}

	return false;
}
