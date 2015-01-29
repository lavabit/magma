
/**
 * @file /mason/lzo.c
 *
 * @brief LZO engine functions.
 *
 * $Author: Ladar Levison $
 * $Date: 2010/07/03 16:59:52 $
 * $Revision: cafc504385e565e4ff418602dce85acc8bcdddff $
 *
 */

#include "mason.h"

extern bool lzo1;
extern bool lzo999;
extern lzo_byte *wrkmem;

/**
 * Decompress a block of data using the LZO engine.
 *
 * @param block The compressed data block.
 * @param length The length of the compressed block.
 * @param uncompressed The size of the uncompressed data.
 * @return The amount of data uncompressed, or 0 if an error occurs. The original buffer is freed, and block is pointed at the newly compressed buffer.
 */
size_t lzo_decompress(void **block, size_t length, size_t uncompressed) {

	void *result = NULL;

	if (!(result = malloc(uncompressed))) {
		fprintf(stderr, "Could not allocate %li bytes for holding the compressed data.", uncompressed);
		fflush(stderr);
		return 0;
	}

	if (lzo1x_decompress_safe(*(const unsigned char **)block, length, result, &uncompressed, NULL) != LZO_E_OK) {
		fprintf(stderr, "Unable to decompress the buffer.");
		free(result);
		return 0;
	}

	free(*block);
	*block = result;
	return uncompressed;
}

/**
 * Compresses the buffer using the LZO engine.
 *
 * @param block The buffer holding the uncompressed data. If successful *block is freed, and replaced with a pointer to the compressed data.
 * @param length The length of the buffer.
 * @return Returns the compressed buffer length or 0 if an error occurs. The original buffer is freed, and block is pointed at the newly compressed buffer.
 */
size_t lzo_compress(void **block, size_t length) {

	void *result = NULL;
	lzo_uint out = length + length / 16 + 64 + 3;
	if (!(result = malloc(out))) {
		fprintf(stderr, "Could not allocate %li bytes for holding the compressed data.", length + length / 16 + 64 + 3);
		fflush(stderr);
		return 0;
	}
	if (lzo1 && lzo1x_1_compress(*block, length, result, &out, wrkmem) != LZO_E_OK) {
		fprintf(stderr, "Unable to compress the buffer.");
		fflush(stderr);
		free(result);
		return 0;
	} else if (lzo999 && lzo1x_999_compress(*block, length, result, &out, wrkmem) != LZO_E_OK)   {
		fprintf(stderr, "Unable to compress the buffer.");
		fflush(stderr);
		free(result);
		return 0;
	}
	free(*block);
	*block = result;
	return out;
}

