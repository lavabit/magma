/*
 * @file /check/magma/servers/pop/pop_check.h
 *
 * @brief POP interface test functions.
 */

#ifndef POP_CHECK_H
#define POP_CHECK_H

/// pop_check_network.c
bool_t 	check_pop_client_read_lines_to_end(client_t *client);
bool_t 	check_pop_network_basic_sthread(stringer_t *errmsg, uint32_t port);

/// pop_check.c
Suite * suite_check_pop(void);

#endif
