
/**
 * @file /magma/network/addresses.c
 *
 * @brief	A collection of functions used to allocate, configure and destroy connection structures.
 */

#include "magma.h"

/**
 * @brief	Return the IP address information of the remote end of a client connection.
 * @param	con		the input client connection.
 * @param	output	a pointer to an ip_t structure to receive the remote IP address, which is allocated for the caller if output is NULL.
 * @return	NULL on error, or a pointer to the IP address of the remote host.
 */
ip_t * con_addr(connection_t *con, ip_t *output) {

//	ip_t *result;
//	struct sockaddr *address = MEMORYBUF(sizeof(struct sockaddr_in6));
//	socklen_t len = sizeof(struct sockaddr_in6);

//	if (!(result = output) && !(result = mm_alloc(sizeof(ip_t)))) {
//		log_pedantic("The output buffer memory allocation request failed. { requested = %zu }", sizeof(ip_t));
//		return NULL;
//	}
//
//	// Extract the socket structure.
//	else if (getpeername(con->network.sockd, address, &len)) {
//
//		if (!output) {
//			mm_free(result);
//		}
//
//		return NULL;
//	}

	// Classify and copy to the IP information.
//	else if (len == sizeof(struct sockaddr_in6) && ((struct sockaddr_in6 *)address)->sin6_family == AF_INET6) {
//		mm_copy(&(result->ip6), &(((struct sockaddr_in6 *)address)->sin6_addr), sizeof(struct in6_addr));
//		result->family = AF_INET6;
//	}
//	else if (len == sizeof(struct sockaddr_in) && ((struct sockaddr_in *)address)->sin_family == AF_INET) {
//		mm_copy(&(result->ip4), &(((struct sockaddr_in *)address)->sin_addr), sizeof(struct in_addr));
//		result->family = AF_INET;
//	}

	ip_t *result = NULL;

	// We only attempt the copy if a valid IP address is available.
	if (con && con->network.reverse.ip) {

		// If the output pointer is NULL, we need to allocate a buffer.
		if (!output && (result = mm_alloc(sizeof(ip_t)))) {
			ip_copy(result, con->network.reverse.ip);
		}
		// Otherwise, if the output buffer is valid, we use that instead. We could also end up
		// here if the allocation attempt, fails. So we check that output isn't NULL to avoid an error.
		else if (output) {
			result = ip_copy(output, con->network.reverse.ip);
		}


	}

	return result;
}

/**
 * @brief	Return a textual representation of the IP address of a specified connection handle.
 * @param	con		the remote connection to be queried.
 * @param	output	a managed string to store the result, which will be allocated for the caller if output is NULL.
 * @return	NULL on failure, or a pointer to a managed string containing a textual representation of the IP address.
 */
stringer_t * con_addr_presentation(connection_t *con, stringer_t *output) {

	ip_t *ip, buf;
	stringer_t *result = NULL;

	if ((ip = con_addr(con, &buf))) {
		result = ip_presentation(ip, output);
	}

	return result;
}

/**
 * @brief	Get the IP address string for the client connection.
 * @param	con		the client connection to be queried.
 * @param	output	a managed string to receive the output, which will be allocated for the caller if output is NULL.
 * @return	NULL on failure, or a pointer to a managed string containing a textual representation of the IP address.
 */
stringer_t * con_addr_standard(connection_t *con, stringer_t *output) {

	ip_t *ip, buf;
	stringer_t *result = NULL;

	if ((ip = con_addr(con, &buf))) {
		result = ip_standard(ip, output);
	}

	return result;
}

/**
 * @brief	Get the reversed-IP address string for a client connection.
 * @param	con		the client connection to be queried.
 * @param	output	a managed string to receive the output, which will be allocated for the caller if passed as NULL.
 * @return	NULL on failure, or a pointer to a managed string containing a textual representation of the IP address on success.
 */
stringer_t * con_addr_reversed(connection_t *con, stringer_t *output) {

	ip_t *ip, buf;
	stringer_t *result = NULL;

	if ((ip = con_addr(con, &buf))) {
		result = ip_reversed(ip, output);
	}

	return result;
}

/**
 * @brief	Get the subnet string for a specified connection.
 * @param	con		the client connection to be queried.
 * @param	output	a managed string that will store the output of the subnet string lookup.
 * @return	NULL on failure, or a managed string containing a textual representation of a subnet address on success.
 */
stringer_t * con_addr_subnet(connection_t *con, stringer_t *output) {

	ip_t *ip, buf;
	stringer_t *result = NULL;

	if ((ip = con_addr(con, &buf))) {
		result = ip_subnet(ip, output);
	}

	return result;
}

/**
 * @brief	Get a specified 32-bit segment of a connection's peer IP address.
 * @param	con			a pointer to the connection object to be examined.
 * @param	position	a zero-based index into the 32-bit word(s) that comprise the target IP address.
 * @return	-1 if the connecting address can't be looked up, 0 on general error, or the specified 32-bit segment of the passed address on success.
 */
uint32_t con_addr_word(connection_t *con, int_t position) {

	ip_t *ip, buf;
	uint32_t result = -1;

	if ((ip = con_addr(con, &buf))) {
		result = ip_word(ip, position);
	}

	return result;
}

/**
 * @brief	Get a specified 8-bit octet of a connection's peer IP address.
 * @param	con			a pointer to the connection object to be examined.
 * @param	position	a zero-based index into the 8-bit octets that comprise the target IP address.
 * @return	-1 on error, or the specified 8-bit octet of the passed address on success.
 */
octet_t con_addr_octet(connection_t *con, int_t position) {

	ip_t *ip, buf;
	octet_t result = -1;

	if ((ip = con_addr(con, &buf))) {
		result = ip_octet(ip, position);
	}

	return result;
}

/**
 * @brief	Extract a specified 16 bit segment from a connection's peer IP address.
 * @param	con			a pointer to the connection object to be examined.
 * @param	position 	the zero-indexed (starting at least-significant word) 16-bit segment of the IP address to be evaluated.
 * @return	-1 on failure, or the 32 bit-widened segment extracted from the supplied IP address.
 */
segment_t con_addr_segment(connection_t *con, int_t position) {

	ip_t *ip, buf;
	segment_t result = -1;

	if ((ip = con_addr(con, &buf))) {
		result = ip_segment(ip, position);
	}

	return result;
}
