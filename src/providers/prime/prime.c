
/**
 * @file /magma/src/providers/prime/prime.c
 *
 * @brief DESCRIPTIONxxxGOESxxxHERE
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

EC_GROUP *prime_curve_group = NULL;


/**
 * @brief	Initialize the PRIME structures.
 * @return	returns true if everything initializes properly, or false if an error occurrs.
 */
bool_t prime_start(void) {

	if (!(prime_curve_group = EC_GROUP_new_by_curve_name_d(NID_secp256k1))) {
		log_error("An error occurred while trying to create the elliptical group. {%s}", ssl_error_string(MEMORYBUF(256), 256));
		return false;
	}
	else if (EC_GROUP_precompute_mult_d(prime_curve_group, NULL) != 1) {
		log_error("Unable to precompute the required elliptical curve point data. {%s}", ssl_error_string(MEMORYBUF(256), 256));
		EC_GROUP_free_d(prime_curve_group);
		prime_curve_group = NULL;
		return false;
	}

	EC_GROUP_set_point_conversion_form_d(prime_curve_group, POINT_CONVERSION_COMPRESSED);

	return true;
}

/**
 * @brief	Destroy any initialized PRIME structures.
 * @return	This function returns no value.
 */
void prime_stop(void) {

	EC_GROUP *group;

	if (prime_curve_group) {
		group = prime_curve_group;
		prime_curve_group = NULL;
		EC_GROUP_free_d(group);
	}

	return;
}
