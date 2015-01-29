
/**
 * @file /magma/objects/warehouse/warehouse.c
 *
 * @brief	The warehouse management functions.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Update the warehouse components.
 * @note	This will update the patterns list.
 * @return	This function returns no value.
 */
void warehouse_update(void) {

	pattern_update();

	return;
}

/**
 * @brief	Stop the warehouse facility.
 * @note	This will destroy the patterns and domain lists.
 * @return	This function returns no value.
 */
void warehouse_stop(void) {

	pattern_stop();
	domain_stop();

	return;
}

/**
 * @brief	Start the warehouse facility.
 * @note	This will initialize the domains and patterns lists.
 * @return	false if any of the warehouse components failed to load, or true on success.
 */
bool_t warehouse_start(void) {

	// Load the domain name configuration.
	if (!domain_start()) {
		warehouse_stop();
		return false;
	}
	else if (!pattern_start()) {
		warehouse_stop();
		return false;
	}

	return true;
}
