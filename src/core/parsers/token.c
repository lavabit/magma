
/**
 * @file /magma/core/parsers/token.c
 *
 * @brief	Functions for tokenizing strings.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Count the number of times a token is found in a specified block of memory.
 * @param	block	a pointer to a block of memory to be scanned.
 * @param	length	the length, in bytes, of the block of memory to be scanned.
 * @param	token	a character specifying the token to be searched.
 * @return	the number of times the token was found in the block of memory, or 0 on failure.
 */
// QUESTION: Why not return -1 on failure?
uint64_t tok_get_count_bl(void *block, size_t length, char token) {

	uint64_t count = 1;

#ifdef MAGMA_PEDANTIC
	if (!block) {
		log_options(M_LOG_PEDANTIC | M_LOG_STACK_TRACE, "Attempted to count the tokens using a NULL string pointer. Printing stack:");
	}
	else if (!length) {
		log_options(M_LOG_PEDANTIC | M_LOG_STACK_TRACE, "Attempted to count the tokens inside an empty string. Printing stack:");
	}
#endif

	// We can't search NULL pointers or empty strings.
	if (mm_empty(block, length)) {
		return 0;
	}

	for (uint64_t i = 0; i < length; i++) {

		if (*(char *)(block++) == token && i != length) {
			count++;
		}

	}
	return count;
}

/**
 * @brief	Count the number of times a token is found in a managed string.
 * @see		tok_get_count_bl()
 * @param	string	a pointer to the managed string to be scanned.
 * @param	token	a character specifying the token to be searched.
 * @return	the number of times the token was found in the managed string, or 0 on failure.
 */
uint64_t tok_get_count_st(stringer_t *string, char token) {

	return tok_get_count_bl(st_data_get(string), st_length_get(string), token);
}

/**
 * @brief	Retrieve a specified token from a null-terminated string.
 * @param	string		a pointer to the null-terminated string to be tokenized.
 * @param	length		the maximum number of characters to be scanned from the input string.
 * @param	token		the token character that will be used to split the string.
 * @param	fragment	the zero-indexed token number to be extracted from the string.
 * @param	value		a pointer to a placer that will receive the value of the extracted token on success, or pl_null() on failure.
 * @return	-1 on failure, 0 on success, or 1 if the token was extracted successfully, but was the last one in the string.
 */
int tok_get_ns(char *string, size_t length, char token, uint64_t fragment, placer_t *value) {

#ifdef MAGMA_PEDANTIC
	if (!string) {
		log_options(M_LOG_PEDANTIC | M_LOG_STACK_TRACE, "Attempted to count the tokens using a NULL string pointer. Printing stack:");
	}
	else if (!length) {
		log_options(M_LOG_PEDANTIC | M_LOG_STACK_TRACE, "Attempted to count the tokens inside an empty string. Printing stack:");
	}
#endif

	char *start;

	// We can't search NULL pointers or empty strings.
	if (!value || mm_empty(string, length)) {

		if (value) {
			*value = pl_null();
		}

		return -1;
	}

	while (fragment && length--) {

		if (*string++ == token) {
			fragment--;
		}

	}

	if (fragment) {
		*value = pl_null();
		return 1;
	}

	start = string;

	while (length && *string != token) {
		string++;
		length--;
	}

	// If we hit the token on the first character, return NULL
	if (start == string) {
		*value = pl_null();
	}
	else {
		*value = pl_init(start, string - start);
	}

	// We hit the end of the string
	if (!length) {
		return 1;
	}

	return 0;
}

/**
 * @brief	Retrieve a specified token from a block of data.
 * @see		tok_get_ns()
 * @param	block		a pointer to the memory block to be tokenized.
 * @param	length		the maximum number of characters to be scanned at the input data block.
 * @param	token		the token character that will be used to split the data block.
 * @param	fragment	the zero-indexed token number to be extracted from the data block.
 * @param	value		a pointer to a placer that will receive the value of the extracted token on success, or pl_null() on failure.
 * @return	-1 on failure, 0 on success, or 1 if the token was extracted successfully, but was the last one found in the data block.
 */
int tok_get_bl(void *block, size_t length, char token, uint64_t fragment, placer_t *value) {

	return tok_get_ns(block, length, token, fragment, value);
}

/**
 * @brief	Retrieve a specified token from a managed string.
 * @see		tok_get_ns()
 * @param	string		a pointer to the managed string to be tokenized.
 * @param	token		the token character that will be used to split the managed string.
 * @param	fragment	the zero-indexed token number to be extracted from the managed string.
 * @param	value		a pointer to a placer that will receive the value of the extracted token on success, or pl_null() on failure.
 * @return	-1 on failure, 0 on success, or 1 if the token was extracted successfully, but was the last one found in the managed string.
 */
int tok_get_st(stringer_t *string, char token, uint64_t fragment, placer_t *value) {

	return tok_get_bl(st_data_get(string), st_length_get(string), token, fragment, value);
}

/**
 * @brief	Retrieve a specified token from a placer.
 * @see		token_get_ns()
 * @param	string		a pointer to the placer to be tokenized.
 * @param	token		the token character that will be used to split the placer.
 * @param	fragment	the zero-indexed token number to be extracted from the placer.
 * @param	value		a pointer to a placer that will receive the value of the extracted token on success, or pl_null() on failure.
 * @return	-1 on failure, 0 on success, or 1 if the token was extracted successfully, but was the last one found in the placer.
 */
int tok_get_pl(placer_t string, char token, uint64_t fragment, placer_t *value) {

	return tok_get_bl(pl_data_get(string), pl_length_get(string), token, fragment, value);
}

void tok_pop_init_bl(tok_state_t *state, void *block, size_t length, char token) {

	if (!state) {
		return;
	}

	state->token = token;
	state->block = state->position = block;
	state->length = state->remaining = length;
}

void tok_pop_init_st(tok_state_t *state, stringer_t *string, char token) {

	if (!state) {
		return;
	}

	state->token = token;
	state->block = state->position = (string == NULL ? NULL : st_char_get(string));
	state->length = state->remaining = (string == NULL ? 0 : st_length_get(string));
}

/**
 * on error the function returns -1 and sets value to EMPTY_PLACER
 * on success the function returns 0 and stores the token in the placer pointed at by value
 * if the end is hit, then the function returns 1, and places any remaining data in value
 */
int tok_pop(tok_state_t *state, placer_t *value) {

	char *startPosition;

	// We can't search NULL pointers or empty strings.
	if (!value || !state || !state->position) {

		if (value) {
			*value = pl_null();
		}

		return -1;
	}

	// Handle situations where there is no more data.
	if (!(state->remaining)) {
		*value = pl_null();
		return 1;
	}


	// Handle situations where the pointer is already at the token.
	if (*(state->position) == state->token) {

		(state->position)++;
		(state->remaining)--;
		*value = pl_null();

		if (!(state->remaining)) {
			return 1;
		}

		return 0;
	}

	startPosition = state->position;

	while (*(state->position) != state->token && (state->remaining)) {
		(state->position)++;
		(state->remaining)--;
	}

	// If we hit the token on the first character, return NULL
	if ((startPosition) == state->position) {
		*value = pl_null();
	} else {
		*value = pl_init(startPosition, state->position - startPosition);
	}

	// We hit the end of the block
	if (!(state->remaining)) {
		return 1;
	}

	// Advance to the next character for next time.
	(state->position)++;
	(state->remaining)--;

	return 0;
}

/**
 * @brief	Count the number of times a string token is found in a specified block of memory.
 * @param	block	a pointer to a block of memory to be scanned.
 * @param	length	the length, in bytes, of the block of memory to be scanned.
 * @param	token	a pointer to the string token being used to split the input data.
 * @param	toklen	the length, in bytes, of the specified token.
 * @return	the number of times the string token was found in the block of memory, or 0 on failure.
 */
uint64_t str_tok_get_count_bl(void *block, size_t length, chr_t *token, size_t toklen) {

	uint64_t count = 0;
	placer_t haystack, needle;
	size_t hptr, skipped = 0;

	// We can't search NULL pointers or empty strings.
	if (mm_empty(block, length) || mm_empty(token, toklen)) {
		return 0;
	}

	haystack = pl_init(block, length);
	needle = pl_init(token, toklen);

	while ((skipped < length) && st_search_cs(&haystack, &needle, &hptr)) {
		skipped += pl_length_get (needle) + hptr;
		haystack = pl_init((char *) block + skipped, length-skipped);
		count++;
	}

	return count;
}

/**
 * @brief	Retrieve a specified string-split token from a null-terminated string.
 * @param	block		a pointer to the block of memory to be tokenized.
 * @param	length		the maximum number of characters to be scanned from the input data.
 * @param	token		the token string that will be used to split the data.
 * @param	toklen		the length, in bytes, of the token string.
 * @param	fragment	the zero-indexed token number to be extracted from the data.
 * @param	value		a pointer to a placer that will receive the value of the extracted token on success, or pl_null() on failure.
 * @return	-1 on failure, 0 on success, or 1 if the token was extracted successfully, but was the last one in the string.
 */
int str_tok_get_bl(char *block, size_t length, chr_t *token, size_t toklen, uint64_t fragment, placer_t *value) {

	placer_t haystack, needle;
	size_t hptr, skipped = 0;
	bool_t found;

	// We can't search NULL pointers or empty strings.
	if (!value || mm_empty(block, length) || mm_empty(token, toklen)) {
		*value = pl_null();
		return -1;
	}

	haystack = pl_init(block, length);
	needle = pl_init(token, toklen);

	while (fragment) {

		if (!(found = st_search_cs(&haystack, &needle, &hptr))) {
			*value = pl_null();
			return -1;
		}

		// Haystack becomes the entire block after the token.
		skipped += pl_length_get (needle) + hptr;
		haystack = pl_init(pl_char_get(haystack) + skipped, length-skipped);
		fragment--;
	}

	// If no more tokens are present, return everything we have left
	if (!st_search_cs(&haystack, &needle, &hptr)) {
		*value = haystack;
		return 1;
	}

	*value = pl_init(pl_char_get(haystack), hptr);

	return 0;
}

/**
 * @brief	Skip past any of the specified characters found at the beginning of the placer, and update the placer accordingly.
 * @param	place		a pointer to a placer that will be updated to skip past any of the specified characters.
 * @param	skipchars	a pointer to a buffer containing bytes that will be skipped at the beginning of the placer.
 * @param	nchars		the number of characters to be tested in the collection in skipchars.
 * @return	true if the skip operation completed before the end of the placer was reached, or false otherwise.
 */
bool_t pl_skip_characters (placer_t *place, char *skipchars, size_t nchars) {

	char *ptr = pl_char_get(*place);

	if (pl_empty(*place)) {
		return false;
	}

	for (int i = 0; i < place->length; i++, ptr++) {

		for (int j = 0; j <= nchars; j++) {

			// We went through all the skip characters without finding something... so this is where we return.
			if (j == nchars) {
				place->data = (char *) place->data + i;
				place->length -= i;
				return true;
			}

			if (*ptr == skipchars[j])
				break;

		}

	}

	return false;
}

/**
 * @brief	Skip to the first instance of any of the specified characters in the placer, and update the placer accordingly.
 * @param	place		a pointer to a placer that will be updated to skip to any of the specified characters.
 * @param	skiptochars	a pointer to a buffer containing bytes that will be skipped to when they are first found in the placer.
 * @param	nchars		the number of characters to be tested in the collection in skiptochars.
 * @return	true if the skip operation completed before the end of the placer was reached, or false otherwise.
 */
bool_t pl_skip_to_characters (placer_t *place, char *skiptochars, size_t nchars) {

	char *ptr = pl_char_get(*place);

	if (pl_empty(*place)) {
		return false;
	}

	for (int i = 0; i < place->length; i++, ptr++) {

		for (int j = 0; j < nchars; j++) {

			// We went through all the skip characters without finding something... so this is where we return.
			if (*ptr == skiptochars[j]) {
				place->data = (char *) place->data + i;
				place->length -= i;
				return true;
			}

		}

	}

	return false;
}

/**
 * @brief	Truncate a placer to start before any of the specified characters, and update the placer accordingly.
 * @param	place		a pointer to a placer that will be updated to be truncated before any of the specified characters.
 * @param	shrinkchars	a pointer to a buffer containing bytes that will be skipped when they are found at the end of the placer.
 * @param	nchars		the number of characters to be tested in the collection in shrinkchars.
 * @return	true if the shrink operation completed before the end of the placer was reached, or false otherwise.
 */
bool_t pl_shrink_before_characters (placer_t *place, char *shrinkchars, size_t nchars) {

	char *ptr = pl_char_get(*place) + pl_length_get(*place) - 1;

	if (pl_empty(*place)) {
		return false;
	}

	for (int i = 0; i < place->length; i++, ptr--) {

		for (int j = 0; j <= nchars; j++) {

				// We went through all the skip characters without finding something... so this is where we return.
				if (j == nchars) {
					place->length -= i;
					return true;
				}

				if (*ptr == shrinkchars[j])
					break;

		}

	}

	return false;
}

/**
 * @brief	Get a placer pointing to the text contained within a specified pair of opening and closing braces.
 * @param	str			a placer pointing to the string to be parsed.
 * @param	out			a pointer to a placer that will store the string between the braces on success.
 * @param	opening		the character to be the opening brace of the sequence.
 * @param	closing		the character to be the closing brace of the sequence.
 * @param	required	if true, fail if no data was found between the pair of braces.
 * @return	true if the brace sequence was complete, or false if either component could not be located.
 */
bool_t pl_get_embraced (placer_t str, placer_t *out, unsigned char opening, unsigned char closing, bool_t required) {

	char *ptr = pl_char_get(str);

	if (pl_empty(str)) {
		return false;
	}

	// Must have at least 2 characters for the opening and closing
	if (str.length < 2) {
		return false;
	} else if (*ptr != opening) {
		return false;
	}

	ptr++;

	for (int i = 1; i < str.length; i++, ptr++) {

		if (*ptr == closing) {

			if (required && i <= 1) {
				return false;
			}

			out->data = pl_char_get(str) + 1;
			out->length = i - 1;
			return true;
		}

	}

	// We hit the end without finding a closing character...
	return false;
}

/**
 * @brief	Ensure that a placer contains at least a certain amount of data, and update it accordingly.
 * @param	place	a pointer to the placer containing the original data, which will be updated on success.
 * @param	nchars	the minimum number of characters that the placer needs to contain to pass the test.
 * @param	more	if true, more characters following the minimum buffer size are necessary.
 * @return	true if the placer meets the minimum size check, or false if it does not.
 */
bool_t pl_update_start (placer_t *place, size_t nchars, bool_t more) {

	size_t ncheck = more ? nchars+1 : nchars;

	if (place->length < ncheck) {
		return false;
	}

	place->data = place->data + nchars;
	place->length -= nchars;

	return true;
}
