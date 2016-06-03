
/**
 * @file /magma/core/encodings/hex.c
 *
 * @brief	Functions for encoding/decoding hexadecimal data.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Determine whether a character is a valid hexadecimal (base 16) character.
 * @param	c	the character to be tested.
 * @return	true if the character is a valid hexadecimal character; false otherwise.
 */
bool_t hex_valid_chr(uchr_t c) {
	if ((c >= '0' && c <= '9') || (c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f')) {
		return true;
	}
	return false;
}

/**
 * @brief	Determine whether a managed string is a properly formatted hex string and return the number found.
 * @note	A valid hex string consists of only hexadecimal characters and whitespace, and the number of hex characters is divisible by two.
 * @param	s	the managed string to be tested.
 * @return	0 on failure, or the number of hexadecimal characters found in the managed string.
 */
size_t hex_valid_st(stringer_t *s) {

	uchr_t *p;
	size_t c = 0, len;
	bool_t result = true;

	if (st_empty_out(s, &p, &len)) {
		return 0;
	}

	// Iterates through and counts valid characters. If an invalid character is found the loop is broken.
	for (size_t i = 0; result && i < len; i++) {
		if (hex_valid_chr(*p)) c++;
		else if (*p != ' ' && *p != '\t' && *p != '\r' && *p != '\n') result = false;
		p++;
	}

	// This check ensures the number of valid characters is at least two and evenly divisible by two.
	if (!result || (c % 2)) {
		c = 0;
	}

	return c;
}

/**
 * @brief	Count the number of hex characters in a string.
 * @param	s	the managed string to be scanned.
 * @return	the number of valid hexadecimal characters found in the string.
 */
size_t hex_count_st(stringer_t *s) {

	uchr_t *p;
	size_t c = 0, len;

	if (st_empty_out(s, &p, &len)) {
		return 0;
	}

	// Iterates through and counts valid characters. If an invalid character is found the loop is broken.
	for (size_t i = 0; i < len; i++) {
		if (hex_valid_chr(*p++)) c++;
	}

	return c;
}

// QUESTION: Wait, what does this do???
/**
 * Converts a binary octet into a pair of lowercase hex characters. The result is returned inside a thread specific static character buffer.
 * If a valid pointer is also supplied using the output variable then the result will also be written to the memory the pointer indicates.
 *
 * @param b The binary octet being encoded.
 * @param output A pointer to the memory where the result should be stored or NULL if the result should only be returned.
 * @return Returns the two character hex representation of the binary input using a thread localized character buffer.
 */
uchr_t * hex_encode_chr(byte_t b, uchr_t *output) {

	int_t number;
	uchr_t nibble[3];

	// Reset the buffer.
	mm_wipe(nibble, sizeof(nibble));

	// The first character.
	if ((number = b / 16) < 10) {
		nibble[0] = (uchr_t)('0' + number);
	}
	else {
		nibble[0] = (uchr_t)('a' + (number - 10));
	}

	// The second character..
	if ((number = b % 16) < 10) {
		nibble[1] = (uchr_t)('0' + number);
	}
	else {
		nibble[1]  = (uchr_t)('a' + (number - 10));
	}

	// See if we should write the result to the output pointer.
	if (output) {
		*output = nibble[0];
		*(output + 1) = nibble[1];
	}

	return NULL;//&nibble[0];
}

/**
 * @brief	Convert a block of binary data into a hex string.
 * @param	b		a managed string containing the raw data to be encoded.
 * @param	output	if not NULL, a pointer to a managed string that will store the encoded output; if NULL, a new managed string will
 * 					be allocated and returned to the caller.
 * @return 	NULL on failure, or a pointer to a managed string containing the hex-encoded output on success.
 */
stringer_t * hex_encode_st(stringer_t *b, stringer_t *output) {

	size_t len = 0;
	uint32_t opts = 0;
	uchr_t *p = NULL, *o;
	stringer_t *result = NULL;

	if (output && !st_valid_destination((opts = *((uint32_t *)output)))) {
		log_pedantic("An output string was supplied but it does not represent a buffer capable of holding the output.");
		return NULL;
	}
	else if (st_empty_out(b, &p, &len)) {
		log_pedantic("The input block does not appear to hold any data ready for encoding. {%slen = %zu}", p ? "" : "p = NULL / ", len);
		return NULL;
	}

	// Make sure the output buffer is large enough or if output was passed in as NULL we'll attempt the allocation of our own buffer.
	if ((result = output) && ((st_valid_avail(opts) && st_avail_get(output) < (len * 2)) ||
			(!st_valid_avail(opts) && st_length_get(output) < (len * 2)))) {
		log_pedantic("The output buffer supplied is not large enough to hold the result. {avail = %zu / required = %zu}",
				st_valid_avail(opts) ? st_avail_get(output) : st_length_get(output), len * 2);
		return NULL;
	}
	else if (!output && !(result = st_alloc(len * 2))) {
		log_pedantic("The output buffer memory allocation request failed. {requested = %zu}", len * 2);
		return NULL;
	}

	// Store the memory address where the output should be written.
	o = st_data_get(result);

	// Loop through the input buffer and write character pairs to the result string data buffer.
	for (size_t i = 0; i < len; i++) {
		hex_encode_chr(*p, o);
		o += 2;
		p += 1;
	}

	// If an output buffer was supplied that is capable of tracking the data length, or a managed string buffer was allocated update the length param.
	if (!output || st_valid_tracked(opts)) {
		st_length_set(result, len * 2);
	}

	return result;
}

/**
 * @brief	Encode data in a human readable way, for debugging purposes.
 * @note	All printable ASCII data will be displayed as-is; all other characters will be shown as formatted hex pairs.
 * @param	input	a pointer to a managed string containing the data to be formatted.
 * @param	maxlen	the maximum number of bytes to be displayed. If more characters are available, an ellipsis will be shown in the middle of the string
 * 			to represent the abridged data, with only the leading and trailing characters returned as part of the display string.
 * @return	NULL on failure, or a pointer to a newly allocated managed string containing the encoded debug data on success.
 */
stringer_t * hex_encode_st_debug(stringer_t *input, size_t maxlen) {

	stringer_t *result;
	chr_t *iptr, *rptr, *rstart;
	size_t total, pass_len;

	// Not the most efficient, but this is mostly for internal debugging purposes anyhow
	total = (maxlen * 4) + 16;

	if (!(result = st_alloc (total))) {
		log_error("Unable to allocate space for debug string.");
		return NULL;
	}

	rstart = rptr = st_char_get(result);
	iptr = st_char_get(input);
	*rptr++ = '[';

	if (maxlen >= st_length_get(input)) {
		pass_len = st_length_get(input);
	} else {
		pass_len = maxlen / 2;
	}

	// First pass happens regardless.
	for (int i = 0; i < pass_len; i++) {

		if (chr_printable(*iptr) || (*iptr == '\r') || (*iptr == '\n') || (*iptr == '\t')) {
			*rptr++ = *iptr++;
		} else {
			*rptr++ = '\\';
			*rptr++ = 'x';
			hex_encode_chr(*iptr++, (uchr_t *)rptr);
			rptr += 2;
		}

	}

	// Second pass only if there's another round.
	if (pass_len != st_length_get(input)) {
		*rptr++ = ' ';
		*rptr++ = '.';
		*rptr++ = '.';
		*rptr++ = '.';
		*rptr++ = ' ';
		iptr = st_char_get(input) + st_length_get(input) - (maxlen / 2);

		for (int i = 0; i < pass_len; i++) {

			if (chr_printable(*iptr) || (*iptr == '\r') || (*iptr == '\n') || (*iptr == '\t')) {
				*rptr++ = *iptr++;
			} else {
				*rptr++ = '\\';
				*rptr++ = 'x';
				hex_encode_chr(*iptr++, (uchr_t *)rptr);
				rptr += 2;
			}

		}
	}

	*rptr++ = ']';
	*rptr = 0;

	st_length_set(result, (rptr-rstart));
	return result;
}

/**
 * @brief	Decode a hex character pair as a single byte (usually for URL decoding).
 * @param	a	the higher order 4-bit hexadecimal character of the byte to be encoded.
 * @param	b	the lower order 4-bit hexadecimal character of the byte to be decoded.
 * @return	a byte containing the value of the decoded hexadecimal character pair, or 0 on failure.
 */
byte_t hex_decode_chr(uchr_t a, uchr_t b) {

	byte_t result = 0;

#ifdef MAGMA_PEDANTIC
	if (!hex_valid_chr(a) || !hex_valid_chr(b)) {
		log_pedantic("Invalid hex characters passed in for decoding. {a = %c / b = %c}", lower_chr(a), lower_chr(b));
	}
#endif

	// Only process valid hex characters.
	if (hex_valid_chr(a)) {
		if (a >= '0' && a <= '9') result += (16 * (a - '0'));
		else if (a >= 'A' && a <= 'F') result += (16 * ((a - 'A') + 10));
		else if (a >= 'a' && a <= 'f') result += (16 * ((a - 'a') + 10));
	}

	if (hex_valid_chr(b)) {
		if (b >= '0' && b <= '9') result += (b - '0');
		else if (b >= 'A' && b <= 'F') result += ((b - 'A') + 10);
		else if (b >= 'a' && b <= 'f') result += ((b - 'a') + 10);
	}

	return result;
}

/**
 * @brief	Convert a hex string into a binary data blob.
 * @note	All hex strings should be composed of pairs of two hex characters representing individual bytes.
 * 			Invalid hex characters will simply be ignored during processing.
 * @param	h		a managed string containing the input hex string to be decoded.
 * @param	output	if not NULL, a pointer to a managed string to contain the decoded binary output; if NULL, a new string
 * 					will be allocated and returned to the caller.
 * @return	a pointer to a managed string containing the decoded output, or NULL on failure.
 */
stringer_t * hex_decode_st(stringer_t *h, stringer_t *output) {

	uint32_t opts = 0;
	uchr_t *p = NULL, *o, c = 0;
	size_t w = 0, len = 0, valid;
	stringer_t *result = NULL;

	if (output && !st_valid_destination((opts = *((uint32_t *)output)))) {
		log_pedantic("An output string was supplied but it does not represent a buffer capable of holding the output.");
		return NULL;
	}
	else if (st_empty_out(h, &p, &len) || !(valid = hex_count_st(h))) {
		log_pedantic("The input block does not appear to hold any data ready for decoding. {%slen = %zu}", p ? "" : "p = NULL / ", len);
		return NULL;
	}

	// Make sure the output buffer is large enough or if output was passed in as NULL we'll attempt the allocation of our own buffer.
	if ((result = output) && ((st_valid_avail(opts) && st_avail_get(output) < (valid / 2)) ||
			(!st_valid_avail(opts) && st_length_get(output) < (valid / 2)))) {
		log_pedantic("The output buffer supplied is not large enough to hold the result. {avail = %zu / required = %zu}",
				st_valid_avail(opts) ? st_avail_get(output) : st_length_get(output), valid / 2);
		return NULL;
	}
	else if (!output && !(result = st_alloc(valid / 2))) {
		log_pedantic("The output buffer memory allocation request failed. {requested = %zu}", (valid / 2));
		return NULL;
	}

	// Store the memory address where the output should be written.
	o = st_data_get(result);

	// Loop through the input buffer and translate valid characters into a binary octet.
	for (size_t i = 0; i < len; i++) {
		if (hex_valid_chr(*p)) {
			if (!c) {
				c = *p;
			}
			else {
				*o++ = hex_decode_chr(c, *p);
				c = 0;
				w++;
			}
		}
		p++;
	}

	// If an output buffer was supplied that is capable of tracking the data length, or a managed string buffer was allocated update the length param.
	if (!output || st_valid_tracked(opts)) {
		st_length_set(result, w);
	}

	return result;
}

/**
 * @brief	Allocates an output string of appropriate size with specified opts for hex encoding of input
 * @param	input	Input stringer to be encoded.
 * @return	NULL on failure, otherwise allocated stringer with encoded data.
 */
stringer_t * hex_encode_opts(stringer_t *input, uint32_t opts) {

	stringer_t *result = NULL;

	if(st_empty(input)) {
		log_pedantic("Empty stringer was passed in.");
		goto error;
	}

	if(!opts) {
		log_pedantic("Invalid stringer options were passed in.");
		goto error;
	}

	if(!(result = st_alloc_opts(opts, st_length_get(input) * 2))) {
		log_error("Failed to allocate memory for hex-encoded output.");
		goto error;
	}

	if(result != hex_encode_st(input, result)) {
		log_error("Failed to encode data.");
		goto cleanup_result;
	}

	return result;

cleanup_result:
	st_free(result);
error:
	return NULL;
}

/**
 * @brief	Allocates an output string of appropriate size with specified opts for hex decoding of input
 * @param	input	Input stringer to be decoded.
 * @return	NULL on failure, otherwise allocated stringer with decoded data.
 */
stringer_t * hex_decode_opts(stringer_t *input, uint32_t opts) {

	stringer_t *result = NULL;
	size_t insize;

	if(st_empty(input)) {
		log_pedantic("Empty stringer was passed in.");
	}

	if(!opts) {
		log_pedantic("Invalid stringer options were passed in.");
		goto error;
	}

	insize = st_length_get(input);

	if(!(result = st_alloc_opts(opts, (insize % 2) ? ((insize + 1) / 2) : (insize / 2) ))) {
		log_error("Failed to allocate memory for hex-encoded output.");
		goto error;
	}

	if(result != hex_decode_st(input, result)) {
		log_error("Failed to encode data.");
		goto cleanup_result;
	}

	return result;

cleanup_result:
	st_free(result);
error:
	return NULL;
}
