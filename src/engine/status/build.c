
/**
 * @file /magma/engine/status/build.c
 *
 * @brief	Functions for retrieving the version and build information.
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

/**
 * @brief	Get the magma major version number.
 * @return	an unsigned integer with the major version number.
 */
uint64_t build_version_major(void) {

	uint64_t major = 0;
	placer_t holder = pl_null();

	// Try to extract the major version. If anything goes wrong, return the initialized value of 0.
	if (tok_get_st(PLACER(MAGMA_VERSION, ns_length_get(MAGMA_VERSION)), '.', 0, &holder) >= 0)
		uint64_conv_pl(holder, &major);

	return major;
}

/**
 * @brief	Get the magma minor version number.
 * @return	an unsigned integer with the minor version number.
 */
uint64_t build_version_minor(void) {

	uint64_t minor = 0;
	placer_t holder = pl_null();

	// Try to extract the minor version. If anything goes wrong, return the initialized value of 0.
	if (tok_get_st(PLACER(MAGMA_VERSION, ns_length_get(MAGMA_VERSION)), '.', 1, &holder) >= 0)
		uint64_conv_pl(holder, &minor);

	return minor;
}

/**
 * @brief	Get the magma patch level.
 * @return	an unsigned integer with the patch level.
 */
uint64_t build_version_patch(void) {

	uint64_t patch = 0;
	placer_t holder = pl_null();

	// Try to extract the patch level. If anything goes wrong, return the initialized value of 0.
	if (tok_get_st(PLACER(MAGMA_VERSION, ns_length_get(MAGMA_VERSION)), '.', 2, &holder) >= 0)
		uint64_conv_pl(holder, &patch);

	return patch;
}
