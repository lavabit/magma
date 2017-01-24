
/**
 * @file /magma/providers/images/images.h
 *
 * @brief The functions used to create and modify image files. For our purposes that include fonts.
 */

#ifndef MAGMA_PROVIDERS_IMAGES_H
#define MAGMA_PROVIDERS_IMAGES_H

/// gd.c
bool_t   lib_load_gd(void);
chr_t *  lib_version_gd(void);

/// freetype.c
bool_t   lib_load_freetype(void);
chr_t *  lib_version_freetype(void);

/// jpeg.c
bool_t   lib_load_jpeg(void);
chr_t *  lib_version_jpeg(void);

/// png.c
bool_t   lib_load_png(void);
chr_t *  lib_version_png(void);

#endif

