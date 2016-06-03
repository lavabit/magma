
/**
 * @file /magma/providers/images/png.c
 *
 * @brief The functions used to create and modify PNG image files
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

static chr_t png_version[16];

/**
 * @brief	Return the version string of png library.
 * @return	a pointer to a character string containing the png library version information.
 */
chr_t * lib_version_png(void) {

	uint32_t holder;
	uint16_t version, release, major;

	if ((holder = png_access_version_number_d()) == 0) {
		return NULL;
	}

	version = holder % 100;
	holder /= 100;
	release = holder % 100;
	holder /= 100;
	major = holder % 100;

	snprintf(png_version, 16, "%hu.%hu.%hu", major, release, version);
	return png_version;
}

/**
 * @brief	Initialize the png library and bind dynamically to the exported functions that are required.
 * @return	true on success or false on failure.
 */
bool_t lib_load_png(void) {

	symbol_t png[] = {
		M_BIND(png_access_version_number)
	};

	if (lib_symbols(sizeof(png) / sizeof(symbol_t), png) != 1) {
		return false;
	}

	return true;
}
