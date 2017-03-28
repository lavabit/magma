
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
