
/**
 * @file /magma/core/encodings/zbase32.c
 *
 * @brief	A modified base32 encoding routine (zbase32) which selects characters to enhance readability.
 * 			zbase32 strings may be used in URLs without any further encoding.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Encode data as a zbase32 string.
 * @param	s	a managed string containing the data to be encoded.
 * @return	NULL on failure, or a freshly allocated managed string containing the zbase32-encoded data on success.
 */
stringer_t * zbase32_encode(stringer_t *s) {

	uchr_t *p, *o;
	stringer_t *output;
	size_t len, written = 0;
	uint32_t v = 0, bits = 0;

	// Check the input and setup the output buffer.
	if (st_empty_out(s, &p, &len)) {
		log_pedantic("An empty string was passed in for encoding.");
		return NULL;
	}

	if (!(output = st_alloc(((len * 8) + 4) / 5))) {
		log_pedantic("Could not allocate a buffer large enough to hold encoded result. {requested = %zu}", ((len * 8) + 4) / 5);
		return NULL;
	}

	// Output cursor.
	o = st_data_get(output);

	for (size_t i = 0; i < len; i++) {
		v = v | (*p++ << bits);
		bits += 8;

		while (bits >= 5) {
			*o++ = mappings.zbase32.characters[v & 31];
			bits -= 5;
			v = v >> 5;
			written++;
		}
	}

	if (bits) {
		*o++ = mappings.zbase32.characters[v];
		written++;
	}

	st_length_set(output, written);
	return output;
}

/**
 * @brief	Decode a zbase32 string.
 * @param	s	a managed string containing the data to be decoded.
 * @return	NULL on failure, or a freshly allocated managed string containing the zbase32-decoded data on success.
 */

stringer_t * zbase32_decode(stringer_t *s) {

	uchr_t *p, *o;
	stringer_t *output;
	size_t len, written = 0;
	uint32_t b, v = 0, bits = 0;

	// Check the input and setup the output buffer.
	if (st_empty_out(s, &p, &len)) {
		log_pedantic("An empty string was passed in for decoding.");
		return NULL;
	}

	if (!(output = st_alloc((len * 5) / 8))) {
		log_pedantic("Could not allocate a buffer large enough to hold decoded result. {requested = %zu}", (len * 5) / 8);
		return NULL;
	}

	// Output cursor.
	o = st_data_get(output);

	for (size_t i = 0; i < len; i++) {

		// If an invalid character is encountered, clean up and return NULL.
		// The first expression matches anything over 127; this is critical because the value array only has 128 entries.
		// The value array returns 255 for positions that are not associated with valid zbase32 characters.
		if ((p[i] & 0x80) || (b = mappings.zbase32.values[p[i]]) > 31) {
			st_free(output);
			return NULL;
		}

		v = v | (b << bits);
		bits += 5;

		if (bits >= 8) {
			*o++ = v;
			bits -= 8;
			v = v >> 8;
			written++;
		}

	}

	st_length_set(output, written);
	return output;
}
