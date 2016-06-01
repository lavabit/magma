
/**
 * @file /magma/web/register/datatier.c
 *
 * @brief	Functions for handling the new user registration process.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/// HIGH: The prepared statements being used aren't valid. The queries need to be copied over and created.
/// register_fetch_blocklist still needs to be defined.
/**
 * @brief	Fetch the blocklist for new user registration from the database.
 * @return	an inx holder containing the registration blocklist as a collection of managed strings.
 */
inx_t * register_data_fetch_blocklist(void) {

	row_t *row;
	table_t *result;
	inx_t *blocklist;
	stringer_t *holder;
	multi_t key = { .type = M_TYPE_UINT64, .val.u64 = 1 };

	if (!(result = stmt_get_result(stmts.register_fetch_blocklist, NULL))) {
		return NULL;
	}

	if (!(row = res_row_next(result))) {
		res_table_free(result);
		return NULL;
	}

	if (!(blocklist = inx_alloc(M_INX_LINKED, &st_free))) {
		log_pedantic("Unable to allocate memory for the registration engine blocklist.");
		res_table_free(result);
		return NULL;
	}

	while (row && (holder = res_field_string(row, 0))) {

		if (inx_insert(blocklist, key, holder) != 1) {
			log_pedantic("Unable to add the sequence stringer to the linked list.");
			st_free(holder);
			res_table_free(result);
			inx_free(blocklist);
			return NULL;
		}

		row = res_row_next(result);
		key.val.u64++;
	}

	res_table_free(result);

	return blocklist;
}

/**
 * @brief	Check to see if a username requested by a registration attempt has already been taken.
 * @param	username	the username to be checked against the database.
 * @return	true if the username is taken or false if it is not.
 */
bool_t register_data_check_username(stringer_t *username) {

	row_t *row;
	table_t *result;
	MYSQL_BIND parameters[1];

	mm_wipe(parameters, sizeof(parameters));

	// The username.
	parameters[0].buffer_type = MYSQL_TYPE_STRING;
	parameters[0].buffer = (chr_t *)st_char_get(username);
	parameters[0].buffer_length = st_length_get(username);

	if (!(result = stmt_get_result(stmts.register_check_username, parameters))) {
		return false;
	}

	// The username is taken.
	if ((row = res_row_next(result))) {
		res_table_free(result);
		return true;
	}

	res_table_free(result);

	return false;
}

/**
 * @brief	Insert a newly registered user into the database using information gathered by registration step #2.
 * @note	The following steps occur:
 * 			1. Insert a new user into the Users table, supplying username, hashed password, plan info, and quota.
 * 			2. Insert a blank entry into the Profile table for the user.
 * 			3. Insert an entry into the Folders table for the user's "Inbox" folder.
 *			4. Insert a new entry into the Log table containing the usernum and IP address of the client request.
 *			5. Insert a new entry into the Dispatch table for the user, configuring the spam folder, inbox, send/receive/daily send/daily receive limits, etc.
 *			6. Insert a new entry into the Mailboxes table for the user.
 * @param	con			a pointer to the connection object of the client making the registration request.
 * @param	reg			the current registration session of the user to be added.
 * @param	transaction	a mysql transaction id for all database operations, since they all need to be committed atomically or rolled back.
 * @param	outuser		a pointer to a numerical id to receive the newly generated and inserted user id.
 * @result	true if the new user account was successfully created, or false on failure.
 */
bool_t register_data_insert_user(connection_t *con, uint16_t plan, stringer_t *username, stringer_t *password, int64_t transaction, uint64_t *outuser) {

	chr_t buffer[32];
	uint32_t bonus = 0;
	MYSQL_BIND parameters[8];
	credential_t *credential;
	size_t key_len = 512;
	stringer_t *privkey, *pubkey, *newaddr, *salt, *hex_salt;
	const chr_t *basic = "BASIC", *personal = "PERSONAL", *enhanced = "ENHANCED", *premium = "PREMIUM";
	uint64_t name_len, plan_len, date_len = 10, quota = 0, usernum, inbox, size_limit, send_limit, recv_limit;

	mm_wipe(parameters, sizeof(parameters));

	// The username.
	name_len = st_length_get(username);
	parameters[0].buffer_type = MYSQL_TYPE_STRING;
	parameters[0].buffer = (chr_t *)st_char_get(username);
	parameters[0].length = &name_len;

	// The plan.
	if (plan == 1) {
		plan_len = 5;
		parameters[4].buffer_type = MYSQL_TYPE_STRING;
		parameters[4].buffer = (chr_t *)basic;
		parameters[4].length = &plan_len;
	}
	else if (plan == 2) {
		plan_len = 8;
		parameters[4].buffer_type = MYSQL_TYPE_STRING;
		parameters[4].buffer = (chr_t *)personal;
		parameters[4].length = &plan_len;
	}
	else if (plan == 3) {
		plan_len = 8;
		parameters[4].buffer_type = MYSQL_TYPE_STRING;
		parameters[4].buffer = (chr_t *)enhanced;
		parameters[4].length = &plan_len;
	}
	else if (plan == 4) {
		plan_len = 7;
		parameters[4].buffer_type = MYSQL_TYPE_STRING;
		parameters[4].buffer = (chr_t *)premium;
		parameters[4].length = &plan_len;
	}
	else {
		log_pedantic("Invalid plan number specified.");
		return false;
	}

	// The quota.
	if (plan == 1) {
		quota = 134217728ll; // 128 MB
	}
	else if (plan == 2) {
		quota = 1073741824ll; // 1,024 MB
	}
	else if (plan == 3) {
		quota = 1073741824ll; // 1,024 MB
	}
	else if (plan == 4) {
		quota = 8589934592ll; // 8,192 MB
	}

	parameters[5].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[5].buffer_length = sizeof(uint64_t);
	parameters[5].buffer = &quota;
	parameters[5].is_unsigned = true;

	// Calculate the date.
	// Removed code for paid plan time calculation.
	// The default string for non paid plans.
	if (snprintf(buffer, 32, "0000-00-00") != 10) {
		log_pedantic("Unable to build the date string.");
		return false;
	}

	parameters[6].buffer_type = MYSQL_TYPE_STRING;
	parameters[6].buffer = &buffer;
	parameters[6].length = &date_len;

	// Hash the password.
	if(!(credential = credential_alloc_auth(username))) {
		log_error("Failed to allocate credentials structure.");
		return false;
	}

	if(!(salt = stacie_salt_create())) {
		log_error("Failed to generate a new user salt.");
		credential_free(credential);
		return false;
	}

	if(!credential_calc_auth(credential, password, salt)) {
		log_error("Failed to calculate user credentials.");
		credential_free(credential);
		st_free(salt);
		return false;
	}

	hex_salt = hex_encode_st(salt, NULL);
	st_free(salt);

	if(!hex_salt) {
		log_error("Failed to hex encode the user salt.");
		credential_free(credential);
		return false;
	}

	// The user specific salt value.
	parameters[1].buffer_type = MYSQL_TYPE_STRING;
	parameters[1].buffer = st_data_get(hex_salt);
	parameters[1].buffer_length = st_length_get(hex_salt);


	// The authentication token derived from the password.
	parameters[2].buffer_type = MYSQL_TYPE_STRING;
	parameters[2].buffer = st_data_get(credential->auth.password);
	parameters[2].buffer_length = st_length_get(credential->auth.password);

	// The number of bonus hash rounds.
	parameters[3].buffer_type = MYSQL_TYPE_LONG;
	parameters[3].buffer_length = sizeof(uint32_t);
	parameters[3].buffer = &bonus;
	parameters[3].is_unsigned = true;

	// Insert the user.
	if ((usernum = stmt_insert_conn(stmts.register_insert_stacie_user, parameters, transaction)) == 0) {
		log_pedantic("Unable to insert the user into the database. (Failed on User table.)");
		credential_free(credential);
		return false;
	}

	st_free(hex_salt);

	// Create a pair of storage keys for the new user.
	if ((!(privkey = st_alloc_opts(MANAGED_T | CONTIGUOUS | SECURE, key_len))) || (!(pubkey = st_alloc_opts(MANAGED_T | CONTIGUOUS | SECURE, key_len)))) {
		log_pedantic("Unable to allocate storage for ECIES keys.");
		credential_free(credential);
		st_cleanup(privkey);
		return false;
	} else if (meta_data_user_build_storage_keys(usernum, credential->auth.key, &privkey, &pubkey, false, true, transaction) < 0) {
		log_pedantic("Unable to generate storage keys for the new user.");
		credential_free(credential);
		st_free(pubkey);
		st_free(privkey);
		return false;
	}

	credential_free(credential);
	st_free(pubkey);
	st_free(privkey);

	mm_wipe(parameters,sizeof(parameters));

	// Usernum
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &usernum;
	parameters[0].is_unsigned = true;

	// Profile table.
	if (!stmt_exec_conn(stmts.register_insert_profile, parameters, transaction)) {
		log_pedantic("Unable to insert the user into the database. (Failed on Profile table.)");
		return false;
	}

	// Folders table.

	// Add the folder name.
	parameters[1].buffer_type = MYSQL_TYPE_STRING;
	parameters[1].buffer = "Inbox";
	name_len = ns_length_get(parameters[1].buffer);
	parameters[1].length = &name_len;

	if ((inbox = stmt_insert_conn(stmts.register_insert_folder_name, parameters, transaction)) == 0) {
		log_pedantic("Unable to insert the user into the database. (Failed on Folders table.)");
		return false;
	}

	// IP
	name_len = snprintf(buffer, 32, "%s", st_char_get(con_addr_presentation(con, MANAGEDBUF(64))));
	parameters[1].buffer_type = MYSQL_TYPE_STRING;
	parameters[1].buffer = &buffer;
	parameters[1].length = &name_len;

	// Log table.
	if (!stmt_exec_conn(stmts.register_insert_log, parameters, transaction)) {
		log_pedantic("Unable to insert the user into the database. (Failed on Log table.)");
		return false;
	}

	mm_wipe(parameters, sizeof(parameters));

	// Usernum
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &usernum;
	parameters[0].is_unsigned = true;

	// Spam Folder.
	parameters[1].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[1].buffer_length = sizeof(uint64_t);
	parameters[1].buffer = &inbox;
	parameters[1].is_unsigned = true;

	// Inbox
	parameters[2].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[2].buffer_length = sizeof(uint64_t);
	parameters[2].buffer = &inbox;
	parameters[2].is_unsigned = true;

	if (plan == 1) {
		size_limit = 33554432;
		recv_limit = 1024;
		send_limit = 256;
	}
	else if (plan) {
		size_limit = 64LL << 20;
		recv_limit = 1024;
		send_limit = 256;
	}
	else if (plan == 3) {
		size_limit = 64LL << 20;
		recv_limit = 1024;
		send_limit = 512;
	}
	else if (plan == 4) {
		size_limit = 128LL << 20;
		recv_limit = 8192;
		send_limit = 768;
	}

	// Send size limit.
	parameters[3].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[3].buffer_length = sizeof(uint64_t);
	parameters[3].buffer = (chr_t *)&size_limit;
	parameters[3].is_unsigned = true;

	// Receive size limit.
	parameters[4].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[4].buffer_length = sizeof(uint64_t);
	parameters[4].buffer = (chr_t *)&size_limit;
	parameters[4].is_unsigned = true;

	// Daily send limit.
	parameters[5].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[5].buffer_length = sizeof(uint64_t);
	parameters[5].buffer = (chr_t *)&send_limit;
	parameters[5].is_unsigned = true;

	// Daily receive limit.
	parameters[6].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[6].buffer_length = sizeof(uint64_t);
	parameters[6].buffer = (chr_t *)&recv_limit;
	parameters[6].is_unsigned = true;

	// Daily receive limit, IP.
	parameters[7].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[7].buffer_length = sizeof(uint64_t);
	parameters[7].buffer = (chr_t *)&recv_limit;
	parameters[7].is_unsigned = true;

	// Dispatch table.
	if (!stmt_exec_conn(stmts.register_insert_dispatch, parameters, transaction)) {
		log_pedantic("Unable to insert the user into the database. (Failed on Dispatch table.)");
		return false;
	}

	if (!(newaddr = st_merge("sns", username, "@", magma.system.domain))) {
		log_pedantic("Unable to generate an email address for the new user.");
		return false;
	}

	mm_wipe(parameters, sizeof(parameters));

	// The username.
	name_len = st_length_get(newaddr);
	parameters[0].buffer_type = MYSQL_TYPE_STRING;
	parameters[0].buffer = (chr_t *)st_char_get(newaddr);
	parameters[0].length = &name_len;

	// Usernum
	parameters[1].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[1].buffer_length = sizeof(uint64_t);
	parameters[1].buffer = &usernum;
	parameters[1].is_unsigned = true;

	// The mailboxes table.
	if (!stmt_exec_conn(stmts.register_insert_mailboxes, parameters, transaction)) {
		log_pedantic("Unable to insert the user into the database. (Failed on Mailboxes table.)");
		st_free(newaddr);
		return false;
	}

	st_free(newaddr);
	*outuser = usernum;

	return true;
}
