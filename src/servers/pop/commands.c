
/**
 * @file /magma/servers/pop/commands.c
 *
 * @brief	Functions involved with parsing and dispatching POP commands.
 */

#include "magma.h"
#include "commands.h"

int_t pop_compare(const void *compare, const void *command) {

	int_t result;
	command_t *cmd = (command_t *)command, *cmp = (command_t *)compare;

	if (!cmp->function)	result = st_cmp_ci_starts(PLACER(cmp->string, cmp->length), PLACER(cmd->string, cmd->length));
	else result = st_cmp_ci_eq(PLACER(cmp->string, cmp->length), PLACER(cmd->string, cmd->length));

	return result;
}

/**
 * @brief	Sort the POP3 command table to be ready for binary searches.
 * @return	This function returns no value.
 */
void pop_sort(void) {
	qsort(pop_commands, sizeof(pop_commands) / sizeof(pop_commands[0]), sizeof(command_t), &pop_compare);
	return;
}

void pop_requeue(connection_t *con) {

	if (!status() || con_status(con) < 0 || con->protocol.violations > con->server->violations.cutoff) {
		enqueue(&pop_quit, con);
	}
	else {
		enqueue(&pop_process, con);
	}

	return;
}

void pop_process(connection_t *con) {

	command_t *command, client = { .function = NULL };

	if (con_read_line(con, true) < 0) {
		con->command = NULL;
		enqueue(&pop_quit, con);
		return;
	}
	else if (pl_empty(con->network.line) && ((con->protocol.spins++) + con->protocol.violations) > con->server->violations.cutoff) {
		con->command = NULL;
		enqueue(&pop_quit, con);
		return;
	}
	else if (pl_empty(con->network.line)) {
		con->command = NULL;
		enqueue(&pop_process, con);
		return;
	}

	client.string = pl_char_get(con->network.line);
	client.length = pl_length_get(con->network.line);

	if ((command = bsearch(&client, pop_commands, sizeof(pop_commands) / sizeof(pop_commands[0]), sizeof(command_t), pop_compare))) {

		con->command = command;
		con->protocol.spins = 0;

		if (command->function == &pop_quit) {
			con->pop.expunge = true;
			enqueue(command->function, con);
		}
		else {
			requeue(command->function, &pop_requeue, con);
		}
	}
	else {
		con->command = NULL;
		requeue(&pop_invalid, &pop_requeue, con);
	}
	return;
}
