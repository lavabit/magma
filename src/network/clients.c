
/**
 * @file /magma/network/clients.c
 *
 * @brief	Functions for handling network client connections.
 */

#include "magma.h"

/**
 * @brief	Get the status of a network client.
 * @param	client	a pointer to the network client object to be queried.
 * @return	 -1 on network errors, 0 for an unknown status, 1 for connected, and 2 for a graceful shutdown.
 */
int_t client_status(client_t *client) {

	int_t result = -1;

	if (client && client->sockd != -1) {
		result = client->status;
	}


	// If the status is positive, and tls_status returns 0, we use the existing status state.
	if (client && client->tls && client->status >= 0 && !tls_status(client->tls)) {
		result = client->status;
	}
	// If the status is positive, and tcp_status returns 0, we use the existing status state.
	else if (client && client->sockd != -1 && client->status >= 0 && !tcp_status(client->sockd)) {
		result = client->status;
	}
	// We return -1 if the status is already negative, or connection is otherwise invalid.
	else {
		result = client->status = -1;
	}

	return result;
}

/**
 * @brief	Establish an TLS connection with a network client instance.
 * @param	client	a pointer to the network client object to have its transport security upgraded.
 * @return	-1 on failure or 0 on success.
 */
int_t client_secure(client_t *client) {

	if (!client) {
		return -1;
	}
	else if (client->tls) {
		return 0;
	}

	else if (!(client->tls = tls_client_alloc(client->sockd))) {
		client->status = -1;
		return -1;
	}

	client->status = 1;

	return 0;
}

/**
 * @brief	Establish a network client connection to a remote host.
 * @param	host	a pointer to a null-terminated string containing the hostname of the remote server.
 * @param	port	the port number of the server to which the connection will be established.
 * @return	NULL on failure or a pointer to a newly initialized network client object for the connection upon success.
 */
client_t * client_connect(chr_t *host, uint32_t port) {

	client_t *result;
	chr_t service[20];
	int_t sd = -1, ret;
	struct addrinfo hints, *info = NULL, *holder = NULL;

	 memset(&hints, 0, sizeof(struct addrinfo));
	 hints.ai_family = AF_UNSPEC;     // IPv4 or IPv6
	 hints.ai_socktype = SOCK_STREAM; // TCP stream socket
	 hints.ai_flags = AI_NUMERICSERV; // Require a numeric service (aka port) number.

	 snprintf(service, 20, "%u", port);

	// Resolve the hostname.
	if ((ret = getaddrinfo(host, service, &hints, &info)) || !info || info->ai_socktype != SOCK_STREAM) {
		log_pedantic("Unable to resolve the host %s:%u and create a client connection. { getaddrinfo = %i / errno = %s }",
			host, port, ret, strerror_r(errno, MEMORYBUF(256), 256));
		if (info) freeaddrinfo(info);
		return NULL;
	}

	// We need to loop through all addresses because we may get an multiple IP addresses, and some of
	// those addresses may be invalid, or inaccessible. We use a disposable "holder" variable as the iterator,
	// so we still call freeaddrinfo on the original info variable, otherwise we will create a memory leak.
	holder = info;

	while (holder) {

		// Create a socket.
		if ((sd = socket(holder->ai_family, holder->ai_socktype, holder->ai_protocol)) == -1) {
			log_pedantic("Unable to create a socket connection with the host %s:%u. { socket = -1 / errno = %s }",
				host, port, strerror_r(errno, MEMORYBUF(1024), 1024));
			freeaddrinfo(info);
			return NULL;
		}

		if (!(ret = connect(sd, holder->ai_addr, holder->ai_addrlen))) {
			break;
		}

		close(sd);
		holder = holder->ai_next;
	}

	// Free the address info.
	freeaddrinfo(info);

	if (ret != 0) {
		log_pedantic("We were unable to connect with the host %s:%u. { connect = %i / errno = %s }",
			host, port, ret, strerror_r(errno, MEMORYBUF(1024), 1024));
		close(sd);
		return NULL;
	}

	if (!(result = mm_alloc(sizeof(client_t))) || !(result->buffer = st_alloc(8192))) {
		log_pedantic("Unable to allocate memory for the client connection context.");

		if (result) {
			mm_free(result);
		}

		close(sd);
		return NULL;
	}

	result->sockd = sd;
	result->status = 1;
	result->line.opts = PLACER_T | JOINTED | STACK | FOREIGNDATA;

	return result;
}

/**
 * @brief	Close a network client connection.
 * @note	If ssl is in use, this will also destroy the overlying ssl session.
 * @return	This function returns no value.
 */
void client_close(client_t *client) {

	if (client) {

		if (client->tls) {
			tls_free(client->tls);
		}

		if (client->sockd != -1) {
			close(client->sockd);
		}

		st_cleanup(client->buffer);
		mm_free(client);
	}

	return;
}
