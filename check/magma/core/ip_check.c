
/**
 * @file /magma/check/magma/core/ip_check.c
 *
 * @brief Various checks designed to make sure the IP address interface works as expected.
 */

#include "magma_check.h"

bool_t check_uint16_to_hex_st(uint16_t val, stringer_t *buff) {

	uint32_t digits[4] = { 0, 0, 0, 0 };
	chr_t *hex_chrs = "0123456789abcdef";

	if ((st_length_get(buff) + st_avail_get(buff)) < 5) return false;

	digits[0] = val / 4096;
	val -= digits[0] * 4096;

	digits[1] = val / 256;
	val -= digits[1] * 256;

	digits[2] = val / 16;
	val -= digits[2] * 16;

	digits[3] = val;

	st_wipe(buff);

	if (st_sprint(buff, "%c%c%c%c", *(hex_chrs + digits[0]), *(hex_chrs + digits[1]),
		*(hex_chrs + digits[2]), *(hex_chrs + digits[3])) == -1) {

		return false;
	}

	return true;
}

void check_address_standard_s (int _i CK_ATTRIBUTE_UNUSED) {

	log_disable();
	stringer_t *buffer;
	stringer_t *errmsg = NULL;
	ip_t ip[4] = {
		{
			AF_INET, {
				.ip4 = {
					0x0100007f
				}
			}
		}, {
			AF_INET, {
				.ip4 = {
					0xffffffff
				}
			}
		}, {
			AF_INET6, {
				.ip6 = in6addr_loopback
			}
		}, {
			AF_INET6, {
				.ip6 = in6addr_any
			}
		}
	};

	tcase_fn_start ("check_address_standard_s", __FILE__, __LINE__);

	if (status() && (buffer = st_alloc(INET6_ADDRSTRLEN))) {

		if (!ip_standard(&ip[0], buffer) || st_cmp_cs_eq(buffer, CONSTANT("127.0.0.1")) ||
			!ip_standard(&ip[1], buffer) || st_cmp_cs_eq(buffer, CONSTANT("255.255.255.255"))) {
			errmsg = NULLER("IPv4 standard format test failed.");
		}

		else if (!ip_standard(&ip[2], buffer) || st_cmp_cs_eq(buffer, CONSTANT("0000:0000:0000:0000:0000:0000:0000:0001")) ||
			!ip_standard(&ip[3], buffer) || st_cmp_cs_eq(buffer, CONSTANT("0000:0000:0000:0000:0000:0000:0000:0000"))) {
			errmsg = NULLER("IPv6 standard format test failed.");
		}

		st_free(buffer);
	}

	log_test("CORE / HOST / ADDRESS / STANDARD / SINGLE THREADED:", errmsg);
	ck_assert_msg(!errmsg, st_char_get(errmsg));
}

void check_address_presentation_s (int _i CK_ATTRIBUTE_UNUSED) {

	log_disable();
	stringer_t *buffer;
	stringer_t *errmsg = NULL;
	ip_t ip[4] = {
		{
			AF_INET, {
				.ip4 = {
					0x0100007f
				}
			}
		}, {
			AF_INET, {
				.ip4 = {
					0xffffffff
				}
			}
		}, {
			AF_INET6, {
				.ip6 = in6addr_loopback
			}
		}, {
			AF_INET6, {
				.ip6 = in6addr_any
			}
		}
	};

	tcase_fn_start ("check_address_presentation_s", __FILE__, __LINE__);

	if (status() && (buffer = st_alloc(INET6_ADDRSTRLEN))) {

		if (!ip_presentation(&ip[0], buffer) || st_cmp_cs_eq(buffer, CONSTANT("127.0.0.1")) ||
			!ip_presentation(&ip[1], buffer) || st_cmp_cs_eq(buffer, CONSTANT("255.255.255.255"))) {
			errmsg = NULLER("IPv4 presentation format test failed.");
		}

		else if (!ip_presentation(&ip[2], buffer) || st_cmp_cs_eq(buffer, CONSTANT("::1")) ||
			!ip_presentation(&ip[3], buffer) || st_cmp_cs_eq(buffer, CONSTANT("::"))) {
			errmsg = NULLER("IPv6 presentation format test failed.");
		}

		st_free(buffer);
	}

	log_test("CORE / HOST / ADDRESS / PRESENTATION / SINGLE THREADED:", errmsg);
	ck_assert_msg(!errmsg, st_char_get(errmsg));
}

void check_address_reversed_s (int _i CK_ATTRIBUTE_UNUSED) {

	log_disable();

	stringer_t *buffer;
	stringer_t *errmsg = NULL;
	ip_t ip[4] = {
		{
			AF_INET, {
				.ip4 = {
					0x0100007f
				}
			}
		}, {
			AF_INET, {
				.ip4 = {
					0xffffffff
				}
			}
		}, {
			AF_INET6, {
				.ip6 = in6addr_loopback
			}
		}, {
			AF_INET6, {
				.ip6 = in6addr_any
			}
		}
	};

	tcase_fn_start ("check_address_reversed_s", __FILE__, __LINE__);

	if (status() && (buffer = st_alloc(INET6_ADDRSTRLEN * 4))) {

		if (!ip_reversed(&ip[0], buffer) || st_cmp_cs_eq(buffer, CONSTANT("1.0.0.127")) ||
			!ip_reversed(&ip[1], buffer) || st_cmp_cs_eq(buffer, CONSTANT("255.255.255.255"))) {
			errmsg = NULLER("IPv4 reversed format test failed.");
		}

		else if (!ip_reversed(&ip[2], buffer) || st_cmp_cs_eq(buffer, CONSTANT("1.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0")) ||
			!ip_reversed(&ip[3], buffer) || st_cmp_cs_eq(buffer, CONSTANT("0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0"))) {
			errmsg = NULLER("IPv6 reversed format test failed.");
		}

		st_free(buffer);
	}

	log_test("CORE / HOST / ADDRESS / REVERSED / SINGLE THREADED:", errmsg);
	ck_assert_msg(!errmsg, st_char_get(errmsg));
}

void check_address_subnet_s (int _i CK_ATTRIBUTE_UNUSED) {

	log_disable();
	stringer_t *buffer;
	stringer_t *errmsg = NULL;
	ip_t ip[4] = {
		{
			AF_INET, {
				.ip4 = {
					0x0100007f
				}
			}
		}, {
			AF_INET, {
				.ip4 = {
					0xffffffff
				}
			}
		}, {
			AF_INET6, {
				.ip6 = in6addr_loopback
			}
		}, {
			AF_INET6, {
				.ip6 = in6addr_any
			}
		}
	};

	tcase_fn_start ("check_address_subnet_s", __FILE__, __LINE__);

	if (status() && (buffer = st_alloc(INET6_ADDRSTRLEN))) {

		if (!ip_subnet(&ip[0], buffer) || st_cmp_cs_eq(buffer, CONSTANT("127.0.0")) ||
			!ip_subnet(&ip[1], buffer) || st_cmp_cs_eq(buffer, CONSTANT("255.255.255"))) {
			errmsg = NULLER("IPv4 subnet format test failed.");
		}

		else if (!ip_subnet(&ip[2], buffer) || st_cmp_cs_eq(buffer, CONSTANT("0000:0000:0000:0000")) ||
			!ip_subnet(&ip[3], buffer) || st_cmp_cs_eq(buffer, CONSTANT("0000:0000:0000:0000"))) {
			errmsg = NULLER("IPv6 subnet test failed.");
		}

		st_free(buffer);
	}

	log_test("CORE / HOST / ADDRESS / SUBNET / SINGLE THREADED:", errmsg);
	ck_assert_msg(!errmsg, st_char_get(errmsg));
}

void check_address_segment_s (int _i CK_ATTRIBUTE_UNUSED) {

	log_disable();

	stringer_t *buffer;
	stringer_t *errmsg = NULL;
	ip_t ip[4] = {
		{
			AF_INET, {
				.ip4 = {
					0x0100007f
				}
			}
		}, {
			AF_INET, {
				.ip4 = {
					0xffffffff
				}
			}
		}, {
			AF_INET6, {
				.ip6 = in6addr_loopback
			}
		}, {
			AF_INET6, {
				.ip6 = in6addr_any
			}
		}
	};

	tcase_fn_start ("check_address_segment_s", __FILE__, __LINE__);

	if (status() && (buffer = st_alloc(INET6_ADDRSTRLEN))) {

		if (ip_segment(&ip[0], 0) != 127 || ip_segment(&ip[0], 1) != 256 || ip_segment(&ip[0], 2) != -1 ||
			ip_segment(&ip[1], 0) != 65535 || ip_segment(&ip[1], 1) != 65535 || ip_segment(&ip[1], 2) != -1) {
			errmsg = NULLER("IPv4 segment test failed.");
		}

		else if (ip_segment(&ip[2], 0) != 0 || ip_segment(&ip[2], 1) != 0 || ip_segment(&ip[2], 2) != 0 ||	ip_segment(&ip[2], 3) != 0 ||
			ip_segment(&ip[2], 4) != 0 || ip_segment(&ip[2], 5) != 0 || ip_segment(&ip[2], 6) != 0 || ip_segment(&ip[2], 7) != 256 ||
			ip_segment(&ip[2], 8) != -1 || ip_segment(&ip[3], 0) != 0 || ip_segment(&ip[3], 1) != 0 || ip_segment(&ip[3], 2) != 0 ||
			ip_segment(&ip[3], 3) != 0 ||	ip_segment(&ip[3], 4) != 0 || ip_segment(&ip[3], 5) != 0 || ip_segment(&ip[3], 6) != 0 ||
			ip_segment(&ip[3], 7) != 0 || ip_segment(&ip[3], 8) != -1) {
			errmsg = NULLER("IPv6 segment test failed.");
		}

		st_free(buffer);
	}

	log_test("CORE / HOST / ADDRESS / SEGMENT / SINGLE THREADED:", errmsg);
	ck_assert_msg(!errmsg, st_char_get(errmsg));
}

void check_address_octet_s (int _i CK_ATTRIBUTE_UNUSED) {

	log_disable();
	stringer_t *buffer;
	stringer_t *errmsg = NULL;
	ip_t ip[4] = {
		{
			AF_INET, {
				.ip4 = {
					0x0100007f
				}
			}
		}, {
			AF_INET, {
				.ip4 = {
					0xffffffff
				}
			}
		}, {
			AF_INET6, {
				.ip6 = in6addr_loopback
			}
		}, {
			AF_INET6, {
				.ip6 = in6addr_any
			}
		}
	};

	tcase_fn_start ("check_address_octet_s", __FILE__, __LINE__);

	if (status() && (buffer = st_alloc(INET6_ADDRSTRLEN))) {

		if (ip_octet(&ip[0], 0) != 127 || ip_octet(&ip[0], 1) != 0 || ip_octet(&ip[0], 2) != 0 || ip_octet(&ip[0], 3) != 1 ||
			ip_octet(&ip[0], 4) != -1 || ip_octet(&ip[1], 0) != 255 || ip_octet(&ip[1], 1) != 255 || ip_octet(&ip[1], 2) != 255 ||
			ip_octet(&ip[1], 3) != 255 || ip_octet(&ip[1], 4) != -1) {
				errmsg = NULLER("IPv4 octet test failed.");
		}

		else if (ip_octet(&ip[2], 0) != 0 || ip_octet(&ip[2], 1) != 0 || ip_octet(&ip[2], 2) != 0 ||	ip_octet(&ip[2], 3) != 0 ||
			ip_octet(&ip[2], 4) != 0 || ip_octet(&ip[2], 5) != 0 || ip_octet(&ip[2], 6) != 0 || ip_octet(&ip[2], 7) != 0 ||
			ip_octet(&ip[2], 8) != 0 || ip_octet(&ip[2], 9) != 0 || ip_octet(&ip[2], 10) != 0 || ip_octet(&ip[2], 11) != 0 ||	ip_octet(&ip[2], 12) != 0 ||
			ip_octet(&ip[2], 13) != 0 || ip_octet(&ip[2], 14) != 0 || ip_octet(&ip[2], 15) != 1 || ip_octet(&ip[2], 16) != -1 ||
			ip_octet(&ip[3], 0) != 0 || ip_octet(&ip[3], 1) != 0 || ip_octet(&ip[3], 2) != 0 ||	ip_octet(&ip[3], 3) != 0 ||
			ip_octet(&ip[3], 4) != 0 || ip_octet(&ip[3], 5) != 0 || ip_octet(&ip[3], 6) != 0 || ip_octet(&ip[3], 7) != 0 ||
			ip_octet(&ip[3], 8) != 0 || ip_octet(&ip[3], 9) != 0 || ip_octet(&ip[3], 10) != 0 || ip_octet(&ip[3], 11) != 0 ||	ip_octet(&ip[3], 12) != 0 ||
			ip_octet(&ip[3], 13) != 0 || ip_octet(&ip[3], 14) != 0 || ip_octet(&ip[3], 15) != 0 || ip_octet(&ip[3], 16) != -1) {
			errmsg = NULLER("IPv6 octet test failed.");
		}

		st_free(buffer);
	}

	log_test("CORE / HOST / ADDRESS / OCTET / SINGLE THREADED:", errmsg);
	ck_assert_msg(!errmsg, st_char_get(errmsg));
}

bool_t check_ip_private_sthread(stringer_t *errmsg) {

	ip_t ip;
	uint16_t segments[4] = { 0, 0, 0, 0 };
	stringer_t *ipstr = MANAGEDBUF(64), *hex_buffs[8] = { MANAGEDBUF(5), MANAGEDBUF(5), MANAGEDBUF(5), MANAGEDBUF(5),
		MANAGEDBUF(5), MANAGEDBUF(5), MANAGEDBUF(5), MANAGEDBUF(5) };

	/// Use the ip_addr_st() function to turn test addresses into IP address structs and confirm ip_private() returns
	/// the correct response.
	///
	/// Generate random addresses in the 10.0.0.0/8 range and verify the outcome.

	st_wipe(ipstr);

	segments[0] = 10;
	for (size_t i = 0; i < IP_CHECK_ROUNDS; i++) {

		for (size_t j = 1; j < 4; j++) segments[j] = rand() % 256;
		st_sprint(ipstr, "%d.%d.%d.%d", segments[0], segments[1], segments[2], segments[3]);

		if (!ip_addr_st(st_char_get(ipstr), &ip)) {
			st_sprint(errmsg, "Failed to construct an ip address object. { ipstr = %.*s }",
				st_length_int(ipstr), st_char_get(ipstr));
			return false;
		}
		else if (!ip_private(&ip)) {
			st_sprint(errmsg, "ip_private() failed to return true. { ipstr = %.*s }",
				st_length_int(ipstr), st_char_get(ipstr));
			return false;
		}

		st_wipe(ipstr);
	}

	/// Generate random addresses in the 127.0.0.0/8 range and verify the outcome (localhost).
	segments[0] = 127;
	for (size_t i = 0; i < IP_CHECK_ROUNDS; i++) {

		for (size_t j = 1; j < 4; j++) segments[j] = rand() % 256;
		st_sprint(ipstr, "%d.%d.%d.%d", segments[0], segments[1], segments[2], segments[3]);

		if (!ip_addr_st(st_char_get(ipstr), &ip)) {
			st_sprint(errmsg, "Failed to construct an ip address object. { ipstr = %.*s }",
				st_length_int(ipstr), st_char_get(ipstr));
			return false;
		}
		else if (!ip_private(&ip)) {
			st_sprint(errmsg, "ip_private() failed to return true. { ipstr = %.*s }",
				st_length_int(ipstr), st_char_get(ipstr));
			return false;
		}

		st_wipe(ipstr);
	}

	/// Generate random addresses in the 172.16.0.0/12 range and verify the outcome.
	segments[0] = 172;
	for (size_t i = 0; i < IP_CHECK_ROUNDS; i++) {

		segments[1] = 16 + (rand() % 16);
		for (size_t j = 2; j < 4; j++) segments[j] = rand() % 256;
		st_sprint(ipstr, "%d.%d.%d.%d", segments[0], segments[1], segments[2], segments[3]);

		if (!ip_addr_st(st_char_get(ipstr), &ip)) {
			st_sprint(errmsg, "Failed to construct an ip address object. { ipstr = %.*s }",
				st_length_int(ipstr), st_char_get(ipstr));
			return false;
		}
		else if (!ip_private(&ip)) {
			st_sprint(errmsg, "ip_private() failed to return true. { ipstr = %.*s }",
				st_length_int(ipstr), st_char_get(ipstr));
			return false;
		}

		st_wipe(ipstr);
	}

	/// Generate random addresses in the 192.168.0.0/16 range and verify the outcome.
	segments[0] = 192;
	segments[1] = 168;
	for (size_t i = 0; i < IP_CHECK_ROUNDS; i++) {

		for (size_t j = 2; j < 4; j++) segments[j] = rand() % 256;
		st_sprint(ipstr, "%d.%d.%d.%d", segments[0], segments[1], segments[2], segments[3]);

		if (!ip_addr_st(st_char_get(ipstr), &ip)) {
			st_sprint(errmsg, "Failed to construct an ip address object. { ipstr = %.*s }",
				st_length_int(ipstr), st_char_get(ipstr));
			return false;
		}
		else if (!ip_private(&ip)) {
			st_sprint(errmsg, "ip_private() failed to return true. { ipstr = %.*s }",
				st_length_int(ipstr), st_char_get(ipstr));
			return false;
		}

		st_wipe(ipstr);
	}

	/// Generate random addresses in the 72.0.0.0/8 and 172.0.0.0/8 ranges then verify the outcome (non-private).
	segments[0] = 72;
	for (size_t i = 0; i < IP_CHECK_ROUNDS; i++) {

		for (size_t j = 1; j < 4; j++) segments[j] = rand() % 256;
		st_sprint(ipstr, "%d.%d.%d.%d", segments[0], segments[1], segments[2], segments[3]);

		if (!ip_addr_st(st_char_get(ipstr), &ip)) {
			st_sprint(errmsg, "Failed to construct an ip address object. { ipstr = %.*s }",
				st_length_int(ipstr), st_char_get(ipstr));
			return false;
		}
		else if (ip_private(&ip)) {
			st_sprint(errmsg, "ip_private() failed to return false. { ipstr = %.*s }",
				st_length_int(ipstr), st_char_get(ipstr));
			return false;
		}

		st_wipe(ipstr);
	}

	segments[0] = 172;
	for (size_t i = 0; i < IP_CHECK_ROUNDS; i++) {

		segments[1] = rand() % 256;
		if (segments[1] > 15 && segments[1] < 32) segments[1] += 16;
		for (size_t j = 2; j < 4; j++) segments[j] = rand() % 256;
		st_sprint(ipstr, "%d.%d.%d.%d", segments[0], segments[1], segments[2], segments[3]);

		if (!ip_addr_st(st_char_get(ipstr), &ip)) {
			st_sprint(errmsg, "Failed to construct an ip address object. { ipstr = %.*s }",
				st_length_int(ipstr), st_char_get(ipstr));
			return false;
		}
		else if (ip_private(&ip)) {
			st_sprint(errmsg, "ip_private() failed to return false. { ipstr = %.*s }",
				st_length_int(ipstr), st_char_get(ipstr));
			return false;
		}

		st_wipe(ipstr);
	}

	/// Generate an address using ::1 and verify the outcome.
	if (!ip_addr_st("::1", &ip)) {
		st_sprint(errmsg, "Failed to construct an ip address object. { ipstr = %.*s }",
			st_length_int(ipstr), st_char_get(ipstr));
		return false;
	}
	else if (!ip_private(&ip)) {
		st_sprint(errmsg, "ip_private() failed to return true. { ipstr = %.*s }",
			st_length_int(ipstr), st_char_get(ipstr));
		return false;
	}

	st_wipe(ipstr);

	/// Generate random addresses in the fc00::/7 range and verify the outcome.
	for (size_t i = 0; i < IP_CHECK_ROUNDS; i++) {

		for (size_t j = 1; j < 8; j++) check_uint16_to_hex_st(rand() % 65536, hex_buffs[j]);
		st_sprint(ipstr, "fc00:%s:%s:%s:%s:%s:%s:%s", st_char_get(hex_buffs[1]), st_char_get(hex_buffs[2]),
			st_char_get(hex_buffs[3]), st_char_get(hex_buffs[4]), st_char_get(hex_buffs[5]), st_char_get(hex_buffs[6]),
			st_char_get(hex_buffs[7]));

		if (!ip_addr_st(st_char_get(ipstr), &ip)) {
			st_sprint(errmsg, "Failed to construct an ip address object. { ipstr = %.*s }",
				st_length_int(ipstr), st_char_get(ipstr));
			return false;
		}
		else if (!ip_private(&ip)) {
			st_sprint(errmsg, "ip_private() failed to return true. { ipstr = %.*s }",
				st_length_int(ipstr), st_char_get(ipstr));
			return false;
		}

		st_wipe(ipstr);
	}

	/// Generate random addresses in the (pick well known non-private prefix)::/7 range and verify the outcome.
	for (size_t i = 0; i < IP_CHECK_ROUNDS; i++) {

		for (size_t j = 1; j < 8; j++) check_uint16_to_hex_st(rand() % 65536, hex_buffs[j]);
		st_sprint(ipstr, "fd00:%s:%s:%s:%s:%s:%s:%s", st_char_get(hex_buffs[1]), st_char_get(hex_buffs[2]),
			st_char_get(hex_buffs[3]), st_char_get(hex_buffs[4]), st_char_get(hex_buffs[5]), st_char_get(hex_buffs[6]),
			st_char_get(hex_buffs[7]));

		if (!ip_addr_st(st_char_get(ipstr), &ip)) {
			st_sprint(errmsg, "Failed to construct an ip address object. { ipstr = %.*s }",
				st_length_int(ipstr), st_char_get(ipstr));
			return false;
		}
		else if (ip_private(&ip)) {
			st_sprint(errmsg, "ip_private() failed to return false. { ipstr = %.*s }",
				st_length_int(ipstr), st_char_get(ipstr));
			return false;
		}

		st_wipe(ipstr);
	}

	/// Generate random addresses in the private IPv4 ranges above and map them into the IPv4 to IPv6 compatability/translation
	/// range... ::ffff:0:0/96 and verify the outcome.

	for (size_t i = 0; i < IP_CHECK_ROUNDS; i++) {

		// 10.0.0.0/8
		segments[0] = 10;
		for (size_t j = 1; j < 4; j++) segments[j] = rand() % 256;
		st_sprint(ipstr, "::ffff:%d.%d.%d.%d", segments[0], segments[1], segments[2], segments[3]);

		if (!ip_addr_st(st_char_get(ipstr), &ip)) {
			st_sprint(errmsg, "Failed to construct an ip address object. { ipstr = %.*s }",
				st_length_int(ipstr), st_char_get(ipstr));
			return false;
		}
		else if (ip_private(&ip)) {
			st_sprint(errmsg, "ip_private() failed to return true. { ipstr = %.*s }",
				st_length_int(ipstr), st_char_get(ipstr));
			return false;
		}

		st_wipe(ipstr);

		// 192.168.0.0/16
		segments[0] = 192;
		segments[1] = 168;
		for (size_t j = 2; j < 4; j++) segments[j] = rand() % 256;
		st_sprint(ipstr, "::ffff:%d.%d.%d.%d", segments[0], segments[1], segments[2], segments[3]);

		if (!ip_addr_st(st_char_get(ipstr), &ip)) {
			st_sprint(errmsg, "Failed to construct an ip address object. { ipstr = %.*s }",
				st_length_int(ipstr), st_char_get(ipstr));
			return false;
		}
		else if (ip_private(&ip)) {
			st_sprint(errmsg, "ip_private() failed to return true. { ipstr = %.*s }",
				st_length_int(ipstr), st_char_get(ipstr));
			return false;
		}

		st_wipe(ipstr);

		// 172.16.0.0/12
		segments[0] = 172;
		segments[1] = 16 + (rand() % 16);
		for (size_t j = 2; j < 4; j++) segments[j] = rand() % 256;
		st_sprint(ipstr, "::ffff:%d.%d.%d.%d", segments[0], segments[1], segments[2], segments[3]);

		if (!ip_addr_st(st_char_get(ipstr), &ip)) {
			st_sprint(errmsg, "Failed to construct an ip address object. { ipstr = %.*s }",
				st_length_int(ipstr), st_char_get(ipstr));
			return false;
		}
		else if (ip_private(&ip)) {
			st_sprint(errmsg, "ip_private() failed to return true. { ipstr = %.*s }",
				st_length_int(ipstr), st_char_get(ipstr));
			return false;
		}
	}

	/// Finally, call ip_private() using an invalid IP address struct. This includes passing NULL. Providing an IP struct
	/// with an address family other than AF_INET or AF_INET6 and any other edge case you can think of.

	if (!ip_addr_st("0.42.42.42", &ip)) {

		st_sprint(errmsg, "Failed to create invalid ip_t object.");
		return false;
	}
	else if (ip_private(NULL) || ip_private(&ip)) {

		st_sprint(errmsg, "ip_private() failed to return false when passed an invalid ip_t object.");
		return false;
	}

	return true;
}

bool_t check_ip_localhost_sthread(stringer_t *errmsg) {

	ip_t ip;
	uint16_t segments[4] = { 0, 0, 0, 0 };
	stringer_t *ipstr = MANAGEDBUF(64), *hex_buffs[8] = { MANAGEDBUF(5), MANAGEDBUF(5), MANAGEDBUF(5), MANAGEDBUF(5),
		MANAGEDBUF(5), MANAGEDBUF(5), MANAGEDBUF(5), MANAGEDBUF(5) };

	/// Use the ip_addr_st() function to turn test addresses into IP address structs and confirm ip_localhost() returns
	/// the correct response.
	/// Generate random addresses in the 127.0.0.1/8 range and verify the outcome.

	st_wipe(ipstr);

	segments[0] = 127;
	for (size_t i = 0; i < IP_CHECK_ROUNDS; i++) {

		segments[1] = rand() % 256;
		segments[2] = rand() % 256;
		segments[3] = rand() % 255 + 1;

		st_sprint(ipstr, "%d.%d.%d.%d", segments[0], segments[1], segments[2], segments[3]);

		if (!ip_addr_st(st_char_get(ipstr), &ip)) {
			st_sprint(errmsg, "Failed to construct an ip address object. { ipstr = %.*s }",
				st_length_int(ipstr), st_char_get(ipstr));
			return false;
		}
		else if (!ip_localhost(&ip)) {
			st_sprint(errmsg, "ip_localhost() failed to return true. { ipstr = %.*s }",
				st_length_int(ipstr), st_char_get(ipstr));
			return false;
		}

		st_wipe(ipstr);
	}

	/// Generate random addresses in the 72.0.0.0/8 and 172.0.0.0/8 ranges then verify the outcome (non-private).

	segments[0] = 72;
	for (size_t i = 0; i < IP_CHECK_ROUNDS; i++) {

		segments[1] = rand() % 256;
		segments[2] = rand() % 256;
		segments[3] = rand() % 255 + 1;

		st_sprint(ipstr, "%d.%d.%d.%d", segments[0], segments[1], segments[2], segments[3]);

		if (!ip_addr_st(st_char_get(ipstr), &ip)) {
			st_sprint(errmsg, "Failed to construct an ip address object. { ipstr = %.*s }",
				st_length_int(ipstr), st_char_get(ipstr));
			return false;
		}
		else if (ip_localhost(&ip)) {
			st_sprint(errmsg, "ip_localhost() failed to return true. { ipstr = %.*s }",
				st_length_int(ipstr), st_char_get(ipstr));
			return false;
		}

		st_wipe(ipstr);
	}
	segments[0] = 172;
	for (size_t i = 0; i < IP_CHECK_ROUNDS; i++) {

		segments[1] = rand() % 256;
		segments[2] = rand() % 256;
		segments[3] = rand() % 255 + 1;

		st_sprint(ipstr, "%d.%d.%d.%d", segments[0], segments[1], segments[2], segments[3]);

		if (!ip_addr_st(st_char_get(ipstr), &ip)) {
			st_sprint(errmsg, "Failed to construct an ip address object. { ipstr = %.*s }",
				st_length_int(ipstr), st_char_get(ipstr));
			return false;
		}
		else if (ip_localhost(&ip)) {
			st_sprint(errmsg, "ip_localhost() failed to return true. { ipstr = %.*s }",
				st_length_int(ipstr), st_char_get(ipstr));
			return false;
		}

		st_wipe(ipstr);
	}

	/// Generate an address using ::1 and verify the outcome.
	if (!ip_addr_st("::1", &ip)) {
		st_sprint(errmsg, "Failed to construct an ip address object. { ipstr = %.*s }",
			st_length_int(ipstr), st_char_get(ipstr));
		return false;
	}
	else if (!ip_localhost(&ip)) {
		st_sprint(errmsg, "ip_localhost() failed to return true. { ipstr = %.*s }",
			st_length_int(ipstr), st_char_get(ipstr));
		return false;
	}

	st_wipe(ipstr);

	/// Generate random addresses in the (pick well known non-private prefix)::/7 range and verify the outcome.
	for (size_t i = 0; i < IP_CHECK_ROUNDS; i++) {

		for (size_t j = 1; j < 8; j++) check_uint16_to_hex_st(rand() % 65536, hex_buffs[j]);
		st_sprint(ipstr, "fd00:%s:%s:%s:%s:%s:%s:%s", st_char_get(hex_buffs[1]), st_char_get(hex_buffs[2]),
			st_char_get(hex_buffs[3]), st_char_get(hex_buffs[4]), st_char_get(hex_buffs[5]), st_char_get(hex_buffs[6]),
			st_char_get(hex_buffs[7]));

		if (!ip_addr_st(st_char_get(ipstr), &ip)) {
			st_sprint(errmsg, "Failed to construct an ip address object. { ipstr = %.*s }",
				st_length_int(ipstr), st_char_get(ipstr));
			return false;
		}
		else if (ip_private(&ip)) {
			st_sprint(errmsg, "ip_localhost() failed to return false. { ipstr = %.*s }",
				st_length_int(ipstr), st_char_get(ipstr));
			return false;
		}

		st_wipe(ipstr);
	}

	/// Generate IPv4 addresses in the range above and map them into the IPv4 to IPv6 compatability/translation
	/// range... ::ffff:0:0/96 and verify the outcome.

	segments[0] = 127;
	for (size_t i = 0; i < IP_CHECK_ROUNDS; i++) {

		segments[1] = rand() % 256;
		segments[2] = rand() % 256;
		segments[3] = rand() % 255 + 1;

		st_sprint(ipstr, "::ffff:%d.%d.%d.%d", segments[0], segments[1], segments[2], segments[3]);

		if (!ip_addr_st(st_char_get(ipstr), &ip)) {
			st_sprint(errmsg, "Failed to construct an ip address object. { ipstr = %.*s }",
				st_length_int(ipstr), st_char_get(ipstr));
			return false;
		}
		else if (!ip_localhost(&ip)) {
			st_sprint(errmsg, "ip_localhost() failed to return true. { ipstr = %.*s }",
				st_length_int(ipstr), st_char_get(ipstr));
			return false;
		}

		st_wipe(ipstr);
	}

	/// Finally, call ip_localhost() using an invalid IP address struct. This includes passing NULL. Providing an IP struct
	/// with an address family other than AF_INET or AF_INET6 and any other edge case you can think of.

	if (!ip_addr_st("0.42.42.42", &ip)) {

		st_sprint(errmsg, "Failed to create invalid ip_t object.");
		return false;
	}
	else if (ip_localhost(NULL) || ip_localhost(&ip)) {

		st_sprint(errmsg, "ip_localhost() failed to return false when passed an invalid ip_t object.");
		return false;
	}

	return true;
}
