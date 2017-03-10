/**
 * @file /check/magma/servers/smtp/smtp_check.h
 *
 * @brief SMTP interface test functions.
 */

#ifndef SMTP_CHECK_H
#define SMTP_CHECK_H

/// accept_check.c
bool_t check_smtp_accept_message_sthread(stringer_t *errmsg);

/// checkers_check.c
bool_t check_smtp_checkers_greylist_sthread(stringer_t *errmsg);
bool_t check_smtp_checkers_regex_sthread(stringer_t *errmsg);
bool_t check_smtp_checkers_filters_sthread(stringer_t *errmsg, int_t action, int_t expected);

/// smtp_check_network.c
bool_t check_smtp_client_read_line_to_end(client_t *client);
bool_t check_smtp_network_simple_sthread(stringer_t *errmsg, uint32_t port);

Suite * suite_check_smtp(void);

#endif

