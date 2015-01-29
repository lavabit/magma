
/**
 * @file /magma/providers/compress/compress.h
 *
 * @brief Compression interface functions/handlers.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_PROVIDERS_EXTERNAL_COMPRESS_H
#define MAGMA_PROVIDERS_EXTERNAL_COMPRESS_H

enum {
	COMPRESS_ENGINE_LZO = 1,
	COMPRESS_ENGINE_ZLIB = 2,
	COMPRESS_ENGINE_BZIP = 4
} COMPRESS_ENGINE;

typedef struct {

	uint8_t engine;

	struct {
		uint64_t original;
		uint64_t compressed;
	} length;

	struct {
		uint64_t original;
		uint64_t compressed;
	} hash;

} __attribute__ ((packed)) compress_head_t;

typedef stringer_t compress_t;

/// bzip.c
bool_t lib_load_bzip(void);
const char * lib_version_bzip(void);
compress_t * compress_bzip(stringer_t *input);
stringer_t * decompress_bzip(compress_t *compressed);

/// compress.c
compress_t *  compress_alloc(size_t length);
size_t        compress_block_length(void);
void *        compress_body_data(compress_t *buffer);
uint64_t      compress_body_hash(compress_t *buffer);
uint64_t      compress_body_length(compress_t *buffer);
size_t        compress_body_offset(void);
void          compress_free(compress_t *buffer);
compress_t *  compress_import(stringer_t *s);
uint64_t      compress_orig_hash(compress_t *buffer);
uint64_t      compress_orig_length(compress_t *buffer);
uint64_t      compress_total_length(compress_t *buffer);

/// engine.c
compress_t * engine_compress(uint8_t engine, stringer_t *s);
stringer_t * engine_decompress(compress_t *buffer);

/// lzo.c
bool_t lib_load_lzo(void);
compress_t * compress_lzo(stringer_t *input);
const char * lib_version_lzo(void);
stringer_t * decompress_block_lzo(stringer_t *block);
stringer_t * decompress_lzo(compress_t *compressed);

/// zlib.c
bool_t lib_load_zlib(void);
const char * lib_version_zlib(void);
compress_t * compress_zlib(stringer_t *input);
stringer_t * decompress_zlib(compress_t *compressed);

#endif

