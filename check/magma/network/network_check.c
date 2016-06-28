
/**
 * @file /check/network/network_check.c
 *
 * @brief Check the network functions.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma_check.h"

Suite * suite_check_network(void) {

	TCase *tc;
	Suite *s = suite_create("\tNetwork");

	testcase(s, tc, "Network / Address / Standard / S", check_address_standard_s);
	testcase(s, tc, "Network / Address / Presentation / S", check_address_presentation_s);
	testcase(s, tc, "Network / Address / Reversed / S", check_address_reversed_s);
	testcase(s, tc, "Network / Address / Subnet / S", check_address_subnet_s);
	testcase(s, tc, "Network / Address / Segment / S", check_address_segment_s);
	testcase(s, tc, "Network / Address / Octet / S", check_address_octet_s);


	return s;
}
