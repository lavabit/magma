
/**
 * @file /magma/core/encodings/qp.c
 *
 * @brief	Functions for encoding/decoding quoted printable data, as described by RFC 2045, section 6.7.
 * @note	This function operates on standard 8-bit characters, transforming non-printable characters into printable ones.
 * 			It is used as a MIME content encoding, and wraps lines at 76 characters.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Perform QP (quoted-printable) encoding of a string.
 * @param	s	a pointer to a managed string containing data to be encoded.
 * @return	a pointer to a managed string containing the QP encoded data, or NULL on failure.
 */
stringer_t * qp_encode(stringer_t *s) {

	chr_t hex[4];
	uchr_t *p, *o;
	stringer_t *output, *r;
	size_t len, expected = 0, line = 0;

	if (st_empty_out(s, &p, &len)) {
		log_pedantic("An empty string was passed in for encoding.");
		return NULL;
	}

	// Increment through the stringer and count the characters that need to be encoded.
	for (size_t i = 0; i < len; i++) {

		if (*p < '!' || *p > '~' || *p == '=' || *p == ' ' || *p == '\r' || *p == '\n' || *p == '\t') {
			expected += 3;
		}
		else {
			expected++;
		}

		p++;
	}

	// Include room for the soft line break sequence every seventy six characters.
	expected += ((expected + QP_LINE_WRAP_LENGTH) / QP_LINE_WRAP_LENGTH) * 3;

	// Allocate one byte for printable characters and three bytes for non-printable characters.
	if (!(output = st_alloc_opts(MANAGED_T | JOINTED | HEAP, expected))) {
		log_pedantic("Could not allocate a buffer large enough to hold encoded result. {requested = %zu}", expected);
		return NULL;
	}

	// Get setup.
	p = st_data_get(s);
	o = st_data_get(output);

	// Increment through the stringer and copy the data into the new stringer.
	for (size_t i = 0; i < len; i++) {

		// Escape the characters matching this boolean while simply copying any other characters we encounter.
		if (*p < '!' || *p > '~' || *p == '=' || *p == ' ' || *p == '\r' || *p == '\n' || *p == '\t') {

			// If were within three characters of the limit append a soft line break to the buffer.
			if (line > (QP_LINE_WRAP_LENGTH - 3) && snprintf(hex, 4, "=\r\n") == 3 && (r = st_append(output, PLACER(&hex[0], 3)))) {
				output = r;
				line = 0;
			}

			if (snprintf(hex, 4, "=%02X", *p) == 3 && (r = st_append(output, PLACER(&hex[0], 3)))) {
				output = r;
				line += 3;
			}
		}
		else {

			// If were near the line length limit this will append a soft line break before appending the next character.
			if (line > (QP_LINE_WRAP_LENGTH - 1) && snprintf(hex, 4, "=\r\n") == 3 && (r = st_append(output, PLACER(&hex[0], 3)))) {
				output = r;
				line = 0;
			}

			if ((r = st_append(output, PLACER(p, 1)))) {
				output = r;
				line++;
			}

		}

		// We always advance the input pointer.
		p++;
	}

	return output;
}

/**
 * @brief	Perform QP (quoted-printable) decoding of a string.
 * @param	s	the managed string containing data to be decoded.
 * @return	a pointer to a managed string containing the 8-bit decoded output, or NULL on failure.
 */
stringer_t * qp_decode(stringer_t *s) {

	uchr_t *p, *o;
	stringer_t *output;
	size_t len, written = 0;

	if (st_empty_out(s, &p, &len)) {
		log_pedantic("An empty string was passed in for decoding.");
		return NULL;
	}

	// Allocate one byte for printable characters and three bytes for non-printable characters.
	if (!(output = st_alloc(len))) {
		log_pedantic("Could not allocate a buffer large enough to hold decoded result. {requested = %zu}", len);
		return NULL;
	}

	// Get setup.
	o = st_data_get(output);

#ifdef MAGMA_PEDANTIC
	// In pedantic mode we perform an extra check to make sure the loop doesn't loop past zero.
	while (len && len <= st_length_get(s)) {
#else
	while (len) {
#endif

		// Advance past the trigger.
		if (*p == '=') {

			len--;
			p++;

			// Valid hex pair.
			if (len >= 2 && hex_valid_chr(*p) && hex_valid_chr(*(p + 1))) {
				*o++ = hex_decode_chr(*p, *(p + 1));
				written++;
				len -= 2;
				p += 2;
			}
			// Soft line breaks are signaled by a line break following an equal sign.
			else if (len >= 2 && *p == '\r' && *(p + 1) == '\n') {
				len -= 2;
				p += 2;
			}
			else if (len >= 1 && *p == '\n') {
				len--;
				p++;
			}
			// Equal signs which aren't followed by a valid hex pair or a line break are illegal, but if the character is printable
			// we can let through the original sequence.
			else if (len >= 1 && ((*p >= '!' && *p <= '<') || (*p >= '>' && *p <= '~'))) {
				*o++ = '=';
				*o++ = *p++;
				written += 2;
				len--;
			}
			// Characters outside the printable range are simply skipped.
			else if (len >= 1) {
				len--;
				p++;
			}
		}
		// Let through any characters found inside this range.
		else if ((*p >= '!' && *p <= '<') || (*p >= '>' && *p <= '~')) {
			*o++ = *p++;
			written++;
			len--;
		}
		// Characters outside the range above should have been encoded. Any that weren't should be skipped.
		else {
			len--;
			p++;
		}

	}

	// We allocated a default string buffer, which means the length is tracked so we need to set the data length.
	st_length_set(output, written);
	return output;
}
