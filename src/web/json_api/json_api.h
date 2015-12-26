
/**
 * @file /magma/src/web/json_api/json_api.h
 *
 * @brief The the JSON API interface functions.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_WEB_JSON_API_H
#define MAGMA_WEB_JSON_API_H

void json_api_dispatch(connection_t *con);

void api_endpoint_auth(connection_t *con);
void api_endpoint_register(connection_t *con);
void api_endpoint_delete_user(connection_t *con);
void api_endpoint_change_password(connection_t *con);

void api_error(connection_t *con, int_t http_code, int_t error_code, chr_t *message);
void api_response(connection_t *con, int_t http_code, chr_t *format, ...);

#endif
