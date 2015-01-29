
/**
 * @file /magma/servers/pop/parse.c
 *
 * @brief	The POP protocol parsers.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Parse the argument to a POP3 command as an unsigned number.
 * @param	con			the connection across which the pop command was issued.
 * @param	outnum		a pointer to an unsigned 64-bit integer to receive the numerical argument of the pop command on success.
 * @param	required	specifies whether or not a parameter is required.
 * @return	true if the pop command contained a valid unsigned number or false on failure.
 */
bool_t pop_num_parse(connection_t *con, uint64_t *outnum, bool_t required) {

	chr_t *input;
	chr_t *holder;
	size_t length;
	uint64_t result;

	if (!con || st_length_get(&(con->network.line)) <= 4) {
		log_pedantic("Invalid data was passed in for parsing.");
		return false;
	}

	// Get setup.
	input = st_data_get(&(con->network.line));
	length = st_length_get(&(con->network.line));

	// This is a POP3 command so we skip the first four characters.
	input += 4;
	length -= 4;

	// Skip leading spaces.
	while (length && *input == ' ') {
		input++;
		length--;
	}

	// Make sure something is still there.
	if (!length) {

		if (required) {
			log_pedantic("Did not find a valid argument.");
		}

		return false;
	}

	holder = input;

	// Get all of the valid characters.
	while (length && *holder >= '0' && *holder <= '9') {
		holder++;
		length--;
	}

	// Make sure we still have something to process.
	if (holder == input) {

		if (required) {
			log_pedantic("Did not find a valid argument.");
		}

		return false;
	}

	// Convert the string to an unsigned long long.
	if (!uint64_conv_bl(input, holder - input, &result)) {
		log_pedantic("We were unable to extract the numeric argument.");
		return false;
	}

	*outnum = result;

	return true;
}

/**
 * @brief	Parse the arguments to a POP3 TOP command.
 * @param	con		the POP3 client connection that issued the TOP command.
 * @param	number	a pointer to an unsigned 64 bit integer to receive the value of the TOP command's message sequence number.
 * @param	lines	a pointer to an unsigned 64 bit integer to receive the value of the TOP command's line number count.
 * @return	true on success or false if an error was encountered.
 */
bool_t pop_top_parse(connection_t *con, uint64_t *number, uint64_t *lines) {

	chr_t *input;
	chr_t *holder;
	size_t length;
	uint64_t result;

	if (!con || st_length_get(&(con->network.line)) <= 4) {
		log_pedantic("Invalid data was passed in for parsing.");
		return false;
	}

	// Get setup.
	input = st_data_get(&(con->network.line));
	length = st_length_get(&(con->network.line));

	// This is the TOP command so we skip the first three characters.
	input += 4;
	length -= 4;

	// Skip leading spaces.
	while (length && *input == ' ') {
		input++;
		length--;
	}

	// Make sure something is still there.
	if (!length) {
		log_pedantic("Did not find a valid argument.");
		return false;
	}

	holder = input;

	// Get all of the valid characters.
	while (length && *holder >= '0' && *holder <= '9') {
		holder++;
		length--;
	}

	// Make sure we still have something to process.
	if (holder == input) {
		log_pedantic("Did not find a valid argument.");
		return false;
	}

	// Convert the string to an unsigned long long.
	if (uint64_conv_bl(input, holder - input, &result) != true) {
		log_pedantic("We were unable to extract the numeric argument.");
		return false;
	}

	// Store the message number.
	*number = result;

	// Now go looking for the the number of lines.
	input = holder;

	// Skip leading spaces.
	while (length && *input == ' ') {
		input++;
		length--;
	}

	// Make sure something is still there.
	if (!length) {
		log_pedantic("Did not find a valid argument.");
		return false;
	}

	holder = input;

	// Get all of the valid characters.
	while (length && *holder >= '0' && *holder <= '9') {
		holder++;
		length--;
	}

	// Make sure we still have something to process.
	if (holder == input) {
		log_pedantic("Did not find a valid argument.");
		return false;
	}

	// Convert the string to an unsigned long long.
	if (uint64_conv_bl(input, holder - input, &result) != true) {
		log_pedantic("We were unable to extract the numeric argument.");
		return false;
	}

	// Store the lines parameter.
	*lines = result;

	return true;
}

/**
 * @brief	Parse a POP3 PASS command.
 * @note	This function will stop reading in password characters when an invalid character is encountered.
 * @param	con		the POP3 client connection that issued the PASS command.
 * @return	a managed string containing the user supplied password, or NULL on failure.
 */
stringer_t * pop_pass_parse(connection_t *con) {

	chr_t *input;
	chr_t *holder;
	size_t length;
	stringer_t *result;

	if (!con || st_length_get(&(con->network.line)) <= 4) {
		log_pedantic("Invalid data was passed in for parsing.");
		return NULL;
	}

	// Get setup.
	input = st_data_get(&(con->network.line));
	length = st_length_get(&(con->network.line));

	// This is a PASS command so we skip the first four characters.
	input += 4;
	length -= 4;

	// Skip leading spaces.
	while (length && *input == ' ') {
		input++;
		length--;
	}

	// Make sure something is still there.
	if (!length) {
		log_pedantic("Did not find any valid characters in the PASS argument.");
		return NULL;
	}

	holder = input;

	// Get all of the valid characters.
	while (length && *holder <= '~' && *holder >= '!') {
		holder++;
		length--;
	}

	// Make sure we still have something to process.
	if (holder == input) {
		log_pedantic("Did not find any valid characters in the PASS argument.");
		return NULL;
	}

	// Import the PASS argument into a stringer.
	if (!(result = st_import(input, holder - input))) {
		log_pedantic("POP3 PASS argument could not be retrieved.");
	}

	return result;
}

/**
 * @brief	Parse a POP3 USER command.
 * @note	The username can be at most 255 characters and will be returned in lowercase.
 * 			This function will stop reading in username characters when an invalid character is encountered.
 * @param	con		the POP3 client connection that issued the USER command.
 * @return	a managed string containing the validated username, or NULL on failure.
 */
stringer_t * pop_user_parse(connection_t *con) {

	chr_t *input;
	chr_t *holder;
	size_t length;
	stringer_t *result;

	if (!con || st_length_get(&(con->network.line)) <= 4) {
		log_pedantic("Invalid data was passed in for parsing.");
		return NULL;
	}

	// Get setup.
	input = st_data_get(&(con->network.line));
	length = st_length_get(&(con->network.line));

	// This is a USER command so we skip the first four characters.
	input += 4;
	length -= 4;

	// Skip leading spaces.
	while (length && *input == ' ') {
		input++;
		length--;
	}

	// Make sure something is still there.
	if (!length) {
		log_pedantic("Did not find any valid characters in the USER argument.");
		return NULL;
	}

	holder = input;

	// Get all of the valid characters.
	while (length && ((*holder >= 'A' && *holder <= 'Z') || (*holder >= 'a' && *holder <= 'z') ||
		(*holder >= '0' && *holder <= '9') || *holder == '-' || *holder == '_' || *holder == '.' ||
		*holder == '?' || *holder == '@' || *holder == '+' || *holder == '#' || *holder == '=')) {
		*holder = lower_chr(*holder);
		holder++;
		length--;
	}

	// Make sure we still have something to process.
	if (holder == input) {
		log_pedantic("Did not find any valid characters in the USER argument.");
		return NULL;
	}

	// Import the USER argument into a stringer. The database doesn't support usernames greater than 255 characters.
	if (holder - input > 255) {
		result = st_import(input, 255);
	}
	else {
		result = st_import(input, holder - input);
	}

	if (!result) {
		log_pedantic("POP3 USER argument could not be retrieved.");
	}

	return result;
}
