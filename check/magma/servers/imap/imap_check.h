/**
 * @file /check/magma/servers/imap/imap_check.h
 *
 * @brief IMAP interface test functions.
 */

#ifndef IMAP_CHECK_H
#define IMAP_CHECK_H

/// imap_check_network.c
bool_t check_imap_client_read_end(client_t *client, chr_t *tag);
bool_t check_imap_client_login(client_t *client, chr_t *user, chr_t *pass, chr_t *tag, stringer_t *errmsg);
bool_t check_imap_client_select(client_t *client, chr_t *folder, chr_t *tag, stringer_t *errmsg);
bool_t check_imap_client_close_logout(client_t *client, uint32_t tag_num, stringer_t *errmsg);
bool_t check_imap_network_basic_sthread(stringer_t *errmsg, uint32_t port, bool_t secure);
bool_t check_imap_network_search_sthread(stringer_t *errmsg, uint32_t port, bool_t secure);
bool_t check_imap_network_fetch_sthread(stringer_t *errmsg, uint32_t port, bool_t secure);

Suite * suite_check_imap(void);

#endif
