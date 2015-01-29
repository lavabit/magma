
/**
 * @file /magma/core/strings/opts.c
 *
 * @brief	Functions for handling managed string options.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Enable or disable a set of option(s) for a managed string, with validity testing.
 * @param	s	the input managed string.
 * @param	opt	the bitmask of option(s) to be enabled or disabled for the managed string.
 * @param	enabled	if true, set the option(s); if false, disable them.
 * @return	-1 on error, 0 if successfully disabled, or 1 if successfully enabled.
 */
int_t st_opt_set(stringer_t *s, uint32_t opt, bool_t enabled) {

	uint32_t opts;

	if (!s || !(opts = *((uint32_t *)s))) {
		return -1;
	}

	else if (!st_valid_opts((enabled ? opts | opt : (opts | opt) ^ opt))) {
		log_pedantic("The set operation would create an illegal option combination. { opts = %s / target = %s / enabled = %s }", st_info_opts(opts, MEMORYBUF(128), 128),
			st_info_opts(opt, MEMORYBUF(128), 128), enabled ? "true" : "false");
		return -1;
	}

#ifdef MAGMA_PEDANTIC
	else if (!st_valid_opts(opts)) {
		log_pedantic("Invalid string options. { opt = %u = %s }", opts, st_info_opts(opts, MEMORYBUF(128), 128));
		return -1;
	}
#endif

	*((uint32_t *)s) = (enabled ? opts | opt : (opts | opt) ^ opt);
	return (enabled ? 1 : 0);
}

/**
 * @brief	Check to see if the managed string has the specified option enabled.
 * @param	s	the managed string to be checked.
 * @opt		opt	a bitmask of the managed string options to be tested.
 * @return	-1 on error, 0 if opt is not set, and 1 if opt is set.
 */
bool_t st_opt_get(stringer_t *s, uint32_t opt) {

	uint32_t opts;

	if (!s || !(opts = *((uint32_t *)s))) {
		return -1;
	}
#ifdef MAGMA_PEDANTIC
	else if (!st_valid_opts(opts)) {
		log_pedantic("Invalid string options. { opt = %u = %s }", opts, st_info_opts(opts, MEMORYBUF(128), 128));
	}
#endif


	return (opts & opt) ? true : false;
}
