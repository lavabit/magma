
/**
 * @file /magma/engine/status/build.c
 *
 * @brief	Functions for retrieving the version and build information.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

#ifndef MAGMA_BUILD
#error Magma build version is missing.
#endif

#ifndef MAGMA_STAMP
#error Magma timestamp is missing.
#endif

/**
 * @brief	Get the magma version string.
 * @return	a pointer to a null-terminated string containing the magma version string.
 */
const char * build_version(void) {
	return (const char *)MAGMA_BUILD;
}

/**
 * @brief	Get the magma build stamp.
 * @return	a pointer to a null-terminated string containing the magma build information string.
 */
const char * build_stamp(void) {
	return (const char *)MAGMA_STAMP;
}

