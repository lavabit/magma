
/**
 * @file /magma/core/encodings/url.c
 *
 * @brief	Functions used to encode and decode website URLs.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Determine whether a given character is a valid character in a URL.
 * @param	c	the character to be examined.
 * @return	true if the character is valid in a URL or false if it must be escaped.
 */
bool_t url_valid_chr(uchr_t c) {
	if ((c >= 'A' && c <= 'Z') ||	(c >= 'a' && c <= 'z') ||	(c >= '0' && c <= '9') || c == '-' || c == '.' || c == '_' || c == '~') {
		return true;
	}
	return false;
}

/**
 * @brief	Check a URL string for validity.
 * @note	This function confirms that the URL consists of only legal characters and that all escaped sequences are also valid.
 * @param	s 	a managed string containing the URL to be verified.
 * @return	0 on failure, or the number of valid characters in the URL if the string is valid.
 */
size_t url_valid_st(stringer_t *s) {

	uchr_t *p;
	size_t c = 0, len;
	bool_t result = true;

	if (st_empty_out(s, &p, &len)) {
		return 0;
	}

	// Iterates through and counts valid characters.
	while (result && len) {
		if (url_valid_chr(*p)) {
			len--;
			c++;
			p++;
		}
		// If an invalid character is found check whether its a properly formed escape sequence.
		else if (len >= 3 && *p == '%' && hex_valid_chr(*(p + 1)) && hex_valid_chr(*(p + 2))) {
			len -= 3;
			p += 3;
			c++;
		}
		else {
			result = false;
			len = 0;
		}
	}

	return c;
}

/**
 * @brief	Encode a data buffer as a valid URL component.
 * @param	s 	a managed string containing the data to be encoded.
 * @return	NULL on failure, or a freshly allocated managed string containing the fully-escaped string suitable for use in a URL.
 */
stringer_t * url_encode(stringer_t *s) {

	chr_t hex[4];
	uchr_t *p, *o;
	stringer_t *output, *r;
	size_t len, expected = 0, written = 0;

	if (st_empty_out(s, &p, &len)) {
		log_pedantic("An empty string was passed in for encoding.");
		return NULL;
	}

	// Increment through the stringer and count the characters that need to be encoded.
	for (size_t i = 0; i < len; i++) {
		if (url_valid_chr(*p)) {
			expected++;
		}
		else {
			expected += 3;
		}

		p++;
	}

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

		// Escape the invalid characters.
		if (url_valid_chr(*p)) {
			if ((r = st_append(output, PLACER(p, 1)))) {
				output = r;
				written++;
			}
		}
		else if (snprintf(hex, 4, "%%%02X", *p) == 3 && (r = st_append(output, PLACER(&hex[0], 3)))) {
			output = r;
			written += 3;
		}

		// We always advance the input pointer.
		p++;
	}

	st_length_set(output, written);
	return output;
}

/**
 * @brief	Decode a URL-encoded string into its original representation.
 * @param	s 	a managed string containing the UR componentL to be decoded.
 * @return	NULL on failure, or a freshly allocated managed string containing the original data represented by the URL-encoded input on success.
 */
stringer_t * url_decode(stringer_t *s) {

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
		if (*p == '%') {

			len--;
			p++;

			// Valid hex pair.
			if (len >= 2 && hex_valid_chr(*p) && hex_valid_chr(*(p + 1))) {
				*o++ = hex_decode_chr(*p, *(p + 1));
				written++;
				len -= 2;
				p += 2;
			}
			// Percent signs that aren't followed by a valid hex pair are invalid, but in the interest of compatibility we'll simply let
			// those characters through.
			else if (len >= 1){
				*o++ = '%';
				*o++ = *p++;
				written += 2;
				len--;
			}
		}
		// Characters not prefixed by a percent sign are simply copied into the output buffer.
		else {
			*o++ = *p++;
			written++;
			len--;
		}

	}

	// We allocated a default string buffer, which means the length is tracked so we need to set the data length.
	st_length_set(output, written);
	return output;
}
