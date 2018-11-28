
/**
 * @file /magma/src/core/host/ip.c
 *
 * @brief Generic interface for parsing, and manipulating the IP address structure.
 *
 */

#include "magma.h"

/**
 * @brief	An abstract method of retrieving the address family for an IP structure.
 * @return	returns a value between AF_UNSPEC (typically defined as 0) and AF_MAX, if an invalid family value is encountered
 * 				outside this range, then it will be replaced with AF_UNSPEC. If an invalid address strecture is provided, -1 will
 * 				be returned.
 */
int_t ip_family(ip_t *address) {

	int_t result = -1;

	// Valid structures, with what appear to be invalid address families result in AF_UNSPEC.
	if (address && (address->family <= AF_UNSPEC || address->family >= AF_MAX)) {
		result = AF_UNSPEC;
	}
	else if (address) {
		result = address->family;
	}

	return result;
}

/**
 * @brief	Determine whether a structure holds an IPv4 or IPv6 address.
 * @return	returns -1 for invalid inputs, or system errors, 0 if the function isn't provided with a TCP/IP address,
 * 				the number 4 if the address is IPv4 and 6 if the address is IPv6.
 */
int8_t ip_type(ip_t *address) {

	int8_t result = 0;

	if (address) {
		switch (ip_family(address)) {

			// Handle TCP/IP v4 and v6 addresses.
			case (AF_INET6):
				result = 6;
				break;
			case (AF_INET):
				result = 4;
				break;

			// If the address family is valid, but not IPv4 or IPv6 then we return 0. If the family is invalid, we let
			// default value of -1 get returned.
			default:
				if (ip_family(address) != -1) result = 0;
				break;
		}
	}

	return result;
}

/**
 * @brief	Determine whether an IP address matches the localhost loopback address.
 * @note	This function only matches if the remote peer is connected using IPv4 from the 127.0.0.0/8 range, from the same range
 * 				using an IPv4 to IPv6 mapping, or frm the specific IPv6  using IPv6
 * 				from the ::1/128 address by IPv6.
 * @remark	The rationale behind this function is to make it easy for us to detect connections from the local host, and thus
 * 				dictating that the packets associated with the connection never travel across a network cable, which allows
 * 				us to excempt the peer from transport security requirements, SPF checks. and allow the use of administrative
 * 				functions without authentication. In theory connections from IP addresses associated with the network
 * 					interfaces on the local host are just as safe, but we currently only match against the loopback address.
 * @return	true if the connection appears to be using the loopback adapter, or false, if the connection appears to be from anywhere else.
 */
bool_t ip_localhost(ip_t *address) {

	bool_t result = false;

	// Match the singular IPv6 loopback address, ::1
	if (address && ip_type(address) == 6 && IN6_IS_ADDR_LOOPBACK(&(address->ip6))) {
		result = true;
	}
	// Match an IPv4 address in the loopback address range after it has been mapped into the IPv6 compatibility space.
	else if (address && ip_type(address) == 6 && IN6_IS_ADDR_V4MAPPED(&(address->ip6)) &&
		(0x000000ff & ip_word(address, 3)) == 127) {
		result = true;
	}
	// Match any IPv4 address in the 127.0.0.0/8 address space.
	else if (address && ip_type(address) == 4 && ip_octet(address, 0) == 127) {
		result = true;
	}

	return result;
}

/**
 * @brief	Determine whether an IP address appears to be associated with a non-routeable, private address space.
 * @note	This function looks for the IP addresses in the address ranges set aside for private networks. For IPv4
 * 				the ranges 10.0.0.0/8, 172.16.0.0/12 and 192.168.0.0/16 are reserved. For IPv6 the range fc00::/7 has
 * 				been set aside. By convention addresses in these ranges are unrouteable on the open internet, so if a peer
 * 				appears to be connecting from a private address then it should be on the same private network.
 * @remark	The rationale behind this function is to make it easy for us to detect connections from hosts on the same
 * 				 (internal) network, and thus treat those connections differently than connections from the public internet.
 * @return	false by default, and true if the address falls inside the ranges reserved for private networks.
 */
bool_t ip_private(ip_t *address) {

	bool_t result = false;

	// The IPv6 private address space is the singular address range fc00::/7.
	if (address && ip_type(address) == 6 && ip_octet(address, 0) == 252) {
		result = true;
	}

	// Match IPv4 private addresses mapped into the IPv6 compatability/translation range.
	else if (address && ip_type(address) == 6 && IN6_IS_ADDR_V4MAPPED(&(address->ip6)) &&
		// Match anything inside the 0:0:0:0:0:ffff:a00:0/104 address range.
		((0x000000ff & ip_word(address, 0)) == 10 ||
		// Match anything inside the 0:0:0:0:0:ffff:ac10:0/108 address range.
		((0x000000ff & ip_word(address, 0)) == 172 && ((0x0000ff00 & ip_word(address, 0)) >> 8) >= 16 &&
			((0x0000ff00 & ip_word(address, 0)) >> 8) <= 31) ||
		// Match anything inside the 0:0:0:0:0:ffff:c0a8:0/112 address range.
		((0x000000ff & ip_word(address, 0)) == 192 && ((0x0000ff00 & ip_word(address, 0)) >> 8) == 168))) {
		result = true;
	}

	// The IPv4 private address space is divided into three different groupings.
	else if (address && ip_type(address) == 4 &&
		// Match anything inside the 10.0.0.0/8 address range.
		(ip_octet(address, 0) == 10 ||
		// Match anything inside the 172.16.0.0/12 address range.
		(ip_octet(address, 0) == 172 && ip_octet(address, 1) >= 16 && ip_octet(address, 1) <= 31) ||
		// Match anything inside the 192.168.0.0/16 address range.
		(ip_octet(address, 0) == 192 && ip_octet(address, 1) == 168))) {
		result = true;
	}

	// If the result is still false, because it didn't match any of the private address ranges, then we need to run it
	// through the ip_localhost() function so loopback addresses also return true.
	else if (address && !result) {
		result = ip_localhost(address);
	}

	return result;
}

/**
 * @brief	Create a copy of an IP address object.
 * @param	dst		a pointer to the destination IP address object to receive the copy.
 * @param	src		a pointer to the source IP address object to be copied.
 * @param	NULL on failure, or a pointer to the destination IP address buffer on success.
 */
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
 * @note	IPv4 addresses will yield a /24 subnet address, and IPv6 addresses will result in a /64 subnet address.
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

	// For IPv4 addresses use the first 24 bits, out of the total 32 bits available.
	if (address->family == AF_INET) {
		len = st_sprint(result, "%u.%u.%u", (0x000000ff & address->ip4.s_addr), ((0x0000ff00 & address->ip4.s_addr) >> 8),
			((0x00ff0000 & address->ip4.s_addr) >> 16));
	}
	// For IPv6 addresses use the first 64 bits, out of the total 128 bits available. The first 64 bits should contain the
	// routing prefix, plus the subnet id.
	else if (address->family == AF_INET6) {
		len = st_sprint(result, "%02x%02x:%02x%02x:%02x%02x:%02x%02x", address->ip6.__in6_u.__u6_addr8[0], address->ip6.__in6_u.__u6_addr8[1],
			address->ip6.__in6_u.__u6_addr8[2], address->ip6.__in6_u.__u6_addr8[3], address->ip6.__in6_u.__u6_addr8[4],
			address->ip6.__in6_u.__u6_addr8[5], address->ip6.__in6_u.__u6_addr8[6], address->ip6.__in6_u.__u6_addr8[7]);
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
		log_pedantic("An error occurred while trying to translate the address into a string. {inet_ntop = NULL / error = %s}", strerror_r(errno, MEMORYBUF(1024), 1024));
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
	if (address->family == AF_INET) {
		len = st_sprint(result, "%u.%u.%u.%u", (0x000000ff & address->ip4.s_addr), 	((0x0000ff00 & address->ip4.s_addr) >> 8),
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
		len = st_sprint(result, "%u.%u.%u.%u", ((0xff000000 & address->ip4.s_addr) >> 24), ((0x00ff0000 & address->ip4.s_addr) >> 16),
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
bool_t ip_addr_eq(ip_t *ip1, ip_t *ip2) {

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
 * @brief	Convert a string into an IP address object.
 * @param	ipstr	a pointer to a null-terminated string containing the IP string to be parsed.
 * @param	out		a pointer to an IP address object that will receive result of the operation.
 * @return	true if the input string was a valid IP address or false if it was unable to be parsed.
 */
bool_t ip_addr_st(chr_t *ipstr, ip_t *out) {

	size_t i;
	struct in_addr in4;
	struct in6_addr in6;
	sa_family_t class = 0;

	// Delimiter of '.' indicates IPv4 address; ':' for IPv6
	for (i = 0; i < ns_length_get(ipstr) && class == 0; i++) {

		if (ipstr[i] == '.') {
			class = AF_INET;
		}
		else if (ipstr[i] == ':') {
			class = AF_INET6;
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
	}
	else {

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
bool_t ip_subnet_st(chr_t *substr, subnet_t *out) {

	size_t i;
	ip_t addr;
	chr_t *haddr;
	uint8_t subval;
	placer_t netaddr, netmask;

	// If there's no slash, then all we really have is a hostname. If there's more than 1, it's a bad string.
	i = tok_get_count_st(NULLER(substr), '/');

	if (!i || i > 2) {
		return false;
	} else if (i == 1) {

		if (!ip_addr_st(substr, &addr)) {
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

	if (!ip_addr_st(haddr, &addr)) {
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

	uchr_t this_mask;
	uint_t byteno = 0, bitno = 0;

	if (subnet->address.family != addr->family) {
		return false;
	}

	for (size_t i = 0; i < subnet->mask; i++) {

		if (!(i % 8)) {
			byteno++;
			bitno = 7;
		}
		else {
			bitno--;
		}

		this_mask = 1 << bitno;

		if ((((chr_t *)&(subnet->address.ip))[byteno-1] & this_mask) != (((chr_t *)&(addr->ip))[byteno-1] & this_mask)) {
			return false;
		}

	}

	return true;
}
