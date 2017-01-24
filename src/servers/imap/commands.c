
/**
 * @file /magma/servers/imap/commands.c
 *
 * @brief The functions involved with parsing and routing POP commands.
 */

#include "magma.h"
#include "commands.h"

/**
 * @brief	Compare the names of two commands.
 * @note	This is an internal function used to sort imap commands and search for them.
 * @param	compare		a pointer to the first command to be compared.
 * @param	command		a pointer to the second command to be compared.
 * @return	-1 if compare < command, 1 if command < compare, or 0 if the two commands are equal.
 */
int_t imap_compare(const void *compare, const void *command) {

	int_t result;
	command_t *cmd = (command_t *)command, *cmp = (command_t *)compare;

	if (!cmp->function)	result = st_cmp_ci_starts(PLACER(cmp->string, cmp->length), PLACER(cmd->string, cmd->length));
	else result = st_cmp_ci_eq(PLACER(cmp->string, cmp->length), PLACER(cmd->string, cmd->length));

	return result;
}

/**
 * @brief	Sort the IMAP command table to be ready for binary searches.
 * @return	This function returns no value.
 */
void imap_sort(void) {
	qsort(imap_commands, sizeof(imap_commands) / sizeof(imap_commands[0]), sizeof(command_t), &imap_compare);
	return;
}

/**
 * @brief	Requeue an imap connection for processing, or log it out if there was an error or excess of protocol violations.
 * @param	con		a pointer to the connection object of the imap session.
 * @return	This function returns no value.
 */
void imap_requeue(connection_t *con) {

	if (!status() || con_status(con) < 0 || con_status(con) == 2 || con->protocol.violations > con->server->violations.cutoff) {
		enqueue(&imap_logout, con);
	}
	else {
		enqueue(&imap_process, con);
	}

	return;
}

/**
 * @brief	Perform client command processing on an established imap session.
 * @note	This function will read the next line of user input, parse the command, and then attempt to execute it with the appropriate handler.
 * @param	con		a pointer to the connection object underlying the imap session.
 * @return	This function returns no value.
 */
void imap_process(connection_t *con) {

	int_t state;
	command_t *command, client = { .function = NULL };

	// If the connection indicates an error occurred, or the socket was closed by the client we send the connection to the logout function.
	if (((state = con_read_line(con, false)) < 0) || (state == -2)) {
		con->command = NULL;
		enqueue(&imap_logout, con);
		return;
	}
	else if (pl_empty(con->network.line) && ((con->protocol.spins++) + con->protocol.violations) > con->server->violations.cutoff) {
		con->command = NULL;
		enqueue(&imap_logout, con);
		return;
	}
	else if (pl_empty(con->network.line)) {
		con->command = NULL;
		enqueue(&imap_process, con);
		return;
	}

	// Parse the line into its tag and command elements.
	if ((state = imap_command_parser(con)) < 0) {

		// Try to be helpful about the parsing error.
		if (state == -1) {
			con_write_bl(con, "* BAD Unable to parse the command tag.\r\n", 40);
		}
		else if (state == -2) {
			con_print(con, "%.*s BAD Unable to parse the command.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		}
		else {
			con_print(con, "%.*s BAD The command arguments were submitted using an invalid syntax.\r\n", st_length_int(con->imap.tag), st_char_get(con->imap.tag));
		}

		// If the client keeps breaking rules drop them.
		if (((con->protocol.spins++) + con->protocol.violations) > con->server->violations.cutoff) {
			con->command = NULL;
			enqueue(&imap_logout, con);
			return;
		}

		// Requeue and hope the next line of data is useful.
		con->command = NULL;
		enqueue(&imap_process, con);
		return;

	}

	client.string = st_char_get(con->imap.command);
	client.length = st_length_get(con->imap.command);

	if ((command = bsearch(&client, imap_commands, sizeof(imap_commands) / sizeof(imap_commands[0]), sizeof(command_t), imap_compare))) {

		con->command = command;
		con->protocol.spins = 0;

		if (command->function == &imap_logout) {
			enqueue(command->function, con);
		}
		else {
			requeue(command->function, &imap_requeue, con);
		}
	}
	else {
		con->command = NULL;
		requeue(&imap_invalid, &imap_requeue, con);
	}
	return;
}
