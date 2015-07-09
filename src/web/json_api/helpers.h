#ifndef MAGMA_WEB_JSON_API_HELPERS_H
#define MAGMA_WEB_JSON_API_HELPERS_H

/**
 * @brief
 *  Return a json-rpc error response to the remote client.
 * @param con
 *  a pointer to the connection object across which the response will be sent.
 * @param http_code
 *  the http response code to be sent to the remote client in the response
 *  header.
 * @param error_code
 *  the numerical error code to be encoded in the json-rpc error message.
 * @param message
 *  a descriptive error string to be encoded in the json-rpc error message.
 * @return
 *  This function returns no value.
 */
void
api_error(
	connection_t *con,
	int_t http_code,
	int_t error_code,
	chr_t *message);

/**
 * @brief
 *  Generate a json-rpc 2.0 response to a portal request.
 * @see
 *  json_vpack_ex()
 * @note
 *  This function indents the json response if specified in the configuration,
 *  and also automatically decreases the reference count of any json object
 *  that was packed for the reply.
 * @param con
 *  a pointer to the connection object across which the portal
 *  response will be sent.
 * @param format
 *  a pointer to a format string specifying the construction of the json-rpc
 *  response.
 * @param ...
 *  a variable arguments style list of parameters to be passed to the json
 *  packing function.
 * @return
 *  This function returns no value.
 */
void
api_response(
	connection_t *con,
	int_t http_code,
	chr_t *format,
	...);

#endif
