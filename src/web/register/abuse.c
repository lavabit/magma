
/**
 * @file /magma/web/register/abuse.c
 *
 * @brief	Functions for handling potential abuse of the new user registration process.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

inx_t *register_blocklist = NULL;
pthread_rwlock_t register_blocklist_lock = PTHREAD_RWLOCK_INITIALIZER;


/**
 * @brief	Free a registration blocked list.
 * @return	This function returns no value.
 */
void register_blocklist_free(void) {

	if (register_blocklist) {
		inx_free(register_blocklist);
		register_blocklist = NULL;
	}

	return;
}

/**
 * @brief	Update the registration blocklist from the database.
 * @return	This function returns no value.
 */
void register_blocklist_update(void) {

	inx_t *holder;

	rwlock_lock_write(&register_blocklist_lock);

	// Fetch the blocklist.
	holder = register_data_fetch_blocklist();

	// Free an existing list.
	if (register_blocklist) {
		inx_free(register_blocklist);
	}

	// Update
	register_blocklist = holder;
	rwlock_unlock(&register_blocklist_lock);

	return;
}

/**
 * @brief	Check to see if the remote client is on the registration blocklist; if so, increment the web registration blocked counter.
 * @param	con		the client connection to be checked.
 * @return	false if the check was passed, or true if the connection was made from an IP on the blocklist.
 */
bool_t register_abuse_check_blocklist(connection_t *con) {

	int ret = false;
	inx_cursor_t *cursor;
	stringer_t *pattern, *address = con_addr_standard(con, MANAGEDBUF(128));

	rwlock_lock_read(&register_blocklist_lock);

	// Loop through the blocklist comparing the sequences.
	if ((cursor = inx_cursor_alloc(register_blocklist))) {

		while (!ret && (pattern = inx_cursor_value_next((cursor)))) {

			// QUESTION: first st_length_get() could read out of bounds?
			if (!st_cmp_ci_starts(address, pattern) && (*(st_char_get(address) + st_length_get(pattern)) == '.' || st_length_get(address) == st_length_get(pattern))) {
				ret = true;
			}

		}

		inx_cursor_free(cursor);
	}

	rwlock_unlock(&register_blocklist_lock);

	if (ret) {
		stats_increment_by_name("web.register.blocked");
	}

	return ret;
}

/**
 * @brief	Increment the registration abuse counter for the requesting IP address.
 * @param	con		a pointer to the connection object of the client making the registration request.
 * @return	This function returns nov alue.
 */
void register_abuse_increment_history(connection_t *con) {

	chr_t key[128];

	// Build the key.
	snprintf(key, 64, "magma.web.register.history.%s", st_char_get(con_addr_standard(con, MANAGEDBUF(128))));
	cache_increment(NULLER(key), 1, 1, 604800);

	return;
}

/**
 * @brief	Check to see if a registration request is allowed by a remote host; if not, display a banner.
 * @param	con		the remote connection to be checked.
 * @return	false if registration is allowed or true if not (remote host is on blocklist or registration has beean throttled).
 */
// HIGH: This checking process is way too simplistic
bool_t register_abuse_checks(connection_t *con) {

	chr_t key[128], message[1024];

	if (register_abuse_check_blocklist(con)) {
		snprintf(message, 1024, "Your IP (%s) has been flagged for past abuses and has been blocked from registering new accounts with our service. If you are not a spammer or "
			"phisher and you haven't had an account locked by us in the past, you can try requesting an account via our <a href=\"https://lavabit.com/contact\">"
			"contact page</a>. Please be sure to tell us the username, password and account plan you want.", st_char_get(con_addr_presentation(con, MANAGEDBUF(64))));
		register_print_message(con, message);
		return true;
	}

	// Build the key.
	snprintf(key, 64, "magma.web.register.history.%s", st_char_get(con_addr_standard(con, MANAGEDBUF(128))));

	if (cache_get_u64(NULLER(key)) >= 2) {
		register_print_message(con, "To prevent abuse, our system only allows two user registrations from the same subnet in a twenty-four hour period. It "
			"appears that there has already been two registrations from your subnet in the past twenty-four hours. If you are not the person responsible "
			"for registering the accounts please access our website from a different location or come back in twenty-four hours.");
		return true;
	}

	return false;
}
