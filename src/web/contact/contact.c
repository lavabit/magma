
/**
 * @file /magma/web/contact/contact.c
 *
 * @brief	Handle the contact form.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Display the contact or abuse notification form to the requesting user.
 * @note	Both the contact and abuse forms operate on the underlying template found in "contact/message"
 * @param	con			a pointer to the connection object across which the response will be sent.
 * @param	branch		a null-terminated string specifying the type of contact, either "Contact" or "Abuse".
 * @param	message		a null-terminated string pointing to a custom message to be displayed to the user.
 * @return	This function returns no value.
 */
void contact_print_message(connection_t *con, chr_t *branch, chr_t *message) {

	stringer_t *raw;
	http_page_t *page;

	if (!(page = http_page_get("contact/message"))) {
		http_print_500(con);
		return;
	}

	// For contact page submissions.
	if (!st_cmp_cs_eq(NULLER(branch), PLACER("Contact", 7))) {
		// Update the title.
		xml_set_xpath_ns(page->xpath_ctx, (xmlChar *)"//xhtml:title", (uchr_t *)"Lavabit ..::.. Contact");
		// Set the proper active indicators.
		xml_set_xpath_property(page->xpath_ctx, (xmlChar *)"//xhtml:a[@id='nav_contact']", (uchr_t *)"class", (uchr_t *)"active");
		xml_set_xpath_property(page->xpath_ctx, (xmlChar *)"//xhtml:a[text()='Contact Lavabit']", (uchr_t *)"class", (uchr_t *)"active");
	}

	// For abuse page submissions.
	if (!st_cmp_cs_eq(NULLER(branch), PLACER("Abuse", 5))) {
		// Update the title.
		xml_set_xpath_ns(page->xpath_ctx, (xmlChar *)"//xhtml:title", (uchr_t *)"Lavabit ..::.. Report Abuse");
		// Set the proper active indicators.
		xml_set_xpath_property(page->xpath_ctx, (xmlChar *)"//xhtml:div[@id='secondary']//xhtml:a[text()='Report Abuse']", (uchr_t *)"class", (uchr_t *)"active");
	}

	// Set the message.
	if (message) {
		xml_set_xpath_ns(page->xpath_ctx, (xmlChar *)"//xhtml:p[@id='message']", (uchr_t *)message);
	}

	if (!(raw = xml_dump_doc(page->doc_obj))) {
		http_print_500_log(con, "Contact page could not be generated.");
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
 * @brief	Display the contact or abuse form to the user.
 * @param	con		the connection across which the form will be returned.
 * @param	branch	a null-terminated string specifying the type of contact, either "Contact" or "Abuse".
 * @param	page	a pointer to an http page containing the contact document to be populated with the user's name, email, and message.
 * @return	This function returns no value.
 */
void contact_print_form(connection_t *con, chr_t *branch, http_page_t *page) {

	stringer_t *raw;
	http_data_t *data;
	xmlChar *escaped = NULL;

	if (!page && !st_cmp_cs_eq(NULLER(branch), PLACER("Abuse", 5)) && !(page = http_page_get("contact/abuse"))) {
		contact_print_message(con, branch, "An error occurred while trying to process your request. Please try again in a few minutes.");
		return;
	}
	else if (!page && !(page = http_page_get("contact/contact"))) {
		contact_print_message(con, branch, "An error occurred while trying to process your request. Please try again in a few minutes.");
		return;
	}

	// Update the name field.
	if ((data = http_data_get(con, HTTP_DATA_POST, "your_name")) && data->value) {
		xml_set_xpath_property(page->xpath_ctx, (xmlChar *) "//xhtml:input[@id='your_name']", (uchr_t *)"value", (uchr_t *)st_char_get(data->value));
	}

	// Update the email field.
	if ((data = http_data_get(con, HTTP_DATA_POST, "your_email")) && data->value) {
		xml_set_xpath_property(page->xpath_ctx, (xmlChar *) "//xhtml:input[@id='your_email']", (uchr_t *)"value", (uchr_t *)st_char_get(data->value));
	}

	// Update the message field.
	if ((data = http_data_get(con, HTTP_DATA_POST, "your_message")) && data->value && (escaped = xml_encode(page->doc_obj, data->value))) {
		xml_set_xpath_ns(page->xpath_ctx, (xmlChar *) "//xhtml:textarea[@id='your_message']", escaped);
	}

	mm_cleanup(escaped);

	if (!(raw = xml_dump_doc(page->doc_obj))) {
		contact_print_message(con, "Contact", "An error occurred while trying to process your request. Please try again in a few minutes.");
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
 * @brief	Process all user contact requests.
 * @param	con		a pointer to the connection object generating the contact request.
 * @param	branch	a null-terminated string specifying the request type (can be "Abuse" or "Contact").
 */
void contact_process(connection_t *con, chr_t *branch) {

	// Access to the contact form requires SSL.
	if (con_secure(con) != 1) {

		if (!st_cmp_cs_eq(NULLER(branch), PLACER("Abuse", 5))) {
			http_print_301(con, "/report_abuse", 1);
		}
		else {
			http_print_301(con, "/contact", 1);
		}

		return;
	}

	if (!con->http.pairs) {
		con->http.mode = HTTP_PARSE_PAIRS;
		return;
	}

	// Check for abuse.
	if (contact_abuse_checks(con, branch)) {
		return;
	}

	// If the form is being submitted, process the business rules.
	if (http_data_get(con, HTTP_DATA_POST, "submit")) {
		contact_business(con, branch);
		return;
	}

	contact_print_form(con, branch, NULL);

	return;
}
