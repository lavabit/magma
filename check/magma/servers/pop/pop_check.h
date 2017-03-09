/*
 * @file /check/magma/servers/pop/pop_check.h
 *
 * @brief POP interface test functions.
 */

#ifndef POP_CHECK_H
#define POP_CHECK_H

/// pop_check.c
bool_t pop_client_read_lines(client_t*, uint32_t);

Suite * suite_check_pop(void);

#endif
