
/**
 *
 * @file /magma/engine/config/servers/servers.c
 *
 * @brief	Functions for handling the server instance configurations.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"
#include "keys.h"

/**
 * @brief	Get the number of servers that are configured to use a specified port.
 * @param	port	the port number to be matched against the server list.
 * @return	the number of servers configured to use the port.
 */
uint64_t servers_get_count_using_port(uint32_t port) {

	uint64_t count = 0;

	for (uint32_t i = 0; i < MAGMA_SERVER_INSTANCES; i++) {
		if (magma.servers[i] && magma.servers[i]->network.port == port)
			count++;
	}

	return count;
}

/**
 * @brief	Lookup the server instance associated with a socket descriptor.
 * @param	sockd	the socket file descriptor to be looked up.
 * @return	NULL on failure, or a pointer to the server structure for the specified file descriptor on success.
 */
inline server_t * servers_get_by_socket(int sockd) {

	server_t *result = NULL;

	for (uint64_t i = 0; result == NULL && i < MAGMA_SERVER_INSTANCES; i++)
		if (magma.servers[i] && magma.servers[i]->network.sockd == sockd)
			result = magma.servers[i];

	return result;
}

/**
 * @brief	Setup the listening sockets for all configured servers, or shut them all down if any one of them fails.
 * @note	If any of the servers have an empty name or domain, the value of magma.host.name will be used instead.
 * @return	true if all server listening sockets were initialized successfully, or false on failure.
 */
bool_t servers_network_start(void) {
	for (uint32_t i = 0; i < MAGMA_SERVER_INSTANCES; i++) {
		if (magma.servers[i]) {
			// If the server name, or domain are NULL, use the hostname as the default value. Then initialize the socket.
			if ((st_empty(magma.servers[i]->name) && !(magma.servers[i]->name = st_import(magma.host.name, ns_length_get(magma.host.name)))) ||
					(st_empty(magma.servers[i]->domain) && !(magma.servers[i]->domain = st_import(magma.host.name, ns_length_get(magma.host.name)))) ||
					!net_init(magma.servers[i])) {
				servers_network_stop();
				return false;
			}
		}
	}
	return true;
}

/**
 * @brief	Create an SSL context for each SSL-configured server instance.
 * @return	true on success or false on failure.
 */
bool_t servers_encryption_start(void) {
	// Loop through and setup the transport security layer for all of the server instances that provided SSL/TLS certificates.
	for (uint32_t i = 0; i < MAGMA_SERVER_INSTANCES; i++) {

		// The trenary increases the security level for DMTP (and DMAP in the future) connections, which require TLSv1.2 and only allow
		// two cipher suites. Otherwise we allow any version of TSLv1.0 and higher, and any ciphersuite with forward secrecy.
		if (magma.servers[i] && magma.servers[i]->ssl.certificate &&
			!ssl_server_create(magma.servers[i], magma.servers[i]->protocol == DMTP ? 3 : 2)) {
			return false;
		}
	}
	return true;
}

/**
 * @brief	Close the listening sockets of all running servers.
 * @return	This function returns no value.
 */
void servers_network_stop(void) {
	for (uint32_t i = 0; i < MAGMA_SERVER_INSTANCES; i++) {
		if (magma.servers[i] && magma.servers[i]->network.sockd != -1) {
				net_shutdown(magma.servers[i]);
		}
	}
	return;
}

/**
 * @brief	Destroy the contexts of all SSL-enabled servers.
 * @return	This function returns no value.
 */
void servers_encryption_stop(void) {
	for (uint32_t i = 0; i < MAGMA_SERVER_INSTANCES; i++) {
		if (magma.servers[i] && magma.servers[i]->ssl.context) {
			ssl_server_destroy(magma.servers[i]);
		}
	}
	return;
}

/**
 * @brief	Free magma's  server configuration options.
 * @note	This function assumes all worker threads have finished, and should only be called during the normal shutdown process.
 * @param	This function returns no value.
 */
void servers_free(void) {

	for (uint32_t i = 0; i < MAGMA_SERVER_INSTANCES; i++) {

		for (uint64_t j = 0; magma.servers[i] && j < sizeof(server_keys) / sizeof(server_keys_t); j++) {
			switch (server_keys[j].norm.type) {
			case (M_TYPE_BLOCK):
				if (*((void **)(((char *)magma.servers[i]) + server_keys[j].offset))) {
					mm_free(*((void **)(((char *)magma.servers[i]) + server_keys[j].offset)));
				}
				break;
			case (M_TYPE_NULLER):
				if (!ns_empty(*((char **)(((char *)magma.servers[i]) + server_keys[j].offset)))) {
					ns_free(*((char **)(((char *)magma.servers[i]) + server_keys[j].offset)));
				}
				break;
			case (M_TYPE_STRINGER):
				if (!st_empty(*((stringer_t **)(((char *)magma.servers[i]) + server_keys[j].offset)))) {
					st_free(*((stringer_t **)(((char *)magma.servers[i]) + server_keys[j].offset)));
				}
				break;
			default:
#ifdef MAGMA_PEDANTIC
			if (server_keys[j].norm.type != M_TYPE_BOOLEAN && server_keys[j].norm.type != M_TYPE_DOUBLE && server_keys[j].norm.type != M_TYPE_FLOAT &&
					server_keys[j].norm.type != M_TYPE_INT16 && server_keys[j].norm.type != M_TYPE_INT32 && server_keys[j].norm.type != M_TYPE_INT64 &&
					server_keys[j].norm.type != M_TYPE_INT8 && server_keys[j].norm.type != M_TYPE_UINT8 && server_keys[j].norm.type != M_TYPE_UINT16 &&
					server_keys[j].norm.type != M_TYPE_UINT32 && server_keys[j].norm.type != M_TYPE_UINT64 && server_keys[j].norm.type != M_TYPE_ENUM) {
				log_pedantic("Unexpected type. {type = %s = %u}", type(server_keys[j].norm.type), server_keys[j].norm.type);
			}
#endif
				break;
			}
		}

		if (magma.servers[i]) {
			mm_free(magma.servers[i]);
			magma.servers[i] = NULL;
		}
	}
	return;
}

/**
 * @brief	Allocate a new magma server configuration entry and initialize it to its default values.
 * @param	number	the index of the magma server instance in the global magma server array.
 * @return	NULL on failure, or a pointer to the requested magma server configuration entry on success.
 */
server_t * servers_alloc(uint32_t number) {

	if (magma.servers[number])
		return magma.servers[number];

	if (!(magma.servers[number] = mm_alloc(sizeof(server_t))))
		return NULL;

	// Loop through and set the default values.
	for (uint64_t i = 0; i < sizeof(server_keys) / sizeof(server_keys_t); i++) {
		if (!server_keys[i].required && !servers_set_value(&server_keys[i], magma.servers[number], NULL)) {
			log_critical("magma.servers%s has an invalid default value.", server_keys[i].name);
			return NULL;
		}
	}

	// Set the default value to -1 so the shutdown function can detect uninitialized sockets.
	magma.servers[number]->network.sockd = -1;

	return magma.servers[number];
}

/**
 * @brief	Validate all of the configured magma server instances.
 * @note	This function also checks to see that no servers are binding to the same port.
 * @return	true if all servers have been validated, or false on failure.
 */
bool_t servers_validate(void) {

	chr_t *sval;
	bool_t result = true;

	for (uint32_t i = 0; i < MAGMA_SERVER_INSTANCES; i++) {
		for (uint64_t j = 0; magma.servers[i] && j < sizeof(server_keys) / sizeof(server_keys_t); j++) {

			// Do some very basic file-system validation on ssl certificates
			if (!st_cmp_ci_eq(NULLER(server_keys[j].name), CONSTANT(".ssl.certificate"))) {
				sval = *((char **)(((char *)magma.servers[i]) + server_keys[j].offset));

				if (sval &&  !file_readwritable(sval)) {
					log_critical("SSL certificate cannot be read: { path = %s, error = %s }", sval, strerror_r(errno, bufptr, buflen));
					result = false;
				} else if (sval && file_world_accessible(sval)) {
					log_critical("Warning: SSL certificate has world-access file permissions! Please fix. { path = %s }", sval);
				}

			}

			if (server_keys[j].required) {
				switch (server_keys[j].norm.type) {
				case (M_TYPE_NULLER):
					if (ns_empty(*((char **)(((char *)magma.servers[i]) + server_keys[j].offset)))) {
						log_critical("magma.servers[%u]%s is required and has not been set.", i, server_keys[j].name);
						result = false;
					}
					break;
				case (M_TYPE_STRINGER):
					if (st_empty(*((stringer_t **)(((char *)magma.servers[i]) + server_keys[j].offset)))) {
						log_critical("magma.servers[%u]%s is required and has not been set.", i, server_keys[j].name);
						result = false;
					}
					break;

				case (M_TYPE_ENUM):
					if (!st_cmp_ci_eq(NULLER(server_keys[j].name), CONSTANT(".protocol")) && (*((M_PROTOCOL *)(((char *)magma.servers[i]) + server_keys[j].offset))) == EMPTY) {
						log_critical("magma.servers[%u]%s is required and has not been set.", i, server_keys[j].name);
						result = false;
					} else if (!st_cmp_ci_eq(NULLER(server_keys[j].name), CONSTANT(".network.type")) && (*((M_PORT *)(((char *)magma.servers[i]) + server_keys[j].offset))) == SSL_PORT &&
							ns_empty(magma.servers[i]->ssl.certificate)) {
						log_critical("magma.servers[%u]%s has been configured to use SSL, but no certificate file has been provided.", i, server_keys[j].name);
						result = false;
					}
					break;

				case (M_TYPE_INT8):
					if (*((int8_t *)(((char *)magma.servers[i]) + server_keys[j].offset)) == server_keys[j].norm.val.i8) {
						log_critical("magma.servers[%u]%s is required and has not been set.", i, server_keys[j].name);
						result = false;
					}
					break;

				case (M_TYPE_INT16):
					if (*((int16_t *)(((char *)magma.servers[i]) + server_keys[j].offset)) == server_keys[j].norm.val.i16) {
						log_critical("magma.servers[%u]%s is required and has not been set.", i, server_keys[j].name);
						result = false;
					}
					break;

				case (M_TYPE_INT32):
					if (*((int32_t *)(((char *)magma.servers[i]) + server_keys[j].offset)) == server_keys[j].norm.val.i32) {
						log_critical("magma.servers[%u]%s is required and has not been set.", i, server_keys[j].name);
						result = false;
					}
					break;

				case (M_TYPE_INT64):
					if (*((int64_t *)(((char *)magma.servers[i]) + server_keys[j].offset)) == server_keys[j].norm.val.i64) {
						log_critical("magma.servers[%u]%s is required and has not been set.", i, server_keys[j].name);
						result = false;
					}
					break;

				case (M_TYPE_UINT8):
					if (*((uint8_t *)(((char *)magma.servers[i]) + server_keys[j].offset)) == server_keys[j].norm.val.u8) {
						log_critical("magma.servers[%u]%s is required and has not been set.", i, server_keys[j].name);
						result = false;
					}
					break;

				case (M_TYPE_UINT16):
					if (*((uint16_t *)(((char *)magma.servers[i]) + server_keys[j].offset)) == server_keys[j].norm.val.u16) {
						log_critical("magma.servers[%u]%s is required and has not been set.", i, server_keys[j].name);
						result = false;
					}
					break;

				case (M_TYPE_UINT32):
					if (*((uint32_t *)(((char *)magma.servers[i]) + server_keys[j].offset)) == server_keys[j].norm.val.u32) {
						log_critical("magma.servers[%u]%s is required and has not been set.", i, server_keys[j].name);
						result = false;
					}
					break;

				case (M_TYPE_UINT64):
					if (*((uint64_t *)(((char *)magma.servers[i]) + server_keys[j].offset)) == server_keys[j].norm.val.u64) {
						log_critical("magma.servers[%u]%s is required and has not been set.", i, server_keys[j].name);
						result = false;
					}
					break;

					// Booleans must always be valid, since they only have two possible states. But since implementations vary, we check to make sure the
					// actual value matches what's been defined for true and false.
				case (M_TYPE_BOOLEAN):
					if (*((bool_t *)(((char *)magma.servers[i]) + server_keys[j].offset)) != true && *((bool_t *)(((char *)magma.servers[i]) + server_keys[j].offset)) != false) {
						log_critical("magma.servers[%u]%s requires a valid boolean.", i, server_keys[j].name);

						result = false;
					}
					break;

				default:
					log_pedantic("Unexpected type. {type = %s = %u}", type(server_keys[j].norm.type), server_keys[j].norm.type);
					break;
				}
			}
		}
	}

	// Check for a server that is using the same port more than once.
	for (uint32_t i = 0; i < MAGMA_SERVER_INSTANCES; i++) {
		if (magma.servers[i] && servers_get_count_using_port(magma.servers[i]->network.port) != 1) {
			log_critical("The port %u cannot be assigned to more than one server instance.", magma.servers[i]->network.port);
			return false;
		}
	}

	return result;
}

/**
 * @brief	Log the full contents of the magma server instance settings.
 * @return	This function returns no value.
 */
void servers_output_settings(void) {

	for (uint32_t i = 0; i < MAGMA_SERVER_INSTANCES; i++) {

		if (magma.servers[i])
			log_info("\nServer Instance %u\n--------------------------------------------------------------------------------", i);

		for (uint64_t j = 0; magma.servers[i] && j < sizeof(server_keys) / sizeof(server_keys_t); j++) {

			switch (server_keys[j].norm.type) {
			case (M_TYPE_NULLER):
				if (ns_empty(*((char **)(((char *)magma.servers[i]) + server_keys[j].offset))))
					log_info("magma.servers[%u]%s = NULL", i, server_keys[j].name);
					else
					log_info("magma.servers[%u]%s = %s", i, server_keys[j].name, *((char **)(((char *)magma.servers[i]) + server_keys[j].offset)));
				break;

			case (M_TYPE_STRINGER):
				if (st_empty(*((stringer_t **)(((char *)magma.servers[i]) + server_keys[j].offset))))
					log_info("magma.servers[%u]%s = NULL", i, server_keys[j].name);
					else
					log_info("magma.servers[%u]%s = %.*s", i, server_keys[j].name, st_length_int(*((stringer_t **)(((char *)magma.servers[i]) + server_keys[j].offset))),
								st_char_get(*((stringer_t **)(((char *)magma.servers[i]) + server_keys[j].offset))));
				break;

			case (M_TYPE_ENUM):
				if (!st_cmp_cs_eq(NULLER(server_keys[j].name), CONSTANT(".protocol"))) {
					if (*((M_PROTOCOL *)(((char *)magma.servers[i]) + server_keys[j].offset)) == MOLTEN)
						log_info("magma.servers[%u]%s = %s", i, server_keys[j].name, "MOLTEN");
					else if (*((M_PROTOCOL *)(((char *)magma.servers[i]) + server_keys[j].offset)) == HTTP)
						log_info("magma.servers[%u]%s = %s", i, server_keys[j].name, "HTTP");
					else if (*((M_PROTOCOL *)(((char *)magma.servers[i]) + server_keys[j].offset)) == POP)
						log_info("magma.servers[%u]%s = %s", i, server_keys[j].name, "POP");
					else if (*((M_PROTOCOL *)(((char *)magma.servers[i]) + server_keys[j].offset)) == IMAP)
						log_info("magma.servers[%u]%s = %s", i, server_keys[j].name, "IMAP");
					else if (*((M_PROTOCOL *)(((char *)magma.servers[i]) + server_keys[j].offset)) == SMTP)
						log_info("magma.servers[%u]%s = %s", i, server_keys[j].name, "SMTP");
					else if (*((M_PROTOCOL *)(((char *)magma.servers[i]) + server_keys[j].offset)) == DMTP)
						log_info("magma.servers[%u]%s = %s", i, server_keys[j].name, "DMTP");
					else if (*((M_PROTOCOL *)(((char *)magma.servers[i]) + server_keys[j].offset)) == SUBMISSION)
						log_info("magma.servers[%u]%s = %s", i, server_keys[j].name, "SUBMISSION");
					else if (*((M_PROTOCOL *)(((char *)magma.servers[i]) + server_keys[j].offset)) == EMPTY)
						log_info("magma.servers[%u]%s = %s", i, server_keys[j].name, "EMPTY");
						else
						log_info("magma.servers[%u]%s = %s", i, server_keys[j].name, "UNKNOWN");
				} else if (!st_cmp_cs_eq(NULLER(server_keys[j].name), CONSTANT(".network.type"))) {
					if (*((M_PORT *)(((char *)magma.servers[i]) + server_keys[j].offset)) == TCP_PORT)
						log_info("magma.servers[%u]%s = %s", i, server_keys[j].name, "TCP");
					else if (*((M_PORT *)(((char *)magma.servers[i]) + server_keys[j].offset)) == SSL_PORT)
						log_info("magma.servers[%u]%s = %s", i, server_keys[j].name, "SSL");
						else
						log_info("magma.servers[%u]%s = %s", i, server_keys[j].name, "UNKNOWN");
				}
				break;

			case (M_TYPE_BOOLEAN):
				log_info("magma.servers[%u]%s = %s", i, server_keys[j].name, (*((bool_t *) (((char *)magma.servers[i]) + server_keys[j].offset)) == true ? "true" : "false"));
				break;

			case (M_TYPE_INT8):
				log_info("magma.servers[%u]%s = %hhi", i, server_keys[j].name, *((int8_t *) (((char *)magma.servers[i]) + server_keys[j].offset)));
				break;
			case (M_TYPE_INT16):
				log_info("magma.servers[%u]%s = %hi", i, server_keys[j].name, *((int16_t *) (((char *)magma.servers[i]) + server_keys[j].offset)));
				break;

			case (M_TYPE_INT32):
				log_info("magma.servers[%u]%s = %i", i, server_keys[j].name, *((int32_t *) (((char *)magma.servers[i]) + server_keys[j].offset)));
				break;

			case (M_TYPE_INT64):
				log_info("magma.servers[%u]%s = %li", i, server_keys[j].name, *((int64_t *) (((char *)magma.servers[i]) + server_keys[j].offset)));
				break;

			case (M_TYPE_UINT8):
				log_info("magma.servers[%u]%s = %hhu", i, server_keys[j].name, *((uint8_t *) (((char *)magma.servers[i]) + server_keys[j].offset)));
				break;
			case (M_TYPE_UINT16):
				log_info("magma.servers[%u]%s = %hu", i, server_keys[j].name, *((uint16_t *) (((char *)magma.servers[i]) + server_keys[j].offset)));
				break;
			case (M_TYPE_UINT32):
				log_info("magma.servers[%u]%s = %u", i, server_keys[j].name, *((uint32_t *) (((char *)magma.servers[i]) + server_keys[j].offset)));
				break;
			case (M_TYPE_UINT64):
				log_info("magma.servers[%u]%s = %lu", i, server_keys[j].name, *((uint64_t *) (((char *)magma.servers[i]) + server_keys[j].offset)));
				break;
			default:
				log_pedantic("Unexpected type. {type = %s = %u}", type(server_keys[j].norm.type), server_keys[j].norm.type);
				break;
			}
		}
	}

	return;
}

/**
 * @brief	Set the value of a key for a specified magma server configuration entry.
 * @note	This function will allocate space for a copy of the key value, and return an error if it is not able to convert
 * 			it to the proper key data type.
 * @param	setting		a pointer to the magma server key to be set.
 * @param	server 		a pointer to the magma server configuration entry to be modified.
 * @param	value 		a managed string containing the new value of the key.
 * @return	true if the value was was successfully set, or false on failure.
 */
bool_t servers_set_value(server_keys_t *setting, server_t *server, stringer_t *value) {

	bool_t result = true;

	switch (setting->norm.type) {

	// Strings
	case (M_TYPE_NULLER):
		if (!ns_empty(*((char **)(((char *)server) + setting->offset)))) {
			ns_free(*((char **)(((char *)server) + setting->offset)));
			*((char **)(((char *)server) + setting->offset)) = NULL;
		}
		if (!st_empty(value))
			*((char **)(((char *)server) + setting->offset)) = ns_import(st_char_get(value), st_length_get(value));
		else if (!ns_empty(setting->norm.val.ns))
			*((char **)(((char *)server) + setting->offset)) = ns_dupe(setting->norm.val.ns);
		break;

	case (M_TYPE_STRINGER):
		if (!st_empty(*((stringer_t **)(((char *)server) + setting->offset)))) {
			st_free(*((stringer_t **)(((char *)server) + setting->offset)));
			*((stringer_t **)(((char *)server) + setting->offset)) = NULL;
		}
		if (!st_empty(value))
			*((stringer_t **)(((char *)server) + setting->offset)) = st_dupe_opts(MANAGED_T | CONTIGUOUS | HEAP, value);
		else if (!st_empty(setting->norm.val.st))
			*((stringer_t **)(((char *)server) + setting->offset)) = st_dupe_opts(MANAGED_T | CONTIGUOUS | HEAP, setting->norm.val.st);
		break;

		// Booleans
	case (M_TYPE_BOOLEAN):
		if (!st_empty(value)) {
			if (!st_cmp_ci_eq(value, CONSTANT("true")))
				*((bool_t *)(((char *)server) + setting->offset)) = true;
			else if (!st_cmp_ci_eq(value, CONSTANT("false")))
				*((bool_t *)(((char *)server) + setting->offset)) = false;
			else {
				log_critical("Invalid value for %s.", setting->name);
				result = false;
			}
		} else
			*((bool_t *)(((char *)server) + setting->offset)) = setting->norm.val.binary;
		break;

	case (M_TYPE_ENUM):
		if (!st_cmp_ci_eq(NULLER(setting->name), CONSTANT(".protocol"))) {
			if (!st_cmp_ci_eq(value, CONSTANT("MOLTEN")))
				*((M_PROTOCOL *)(((char *)server) + setting->offset)) = MOLTEN;
			else if (!st_cmp_ci_eq(value, CONSTANT("HTTP")))
				*((M_PROTOCOL *)(((char *)server) + setting->offset)) = HTTP;
			else if (!st_cmp_ci_eq(value, CONSTANT("SMTP")))
				*((M_PROTOCOL *)(((char *)server) + setting->offset)) = SMTP;
			else if (!st_cmp_ci_eq(value, CONSTANT("DMTP")))
				*((M_PROTOCOL *)(((char *)server) + setting->offset)) = DMTP;
			else if (!st_cmp_ci_eq(value, CONSTANT("POP")))
				*((M_PROTOCOL *)(((char *)server) + setting->offset)) = POP;
			else if (!st_cmp_ci_eq(value, CONSTANT("IMAP")))
				*((M_PROTOCOL *)(((char *)server) + setting->offset)) = IMAP;
			else if (!st_cmp_ci_eq(value, CONSTANT("SUBMISSION")))
				*((M_PROTOCOL *)(((char *)server) + setting->offset)) = SUBMISSION;
			else {
				log_critical("The %.*s is an invalid protocol.", st_length_int(value), st_char_get(value));
				result = false;
			}
		} else if (!st_cmp_ci_eq(NULLER(setting->name), CONSTANT(".network.type"))) {
			if (st_empty(value))
				*((M_PORT *)(((char *)server) + setting->offset)) = setting->norm.val.u64;
			else if (!st_cmp_ci_eq(value, CONSTANT("TCP")))
				*((M_PORT *)(((char *)server) + setting->offset)) = TCP_PORT;
			else if (!st_cmp_ci_eq(value, CONSTANT("SSL")))
				*((M_PORT *)(((char *)server) + setting->offset)) = SSL_PORT;
			else {
				log_critical("The value %.*s is an invalid port type. Use TCP or SSL as the port type.", st_length_int(value), st_char_get(value));
				result = false;
			}
		} else {
			log_critical("The %s is an an unrecognized enumerated type.", setting->name);
			result = false;
		}
		break;

		// Integers
	case (M_TYPE_INT8):
		if (!st_empty(value)) {
			if (!int8_conv_st(value, (int8_t *)(((char *)server) + setting->offset))) {
				log_critical("Invalid value for %s.", setting->name);
				result = false;
			}
		} else
			*((int8_t *)(((char *)server) + setting->offset)) = setting->norm.val.i8;
		break;

	case (M_TYPE_INT16):
		if (!st_empty(value)) {
			if (!uint16_conv_st(value, (uint16_t *)(((char *)server) + setting->offset))) {
				log_critical("Invalid value for %s.", setting->name);
				result = false;
			}
		} else
			*((int16_t *)(((char *)server) + setting->offset)) = setting->norm.val.u16;
		break;

	case (M_TYPE_INT32):
		if (!st_empty(value)) {
			if (!int32_conv_st(value, (int32_t *)(((char *)server) + setting->offset))) {
				log_critical("Invalid value for %s.", setting->name);
				result = false;
			}
		} else
			*((int32_t *)(((char *)server) + setting->offset)) = setting->norm.val.i32;
		break;

	case (M_TYPE_INT64):
		if (!st_empty(value)) {
			if (!int64_conv_st(value, (int64_t *)(((char *)server) + setting->offset))) {
				log_critical("Invalid value for %s.", setting->name);
				result = false;
			}
		} else
			*((int64_t *)(((char *)server) + setting->offset)) = setting->norm.val.i64;
		break;

		// Unsigned Integers
	case (M_TYPE_UINT8):
		if (!st_empty(value)) {
			if (!uint8_conv_st(value, (uint8_t *)(((char *)server) + setting->offset))) {
				log_critical("Invalid value for %s.", setting->name);
				result = false;
			}
		} else
			*((uint8_t *)(((char *)server) + setting->offset)) = setting->norm.val.u8;
		break;

	case (M_TYPE_UINT16):
		if (!st_empty(value)) {
			if (!uint16_conv_st(value, (uint16_t *)(((char *)server) + setting->offset))) {
				log_critical("Invalid value for %s.", setting->name);
				result = false;
			}
		} else
			*((uint16_t *)(((char *)server) + setting->offset)) = setting->norm.val.u16;
		break;

	case (M_TYPE_UINT32):
		if (!st_empty(value)) {
			if (!uint32_conv_st(value, (uint32_t *)(((char *)server) + setting->offset))) {
				log_critical("Invalid value for %s.", setting->name);
				result = false;
			}
		} else
			*((uint32_t *)(((char *)server) + setting->offset)) = setting->norm.val.u32;
		break;

	case (M_TYPE_UINT64):
		if (!st_empty(value)) {
			if (!uint64_conv_st(value, (uint64_t *)(((char *)server) + setting->offset))) {
				log_critical("Invalid value for %s.", setting->name);
				result = false;
			}
		} else
			*((uint64_t *)(((char *)server) + setting->offset)) = setting->norm.val.u64;
		break;

	default:
		log_critical("Invalid type. {name = %s / type = %s = %u}", setting->name, type(setting->norm.type), setting->norm.type);
		result = false;
		break;
	}

	return result;
}

/**
 * @brief	Set the value of a magma server configuration entry by name.
 * @note	This function is designed to operate on lines from a configuration source, like a file or database.
 * 			When a named server is referenced for the first time, a new magma server configuration instance will be allocated.
 * @param	name	a managed string containing the human-readable magma server configuration parameter to be set.
 * @param	value 	a managed string containing the new value of the magma server config key.
 * @return	true if the value was was successfully set, or false on failure.
 */
bool_t servers_config(stringer_t *name, stringer_t *value) {

	server_t *server;
	uint32_t serv_num;
	placer_t brack_val;
	bool_t match = false;

	// Make sure were passed a valid value.
	if (st_cmp_ci_starts(name, CONSTANT("magma.servers"))) {
		log_critical("%*.s is not a valid setting.", st_length_int(name), st_char_get(name));
		return false;
	}

	// Extract the inner bracket value, and convert it to a number. If either step fails, return an error.
	if (pl_empty((brack_val = bracket_extract_pl(st_char_get(name) + 13, st_length_get(name) - 13))) || !uint32_conv_bl(pl_char_get(brack_val), pl_length_get(brack_val), &serv_num)) {
		log_critical("%*.s is not a valid setting.", st_length_int(name), st_char_get(name));
		return false;
	}

	// Enforce the limit on server instances.
	if (serv_num >= MAGMA_SERVER_INSTANCES) {
		log_critical("%.*s is invalid, %u is the maximum number of servers possible.", st_length_int(name), st_char_get(name), MAGMA_SERVER_INSTANCES);
		return false;
	}

	// Get a pointer to the server structure. If necessary, allocate a new value.
	if (!(server = servers_alloc(serv_num))) {
		log_critical("Unable to allocate a clean server structure.");
		return false;
	}

	for (uint64_t i = 0; !match && i < sizeof(server_keys) / sizeof(server_keys_t); i++) {

		if (!st_cmp_ci_eq(PLACER(st_char_get(name) + 15 + pl_length_get(brack_val), st_length_get(name) - 15 - pl_length_get(brack_val)), NULLER(server_keys[i].name))) {

			match = true;

			if (!servers_set_value(&server_keys[i], server, value)) {
				log_critical("%.*s has an invalid value.", st_length_int(name), st_char_get(name));
				return false;
			}

		}
	}

	if (!match) {
		log_critical("%.*s is not a valid setting.", st_length_int(name), st_char_get(name));
		return false;
	}

	return true;
}

/**
 * @brief	Log all server key information to be returned via "magma -h".
 * @return	This function returns no value.
 */
void servers_output_help(void) {
	log_info("\nServer options:");

	for (uint64_t j = 0; j < sizeof(server_keys) / sizeof(server_keys_t); j++) {
		log_options(M_LOG_LINE_FEED_DISABLE | M_LOG_TIME_DISABLE | M_LOG_FILE_DISABLE | M_LOG_LINE_DISABLE | M_LOG_FUNCTION_DISABLE |	M_LOG_STACK_TRACE_DISABLE, "\t");
		config_output_value_generic("magma.servers[n]", server_keys[j].name, server_keys[j].norm.type, &(server_keys[j].norm.val.bl), server_keys[j].required);
		log_info("\t\t%s", server_keys[j].description);
	}

}
