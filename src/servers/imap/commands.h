
/**
 * @file /magma/servers/imap/commands.h
 *
 * @brief The data structure involved with parsing and routing POP commands.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_SERVERS_IMAP_COMMANDS_H
#define MAGMA_SERVERS_IMAP_COMMANDS_H

command_t imap_commands[] = {
	{	.string = "ID", .length = 2, .function = &imap_id},
	{	.string = "COPY", .length = 4, .function = &imap_copy},
	{	.string = "IDLE", .length = 4, .function = &imap_idle},
	{	.string = "LIST", .length = 4, .function = &imap_list},
	{	.string = "LSUB", .length = 4, .function = &imap_lsub},
	{	.string = "NOOP", .length = 4, .function = &imap_noop},
	{	.string = "CHECK", .length = 5, .function = &imap_check},
	{	.string = "CLOSE", .length = 5, .function = &imap_close},
	{	.string = "FETCH", .length = 5, .function = &imap_fetch},
	{	.string = "LOGIN", .length = 5, .function = &imap_login},
	{	.string = "STORE", .length = 5, .function = &imap_store},
	{	.string = "APPEND", .length = 6, .function = &imap_append},
	{	.string = "CREATE", .length = 6, .function = &imap_create},
	{	.string = "DELETE", .length = 6, .function = &imap_delete},
	{	.string = "RENAME", .length = 6, .function = &imap_rename},
	{	.string = "SEARCH", .length = 6, .function = &imap_search},
	{	.string = "SELECT", .length = 6, .function = &imap_select},
	{	.string = "LOGOUT", .length = 6, .function = &imap_logout},
	{	.string = "STATUS", .length = 6, .function = &imap_status},
	{	.string = "EXAMINE", .length = 7, .function = &imap_examine},
	{	.string = "EXPUNGE", .length = 7, .function = &imap_expunge},
	{	.string = "STARTTLS", .length = 8, .function = &imap_starttls},
	{	.string = "SUBSCRIBE", .length = 9, .function = &imap_subscribe},
	{	.string = "CAPABILITY", .length = 10, .function = &imap_capability},
	{	.string = "UNSUBSCRIBE", .length = 11, .function = &imap_unsubscribe}
};

#endif

