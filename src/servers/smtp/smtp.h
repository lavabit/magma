
/**
 * @file /magma/servers/smtp/smtp.h
 *
 * @brief The entry point for the SMTP server module.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_SERVERS_SMTP_H
#define MAGMA_SERVERS_SMTP_H

/// accept.c
int_t   smtp_accept_message(connection_t *con, smtp_inbound_prefs_t *prefs);
int_t   smtp_rollout(smtp_inbound_prefs_t *prefs);
int_t   smtp_store_message(smtp_inbound_prefs_t *prefs, stringer_t **local);
bool_t  smtp_store_spamsig(smtp_inbound_prefs_t *prefs, int_t spam);

/// checkers.c
int_t   smtp_check_filters(smtp_inbound_prefs_t *prefs, stringer_t **local);
int_t   smtp_check_greylist(connection_t *con, smtp_inbound_prefs_t *prefs);
int_t   smtp_check_rbl(connection_t *con);
bool_t  smtp_add_bypass_entry(stringer_t *subnet);
bool_t  smtp_bypass_check(connection_t *con);

/// commands.c
void smtp_requeue(connection_t *con);
int_t   smtp_compare(const void *compare, const void *command);
void    smtp_process(connection_t *con);
void    smtp_sort(void);

/// datatier.c
int_t         smtp_check_authorized_from(uint64_t usernum, stringer_t *address);
int_t         smtp_check_receive_quota(connection_t *con, smtp_inbound_prefs_t *prefs);
int_t         smtp_check_transmit_quota(uint64_t usernum, size_t num_recipients, smtp_outbound_prefs_t *prefs);
int_t         smtp_fetch_authorization(credential_t *cred, smtp_outbound_prefs_t **output);
stringer_t *  smtp_fetch_autoreply(uint64_t autoreply, uint64_t usernum);
int_t         smtp_fetch_inbound(credential_t *cred, stringer_t *address, smtp_inbound_prefs_t **output);
table_t *     smtp_fetch_rollmessages(uint64_t usernum);
int_t         smtp_get_action(chr_t *string, size_t length);
uint64_t      smtp_insert_spamsig(smtp_inbound_prefs_t *prefs, uint64_t key, int_t code);
void          smtp_update_receive_stats(connection_t *con, smtp_inbound_prefs_t *prefs);
void          smtp_update_transmission_stats(connection_t *con);

/// smtp.c
void   smtp_auth_login(connection_t *con);
void   smtp_auth_plain(connection_t *con);
void   smtp_data(connection_t *con);
void   smtp_data(connection_t *con);
void   smtp_disabled(connection_t *con);
void   smtp_ehlo(connection_t *con);
void   smtp_helo(connection_t *con);
void   smtp_init(connection_t *con);
void   smtp_invalid(connection_t *con);
void   smtp_mail_from(connection_t *con);
void   smtp_noop(connection_t *con);
void   smtp_quit(connection_t *con);
void   smtp_rcpt_to(connection_t *con);
void   smtp_rset(connection_t *con);
void   smtp_starttls(connection_t *con);
void   submission_init(connection_t *con);

/// parse.c
stringer_t *  smtp_parse_auth(stringer_t *data);
stringer_t *  smtp_parse_helo_domain(connection_t *con);
stringer_t *  smtp_parse_mail_from_path(connection_t *con);
stringer_t *  smtp_parse_rcpt_to(connection_t *con);

/// relay.c
void        smtp_client_close(client_t *client);
client_t *  smtp_client_connect(int_t premium);
int_t       smtp_client_send_data(client_t *client, stringer_t *message);
int_t       smtp_client_send_helo(client_t *client);
int_t       smtp_client_send_mailfrom(client_t *client, stringer_t *mailfrom, size_t send_size);
int_t       smtp_client_send_nullfrom(client_t *client);
int_t       smtp_client_send_rcptto(client_t *client, stringer_t *rcptto);

/// session.c
void    smtp_add_inbound(connection_t *con, smtp_inbound_prefs_t *inbound);
void    smtp_add_outbound(connection_t *con, smtp_outbound_prefs_t *outbound);
bool_t   smtp_add_recipient(connection_t *con, stringer_t *address);
bool_t   smtp_check_duplicate_recipient(connection_t *con, uint64_t usernum);
void    smtp_free_inbound(smtp_inbound_prefs_t *inbound);
void    smtp_free_outbound(smtp_outbound_prefs_t *outbound);
void    smtp_free_recipients(smtp_recipients_t *recipients);
void    smtp_list_free_filter(smtp_inbound_filter_t *filter);
void    smtp_session_destroy(connection_t *con);
void    smtp_session_reset(connection_t *con);

/// transmit.c
int_t   smtp_bounce(connection_t *con);
int_t   smtp_forward_message(server_t *server, stringer_t *address, stringer_t *message, stringer_t *id, int_t mark, uint64_t signum, uint64_t sigkey);
int_t   smtp_relay_message(connection_t *con, stringer_t **result);
int_t   smtp_reply(stringer_t *from, stringer_t *to, uint64_t usernum, uint64_t autoreply, int_t spf, int_t dkim);
int_t   smtp_send_message(stringer_t *to, stringer_t *from, stringer_t *message);

#endif

