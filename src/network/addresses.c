
/**
 * @file /magma/network/addresses.c
 *
 * @brief	A collection of functions used to allocate, configure and destroy connection structures.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/// LOW: Create a set of sock_addr_* functions and update the con_addr_* functions to use use them. Then add a matching set of
/// sock_peer_* and con_peer_* functions based around the getpeername() function. Add functions for getting a socket's port number
/// as well. Also note that peer _should_ mean the remote address, and addr _should_ mean the local address.
///
///	ip_t ip;
///	char working[64];
///	struct stat64 info;
///	struct sockaddr_in6 saddr;
///	socklen_t len = sizeof(struct sockaddr_in6);
///
///	if (!fstat64(connection, &info) && S_ISSOCK(info.st_mode) && !(ret = getpeername(connection, &saddr, &len))) {
///		if (len == sizeof(struct sockaddr_in6) && saddr.sin6_family == AF_INET6) {
///			mm_copy(&(ip.ip6), &(saddr.sin6_addr), sizeof(struct in6_addr));
///			ip.family = AF_INET6;
///			log_info("%s:%u accepted.", st_char_get(ip_presentation(&ip, PLACER(working, 64))), ntohs(saddr.sin6_port));
///		}
///		else if (len == sizeof(struct sockaddr_in) && ((struct sockaddr_in *)&saddr)->sin_family == AF_INET) {
///			mm_copy(&(ip.ip4), &(((struct sockaddr_in *)&saddr)->sin_addr), sizeof(struct in_addr));
///			ip.family = AF_INET;
///			log_info("%s:%u accepted.", st_char_get(ip_presentation(&ip, PLACER(working, 64))), ntohs(((struct sockaddr_in *)&saddr)->sin_port));
///		}
///	}

/**
 * @brief	Create a copy of an IP address object.
 * @param	dst		a pointer to the destination IP address object to receive the copy.
 * @param	src		a pointer to the source IP address object to be copied.
 * @param	NULL on failure, or a pointer to the destination IP address buffer on success.
 */
// QUESTION: Should return NULL if !src?
ip_t * ip_copy(ip_t *dst, ip_t *src) {

	if (dst && src) {
		mm_copy(dst, src, sizeof(ip_t));
	}

	return dst;
}

/**
 * @brief	Extract a specified 8 bit octet from an IPv4 or IPv6 address.
 * @param	address		the IP address object to be examined.
 * @param	position	the zero-indexed (starting at least-significant byte) byte number of the IP address to be evaluated.
 * @return	-1 on failure, or the 16 bit-widened octet extracted from the supplied IP address.
 */
octet_t ip_octet(ip_t *address, int_t position) {

	octet_t result = -1;

	if (address->family == AF_INET) {

		if (!position) result = (0x000000ff & address->ip4.s_addr);
		else if (position == 1) result = ((0x0000ff00 & address->ip4.s_addr) >> 8);
		else if (position == 2) result = ((0x00ff0000 & address->ip4.s_addr) >> 16);
		else if (position == 3) result = ((0xff000000 & address->ip4.s_addr) >> 24);

	}
	else if (address->family == AF_INET6 && position >= 0 && position <= 15) {
		result = address->ip6.__in6_u.__u6_addr8[position];
	}

	return result;
}

/**
 * @brief	Extract a specified 16 bit segment from an IPv4 or IPv6 address.
 * @param	address		the IP address object to be examined.
 * @param	position	the 0-indexed (starting at least-significant word) 16-bit segment of the IP address to be evaluated.
 * @return	-1 on failure, or the 32 bit-widened segment extracted from the supplied IP address.
 */
segment_t ip_segment(ip_t *address, int_t position) {

	segment_t result = -1;

	if (address->family == AF_INET) {

		if (!position) {
			result = (0x0000ffff & address->ip4.s_addr);
		}
		else if (position == 1) {
			result = ((0xffff0000 & address->ip4.s_addr) >> 16);
		}

	}
	else if (address->family == AF_INET6 && position >= 0 && position <= 7) {
		result = address->ip6.__in6_u.__u6_addr16[position];
	}

	return result;
}

/**
 * @brief	Get a specified 32-bit segment of an IP address.
 * @note	This function operates on both ipv4 and ipv6 addresses.
 * @param	address		a pointer to the IP address object to be examined.
 * @param	position	a zero-based index into the 32-bit word(s) that comprise the target IP address.
 * @return	0 on error, or the specified 32-bit segment of the passed address.
 */
uint32_t ip_word(ip_t *address, int_t position) {

	uint32_t result = 0;

	if (address->family == AF_INET && !position) {
		result = address->ip4.s_addr;
	}
	else if (address->family == AF_INET6 && position >= 0 && position <= 3) {
		result = address->ip6.__in6_u.__u6_addr32[position];
	}

	return result;
}

/**
 * @brief	Display the simple subnet string for an IP address.
 * @note	IPv4 addresses will yield a /24 subnet address, and IPv6 addresses will result in a /48 subnet address.
 * 			Neither address will be displayed with the trailing zero-octet(s).
 * @param	address		a pointer to the ip address to be examined.
 * @param	output		a pointer to a managed string to receive the subnet string, or if passed as NULL, it will be allocated for the caller.
 * @return	NULL on failure, or a pointer to a managed string containing the result on success.
 */
stringer_t * ip_subnet(ip_t *address, stringer_t *output) {

	uint32_t opts = 0;
	stringer_t *result = NULL;
	size_t len = INET6_ADDRSTRLEN;

	if (output && !st_valid_destination((opts = *((uint32_t *)output)))) {
		log_pedantic("An output string was supplied but it does not represent a buffer capable of holding the output.");
		return NULL;
	}

	// Make sure the output buffer is large enough or if output was passed in as NULL we'll attempt the allocation of our own buffer.
	if ((result = output) && ((st_valid_avail(opts) && st_avail_get(output) < len) ||	(!st_valid_avail(opts) && st_length_get(output) < len))) {
		log_pedantic("The output buffer supplied is not large enough to hold the result. {avail = %zu / required = %zu}",	st_valid_avail(opts) ? st_avail_get(output) : st_length_get(output), len);
		return NULL;
	}
	else if (!output && !(result = st_alloc(len))) {
		log_pedantic("The output buffer memory allocation request failed. {requested = %zu}", len);
		return NULL;
	}

	// Store the memory address where the output should be written.
	if (address->family == AF_INET) {
		len = st_sprint(result, "%hhu.%hhu.%hhu", (0x000000ff & address->ip4.s_addr), ((0x0000ff00 & address->ip4.s_addr) >> 8), ((0x00ff0000 & address->ip4.s_addr) >> 16));
	}
	else if (address->family == AF_INET6) {
		len = st_sprint(result, "%02x%02x:%02x%02x:%02x%02x", address->ip6.__in6_u.__u6_addr8[0], address->ip6.__in6_u.__u6_addr8[1], address->ip6.__in6_u.__u6_addr8[2],
			address->ip6.__in6_u.__u6_addr8[3], address->ip6.__in6_u.__u6_addr8[4], address->ip6.__in6_u.__u6_addr8[5]);
	}

	if (!len || len > INET6_ADDRSTRLEN) {
		log_pedantic("An error occurred while trying to translate the address into a string. {length = %zu}", len);

		if (!output) {
			st_free(result);
		}

		return NULL;
	}

	return result;
}

/**
 * @brief	Convert an IP address structure into a readable string.
 * @param	address		a pointer to the IP address to be displayed.
 * @param	output		a managed string to store the output, which will be allocated for the caller if output is NULL.
 * @return	NULL on failure, or a pointer to the managed string containing the IP address as text on success.
 */
stringer_t * ip_presentation(ip_t *address, stringer_t *output) {

	chr_t *ret;
	uint32_t opts = 0;
	stringer_t *result = NULL;
	size_t len = INET6_ADDRSTRLEN;

	if (output && !st_valid_destination((opts = *((uint32_t *)output)))) {
		log_pedantic("An output string was supplied but it does not represent a buffer capable of holding the output.");
		return NULL;
	}

	// Make sure the output buffer is large enough or if output was passed in as NULL we'll attempt the allocation of our own buffer.
	if ((result = output) && st_avail_get(output) < len) {
		log_pedantic("The output buffer supplied is not large enough to hold the result. {avail = %zu / required = %zu}",	st_avail_get(output), len);
		return NULL;
	}
	else if (!output && !(result = st_alloc(len))) {
		log_pedantic("The output buffer memory allocation request failed. {requested = %zu}", len);
		return NULL;
	}

	// Write the address out.
	if (!(ret = (chr_t *)inet_ntop(address->family, (address->family == AF_INET ? (void *)&(address->ip4.s_addr) : (void *)&(address->ip6.__in6_u.__u6_addr32)), st_char_get(result), len))) {
		log_pedantic("An error occurred while trying to translate the address into a string. {inet_ntop = NULL / error = %s}", strerror_r(errno, bufptr, buflen));
		if (!output) st_free(result);
		return NULL;
	}

	// See if were tracking the length.
	if (!output || st_valid_tracked(opts)) {
		st_length_set(result, ns_length_get(st_char_get(result)));
	}
	return result;
}

/**
 * @brief	Convert an IP address structure to string representation.
 * @param	address		a pointer to the IP address to be converted.
 * @param	output		a pointer to the managed string to receive the converted value, which will be allocated for the caller if output is NULL.
 * @return	NULL on failure, or a pointer to a managed string containing a textual representation of the IP address.
 */
stringer_t * ip_standard(ip_t *address, stringer_t *output) {

	uint32_t opts = 0;
	chr_t *p, holder[20];
	stringer_t *result = NULL;
	size_t len = INET6_ADDRSTRLEN;

	if (output && !st_valid_destination((opts = *((uint32_t *)output)))) {
		log_pedantic("An output string was supplied but it does not represent a buffer capable of holding the output.");
		return NULL;
	}

	// Make sure the output buffer is large enough or if output was passed in as NULL we'll attempt the allocation of our own buffer.
	if ((result = output) && st_avail_get(output) < len) {
		log_pedantic("The output buffer supplied is not large enough to hold the result. {avail = %zu / required = %zu}",	st_avail_get(output), len);
		return NULL;
	}
	else if (!output && !(result = st_alloc(len))) {
		log_pedantic("The output buffer memory allocation request failed. {requested = %zu}", len);
		return NULL;
	}

	// Store the memory address where the output should be written.
	// QUESTION: This should probably be architecture independent.
	if (address->family == AF_INET) {
		len = st_sprint(result, "%hhu.%hhu.%hhu.%hhu", (0x000000ff & address->ip4.s_addr), 	((0x0000ff00 & address->ip4.s_addr) >> 8),
			((0x00ff0000 & address->ip4.s_addr) >> 16), ((0xff000000 & address->ip4.s_addr) >> 24));
	}
	else if (address->family == AF_INET6) {

		st_wipe(result);
		p = st_char_get(result);
		for (uint32_t i = 0; i < 4; i++) {

			snprintf(holder, 20, "%08x", htonl(address->ip6.__in6_u.__u6_addr32[i]));

			*p++ = holder[0];
			*p++ = holder[1];
			*p++ = holder[2];
			*p++ = holder[3];
			*p++ = ':';
			*p++ = holder[4];
			*p++ = holder[5];
			*p++ = holder[6];
			*p++ = holder[7];
			if (i != 3) *p++ = ':';
		}

		len = 39;
	}
	// A very unlikely occurrence at this point.
	else {
		log_pedantic("IP address conversion attempted on unsupported family type.");

		if (!output) {
			st_free(result);
		}

		return NULL;
	}

	// See if were tracking the length.
	if (!output || st_valid_tracked(opts)) {
		st_length_set(result, len);
	}

	return result;
}

/**
 * @brief	Get a reversed-IP string, for use in an RBL lookup.
 * @param	address		a pointer to the IP address to be displayed as a string.
 * @param	output		a pointer to the managed string to receive the reversed-IP string, which will be allocated for the caller if output is NULL.
 * @return	NULL on failure, or a pointer to a managed string containing the reversed-IP string on success.
 */
stringer_t * ip_reversed(ip_t *address, stringer_t *output) {

	size_t len = 64;
	uint32_t opts = 0;
	chr_t *p, holder[20];
	stringer_t *result = NULL;

	if (output && !st_valid_destination((opts = *((uint32_t *)output)))) {
		log_pedantic("An output string was supplied but it does not represent a buffer capable of holding the output.");
		return NULL;
	}

	// Make sure the output buffer is large enough or if output was passed in as NULL we'll attempt the allocation of our own buffer.
	if ((result = output) && st_avail_get(output) < len) {
		log_pedantic("The output buffer supplied is not large enough to hold the result. {avail = %zu / required = %zu}",	st_avail_get(output), len);
		return NULL;
	}
	else if (!output && !(result = st_alloc(len))) {
		log_pedantic("The output buffer memory allocation request failed. {requested = %zu}", len);
		return NULL;
	}

	// Store the memory address where the output should be written.
	if (address->family == AF_INET) {
		len = st_sprint(result, "%hhu.%hhu.%hhu.%hhu", ((0xff000000 & address->ip4.s_addr) >> 24), ((0x00ff0000 & address->ip4.s_addr) >> 16),
			((0x0000ff00 & address->ip4.s_addr) >> 8), (0x000000ff & address->ip4.s_addr));
	}
	else if (address->family == AF_INET6) {
		p = st_char_get(result);

		for (uint32_t i = 0; i < 4; i++) {

			snprintf(holder, 20, "%08x", htonl(address->ip6.__in6_u.__u6_addr32[3 - i]));

			*p++ = holder[7];
			*p++ = '.';
			*p++ = holder[6];
			*p++ = '.';
			*p++ = holder[5];
			*p++ = '.';
			*p++ = holder[4];
			*p++ = '.';
			*p++ = holder[3];
			*p++ = '.';
			*p++ = holder[2];
			*p++ = '.';
			*p++ = holder[1];
			*p++ = '.';
			*p++ = holder[0];

			if (i != 3) {
				*p++ = '.';
			}

		}

		len = 63;
	}

	// See if were tracking the length.
	if (!output || st_valid_tracked(opts)) {
		st_length_set(result, len);
	}
	return result;
}

/**
 * @brief	Determine whether two IP addresses are equal.
 * @param	ip1		a pointer to the first ip address object in the comparison.
 * @param	ip2		a pointer to the second ip address object in the comparison.
 * @return	true if the two addresses are equal, or false if they are not.
 */
bool_t ip_address_equal(ip_t *ip1, ip_t *ip2) {

	if (ip1->family != ip2->family) {
		return false;
	}

	if (ip1->family == AF_INET) {
		return (memcmp (&(ip1->ip4), &(ip2->ip4), 4) == 0);
	} else if (ip1->family == AF_INET6) {
		return (memcmp (&(ip1->ip6), &(ip2->ip6), 16) == 0);
	}

	// Hopefully never gets reached.
	return false;
}

/**
 * @brief	Return the IP address information of the remote end of a client connection.
 * @param	con		the input client connection.
 * @param	output	a pointer to an ip_t structure to receive the remote IP address, which is allocated for the caller if output is NULL.
 * @return	NULL on error, or a pointer to the IP address of the remote host.
 */
ip_t * con_addr(connection_t *con, ip_t *output) {

	ip_t *result;
	struct sockaddr_in6 address;
	socklen_t len = sizeof(struct sockaddr_in6);

	if (!(result = output) && !(result = mm_alloc(sizeof(ip_t)))) {
		log_pedantic("The output buffer memory allocation request failed. {requested = %zu}", sizeof(ip_t));
		return NULL;
	}

	// Extract the socket structure.
	else if (getsockname(con->network.sockd, &(address), &len)) {

		if (!output) {
			mm_free(result);
		}

		return NULL;
	}

	// Classify and copy to the result.
	else if (len == sizeof(struct sockaddr_in6) && address.sin6_family == AF_INET6) {
		mm_copy(&(result->ip6), &(address.sin6_addr), sizeof(struct in6_addr));
		result->family = AF_INET6;
	}
	else if (len == sizeof(struct sockaddr_in) && ((struct sockaddr_in *)&address)->sin_family == AF_INET) {
		mm_copy(&(result->ip4), &(((struct sockaddr_in *)&address)->sin_addr), sizeof(struct in_addr));
		result->family = AF_INET;
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

/**
 * @brief	Convert an IP address string to an IP address object.
 * @param	ipstr	a pointer to a null-terminated string containing the IP string to be parsed.
 * @param	out		a pointer to an IP address object that will receive result of the operation.
 * @return	true if the input string was a valid IP address or false if it was unable to be parsed.
 */
bool_t ip_str_addr(chr_t *ipstr, ip_t *out) {

	sa_family_t class = -1;
	struct in6_addr in6;
	struct in_addr in4;
	size_t i;

	// Delimiter of '.' indicates IPv4 address; ':' for IPv6
	for (i = 0; i < ns_length_get(ipstr); i++) {

		if (ipstr[i] == '.') {
			class = AF_INET;
			break;
		} else if (ipstr[i] == ':') {
			class = AF_INET6;
			break;
		}

	}

	if ((class != AF_INET) && (class != AF_INET6)) {
		return false;
	}

	if (class == AF_INET) {

		if (inet_pton (AF_INET, ipstr, &in4) <= 0) {
			return false;
		}

		memcpy(&(out->ip4), &in4, sizeof(in4));
	} else {

		if (inet_pton(AF_INET6, ipstr, &in6) <= 0) {
			return false;
		}

		memcpy (&(out->ip6), &in6, sizeof(in6));
	}

	out->family = class;
	return true;
}

/**
 * @brief	Convert a string to an IP address object.
 * @param	ipstr	a pointer to a null-terminated string containing the IP string to be parsed.
 * @param	out		a pointer to an IP address object that will receive result of the operation.
 * @return	true if the input string was a valid IP address or false if it was unable to be parsed.
 */
bool_t ip_str_subnet(chr_t *substr, subnet_t *out) {

	placer_t netaddr, netmask;
	ip_t addr;
	size_t i;
	chr_t *haddr;
	uint8_t subval;

	// If there's no slash, then all we really have is a hostname. If there's more than 1, it's a bad string.
	i = tok_get_count_st(NULLER(substr), '/');

	if (!i || i > 2) {
		return false;
	} else if (i == 1) {

		if (!ip_str_addr(substr, &addr)) {
			return false;
		}

		out->mask = (addr.family == AF_INET) ? 32 : 128;
		ip_copy(&(out->address), &addr);
		return true;
	}

	// Otherwise, there's both a network component and a subnet component.
	if ((tok_get_ns(substr, ns_length_get(substr), '/', 0, &netaddr) != 0) || (tok_get_ns(substr, ns_length_get(substr), '/', 1, &netmask) != 1)) {
		return false;
	}

	if (!(haddr = ns_import(pl_char_get(netaddr), pl_length_get(netaddr)))) {
		return false;
	}

	if (!ip_str_addr(haddr, &addr)) {
		ns_free(haddr);
		return false;
	}

	ns_free(haddr);

	if (!uint8_conv_bl(pl_char_get(netmask), pl_length_get(netmask), &subval)) {
		return false;
	}

	if (((addr.family == AF_INET) && (subval > 32)) || ((addr.family == AF_INET6) && (subval > 128))) {
		return false;
	}

	out->mask = subval;
	ip_copy(&(out->address), &addr);

	return true;
}

/**
 * @brief	Determines whether a given IP address matches a subnet mask.
 * @param	subnet	a pointer to a subnet that the address will be compared against.
 * @param	addr	a pointer to the IP address of interest.
 * @return	true if the specified IP address falls into the subnet, or false if it does not.
 */
bool_t ip_matches_subnet(subnet_t *subnet, ip_t *addr) {

	uint_t byteno = 0, bitno;
	uchr_t this_mask;

	if (subnet->address.family != addr->family) {
		return false;
	}

	for (size_t i = 0; i < subnet->mask; i++) {

		if (!(i % 8)) {
			byteno++;
			bitno = 7;
		} else {
			bitno--;
		}

		this_mask = 1 << bitno;

		if ((((chr_t *)&(subnet->address.ip))[byteno-1] & this_mask) != (((chr_t *)&(addr->ip))[byteno-1] & this_mask)) {
			return false;
		}

	}

	return true;
}
