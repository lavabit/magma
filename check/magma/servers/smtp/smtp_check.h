
/**
 * @file /check/magma/servers/smtp/smtp_check.h
 *
 * @brief SMTP interface test functions.
 */

#ifndef SMTP_CHECK_H
#define SMTP_CHECK_H

/// accept_check.c
bool_t check_smtp_accept_store_message_sthread(stringer_t*);
bool_t check_smtp_accept_rollout_sthread(stringer_t *errmsg);
bool_t check_smtp_accept_store_spamsig_sthread(stringer_t *errmsg);

/// checkers_check.c
bool_t check_smtp_checkers_greylist_sthread(stringer_t*);

/// smtp_check.h
bool_t smtp_client_read_line_to_end(client_t*);

Suite * suite_check_smtp(void);

#endif

