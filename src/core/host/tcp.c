
/**
 * @file /magma/src/core/host/tcp.c
 *
 * @brief Generic fuctions for interaction with TCP/IP socket connections.
 *
 */

#include "magma.h"



/**
 * @brief Determine whether the error is a permanent/fatal failure, or a transient error.
 * @return	return 0 for errors we should ignore, and -1 for permanent/fatal failures.
 */
int_t tcp_error(int error) {

	int_t result = 0;

	// These error numbers indicate a network connection issue.
	if (error == EPIPE || error == ENETDOWN || error == ENETUNREACH || error == ENETUNREACH ||
		error == ENETRESET || error == ECONNABORTED || error == ECONNRESET || error == ENOTCONN ||
		error == ESHUTDOWN || error == ETIMEDOUT) {
		result = -1;
	}

	// Otherwise if recv returns a negative number, we log the result, while assuming the connection is still valid. Note
	// that on some platforms EWOULDBLOCK and EAGAIN are technically identical, but we explicitly check for both so
	// logic remains compatible with systems where they differ.
	else if (result < 0 && (error != EWOULDBLOCK || error != EAGAIN || error != EINTR)) {
		log_pedantic("Ambiguous TCP error code. { errno = %i / error = %s }",
			error, strerror_r(error, MEMORYBUF(1024), 1024));
	}

	return result;
}

/**
 * @brief Determine whether the socket connection provided by sockd is still valid.
 * @param sockd		The socket connection being checked.
 * @return	return 0 for valid connections
 */
int_t tcp_status(int sockd) {

	struct stat info;
	int result = 0, holder = 0, error = 0;

	errno = 0;

	if (sockd <  0 || fstat(sockd, &info) || !S_ISSOCK(info.st_mode)) {
		result = -1;
	}

	// In theory the PEEK flag will prevent this call from altering the socket buffer state, while the NOSIGNAL flag
	// should cause it to return EPIPE if the connection is no longer valid.
	else if ((holder = recv(sockd, MANAGEDBUF(64), 64, MSG_PEEK | MSG_DONTWAIT | MSG_NOSIGNAL)) <= 0) {

		// Duplicate the errno so the log pedantic statement below doesn't accidently overwrite it.
		error = errno;

		// Determine whether the error number is fatal.
		result = tcp_error(error);
	}

//	log_pedantic("tcp status = %i / errno = %i", result, error);

	return result;
}

/**
 * @brief	Blocks until a socket is ready for a read/write operation, or otherwise becomes invalid.
 * @param sockd
 * @return
 */
int tcp_wait(int sockd) {

	return 0;
}

/**
 * @brief	Return -1 if the connection is invalid, 0 if the operation should be retried, or a positive number indicating the
 * 			number of bytes processed.
 */
int tcp_continue(int sockd, int result, int syserror) {

	chr_t *message = MEMORYBUF(1024);

	// Check that the daemon hasn't initiated a shutdown.
	//TODO better def
#ifdef MAGMA_H
	if (!status()) return -1;
	else
#endif
	// Data was processed, so there is no need to retry the operation.
	 if (result > 0) return result;

	// Handle non-errors.
	else if (result <= 0 && (syserror == 0 || syserror == EWOULDBLOCK || syserror == EAGAIN || syserror == EINTR)) return 0;

	log_pedantic("A TCP error occurred. { errno = %i / error = %s / message = %s }", syserror, errno_name(syserror),
		strerror_r(syserror, message, 1024));
	return -1;
}

/**
 * @brief	Read data from a TCP/IP network socket.
 * @param	sockd	the socket file descriptor we'll read the data from.
 * @param	buffer	a pointer to the buffer where the data will be stored.
 * @param	length	the maximum length, in bytes, we can read into the buffer.
 * @param	block	a boolean to indicating whether to make a blocking write call.
 * @return	-1 if the network connection is no longer viable, otherwise the number bytes read, while a 0 may indicate
 * 				the connection wasn't ready (when using non-blocking read calls), or a non-network error may have ocurred.
 */
int tcp_read(int sockd, void *buffer, int length, bool_t block) {

	int result = 0, counter = 0;

	if (sockd < 0 || !buffer || !length) {
		log_pedantic("Invalid parameters were provided to the TCP read function.");
		return 0;
	}

#ifdef MAGMA_PEDANTIC
	else if (!block) {
		log_pedantic("Non-blocking TCP read calls have not been fully implemented yet.");
	}
#endif

	do {
		errno = 0;
		result = recv(sockd, buffer, length, (block ? 0 : MSG_DONTWAIT));
	} while (block && counter++ < 8 && !(result = tcp_continue(sockd, result, errno)));

	return result;
}

/**
 * @brief	Write data to an open TCP/IP network socket.
 * @param	sockd	the socket file descriptor we'll write the data to.
 * @param	buffer	a pointer to the buffer containing the data to be written.
 * @param	length	the length, in bytes, of the data to be written.
 * @param	block	a boolean to indicating whether to make a blocking write call.
 * @return	-1 on error, or the number of bytes written to the network connection.
 */
int tcp_write(int sockd, const void *buffer, int length, bool_t block) {

	int result = 0, counter = 0;

	if (sockd < 0 || !buffer || !length) {
		log_pedantic("Passed invalid parameters for a call to the TCP write function.");
		return 0;
	}

#ifdef MAGMA_PEDANTIC
	else if (!block) {
		log_pedantic("Non-blocking TCP write calls have not been fully implemented yet.");
	}
#endif

	do {
		errno = 0;
		result = send(sockd, buffer, length, (block ? 0 : MSG_DONTWAIT));
	} while (block && counter++ < 8 && !(result = tcp_continue(sockd, result, errno)));

	return result;
}

ip_t * tcp_addr_ip(int sockd, ip_t *output) {

	ip_t *result = NULL;
	socklen_t len = sizeof(struct sockaddr_in6);
	struct sockaddr *address = MEMORYBUF(sizeof(struct sockaddr_in6));

	// Extract the socket structure.
	if (getpeername(sockd, address, &len)) {
		return NULL;
	}

	// Allocate memory for the result, if necessary.
	else if (!(result = output) && !(result = mm_alloc(sizeof(ip_t)))) {
		return NULL;
	}

	// Classify and copy to the IP information.
	else if (len == sizeof(struct sockaddr_in6) && ((struct sockaddr_in6 *)address)->sin6_family == AF_INET6) {
		mm_copy(&(result->ip6), &(((struct sockaddr_in6 *)address)->sin6_addr), sizeof(struct in6_addr));
		result->family = AF_INET6;
	}
	else if (len == sizeof(struct sockaddr_in) && ((struct sockaddr_in *)address)->sin_family == AF_INET) {
		mm_copy(&(result->ip4), &(((struct sockaddr_in *)address)->sin_addr), sizeof(struct in_addr));
		result->family = AF_INET;
	}

	return result;
}

stringer_t * tcp_addr_st(int sockd, stringer_t *output) {

	ip_t ip;

	return ip_presentation(tcp_addr_ip(sockd, &ip), output);
}
