
/**
 * @file /magma/web/register/business.c
 *
 * @brief	Functions for handling validation for the registration process.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Determine whether a registered password is valid.
 * @note	Each password must be between REGISTER_PASSWORD_MIN_LENGTH (5) and REGISTER_PASSWORD_MAX_LENGTH (200) characters long.
 * @param	password	the user's password to be evaluated.
 * @return	false on failure (too long, too short, or bad characters) or true on success.
 */
bool_t register_business_validate_password(stringer_t *password) {

	chr_t *holder;
	size_t length, increment;

	if (!password) {
		return false;
	}

	holder = st_char_get(password);
	length = st_length_get(password);

	if (length < REGISTER_PASSWORD_MIN_LENGTH || length > REGISTER_PASSWORD_MAX_LENGTH) {
		return false;
	}

	for (increment = 0; increment < length; increment++) {

		if (*(holder + increment) < '!' || *(holder + increment) > '~') {
			return false;
		}

	}

	return true;
}

/**
 * @brief	Determine whether a registered username is valid.
 * @param	username	a managed string containing the proposed username to be evaluated.
 * @return	-1 or 0 on failure (too long, too short, or bad characters) or 1 on success.
 */
int_t register_business_validate_username(stringer_t *username) {

	chr_t *holder;
	int_t consecutive = 0;
	size_t length, increment;

	if (!username) {
		return 0;
	}

	holder = st_char_get(username);
	length = st_length_get(username);

	if (length < REGISTER_USERNAME_MIN_LENGTH || length > REGISTER_USERNAME_MAX_LENGTH) {
		return 0;
	}

	// Check to make sure the username starts with a alpha character.
	if ((*holder < 'a' || *holder > 'z') && (*holder < 'A' || *holder > 'Z')) {
		return 0;
	}

	// Check to make sure the username only contains letters, numbers and underscores.
	for (increment = 0; increment < length; increment++) {

		if ((*(holder + increment) < 'a' || *(holder + increment) > 'z') && (*(holder + increment) < 'A' || *(holder + increment) > 'Z') \
			&& (*(holder + increment) < '0' || *(holder + increment) > '9') && *(holder + increment) != '_') {
			return 0;
		}
		else if (*(holder + increment) == '_' && consecutive == 1) {
			return -1;
		}
		else if (*(holder + increment) == '_' && consecutive == 0) {
			consecutive = 1;
		}
		else if (*(holder + increment) != '_' && consecutive == 1) {
			consecutive = 0;
		}

	}

	// Check to make sure the username ends with an alpha or numeric character.
	length--;

	if ((*(holder + length) < 'a' || *(holder + length) > 'z') && (*(holder + length) < 'A' || *(holder + length) > 'Z') && (*(holder + length) < '0' || *(holder + length) > '9')) {
		return 0;
	}

	return 1;
}

/**
 * @brief	Perform verification checking on all step 1 completed user fields.
 * @note	Checks include captcha verification, username validation, and password reentry verification and validation.
 * @param	con		the underlying client connection.
 * @param	reg		the underlying registration session.
 * @return	NULL on success, or a descriptive error string on failure.
 */
chr_t * register_business_step1(connection_t *con, register_session_t *reg) {

	int_t holder = 0;
	http_data_t *data, *compare;
	static chr_t msgbuf[256] = {};

	// Store the username in the con so the user doesn't need to reenter it.
	if ((data = http_data_get(con, HTTP_DATA_POST, "username"))) {
		reg->username = st_dupe(data->value);
	}

	if ((data = http_data_get(con, HTTP_DATA_POST, "human"))) {
		reg->hvf_input = st_dupe(data->value);
	}

	// The first thing we need to check is the human verification value.
	if (!reg->hvf_input || (data = http_data_get(con, HTTP_DATA_POST, "human")) == NULL || st_cmp_ci_eq(reg->hvf_input, reg->hvf_value)) {
		log_pedantic("HVF mismatch: entered = %.*s / expected = %.*s", st_length_int(reg->hvf_input), st_char_get(reg->hvf_input), st_length_int(reg->hvf_value), st_char_get(reg->hvf_value));
		return "\n\t\t\t\t<p id=\"error\">The value entered in the <q>Human Verification</q> field appears to be "
			"incorrect. Please type the characters you see in the image below into the <q>Human Verification</q> field and submit the "
			"form again. If you continue to experience problems, please use the contact form to let us know.</p>";
	}

	// Next make sure the passwords match.
	if (!(data = http_data_get(con, HTTP_DATA_POST, "passone"))) {
		return "\n\t\t\t\t<p id=\"error\">You must supply a password for registration!</p>";
	} else if (!(compare = http_data_get(con, HTTP_DATA_POST, "passtwo"))) {
		return "\n\t\t\t\t<p id=\"error\">You must re-enter your password for verification!</p>";
	}
	else if (!register_business_validate_password(data->value)) {
		snprintf(msgbuf, sizeof(msgbuf), "\n\t\t\t\t<p id=\"error\">Your password was invalid! Please make sure you use characters that are "
				"allowed, and that your password is between %u and %u characters long.</p>", REGISTER_PASSWORD_MIN_LENGTH, REGISTER_PASSWORD_MAX_LENGTH);
		return msgbuf;
	} else if (st_cmp_ci_eq(data->value, compare->value)) {
		return "\n\t\t\t\t<p id=\"error\">Your registration request did not contain matching passwords!</p>";
	}
	else {
		reg->password = st_dupe(data->value);
	}

	// Validate the username.
	if (!reg->username || (holder = register_business_validate_username(reg->username)) != 1) {

		if (holder == -1) {
			return "\n\t\t\t\t<p id=\"error\">The username you selected is invalid. Usernames with "
			"consecutive underscores are not allowed. Please correct the problem and try again.</p>";
		}

		return "\n\t\t\t\t<p id=\"error\">The username you selected is invalid. The username must start with a "
			"letter and only contain letters, numbers and underscores. Please correct the problem and try again.</p>";
	}
	// Make sure the name isn't already taken.
	else if (register_data_check_username(reg->username)) {
		return  "\n\t\t\t\t<p id=\"error\">The username you selected appears to already have been taken. Please select another "
			"username and try again.</p>";
	}

	return NULL;
}

/**
 * @brief	Perform verification checking on all step 2 completed user fields, and display a welcome banner on success.
 * @note	Checks include plan type validation, and billing information processing.
 * 			After step 2, the user's supplied information will be persisted into the database.
 * @param	con		the underlying client connection.
 * @param	reg		the underlying registration session.
 * @return	NULL on success, or a descriptive error string on failure.
 */
chr_t * register_business_step2(connection_t *con, register_session_t *reg) {

	int64_t transaction;
	http_data_t *data;
	stringer_t *to, *from, *welcome;
	uint64_t usernum = 0;

	// Pull and store the plan.
	if ((data = http_data_get(con, HTTP_DATA_POST, "plan"))) {
		if (!st_cmp_cs_eq(data->value, PLACER("basic", 5))) {
			reg->plan = 1;
		}
		else if (!st_cmp_cs_eq(data->value, PLACER("personal", 8))) {
			reg->plan = 2;
		}
		else if (!st_cmp_cs_eq(data->value, PLACER("enhanced", 8))) {
			reg->plan = 3;
		}
		else if (!st_cmp_cs_eq(data->value, PLACER("premium", 7))) {
			reg->plan = 4;
		}
	}

	// Force the plan to the basic one.
	reg->plan = 1;

	if (reg->plan == 0) {
		return "\n\t\t\t\t<p id=\"error\">You must select one of the plans listed below.</p>";
	}
	else if (reg->plan == 3 || reg->plan == 4) {
		// Payment logic skipped.
	}

	// Start the transaction.
	if ((transaction = tran_start()) == -1) {
		return "\n\t\t\t\t<p id=\"error\">An internal error occurred. Please try again. If you continue " \
			"to experience problems, please report it via the contact form.</p>";
	}

	// Database insert.
	if (!register_data_insert_user(con, reg, transaction, &usernum)) {
		tran_rollback(transaction);
		return "\n\t\t\t\t<p id=\"error\">It appears that between the time its taken you to complete step one and step two the username "
			"you selected has been taken. Please return to step one and select a different username.</p>";
	}

	// Were finally done.
	tran_commit(transaction);

	// Store the user number. This prevents us from trying to create this user twice.
	reg->usernum = usernum;

	// Generate the welcome message.
	// However we can only generate it if a contact address is specified in the configuration.
	if (magma.admin.contact) {
		from = st_dupe(magma.admin.contact);
		to = st_merge("sns", reg->username, "@", magma.system.domain);
		welcome = st_merge("nsnsn", "Subject: Welcome to Magma!\r\nFrom: Support <", magma.admin.contact, ">\r\nTo: ", to, "\r\n\r\n"
				"Your account has been created successfully. Please be sure to remember your password. If you forget your password, we can’t change it for you. Also be sure to "
				"check your e-mail every 120 days, or your account may be locked and eventually deleted.\r\n\r\nWelcome to Magma! Please use our services responsibly.\r\n");

		// Send the welcome message.
		if (to && from && welcome && (smtp_send_message(to, from, welcome) != 1)) {
			log_pedantic("Unable to send the welcome message.");
		} else if (!to || !from || !welcome) {
			log_pedantic("Unable to create welcome message for new user.");
		}

		st_cleanup(to);
		st_cleanup(from);
		st_cleanup(welcome);
	} else {
		log_pedantic("New user was created successfully but could not be sent a welcome message without magma.admin.contact set.");
	}

	// And finally, increment the abuse counter.
	register_abuse_increment_history(con);

	return NULL;
}
