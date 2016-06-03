
/**
 * @file /magma/network/listeners.c
 *
 * @brief	Functions used to listen for incoming connections on specific ports. Successful socket connections are then passed up to the controller for routing.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	The main network handler entry point; poll the listening socket of each configured protocol server, and dispatch the
 * 			protocol-specific handler for any inbound client connection that is accepted.
 * @see		protocol_process()
 * @return	This function returns no value.
 */
void net_listen(void) {

	server_t *server;
	int ed, ready, connection;
	struct epoll_event epoll_context, events[MAGMA_SERVER_INSTANCES];

	mm_wipe(&epoll_context, sizeof(struct epoll_event));

	if ((ed = epoll_create(MAGMA_SERVER_INSTANCES)) == -1) {
		log_critical("The epoll_create() call returned an error. {%s}", strerror_r(errno, bufptr, buflen));
		status_set(-2);
		return;
	}

	// Loop through and add all of the server socket descriptors to our epoll structure.
	for (uint64_t i = 0; i < MAGMA_SERVER_INSTANCES; i++) {

		if ((server = magma.servers[i])) {
			epoll_context.events = EPOLLIN | EPOLLET;
			epoll_context.data.fd = server->network.sockd;

			if ((epoll_ctl(ed, EPOLL_CTL_ADD, server->network.sockd, &epoll_context)) == -1) {
				log_info("The epoll_ctl() call returned an error. {%s}", strerror_r(errno, bufptr, buflen));
				status_set(-2);
				return;
			}

		}

	}

	// Keep looping until its time for the daemon to shutdown.
	while (status()) {

		// Get back a list of sockets ready for data.
		if ((ready = epoll_wait(ed, &events[0], MAGMA_SERVER_INSTANCES, 100)) < 0 && errno != EINTR) {
			log_info("The connection accepter returned an error. { epoll_wait = -1 / error = %s }", strerror_r(errno, bufptr, buflen));
		}
		// Skip socket processing if a timeout occurs.
		else if (ready > 0) {

			for (int i = 0; i < ready; i++) {

				// Determine which server instance is responsible for the requested port.
				if (!(server = servers_get_by_socket(events[i].data.fd))) {
					log_info("Unable to locate the server structure for a socket descriptor. {%i}", events[i].data.fd);
				}
				// Don't bother trying to accept connections on sockets indicating an error event.
				else if (!(events[i].events & (EPOLLERR | EPOLLHUP))) {

					do {

						// Keep calling accept until it fails.
						if ((connection = accept(events[i].data.fd, NULL, NULL)) != -1) {
							protocol_process(server, connection);
						}
						// Keep calling accept until we get an error, but only log errors that are unexpected.
						else if (errno != EAGAIN && errno == EWOULDBLOCK) {
							log_info("Socket connection attempt failed. { accept = -1 / error = %s}", strerror_r(errno, bufptr, buflen));
						}

					} while (connection != -1);

				}
			}
		}
	}

	close(ed);

	return;
}

/**
 * @brief	Initialize a server and listen for connections.
 * @note	Each server listens on either an ipv4 or ipv6 address in non-blocking mode, and will be bound and listen on the configured port.
 * @param	server	a pointer to the server object to be initialized.
 * @return	true on successful initialization of the server, or false on failure.
 */
bool_t net_init(server_t *server) {

	int sd;
	struct sockaddr_in sin4;
	struct sockaddr_in6 sin6;

	// Create the socket.
	if ((sd = socket(server->network.ipv6 ? AF_INET6 : AF_INET, SOCK_STREAM, 0)) == -1) {
		log_critical("Error while calling socket.");
		return false;
	}

	// Set non-blocking IO.
	if (!net_set_non_blocking(sd, false)) {
    log_critical("Error attempting to setup non-blocking IO.");
		return false;
  }

	// Make this a reusable socket.
	if (!net_set_reuseable_address(sd, true)) {
		log_critical("Could not make the socket reusable.");
		return false;
	}

	if (!net_set_buffer_length(sd, magma.system.network_buffer, magma.system.network_buffer)) {
		log_critical("Could not configure the socket buffer size.");
		return false;

	}

	// Zero out the server socket structure, and set the values.
	if (server->network.ipv6) {
		mm_wipe(&sin6, sizeof(sin6));
		sin6.sin6_family = AF_INET6;
		sin6.sin6_addr = in6addr_any;
		sin6.sin6_port = htons(server->network.port);

		// Bind the socket.
		if (bind(sd, (struct sockaddr *)&sin6, sizeof(sin6)) == -1) {
			log_critical("Error while binding to socket. Attempting to use port %u.", server->network.port);
			return false;
		}
	}
	else {
		mm_wipe(&sin4, sizeof(sin4));
		sin4.sin_family = AF_INET;
		sin4.sin_addr.s_addr = INADDR_ANY;
		sin4.sin_port = htons(server->network.port);

		// Bind the socket.
		if (bind(sd, (struct sockaddr *)&sin4, sizeof(sin4)) == -1) {
			log_critical("Error while binding to socket. Attempting to use port %u.", server->network.port);
			return false;
		}
	}

	// Start listening for incoming connections. We set the queue to our config file listen queue value.
	if (listen(sd, server->network.listen_queue) == -1) {
		log_critical("Error while listening to socket. Attempting to use port %u.", server->network.port);
		return false;
	}

	// Store the socket descriptor elsewhere, so it can be shutdown later.
	server->network.sockd = sd;

	return true;
}

/**
 * @brief	Close the listening socket associated with a server.
 * @return	This function returns no value.
 */
void net_shutdown(server_t *server) {

	close(server->network.sockd);
}
