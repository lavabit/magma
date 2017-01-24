
/**
 * @file /magma/web/register/datatier.c
 *
 * @brief	Functions for handling the new user registration process.
 */

#include "magma.h"

/// HIGH: The prepared statements being used aren't valid. The queries need to be copied over and created.
/// register_fetch_blocklist still needs to be defined.

/**
 * @brief	Fetch the blocklist for new user registration from the database.
 *
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
 *
 * @param	username	the username to be checked against the database.
 *
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
 *
 * @note	The following steps occur:
 * 			1. Insert a new user into the Users table, supplying username, hashed password, plan info, and quota.
 * 			2. Insert a random shard value into the User_Realms table.
 * 			3. Insert a blank entry into the Profile table for the user.
 * 			4. Insert an entry into the Folders table for the user's "Inbox" folder.
 *			5. Insert a new entry into the Log table containing the usernum and IP address of the client request.
 *			6. Insert a new entry into the Dispatch table for the user, configuring the spam folder, inbox, send/receive/daily send/daily receive limits, etc.
 *			7. Insert a new entry into the Mailboxes table for the user.
 *
 * @param	con			a pointer to the connection object of the client making the registration request.
 * @param	reg			the current registration session of the user to be added.
 * @param	transaction	a mysql transaction id for all database operations, since they all need to be committed atomically or rolled back.
 * @param	outuser		a pointer to a numerical id to receive the newly generated and inserted user id.
 *
 * @return	true if the new user account was successfully created, or false on failure.
 */
bool_t register_data_insert_user(connection_t *con, uint16_t plan, stringer_t *username, stringer_t *password, int64_t transaction, uint64_t *outuser) {

	uint16_t serial = 0;
	uint32_t bonus = 0;
	MYSQL_BIND parameters[8];
	auth_stacie_t *stacie = NULL;
	const chr_t *account_plans[] = { "BASIC", "PERSONAL", "ENHANCED", "PREMIUM" };
	uint64_t name_len, quota = 0, usernum, inbox, size_limit, send_limit, recv_limit;
	stringer_t *newaddr = NULL, *salt = NULL, *shard = NULL, *b64_salt = NULL, *b64_shard = NULL, *b64_verification = NULL, *ip = NULL;

	/// LOW: This function should be passed a number of days (or years) to use for any of the pre-paid account plans.
	/// LOW: The IP address could be passed in as an ip_t or as a string to avoid the need for the entire connection object.

	// Configure the limits for the plan. We are currently using hard coded values. In the future this may be
	// setup dynamically by pull the default values out of the limits table.
	if (plan == 1) {
		quota = 134217728UL; // 128 MB
		size_limit = 33554432;
		recv_limit = 1024;
		send_limit = 256;
	}
	else if (plan == 2) {
		quota = 1073741824UL; // 1,024 MB
		size_limit = 64UL << 20;
		recv_limit = 1024;
		send_limit = 256;
	}
	else if (plan == 3) {
		quota = 1073741824UL; // 1,024 MB
		size_limit = 64UL << 20;
		recv_limit = 1024;
		send_limit = 512;
	}
	else if (plan == 4) {
		quota = 8589934592UL; // 8,192 MB
		size_limit = 128UL << 20;
		recv_limit = 8192;
		send_limit = 768;
	}
	else {
		log_pedantic("Unrecognized account plan. Only values between 1 and 4 are supported. { plan = %hu }", plan);
		return false;
	}

	// We start by generating all of the values we'll need to complete the registration process. The STACIE values are first.
	if (!(salt = stacie_salt_create(NULL)) || !(b64_salt = base64_encode_mod(salt, NULL)) ||
		!(shard = stacie_shard_create(NULL)) || !(b64_shard = base64_encode_mod(shard, NULL)) ||
		!(stacie = auth_stacie(bonus,  username,  password,  salt,  NULL, NULL)) ||
		!(b64_verification = base64_encode_mod(stacie->tokens.verification, NULL))) {

		log_pedantic("Unable to generate the STACIE authentication values. { username = %.*s }",
			st_length_int(username), st_char_get(username));

		st_cleanup(salt, shard, b64_salt, b64_shard, b64_verification);
		auth_stacie_cleanup(stacie);
		return false;
	}

	// Users Table
	mm_wipe(parameters, sizeof(parameters));

	// The username.
	name_len = st_length_get(username);
	parameters[0].buffer_type = MYSQL_TYPE_STRING;
	parameters[0].buffer = (chr_t *)st_char_get(username);
	parameters[0].length = &name_len;

	// The user specific salt value.
	parameters[1].buffer_type = MYSQL_TYPE_STRING;
	parameters[1].buffer = st_data_get(b64_salt);
	parameters[1].buffer_length = st_length_get(b64_salt);

	// The authentication token derived from the password.
	parameters[2].buffer_type = MYSQL_TYPE_STRING;
	parameters[2].buffer = st_data_get(b64_verification);
	parameters[2].buffer_length = st_length_get(b64_verification);

	// The number of bonus hash rounds.
	parameters[3].buffer_type = MYSQL_TYPE_LONG;
	parameters[3].buffer_length = sizeof(uint32_t);
	parameters[3].buffer = &bonus;
	parameters[3].is_unsigned = true;

	// The name of the account plan.
	parameters[4].buffer_type = MYSQL_TYPE_STRING;
	parameters[4].buffer = &(account_plans[plan - 1]);
	parameters[4].buffer_length = ns_length_get(account_plans[plan - 1]);

	// The quota.
	parameters[5].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[5].buffer_length = sizeof(uint64_t);
	parameters[5].buffer = &quota;
	parameters[5].is_unsigned = true;

	// The expiration date for a pre-paid account plan. Set the all the date to all zeros for a free account plan.
	parameters[6].buffer_type = MYSQL_TYPE_STRING;
	parameters[6].buffer = "0000-00-00";
	parameters[6].buffer_length = 10;

	// Insert the user.
	if ((usernum = stmt_insert_conn(stmts.register_insert_stacie_user, parameters, transaction)) == 0) {
		log_pedantic("Unable to insert the user into the database. (Failed on User table.)");
		st_cleanup(salt, shard, b64_salt, b64_shard, b64_verification);
		auth_stacie_cleanup(stacie);
		return false;
	}

	// User_Realms Table
	mm_wipe(parameters, sizeof(parameters));

	// Usernum
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &usernum;
	parameters[0].is_unsigned = true;

	// Serial
	parameters[1].buffer_type = MYSQL_TYPE_SHORT;
	parameters[1].buffer_length = sizeof(uint16_t);
	parameters[1].buffer = &serial;
	parameters[1].is_unsigned = true;

	// Label
	parameters[2].buffer_type = MYSQL_TYPE_STRING;
	parameters[2].buffer = "mail";
	parameters[2].buffer_length = 4;

	// Shard
	parameters[3].buffer_type = MYSQL_TYPE_STRING;
	parameters[3].buffer = st_data_get(b64_shard);
	parameters[3].buffer_length = st_length_get(b64_shard);

	// User_Realms table.
	if (!stmt_exec_conn(stmts.register_insert_stacie_realms, parameters, transaction)) {
		log_pedantic("Unable to insert the user into the database. (Failed on User_Realms table.)");
		st_cleanup(salt, shard, b64_salt, b64_shard, b64_verification);
		auth_stacie_cleanup(stacie);
		return false;
	}

	// Folders Table
	mm_wipe(parameters, sizeof(parameters));

	// Usernum
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &usernum;
	parameters[0].is_unsigned = true;

	// Profile table.
	if (!stmt_exec_conn(stmts.register_insert_profile, parameters, transaction)) {
		log_pedantic("Unable to insert the user into the database. (Failed on Profile table.)");
		st_cleanup(salt, shard, b64_salt, b64_shard, b64_verification);
		auth_stacie_cleanup(stacie);
		return false;
	}

	// The folder where messages are delivered by default.
	parameters[1].buffer_type = MYSQL_TYPE_STRING;
	parameters[1].buffer = "Inbox";
	parameters[1].buffer_length = 5;

	// Insert the inbox into the folders table and record the folder number for later.
	if ((inbox = stmt_insert_conn(stmts.register_insert_folder_name, parameters, transaction)) == 0) {
		log_pedantic("Unable to insert the user into the database. (Failed on Folders table.)");
		st_cleanup(salt, shard, b64_salt, b64_shard, b64_verification);
		auth_stacie_cleanup(stacie);
		return false;
	}

	// Log Table
	mm_wipe(parameters, sizeof(parameters));

	// Usernum
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &usernum;
	parameters[0].is_unsigned = true;

	// Generate a string representation of the IP address.
	if (!(ip = con_addr_presentation(con, MANAGEDBUF(64)))) {
		log_pedantic("The IP address was invalid. Using an empty address instead.");
		parameters[1].buffer_type = MYSQL_TYPE_STRING;
		parameters[1].buffer = "0.0.0.0";
		parameters[1].buffer_length = 7;
	}
	else {
		parameters[1].buffer_type = MYSQL_TYPE_STRING;
		parameters[1].buffer = st_data_get(ip);
		parameters[1].buffer_length = st_length_get(ip);
	}

	// Insert the data into the Log table.
	if (!stmt_exec_conn(stmts.register_insert_log, parameters, transaction)) {
		log_pedantic("Unable to insert the user into the database. (Failed on Log table.)");
		st_cleanup(salt, shard, b64_salt, b64_shard, b64_verification);
		auth_stacie_cleanup(stacie);
		return false;
	}

	// Dispatch Table
	mm_wipe(parameters, sizeof(parameters));

	// Usernum
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &usernum;
	parameters[0].is_unsigned = true;

	// Spam Folder
	parameters[1].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[1].buffer_length = sizeof(uint64_t);
	parameters[1].buffer = &inbox;
	parameters[1].is_unsigned = true;

	// Inbox
	parameters[2].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[2].buffer_length = sizeof(uint64_t);
	parameters[2].buffer = &inbox;
	parameters[2].is_unsigned = true;

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
		st_cleanup(salt, shard, b64_salt, b64_shard, b64_verification);
		auth_stacie_cleanup(stacie);
		return false;
	}

	// Mailboxes Table
	mm_wipe(parameters, sizeof(parameters));

	// Merge the username with the default system domain name.
	if (!(newaddr = st_merge("sns", username, "@", magma.system.domain))) {
		log_pedantic("Unable to generate an email address for the new user.");
		st_cleanup(salt, shard, b64_salt, b64_shard, b64_verification);
		auth_stacie_cleanup(stacie);
		return false;
	}

	// Username
	parameters[0].buffer_type = MYSQL_TYPE_STRING;
	parameters[0].buffer = (chr_t *)st_char_get(newaddr);
	parameters[0].buffer_length = st_length_get(newaddr);

	// Usernum
	parameters[1].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[1].buffer_length = sizeof(uint64_t);
	parameters[1].buffer = &usernum;
	parameters[1].is_unsigned = true;

	// Insert the email address into the mailboxes table.
	if (!stmt_exec_conn(stmts.register_insert_mailboxes, parameters, transaction)) {
		log_pedantic("Unable to insert the user into the database. (Failed on Mailboxes table.)");
		st_cleanup(newaddr, salt, shard, b64_salt, b64_shard, b64_verification);
		auth_stacie_cleanup(stacie);
		return false;
	}

	// Finally create the storage key pair.
	if ((meta_crypto_keys_create(usernum, username, stacie->keys.master, transaction))) {
		log_pedantic("Unable to insert the user into the database. (Failed on Keys table.)");
		st_cleanup(newaddr, salt, shard, b64_salt, b64_shard, b64_verification);
		auth_stacie_cleanup(stacie);
		return false;
	}

	// Cleanup and tell the caller everything worked.
	st_cleanup(newaddr, salt, shard, b64_salt, b64_shard, b64_verification);
	auth_stacie_cleanup(stacie);
	*outuser = usernum;

	return true;
}
