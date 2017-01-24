
/**
 * @file /magma/web/contact/business.c
 *
 * @brief	Functions for handling the logic of the contact form.
 */

#include "magma.h"

/**
 * @brief	Return the contact/abuse page with a marked error indicator for the user in the event of a user submission error.
 * @param	branch		a null-terminated string containing the source  of the error: "Abuse" or "Contact"
 * @param	xpath		a null-terminated string containing the xpath of the element in the returned page that should be colored red.
 * @param	id			a null-terminated string containing the id of the error text <span> element to be added to the document.
 * @param	message		a null-terminated string containing the actual error message text to be displayed to the user.
 * @return	NULL on failure, or a pointer to the processed contact page with error message on success.
 */
http_page_t * contact_business_add_error(chr_t *branch, uchr_t *xpath, uchr_t *id, uchr_t *message) {

	http_page_t *page = NULL;
	xmlNodePtr node, error;
	xmlXPathObjectPtr xpath_obj;

	// We are here either for the contact or abuse page.
	if (!st_cmp_cs_eq(NULLER(branch), PLACER("Abuse", 5)) && !((page = http_page_get("contact/abuse")))) {
		return NULL;
	}
	else if (!page && !(page = http_page_get("contact/contact"))) {
		return NULL;
	}

	if ((xpath_obj = xml_xpath_eval(xpath, page->xpath_ctx)) && xpath_obj->nodesetval &&
		xpath_obj->nodesetval->nodeNr && xpath_obj->nodesetval->nodeTab[0]) {

		node = (xmlNodePtr)xpath_obj->nodesetval->nodeTab[0];

		// Make the field red.
		xml_node_set_property(node, (uchr_t *)"class", (uchr_t *)"red");

		// Add the error message.
		if ((error = xml_node_new((uchr_t *)"span"))) {
			xml_node_set_property(error, (uchr_t *)"id", id);
			xml_node_set_content(error, message);

			if (!(xml_node_add_sibling(node, error))) {
				xml_node_free(error);
			}

		}
	}

	return page;
}

/**
 * @brief	Validate an email address.
 * @param	email	a managed string containing the email address to be validated.
 * @return	true if the specified email was valid and false if it was not.
 */
bool_t contact_business_valid_email(stringer_t *email) {

	chr_t *holder;
	size_t length, increment;


	if (!email) {
		return false;
	}

	// Lower case the string.
	lower_st(email);

	holder = st_char_get(email);
	length = st_length_get(email);

	if (!length) {
		return false;
	}

	// Catch leading at symbols.
	if (*holder == '@') {
		return false;
	}

	// The address.
	for (increment = 0; increment < length && *(holder + increment) != '@'; increment++) {
		if (*(holder + increment) < '!' || *(holder + increment) > '~') {
			return false;
		}
	}

	// Make sure we have an at symbol.
	if (increment == length || *(holder + increment++) != '@' || *(holder + increment) == '.' || increment == length) {
		return false;
	}

	// Look for the TLD seperator.
	while (increment < length && *(holder + increment) != '.') {
		if ((*(holder + increment) < 'a' || *(holder + increment) > 'z') && *(holder + increment) != '-') {
			return false;
		}
		increment++;
	}

	// Make sure we have a domain name.
	if (increment == length || *(holder + increment++) != '.' || increment == length) {
		return false;
	}

	// Go through the rest.
	while (increment < length) {
		if ((*(holder + increment) < 'a' || *(holder + increment) > 'z') && *(holder + increment) != '-' && *(holder + increment) != '.') {
			return false;
		}
		increment++;
	}

	return true;
}

/**
 * @brief	Send the contents of a user-submitted contact or abuse form to the magma-configured contact email address.
 * @param	con		the connection of the web client making the request.
 * @param	branch	a null-terminated string containing the destination of the contact form: either "Abuse" or "Contact".
 * @return	This function returns no value.
 */
void contact_business(connection_t *con, chr_t *branch) {

	stringer_t *composed, *ip, *daddr;
	http_data_t *name, *email, *message;
	bool_t is_contact;

	// A contact form submission by default
	if (!st_cmp_cs_eq(NULLER(branch), PLACER("Abuse", 5))) {
		is_contact = false;
		daddr = magma.admin.abuse;
	} else {
		is_contact = true;
		daddr = magma.admin.contact;
	}

	// One last sanity check, just in case.
	if (is_contact && !daddr) {
		contact_print_message(con, branch, "An unexpected error occurred while trying to process your request. This server does not have a contact address configured.");
		log_error("Contact form routine was reached but no contact address was configured.");
		return;
	}
	else if (!is_contact && !daddr) {
		contact_print_message(con, branch, "An unexpected error occurred while trying to process your request. This server does not have an abuse address configured.");
		log_error("Abuse form routine was reached but no abuse address was configured.");
		return;
	}

	// If either the name, email, or message fields are omitted from the post, spit out an error message.
	if (!(name = http_data_get(con, HTTP_DATA_POST, "your_name")) || !name->value) {
		contact_print_form(con, branch, contact_business_add_error(branch, (uchr_t *)"//xhtml:input[@id='your_name']", (uchr_t *)"your_name_msg", (uchr_t *)"Please enter your name."));
		return;
	}
	else if (!(email = http_data_get(con, HTTP_DATA_POST, "your_email")) || !email->value || !contact_business_valid_email(email->value)) {
		contact_print_form(con, branch, contact_business_add_error(branch,  (uchr_t *)"//xhtml:input[@id='your_email']",  (uchr_t *)"your_email_msg",  (uchr_t *)"A valid e-mail address is required."));
		return;
	}
	else if (!(message = http_data_get(con, HTTP_DATA_POST, "your_message")) || !message->value) {
		contact_print_form(con, branch, contact_business_add_error(branch,  (uchr_t *)"//xhtml:textarea[@id='your_message']",  (uchr_t *)"your_message_msg",  (uchr_t *)"Please enter your message."));
		return;
	}

	// Build the IP string.
	ip = con_addr_presentation(con, MANAGEDBUF(64));

	if ((composed = st_merge("nnnsnsnsnsnsnsnsn", "Subject: ", (is_contact ? "* Contact Form Submission *" : "*Abuse Form Submission*"),
		"\r\nFrom: ", magma.admin.contact, "\r\nTo: ", magma.admin.contact, "\r\nReply-To: ",
		email->value, "\r\n\r\nip = ", ip, "\r\nname = ", name->value, "\r\nemail = ", email->value, "\r\n------------\r\n\r\n ", message->value, "\r\n"))) {

		// Dotstuff the message. Just to prevent SMTP injection attacks.
		st_replace(&composed, PLACER("\n.",2), PLACER("\n..", 3));

		// Send the message.
		if (smtp_send_message(daddr, daddr, composed) != 1) {
			contact_print_message(con, branch, "An error occurred while trying to process your request. Please try again in a few minutes.");
			st_free(composed);
			return;
		}

		// Increment our abuse counters.
		contact_abuse_increment_history(con);
		st_free(composed);
	}

	contact_print_message(con, branch, "Your message has been submitted. We'll review your message-typically within one business day-and get back to you. However "
		"during certain busy periods our team can take up to three business days to respond.");

	return;
}
