
/**
 * @file /magma/engine/controller/protocol.c
 *
 * @brief	Functions used to route incoming network connections to their designated protocol modules. This layer also tracks overall protocol
 * 			statistics and establishes SSL connections for connections that arrive on secure ports.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Initialize all protocol modules, and prime their command arrays for binary searching.
 * @return	This function always returns true.
 */
bool_t protocol_init(void) {
	pop_sort();
	imap_sort();
	smtp_sort();
	molten_sort();
	portal_endpoint_sort();
	return true;
}

/**
 * @brief	Enqueue a protocol-specific handler to service a specified connection, and update any statistics accordingly.
 * @note	If an invalid protocol is specified, the connection will be destroyed gracefully.
 * @param	con		a pointer to the connection object to be serviced.
 * @return	This function returns no value.
 */
void protocol_enqueue(connection_t *con) {

	void *function = NULL;

	// Pedantic sanity checks of the passed in data.
	log_check(con == NULL);
	log_check(con->server == NULL);

	switch (con->server->protocol) {

	case (POP):
		stats_increment_by_name("pop.connections.total");
		if (con_secure(con) == 1) stats_increment_by_name("pop.connections.secure");
		function = &pop_init;
		break;
	case (IMAP):
		stats_increment_by_name("imap.connections.total");
		if (con_secure(con) == 1) stats_increment_by_name("imap.connections.secure");
		function = &imap_init;
		break;
	case (HTTP):
		stats_increment_by_name("http.connections.total");
		if (con_secure(con) == 1) stats_increment_by_name("http.connections.secure");
		function = &http_init;
		break;
	case (SMTP):
		stats_increment_by_name("smtp.connections.total");
		if (con_secure(con) == 1) stats_increment_by_name("smtp.connections.secure");
		function = &smtp_init;
		break;
	case (SUBMISSION):
		stats_increment_by_name("smtp.connections.total");
		if (con_secure(con) == 1) stats_increment_by_name("smtp.connections.secure");
		function = &submission_init;
		break;
	case (MOLTEN):
		stats_increment_by_name("molten.connections.total");
		if (con_secure(con) == 1) stats_increment_by_name("molten.connections.secure");
		function = &molten_init;
		break;
	default:
		log_pedantic("Protocol enqueue was passed a connection using an unsupported or unknown protocol.");
		con_destroy(con);
		return;
	}

	enqueue(function, con);
	return;

}

/**
 * @brief	Establish a secure channel for an inbound ssl connection before passing it off to the general server request handler.
 * @see		protocol_enqueue()
 * @note	This function destroys the client connection completely and returns silently upon any ssl-related failure.
 * @param	con		the The connection object associated with the inbound ssl connection.
 * @return	This function returns no value.
 */
void protocol_secure(connection_t *con) {

	// Pedantic sanity checks of the passed in data.
	log_check(con == NULL);
	log_check(con->server == NULL);

	// Create a new SSL object.
	if (!(con->network.ssl = ssl_alloc(con->server, con->network.sockd, BIO_NOCLOSE))) {

		log_pedantic("The SSL connection attempt failed.");

		// We manually free the connection structure since calling con_destroy() would improperly decrement the statistical counters.
		if (con->network.ssl) ssl_free(con->network.ssl);
		if (con->network.sockd != -1) close(con->network.sockd);
		if (con->network.buffer) st_free(con->network.buffer);
		mutex_destroy(&(con->lock));
		mm_free(con);
		return;
	}

	protocol_enqueue(con);
	return;
}

/**
 * @brief	Create a connection object for an accepted connection, and enqueue it to be handled.
 * @see		protocol_secure(), protocol_enqueue()
 * @note	Depending on whether the server specifies a secure transport layer, protocol_secure() or protocol_enqueue() will be dispatched.
 * @param	server	a pointer to the server object of the server handling the connection.
 * @param	sockd	the socket descriptor of the newly accepted connection.
 */
void protocol_process(server_t *server, int sockd) {

	connection_t *con;

	if (!server || sockd == -1 || !net_set_timeout(sockd, server->network.timeout, server->network.timeout)) {
		log_pedantic("Invalid parameters were passed into the protocol processor.");
		if (sockd != -1)
			close(sockd);
		return;
	}

	if (!(con = con_init(sockd, server))) {
		close(sockd);
		return;
	}

	server->network.type == SSL_PORT && server->ssl.context ? enqueue(&protocol_secure, con) : enqueue(&protocol_enqueue, con);
	return;
}
