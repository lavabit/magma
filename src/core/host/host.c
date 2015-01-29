
 /**
 * @file /magma/core/host/host.c
 *
 * @brief	Functions to retrieve information about the operating system.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Get a description of the local operating system.
 * @param	output	a pointer to a managed string to receive the OS description.
 * @return	NULL on failure, or the user-specified managed string containing the OS info on success.
 *
 */
stringer_t * host_platform(stringer_t *output) {

	struct utsname os;

	if (!output || uname(&os) < 0) {
		return NULL;
	}

#ifdef MAGMA_PEDANTIC
	if (ns_length_get(os.sysname) > st_avail_get(output)) {
		log_pedantic("Output buffer is not large enough to hold the name of the platform.");
	}
#endif

	if (st_sprint(output, "%s", os.sysname) > st_avail_get(output)) {
		return NULL;
	}

	return output;
}

/**
 * @brief	Get release information about the local OS.
 * @param	output	a pointer to a managed string to receive the release information.
 * @return	NULL on failure, or the user-specified managed string containing the release info on success.
 */
stringer_t * host_version(stringer_t *output) {

	struct utsname os;

	if (!output || uname(&os) < 0) {
		return NULL;
	}

#ifdef MAGMA_PEDANTIC
	if (ns_length_get(os.release) > st_avail_get(output)) {
		log_pedantic("Output buffer is not large enough to hold the platform version.");
	}
#endif

	if (st_sprint(output, "%s", os.release) > st_avail_get(output)) {
		return NULL;
	}

	return output;
}
