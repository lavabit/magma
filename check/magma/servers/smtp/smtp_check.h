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
bool_t check_smtp_checkers_rbl_sthread(stringer_t *errmsg);
bool_t check_smtp_checkers_regex_sthread(stringer_t *errmsg);
bool_t check_smtp_checkers_greylist_sthread(stringer_t *errmsg);
bool_t check_smtp_checkers_filters_sthread(stringer_t *errmsg, int_t action, int_t expected);

/// smtp_check_network.c
bool_t check_smtp_client_read_end(client_t *client);
bool_t check_smtp_client_quit(client_t *client, stringer_t *errmsg);
bool_t check_smtp_client_auth_plain(client_t *client, stringer_t *auth);
bool_t check_smtp_network_auth_sthread(stringer_t *errmsg, uint32_t port, bool_t login);
bool_t check_smtp_client_auth_login(client_t *client, stringer_t *user, stringer_t *pass);
bool_t check_smtp_network_basic_sthread(stringer_t *errmsg, uint32_t port, bool_t secure);
bool_t check_smtp_network_outbound_quota_sthread(stringer_t *errmsg, uint32_t port, bool_t secure);
bool_t check_smtp_network_starttls_sthread(stringer_t *errmsg, uint32_t tcp_port, uint32_t tls_port);
bool_t check_smtp_client_mail_rcpt_data(client_t *client, chr_t *from, chr_t *to, stringer_t *errmsg);
bool_t check_smtp_network_locked_rcpt_sthread(stringer_t *errmsg, uint32_t port, stringer_t *from, stringer_t *recipient, auth_lock_status_t lock);
bool_t check_smtp_network_locked_auth_sthread(stringer_t *errmsg, uint32_t port, bool_t login, stringer_t *username, stringer_t *password, auth_lock_status_t lock);

Suite * suite_check_smtp(void);

#endif
