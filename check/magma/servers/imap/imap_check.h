/**
 * @file /check/magma/servers/imap/imap_check.h
 *
 * @brief IMAP interface test functions.
 */

#ifndef IMAP_CHECK_H
#define IMAP_CHECK_H

/// imap_check.h
bool_t imap_client_read_line_to_end(client_t*);

Suite * suite_check_imap(void);

#endif
