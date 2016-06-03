
/**
 * @file /magma/providers/compress/zlib.c
 *
 * @brief The interface for the ZLIB compression functions.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

int (*deflateEnd_d)(z_streamp strm) = NULL;
int (*deflate_d)(z_streamp strm, int flush) = NULL;
int (*deflateInit2__d)(z_streamp strm, int level, int method, int windowBits, int memLevel, int strategy, const char *version, int stream_size) = NULL;


/**
 * @brief	Return the version string of zlib.
 * @return	a pointer to a character string containing the zlib version information.
 */
const char * lib_version_zlib(void) {
	return zlibVersion_d();
}

/**
 * @brief	Initialize the zlib library and bind dynamically to the exported functions that are required.
 * @return	true on success or false on failure.
 */
bool_t lib_load_zlib(void) {

	symbol_t zlib[] = {
		M_BIND(compress2), M_BIND(compressBound), M_BIND(deflate), M_BIND(deflateEnd),	M_BIND(deflateInit2_),
		M_BIND(uncompress),	M_BIND(zlibVersion)
	};

	if (lib_symbols(sizeof(zlib) / sizeof(symbol_t), zlib) != 1) {
		return false;
	}
	return true;
}

/**
 * @brief	Decompress a block of data using the zlib engine.
 * @param	compressed	a pointer to the head of the compressed data.
 * @return	NULL on failure (e.g. corruption), or a managed string containing the uncompressed data on success.
 */
stringer_t * decompress_zlib(compress_t *compressed) {

	int ret;
	void *bptr;
	uint64_t hash, rlen, blen;
	stringer_t *result = NULL;
	compress_head_t *head;

	if (!(head = (compress_head_t *)compressed)) {
		log_info("Invalid compression header. {compress_head = NULL}");
		return NULL;
	} else if (head->engine != COMPRESS_ENGINE_ZLIB) {
		log_info("The buffer passed in was not compressed using the ZLIB engine. {engine = %hhu}", head->engine);
		return NULL;
	} else if (!(bptr = compress_body_data(compressed)) || !(blen = head->length.compressed) || !(rlen = head->length.original) || head->hash.compressed != (hash = hash_adler32(bptr, blen))) {
		log_info("The compressed has been corrupted. {expected = %lu / input = %lu}", head->hash.compressed, hash);
		return NULL;
	} else if (!(result = st_alloc(head->length.original + 1))) {
		log_info("Could not allocate a block of %lu bytes for the uncompressed data.", head->length.original);
		return NULL;
	} else if ((ret = uncompress_d(st_data_get(result), &rlen, bptr, blen)) != Z_OK) {
		log_info("Unable to decompress the buffer. {uncompress = %i}", ret);
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
 * @brief	Compress a block of data using the zlib engine.
 * @param	input	a managed string containing the data to be compressed.
 * @return	NULL on failure, or a pointer to the compressed data header on success..
 */
compress_t * compress_zlib(stringer_t *input) {
/*
	z_stream zs;
	size_t zbufsize =  pl_get_length(input) + 512;

	zs.zalloc = Z_NULL;
	zs.zfree = Z_NULL;
	zs.opaque = Z_NULL;
	if (deflateInit2__d(&zs, 5, Z_DEFLATED, -15, 7, Z_DEFAULT_STRATEGY, ZLIB_VERSION, sizeof(z_stream)) != Z_OK)
		return NULL;

	int asiz = pl_get_length(input) + 16;
	if (asiz < zbufsize)
		asiz = zbufsize;
	char *buf;
	if (!(buf = mm_alloc(asiz))) {
		deflateEnd_d(&zs);
		return NULL;
	}

	unsigned char obuf[zbufsize];
	int bsiz = 0;
	zs.next_in = (unsigned char *)pl_data_get(input);
	zs.avail_in = pl_get_length(input);
	zs.next_out = obuf;
	zs.avail_out = zbufsize;
	int rv;

	while ((rv = deflate_d(&zs, Z_FINISH)) == Z_OK) {
		int osiz = zbufsize - zs.avail_out;
		if (bsiz + osiz > asiz) {
			asiz = asiz * 2 + osiz;
			char *swap;
			if (!(swap = realloc(buf, asiz))) {
				free(buf);
				deflateEnd_d(&zs);
				return NULL;
			}
			buf = swap;
		}
		memcpy(buf + bsiz, obuf, osiz);
		bsiz += osiz;
		zs.next_out = obuf;
		zs.avail_out = zbufsize;
	}
	if (rv != Z_STREAM_END) {
		free(buf);
		deflateEnd_d(&zs);
		return NULL;
	}
	int osiz = zbufsize - zs.avail_out;
	if (bsiz + osiz + 1 > asiz) {
		asiz = asiz * 2 + osiz;
		char *swap;
		if (!(swap = realloc(buf, asiz))) {
			free(buf);
			deflateEnd_d(&zs);
			return NULL;
		}
		buf = swap;
	}
	memcpy(buf + bsiz, obuf, osiz);
	bsiz += osiz;

	deflateEnd_d(&zs);

	compress_head_t *head;
	compress_t *result = NULL;

	if (!(head = (compress_head_t *)(result = compress_alloc(bsiz)))) {
		log_info("Unable to allocate the compression buffers.");
		return NULL;
	}

	// Setup the header.
	head->engine = COMPRESS_ENGINE_ZLIB;
	head->length.original = pl_get_length(input);
	head->hash.original = hash_adler32(pl_data_get(input), pl_get_length(input));

	memcpy(compress_body_data(result), buf, bsiz);
	head->length.compressed = bsiz;
	head->hash.compressed = hash_adler32(compress_body_data(result), bsiz);

	free(buf);

#ifdef MAGMA_PEDANTIC
	stringer_t *verify;

	if (!(verify = decompress_zlib(result))) {
		log_info("Verification failed!");
		compress_free(result);
		return NULL;
	}

	st_free(verify);
#endif

	return result;
	*/
	 uint64_t out;
	 compress_head_t *head;
	 compress_t *result = NULL;

	 // This represents the maximum amount of space the compressed block could end up using.
	 out = compressBound_d(st_length_get(input));

	 // Allocate a buffer to hold the result and the working memory buffer.
	 if (!(head = (compress_head_t *)(result = compress_alloc(out)))) {
	 log_info("Unable to allocate the compression buffers.");
	 return NULL;
	 }

	 // Setup the header.
	 head->engine = COMPRESS_ENGINE_ZLIB;
	 head->length.original = st_length_get(input);
	 head->hash.original = hash_adler32(st_data_get(input), st_length_get(input));

	 // Perform the compression.
	 if (compress2_d(compress_body_data(result), &out, st_data_get(input), st_length_get(input), 9) != Z_OK) {
	 log_info("Unable to compress the buffer.");
	 compress_free(result);
	 return NULL;
	 }

	 head->length.compressed = out;
	 head->hash.compressed = hash_adler32(compress_body_data(result), out);

	 #ifdef MAGMA_PEDANTIC
	 stringer_t *verify;

	 if (!(verify = decompress_zlib(result))) {
	 log_info("Verification failed!");
	 compress_free(result);
	 return NULL;
	 }

	 st_free(verify);
	 #endif

	 return result;
}
