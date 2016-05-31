
/**
 * @file /magma/web/teacher/teacher.c
 *
 * @brief	Functions to allow users to train the statistical mail filter.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Display a custom message to the remote client using the teacher/message template.
 * @param	con		the client connection to receive the message.
 * @param	message	a null-terminated string containing the custom message to be displayed inside the template sent to the remote client.
 * @return	This function returns no value.
 */
void teacher_print_message(connection_t *con, chr_t *message) {

	http_page_t *page;
	inx_cursor_t *cursor;
	stringer_t *raw, *header;

	if (!(page = http_page_get("teacher/message"))) {
		http_print_500(con);
		return;
	}

	// Set the message.
	xml_set_xpath_ns(page->xpath_ctx, (xmlChar *)"//xhtml:p[@id='message']", (uchr_t *)message);

	if (!(raw = xml_dump_doc(page->doc_obj))) {
		http_print_500(con);
		http_page_free(page);
		return;
	}

	// TODO: This can be cleaned up?

	con_print(con, "HTTP/1.1 200 OK\r\n");

	if (con->http.headers && (cursor = inx_cursor_alloc(con->http.headers))) {

		while ((header = inx_cursor_value_next(cursor))) {
			con_write_st(con, header);
		}

		inx_cursor_free(cursor);
	}

	con_print(con, "Content-Type: %.*s\r\nContent-Length: %u\r\n\r\n", st_length_get(page->content->type), st_char_get(page->content->type), st_length_get(raw));
	con_write_st(con, raw);
	http_page_free(page);
	st_free(raw);

	return;
}

/**
 * @brief	Display a custom message to the remote client using the teacher/message template.
 * @param	con		the client connection to receive the message.
 * @param	message	a null-terminated string containing the custom message to be displayed inside the template sent to the remote client.
 * @return	This function returns no value.
 */
void teacher_print_form(connection_t *con, http_page_t *page, teacher_data_t *teach) {

	stringer_t *raw;
	chr_t buffer[64];

	if (!page && !(page = http_page_get("teacher/teacher"))) {
		teacher_print_message(con, "An error occurred while trying to process your request. Please try again in a few minutes.");
		return;
	}

	xml_set_xpath_ns(page->xpath_ctx, (xmlChar *)"//xhtml:strong[@id='current']", (uchr_t *)(teach->disposition == 1 ? "spam" : "innocent"));
	xml_set_xpath_ns(page->xpath_ctx, (xmlChar *)"//xhtml:strong[@id='new']", (uchr_t *)(teach->disposition == 1 ? "innocent" : "spam"));

	// Set the signature number.
	if (snprintf(buffer, 64, "%lu", teach->signum) > 0) {
		xml_set_xpath_property(page->xpath_ctx, (xmlChar *)"//xhtml:input[@id='sig']", (uchr_t *)"value", (uchr_t *)buffer);
	}

	// Set the key number.
	if (snprintf(buffer, 64, "%lu", teach->keynum) > 0) {
		xml_set_xpath_property(page->xpath_ctx, (xmlChar *)"//xhtml:input[@id='key']", (uchr_t *)"value", (uchr_t *)buffer);
	}

	if (!(raw = xml_dump_doc(page->doc_obj))) {
		http_print_500(con);
		http_page_free(page);
		return;
	}

	http_response_header(con, 200, page->content->type, st_length_get(raw));
	con_write_st(con, raw);
	http_page_free(page);
	st_free(raw);

	return;
}

/**
  * @brief	Get the teacher/teacher template and add an error message to it.
  * @param	xpath	a null-terminated string containing the xpath of the node in the template to be marked with the error message.
  * @param	id		a null-terminated string containing the id of the error message node to be added as the sibling of the node in the specified xpath.
  * @param	message	a null-terminated string containing the error message to be displayed to the user.
  * @param	NULL on failure, or a pointer to the modified teacher/teacher template page on success.
 */
http_page_t * teacher_add_error(chr_t *xpath, chr_t *id, chr_t *message) {

	http_page_t *page = NULL;
	xmlNodePtr node, error;
	xmlXPathObjectPtr xpath_obj;

	if (!(page = http_page_get("teacher/teacher"))) {
		return NULL;
	}

	if ((xpath_obj = xml_xpath_eval((uchr_t *)xpath, page->xpath_ctx)) && xpath_obj->nodesetval &&
		xpath_obj->nodesetval->nodeNr && xpath_obj->nodesetval->nodeTab[0]) {

		node = (xmlNodePtr)xpath_obj->nodesetval->nodeTab[0];

		// Make the field red.
		xml_node_set_property(node, (uchr_t *)"class", (uchr_t *)"red");

		// Add the error message.
		if ((error = xml_node_new((uchr_t *)"div"))) {
			xml_node_set_property(error, (uchr_t *)"id", (uchr_t *)id);
			xml_node_set_content(error, (uchr_t *)message);

			if (!(xml_node_add_sibling(node, error))) {
				xml_node_free(error);
			}

		}
	}

	return page;
}

/**
 * @brief	Create a cookie for a successfully password-authenticated teacher request.
 * @param	con		the connection of the web client accessing the teacher facility.
 * @param	teach	the spam signature associated with the cookie request; it supplies the identifying password stored in the cookie.
 * @return	This function returns no value.
 */
void teacher_add_cookie(connection_t *con, teacher_data_t *teach) {

	struct tm tm_time;
	stringer_t *holder;
	time_t current, sm_time;
	chr_t header[512], date[128];
	multi_t key = { .type = M_TYPE_STRINGER, .val.st = NULL };

	if (!teach || !teach->password) {
		log_pedantic("Invalid teacher data passed in.");
		return;
	}

	// Store the current time.
	if ((current = sm_time = time(NULL)) == (time_t)-1) {
		log_pedantic("Unable to retrieve the current time.");
		return;
	}

	// Add approximately one year to the expiration.
	sm_time += 31556926L;

	// Error check. This will catch dates after 2038, if the time_t bug hasn't been fixed by then. (Does not apply to some 64 bit systems.)
	if (sm_time <= current) {
		log_pedantic("Date wrap around error. The time_t datatype is not large enough to hold the plan expiration.");
		return;
	}

	// Break the time up into its component parts.
	if (!localtime_r(&sm_time, &tm_time)) {
		log_pedantic("Unable to break the current time up into its component parts.");
		return;
	}

	// Build the string as YYYY-MM-DD.
	if (strftime(date, 128, "%a, %d-%b-%Y %H:%M:%S GMT", &tm_time) <= 0 ||
		snprintf(header, 512, "Set-Cookie: con=%.16s; secure=yes; path=/teacher; expires=%s\r\n", st_char_get(teach->password), date) <= 0) {
		log_pedantic("Unable to build the header string.");
		return;
	}

	// Create the linked list, if needed.
	if (!con->http.headers && !(con->http.headers = inx_alloc(M_INX_LINKED, &st_free))) {
		log_pedantic("Unable to create the linked list.");
		return;
	}

	// Create the stringer.
	if (!(key.val.st = holder = st_import(header, ns_length_get(header)))) {
		log_pedantic("Unable to import the header.");
		return;
	}

	// Add it.
	if (!inx_insert(con->http.headers, key, holder)) {
		log_pedantic("Unable to add the header.");
		st_free(holder);
		return;
	}

	return;
}

/**
 * @brief	The main entry point for the /teacher web application.
 * @note	Each /teacher request must contain a spam signature and key specified by the user.
 * 			If magma was able to find the signature, the signature's key must match the user-specified key value.
 * 			The signature must also not have been previously trained.
 * 			On successful training, the signature will be freed in memory and in the database, but kept for a couple of hours in distributed cache.
 * 			User password authentication is necessary for training, and this routine manages cookies for subsequent training requests.
 * @param	con		the connection object underlying the client /teacher request.
 * @return	This function returns no value.
 */
void teacher_process(connection_t *con) {

	teacher_data_t *teach;
	credential_t *credential;
	stringer_t *cookie = NULL;
	http_data_t *sig, *key, *pass;
	uint64_t signum, keynum;

	// Access to the teacher requires SSL.
	if (con_secure(con) != 1) {
		http_print_301(con, "/teacher", 1);
		return;
	}

	con->http.mode = HTTP_COMPLETE;

	// Get the spam signature and key from user input.
	if (!(sig = http_data_get(con, HTTP_DATA_ANY, "sig")) || !sig->value ||	!(key = http_data_get(con, HTTP_DATA_ANY, "key")) || !key->value ||
		!uint64_conv_st(sig->value, &signum) || !signum || !uint64_conv_st(key->value, &keynum) || !keynum) {
		teacher_print_message(con, "Invalid URL. Please check the URL string and try again. Be weary that sometimes e-mail programs will mangle the URL.");
		return;
	}

	if (!(teach = teacher_data_get(signum))) {
		teacher_print_message(con, "The signature number provided does not match a message currently in the database. Spam signatures can only be trained "
			"once. Spam signatures are only stored for 30 days after a message arrives. If the message is recent, and has not already been trained, "
			"please check the URL and try again.");
		return;
	}

	// If there is a cookie, parse the value.
	if ((pass = http_data_get(con, HTTP_DATA_HEADER, "Cookie")) && pass->value && (cookie = st_dupe(pass->value))) {
		st_replace(&cookie, PLACER("con=", 4), PLACER("", 0));
	}

	// Check the cryptographic key.
	if (teach->keynum != keynum) {
		teacher_print_message(con, "The cryptographic key provided does not match the cryptographic key associated with the requested spam " \
			"signature. Please check the URL and try again.");
	}

	// Check whether the signature was recently trained.
	else if (teach->completed == 1) {
			teacher_print_message(con, "This spam signature has already been trained. Please select another message for training.");
	}

	// Check for a cookie. Make sure the comparison value is at least 16 characters long; and then wrap it in a placer to ensure only 16 characters are considered.
	else if (cookie && st_length_get(cookie) >= 16 && !st_cmp_cs_starts(teach->password, PLACER(st_char_get(cookie), 16))) {

		// Train the engine.
		// QUESTION: Is the reason we aren't checking the return value?
		dspam_train(teach->usernum, teach->disposition, teach->signature);

		// Delete the signature from the database.
		teacher_data_delete(teach);

		// Print_t the output message.
		teacher_print_message(con, "Signature data trained.");
	}

	else if (http_data_get(con, HTTP_DATA_POST, "submit")) {
/** TODO FIXME: Memory leaks here. This part isn't fixed to work with the new credentials functions, so be careful reusing this code.*/
		if (!(pass = http_data_get(con, HTTP_DATA_POST, "password")) || !pass->value || !(credential = credential_alloc_auth(teach->username)) ||
			st_cmp_cs_eq(teach->password, credential->auth.password)) {
			teacher_print_form(con, teacher_add_error("//xhtml:input[@id='password']", "password_msg", "An invalid password was provided. Please try again."), teach);
		}
		else {

			// Train the engine.
			// QUESTION: Same question as above.
			dspam_train(teach->usernum, teach->disposition, teach->signature);

			// Add the cookie to the output, if necessary.
			if ((pass = http_data_get(con, HTTP_DATA_POST, "cookie")) && pass->value && !st_cmp_cs_eq(pass->value, PLACER("on", 2))) {
				teacher_add_cookie(con, teach);
			}

			// Delete the signature from the database.
			teacher_data_delete(teach);

			// Print_t the output message.
			teacher_print_message(con, "Signature data trained.");
		}

		// Securely delete the message.
		credential_free(credential);
	}
	else {
		teacher_print_form(con, NULL, teach);
	}

	st_cleanup(cookie);
	// Free the signature object (it's already gone from the database), but keep it lying around in the cache.
	teacher_data_save(teach);
	teacher_data_free(teach);

	return;
}
