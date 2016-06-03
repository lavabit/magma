
/**
 * @file /magma/providers/compress/lzo.c
 *
 * @brief	The interface for the LZO compression functions.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */


#include "magma.h"

/**
 * @brief	Initialize the lzo library and bind dynamically to the exported functions that are required.
 * @return	true on success or false on failure.
 */
bool_t lib_load_lzo(void) {

	symbol_t lzo[] = {
		M_BIND(lzo1x_1_compress), M_BIND(lzo1x_decompress_safe), M_BIND(lzo_adler32), M_BIND(__lzo_init_v2), M_BIND(lzo_version_string)
	};

	if (!lib_symbols(sizeof(lzo) / sizeof(symbol_t), lzo)) {
		return false;
	}

	if (__lzo_init_v2_d(LZO_VERSION, (int)sizeof(short), (int)sizeof(int), (int)sizeof(long), (int)sizeof(lzo_uint32), (int)sizeof(lzo_uint), (int)lzo_sizeof_dict_t, (int)sizeof(char *), (int)sizeof(lzo_voidp), (int)sizeof(lzo_callback_t)) != LZO_E_OK) {
		log_critical("Could not initialize the LZO library.");
		return false;
	}

	return true;
}

/**
 * @brief	Return the version string of the lzo library.
 * @return	a pointer to a character string containing the lzo library version information.
 */
const char * lib_version_lzo(void) {
	return lzo_version_string_d();
}

/**
 * @brief	Decompress a single block of data using the lzo engine.
 * @param	block	a managed string containing the block of compressed data.
 * @return	NULL on failure, or a managed string containing the uncompressed data on success.
 */
stringer_t * decompress_block_lzo(stringer_t *block) {


	int_t ret;
	void *bptr;
	uint64_t rlen, blen;
	stringer_t *result;

	if (!(bptr = st_data_get(block)) || !(blen = st_length_get(block))) {
		return NULL;
	}

	// Input buffers larger than 8192 use a restricted output buffer to ensure we don't experience an input overrun.
	rlen = (blen - (blen % 8192));
	rlen = (rlen ? rlen : blen);

	// The maximum size of the output block.
	if (!(result = st_alloc(rlen))) {
		return NULL;
	}

	// Because we planning to only decompress a portion of the overall data buffer we can't guarantee there won't be an output overrun.
	ret = lzo1x_decompress_safe_d(bptr, blen, st_data_get(result), &rlen, NULL);

	if (ret != LZO_E_OK && ret != LZO_E_OUTPUT_OVERRUN) {
		log_info("Unable to decompress the buffer. {lzo1x_decompress_safe = %i}", ret);
		st_free(result);
		return NULL;
	}

	st_length_set(result, rlen);

	return result;
}

/**
 * @brief	Decompress data using the lzo engine.
 * @param	compressed	a pointer to the head of the compressed data.
 * @return	NULL on failure, or a managed string containing the uncompressed data on success.
 */
stringer_t * decompress_lzo(compress_t *compressed) {

	int ret;
	void *bptr;
	uint64_t hash, rlen, blen;
	stringer_t *result = NULL;
	compress_head_t *head;

	if (!(head = (compress_head_t *)compressed)) {
		log_info("Invalid compression header. {compress_head = NULL}");
		return NULL;
	}
	else if (head->engine != COMPRESS_ENGINE_LZO) {
		log_info("The buffer passed in was not compressed using the LZO engine. {engine = %hhu}", head->engine);
		return NULL;
	}
	else if (!(bptr = compress_body_data(compressed)) || !(blen = head->length.compressed) || !(rlen = head->length.original) ||
			head->hash.compressed != (hash = hash_adler32(bptr, blen))) {
		log_info("The compressed data has been corrupted. {expected = %lu / input = %lu}", head->hash.compressed, hash);
		return NULL;
	}
	else if (!(result = st_alloc(rlen))) {
		log_info("Could not allocate a block of %lu bytes for the uncompressed data.", head->length.original);
		return NULL;
	}
	else if ((ret = lzo1x_decompress_safe_d(bptr, blen, st_data_get(result), &rlen, NULL)) != LZO_E_OK) {
		log_info("Unable to decompress the buffer. {lzo1x_decompress_safe = %i}", ret);
		st_free(result);
		return NULL;
	}
	else if (head->length.original != rlen || head->hash.original != (hash = hash_adler32(st_data_get(result), rlen))) {
		log_info("The uncompressed data is corrupted. {input = %lu != %lu / hash = %lu != %lu}", head->length.original, rlen, head->hash.original, hash);
		st_free(result);
		return NULL;
	}

	st_length_set(result, rlen);

	return result;
}

/**
 * @brief	Compress data using the lzo engine.
 * @param	input	a managed string containing the data to be compressed.
 * @return	NULL on failure, or a pointer to the head of the compressed data on success.
 */
compress_t * compress_lzo(stringer_t *input) {

	uint64_t out;
	compress_head_t *head;
	lzo_byte *wrkmem = NULL;
	compress_t *result = NULL;

	// This represents the maximum amount of space the compressed block could end up using.
	out = st_length_get(input) + (st_length_get(input) / 16) + 64 + 3;

	// Allocate a buffer to hold the result and the working memory buffer.
	if (!(head = (compress_head_t *)(result = compress_alloc(out))) || !(wrkmem = mm_alloc(LZO1X_1_MEM_COMPRESS))) {
		log_info("Unable to allocate the compression buffers.");

		if (result) {
			compress_free(result);
		}

		return NULL;
	}

	// Setup the header.
	head->engine = COMPRESS_ENGINE_LZO;
	head->length.original = st_length_get(input);
	head->hash.original = hash_adler32(st_data_get(input), st_length_get(input));

	// Perform the compression.
	if (lzo1x_1_compress_d(st_data_get(input), st_length_get(input), compress_body_data(result), &out, wrkmem) != LZO_E_OK) {
		log_info("Unable to compress the buffer.");
		compress_free(result);
		mm_free(wrkmem);
		return NULL;
	}

	mm_free(wrkmem);

	head->length.compressed = out;
	head->hash.compressed = hash_adler32(compress_body_data(result), out);

#ifdef MAGMA_PEDANTIC
	stringer_t *verify;

	if (!(verify = decompress_lzo(result))) {
		log_info("Verification failed!");
		compress_free(result);
		return NULL;
	}

	st_free(verify);
#endif

	return result;
}
