
/**
 * @file /magma/network/read.c
 *
 * @brief	Functions used to read in data via the network.
 */

#include "magma.h"

// HIGH: This whole file needs a lot more thought. Especially when returning values <= 0.

/**
 * @brief	Read a line of input from a network connection.
 * @note	This function handles reading data from both regular and ssl connections.
 * 			This function continually attempts to read incoming data from the specified connection until a \n terminated line of input is received.
 * 			If a new line is read, the length of that line is returned to the caller, including the trailing \n.
 * 			If the read returns -1 and wasn't caused by a syscall interruption or blocking error, -1 is returned, and the connection status is set to -1.
 * 			If the read returns 0 and wasn't caused by a syscall interruption or blocking error, -2 is returned, and the connection status is set to 2.
 * 			Once a \n character is reached, the length of the current line of input is returned to the user, and the connection status is set to 1.
 * @param	con		the network connection across which the line of data will be read.
 * @return	-1 on general failure, -2 if the connection was reset, or the length of the current line of input, including the trailing \n.
 */
int64_t con_read_line(connection_t *con, bool_t block) {

	int_t counter = 0;
	ssize_t bytes = 0;
	bool_t line = false;
	stringer_t *ip = NULL, *cipher = NULL, *error = NULL;

	if (!con || con->network.sockd == -1 || con_status(con) < 0) {
		if (con) con->network.status = -1;
		return -1;
	}

	// Check for an existing network buffer. If there isn't one, try creating it.
	else if (!con->network.buffer && !con_init_network_buffer(con)) {
		con->network.status = -1;
		return -1;
	}

	// Check if we have received more data than just what is in the current line of input.
	else if (pl_length_get(con->network.line) && st_length_get(con->network.buffer) > pl_length_get(con->network.line)) {

		// If so, move the unused "new" data after the current line marker to the front of the buffer.
		mm_move(st_data_get(con->network.buffer), st_data_get(con->network.buffer) + pl_length_get(con->network.line),
			st_length_get(con->network.buffer) - pl_length_get(con->network.line));

		// Update the buffer length.
		st_length_set(con->network.buffer, st_length_get(con->network.buffer) - pl_length_get(con->network.line));

		// Check whether the data we just moved contains a complete line.
		if (!pl_empty((con->network.line = line_pl_st(con->network.buffer, 0)))) {
			con->network.status = 1;
			return pl_length_get(con->network.line);
		}

	}
	// Otherwise reset the buffer and line lengths to zero.
	else {
		st_length_set(con->network.buffer, 0);
		con->network.line = pl_null();
	}

	// Loop until we get a complete line, an error, or the buffer is filled.
	do {

		// Read bytes off the network. Skip past any existing data in the buffer.
		if (con->network.tls) {

			// If bytes is zero or below and the library isn't asking for another read, then an error occurred.
			bytes = tls_read(con->network.tls, st_char_get(con->network.buffer) + st_length_get(con->network.buffer),
				st_avail_get(con->network.buffer) - st_length_get(con->network.buffer), block);

			if (bytes <= 0) {
				error = tls_error(con->network.tls, bytes, MANAGEDBUF(512));
				cipher = tls_cipher(con->network.tls, MANAGEDBUF(128));
				ip = con_addr_presentation(con, MANAGEDBUF(INET6_ADDRSTRLEN));

				log_pedantic("TLS read operation failed. { ip = %.*s / %.*s / result = %zi%s%.*s }", st_length_int(ip), st_char_get(ip),
					st_length_int(cipher), st_char_get(cipher), bytes, (error ? " / " : ""), st_length_int(error), st_char_get(error));

				int_t tlserr = SSL_get_error_d(con->network.tls, bytes);
				if (tlserr != SSL_ERROR_NONE && tlserr != SSL_ERROR_WANT_READ && tlserr != SSL_ERROR_WANT_WRITE) {
					con->network.status = -1;
					return -1;
				}
				else {
					bytes = 0;
				}
			}
		}
		else {
			bytes = recv(con->network.sockd, st_char_get(con->network.buffer) + st_length_get(con->network.buffer),
				st_avail_get(con->network.buffer) - st_length_get(con->network.buffer), (block ? 0 : MSG_DONTWAIT));

			// Check for errors on non-SSL reads in the traditional way.
			if (bytes <= 0 && tcp_status(con->network.sockd)) {
				con->network.status = -1;
				return -1;
			}

		}

		if (bytes > 0) {
			st_length_set(con->network.buffer, st_length_get(con->network.buffer) + bytes);
		}

		// Check whether we have a complete line before checking whether the connection was closed.
		if (!st_empty(con->network.buffer) && !pl_empty((con->network.line = line_pl_st(con->network.buffer, 0)))) {
			line = true;
		}

	} while (!line && block && counter++ < 128 && st_length_get(con->network.buffer) != st_avail_get(con->network.buffer) && status());

	if (st_length_get(con->network.buffer) > 0) {
		con->network.status = 1;
	}

	return pl_length_get(con->network.line);
}

/**
 * @brief	Read data from a network connection, and store the data in the connection context buffer.
 * @note	This function handles reading data from both regular and SSL connections.
 * 			If the connection's network buffer hasn't been allocated, it will be initialized.
 * @param	con		a pointer to the connection object from which the data will be read.
 * @return	-1 on internal error, or on a read error.
 */
int64_t con_read(connection_t *con) {

	int_t counter = 0;
	ssize_t bytes = 0;
	bool_t blocking = true;
	stringer_t *ip = NULL, *cipher = NULL, *error = NULL;

	if (!con || con->network.sockd == -1 || con_status(con) < 0) {
		if (con) con->network.status = -1;
		return -1;
	}

	// Check for an existing network buffer. If there isn't one, try creating it.
	else if (!con->network.buffer && !con_init_network_buffer(con)) {
		con->network.status = -1;
		return -1;
	}

	// Check for data past the current line buffer.
	else if (pl_length_get(con->network.line) && st_length_get(con->network.buffer) > pl_length_get(con->network.line)) {

		// Move the unused data to the front of the buffer.
		mm_move(st_data_get(con->network.buffer), st_data_get(con->network.buffer) + pl_length_get(con->network.line),
			st_length_get(con->network.buffer) - pl_length_get(con->network.line));

		// Update the length.
		st_length_set(con->network.buffer, st_length_get(con->network.buffer) - pl_length_get(con->network.line));

		// Clear the line buffer.
		con->network.line = pl_null();

		if (st_length_get(con->network.buffer)) {
			return st_length_get(con->network.buffer);
		}

	}
	// Otherwise reset the buffer and line lengths to zero.
	else {
		st_length_set(con->network.buffer, 0);
		con->network.line = pl_null();
	}

	// Loop until the buffer has data or we get an error.
	do {
		blocking = st_length_get(con->network.buffer) ? false : true;

		// Read bytes off the network. If data is already in the buffer this should be a non-blocking read operation so we can
		// return the already buffered data without delay.
		if (con->network.tls) {
			bytes = tls_read(con->network.tls, st_char_get(con->network.buffer) + st_length_get(con->network.buffer),
				st_avail_get(con->network.buffer) - st_length_get(con->network.buffer), true);

			if (bytes <= 0) {
				error = tls_error(con->network.tls, bytes, MANAGEDBUF(512));
				cipher = tls_cipher(con->network.tls, MANAGEDBUF(128));
				ip = con_addr_presentation(con, MANAGEDBUF(INET6_ADDRSTRLEN));

				log_pedantic("TLS read operation failed. { ip = %.*s / %.*s / result = %zi%s%.*s }", st_length_int(ip), st_char_get(ip),
					st_length_int(cipher), st_char_get(cipher), bytes, (error ? " / " : ""), st_length_int(error), st_char_get(error));

				int_t tlserr = SSL_get_error_d(con->network.tls, bytes);
				if (tlserr != SSL_ERROR_NONE && tlserr != SSL_ERROR_WANT_READ && tlserr != SSL_ERROR_WANT_WRITE) {
					con->network.status = -1;
					return -1;
				}
				else {
					bytes = 0;
				}
			}
		}
		else {
			bytes = recv(con->network.sockd, st_char_get(con->network.buffer) + st_length_get(con->network.buffer),
				st_avail_get(con->network.buffer) - st_length_get(con->network.buffer), blocking ? 0 : MSG_DONTWAIT);

			if (bytes <= 0 && tcp_status(con->network.sockd)) {
				con->network.status = -1;
				return -1;
			}
		}

		if (bytes > 0) {
			st_length_set(con->network.buffer, st_length_get(con->network.buffer) + bytes);
		}

	} while (blocking && counter++ < 128 && !st_length_get(con->network.buffer) && status());

	// If there is data in the buffer process it. Otherwise if the buffer is empty and the connection appears to be closed
	// (as indicated by a return value of 0), then return -1 to let the caller know the connection is dead.
	if (st_length_get(con->network.buffer)) {
		con->network.status = 1;
	}
	else if (!bytes) {
		con->network.status = 2;
		return -2;
	}

	return st_length_get(con->network.buffer);
}

/**
 * @brief	Read a line of input from a network client session.
 *
 *
 * @return
 */
int64_t client_read_line(client_t *client) {

	ssize_t bytes;
	int_t sslerr = -1;
	bool_t line = false;

	if (!client || client->sockd == -1) {
		if (client) client->status = 1;
		return -1;
	}

	// Check for data past the current line buffer.
	else if (pl_length_get(client->line) && st_length_get(client->buffer) > pl_length_get(client->line)) {

		// Move the unused data to the front of the buffer.
		mm_move(st_data_get(client->buffer), st_data_get(client->buffer) + pl_length_get(client->line), st_length_get(client->buffer) - pl_length_get(client->line));

		// Update the length.
		st_length_set(client->buffer, st_length_get(client->buffer) - pl_length_get(client->line));

		// Check whether the data we just moved contains a complete line.
		if (!pl_empty((client->line = line_pl_st(client->buffer, 0)))) {
			client->status = 1;
			return pl_length_get(client->line);
		}
	}
	// Otherwise reset the buffer and line lengths to zero.
	else {
		st_length_set(client->buffer, 0);
		client->line = pl_null();
	}

	// Loop until we get a complete line, an error, or the buffer is filled.
	do {

		// Read bytes off the network. Skip past any existing data in the buffer.
		if (client->tls) {
			bytes = tls_read(client->tls, st_char_get(client->buffer) + st_length_get(client->buffer), st_avail_get(client->buffer) - st_length_get(client->buffer), true);
			sslerr = SSL_get_error_d(client->tls, bytes);
		}
		else {
			bytes = recv(client->sockd, st_char_get(client->buffer) + st_length_get(client->buffer), st_avail_get(client->buffer) - st_length_get(client->buffer), 0);
		}

		// Check for errors on SSL reads.
		if (client->tls) {

			// If 0 bytes were read, and it wasn't related to a shutdown, or if < 0 was returned and there was no more data waiting, it's an error.
			if ((!bytes && sslerr != SSL_ERROR_NONE && sslerr != SSL_ERROR_ZERO_RETURN) ||
				((bytes < 0) && sslerr != SSL_ERROR_WANT_READ)) {
				client->status = -1;
				return -1;
			}
		}

		// Check for errors on non-SSL reads in the traditional way.
		else if (bytes < 0 && errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK) {
			client->status = -1;
			return -1;
		}

		if (bytes > 0) {
			st_length_set(client->buffer, st_length_get(client->buffer) + bytes);
		}

		// Check whether we have a complete line before checking whether the connection was closed.
		if (!st_empty(client->buffer) && !pl_empty((client->line = line_pl_st(client->buffer, 0)))) {
			line = true;
		}
		// Otherwise if the connection has been closed (as indicated by a return value of 0) the line will never terminate. As such
		// the best course of action is to return an error code up the stack to indicate the disconnect.
		else if (!bytes) {
			client->status = 2;
			return -2;
		}

	} while (status() && !line && st_length_get(client->buffer) != st_avail_get(client->buffer));

	if (st_length_get(client->buffer) > 0) {
		client->status = 1;
	}

	return pl_length_get(client->line);
}

int64_t client_read(client_t *client) {

	ssize_t bytes;
	bool_t blocking;
	int_t sslerr = -1;

	if (!client || client->sockd == -1) {
		if (client) client->status = -1;
		return -1;
	}

	// Check for data past the current line buffer.
	else if (pl_length_get(client->line) && st_length_get(client->buffer) > pl_length_get(client->line)) {

		// Move the unused data to the front of the buffer.
		mm_move(st_data_get(client->buffer), st_data_get(client->buffer) + pl_length_get(client->line), st_length_get(client->buffer) - pl_length_get(client->line));

		// Update the length.
		st_length_set(client->buffer, st_length_get(client->buffer) - pl_length_get(client->line));

		// Clear the line buffer.
		client->line = pl_null();
	}
	// Otherwise reset the buffer and line lengths to zero.
	else {
		st_length_set(client->buffer, 0);
		client->line = pl_null();
	}

	// Loop until the buffer has data or we get an error.
	do {
		blocking = st_length_get(client->buffer) ? false : true;

		// Read bytes off the network. If data is already in the buffer this should be a non-blocking read operation so we can
		// return the already buffered data without delay.
		if (client->tls) {
			bytes = tls_read(client->tls, st_char_get(client->buffer) + st_length_get(client->buffer), st_avail_get(client->buffer) - st_length_get(client->buffer), blocking);
			sslerr = SSL_get_error_d(client->tls, bytes);
		}
		else {
			bytes = recv(client->sockd, st_char_get(client->buffer) + st_length_get(client->buffer), st_avail_get(client->buffer) - st_length_get(client->buffer),
				blocking ? 0 : MSG_DONTWAIT);
		}

		// Check for errors on SSL reads.
		if (client->tls) {

			// If 0 bytes were read, and it wasn't related to a shutdown, or if < 0 was returned and there was no more data waiting, it's an error.
			if ((!bytes && sslerr != SSL_ERROR_NONE && sslerr != SSL_ERROR_ZERO_RETURN) ||
				((bytes < 0) && sslerr != SSL_ERROR_WANT_READ)) {
				client->status = -1;
				return -1;
			}
		// Check for errors on non-SSL reads in the traditional way.
		} else if (bytes < 0 && errno != EINTR && errno != EAGAIN && errno != EWOULDBLOCK) {
			client->status = -1;
			return -1;
		}

		if (bytes > 0) {
			st_length_set(client->buffer, st_length_get(client->buffer) + bytes);
		// Or break out of the loop because we've been shutdown.
		} else if (!bytes) {
			break;
		}

	} while (status() && blocking && !st_length_get(client->buffer));

	// If there is data in the buffer process it. Otherwise if the buffer is empty and the connection appears to be closed
	// (as indicated by a return value of 0), then return -1 to let the caller know the connection is dead.
	if (st_length_get(client->buffer)) {
		client->status = 1;
	}
	else if (!bytes) {
		client->status = 2;
		return -2;
	}

	return st_length_get(client->buffer);
}

