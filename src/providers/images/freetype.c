
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

extern void *lib_magma;
static chr_t freetype_version[16];

/**
 * @brief	Return the version string of the font library.
 * @return	a pointer to a character string containing the font library version information.
 */
chr_t * lib_version_freetype(void) {

	FT_Library library = NULL;
	FT_Int major = 0, minor = 0, patch = 0;

	// Initialize a library context so we can find out the version.
	if (FT_Init_FreeType_d(&library) == FT_Err_Ok) {
		FT_Library_Version_d(library, &major, &minor, &patch);
		FT_Done_FreeType_d(library);
	}

	// If the library doesn't initialize, print 0.0.0 instead. Otherwise its the version info.
	snprintf(freetype_version, 16, "%i.%i.%i", major, minor, patch);
	return freetype_version;
}

/**
 * @brief	Initialize the font library and bind dynamically to the exported functions that are required.
 * @return	true on success or false on failure.
 */
bool_t lib_load_freetype(void) {

	symbol_t freetype[] = {
		M_BIND(FT_Library_Version), M_BIND(FT_Init_FreeType), M_BIND(FT_Done_FreeType)
	};

	if (lib_symbols(sizeof(freetype) / sizeof(symbol_t), freetype) != 1) {
		return false;
	}

	return true;
}
