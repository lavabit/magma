
/**
 * @file /check/magma/network/network_check.c
 *
 * @brief Check the network functions.
 */

#include "magma_check.h"

Suite * suite_check_network(void) {

	Suite *s = suite_create("\tNetwork");

	suite_check_testcase(s, "NETWORK", "Network / Address / Standard / S", check_address_standard_s);
	suite_check_testcase(s, "NETWORK", "Network / Address / Presentation / S", check_address_presentation_s);
	suite_check_testcase(s, "NETWORK", "Network / Address / Reversed / S", check_address_reversed_s);
	suite_check_testcase(s, "NETWORK", "Network / Address / Subnet / S", check_address_subnet_s);
	suite_check_testcase(s, "NETWORK", "Network / Address / Segment / S", check_address_segment_s);
	suite_check_testcase(s, "NETWORK", "Network / Address / Octet / S", check_address_octet_s);

	return s;
}
