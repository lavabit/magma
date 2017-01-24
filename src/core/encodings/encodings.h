
/**
 * @file /magma/core/encodings/encodings.h
 *
 * @brief	Functions used to encode and decode data in various formats.
 */

#ifndef MAGMA_CORE_ENCODINGS_H
#define MAGMA_CORE_ENCODINGS_H

/**
 * @note When considering a change to the URL length limit, the following statistics may be useful:
 *
 *	Internet Explorer: 2,048 characters for the host/path and 2,083 characters overall
 *	Firefox: limited to 65,536 visible characters, but longer URL will still work
 *	Safari: at least 80,000 characters
 *	Opera: at least 190,000 characters
 *	IIS: by default the limit is 16,384 but can be increased
 *	Apache: by default 4,000 characters
 */

#define URL_MAX_LENGTH 				1048576
#define QP_LINE_WRAP_LENGTH			76
#define BASE64_LINE_WRAP_LENGTH		76

typedef enum {
	BASE64_LINE_WRAP_NONE = 0,
	BASE64_LINE_WRAP_LF = 1,
	BASE64_LINE_WRAP_CRLF = 2
} base64_wrap_t;

typedef struct {
	struct {
		chr_t characters[32], values[128];
	} zbase32;
	struct {
		chr_t characters[64], values[128];
	} base64;
	struct {
		chr_t characters[64], values[128];
	} base64_mod;
} mappings_t;

extern mappings_t mappings;

/// base64.c
stringer_t *  base64_decode(stringer_t *s, stringer_t *output);
stringer_t *  base64_decode_mod(stringer_t *s, stringer_t *output);
stringer_t *  base64_decode_opts(stringer_t *s, uint32_t opts, bool_t modified);
size_t        base64_decoded_length(size_t length);
size_t        base64_decoded_length_mod(size_t length);
stringer_t *  base64_encode(stringer_t *s, stringer_t *output);
stringer_t *  base64_encode_mod(stringer_t *s, stringer_t *output);
stringer_t *  base64_encode_opts(stringer_t *s, uint32_t opts, bool_t modified);
stringer_t *  base64_encode_wrap(stringer_t *s, size_t wrap, base64_wrap_t type, stringer_t *output);
size_t        base64_encoded_length(size_t length);
size_t        base64_encoded_length_mod(size_t length);
size_t        base64_encoded_length_wrap(size_t length, size_t wrap, base64_wrap_t type);

/// hex.c
bool_t hex_valid_chr(uchr_t c);
byte_t hex_decode_chr(uchr_t a, uchr_t b);
size_t hex_count_st(stringer_t *s);
size_t hex_valid_st(stringer_t *s);
stringer_t * hex_decode_st(stringer_t *h, stringer_t *output);
stringer_t * hex_encode_st(stringer_t *b, stringer_t *output);
uchr_t * hex_encode_chr(byte_t b, uchr_t *output);
stringer_t * hex_encode_st_debug(stringer_t *input, size_t maxlen);
stringer_t * hex_encode_opts(stringer_t *input, uint32_t opts);
stringer_t * hex_decode_opts(stringer_t *input, uint32_t opts);

/// qp.c
stringer_t * qp_decode(stringer_t *s);
stringer_t * qp_encode(stringer_t *s);

/// url.c
bool_t url_valid_chr(uchr_t c);
size_t url_valid_st(stringer_t *s);
stringer_t * url_decode(stringer_t *s);
stringer_t * url_encode(stringer_t *s);

/// zbase32.c
stringer_t * zbase32_decode(stringer_t *s);
stringer_t * zbase32_encode(stringer_t *s);

#endif

