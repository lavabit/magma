
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
 * @return	0 if the new user account was successfully created, -1 if a technical error occurs, and 1 if an invalid value is provided.
 */
int_t register_data_insert_user(connection_t *con, uint16_t plan, stringer_t *username, stringer_t *password, int64_t transaction, uint64_t *outuser) {

	uint16_t serial = 0;
	uint32_t bonus = 0;
	MYSQL_BIND parameters[9];
	auth_stacie_t *stacie = NULL;
	uint8_t rotated = 1, ads = 0, secure = 1;
	const chr_t *account_plans[] = { "BASIC", "PERSONAL", "ENHANCED", "PREMIUM", "STANDARD", "PREMIER" };
	uint64_t name_len, quota = 0, usernum, inbox, size_limit, send_limit, recv_limit;
	stringer_t *newaddr = NULL, *salt = NULL, *shard = NULL, *b64_salt = NULL, *b64_shard = NULL, *b64_verification = NULL,
		*ip = NULL, *realm = NULL, *expiration = MANAGEDBUF(128);

	/// LOW: This function should be passed a number of days (or years) to use for any of the pre-paid account plans.
	/// LOW: The IP address could be passed in as an ip_t or as a string to avoid the need for the entire connection object.

		// The password must be longer than the required minimum in both bytes, and and characters. We check both lengths
		// because we are paranoid. We don't want a malformed unicode string slipping through, because it will
		// still require excessive processing by the STACIE dervication functions later.
		if (st_length_get(password) < magma.secure.minimum_password_length ||
			utf8_length_st(password) < magma.secure.minimum_password_length) {
			log_pedantic("A user attempt to register with a password that was shorter than the minimum. { length = %zu }", utf8_length_st(password));
			return 1;
		}


	// Configure the limits for the plan. We are currently using hard coded values. In the future this may be
	// setup dynamically by pull the default values out of the limits table.
	if (plan == 1) {
		st_sprint(expiration, "0000-00-00");
		quota = 134217728UL; // 128 MB
		size_limit = 33554432;
		recv_limit = 1024;
		send_limit = 256;
		ads = 0;

	}
	else if (plan == 2) {
		st_sprint(expiration, "0000-00-00");
		quota = 1073741824UL; // 1,024 MB
		size_limit = 64UL << 20;
		recv_limit = 1024;
		send_limit = 256;
		ads = 1;
	}
	else if (plan == 3) {
		expiration = time_print_local(expiration, "%Y-%m-%d", time(NULL) + 31536000);
		quota = 1073741824UL; // 1,024 MB
		size_limit = 64UL << 20;
		recv_limit = 1024;
		send_limit = 512;
		ads = 0;
	}
	else if (plan == 4) {
		expiration = time_print_local(expiration, "%Y-%m-%d", time(NULL) + 31536000);
		quota = 8589934592UL; // 8,192 MB
		size_limit = 128UL << 20;
		recv_limit = 8192;
		send_limit = 768;
		ads = 0;
	}
	else if (plan == 5) {
		expiration = time_print_local(expiration, "%Y-%m-%d", time(NULL) + 31536000);
		quota = 5368709120UL; // 5,120 MB
		size_limit = 64UL << 20;
		recv_limit = 8192;
		send_limit = 128;
		ads = 0;
	}
	else if (plan == 6) {
		expiration = time_print_local(expiration, "%Y-%m-%d", time(NULL) + 31536000);
		quota = 21474836480UL; // 20,480 MB
		size_limit = 128UL << 20;
		recv_limit = 8192;
		send_limit = 128;
		ads = 0;
	}
	else {
		log_pedantic("Unrecognized account plan. Only values between 1 and 6 are supported. { plan = %hu }", plan);
		return 1;
	}

	// We start by generating all of the values we'll need to complete the registration process. The STACIE values are first.
	if (!(salt = stacie_create_salt(NULL)) || !(b64_salt = base64_encode_mod(salt, NULL)) ||
		!(shard = stacie_create_shard(NULL)) || !(b64_shard = base64_encode_mod(shard, NULL)) ||
		!(stacie = auth_stacie(bonus,  username,  password,  salt,  NULL, NULL)) ||
		!(b64_verification = base64_encode_mod(stacie->tokens.verification, NULL))) {

		log_pedantic("Unable to generate the STACIE authentication values. { username = %.*s }",
			st_length_int(username), st_char_get(username));

		st_cleanup(salt, shard, b64_salt, b64_shard, b64_verification);
		auth_stacie_cleanup(stacie);
		return -1;
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
	parameters[4].buffer = (chr_t *)account_plans[plan - 1];
	parameters[4].buffer_length = ns_length_get(account_plans[plan - 1]);

	// The advertising boolean.
	parameters[5].buffer_type = MYSQL_TYPE_TINY;
	parameters[5].buffer_length = sizeof(uint8_t);
	parameters[5].buffer = &ads;
	parameters[5].is_unsigned = true;

	// The quota.
	parameters[6].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[6].buffer_length = sizeof(uint64_t);
	parameters[6].buffer = &quota;
	parameters[6].is_unsigned = true;

	// The expiration date for a pre-paid account plan. Set the all the date to all zeros for a free account plan.
	parameters[7].buffer_type = MYSQL_TYPE_STRING;
	parameters[7].buffer = st_data_get(expiration);
	parameters[7].buffer_length = st_length_get(expiration);

	// Insert the user.
	if ((usernum = stmt_insert_conn(stmts.register_insert_stacie_user, parameters, transaction)) == 0) {
		log_pedantic("Unable to insert the user into the database. (Failed on User table.)");
		st_cleanup(salt, shard, b64_salt, b64_shard, b64_verification);
		auth_stacie_cleanup(stacie);
		return -1;
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

	// Rotated
	parameters[4].buffer_type = MYSQL_TYPE_TINY;
	parameters[4].buffer_length = sizeof(uint8_t);
	parameters[4].buffer = &rotated;
	parameters[4].is_unsigned = true;

	// User_Realms table.
	if (!stmt_exec_conn(stmts.register_insert_stacie_realms, parameters, transaction)) {
		log_pedantic("Unable to insert the user into the database. (Failed on User_Realms table.)");
		st_cleanup(salt, shard, b64_salt, b64_shard, b64_verification);
		auth_stacie_cleanup(stacie);
		return -1;
	}

	// Profile Table
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
		return -1;
	}

	// Folders Table
	mm_wipe(parameters, sizeof(parameters));

	// Usernum
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &usernum;
	parameters[0].is_unsigned = true;

	// The folder where messages are delivered by default.
	parameters[1].buffer_type = MYSQL_TYPE_STRING;
	parameters[1].buffer = "Inbox";
	parameters[1].buffer_length = 5;

	// Insert the inbox into the folders table and record the folder number for later.
	if ((inbox = stmt_insert_conn(stmts.register_insert_folder_name, parameters, transaction)) == 0) {
		log_pedantic("Unable to insert the user into the database. (Failed on Folders table.)");
		st_cleanup(salt, shard, b64_salt, b64_shard, b64_verification);
		auth_stacie_cleanup(stacie);
		return -1;
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
		return -1;
	}

	// Dispatch Table
	mm_wipe(parameters, sizeof(parameters));

	// Usernum
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].buffer = &usernum;
	parameters[0].is_unsigned = true;

	// The secure storage boolean.
	parameters[1].buffer_type = MYSQL_TYPE_TINY;
	parameters[1].buffer_length = sizeof(uint8_t);
	parameters[1].buffer = &secure;
	parameters[1].is_unsigned = true;

	// Spam Folder
	parameters[2].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[2].buffer_length = sizeof(uint64_t);
	parameters[2].buffer = &inbox;
	parameters[2].is_unsigned = true;

	// Inbox
	parameters[3].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[3].buffer_length = sizeof(uint64_t);
	parameters[3].buffer = &inbox;
	parameters[3].is_unsigned = true;

	// Send size limit.
	parameters[4].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[4].buffer_length = sizeof(uint64_t);
	parameters[4].buffer = (chr_t *)&size_limit;
	parameters[4].is_unsigned = true;

	// Receive size limit.
	parameters[5].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[5].buffer_length = sizeof(uint64_t);
	parameters[5].buffer = (chr_t *)&size_limit;
	parameters[5].is_unsigned = true;

	// Daily send limit.
	parameters[6].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[6].buffer_length = sizeof(uint64_t);
	parameters[6].buffer = (chr_t *)&send_limit;
	parameters[6].is_unsigned = true;

	// Daily receive limit.
	parameters[7].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[7].buffer_length = sizeof(uint64_t);
	parameters[7].buffer = (chr_t *)&recv_limit;
	parameters[7].is_unsigned = true;

	// Daily receive limit, IP.
	parameters[8].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[8].buffer_length = sizeof(uint64_t);
	parameters[8].buffer = (chr_t *)&recv_limit;
	parameters[8].is_unsigned = true;

	// Dispatch table.
	if (!stmt_exec_conn(stmts.register_insert_dispatch, parameters, transaction)) {
		log_pedantic("Unable to insert the user into the database. (Failed on Dispatch table.)");
		st_cleanup(salt, shard, b64_salt, b64_shard, b64_verification);
		auth_stacie_cleanup(stacie);
		return -1;
	}

	// Mailboxes Table
	mm_wipe(parameters, sizeof(parameters));

	// Merge the username with the default system domain name.
	if (!(newaddr = st_merge("sns", username, "@", magma.system.domain))) {
		log_pedantic("Unable to generate an email address for the new user.");
		st_cleanup(salt, shard, b64_salt, b64_shard, b64_verification);
		auth_stacie_cleanup(stacie);
		return -1;
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
		return -1;
	}

	// Finally derive the realm key, and then create the DIME signet and key pair.
	if (!(realm = stacie_realm_key(stacie->keys.master, PLACER("mail",  4), salt, shard)) || meta_crypto_keys_create(usernum, username, realm, transaction)) {
		log_pedantic("Unable to insert the user into the database. (Failed on Keys table.)");
		st_cleanup(newaddr, salt, shard, b64_salt, b64_shard, b64_verification);
		auth_stacie_cleanup(stacie);
		return -1;
	}

	// Cleanup and tell the caller everything worked.
	st_cleanup(newaddr, salt, shard, b64_salt, b64_shard, b64_verification, realm);
	auth_stacie_cleanup(stacie);
	*outuser = usernum;

	return 0;
}
