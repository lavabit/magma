/*
 * @file /check/magma/servers/pop/pop_check.h
 *
 * @brief POP interface test functions.
 */

#ifndef POP_CHECK_H
#define POP_CHECK_H

/// pop_check_network.c
uint64_t check_pop_client_read_list(client_t *client, stringer_t *errmsg);
bool_t check_pop_client_read_end(client_t *client, uint64_t* size, chr_t *token);
bool_t 	check_pop_network_basic_sthread(stringer_t *errmsg, uint32_t port, bool_t secure);
bool_t check_pop_client_auth(client_t *client, chr_t *user, chr_t *pass, stringer_t *errmsg);
bool_t check_pop_network_stls_ad_sthread(stringer_t *errmsg, uint32_t tcp_port, uint32_t tls_port);

/// pop_check.c
Suite * suite_check_pop(void);

#endif
