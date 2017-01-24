
/**
 * @file /magma/servers/molten/commands.c
 *
 * @brief Functions used to parse protocol specific data out of the inbound network stream.
 */

#include "magma.h"
#include "commands.h"

// Compare points to the line read in via the network, while command points to the array of legal commands. Were trying to figure out if the line starts with a command we recognize.
int_t molten_compare(const void *compare, const void *command) {

	int_t result;
	command_t *cmd = (command_t *)command, *cmp = (command_t *)compare;

	if (!cmp->function)	result = st_cmp_ci_starts(PLACER(cmp->string, cmp->length), PLACER(cmd->string, cmd->length));
	else result = st_cmp_ci_eq(PLACER(cmp->string, cmp->length), PLACER(cmd->string, cmd->length));

	return result;
}

/**
 * @brief	Sort the Molten command table to be ready for binary searches.
 * @return	This function returns no value.
 */
void molten_sort(void) {
	qsort(molten_commands, sizeof(molten_commands) / sizeof(molten_commands[0]), sizeof(command_t), &molten_compare);
	return;
}

void molten_parse(connection_t *con) {

	command_t *command, client = { .function = NULL };

	if (con_read_line(con, false) < 0) {
		con->command = NULL;
		enqueue(&molten_quit, con);
		return;
	}
	else if (pl_empty(con->network.line)) {
		con->command = NULL;
		enqueue(&molten_parse, con);
		return;
	}

	client.string = pl_char_get(con->network.line);
	client.length = pl_length_get(con->network.line);

	if ((command = bsearch(&client, molten_commands, sizeof(molten_commands) / sizeof(molten_commands[0]), sizeof(command_t), molten_compare))) {
		con->command = command;
		enqueue(command->function, con);
	} else {
		con->command = NULL;
		enqueue(&molten_invalid, con);
	}
	return;
}

