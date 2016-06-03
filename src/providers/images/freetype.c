
/**
 * @file /magma/providers/images/freetype.c
 *
 * @brief Functions used to handle fonts.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

static chr_t freetype_version[16];

/**
 * @brief	Return the version string of the font library.
 * @return	a pointer to a character string containing the font library version information.
 */
chr_t * lib_version_freetype(void) {

	int_t major, minor, patch;

	FT_Library_Version_Static_d(&major, &minor, &patch);
	snprintf(freetype_version, 16, "%hu.%hu.%hu", major, minor, patch);
	return freetype_version;
}
/**
 * @brief	Initialize the font library and bind dynamically to the exported functions that are required.
 * @return	true on success or false on failure.
 */
bool_t lib_load_freetype(void) {

	symbol_t freetype[] = {
		M_BIND(FT_Library_Version_Static)
	};

	if (lib_symbols(sizeof(freetype) / sizeof(symbol_t), freetype) != 1) {
		return false;
	}

	return true;
}
