
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
bool_t check_smtp_accept_accept_message_sthread(stringer_t *errmsg);

/// checkers_check.c
bool_t check_smtp_checkers_greylist_sthread(stringer_t*);

/// commands_check.c

/// datatier_check.c

/// messages_check.c

/// parse_check.c

/// relay_check.c

/// session_check.c

/// smtp_check.c

/// transmit_check.c

Suite * suite_check_smtp(void);

#endif

