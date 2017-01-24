
/**
 * @file /magma/core/encodings/base64.c
 *
 * @brief	Functions for base64 encoding/decoding of data.
 */

#include "magma.h"

/// TODO: Switch to always using the "_wrap" length function, which can then become base64_(en|de)coded_length().

size_t base64_encoded_length_mod(size_t length) {

	size_t result = 0, remainder = 0;

	if (length <= 0) {
		return 0;
	}

	// If the input lenght isn't divisible by 3, find out how many extra bytes we have (1 or 2).
	remainder = length % 3;

	// This is the length of the base64 encoded data
	result = (((length - remainder) / 3) * 4);

	// We could have 1 or 2 extra bytes. If so, we'll need 2 or 3 extra bytes in the output, respectively. If the input
	// was evenly divisible by 3, we'll need none.
	result = remainder ? result + remainder + 1 : result;

	// Finally return the output length, with one extra byte, in case the buffer is a nuller_t and we need a terminating NULL character.
    return result + 1;
}

size_t base64_encoded_length(size_t length) {

	size_t result = 0, remainder = 0;

	if (length <= 0) {
		return 0;
	}
	// If the input lenght isn't divisible by 3, find out how many extra bytes we have (potentially 1 or 2).
	remainder = length % 3;

	// This is the length of the base64 encoded data
	result = (((length - remainder) / 3) * 4);

	// This will figure out how many lines the output will be split into and then adds 2 extra bytes for each CR + LF sequence.
	result = result + (((result) / BASE64_LINE_WRAP_LENGTH) * 2) + 2;

	// We could have 1 or 2 extra bytes. If so, we'll need 4 extra bytes, 2 or 3 for the data, plus 2 or 1 for the padding. Since we also
	// end every base64 encoding with a CR + LF, we'll need 2 extra bytes for that, even if the remainder is 0. Note, its possible there
	// was no remainder, and the output was evenly divisible by 76 (aka the line wrap length), in which case the extra CR + LF isn't
	// appended. We don't bother taking that into account here.
	result = remainder ? result + 6 : result + 2;

	// Finally return the output length, with one additional byte for the terminating NULL character, just in case.
    return result + 1;
}

/**
 * @brief	Calculate the length of a binary buffer after being encoded with using base64.
 *
 * @note	If line wrapping is enabled, the output will assume a trailing end of line sequence.
 *
 * @param 	length	the length of the binary buffer.
 * @param 	wrap	the maximum length of each line, or 0 to disable line wrapping.
 * @param 	type	the line delimiter sequence being used.
 *
 * @return	the length of the encoded output.
 */
size_t base64_encoded_length_wrap(size_t length, size_t wrap, base64_wrap_t type) {

	size_t result = 0, remainder = 0;

	if (length <= 0) {
		return 0;
	}
#ifdef MAGMA_PEDANTIC
	else if (!wrap && type != BASE64_LINE_WRAP_NONE) {
		log_pedantic("The line wrap length is 0, indicating that line wrapping is disabled, but the end of line type was still set.");
	}
	else if (wrap && type == BASE64_LINE_WRAP_NONE) {
		log_pedantic("The line wrap length limiter is greater than 0, but the end of line type is none.");
	}
#endif

	// If the input lenght isn't divisible by 3, find out how many extra bytes we have (potentially 1 or 2).
	remainder = length % 3;

	// This is the length of the base64 encoded data
	result = (((length - remainder) / 3) * 4);

	// We could have 1 or 2 extra bytes. If so, we'll need 4 extra bytes, 2 or 3 for the data, plus 1 or 2 for the padding. If we also
	// end every base64 encoding with a new line, we'll need to add extra bytes.
	result = remainder ? result + 4 : result;

	// If the output will be wrapped, and the line feed type isn't set to none, add extra bytes for the new line characters.
	if (wrap && type) {

		// We'll add a trailing line wrap to the end, but only if the result isn't perfectly divisible by the line wrap length.
		result = result % wrap ? result + type : result;

		// Divide the output by the line wrap length, and then multiply the result by the length of the line wrap sequence.
		result = result + (((result) / wrap) * type);
	}

	// Finally return the output length.
    return result;
}

size_t base64_decoded_length_mod(size_t length) {

	size_t result = 0, remainder = 0;

	if (length <= 0) {
		return 0;
	}

	remainder = length % 4;

	// We'll end up with 3 bytes for every 4 used by the input. Note the actual number may be smaller if the base64 string was
	// split into lines, as any whitespace encountered will be ignored.
	result = ((length - remainder) / 4) * 3;

	result = result + (remainder ? remainder - 1 : 0);

    return result;
}

size_t base64_decoded_length(size_t length) {

	size_t result = 0, remainder = 0;

	if (length <= 0) {
		return 0;
	}

	remainder = length % 4;

	// We'll end up with 3 bytes for every 4 used by the input. Note the actual number may be smaller if the base64 string was
	// split into lines, as any whitespace encountered will be ignored.
	result = ((length - remainder) / 4) * 3;

	result = result + (remainder ? remainder - 1 : 0);

	// Finally return the output length, with one extra byte, in case the buffer is a nuller_t and we need a terminating NULL character.
    return result + 1;
}

/**
 * @brief	Perform base64 encoding on a managed string with padding and line splitting at BASE64_LINE_WRAP_LENGTH characters.
 * @param	s		the managed string to be base64 encoded.
 * @param	output	a managed string to receive the encoded output; if passed as NULL, one will be allocated to the caller.
 * @return	NULL on failure, or a a pointer to the managed string containing the encoded result on success.
 */
stringer_t * base64_encode(stringer_t *s, stringer_t *output) {

	uchr_t *p, *o;
	int_t c1, c2, c3;
	uint32_t opts = 0;
	stringer_t *result;
	size_t len, new_len, written = 0,  cur_line = 0;

	if (output && !st_valid_destination((opts = *((uint32_t *)output)))) {
		log_pedantic("An output string was supplied but it does not represent a buffer capable of holding the output.");
		return NULL;
	}
	else if (st_empty_out(s, &p, &len)) {
		log_pedantic("An empty string was passed in for encoding.");
		return NULL;
	}

	new_len = base64_encoded_length(len);

	// Make sure the output buffer is large enough or if output was passed in as NULL we'll attempt the allocation of our own buffer.
	if ((result = output) && ((st_valid_avail(opts) && st_avail_get(output) < new_len) ||
			(!st_valid_avail(opts) && st_length_get(output) < new_len))) {
		log_pedantic("The output buffer supplied is not large enough to hold the result. {avail = %zu / required = %zu}",
				st_valid_avail(opts) ? st_avail_get(output) : st_length_get(output), new_len);
		return NULL;
	}
	else if (!output && !(result = st_alloc(new_len))) {
		log_pedantic("Could not allocate a buffer large enough to hold encoded result. {requested = %zu}", new_len);
		return NULL;
	}

	// Lets get setup.
	o = st_data_get(result);

	// This will process three bytes at a time.
	for (size_t i = 0; i < len / 3; ++i) {

		c1 = (*p++) & 0xFF;
		c2 = (*p++) & 0xFF;
		c3 = (*p++) & 0xFF;

		*o++ = mappings.base64.characters[c1 >> 2];
		*o++ = mappings.base64.characters[((c1 << 4) | (c2 >> 4)) & 0x3F];
		*o++ = mappings.base64.characters[((c2 << 2) | (c3 >> 6)) & 0x3F];
		*o++ = mappings.base64.characters[c3 & 0x3F];

		cur_line += 4;
		written += 4;

		// If we go over the line length.
		if (cur_line >= BASE64_LINE_WRAP_LENGTH) {
			*o++ = '\r';
			*o++ = '\n';
			cur_line = 0;
			written += 2;
		}
	}

	// Encode the remaining one or two characters in the input buffer
	switch (len % 3) {

		case 2:
			c1 = (*p++) & 0xFF;
			c2 = (*p++) & 0xFF;
			*o++ = mappings.base64.characters[(c1 & 0xFC) >> 2];
			*o++ = mappings.base64.characters[((c1 & 0x03) << 4) | ((c2 & 0xF0) >> 4)];
			*o++ = mappings.base64.characters[((c2 & 0x0F) << 2)];
			*o++ = '=';
			*o++ = '\r';
			*o++ = '\n';
			written += 6;
			break;

		case 1:
			c1 = (*p++) & 0xFF;
			*o++ = mappings.base64.characters[(c1 & 0xFC) >> 2];
			*o++ = mappings.base64.characters[((c1 & 0x03) << 4)];
			*o++ = '=';
			*o++ = '=';
			*o++ = '\r';
			*o++ = '\n';
			written += 6;
			break;

		// Unless the output was evenly divisible by the line length, we need to add a line wrap at the end of our partial line.
		case 0:
			if (cur_line != 0) {
				*o++ = '\r';
				*o++ = '\n';
				written += 2;
			}
			break;

		default:
			log_error("Switch statement did not execute correctly. This should never happen.");
			break;
	}

	// If an output buffer was supplied that is capable of tracking the data length, or a managed string buffer was allocated update the length param.
	if (!output || st_valid_tracked(opts)) {
		st_length_set(result, written);
	}

	return result;
}

/**
 * @brief			Encode a binary buffer using base64, with a configurable line length, and terminating character.
 *
 * @param	s		the managed string to be base64 encoded.
 * @param 	wrap	the maximum length of each line, or 0 to disable line wrapping.
 * @param 	type	the line delimiter sequence being used.
 *
 * @param	output	a managed string to receive the encoded output; if passed as NULL, one will be allocated to the caller. * @return
 */
stringer_t * base64_encode_wrap(stringer_t *s, size_t wrap, base64_wrap_t type, stringer_t *output) {

	uchr_t *p, *o;
	int_t c1, c2, c3;
	uint32_t opts = 0;
	stringer_t *result;
	size_t len, new_len, written = 0,  cur_line = 0;

	if (output && !st_valid_destination((opts = *((uint32_t *)output)))) {
		log_pedantic("An output string was supplied but it does not represent a buffer capable of holding the output.");
		return NULL;
	}
	else if (st_empty_out(s, &p, &len)) {
		log_pedantic("An empty string was passed in for encoding.");
		return NULL;
	}

	new_len = base64_encoded_length_wrap(len, wrap, type);

	// Make sure the output buffer is large enough or if output was passed in as NULL we'll attempt the allocation of our own buffer.
	if ((result = output) && ((st_valid_avail(opts) && st_avail_get(output) < new_len) ||
			(!st_valid_avail(opts) && st_length_get(output) < new_len))) {
		log_pedantic("The output buffer supplied is not large enough to hold the result. {avail = %zu / required = %zu}",
				st_valid_avail(opts) ? st_avail_get(output) : st_length_get(output), new_len);
		return NULL;
	}
	else if (!output && !(result = st_alloc(new_len))) {
		log_pedantic("Could not allocate a buffer large enough to hold encoded result. {requested = %zu}", new_len);
		return NULL;
	}

	// Lets get setup.
	o = st_data_get(result);

	// This will process three bytes at a time.
	for (size_t i = 0; i < len / 3; ++i) {

		c1 = (*p++) & 0xff;
		c2 = (*p++) & 0xff;
		c3 = (*p++) & 0xff;

		*o++ = mappings.base64.characters[c1 >> 2];
		*o++ = mappings.base64.characters[((c1 << 4) | (c2 >> 4)) & 0x3f];
		*o++ = mappings.base64.characters[((c2 << 2) | (c3 >> 6)) & 0x3f];
		*o++ = mappings.base64.characters[c3 & 0x3f];

		cur_line += 4;
		written += 4;

		// If we go over the line length.
		if (wrap && type && cur_line >= wrap) {
			if (type == BASE64_LINE_WRAP_LF) {
				*o++ = '\n';
				written += 1;
			}
			else if (type == BASE64_LINE_WRAP_CRLF) {
				*o++ = '\r';
				*o++ = '\n';
				written += 2;
			}
			cur_line = 0;
		}
	}

	// If necessary encode the remaining (1 or 2) characters.
	if (len % 3) {
		switch (len % 3) {
			case 2:
				c1 = (*p++) & 0xff;
				c2 = (*p++) & 0xff;
				*o++ = mappings.base64.characters[(c1 & 0xfc) >> 2];
				*o++ = mappings.base64.characters[((c1 & 0x03) << 4) | ((c2 & 0xf0) >> 4)];
				*o++ = mappings.base64.characters[((c2 & 0x0f) << 2)];
				*o++ = '=';
				written += 4;
				break;
			case 1:
				c1 = (*p++) & 0xff;
				*o++ = mappings.base64.characters[(c1 & 0xfc) >> 2];
				*o++ = mappings.base64.characters[((c1 & 0x03) << 4)];
				*o++ = '=';
				*o++ = '=';
				written += 4;
				break;
			default:
				log_error("Switch statement did not execute correctly. This should never happen.");
				break;
		}

		// If line wrapping is enabled, we'll add a trailing line feed.
		if (wrap && type) {
			if (type == BASE64_LINE_WRAP_LF) {
				*o++ = '\n';
				written += 1;
			}
			else if (type == BASE64_LINE_WRAP_CRLF) {
				*o++ = '\r';
				*o++ = '\n';
				written += 2;
			}
		}
	}

	// If line wrapping is enabled, and the input is evenly divisible by 3, and the output isn't evenly divisible by the line
	// wrap length, add a terminating line wrap.
	else if (wrap && type && cur_line) {
		if (type == BASE64_LINE_WRAP_LF) {
			*o++ = '\n';
			written += 1;
		}
		else if (type == BASE64_LINE_WRAP_CRLF) {
			*o++ = '\r';
			*o++ = '\n';
			written += 2;
		}
	}

	// If an output buffer was supplied that is capable of tracking the data length, or a managed string buffer was allocated
	// update the length param.
	if (!output || st_valid_tracked(opts)) {
		st_length_set(result, written);
	}

#ifdef MAGMA_PEDANTIC
	if (written != new_len) {
		log_pedantic("The base64 encoded buffer length did not match the expected output length. { expected = %zu / actual = %zu }",
			new_len, written);
	}
#endif

	return result;
}

/**
 * @brief	Perform modified base64 encoding on a managed string without padding or line splitting.
 * @note	In this function, the '+' and '/' characters are replaced with '-' and '_' respectively,
 * 			making the output suitable for use in URL parameters without additional encoding.
 * @param	s		the managed string to be base64 modified-encoded.
 * @param	output	a managed string to receive the encoded output; if passed as NULL, an output buffer will be allocated which must be freed by the caller.
 * @return	NULL 	on failure, or a newly allocated managed string containing the modified encoded result on success.
 */
stringer_t * base64_encode_mod(stringer_t *s, stringer_t *output) {

	uchr_t *p, *o;
	stringer_t *result;
	uint32_t opts = 0;
	size_t len, new_len, written = 0;
	int_t c1, c2, c3;

	if (output && !st_valid_destination((opts = *((uint32_t *)output)))) {
		log_pedantic("An output string was supplied but it does not represent a buffer capable of holding the output.");
		return NULL;
	}
	else if (st_empty_out(s, &p, &len)) {
		log_pedantic("An empty string was passed in for encoding.");
		return NULL;
	}

	new_len = base64_encoded_length_mod(len);

	// Make sure the output buffer is large enough or if output was passed in as NULL we'll attempt the allocation of our own buffer.
	if ((result = output) && ((st_valid_avail(opts) && st_avail_get(output) < new_len) ||
			(!st_valid_avail(opts) && st_length_get(output) < new_len))) {
		log_pedantic("The output buffer supplied is not large enough to hold the result. {avail = %zu / required = %zu}",
				st_valid_avail(opts) ? st_avail_get(output) : st_length_get(output), new_len);
		return NULL;
	}
	else if (!output && !(result = st_alloc(new_len))) {
		log_pedantic("Could not allocate a buffer large enough to hold encoded result. {requested = %zu}", new_len);
		return NULL;
	}

	// Lets get setup.
	o = st_data_get(result);

	// This will process three bytes at a time.
	for (size_t i = 0; i < (len / 3); ++i) {

		c1 = (*p++) & 0xFF;
		c2 = (*p++) & 0xFF;
		c3 = (*p++) & 0xFF;

		*o++ = mappings.base64_mod.characters[c1 >> 2];
		*o++ = mappings.base64_mod.characters[((c1 << 4) | (c2 >> 4)) & 0x3F];
		*o++ = mappings.base64_mod.characters[((c2 << 2) | (c3 >> 6)) & 0x3F];
		*o++ = mappings.base64_mod.characters[c3 & 0x3F];

		written += 4;
	}

	// Encode the remaining one or two characters in the input buffer
	switch (len % 3) {

		case 1:
			c1 = (*p++) & 0xFF;
			*o++ = mappings.base64_mod.characters[(c1 & 0xFC) >> 2];
			*o++ = mappings.base64_mod.characters[((c1 & 0x03) << 4)];
			written += 2;
			break;

		case 2:
			c1 = (*p++) & 0xFF;
			c2 = (*p++) & 0xFF;
			*o++ = mappings.base64_mod.characters[(c1 & 0xFC) >> 2];
			*o++ = mappings.base64_mod.characters[((c1 & 0x03) << 4) | ((c2 & 0xF0) >> 4)];
			*o++ = mappings.base64_mod.characters[((c2 & 0x0F) << 2)];
			written += 3;
			break;
	}

	// If an output buffer was supplied that is capable of tracking the data length, or a managed string buffer was allocated update the length param.
	if (!output || st_valid_tracked(opts)) {
		st_length_set(result, written);
	}

	return result;
}

/**
 * @brief	Perform base64 encoding on a managed string with padding and line splitting at BASE64_LINE_WRAP_LENGTH characters.
 * @param	s			the managed string to be base64 encoded.
 * @param	opts		the options value of the managed string which will be allocated and receive the encoded output.
 * @param	modified	if true, use modified base64 encoding; if false, use normal base64 encoding.
 * @return	NULL on failure, or a pointer to a newly allocated managed string containing the encoded result on success.
 */
stringer_t * base64_encode_opts(stringer_t *s, uint32_t opts, bool_t modified) {

	uchr_t *p;
	size_t len, new_len;
	stringer_t *result, *b64_ret;

	if (st_empty_out(s, &p, &len)) {
		log_pedantic("An empty string was passed in for base64 encoding.");
		return NULL;
	}

	new_len = modified ? base64_encoded_length_mod(len) : base64_encoded_length(len);

	if (!(result = st_alloc_opts(opts,new_len))) {
		log_pedantic("Unable to allocate memory for base64 encoding output.");
		return NULL;
	}

	if (modified) {
		b64_ret = base64_encode_mod(s,result);
	}
	else {
		b64_ret = base64_encode(s,result);
	}

	if (!b64_ret) {
		log_pedantic("Unable to perform base64 encoding operation.");
		st_free(result);
		result = NULL;
	}

	return result;
}

/**
 * @brief	Perform base64 decoding on a managed string.
 * @param	s		the managed string to be base64 decoded.
 * @param	output	a managed string to receive the encoded output; if passed as NULL, one will be allocated to the caller.
 * @return	NULL on failure, or a newly allocated managed string containing the decoded result on success.
 */
stringer_t * base64_decode(stringer_t *s, stringer_t *output) {

	uchr_t *p, *o;
	stringer_t *result;
	size_t len, new_len, written = 0;
	uint32_t opts = 0;
	int_t loop = 0, value = 0;

	if (output && !st_valid_destination((opts = *((uint32_t *)output)))) {
		log_pedantic("An output string was supplied but it does not represent a buffer capable of holding the output.");
		return NULL;
	}
	else if (st_empty_out(s, &p, &len)) {
		log_pedantic("An empty string was passed in for encoding.");
		return NULL;
	}

	new_len = base64_decoded_length(len);

	// Make sure the output buffer is large enough or if output was passed in as NULL we'll attempt the allocation of our own buffer.
	if ((result = output) && ((st_valid_avail(opts) && st_avail_get(output) < new_len) ||
			(!st_valid_avail(opts) && st_length_get(output) < new_len))) {
		log_pedantic("The output buffer supplied is not large enough to hold the result. {avail = %zu / required = %zu}",
				st_valid_avail(opts) ? st_avail_get(output) : st_length_get(output), new_len);
		return NULL;
	}
	else if (!output && !(result = st_alloc(new_len))) {
		log_pedantic("Could not allocate a buffer large enough to hold encoded result. {requested = %zu}", new_len);
		return NULL;
	}

	// Get setup. Create a pointer to the output buffer.
	o = st_data_get(result);

	// Get four characters at a time from the input buffer and decode them.
	for (size_t i = 0; i < len; i++) {

		// Only process legit base64 characters.
		if ((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z') || (*p >= '0' && *p <= '9') || *p == '+' || *p == '/') {

			// Do the appropriate operation.
			switch (loop) {

				case 0:
					value = mappings.base64.values[(int_t)*p++] << 18;
					loop++;
					break;

				case 1:
					value += mappings.base64.values[(int_t)*p++] << 12;
					*o++ = (value & 0x00ff0000) >> 16;
					written++;
					loop++;
					break;

				case 2:
					value += (unsigned int)mappings.base64.values[(int_t)*p++] << 6;
					*o++ = (value & 0x0000ff00) >> 8;
					written++;
					loop++;
					break;

				case 3:
					value += (unsigned int)mappings.base64.values[(int_t)*p++];
					*o++ = value & 0x000000ff;
					written++;
					loop = 0;
					break;

				default:
					log_pedantic("Base64 decoder logic failure. Unexpected loop state. {loop = %i}", loop);
					loop = 0;
					break;
			}
		}
		else if (*p == '=') {
			i = len;
		}
		else {
			p++;
		}

	}

	// If an output buffer was supplied that is capable of tracking the data length, or a managed string buffer was allocated update the length param.
	if (!output || st_valid_tracked(opts)) {
		st_length_set(result, written);
	}

	return result;
}

/**
 * @brief	Perform modified base64 decoding on a managed string. without padding or line splitting.
 * @note	In this function, the '+' and '/' characters are replaced with '-' and '_' respectively,
 * 			making the output suitable for use in URL parameters without additional encoding.
 * @param	s		the managed string to be base64 decoded.
 * @param	output	a managed string to receive the encoded output; if passed as NULL, one will be allocated to the caller.
 * @return	NULL on failure, or a newly allocated managed string containing the modified decoded result on success.
 */
stringer_t * base64_decode_mod(stringer_t *s, stringer_t *output) {

	uchr_t *p, *o;
	uint32_t opts = 0;
	stringer_t *result;
	int_t loop = 0, value = 0;
	size_t len, new_len, written = 0;

	if (output && !st_valid_destination((opts = *((uint32_t *)output)))) {
		log_pedantic("An output string was supplied but it does not represent a buffer capable of holding the output.");
		return NULL;
	}
	else if (st_empty_out(s, &p, &len)) {
		log_pedantic("An empty string was passed in for encoding.");
		return NULL;
	}

	new_len = base64_decoded_length_mod(len);

	// Make sure the output buffer is large enough or if output was passed in as NULL we'll attempt the allocation of our own buffer.
	if ((result = output) && ((st_valid_avail(opts) && st_avail_get(output) < new_len) ||
			(!st_valid_avail(opts) && st_length_get(output) < new_len))) {

		log_pedantic("The output buffer supplied is not large enough to hold the result. {avail = %zu / required = %zu}",
				st_valid_avail(opts) ? st_avail_get(output) : st_length_get(output), new_len);
		return NULL;

	}
	else if (!output && !(result = st_alloc(new_len))) {
		log_pedantic("Could not allocate a buffer large enough to hold encoded result. {requested = %zu}", new_len);
		return NULL;
	}

	// Get setup. Create a pointer to the output buffer.
	o = st_data_get(result);

	// Get four characters at a time from the input buffer and decode them.
	for (size_t i = 0; i < len; i++) {

		// Only process legit base64 characters.
		if ((*p >= 'A' && *p <= 'Z') || (*p >= 'a' && *p <= 'z') || (*p >= '0' && *p <= '9') || *p == '-' || *p == '_') {

			// Do the appropriate operation.
			switch (loop) {

				case 0:
					value = mappings.base64_mod.values[(int_t)*p++] << 18;
					loop++;
					break;

				case 1:
					value += mappings.base64_mod.values[(int_t)*p++] << 12;
					*o++ = (value & 0x00ff0000) >> 16;
					written++;
					loop++;
					break;

				case 2:
					value += (unsigned int)mappings.base64_mod.values[(int_t)*p++] << 6;
					*o++ = (value & 0x0000ff00) >> 8;
					written++;
					loop++;
					break;

				case 3:
					value += (unsigned int)mappings.base64_mod.values[(int_t)*p++];
					*o++ = value & 0x000000ff;
					written++;
					loop = 0;
					break;

				default:
					log_pedantic("Base64 decoder logic failure. Unexpected loop state. {loop = %i}", loop);
					loop = 0;
					break;
			}
		}
		else {
			p++;
		}

	}

	// If an output buffer was supplied that is capable of tracking the data length, or a managed string buffer was allocated update the length param.
	if (!output || st_valid_tracked(opts)) {
		st_length_set(result, written);
	}

	return result;
}

/**
 * @brief	Perform base64 decoding on a managed string.
 * @param	s			the managed string to be base64 decoded.
 * @param	opts		the options value of the managed string which will be allocated and receive the decoded output.
 * @param	modified	if true, use modified base64 encoding; if false, use normal base64 decoding.
 * @return	NULL on failure, or a pointer to a newly allocated managed string containing the decoded result on success.
 */
stringer_t * base64_decode_opts(stringer_t *s, uint32_t opts, bool_t modified) {

	uchr_t *p;
	size_t len, new_len;
	stringer_t *result, *b64_ret;

	if (st_empty_out(s, &p, &len)) {
		log_pedantic("An empty string was passed in for base64 decoding.");
		return NULL;
	}

	new_len = modified ? base64_decoded_length_mod(len) : base64_decoded_length(len);

	if (!(result = st_alloc_opts(opts,new_len))) {
		log_pedantic("Unable to allocate memory for base64 decoding output.");
		return NULL;
	}

	if (modified) {
		b64_ret = base64_decode_mod(s,result);
	}
	else {
		b64_ret = base64_decode(s,result);
	}

	if (!b64_ret) {
		log_pedantic("Unable to perform base64 decoding operation.");
		st_free(result);
		result = NULL;
	}

	return result;
}
