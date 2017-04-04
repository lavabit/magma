/*
 * @file /check/magma/servers/camel/camel_check.h
 *
 * @brief Camelface test functions.
 */

#ifndef CAMEL_CHECK_H
#define CAMEL_CHECK_H

/// camel_check_network.c
bool_t check_camel_status(client_t *client);
stringer_t* check_camel_read_json(client_t *client, size_t length);
bool_t check_camel_login(client_t *client, uint32_t id, chr_t *user, chr_t *pass, stringer_t *cookie);
bool_t check_camel_login_sthread(client_t *client, stringer_t *errmsg);
bool_t check_camel_basic_sthread(client_t *client, stringer_t *errmsg);

/// pop_check.c
Suite * suite_check_camel(void);

#endif
