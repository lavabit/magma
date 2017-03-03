
/**
 * @file /magma/servers/smtp/checkers.c
 *
 * @brief Functions used by the SMTP protocol to check and if necessary validate data using external information.
 */

#include "magma.h"

/**
 * @brief	Check to see if a transmitting address is in a user's greylist.
 * @note	The greylist is configured in the Dispatch table and specifies the minimum time, in minutes, that a transmitting smtp
 * 			relay server must wait in order to be able to send more messages to the same recipient address again.
 * @param	con		the connection to have its remote address checked against the user's greylist.
 * @param	prefs	the smtp inbound preferences of the user
 * @return 	-1 on an internal server error, 0 if the sender must wait longer, and 1 if the check was passed.
 */
int_t smtp_check_greylist(connection_t *con, smtp_inbound_prefs_t *prefs) {

	int_t result = 0;
	uint64_t now, stamp, updated;
	stringer_t *value = NULL, *addr = MANAGEDBUF(128), *key = MANAGEDBUF(256);

	// Being on the bypass list skips all of this
	if (con->smtp.bypass) {
		return 1;
	}

	// Store the time.
	now = time(NULL);

	if (!(addr = con_addr_reversed(con, addr))) {
		log_pedantic("Address string creation failed.");
		return -1;
	}

	// The key is the usernum, plus the IP address.
	if (st_sprint(key, "magma.greylist.%lu.%.*s", prefs->usernum, st_length_int(addr), st_char_get(addr)) <= 0) {
		log_pedantic("Unable to build greylist cache key.");
		return -1;
	}

	// Pull the cache value and see if its old enough.
	if ((value = cache_get(key)) && st_length_get(value) == 16) {
		stamp = *((uint64_t *)st_data_get(value));
		updated = *(((uint64_t *)st_data_get(value)) + 1);

		// If the different between now and the stamp is converted to minutes and greater than greytime, let the message through.
		if (((now - stamp) / 60) > prefs->greytime) {
			result = 1;
		}

		// If the updated time is more than 1 day old, set the value in cache again so it won't be expired.
		if ((now - 86400) > updated) {
			*(((uint64_t *)st_data_get(value)) + 1) = now;
			cache_set(key, value, 2592000);
		}
	}

	// If no value was found in the database, store the current time. Errors result in a neutral return code.
	else if (!(value = st_alloc_opts(BLOCK_T | CONTIGUOUS | HEAP, 16)) || !(*((uint64_t *)st_data_get(value)) = now) ||
		!(*(((uint64_t *)st_data_get(value)) + 1) = now) || cache_set(key, value, 2592000) != 1) {
		log_pedantic("Unable to set greylist attempt record.");
		result = -1;
	}

	st_cleanup(value);
	return result;
}

/**
 * @brief	Check the SMTP connection's remote address against a collection of real-time blacklists.
 * @note	The connection's IP address will be checked against each of the servers configured in magma.smtp.blacklists.domain.
 * @param	con		the connection to have its address examined against the RBLs.
 * @return	-1 on general error, -2 if the address was blacklisted, or 1 if it passed the check.
 */
int_t smtp_check_rbl(connection_t *con) {

	int_t ret, result = -1;
	chr_t query[NI_MAXHOST];
	struct addrinfo *lookup, hints;
	stringer_t *addr = MANAGEDBUF(128);

	mm_wipe(&hints, sizeof(struct addrinfo));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = 0;
	hints.ai_flags = 0;
	hints.ai_protocol = 0;

	if (!(addr = con_addr_reversed(con, addr))) {
		log_pedantic("Address string creation failed.");
		return result;
	}

	for (uint32_t i = 0; i < magma.smtp.blacklists.count; i++) {

		// Build the DNS query.
		if ((snprintf(query, NI_MAXHOST, "%.*s.%*s", st_length_int(addr), st_char_get(addr),
			st_length_int(magma.smtp.blacklists.domain[i]),	st_char_get(magma.smtp.blacklists.domain[i]))) == 0) {
				log_pedantic("Address string creation failed.");
		}

		// A zero means we found a blacklist entry, so we need to free the entry.
		else if (!(ret = getaddrinfo(query, NULL, &hints, &lookup))) {
			freeaddrinfo(lookup);
			return -2;
		}
		// If the return code code indicates an error, rather than simply not finding a matching entry.
		// Do we also want to exclude EAI_ADDRFAMILY or EAI_NODATA
		else if (ret != EAI_NONAME) {
			log_pedantic("Blacklist DNS attempt resulted in an error. { getaddrinfo = %s }", gai_strerror(ret));
		}
		else {
			result = 1;
		}

	}

	return result;
}

/**
 * Apply any user specific filters. Return -1 for errors, and -2 to delete a message. Return 1 if no action was taken, and 2 if the message was
 * moved to a different folder, 3 if the message content was modified, and 4 if the message was marked read.
 */
int_t smtp_check_filters(smtp_inbound_prefs_t *prefs, stringer_t **local) {

	placer_t data;
	size_t length;
	stringer_t *field;
	inx_cursor_t *cursor;
	int_t match = -1, result = 0;
	struct re_pattern_buffer regbuff;
	smtp_inbound_filter_t *filter = NULL;

	if (!prefs || !prefs->filters || !(cursor = inx_cursor_alloc(prefs->filters))) {
		return -1;
	}

	while (match == -1 && (filter = inx_cursor_value_next(cursor))) {

		field = NULL;
		data = pl_null();
		mm_wipe(&regbuff, sizeof(struct re_pattern_buffer));

		// Use regcomp, so we can use the case insensitive flag.
		if (regcomp(&regbuff, st_char_get(filter->expression), REG_ICASE) != 0) {
			log_pedantic("Regular expression compilation failed. {user = %lu / rule = %lu / expression = %.*s}",
				prefs->usernum, filter->rulenum, st_length_int(filter->expression), st_char_get(filter->expression));
			inx_cursor_free(cursor);
			return -1;
		}

		// Set the start position beginning, and the length to the end of the header.
		if ((filter->location & SMTP_FILTER_LOCATION_HEADER) == SMTP_FILTER_LOCATION_HEADER) {
			data = pl_init(st_char_get(*local), mail_header_end(*local));
		}
		// Set the start to end of the header, and the length of the body.
		else if ((filter->location & SMTP_FILTER_LOCATION_BODY) == SMTP_FILTER_LOCATION_BODY) {
			length = mail_header_end(*local);
			data = pl_init(st_char_get(*local) + length, st_length_get(*local) - length);
		}
		// Pull a specific field.
		else if ((filter->location & SMTP_FILTER_LOCATION_FIELD) == SMTP_FILTER_LOCATION_FIELD && filter->field) {
			length = mail_header_end(*local);

			if ((field = mail_header_fetch_all(PLACER(st_char_get(*local), length), filter->field)) != NULL) {

				// If the field is the subject, modify it first.
				if (!st_cmp_ci_eq(filter->field, PLACER("Subject", 7)) && prefs->mark != SMTP_MARK_NONE) {

					if ((prefs->mark & SMTP_MARK_VIRUS) == SMTP_MARK_VIRUS) {
						mail_mod_subject(&field, "INFECTED:");
					}
					else if ((prefs->mark & SMTP_MARK_PHISH) == SMTP_MARK_PHISH) {
						mail_mod_subject(&field, "PHISHING:");
					}
					else if ((prefs->mark & SMTP_MARK_SPOOF) == SMTP_MARK_SPOOF) {
						mail_mod_subject(&field, "SPOOFED:");
					}
					else if ((prefs->mark & SMTP_MARK_RBL) == SMTP_MARK_RBL) {
						mail_mod_subject(&field, "BLACKHOLED:");
					}
					else if ((prefs->mark & SMTP_MARK_SPAM) == SMTP_MARK_SPAM) {
						mail_mod_subject(&field, "JUNK:");
					}

				}

				data = pl_init(st_char_get(field), st_length_get(field));
			}

		}
		// Establish what were looking for. The entire body.
		else if ((filter->location & SMTP_FILTER_LOCATION_ENTIRE) == SMTP_FILTER_LOCATION_ENTIRE) {
			data = pl_init(st_char_get(*local), st_length_get(*local));
		}
		else {
			log_pedantic("Unrecognized location %i.", filter->location);
			inx_cursor_free(cursor);
			return -1;
		}

		// Use the re_search function because it allows us to specify length.
		match = re_search(&regbuff, pl_data_get(data), pl_length_get(data), 0, pl_length_get(data), NULL);

		// What do we do with matches? Move it to a folder.
		if (match != -1 && (filter->action & SMTP_FILTER_ACTION_MOVE) == SMTP_FILTER_ACTION_MOVE && filter->foldernum != 0) {
			prefs->foldernum = filter->foldernum;
			result = 2;
		}
		// Label the subject.
		else if (match != -1 && (filter->action & SMTP_FILTER_ACTION_LABEL) == SMTP_FILTER_ACTION_LABEL && filter->label != NULL) {
			mail_mod_subject(local, st_char_get(filter->label));
			result = 3;
		}

		// Mark it read.
		if (match != -1 && (filter->action & SMTP_FILTER_ACTION_MARK_READ) == SMTP_FILTER_ACTION_MARK_READ) {
			prefs->mark += SMTP_MARK_READ;
			result = 4;
		}

		// Cleanup
		regfree(&regbuff);
		st_cleanup(field);
	}

	// Detect deletes and return a -2 to trigger the action.
	if (match != -1 && filter && (filter->action & SMTP_FILTER_ACTION_DELETE) == SMTP_FILTER_ACTION_DELETE) {
		result = -2;
	}
	else if (result == 0) {
		result = 1;
	}

	inx_cursor_free(cursor);

	return result;
}

/**
 * @brief	Add an entry to the SMTP subnet bypass list.
 * @param	subnet	a pointer to a managed string containing the IP address or subnet address to be bypassed for checks.
 * @return	true if the entry was valid and was added, or false on failure.
 */
bool_t smtp_add_bypass_entry(stringer_t *subnet) {

	subnet_t *sn;
	multi_t key;
	static size_t kval = 1;

	if (!magma.smtp.bypass_subnets && !(magma.smtp.bypass_subnets = inx_alloc(M_INX_LINKED, &mm_free))) {
		log_pedantic("Could not allocate space for smtp bypass list.");
		return false;
	}

	if (!(sn = mm_alloc(sizeof(subnet_t)))) {
		log_error("Could not allocate space for smtp bypass entry.");
		return false;
	}

	if (!ip_str_subnet(st_char_get(subnet), sn)) {
		log_pedantic("SMTP bypass subnet was invalid { subnet = %s }", st_char_get(subnet));
		mm_free(sn);
		return false;
	}

	key.type = M_TYPE_UINT64;
	key.val.u64 = kval++;

	if (!inx_insert(magma.smtp.bypass_subnets, key, sn)) {
		log_pedantic("Unable to create smtp bypass entry.");
		mm_free(sn);
		return false;
	}

	return true;
}

/**
 * @brief	Check if a connection should bypass certain SMTP checks.
 * @note	This check is run against host and/or subnet masks configured in the magma.smtp.bypass_addr option.
 * @param	con		a pointer to the connection object to be checked.
 * @return	true if the specified connection meets the SMTP bypass check or false on failure or if it does not.
 */
bool_t smtp_bypass_check(connection_t *con) {

	ip_t remote;
	bool_t result = false;
	subnet_t *subnet = NULL;
	inx_cursor_t *cursor = NULL;

	if (!magma.smtp.bypass_subnets || !con_addr(con, &remote)) {
		return false;
	}
	else if (!(cursor = inx_cursor_alloc(magma.smtp.bypass_subnets))) {
		return false;
	}

	while (!result && (subnet = inx_cursor_value_next(cursor))) {
		if (ip_matches_subnet(subnet, &remote)) result = true;
	}

	inx_cursor_free(cursor);
	return result;
}
