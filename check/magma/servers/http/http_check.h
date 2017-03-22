/**
 * @file /check/magma/servers/smtp/smtp_check.h
 *
 * @brief SMTP interface test functions.
 */

#ifndef HTTP_CHECK_H
#define HTTP_CHECK_H

/// http_check_network.c
bool_t check_http_read_to_empty(client_t *client);
uint32_t check_http_content_length_get(client_t *client, stringer_t *errmsg);
bool_t check_http_content_length_test(client_t *client, uint32_t content_length, stringer_t *errmsg);
bool_t check_http_network_basic_sthread(stringer_t *errmsg, uint32_t port, bool_t secure);

Suite * suite_check_http(void);

#endif
