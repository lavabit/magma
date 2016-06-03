
/**
 * @file /magma/providers/compress/bzip.c
 *
 * @brief The interface for the BZIP compression functions.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

char bzip_version[16];

/**
 * @brief	Return the version string of the bzip library.
 * @return	a pointer to a character string containing the bzip library version information.
 */
const char * lib_version_bzip(void) {
	return bzip_version;
}

/**
 * @brief	Initialize the bzip library and bind dynamically to the exported functions that are required.
 * @return	true on success or false on failure.
 */
bool_t lib_load_bzip(void) {

	int len;
	char *ver;
	symbol_t bzip[] = {
		M_BIND(BZ2_bzBuffToBuffCompress), M_BIND(BZ2_bzBuffToBuffDecompress), M_BIND(BZ2_bzlibVersion)
	};

	if (lib_symbols(sizeof(bzip) / sizeof(symbol_t), bzip) != 1) {
		return false;
	}

	// Expecting a version string similar to 1.0.5, 10-Dec-2007.
	if (!(ver = (char *)BZ2_bzlibVersion_d()) || !(len = ns_length_int(ver))) {
		log_pedantic("Invalid BZIP version string.");
		return false;
	}

	for (size_t i = 0; i < len; i++) {
		if (*(ver + i) != '.' && !(*(ver + i) >= '0' && *(ver + i) <= '9')) {
			len = i;
		}
	}

	if (!snprintf(bzip_version, 16, "%.*s", len, ver)) {
		log_pedantic("Invalid BZIP version string.");
		return false;
	}

	return true;
}

/**
 * @brief	Decompress data using the bzip engine.
 * @param	compressed	a pointer to the head of the compressed data.
 * @return	NULL on failure, or a managed string containing the uncompressed data on success.
 */
stringer_t * decompress_bzip(compress_t *compressed) {

	int ret;
	void *bptr;
	uint64_t hash, rlen, blen;
	stringer_t *result = NULL;
	compress_head_t *head;

	if (!(head = (compress_head_t *)compressed)) {
		log_info("Invalid compression header. {compress_head = NULL}");
		return NULL;
	} else if (head->engine != COMPRESS_ENGINE_BZIP) {
		log_info("The buffer passed in was not compressed using the BZIP engine. {engine = %hhu}", head->engine);
		return NULL;
	} else if (!(bptr = compress_body_data(compressed)) || !(blen = head->length.compressed) || !(rlen = head->length.original) || head->hash.compressed != (hash = hash_adler32(bptr, blen))) {
		log_info("The compressed has been corrupted. {expected = %lu / input = %lu}", head->hash.compressed, hash);
		return NULL;
	} else if (!(result = st_alloc(head->length.original + 1))) {
		log_info("Could not allocate a block of %lu bytes for the uncompressed data.", head->length.original);
		return NULL;
	} else if ((ret = BZ2_bzBuffToBuffDecompress_d(st_data_get(result), (unsigned int *)&rlen, bptr, blen, 0, 0)) != BZ_OK) {
		log_info("Unable to decompress the buffer. {BZ2_bzBuffToBuffDecompress = %i}", ret);
		st_free(result);
		return NULL;
	} else if (head->length.original != rlen || head->hash.original != (hash = hash_adler32(st_data_get(result), rlen))) {
		log_info("The uncompressed data is corrupted. {input = %lu != %lu / hash = %lu != %lu}", head->length.original, rlen, head->hash.original, hash);
		st_free(result);
		return NULL;
	}

	st_length_set(result, rlen);
	return result;
}

/**
 * @brief	Compress data using the bzip engine.
 * @param	input	a managed string containing the data to be compressed.
 * @return	NULL on failure, or a pointer to the head of the compressed data on success.
 */
compress_t * compress_bzip(stringer_t *input) {

	uint64_t out;
	compress_head_t *head;
	compress_t *result = NULL;

	// This represents the maximum amount of space the compressed block could end up using.
	out = ((double)st_length_get(input) * 1.10L) + 1024;

	// Allocate a buffer to hold the result and the working memory buffer.
	if (!(head = (compress_head_t *)(result = compress_alloc(out)))) {
		log_info("Unable to allocate the compression buffers.");
		return NULL;
	}

	// Setup the header.
	head->engine = COMPRESS_ENGINE_BZIP;
	head->length.original = st_length_get(input);
	head->hash.original = hash_adler32(st_data_get(input), st_length_get(input));

	// Perform the compression.
	if (BZ2_bzBuffToBuffCompress_d(compress_body_data(result), (unsigned int *)&out, st_data_get(input), st_length_get(input), 9, 0, 0) != BZ_OK) {
		log_info("Unable to compress the buffer.");
		compress_free(result);
		return NULL;
	}

	head->length.compressed = out;
	head->hash.compressed = hash_adler32(compress_body_data(result), out);

#ifdef MAGMA_PEDANTIC
	stringer_t *verify;

	if (!(verify = decompress_bzip(result))) {
		log_info("Verification failed!");
		compress_free(result);
		return NULL;
	}

	st_free(verify);
#endif

	return result;
}

