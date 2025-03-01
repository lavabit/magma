
/**
 * @file /magma/web/teacher/teacher.c
 *
 * @brief	Functions to allow users to train the statistical mail filter.
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

	// Error check. This will catch dates after 2038, if the time_t bug hasn't been fixed by then, or in the year 292277026596
	// if a 64 bit time value is used. This assumes Magma will run for that long.
	if (difftime(sm_time, current) != 31556926UL) {
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
