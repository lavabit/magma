#ifndef MAGMA_WEB_JSON_API_H
#define MAGMA_WEB_JSON_API_H

/**
 * @brief
 *  The entry point for json api requests
 * @param con
 *  The connection object corresponding to the web client making the request.
 * @return
 *  This function returns no value.
 */
void json_api_dispatch(connection_t *con);

// NOTE - this is TEMPORARY until the magma header dependencies can be sorted!
#include "endpoints.h"
#include "helpers.h"

#endif
