
/**
 * @file /check/magma/network/network_check.h
 *
 * @brief Check the network functions.
 */

#ifndef NETWORK_CHECK_H
#define NETWORK_CHECK_H

/// address_check.c
void check_address_octet_s (int _i CK_ATTRIBUTE_UNUSED);
void check_address_presentation_s (int _i CK_ATTRIBUTE_UNUSED);
void check_address_reversed_s (int _i CK_ATTRIBUTE_UNUSED);
void check_address_segment_s (int _i CK_ATTRIBUTE_UNUSED);
void check_address_standard_s (int _i CK_ATTRIBUTE_UNUSED);
void check_address_subnet_s (int _i CK_ATTRIBUTE_UNUSED);

Suite * suite_check_network(void);






#endif

