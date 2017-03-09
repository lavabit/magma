/**
 * @file /check/magma/servers/imap/imap_check.h
 *
 * @brief IMAP interface test functions.
 */

#ifndef IMAP_CHECK_H
#define IMAP_CHECK_H

/// imap_check.h
bool_t check_imap_client_read_lines_to_end(client_t*, chr_t*);
bool_t check_imap_network_simple_sthread(stringer_t*);

Suite * suite_check_imap(void);

#endif
