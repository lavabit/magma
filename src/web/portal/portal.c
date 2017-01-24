
/**
 * @file /magma/web/portal/portal.c
 *
 * @brief	The portal web application.
 */

#include "magma.h"

/**
 * @brief	Display the http portal login page.
 * @note	The portal login page can be found in 'portal/login.template'
 * @param	con			a pointer to the connection object of the client requesting the portal login page.
 * @param	message		a pointer to a null-terminated string that will be displayed to the user in the login page
 * 						as the contents of the node with the id of "message" in the portal login template.
 * @return	This function returns no value.
 */
void portal_print_login(connection_t *con, chr_t *message) {

	xmlNodePtr node;
	stringer_t *raw;
	http_page_t *page;
	xmlXPathObjectPtr xpath_obj = NULL;

	if ((page = http_page_get("portal/login")) == NULL) {
		http_print_500(con);
		return;
	}

	// Set the message.
	if (message != NULL && (xpath_obj = xml_xpath_eval((uchr_t *)"//xhtml:p[@id='message']", page->xpath_ctx)) != NULL && xpath_obj->nodesetval != NULL &&
		xpath_obj->nodesetval->nodeNr != 0 && xpath_obj->nodesetval->nodeTab[0] != NULL) {
		node = (xmlNodePtr)xpath_obj->nodesetval->nodeTab[0];
		xml_node_set_content(node, (uchr_t *)message);
	}

	if ((raw = xml_dump_doc(page->doc_obj)) == NULL) {
		http_print_500(con);
		http_page_free(page);
		return;
	}

	http_response_header(con, 200, page->content->type, st_length_get(raw));
	//con_print(con, "HTTP/1.1 200 OK\r\nContent-Type: %.*s\r\nContent-Length: %u\r\n\r\n", st_length_get(page->content->type), st_char_get(page->content->type), st_length_get(raw));
	con_write_st(con, raw);
	http_page_free(page);
	st_free(raw);
	return;
}

/**
 * @brief	Process a connection to the web portal.
 * @note	If magma.web.portal.safeguard is set, the user will be redirected to a secure login.
 * @param	con		a pointer to the connection of the http client requesting the portal.
 * @return	This function returns no value.
 */
void portal_process(connection_t *con) {


	// Is HTTPS required to access to the portal from anywhere but the localhost.
	if (magma.web.portal.safeguard && con_secure(con) != 1 && con_addr_word(con, 0) != 0x0100007f) {
		http_print_301(con, "/portal", 1);
		return;
	}

	// Try extracting the session from either a cookie, or the location.
	http_parse_context(con, PLACER("portal", 6), PLACER("/portal/", 8));

	// QUESTION: Maybe not this hello world?
	portal_print_login(con, "Hello world.");
	return;
}
