
/**
 * @file /magma/web/register/register.c
 *
 * @brief	Functions for handling the registration process.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"


/**
 * @brief	Display a custom message to the remote client using the register/message template.
 * @param	con		the client connection to receive the message.
 * @param	message	a pointer to a null-terminated string containing the custom message to be displayed inside the template sent to the remote client.
 * @return	This function returns no value.
 */
void register_print_message(connection_t *con, chr_t *message) {

	stringer_t *template;
	http_content_t *content;

	if (!(content = http_get_template("register/message")) || !(template = st_dupe(content->resource))) {
		http_print_500(con);
		return;
	}

	if (st_replace(&template, PLACER("$MESSAGE$", 9), NULLER(message)) != 1) {
		http_print_500(con);
		st_free(template);
		return;
	}

	http_response_header(con, 200, content->type, st_length_get(template));
	con_write_st(con, template);
	st_free(template);

	return;
}

/**
 * @brief	Print out a captcha challenge for a specified registration process.
 * @param	con		the underlying client connection.
 * @param	reg		the pending registration session of the remote client.
 * @return	This function returns no value.
 */
void register_print_captcha(connection_t *con, register_session_t *reg) {

	stringer_t *image;

	// Generate a new human verification field.
	if (!(reg->hvf_value = rand_choices("23456789abcdefghjkmnpqrstuvwxyzABCDEFGHJKMNPQRSTUVWXYZ%#@&*?+=", 10))) {
		http_print_500(con);
		return;
	}

	log_info("xxx: captcha hvf value = [%.*s]\n", (int)st_length_get(reg->hvf_value), st_char_get(reg->hvf_value));

	// Print_t the appropriate image.
	if (!(image = register_captcha_generate(reg->hvf_value))) {
		http_print_500(con);
		return;
	}

	con_print(con, "HTTP/1.1 200 OK\r\nPragma: no-cache\r\nCache-Control: no-store\r\nContent-Type: image/gif\r\nContent-Length: %u\r\n\r\n", st_length_get(image));
	con_write_st(con, image);
	st_free(image);

	return;
}

/**
 * @brief	Display step 1 of the registration process (collect username, password, and captcha challenge)
 * @param	con		a pointer to the connection object of the remote client making the registration request.
 * @param	reg		a pointer to the pending registration session of the remote client.
 * @param	message	if not NULL, an optional error message to be displayed inside the registration step 1 template.
 * @return	This function returns no value.
 */
void register_print_step1(connection_t *con, register_session_t *reg, chr_t *message) {

	stringer_t *template, *pmessage = NULL;
	http_content_t *content;

	if (message) {
		pmessage = PLACER(message, ns_length_get(message));
	}

	if (!(content = http_get_template("register/step1")) || !(template = st_dupe(content->resource))) {
		http_print_500_log(con, "Could not open user registration template.");
		return;
	}

	if ((reg->username && st_replace(&template, PLACER("$USERNAME$", 10), reg->username) != 1) || (!reg->username && st_replace(&template, PLACER("$USERNAME$", 10), PLACER("", 0)) != 1)) {
		http_print_500_log(con, "Unable to process username variable in registration template.");
		st_free(template);
		return;
	}

	if ((pmessage && st_replace(&template, PLACER("$ERROR$", 7), pmessage) != 1) || (!message && st_replace(&template, PLACER("$ERROR$", 7), PLACER("", 0)) != 1)) {
		http_print_500_log(con, "Unable to process error variable in registration template.");
		st_free(template);
		return;
	}
	if (st_replace(&template, PLACER("$SESSION$", 9), reg->name) != 2) {
		http_print_500_log(con, "Unable to process session variable in registration template.");
		st_free(template);
		return;
	}

	http_response_header(con, 200, content->type, st_length_get(template));
	con_write_st(con, template);
	st_free(template);

	return;
}

/**
 * @brief	Display step 2 of the registration process.
 * @note	Previously, this step used to obtain information like the desired user plan and billing info.
 * 			At the moment, however, this step effectively does nothing but display a page to be clicked-through.
 * @param	con		a pointer to the connection object of the remote client making the registration request.
 * @param	reg		a pointer to the pending registration session of the remote client.
 * @param	message	if not NULL, an optional error message to be displayed inside the registration step 2 template.
 * @return	This function returns no value.
 */
void register_print_step2(connection_t *con, register_session_t *reg, chr_t *message) {

	http_content_t *content;
	stringer_t *template, *pmessage = NULL;

	if (message) {
		pmessage = PLACER(message, ns_length_get(message));
	}

	if (!(content = http_get_template("register/step2")) || !(template = st_dupe(content->resource))) {
		http_print_500_log(con, "Could not load step2 template.");
		return;
	}

	if ((message && st_replace(&template, PLACER("$ERROR$", 7), pmessage) != 1) || (!message && st_replace(&template, PLACER("$ERROR$", 7), PLACER("", 0)) != 1)) {
		http_print_500_log(con, "Unable to replace variable $ERROR$ in step 2 template.");
		st_free(template);
		return;
	}

	if (st_replace(&template, PLACER("$SESSION$", 9), reg->name) != 1) {
		http_print_500_log(con, "Unable to replace variable $SESSIONS$ in step 2 template.");
		st_free(template);
		return;
	}

	// Short-circuit all of this for now.
	http_response_header(con, 200, content->type, st_length_get(template));
	con_write_st(con, template);
	st_free(template);

	return;
}

/**
 * @brief	Display the registration step 3 template for a connection that has successfully created a new user.
 * @param	the remote connection making the registration request.
 * @return	This function returns no value.
 */
void register_print_step3(connection_t *con) {

	stringer_t *template;
	http_content_t *content;

	if (!(content = http_get_template("register/step3"))  || !(template = st_dupe(content->resource))) {
		http_print_500(con);
		return;
	}

	http_response_header(con, 200, content->type, st_length_get(template));
	con_write_st(con, template);
	st_free(template);

	return;
}

/**
 * @brief	Process a user registration request (/register).
 * @note	All requests will be redirected to a secure location.
 * 			All new registration requests will also need to pass a test to ensure that they haven't been the product of abusive behavior.
 * 			If an existing registration session can't be located, a new one will be generated.
 * 			Depending on the user supplied http POST data, one of the three registration steps will be shown,
 * 			or a captcha challenge will be presented to the user.
 * @return	This function returns no value.
 */
void register_process(connection_t *con) {

	chr_t *message;
	http_data_t *key;
	register_session_t *reg = NULL;

	// Access to the registration engine requires SSL.
	if (con_secure(con) != 1) {
		http_print_301(con, "/register", 1);
		return;
	}

	// Check for abuse.
	if (register_abuse_checks(con)) {
		con->http.mode = HTTP_COMPLETE;
		return;
	}

	// Look for the con.
	if ((key = http_data_get(con, HTTP_DATA_POST, "con"))) {
		reg = register_session_get(con, key->value);
	}
	else if ((key = http_data_get(con, HTTP_DATA_GET, "captcha"))) {
		reg = register_session_get(con, key->value);
	}

	if ((key = http_data_get(con, HTTP_DATA_POST, "session"))) {
		reg = register_session_get(con, key->value);
	}


	// If no con is found, create a new one.
	if (!reg && (!(reg = register_session_generate()))) {
		log_pedantic("Unable to generate new registration session.");
		con->http.mode = HTTP_ERROR_500;
		return;
	}

	// We need to print_t the captcha.
	if (http_data_get(con, HTTP_DATA_GET, "captcha")) {
		register_print_captcha(con, reg);
	}
	// If there's a usernum, this con has already created a user.
	else if (reg->usernum) {
		register_print_step3(con);
	}
	// The step2 data has been submitted.
	else if (http_data_get(con, HTTP_DATA_POST, "next2")) {

		if ((message = register_business_step2(con, reg))) {
			register_print_step2(con, reg, message);
		}
		else {
			register_print_step3(con);
		}

	}
	// The step1 data has been submitted.
	else if (http_data_get(con, HTTP_DATA_POST, "next1")) {

		if ((message = register_business_step1(con, reg))) {
			register_print_step1(con, reg, message);
		}
		else {
			register_print_step2(con, reg, NULL);
		}

	}
	// The default action is to output step1.
	else {
		register_print_step1(con, reg, NULL);
	}

	register_session_cache(con, reg);
	register_session_free(reg);
	con->http.mode = HTTP_COMPLETE;

	return;
}
