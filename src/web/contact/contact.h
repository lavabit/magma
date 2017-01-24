
/**
 * @file /magma/web/contact/contact.h
 *
 * @brief	Definitions for handling the web contact form.
 */

#ifndef MAGMA_WEB_CONTACT_H
#define MAGMA_WEB_CONTACT_H

/// abuse.c
bool_t  contact_abuse_checks(connection_t *con, chr_t *branch);
void    contact_abuse_increment_history(connection_t *con);

/// business.c
void           contact_business(connection_t *con, chr_t *branch);
http_page_t *  contact_business_add_error(chr_t *branch, uchr_t *xpath, uchr_t *id, uchr_t *message);
bool_t         contact_business_valid_email(stringer_t *email);

/// contact.c
void   contact_print_form(connection_t *con, chr_t *branch, http_page_t *page);
void   contact_print_message(connection_t *con, chr_t *branch, chr_t *message);
void   contact_process(connection_t *con, chr_t *branch);

#endif

