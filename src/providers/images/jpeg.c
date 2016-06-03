
/**
 * @file /magma/providers/images/jpeg.c
 *
 * @brief The functions used to create and modify JPEG image files
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Return the version string of the jpeg library.
 * @return	a pointer to a character string containing the jpeg library version information.
 */
chr_t * lib_version_jpeg(void) {
	return (chr_t *)jpeg_version_d();
}

/**
 * @brief	Initialize the jpeg library and bind dynamically to the exported functions that are required.
 * @return	true on success or false on failure.
 */
bool_t lib_load_jpeg(void) {

	symbol_t jpeg[] = {
		M_BIND(jpeg_version)
	};

	if (lib_symbols(sizeof(jpeg) / sizeof(symbol_t), jpeg) != 1) {
		return false;
	}

	return true;
}
