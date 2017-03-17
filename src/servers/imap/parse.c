
/**
 * @file /magma/servers/imap/parse.c
 *
 * @brief	Functions used to handle IMAP commands and actions.
 */

#include "magma.h"

/// TODO: The logic here is just plain ugly. Specifically the logic used to read from the network and advance the buffers.

/**
 * @brief	Get the type code for a specified object in an array.
 * @note 	Valid return values include IMAP_ARGUMENT_TYPE_ARRAY, IMAP_ARGUMENT_TYPE_ASTRING, IMAP_ARGUMENT_TYPE_QSTRING,
 * 			IMAP_ARGUMENT_TYPE_NSTRING,	and IMAP_ARGUMENT_TYPE_LITERAL.
 * @param	array		a pointer to an imap arguments array.
 * @param	element		the zero-based index of the object in the imap arguments array to be examined.
 * @return	IMAP_ARGUMENT_TYPE_EMPTY on failure, or the element type code of the specified object on success.
 */
int_t imap_get_type_ar(imap_arguments_t *array, size_t element) {

	int_t result;

	if (!array) {
		log_pedantic("A NULL pointer was passed in.");
		return IMAP_ARGUMENT_TYPE_EMPTY;
	}

	// Make sure the element that was requested exists.
	if (element >= ar_length_get(array)) {
		log_pedantic("An invalid element number was passed in.");
		return IMAP_ARGUMENT_TYPE_EMPTY;
	}

	result = *(int_t *)(array + sizeof(size_t) + sizeof(size_t) + (element * (sizeof(uint32_t) + sizeof(void *))));

	return result;
}

/**
 * @brief	Return the value of a specified object in an array as a generic pointer.
 * @param	array		a pointer to an imap arguments array.
 * @param	element		the zero-based index of the object in the imap arguments array to be examined.
 * @return	NULL on failure, or a pointer to the specified object's value on success.
 */
void * imap_get_ptr(imap_arguments_t *array, size_t element) {

	void *result;

	if (!array) {
		log_pedantic("A NULL pointer was passed in.");
		return NULL;
	}

	if (element >= ar_length_get(array)) {
		log_pedantic("An invalid element number was passed in.");
		return NULL;
	}

	result = *(void **)(array + sizeof(size_t) + sizeof(size_t) + (element * (sizeof(uint32_t) + sizeof(void *))) + sizeof(uint32_t));

	return result;
}

/**
 * @brief	Return the value of a specified object in an array as a managed string.
 * @param	array		a pointer to an imap arguments array.
 * @param	element		the zero-based index of the object in the imap arguments array to be examined.
 * @return	NULL on failure, or a pointer to the specified object's value as a managed string on success.
 */
stringer_t * imap_get_st_ar(imap_arguments_t *array, size_t element) {

	stringer_t *result;

	if (!array) {
		log_pedantic("A NULL pointer was passed in.");
		return NULL;
	}

	if (element >= ar_length_get(array)) {
		log_pedantic("An invalid element number was passed in.");
		return NULL;
	}

	if (imap_get_type_ar(array, element) <= IMAP_ARGUMENT_TYPE_ARRAY) {
		log_pedantic("A stringer was requested, but this array element doesn't contain a stringer.");
	}

	result = *(stringer_t **)(array + sizeof(size_t) + sizeof(size_t) + (element * (sizeof(uint32_t) + sizeof(void *))) + sizeof(uint32_t));

	return result;
}

/**
 * @brief	Return the value of a specified object in an array as an imap arguments array.
 * @param	array		a pointer to an imap arguments array.
 * @param	element		the zero-based index of the object in the imap arguments array to be examined.
 * @return	NULL on failure, or a pointer to the specified object's value as an imap arguments array on success.
 */
imap_arguments_t * imap_get_ar_ar(imap_arguments_t *array, size_t element) {

	imap_arguments_t *result;

	if (!array) {
		log_pedantic("A NULL pointer was passed in.");
		return NULL;
	}

	if (element >= ar_length_get(array)) {
		log_pedantic("An invalid element number was passed in.");
		return NULL;
	}

	if (imap_get_type_ar(array, element) != IMAP_ARGUMENT_TYPE_ARRAY) {
		log_pedantic("An array was requested, but this element doesn't contain an array.");
	}

	result = *(imap_arguments_t **)(array + sizeof(size_t) + sizeof(size_t) + (element * (sizeof(uint32_t) + sizeof(void *))) + sizeof(uint32_t));

	return result;
}

/*
// Mailbox names can contain base64 data that needs to be decoded. Base64 data is inside the &- characters.
stringer_t * imap_string_to_mailbox(stringer_t *string) {



	return NULL;
}

// Non US characters need to be escaped and turned into base64.
stringer_t * imap_mailbox_to_string(stringer_t *mailbox) {



	return NULL;
}

*/

/**
 * @brief	Extract the contents of an atomic string and advance the position of the parser stream.
 * @note	This function scans a string, expecting printable ASCII characters until it encounters a space, \r, \n, (, or [.
 * 			If any other character is encountered before  that point, an error will be returned.
 * 			The supplied start and length pointers will be updated to reflect the input stream if the quoted string is parsed successfully.
 * @param	output	the address of a managed string that will receive a copy of the atomic string's contents on success, or NULL on failure.
 * @param	start	the address of a pointer to the start of the buffer to be parsed, that will also be updated
 * 					to point to the next argument in the sequence on success.
 * @param	length	a pointer to a size_t variable that contains the length of the string to be parsed, and which will be updated to reflect
 * 					the length of the remainder of the input string that follows the parsed atomic string.
 * @return	-1 on general error or if an invalid character was encountered, or 1 if the supplied atomic string was valid.
 */
int_t imap_parse_astring(stringer_t **output, chr_t **start, size_t *length) {

	chr_t *holder;
	size_t left;
	stringer_t *result;

	// Get setup.
	holder = *start;
	left = *length;
	*output = NULL;

	// Advance until we have a break character.
	while (left && *holder >= '!' && *holder <= '~' && *holder != '"' && *holder != '[' && *holder != '(') {
		holder++;
		left--;
	}

	// There was nothing to process or we hit an invalid character.
	if (*start == holder || (*holder != ' ' && *holder != '[' && *holder != '(' && *holder != '\r' && *holder != '\n')) {
		return -1;
	}

	// Create a stringer with the result.
	if (!(result = st_import(*start, holder - *start))) {
		log_pedantic("Unable to extract the atomic string.");
		return -1;
	}

	// Advance to the next argument.
	if (left && *holder != '[' && *holder != '(') {
		holder++;
		left--;
	}

	// Update
	*start = holder;
	*length = left;
	*output = result;

	return 1;
}

// Extracts an nil string. Not quite as liberal as the atomic string.
int_t imap_parse_nstring(stringer_t **output, chr_t **start, size_t *length, chr_t type) {

	chr_t *holder;
	size_t left;
	stringer_t *result;

	// Get setup.
	holder = *start;
	left = *length;
	*output = NULL;

	// Advance until we have a break character.
	while (left != 0 && *holder >= '!' && *holder <= '~' && *holder != '"' && *holder != '[' && *holder != type) {
		holder++;
		left--;
	}

	// There was nothing to process or we hit an invalid character.
	if (*start == holder || (*holder != ' ' && *holder != '\r' && *holder != '\n' && *holder != '[' && *holder != type)) {
		return -1;
	}

	// Create a stringer with the result.
	if (!(result = st_import(*start, holder - *start))) {
		log_error("Unable to extract the nil string.");
		return -1;
	}

	// Advance to the next argument.
	if (left != 0 && *holder != type && *holder != '[') {
		holder++;
		left--;
	}

	// Update
	*start = holder;
	*length = left;
	*output = result;

	return 1;
}

/**
 * @brief	Extract the contents of a quoted string and advance the position of the parser stream.
 * @note	This function scans a string beginning with '"' for a terminating '"', returning an error if \r or \n is encountered first.
 * 			It is also able to handle escaped characters.
 * 			The supplied start and length pointers will be updated to reflect the input stream if the quoted string is parsed successfully.
 * @param	output	the address of a managed string that will receive a copy of the quoted string's contents on success, or NULL on failure or if it is zero length.
 * @param	start	the address of a pointer to the start of the buffer to be parsed (beginning with \"), that will also be updated to
 * 					point to the next argument in the sequence on success.
 * @param	length	a pointer to a size_t variable that contains the length of the string to be parsed, and which will be updated to reflect
 * 					the length of the remainder of the input string that follows the parsed quoted string.
 * @return	-1 on general error or if an enclosing pair of double quotes was not found, or 1 if the supplied quoted string was valid.
 */
int_t imap_parse_qstring(stringer_t **output, chr_t **start, size_t *length) {

	chr_t *writer;
	chr_t *holder;
	size_t left;
	int_t escape = 0;
	stringer_t *result;

	// Get setup.
	holder = *start;
	left = *length;
	*output = NULL;

	// Skip the opening quote.
	if (*holder != '"' || !left) {
		return -1;
	}
	else {
		holder++;
		left--;
	}

	// Advance until we have a break character. Technically holder should be less than 127, but because were using a signed char, greater values are below NULL.
	while (left && *holder > '\0' && *holder != '\r' && *holder != '\n' && (escape == 1 || *holder != '"')) {

		if (escape == 0 && *holder == '\\') {
			escape = 1;
		}
		else if (escape != 0) {
			escape = 0;
		}

		holder++;
		left--;
	}

	// Check for valid data.
	if (*holder != '"') {
		return -1;
	}
	// Check for empty quoted strings. We return 1, but we also return a NULL pointer.
	if ((*start) + 1 == holder) {

		// Advance to the next argument.
		if (left) {
			holder++;
			left--;
		}

		// There should be a space before the next argument.
		if (left && *holder == ' ') {
			holder++;
			left--;
		}

		// Update
		*start = holder;
		*length = left;
		return 1;
	}

	// Allocate a buffer for the output.
	if (!(result = st_alloc(holder - *start))) {
		log_pedantic("Unable to allocate a buffer for the quoted string.");
		return -1;
	}

	// Get setup for the copy.
	writer = st_char_get(result);
	holder = *start;
	left = *length;

	// Skip the open quote again.
	holder++;
	left--;

	// Advance until we have a break character. Technically holder should be less than 127, but because were using a signed char, greater values are below NULL.
	while (left && *holder > '\0' && *holder != '\r' && *holder != '\n' && (escape == 1 || *holder != '"')) {

		if (escape == 0 && *holder == '\\') {
			escape = 1;
		}
		else if (escape != 0) {
			escape = 0;
		}

		// Copy unless we have the escape character.
		if (!escape) {
			*writer = *holder;
			writer++;
		}

		holder++;
		left--;
	}

	st_length_set(result, writer - (chr_t *)st_char_get(result));

	// Advance to the next argument.
	if (left) {
		holder++;
		left--;
	}

	// There should be a space before the next argument.
	if (left && *holder == ' ') {
		holder++;
		left--;
	}

	// Update
	*start = holder;
	*length = left;
	*output = result;

	return 1;
}

/**
 * @brief	Extract the contents of a literal string and advance the position of the parser stream.
 * @note	This function expects as input a string beginning with '{' and followed by a numerical string, an optional '+', and a closing '}'.
 	 	 	After reading in the numerical size parameter, it then attempts to read in that many bytes of input from the network stream.
 * @param	con		the client IMAP connection passing the literal string as input to the server.
 * @param	output	the address of a managed string that will receive a copy of the literal string's contents on success, or NULL on failure or if it is zero length.
 * @param	start	the address of a pointer to the start of the buffer to be parsed (beginning with '{'), that will also be updated to
 * 					point to the next argument in the sequence on success.
 * @param	length	a pointer to a size_t variable that contains the length of the string to be parsed, and that will be updated to reflect
 * 					the length of the remainder of the input string that follows the parsed literal string.
 * @return	-1 on general or parse error or if an enclosing pair of double quotes was not found, or 1 if the supplied quoted string was valid.
 */
int_t imap_parse_literal(connection_t *con, stringer_t **output, chr_t **start, size_t *length) {

	chr_t *holder;
	int_t plus = 0;
	stringer_t *result;
	size_t characters, left;
	ssize_t nread;
	uint64_t literal, number;

	// Get setup.
	holder = *start;
	left = *length;
	*output = NULL;

	// Skip the opening bracket.
	if (*holder != '{' || !left) {
		return -1;
	}
	else {
		holder++;
		left--;
	}

	// Advance until we have a break character.
	while (left && *holder >= '0' && *holder <= '9') {
		holder++;
		left--;
	}

	// Store the length.
	characters = holder - *start - 1;

	if (left && *holder == '+') {
		plus = 1;
		holder++;
		left--;
	}

	if (*holder != '}' || !characters) {
		return -1;
	}

	// Convert to a number. Make sure the number is positive.
	if (!uint64_conv_bl(*start + 1, characters, &number)) {
		return -1;
	}

	literal = (size_t)number;

	// If the number is larger than 128 megabytes, then reject it.
	if (!plus && number > 134217728) {
		return -1;
	}
	// They client is already transmitting, so read the entire file, then reject it.
	else if (number > 134217728) {

		while (number > 0) {

			// Read the data.
			if ((nread = con_read(con)) <= 0) {
				log_pedantic("The connection was dropped while reading the literal.");
				return -1;
			}

			// Deal with signedness problem.
			characters = nread;

			if (number > (uint64_t)characters) {
				number -= characters;
			}
			else {

				// If we have any extra characters in the buffer, move them to the beginning.
				if ((uint64_t)characters > number) {
					mm_move(st_char_get(con->network.buffer), st_char_get(con->network.buffer) + number, characters - number);
					st_length_set(con->network.buffer, characters - number);
					con->network.line = line_pl_st(con->network.buffer, 0);
				}
				else {
					st_length_set(con->network.buffer, 0);
					con->network.line = pl_null();
				}

				// Make sure we have a full line.
				if (pl_empty(con->network.line) && con_read_line(con, true) <= 0) {
					log_pedantic("The connection was dropped while reading the literal.");
					return -1;
				}

				number = 0;
			}

		}

		return -1;
	}

	// If this is not a plus literal, output the proceed statement.
	if (!plus) {
		con_write_bl(con, "+ GO\r\n", 6);
	}

	// Handle the special case of a zero length literal.
	if (literal == 0) {

		// Read the next line.
		if (con_read_line(con, true) <= 0) {
			log_pedantic("The connection was dropped while reading the literal.");
			return -1;
		}

		*start = st_char_get(con->network.buffer);
		*length = pl_length_get(con->network.line);

		// There should be a space before the next argument.
		if (*length && **start == ' ') {
			(*start)++;
			(*length)--;
		}

		return 1;
	}

	// Allocate a stringer for the buffer.
	if (!(result = st_alloc(literal))) {
		log_pedantic("Unable to allocate a buffer of %lu bytes for the literal argument.", literal);
		return -1;
	}

	// So we know how many more characters to read.
	left = literal;

	// Where we put the data.
	holder = st_char_get(result);

	// Keep looping until we run out of data.
	while (left) {

		// Read the data.
		if ((nread = con_read(con)) <= 0) {
			log_pedantic("The connection was dropped while reading the literal.");
			st_free(result);
			return -1;
		}

		characters = nread;

		// If we have a buffer, copy the data into the buffer.
		mm_copy(holder, st_char_get(con->network.buffer), (left > characters) ? characters : left);

		if (left > characters) {
			holder += characters;
			left -= characters;
		}
		else {
		 	st_length_set(result, literal);

			// If we have any extra characters in the buffer, move them to the beginning.
			if (characters > left) {
				mm_move(st_char_get(con->network.buffer), st_char_get(con->network.buffer) + left, characters - left);
				st_length_set(con->network.buffer, characters - left);
				con->network.line = line_pl_st(con->network.buffer, 0);
			}
			else {
					st_length_set(con->network.buffer, 0);
					con->network.line = pl_null();
			}

			// Make sure we have a full line.
			if (pl_empty(con->network.line) && con_read_line(con, true) <= 0) {
				log_pedantic("The connection was dropped while reading the literal.");
				st_free(result);
				return -1;
			}
			left = 0;
		}
	}

	*start = st_char_get(con->network.buffer);
	*length = pl_length_get(con->network.line);

	// There should be a space before the next argument.
	if (*length && **start == ' ') {
		(*start)++;
		(*length)--;
	}

	if (result != NULL) {
		*output = result;
	}
	else {
		return -1;
	}

	return 1;
}

// Parses a parenthetical/block array list.
/**
 * @brief	Extract the contents of an array argument and advance the position of the parser stream.
 *
 * @note	This function expects as input a string beginning either with '(' or '['.

 * @param	length	a pointer to a size_t variable that contains the length of the string to be parsed, and that will be updated to reflect
 * 					the length of the remainder of the input string that follows the parsed literal string.
 *
 * @param	recursion	d
 *
 * @param	con			the client IMAP connection passing the array as input to the server.
 * @param	array		-
 * @param	start		-
 * @param	length		-
 * @return
 */
int_t imap_parse_array(int_t recursion, connection_t *con, imap_arguments_t **array, chr_t **start, size_t *length) {

	chr_t type;
	stringer_t *result = NULL;
	imap_arguments_t *inner = NULL;

	// Recursion limiter.
	if (recursion >= IMAP_ARRAY_RECURSION_LIMIT) {
		log_pedantic("Recursion limit hit.");
		return -1;
	}

	if (!*start || !*length) {
		log_pedantic("Could not parse NULL IMAP argument array.");
		return -1;
	}

	// Skip the opening parentheses.
	if (**start == '(') {
		type = ')';
		(*start)++;
		(*length)--;
	}
	else if (**start == '[') {
		type = ']';
		(*start)++;
		(*length)--;
	}
	else {
		return -1;
	}

	while (*length && **start != type && **start >= '!' && **start <= '~') {
		// Quoted strings.
		if (**start == '"') {

			if (imap_parse_qstring(&result, start, length) != 1) {
				return -1;
			}

			ar_append(array, IMAP_ARGUMENT_TYPE_QSTRING, result);
		}
		// Literal strings.
		else if (**start == '{') {

			if (imap_parse_literal(con, &result, start, length) != 1) {
				return -1;
			}

			ar_append(array, IMAP_ARGUMENT_TYPE_LITERAL, result);
		}
		// Parenthetical/blocked arrays.
		else if (**start == '(' || **start == '[') {

			if (imap_parse_array(recursion + 1, con, &inner, start, length) != 1) {

				if (inner) {
					ar_free(inner);
				}

				return -1;
			}

			ar_append(array, IMAP_ARGUMENT_TYPE_ARRAY, inner);
			inner = NULL;
		}
		// Nil strings.
		else if (**start >= '!' && **start <= '~') {

			if (imap_parse_nstring(&result, start, length, type) != 1) {
				return -1;
			}

			ar_append(array, IMAP_ARGUMENT_TYPE_NSTRING, result);
		}
	}

	// Parser error.
	if (**start != type) {
		return -1;
	}

	// Skip the closing token.
	if (length && **start == type) {
		(*start)++;
		(*length)--;
	}

	// Advance a space to the next argument.
	if (length && **start == ' ') {
		(*start)++;
		(*length)--;
	}

	return 1;
}

// Figures out what type of item the incoming data is.
/**
 * @brief	Parse a chunk of input into an array of IMAP arguments.
 * @see		imap_parse_qstring(), imap_parse_literal(), imap_parse_array(), imap_parse_astring()
 * @note	This function scans a string for an array of arguments, performing the following logic when a particular character is encountered:
 * 			"      : Parse argument as quoted string with imap_parse_qstring().
 * 			{      : Parse argument as literal string with imap_parse_literal().
 * 			( or [ : Parse argument as array with imap_parse_astring().
 * 			other  : Defaults to parsing argument as atomic string with imap_parse_atomic().
 * 			The supplied start and length pointers will be updated to reflect the input stream if the quoted string is parsed successfully.

 * @param	start	the address of a pointer to the start of the buffer to be parsed, that will be continually updated
 * 					to point to the next argument in the sequence during the parsing loop.
 * @param	length	a pointer to a size_t variable that contains the length of the string to be parsed, that will be continually updated
 * 					with the input stream position during the parsing loop.
 * @return	-1 on parsing error or 1 on success.
 */
int_t imap_parse_arguments(connection_t *con, chr_t **start, size_t *length) {

	stringer_t *result = NULL;
	imap_arguments_t *array = NULL;

	while (*length && **start >= '!' && **start <= '~') {
		// Quoted strings.
		if (**start == '"') {

			if (imap_parse_qstring(&result, start, length) != 1) {
				return -1;
			}

			ar_append(&(con->imap.arguments), IMAP_ARGUMENT_TYPE_QSTRING, result);
		}
		// Literal strings.
		else if (**start == '{') {

			if (imap_parse_literal(con, &result, start, length) != 1) {
				return -1;
			}

			ar_append(&(con->imap.arguments), IMAP_ARGUMENT_TYPE_LITERAL, result);
		}
		// Parenthetical/blocked arrays.
		else if (**start == '(' || **start == '[') {

			if (imap_parse_array(0, con, &array, start, length) != 1) {

				if (array != NULL) {
					ar_free(array);
				}

				return -1;
			}

			ar_append(&(con->imap.arguments), IMAP_ARGUMENT_TYPE_ARRAY, array);
			array = NULL;
		}
		// Atomic strings.
		else if (**start >= '!' && **start <= '~') {

			if (imap_parse_astring(&result, start, length) != 1) {
				return -1;
			}

			ar_append(&(con->imap.arguments), IMAP_ARGUMENT_TYPE_ASTRING, result);
		}

	}

	return 1;
}

/// LOW: Have the merge related kinks in the argument parsing system been resolved? If so, can this function be returned to the graveyard?
/*void print_ar(size_t recursion, array_t *array) {

	size_t size, increment, inner;

	size = ar_length_get(array);

	for (increment = 0; increment < size; increment++) {

		if (imap_get_type_ar(array, increment) == ARRAY_TYPE_ARRAY) {
			print_ar(recursion + 1, imap_get_ar_ar(array, increment));
		}
		else {
			for (inner = 0; inner < recursion; inner++) {
				log_internal (__FILE__, __FUNCTION__, __LINE__, M_LOG_LINE_FEED_DISABLE | M_LOG_TIME_DISABLE | M_LOG_FILE_DISABLE | M_LOG_LINE_DISABLE | M_LOG_FUNCTION_DISABLE | M_LOG_STACK_TRACE_DISABLE, "\t");
			}
			log_internal (__FILE__, __FUNCTION__, __LINE__, M_LOG_LINE_FEED_DISABLE | M_LOG_TIME_DISABLE | M_LOG_FILE_DISABLE | M_LOG_LINE_DISABLE | M_LOG_FUNCTION_DISABLE | M_LOG_STACK_TRACE_DISABLE, "%.*s\n", st_length_int(imap_get_st_ar(array, increment)), st_char_get(imap_get_st_ar(array, increment)));
		}
	}

	return;
}*/

void imap_command_log_safe(stringer_t *line) {

	uchr_t *stream, *copy;
	size_t len, loc = 0;
	int_t i;

	if (st_empty_out(line, &stream, &len)) {
		return;
	}

	// A command string that doesn't even contain "LOGIN" is inherently "safe".
	if (!st_search_ci(line, PLACER("LOGIN", 5), &loc) || !loc) {
		log_info("%.*s", st_length_int(line), st_char_get(line));
		return;
	}


	// The LOGIN command should have been preceded by a whitespace.
	if (!chr_whitespace(stream[loc-1])) {
		log_info("%.*s", st_length_int(line), st_char_get(line));
		return;
	}

	// There should be should only be ONE more non-whitespace tag before the LOGIN command.
	for (i = loc-1; i >= 0; i--) {

		if (!chr_whitespace(stream[i])) {
			break;
		}

	}

	if (i < 0) {
		log_info("%.*s", st_length_int(line), st_char_get(line));
		return;
	}

	while (i >= 0) {

		if (chr_whitespace(stream[i])) {
			log_info("%.*s", st_length_int(line), st_char_get(line));
			return;
		}

		i--;
	}

	if (!(copy = mm_dupe(stream, len))) {
		return;
	}

	// Skip past "LOGIN" ...
	i = loc + 5;

	// and the trailing spaces.
	while ((i < len) && chr_whitespace(stream[i])) {
		i++;
	}

	if (i == len) {
		log_info("%.*s", st_length_int(line), st_char_get(line));
		mm_free(copy);
		return;
	}

	// The next parameter is the username. Skip past that as well.
	while ((i < len) && !chr_whitespace(stream[i])) {
		i++;
	}

	while ((i < len) && chr_whitespace(stream[i])) {
		i++;
	}

	if (i == len) {
		log_info("%.*s", st_length_int(line), st_char_get(line));
		mm_free(copy);
		return;
	}

	for(;i < len; i++) {
		copy[i] = '*';
	}

	log_info("%.*s", (int)len, copy);
	mm_free(copy);

	return;
}

/**
 * @brief	Parse an input line containing an IMAP command.
 * @note	This function updates the protocol-specific IMAP structure with parsed values for the session's tag, command, and arguments fields.
 * 			Special handling is performed for any command that is preceded by a "UID" prefix.
 * @param	con		the IMAP client connection issuing the command.
 * @return	1 on success or < 0 on error.
 *         -1: the tag could not be read.
 *         -2: the IMAP command could not be read.
 *         -3: the arguments to the IMAP command could not be read.
 */
int_t imap_command_parser(connection_t *con) {

	chr_t *holder;
	size_t length;

	if (!con) {
		log_pedantic("Invalid data was passed in for parsing.");
		return 0;
	}

	// Free the previous tag.
	st_cleanup(con->imap.tag);
	con->imap.tag = NULL;

	// Free the previous command.
	st_cleanup(con->imap.command);
	con->imap.command = NULL;

	// Free the previous arguments array.
	if (con->imap.arguments) {
		ar_free(con->imap.arguments);
		con->imap.arguments = NULL;
	}

	// Debug info.
	if (magma.log.imap) {
		imap_command_log_safe(&con->network.line);
	}

	// Get setup.
	holder = st_data_get(&(con->network.line));
	length = st_length_get(&(con->network.line));

	// Get the tag.
	if (imap_parse_astring(&(con->imap.tag), &holder, &length) != 1 || !con->imap.tag) {
		return -1;
	}

	// Get the command.
	if (imap_parse_astring(&(con->imap.command), &holder, &length) != 1 || !con->imap.command) {
		return -2;
	}

	// Check for the UID modifier.
	if (!st_cmp_ci_eq(con->imap.command, PLACER("UID", 3))) {
		con->imap.uid = 1;
		st_free(con->imap.command);
		con->imap.command = NULL;

		if (imap_parse_astring(&(con->imap.command), &holder, &length) != 1 || con->imap.command == NULL) {
			return -2;
		}

	}
	else if (con->imap.uid == 1) {
		con->imap.uid = 0;
	}

	// Now append the arguments to the array.
	if (imap_parse_arguments(con, &holder, &length) != 1) {
		return -3;
	}

	return 1;
}
