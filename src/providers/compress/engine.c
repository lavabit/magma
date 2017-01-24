
/**
 * @file /magma/providers/compress/engine.c
 *
 * @brief Generic engine independent compression interfaces.
 */

#include "magma.h"

compress_t * engine_compress(uint8_t engine, stringer_t *s) {

	compress_t *result = NULL;

	switch (engine) {
		case(COMPRESS_ENGINE_LZO):
			result = compress_lzo(s);
			break;

		case(COMPRESS_ENGINE_ZLIB):
			result = compress_zlib(s);
			break;

		case(COMPRESS_ENGINE_BZIP):
			result = compress_bzip(s);
			break;

		default:
			log_pedantic("Invalid compression engine provided. {engine = %hhu}", engine);
			break;
	}

	return result;
}

stringer_t * engine_decompress(compress_t *buffer) {

	stringer_t *result = NULL;
	compress_head_t *head = (compress_head_t *)buffer;

	switch (head->engine) {
		case(COMPRESS_ENGINE_LZO):
			result = decompress_lzo(buffer);
			break;

		case(COMPRESS_ENGINE_ZLIB):
			result = decompress_zlib(buffer);
			break;

		case(COMPRESS_ENGINE_BZIP):
			result = decompress_bzip(buffer);
			break;

		default:
			log_pedantic("Invalid compression engine indicator. {engine = %hhu}", head->engine);
			break;
	}

	return result;
}
