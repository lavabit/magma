
/**
 * @file /magma/providers/images/gd.c
 *
 * @brief The functions used to create and images using the GD library.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Return the version string of the gd library.
 * @return	a pointer to a character string containing the gd library version information.
 */
chr_t * lib_version_gd(void) {
	return (chr_t *)gd_version_d();
}

/**
 * @brief	Initialize the gd library and bind dynamically to the exported functions that are required.
 * @return	true on success or false on failure.
 */
bool_t lib_load_gd(void) {

	symbol_t gd[] = {
		M_BIND(gdFree), M_BIND(gdImageColorResolve), M_BIND(gdImageCreate),	M_BIND(gdImageDestroy), M_BIND(gdImageGifPtr),
		M_BIND(gdImageJpegPtr),	M_BIND(gdImageSetPixel), M_BIND(gdImageStringFT), M_BIND(gd_version)
	};

	if (lib_symbols(sizeof(gd) / sizeof(symbol_t), gd) != 1) {
		return false;
	}

	return true;
}
