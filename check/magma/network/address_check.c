
/**
 * @file /check/magma/network/address_check.c
 *
 * @brief Address unit tests.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma_check.h"

void check_address_standard_s (int _i CK_ATTRIBUTE_UNUSED) {

	tcase_fn_start ("check_address_standard_s", __FILE__, __LINE__);

	stringer_t *buffer;
	chr_t *errmsg = NULL;
	ip_t ip[4] = { {AF_INET,{.ip4={0x0100007f}}}, {AF_INET,{.ip4={0xffffffff}}}, {AF_INET6,{.ip6=in6addr_loopback}}, {AF_INET6,{.ip6=in6addr_any}} };

	log_unit("%-64.64s", "NETWORK / ADDRESSES / STANDARD / SINGLE THREADED:");

	if (status() && (buffer = st_alloc(INET6_ADDRSTRLEN))) {

		if (!ip_standard(&ip[0], buffer) || st_cmp_cs_eq(buffer, CONSTANT("127.0.0.1")) ||
			!ip_standard(&ip[1], buffer) || st_cmp_cs_eq(buffer, CONSTANT("255.255.255.255"))) {
			errmsg = "IPv4 standard format test failed.";
		}

		else if (!ip_standard(&ip[2], buffer) || st_cmp_cs_eq(buffer, CONSTANT("0000:0000:0000:0000:0000:0000:0000:0001")) ||
			!ip_standard(&ip[3], buffer) || st_cmp_cs_eq(buffer, CONSTANT("0000:0000:0000:0000:0000:0000:0000:0000"))) {
			errmsg = "IPv6 standard format test failed.";
		}

		st_free(buffer);
	}

	log_unit("%10.10s\n", (!errmsg ? (status() ? "PASSED" : "SKIPPED") : "FAILED"));
	fail_unless(!errmsg, errmsg);
}

void check_address_presentation_s (int _i CK_ATTRIBUTE_UNUSED) {

	tcase_fn_start ("check_address_presentation_s", __FILE__, __LINE__);

	stringer_t *buffer;
	chr_t *errmsg = NULL;
	ip_t ip[4] = { {AF_INET,{.ip4={0x0100007f}}}, {AF_INET,{.ip4={0xffffffff}}}, {AF_INET6,{.ip6=in6addr_loopback}}, {AF_INET6,{.ip6=in6addr_any}} };

	log_unit("%-64.64s", "NETWORK / ADDRESSES / PRESENTATION / SINGLE THREADED:");

	if (status() && (buffer = st_alloc(INET6_ADDRSTRLEN))) {

		if (!ip_presentation(&ip[0], buffer) || st_cmp_cs_eq(buffer, CONSTANT("127.0.0.1")) ||
			!ip_presentation(&ip[1], buffer) || st_cmp_cs_eq(buffer, CONSTANT("255.255.255.255"))) {
			errmsg = "IPv4 presentation format test failed.";
		}

		else if (!ip_presentation(&ip[2], buffer) || st_cmp_cs_eq(buffer, CONSTANT("::1")) ||
			!ip_presentation(&ip[3], buffer) || st_cmp_cs_eq(buffer, CONSTANT("::"))) {
			errmsg = "IPv6 presentation format test failed.";
		}

		st_free(buffer);
	}

	log_unit("%10.10s\n", (!errmsg ? (status() ? "PASSED" : "SKIPPED") : "FAILED"));
	fail_unless(!errmsg, errmsg);

}

void check_address_reversed_s (int _i CK_ATTRIBUTE_UNUSED) {

	tcase_fn_start ("check_address_reversed_s", __FILE__, __LINE__);

	stringer_t *buffer;
	chr_t *errmsg = NULL;
	ip_t ip[4] = { {AF_INET,{.ip4={0x0100007f}}}, {AF_INET,{.ip4={0xffffffff}}}, {AF_INET6,{.ip6=in6addr_loopback}}, {AF_INET6,{.ip6=in6addr_any}} };


	log_unit("%-64.64s", "NETWORK / ADDRESSES / REVERSED / SINGLE THREADED:");

	if (status() && (buffer = st_alloc(INET6_ADDRSTRLEN * 4))) {

		if (!ip_reversed(&ip[0], buffer) || st_cmp_cs_eq(buffer, CONSTANT("1.0.0.127")) ||
			!ip_reversed(&ip[1], buffer) || st_cmp_cs_eq(buffer, CONSTANT("255.255.255.255"))) {
			errmsg = "IPv4 reversed format test failed.";
		}

		else if (!ip_reversed(&ip[2], buffer) || st_cmp_cs_eq(buffer, CONSTANT("1.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0")) ||
			!ip_reversed(&ip[3], buffer) || st_cmp_cs_eq(buffer, CONSTANT("0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0.0"))) {
			errmsg = "IPv6 reversed format test failed.";
		}

		st_free(buffer);
	}

	log_unit("%10.10s\n", (!errmsg ? (status() ? "PASSED" : "SKIPPED") : "FAILED"));
	fail_unless(!errmsg, errmsg);

}

void check_address_subnet_s (int _i CK_ATTRIBUTE_UNUSED) {

	tcase_fn_start ("check_address_subnet_s", __FILE__, __LINE__);

	stringer_t *buffer;
	chr_t *errmsg = NULL;
	ip_t ip[4] = { {AF_INET,{.ip4={0x0100007f}}}, {AF_INET,{.ip4={0xffffffff}}}, {AF_INET6,{.ip6=in6addr_loopback}}, {AF_INET6,{.ip6=in6addr_any}} };

	log_unit("%-64.64s", "NETWORK / ADDRESSES / SUBNET / SINGLE THREADED:");

	if (status() && (buffer = st_alloc(INET6_ADDRSTRLEN))) {

		if (!ip_subnet(&ip[0], buffer) || st_cmp_cs_eq(buffer, CONSTANT("127.0.0")) ||
			!ip_subnet(&ip[1], buffer) || st_cmp_cs_eq(buffer, CONSTANT("255.255.255"))) {
			errmsg = "IPv4 subnet format test failed.";
		}

		else if (!ip_subnet(&ip[2], buffer) || st_cmp_cs_eq(buffer, CONSTANT("0000:0000:0000")) ||
			!ip_subnet(&ip[3], buffer) || st_cmp_cs_eq(buffer, CONSTANT("0000:0000:0000"))) {
			errmsg = "IPv6 subnet test failed.";
		}

		st_free(buffer);
	}

	log_unit("%10.10s\n", (!errmsg ? (status() ? "PASSED" : "SKIPPED") : "FAILED"));
	fail_unless(!errmsg, errmsg);

}

void check_address_segment_s (int _i CK_ATTRIBUTE_UNUSED) {

	tcase_fn_start ("check_address_segment_s", __FILE__, __LINE__);

	stringer_t *buffer;
	chr_t *errmsg = NULL;
	ip_t ip[4] = { {AF_INET,{.ip4={0x0100007f}}}, {AF_INET,{.ip4={0xffffffff}}}, {AF_INET6,{.ip6=in6addr_loopback}}, {AF_INET6,{.ip6=in6addr_any}} };

	log_unit("%-64.64s", "NETWORK / ADDRESSES / SEGMENT / SINGLE THREADED:");

	if (status() && (buffer = st_alloc(INET6_ADDRSTRLEN))) {

		if (ip_segment(&ip[0], 0) != 127 || ip_segment(&ip[0], 1) != 256 || ip_segment(&ip[0], 2) != -1 ||
			ip_segment(&ip[1], 0) != 65535 || ip_segment(&ip[1], 1) != 65535 || ip_segment(&ip[1], 2) != -1) {
			errmsg = "IPv4 segment test failed.";
		}

		else if (ip_segment(&ip[2], 0) != 0 || ip_segment(&ip[2], 1) != 0 || ip_segment(&ip[2], 2) != 0 ||	ip_segment(&ip[2], 3) != 0 ||
			ip_segment(&ip[2], 4) != 0 || ip_segment(&ip[2], 5) != 0 || ip_segment(&ip[2], 6) != 0 || ip_segment(&ip[2], 7) != 256 ||
			ip_segment(&ip[2], 8) != -1 || ip_segment(&ip[3], 0) != 0 || ip_segment(&ip[3], 1) != 0 || ip_segment(&ip[3], 2) != 0 ||
			ip_segment(&ip[3], 3) != 0 ||	ip_segment(&ip[3], 4) != 0 || ip_segment(&ip[3], 5) != 0 || ip_segment(&ip[3], 6) != 0 ||
			ip_segment(&ip[3], 7) != 0 || ip_segment(&ip[3], 8) != -1) {
			errmsg = "IPv6 segment test failed.";
		}

		st_free(buffer);
	}

	log_unit("%10.10s\n", (!errmsg ? (status() ? "PASSED" : "SKIPPED") : "FAILED"));
	fail_unless(!errmsg, errmsg);

}

void check_address_octet_s (int _i CK_ATTRIBUTE_UNUSED) {

	tcase_fn_start ("check_address_octet_s", __FILE__, __LINE__);

	stringer_t *buffer;
	chr_t *errmsg = NULL;
	ip_t ip[4] = { {AF_INET,{.ip4={0x0100007f}}}, {AF_INET,{.ip4={0xffffffff}}}, {AF_INET6,{.ip6=in6addr_loopback}}, {AF_INET6,{.ip6=in6addr_any}} };

	log_unit("%-64.64s", "NETWORK / ADDRESSES / OCTET / SINGLE THREADED:");

	if (status() && (buffer = st_alloc(INET6_ADDRSTRLEN))) {

		if (ip_octet(&ip[0], 0) != 127 || ip_octet(&ip[0], 1) != 0 || ip_octet(&ip[0], 2) != 0 || ip_octet(&ip[0], 3) != 1 ||
			ip_octet(&ip[0], 4) != -1 || ip_octet(&ip[1], 0) != 255 || ip_octet(&ip[1], 1) != 255 || ip_octet(&ip[1], 2) != 255 ||
			ip_octet(&ip[1], 3) != 255 || ip_octet(&ip[1], 4) != -1) {
				errmsg = "IPv4 octet test failed.";
		}

		else if (ip_octet(&ip[2], 0) != 0 || ip_octet(&ip[2], 1) != 0 || ip_octet(&ip[2], 2) != 0 ||	ip_octet(&ip[2], 3) != 0 ||
			ip_octet(&ip[2], 4) != 0 || ip_octet(&ip[2], 5) != 0 || ip_octet(&ip[2], 6) != 0 || ip_octet(&ip[2], 7) != 0 ||
			ip_octet(&ip[2], 8) != 0 || ip_octet(&ip[2], 9) != 0 || ip_octet(&ip[2], 10) != 0 || ip_octet(&ip[2], 11) != 0 ||	ip_octet(&ip[2], 12) != 0 ||
			ip_octet(&ip[2], 13) != 0 || ip_octet(&ip[2], 14) != 0 || ip_octet(&ip[2], 15) != 1 || ip_octet(&ip[2], 16) != -1 ||
			ip_octet(&ip[3], 0) != 0 || ip_octet(&ip[3], 1) != 0 || ip_octet(&ip[3], 2) != 0 ||	ip_octet(&ip[3], 3) != 0 ||
			ip_octet(&ip[3], 4) != 0 || ip_octet(&ip[3], 5) != 0 || ip_octet(&ip[3], 6) != 0 || ip_octet(&ip[3], 7) != 0 ||
			ip_octet(&ip[3], 8) != 0 || ip_octet(&ip[3], 9) != 0 || ip_octet(&ip[3], 10) != 0 || ip_octet(&ip[3], 11) != 0 ||	ip_octet(&ip[3], 12) != 0 ||
			ip_octet(&ip[3], 13) != 0 || ip_octet(&ip[3], 14) != 0 || ip_octet(&ip[3], 15) != 0 || ip_octet(&ip[3], 16) != -1) {
			errmsg = "IPv6 octet test failed.";
		}

		st_free(buffer);
	}

	log_unit("%10.10s\n", (!errmsg ? (status() ? "PASSED" : "SKIPPED") : "FAILED"));
	fail_unless(!errmsg, errmsg);

}
