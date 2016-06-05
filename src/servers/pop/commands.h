
/**
 * @file /magma/servers/pop/commands.h
 *
 * @brief The data structure involved with parsing and routing POP commands.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_SERVERS_POP_COMMANDS_H
#define MAGMA_SERVERS_POP_COMMANDS_H

// Didn't recognize the POP3 command.
command_t pop_commands[] = {
	{
		.string = "TOP",
		.length = 3,
		.function = &pop_top
	}, {
		.string = "QUIT",
		.length = 4,
		.function = &pop_quit
	}, {
		.string = "NOOP",
		.length = 4,
		.function = &pop_noop
	}, {
		.string = "USER",
		.length = 4,
		.function = &pop_user
	}, {
		.string = "PASS",
		.length = 4,
		.function = &pop_pass
	}, {
		.string = "RSET",
		.length = 4,
		.function = &pop_rset
	}, {
		.string = "STAT",
		.length = 4,
		.function = &pop_stat
	}, {
		.string = "LIST",
		.length = 4,
		.function = &pop_list
	}, {
		.string = "DELE",
		.length = 4,
		.function = &pop_dele
	}, {
		.string = "RSET",
		.length = 4,
		.function = &pop_rset
	}, {
		.string = "UIDL",
		.length = 4,
		.function = &pop_uidl
	}, {
		.string = "RETR",
		.length = 4,
		.function = &pop_retr
	}, {
		.string = "CAPA",
		.length = 4,
		.function = &pop_capa
	}, {
		.string = "LAST",
		.length = 4,
		.function = &pop_last
	}, {
		.string = "STLS",
		.length = 4,
		.function = &pop_starttls
	}, {
		.string = "STARTTLS",
		.length = 8,
		.function = &pop_starttls
	}
};

#endif

