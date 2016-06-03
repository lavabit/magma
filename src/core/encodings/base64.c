
/**
 * @file /magma/core/encodings/base64.c
 *
 * @brief	Functions for base64 encoding/decoding of data.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

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

	new_len = modified ? BASE64_ENCODED_MOD_LEN(len) : BASE64_ENCODED_LEN(len);

	if (!(result = st_alloc_opts(opts,new_len))) {
		log_pedantic("Unable to allocate memory for base64 encoding output.");
		return NULL;
	}

	if (modified) {
		b64_ret = base64_encode_mod(s,result);
	} else {
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
 * @brief	Perform base64 encoding on a managed string with padding and line splitting at BASE64_LINE_WRAP_LENGTH characters.
 * @param	s		the managed string to be base64 encoded.
 * @param	output	a managed string to receive the encoded output; if passed as NULL, one will be allocated to the caller.
 * @return	NULL on failure, or a a pointer to the managed string containing the encoded result on success.
 */
stringer_t * base64_encode(stringer_t *s, stringer_t *output) {

	uchr_t *p, *o;
	size_t len, new_len, written = 0;
	uint32_t opts = 0;
	int_t c1, c2, c3, cur_line = 0;
	stringer_t *result;

	if (output && !st_valid_destination((opts = *((uint32_t *)output)))) {
		log_pedantic("An output string was supplied but it does not represent a buffer capable of holding the output.");
		return NULL;
	}
	else if (st_empty_out(s, &p, &len)) {
		log_pedantic("An empty string was passed in for encoding.");
		return NULL;
	}

	new_len = BASE64_ENCODED_LEN(len);

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
		if (cur_line > BASE64_LINE_WRAP_LENGTH) {
			*o++ = '\r';
			*o++ = '\n';
			cur_line = 0;
			written += 2;
		}
	}

	// Encode the remaining one or two characters in the input buffer
	switch (len % 3) {

	case 0:
		*o++ = '\r';
		*o++ = '\n';
		written += 2;
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

	default:
		log_error("Switch statement did not execute correctly. This should never happen.");
		break;
	}

	// If an output buffer was supplied that is capable of tracking the data length, or a managed string buffer was allocated update the length param.
	if (!output || st_valid_tracked(opts)) {
		st_length_set(result, new_len);
	}

	return result;
}

/**
 * @brief	Perform modified base64 encoding on a managed string without padding or line splitting.
 * @note	In this function, the '+' and '/' characters are replaced with '-' and '_' respectively,
 * 			making the output suitable for use in URL parameters without additional encoding.
 * @param	s		the managed string to be base64 modified-encoded.
 * @param	output	a managed string to receive the encoded output; if passed as NULL, one will be allocated to the caller.
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
		debug_hook();
		return NULL;
	}

	new_len = BASE64_ENCODED_MOD_LEN(len);

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
		st_length_set(result, new_len);
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

	new_len = BASE64_DECODED_LEN(len);

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

	// Get setup.
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

	new_len = modified ? BASE64_DECODED_MOD_LEN(len) : BASE64_DECODED_LEN(len);

	if (!(result = st_alloc_opts(opts,new_len))) {
		log_pedantic("Unable to allocate memory for base64 decoding output.");
		return NULL;
	}

	if (modified) {
		b64_ret = base64_decode_mod(s,result);
	} else {
		b64_ret = base64_decode(s,result);
	}

	if (!b64_ret) {
		log_pedantic("Unable to perform base64 decoding operation.");
		st_free(result);
		result = NULL;
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

	new_len = BASE64_DECODED_MOD_LEN(len);

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

	// Get setup.
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
