#ifndef MAGMA_WEB_JSON_API_ENDPOINTS_H
#define MAGMA_WEB_JSON_API_ENDPOINTS_H

void api_endpoint_auth(connection_t *con);
void api_endpoint_register(connection_t *con);
void api_endpoint_register_legacy(connection_t *con);
void api_endpoint_change_password(connection_t *con);
void api_endpoint_migrate_account(connection_t *con);

#endif
