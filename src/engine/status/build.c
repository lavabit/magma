
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

#ifndef MAGMA_VERSION
#error Magma build version is missing.
#endif

#ifndef MAGMA_COMMIT
#error Magma commit identity is missing.
#endif

#ifndef MAGMA_TIMESTAMP
#error Magma timestamp is missing.
#endif

/**
 * @brief	Get the magma version string.
 * @return	a pointer to a null-terminated string containing the magma version string.
 */
const char * build_version(void) {
	return (const char *)MAGMA_VERSION;
}

/**
 * @brief	Get the magma commit identity.
 * @return	a pointer to a null-terminated string containing the last eight characters of current commit identity.
 */
const char * build_commit(void) {
	return (const char *)MAGMA_COMMIT;
}

/**
 * @brief	Get the magma build stamp.
 * @return	a pointer to a null-terminated string containing the magma build information string.
 */
const char * build_stamp(void) {
	return (const char *)MAGMA_TIMESTAMP;
}

