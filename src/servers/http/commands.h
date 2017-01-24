
/**
 * @file /magma/servers/http/commands.h
 *
 * @brief The data structure involved with parsing and routing POP commands.
 */

#ifndef MAGMA_SERVERS_HTTP_COMMANDS_H
#define MAGMA_SERVERS_HTTP_COMMANDS_H

/*

command_t http_commands[] = {
	{
		.string = "CLOSE",
		.length = 5,
		.function = &http_close
	}
};

int_t http_compare(const void *compare, const void *command) {

	int_t result;
	command_t *cmd = (command_t *)command, *cmp = (command_t *)compare;

	if (!cmp->function)	result = st_cmp_ci_starts(PLACER(cmp->string, cmp->length), PLACER(cmd->string, cmd->length));
	else result = st_cmp_ci_eq(PLACER(cmp->string, cmp->length), PLACER(cmd->string, cmd->length));

	return result;
}

void http_sort(void) {
	qsort(http_commands, sizeof(http_commands) / sizeof(http_commands[0]), sizeof(command_t), &http_compare);
	return;
}


void http_process(connection_t *con) {

	command_t *command, client = { .function = NULL };

	if (con_read_line(con) == -1) {
		con->command = NULL;
		enqueue(&http_close, con);
		return;
	}
	else if (pl_empty(con->network.line) && ((con->protocol.spins++) + con->protocol.violations) > con->server->violations.cutoff) {
		con->command = NULL;
		enqueue(&http_close, con);
		return;
	}
	else if (pl_empty(con->network.line)) {
		con->command = NULL;
		enqueue(&http_process, con);
		return;
	}

	client.string = pl_char_get(con->network.line);
	client.length = pl_length_get(con->network.line);

	if ((command = bsearch(&client, http_commands, sizeof(http_commands) / sizeof(http_commands[0]), sizeof(command_t), http_compare))) {

		con->command = command;
		con->protocol.spins = 0;

		if (command->function == &http_close) {
			enqueue(command->function, con);
		}
		else {
			requeue(command->function, &http_requeue, con);
		}
	}
	else {
		con->command = NULL;
		requeue(&http_invalid, &http_requeue, con);
	}
	return;
}

*/
#endif

