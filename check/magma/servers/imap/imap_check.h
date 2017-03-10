/**
 * @file /check/magma/servers/imap/imap_check.h
 *
 * @brief IMAP interface test functions.
 */

#ifndef IMAP_CHECK_H
#define IMAP_CHECK_H

/// imap_check_network.c
bool_t check_imap_client_read_lines_to_end(client_t* client, chr_t* token);
bool_t check_imap_network_basic_sthread(stringer_t* errmsg, uint32_t port);

Suite * suite_check_imap(void);

#endif
