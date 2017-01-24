
/**
 * @file /magma/servers/dmtp/dmtp.c
 *
 * @brief	Functions used to handle DMTP commands/actions.
 */

#include "magma.h"

/**
 * @brief	Process an DMTP EHLO command.
 * @param	con		the DMTP client connection issuing the command.
 * @return	This function returns no value.
 */
void dmtp_ehlo(connection_t *con) {

	// Spit back the standard DMTP greeting..
	con_print(con, "250 %.*s\n", st_length_int(con->server->domain), st_char_get(con->server->domain));
	return;

}

/**
 * @brief	Process an DMTP HELO command.
 * @param	con		the DMTP client connection issuing the command.
 * @return	This function returns no value.
 */
void dmtp_helo(connection_t *con) {

	// Spit back the standard DMTP greeting..
	con_print(con, "250 %.*s\n", st_length_int(con->server->domain), st_char_get(con->server->domain));
	return;

}

/**
 * @brief	Perform an DMTP NOOP (no-operation) command.
 * @note	This command does essentially nothing and is mostly a way to keep connections alive without
 * 			timing out due to inactivity.
 * @return	This function returns no value.
 */
void dmtp_noop(connection_t *con) {

	con_write_bl(con, "250 NOOP COMMAND COMPLETE\n", 26);
	return;
}

/**
 * @brief	A function that is executed when an invalid DMTP command is executed.
 * @return	This function returns no value.
 */
void dmtp_invalid(connection_t *con) {

	con->protocol.violations++;
	usleep(con->server->violations.delay);
	con_write_bl(con, "500 INVALID COMMAND\n", 20);
	return;
}

/**
 * @brief	Gracefully terminate an DMTP session, especially in response to an DMTP QUIT command.
 * @param	the DMTP client connection to be terminated.
 * @return	This functi#include "smtp/smtp.h"on returns no value.
 */
void dmtp_quit(connection_t *con) {

	if (con_status(con) == 2) {
		con_write_bl(con, "451 DATA CORRUPTION DETECTED\n", 29);
	}
	else if (con_status(con) >= 0) {
		con_write_bl(con, "221 BYE\n", 8);
	}
	else {
		con_write_bl(con, "421 ABNORMAL CONNECTION SHUTDOWN\n", 33);
	}

	con_destroy(con);
	return;
}

/**
 * @brief	Reset the DMTP session, in response to an DMTP RSET command.
 * @note	This command clears any sender, recipient, and mail data, along with all buffers and state tables.
 * 			The connection structure is reset to the same it was in immediately after the HELO/EHLO command.
 * @param	con		the DMTP client connection issuing the command.
 * @return	This function returns no value.
 */
void dmtp_rset(connection_t *con) {

	dmtp_session_reset(con);
	con_write_bl(con, "250 RSET COMMAND COMPLETE\n", 26);
	return;
}

/**
 * @brief	Specify the destination domain for a message in response to an DMTP RCPT command.
 * @param	con		the DMTP client connection issuing the command.
 * @return	This function returns no value.
 */
void dmtp_rcpt(connection_t *con) {

	con_write_bl(con, "250 RCPT COMMAND COMPLETE\n", 26);
	return;
}

/**
 * @brief	Specify the origin domain for a message in response to an DMTP MAIL command.
 * @param	con		the DMTP client connection issuing the command.
 * @return	This function returns no value.
 */
void dmtp_mail(connection_t *con) {

	// Spit back the all clear.
	con_write_bl(con, "250 MAIL COMMAND COMPLETE\n", 26);
	return;
}

/**
 * @brief	Process an DMTP MAIL command.
 * @param	con		the DMTP client connection issuing the command.
 * @return	This function returns no value.
 */
void dmtp_data(connection_t *con) {

	con_write_bl(con, "451 DATA FAILED - INTERNAL SERVER ERROR - PLEASE TRY AGAIN LATER\n", 65);
	dmtp_requeue(con);
	return;
}

void	dmtp_mode(connection_t *con) {

	// Spit back the all clear.
	con_write_bl(con, "250 DMTP\n", 9);
	return;
}

void	dmtp_sgnt(connection_t *con) {

	// Spit back the all clear.
	con_write_bl(con, "250 SGNT COMMAND COMPLETE\n", 26);
	return;
}

void	dmtp_hist(connection_t *con) {

	// Spit back the all clear.
	con_write_bl(con, "250 HIST COMMAND COMPLETE\n", 26);
	return;
}

void	dmtp_vrfy(connection_t *con) {

	// Spit back the all clear.
	con_write_bl(con, "250 VRFY COMMAND COMPLETE\n", 26);
	return;
}

void	dmtp_help(connection_t *con) {

	con_write_bl(con, "502 HELP COMMAND DISABLED\n", 26);
	return;
}

void	dmtp_verb(connection_t *con) {

	con_write_bl(con, "502 VERB COMMAND DISABLED\n", 26);
	return;
}

/**
 * @brief	The start of the protocol handler for the DMTP server.
 * @param	con		the new inbound DMTP client connection.
 * @return	This function returns no value.
 */
void dmtp_init(connection_t *con) {


	// Queue a reverse lookup.
	con_reverse_enqueue(con);

	// Print_t the greeting and queue for a new command.
	con_print(con, "220 %.*s DSMTP Magma\n", st_length_int(con->server->domain), st_char_get(con->server->domain));
	dmtp_requeue(con);

	return;
}
