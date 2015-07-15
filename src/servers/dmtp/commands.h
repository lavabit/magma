
/**
 * @file /magma/servers/dmtp/commands.h
 *
 * @brief	The data structure for parsing and routing DMTP commands.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_SERVERS_DMTP_COMMANDS_H
#define MAGMA_SERVERS_DMTP_COMMANDS_H

command_t dmtp_commands[] = {

		// Global Commands
		{ .string = "EHLO", .length = 4, .function = &dmtp_ehlo },
		{ .string = "HELO", .length = 4, .function = &dmtp_helo },
		{ .string = "NOOP", .length = 4, .function = &dmtp_noop },
		{ .string = "MODE", .length = 4, .function = &dmtp_mode },
		{ .string = "RSET", .length = 4, .function = &dmtp_rset },
		{ .string = "QUIT", .length = 4, .function = &dmtp_quit },

		// Mail Commands
		{ .string = "MAIL", .length = 9, .function = &dmtp_mail },
		{ .string = "RCPT", .length = 7, .function = &dmtp_rcpt },
		{ .string = "DATA", .length = 4, .function = &dmtp_data },

		// Signet Commands
		{ .string = "SGNT", .length = 9, .function = &dmtp_sgnt },
		{ .string = "HIST", .length = 7, .function = &dmtp_hist },
		{ .string = "VRFY", .length = 4, .function = &dmtp_vrfy },

		// Debug Commands
		{ .string = "HELP", .length = 4, .function = &dmtp_help },
		{ .string = "VERB", .length = 4, .function = &dmtp_verb	}
};

#endif

