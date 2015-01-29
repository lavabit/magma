
/**
 * @file /magma/core/strings/info.c
 *
 * @brief	A collection of functions used to extract information from stringer options.
 *
 * $Author$
 * $Author$
 * $Revision$
 *
 */

#include "magma.h"

chr_t *st_option_flags[] = {
	"FOREIGNDATA"
};

chr_t *st_option_types[] = {
	"UNKNOWN",
	"CONSTANT",
	"PLACER",
	"NULLER",
	"BLOCK",
	"MANAGED",
	"MAPPED"
};

chr_t *st_option_layouts[] = {
	"UNKNOWN",
	"CONTIGUOUS",
	"JOINTED"
};

chr_t *st_option_allocators[] = {
	"UNKNOWN",
	"STACK",
	"HEAP",
	"SECURE"
};

/**
 * @brief	Get a readable description of a managed string's data type.
 * @param	opts	a  value containing the managed string's option mask.
 * @return	a pointer to a null-terminated string containing a description of the managed string's data type.
 */
const chr_t * st_info_type(uint32_t opts) {

	chr_t *result = st_option_types[0];

	switch (opts & (CONSTANT_T | NULLER_T | PLACER_T | BLOCK_T | MANAGED_T | MAPPED_T)) {
		case (CONSTANT_T):
			result = st_option_types[1];
			break;
		case (PLACER_T):
			result = st_option_types[2];
			break;
		case (NULLER_T):
			result = st_option_types[3];
			break;
		case (BLOCK_T):
			result = st_option_types[4];
			break;
		case (MANAGED_T):
			result = st_option_types[5];
			break;
		case (MAPPED_T):
			result = st_option_types[6];
			break;
	}

	return result;
}

/**
 * @brief	Get a readable description of a managed string's data layout.
 * @param	opts	a  value containing the managed string's option mask.
 * @return	a pointer to a null-terminated string containing a description of the managed string's data layout.
 */
const chr_t * st_info_layout(uint32_t opts) {

	chr_t *result = st_option_layouts[0];

	switch (opts & (CONTIGUOUS | JOINTED)) {
		case (CONTIGUOUS):
			result = st_option_layouts[1];
			break;
		case (JOINTED):
			result = st_option_layouts[2];
			break;
	}

	return result;
}

/**
 * @brief	Get a readable description of a managed string's allocator type.
 * @param	opts	a  value containing the managed string's option mask.
 * @return	a pointer to a null-terminated string containing a description of the managed string's allocator type.
 */
const chr_t * st_info_allocator(uint32_t opts) {

	chr_t *result = st_option_allocators[0];

	switch (opts & (STACK | HEAP | SECURE)) {
		case (STACK):
			result = st_option_allocators[1];
			break;
		case (HEAP):
			result = st_option_allocators[2];
			break;
		case (SECURE):
			result = st_option_allocators[3];
			break;
	}

	return result;
}

/**
 * @brief	Get a readable description of all of a managed string's option flags.
 * @param	opts	a value containing the managed string's option mask.
 * @param	s		a pointer to a null-terminated string to receive the options description.
 * @param	len		the length, in bytes, of the description output buffer.
 * @return	NULL on failure, or a pointer to the output buffer containing the managed string's options description on success.
 */
chr_t * st_info_opts(uint32_t opts, chr_t *s, size_t len) {

	chr_t *result = NULL;
	stringer_t *flags = NULL;

#ifdef MAGMA_PEDANTIC
	/// LOW: We should calculate this at startup by iterating through the arrays above and summing the lengths.
	if (len < 34) log_pedantic("The output buffer is smaller than the minimum possible length of 34.");
#endif

	if (s && len) {

		// Flags
		/// LOW: Turn this into a loop.
		if (opts & FOREIGNDATA) {
			flags = st_append(flags, NULLER(st_option_flags[0]));
		}

		snprintf(s, len, "(%s | %s | %s%s%.*s)", st_info_type(opts), st_info_layout(opts), st_info_allocator(opts), st_empty(flags) ? "" : " | ", st_length_int(flags), st_char_get(flags));
		st_cleanup(flags);
		result = s;
	}

	return result;
}
