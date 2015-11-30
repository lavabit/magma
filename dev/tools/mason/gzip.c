
/**
 * @file /mason/gzip.c
 *
 * @brief Gzip engine functions.
 *
 * $Author: Ladar Levison $
 * $Date: 2010/07/03 16:59:52 $
 * $Revision: cafc504385e565e4ff418602dce85acc8bcdddff $
 *
 */

#include "mason.h"


/**
 * Decompress a block of data using the Gzip engine.
 *
 * @param block The compressed data block.
 * @param length The length of the compressed block.
 * @param uncompressed The size of the uncompressed data.
 * @return The amount of data uncompressed, or 0 if an error occurs. The original buffer is freed, and block is pointed at the newly compressed buffer.
 */
size_t gzip_decompress(void **block, size_t length, size_t uncompressed) {

	void *result = NULL;

	if (!(result = malloc(uncompressed))) {
		fprintf(stderr, "Could not allocate %li bytes for holding the compressed data.", uncompressed);
		fflush(stderr);
		return 0;
	}

	if (uncompress(result, &uncompressed, *(const unsigned char **)block, length) != Z_OK) {
		fprintf(stderr, "Unable to decompress the buffer.");
		free(result);
		return 0;
	}

	free(*block);
	*block = result;
	return uncompressed;
}

/**
 * Compresses the buffer using the Gzip engine.
 *
 * @param block The buffer holding the uncompressed data. If successful *block is freed, and replaced with a pointer to the compressed data.
 * @param length The length of the buffer.
 * @return Returns the compressed buffer length or 0 if an error occurs. The original buffer is freed, and block is pointed at the newly compressed buffer.
 */
size_t gzip_compress(void **block, size_t length) {

	void *result = NULL;
	size_t out = compressBound(length);

	if (!(result = malloc(out))) {
		fprintf(stderr, "Could not allocate %li bytes for holding the compressed data.", out);
		fflush(stderr);
		return 0;
	}

	if (compress2(result, &out, *block, length, 9) != Z_OK)   {
		fprintf(stderr, "Unable to compress the buffer.");
		fflush(stderr);
		free(result);
		return 0;
	}

	free(*block);
	*block = result;
	return out;
}


