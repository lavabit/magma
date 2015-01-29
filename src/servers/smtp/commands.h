
/**
 * @file /magma/servers/smtp/commands.h
 *
 * @brief	The data structure for parsing and routing SMTP commands.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_SERVERS_SMTP_COMMANDS_H
#define MAGMA_SERVERS_SMTP_COMMANDS_H

command_t smtp_commands[] = {
	{
		.string = "QUIT",
		.length = 4,
		.function = &smtp_quit
	}, {
		.string = "NOOP",
		.length = 4,
		.function = &smtp_noop
	}, {
		.string = "RSET",
		.length = 4,
		.function = &smtp_rset
	}, {
		.string = "HELO",
		.length = 4,
		.function = &smtp_helo
	}, {
		.string = "EHLO",
		.length = 4,
		.function = &smtp_ehlo
	}, {
		.string = "DATA",
		.length = 4,
		.function = &smtp_data
	}, {
		.string = "RCPT TO",
		.length = 7,
		.function = &smtp_rcpt_to
	}, {
		.string = "STARTTLS",
		.length = 8,
		.function = &smtp_starttls
	}, {
		.string = "MAIL FROM",
		.length = 9,
		.function = &smtp_mail_from
	}, {
		.string = "AUTH PLAIN",
		.length = 10,
		.function = &smtp_auth_plain
	}, {
		.string = "AUTH LOGIN",
		.length = 10,
		.function = &smtp_auth_login
	},

	// Disabled commands.
	{
		.string = "TURN",
		.length = 4,
		.function = &smtp_disabled
	}, {
		.string = "EXPN",
		.length = 4,
		.function = &smtp_disabled
	}, {
		.string = "HELP",
		.length = 4,
		.function = &smtp_disabled
	}, {
		.string = "VRFY",
		.length = 4,
		.function = &smtp_disabled
	}, {
		.string = "SEND",
		.length = 4,
		.function = &smtp_disabled
	}, {
		.string = "SOML",
		.length = 4,
		.function = &smtp_disabled
	}, {
		.string = "SAML",
		.length = 4,
		.function = &smtp_disabled
	}
};

#endif

