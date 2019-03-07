
/**
 * @file /magma/servers/molten/commands.h
 *
 * @brief The data structure involved with parsing and routing Molten commands.
 */

#ifndef MAGMA_SERVERS_MOLTEN_COMMANDS_H
#define MAGMA_SERVERS_MOLTEN_COMMANDS_H

command_t molten_commands[] = {
		{
			.string = "QUIT",
			.length = 4,
			.function = &molten_quit
		},{
			.string = "STATS",
			.length = 5,
			.function = &molten_stats
		},{
			.string = "VERSION",
			.length = 7,
			.function = &molten_version
		}
};

#endif
