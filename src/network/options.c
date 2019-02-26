
/**
 * @file /magma/network/options.c
 *
 * @brief	Functions used to set network socket options.
 */

#include "magma.h"

/**
 * Controls whether to send periodic messages along idle socket connections to ensure the connected peer is still present. If the peer fails
 * to respond the connection can be broken.
 *
 * @param sd The socket being configured.
 * @param keepalive Whether to enable the keepalive process.
 * @param idle Controls how long a connection must be idle (in seconds) before the system will start sending keep alive probes.
 * @param interval They delay (in seconds) between each keep alive probe.
 * @param tolerance The maximum number of failed probes that must be sent before dropping a connection.
 * @return Returns true if all the parameters are configured properly, otherwise false to indicate an error.
 *
 * @brief	Set the keepalive flag, the
 */
bool_t net_set_keepalive(int sd, bool_t keepalive, int_t idle, int_t interval, int_t tolerance) {

	int_t val = (keepalive ? 1 : 0);

	if (setsockopt(sd, SOL_SOCKET, SO_KEEPALIVE, &val, sizeof(val)) ||
		setsockopt(sd, IPPROTO_TCP, TCP_KEEPIDLE, &idle, (socklen_t)sizeof(int_t)) ||
		setsockopt(sd, IPPROTO_TCP, TCP_KEEPINTVL, &interval, (socklen_t)sizeof(int_t)) ||
		setsockopt(sd, IPPROTO_TCP, TCP_KEEPCNT, &tolerance, (socklen_t)sizeof(int_t))) {
		log_pedantic("Socket keepalive configuration failed. {%s}", errno_string(errno, bufptr, buflen));
		return false;
	}

	return true;
}

/**
 * @brief	Set the delay flag for a socket (to disable the Nagle algorithm).
 * @param	sd		the socket descriptor to be adjusted.
 * @param	nodelay	a boolean variable specifying whether the Nagle algorithm should be disabled (true) or not (false).
 * @return	true if the flag was successfully set or false on failure.
 */
bool_t net_set_nodelay(int sd, bool_t nodelay) {

	int_t val = (nodelay ? 1 : 0);

	if (setsockopt(sd, IPPROTO_TCP, TCP_NODELAY, &val, sizeof(val)))  {
		log_pedantic("Socket nodelay configuration failed. {%s}", errno_string(errno, bufptr, buflen));
		return false;
	}

	return true;
}

/**
 * @brief	Set the linger flag for a socket.
 * @param	sd		the socket descriptor to be adjusted.
 * @param	linger	a boolean variable specifying whether the socket will linger before closing until all queued messages for the socket have been sent.
 * @param	timeout	a value specifying the number of seconds to linger, if linger is true.
 * @return	true if the flag was successfully set or false on failure.
 */
bool_t net_set_linger(int sd, bool_t linger, int_t timeout) {

  struct linger val = {
  	.l_onoff = (linger ? 1 : 0), // Linger on close.
  	.l_linger = timeout // Time to linger.
  };

  if (setsockopt(sd, SOL_SOCKET, SO_LINGER, &val, (socklen_t)sizeof(struct linger))) {
	  log_pedantic("Socket linger configuration failed. {%s}", errno_string(errno, bufptr, buflen));
	  return false;
  }

  return true;
}

/**
 * @brief	Set the reusable flag for a socket.
 * @param	sd		the socket descriptor to be adjusted.
 * @param	reuse	a boolean variable specifying whether the listening socket should be reusable or not.
 * @return	true if the flag was successfully set or false on failure.
 */
bool_t net_set_reuseable_address(int sd, bool_t reuse) {

	int val = (reuse ? 1 : 0);

	if (setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &val, sizeof(val)))  {
		log_pedantic("Socket address reuse configuration failed. {%s}", errno_string(errno, bufptr, buflen));
		return false;
	}

	return true;
}

/**
 * @brief	Set the blocking flag for a socket.
 * @param	sd			the socket descriptor to be adjusted.
 * @param	blocking	a boolean, where true indicates the socket should be configured to use blocking system calls,
 * 				and false to indicate the socket should use non-blocking calls.
 * @return	true if the flag was successfully set or false on failure.
 */
bool_t net_set_blocking(int sd, bool_t blocking) {

	// If blocking is true, we retrieve the current socket flags, add the non-blocking flag, and then use the exclusive or
	// operation to ensure the flag is removed the from resulting integer.If the blocking flag is false, then we simply use
	// the or operator to ensure the flag is added to the options.
	int flags = blocking ? ((fcntl(sd, F_GETFL, 0) | O_NONBLOCK) ^ O_NONBLOCK) : (fcntl(sd, F_GETFL, 0) | O_NONBLOCK);

	if (fcntl(sd, F_SETFL, flags) == -1)  {
		log_pedantic("Socket blocking configuration failed. {%s}", errno_string(errno, bufptr, buflen));
		return false;
	}

#ifdef MAGMA_PEDANTIC
	if ((fcntl(sd, F_GETFL, 0) & O_NONBLOCK) == O_NONBLOCK && blocking) {
		log_error("Blocking configuration attempt failed.");
	}
#endif

  return true;
}

/**
 * @brief	Set the send and receive timeouts for a socket.
 * @param	sd				the socket descriptor to be configured.
 * @param	timeout_recv	the receive timeout value for the socket, in seconds.
 * @param	timeout_send	the send timeout value for the socket, in seconds.
 * @return	true if both timeout values were set successfully, or false on failure.
 */
bool_t net_set_timeout(int sd, uint32_t timeout_recv, uint32_t timeout_send) {

	struct timeval timeout_sock;

	mm_wipe(&timeout_sock, sizeof(struct timeval));

	timeout_sock.tv_usec = 0;
	timeout_sock.tv_sec = timeout_recv;

	if (setsockopt(sd, SOL_SOCKET, SO_RCVTIMEO, &timeout_sock, sizeof(timeout_sock))) {
		log_pedantic("Socket receive timeout configuration failed. {%s}", errno_string(errno, bufptr, buflen));
		return false;
	}

	timeout_sock.tv_usec = 0;
	timeout_sock.tv_sec = timeout_send;

	if (setsockopt(sd, SOL_SOCKET, SO_SNDTIMEO, &timeout_sock, sizeof(timeout_sock))) {
		log_pedantic("Socket send timeout configuration failed. {%s}", errno_string(errno, bufptr, buflen));
		return false;
	}

	return true;
}

/**
 * @brief	Set the send and receive buffers for a socket at the system level.
 * @param	sd				the socket descriptor to be configured.
 * @param	buffer_recv		the length, in bytes, of the socket receive buffer.
 * @param	buffer_send		the length, in bytes, of the socket send buffer.
 * @return	true if both buffer values were set successfully, or false on failure.
 */
bool_t net_set_buffer_length(int sd, int buffer_recv, int buffer_send) {

	if (setsockopt(sd, SOL_SOCKET, SO_RCVBUF, &buffer_recv, sizeof(buffer_recv))) {
		log_pedantic("Socket receive buffer length configuration failed. {%s}", errno_string(errno, bufptr, buflen));
		return false;
	}

	if (setsockopt(sd, SOL_SOCKET, SO_SNDBUF, &buffer_send, sizeof(buffer_send))) {
		log_pedantic("Socket send buffer length configuration failed. {%s}", errno_string(errno, bufptr, buflen));
		return false;
	}

	return true;
}
