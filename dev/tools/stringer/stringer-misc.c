/**
 * @file /stringer/stringer-misc.c
 *
 * @brief A collection of functions used handle common bit manipulation tasks.
 *
 * $Author: Ladar Levison $
 * $Date: 2010/12/12 23:59:04 $
 * $Revision: a46af76362b976a414117a5bfee9dfc2e9fd38b2 $
 *
 */

#include "stringer.h"

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
	"FIXED",
	"HEAP",
	"SECURE"
};

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

chr_t * st_info_opts(uint32_t opts, chr_t *s, size_t len) {

	chr_t *result = NULL;

#ifdef MAGMA_PEDANTIC
	if (len < 34) log_pedantic("The output buffer is smaller than the maximum possible length of 34.");
#endif

	if (s && len) {
		snprintf(s, len, "(%s | %s | %s)", st_info_type(opts), st_info_layout(opts), st_info_allocator(opts));
		result = s;
	}

	return result;
}

/**
 * Counts the number of bit positions set to one.
 *
 * @param value Any integer value needing to be counted.
 * @return Returns the number of bits set to one.
 */
uint32_t bits_count(uint64_t value) {

	uint32_t result = 0;

	while (value != 0) {
		value &= value - 1;
		result++;
	}

	return result;
}


