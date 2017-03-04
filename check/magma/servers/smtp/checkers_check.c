/**
 * @file /check/magma/servers/smtp/checkers_check.c
 *
 * @brief SMTP checkers test functions.
 */

#include "magma_check.h"

bool_t check_smtp_checkers_greylist_sthread(stringer_t *errmsg) {

	uint64_t now;
	connection_t con;
	smtp_inbound_prefs_t prefs;
	stringer_t *value = NULL, *addr = MANAGEDBUF(128), *key = MANAGEDBUF(256);

	mm_wipe(&con, sizeof(connection_t));
	mm_wipe(&prefs, sizeof(smtp_inbound_prefs_t));

	con.smtp.bypass = true;
	con.smtp.authenticated = true;
	con.smtp.mailfrom = NULLER("check@example.com");

	prefs.usernum = 1;
	prefs.greytime = 1;

	if (!(addr = con_addr_reversed(&con, addr)) ||
		st_sprint(key, "magma.greylist.%lu.%.*s", prefs.usernum, st_length_int(addr), st_char_get(addr)) <= 0) {
		st_sprint(errmsg, "The SMTP check greylist check failed to create a valid lookup key.");
		return false;
	}

	// Delete the greylist key from the cache to ensure the test can be run in any order.
	cache_delete(key);

	if (smtp_check_greylist(&con, &prefs) != 1) {
		st_sprint(errmsg, "The SMTP check greylist function failed to return 1 when bypass is enabled.");
		return false;
	}

	else if ((con.smtp.bypass = false) || smtp_check_greylist(&con, &prefs) != 0) {
		st_sprint(errmsg, "The SMTP check greylist function failed to return 0 after the initial try.");
		return false;
	}

	else if (smtp_check_greylist(&con, &prefs) != 0) {
		st_sprint(errmsg, "The SMTP check greylist function failed to return 0 when resubmitted too fast.");
		return false;
	}

	// Manually set the timestamp value in the cache (steps taken from the smtp_check_greylist function).
	else if (!(now = time(NULL) - 100) || !(value = cache_get(key)) || !(*(((uint64_t *)st_data_get(value)) + 1) = now) ||
		(cache_set(key, value, 2592000) != 1)) {
		st_sprint(errmsg, "Failed while attempting to update the cached timestamp.");
		st_cleanup(value);
		return false;
	}

	else if (smtp_check_greylist(&con, &prefs) != 0) {
		st_sprint(errmsg, "The SMTP check greylist function failed to return 1 when the last timestamp is old enough.");
		st_free(value);
		return false;
	}

	st_free(value);

	return true;
}
