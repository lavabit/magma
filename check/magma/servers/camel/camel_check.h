/*
 * @file /check/magma/servers/camel/camel_check.h
 *
 * @brief Camelface test functions.
 */

#ifndef CAMEL_CHECK_H
#define CAMEL_CHECK_H

/// camel_check_network.c
bool_t check_camel_basic_sthread(client_t *client, stringer_t *errmsg);

/// pop_check.c
Suite * suite_check_camel(void);

#endif
