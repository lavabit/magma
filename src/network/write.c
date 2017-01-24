
/**
 * @file /magma/network/write.c
 *
 * @brief	Functions used to write data out via the network.
 */

#include "magma.h"

/// HIGH: Create a simpler method of triggering a queue event following a connection write operation, and audit the code
/// to ensure write calls do not accidently orphan a connection by not queuing the connection upon completion.
/// In other words, always ensure that enqueue() is being called on a connection after all processing is performed, so that it is not lost (whether it is to be kept or not).

/**
 * @brief	Write data to a network connection.
 * @note	This function works regardless of whether or not the connection is ssl-enabled.
 * 			If the network write requires multiple system calls, then this code will loop until all the data has been transmitted.
 * @param	con		the connection across which the supplied data will be written.
 * @param	block	a pointer to a data buffer containing the data to be written to the connection's remote client.
 * @param	length	the length, in bytes, of the data buffer to be written.
 * @return	-1 on general network failure, -2 if the connection was reset or closed, or the number of bytes that were written across the connection.
 */
int64_t con_write_bl(connection_t *con, char *block, size_t length) {

	ssize_t written, position = 0;
	int sslerr;

	if (!con || con->network.sockd == -1) {
		con->network.status = -1;
		return -1;
	}
	else if (!block || !length) {
		con->network.status = 0;
		return 0;
	}

	// Loop until bytes have been written to the socket.
	do {

		if (con->network.tls) {
			written = ssl_write(con->network.tls, block + position, length);
			sslerr = SSL_get_error_d(con->network.tls, written);
		}
		else {
			written = send(con->network.sockd, block + position, length, 0);
		}

		// Check for errors on SSL writes.
		if (con->network.tls) {

			// If 0 bytes were written, and it wasn't related to a shutdown, or if < 0 was returned and there was no more data waiting to be written, it's an error.
			if ((!written && sslerr != SSL_ERROR_NONE && sslerr != SSL_ERROR_ZERO_RETURN) || ((written < 0) && sslerr != SSL_ERROR_WANT_WRITE)) {
				con->network.status = -1;
				return -1;
			}
			else if (!written) {
				con->network.status = 2;
				return -2;
			}
		// Check for errors on non-SSL writes in the traditional way.
		}
		else if (written < 0) {

			if (errno == ECONNRESET) {
				con->network.status = 2;
				return -2;
			}
			else if (errno != EAGAIN) {
				con->network.status = -1;
				return -1;
			}

		}

		if (written > 0) {
			length -= written;
			position += written;
		}

	} while (length);

#ifdef MAGMA_PEDANTIC
	if (written < 0) log_pedantic("write = %li {%s}", written, strerror_r(errno, bufptr, buflen));
#endif

	if (written > 0) {
		con->network.status = 1;
	}

	return position;
}

/**
 * @brief	Write a managed string to a network connection.
 * @see		con_write_bl()
 * @param	con		the connection across which the supplied data will be written.
 * @param	string	a managed string containing the data to be written to the connection's remote client.
 * @return	-1 on general network failure, -2 if the connection was reset or closed, or the number of bytes that were written across the connection.
 */
int64_t con_write_st(connection_t *con, stringer_t *string) {

	return con_write_bl(con, st_char_get(string), st_length_get(string));
}

/**
 * @brief	Write a null-terminated string to a network connection.
 * @see		con_write_bl()
 * @param	con		the connection across which the supplied data will be written.
 * @param	string	a pointer to a null-terminated string containing the data to be written to the connection's remote client.
 * @return	-1 on general network failure, -2 if the connection was reset or closed, or the number of bytes that were written across the connection.
 */
int64_t con_write_ns(connection_t *con, char *string) {

	return con_write_bl(con, string, ns_length_get(string));
}

/**
 * @brief	Write a placer to a network connection.
 * @see		con_write_bl()
 * @param	con		the connection across which the supplied data will be written.
 * @param	string	a placer pointing to the data to be written to the connection's remote client.
 * @return	-1 on general network failure, -2 if the connection was reset or closed, or the number of bytes that were written across the connection.
 */
int64_t con_write_pl(connection_t *con, placer_t string) {

	return con_write_bl(con, pl_char_get(string), pl_length_get(string));
}

/**
 * @brief	Write a formatted string to a network connection.
 * @see		con_write_bl()
 * @param	con		the connection across which the supplied data will be written.
 * @param	format	a format string for the output string to be written to the connection's remote client.
 * @param	...		a variable argument list of parameters for the specified format string.
 * @return	-1 on general failure, -2 if the connection was reset or closed, or the number of bytes that were written across the connection.
 */
int64_t con_print(connection_t *con, chr_t *format, ...) {

	va_list args;
	char *buffer;
	size_t length, written;
	int64_t result;

	if (!con || con->network.sockd == -1) {
		con->network.status = -1;
		return -1;
	}
	else if (!format) {
		con->network.status = 0;
		return 0;
	}

	// See if the string will fit inside the standard thread buffer.
	va_start(args, format);
	length = vsnprintf(bufptr, buflen, format, args);
	va_end(args);

	if (length < buflen) {
		// The call to con_write will handle updating the status tracker.
		return con_write_bl(con, bufptr, length);
	}
	// Allocate a large enough buffer.
	else if (!(buffer = mm_alloc(length + 1))) {
		con->network.status = -1;
		return -1;
	}

	// Try building the string again.
	va_start(args, format);

	written = vsnprintf(buffer, length+1, format, args);
	va_end(args);

	if (written != length) {
		mm_free(buffer);
		con->network.status = -1;
		return -1;
	}

	// The call to con_write will handle updating the status tracker.
	result = con_write_bl(con, buffer, length);
	mm_free(buffer);
	return result;
}

/**
 * @brief	Write data to a network client.
 * @note	This function works regardless of whether or not the client connection is ssl-enabled.
 * 			If the network write requires multiple system calls, then this code will loop until all the data has been transmitted.
 * @param	client	the network client connection to which the supplied data will be written.
 * @param	s		a managed string containing the data to be written to the network client connection.
 * @return	-1 on general network failure, -2 if the connection was reset or closed, or the number of bytes that were written across the connection.
 */
int64_t client_write(client_t *client, stringer_t *s) {

	uchr_t *block;
	size_t length;
	ssize_t written, position = 0;
	int sslerr;

	if (!client || client->sockd == -1) {
		client->status = -1;
		return -1;
	}
	else if (st_empty_out(s, &block, &length) || !block || !length) {
		client->status = 0;
		return 0;
	}

	// Loop until bytes have been written to the socket.
	do {

		if (client->tls) {
			written = ssl_write(client->tls, block + position, length);
			sslerr = SSL_get_error_d(client->tls, written);
		}
		else {
			written = send(client->sockd, block + position, length, 0);
		}

		// Check for errors on SSL writes.
		if (client->tls) {

			// If 0 bytes were written, and it wasn't related to a shutdown, or if < 0 was returned and there was no more data waiting to be written, it's an error.
			if ((!written && sslerr != SSL_ERROR_NONE && sslerr != SSL_ERROR_ZERO_RETURN) || ((written < 0) && sslerr != SSL_ERROR_WANT_WRITE)) {
				client->status = -1;
				return -1;
			}
			else if (!written) {
				client->status = 2;
				return -2;
			}
		// Check for errors on non-SSL writes in the traditional way.
		}
		else if (written < 0) {

			if (errno == ECONNRESET) {
				client->status = 2;
				return -2;
			}
			else if (errno != EAGAIN) {
				client->status = -1;
				return -1;
			}

		}

		if (written > 0) {
			length -= written;
			position += written;
		}

	} while (length);

#ifdef MAGMA_PEDANTIC
	if (written < 0) log_pedantic("write = %li {%s}", written, strerror_r(errno, bufptr, buflen));
#endif

	if (written > 0) {
		client->status = 1;
	}

	return position;
}

/**
 * @brief	Write a formatted string to a network client.
 * @see		client_write()
 * @param	client	the network client connection to which the supplied data will be written.
 * @param	format	the format string for the data to be written to the network client connection.
 * @param	...		a va_arg style collection of variables to be expanded by the passed format string.
 * @return	-1 on general network failure, -2 if the connection was reset or closed, or the number of bytes that were written across the connection.
 */
int64_t client_print(client_t *client, chr_t *format, ...) {

	va_list args;
	int64_t result;
	stringer_t *buffer;

	if (!client || client->sockd == -1) {
		return -1;
	}
	else if (!format) {
		return 0;
	}

	va_start(args, format);

	// See if the string will fit inside the standard thread buffer.
	if ((buffer = st_vaprint(format, args))) {
		result = client_write(client, buffer);
		st_free(buffer);
	}

	va_end(args);

	return result;
}

