
/**
 * @file /magma/servers/dmtp/dmtp.h
 *
 * @brief The entry point for the DMTP server module.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_SERVERS_DMTP_H
#define MAGMA_SERVERS_DMTP_H

/// commands.c
void 		dmtp_requeue(connection_t *con);
int_t   dmtp_compare(const void *compare, const void *command);
void    dmtp_process(connection_t *con);
void    dmtp_sort(void);

/// dmtp.c
void		dmtp_ehlo(connection_t *con);
void		dmtp_helo(connection_t *con);
void		dmtp_noop(connection_t *con);
void		dmtp_mode(connection_t *con);
void		dmtp_rset(connection_t *con);
void		dmtp_quit(connection_t *con);
void		dmtp_mail(connection_t *con);
void		dmtp_rcpt(connection_t *con);
void		dmtp_data(connection_t *con);
void		dmtp_sgnt(connection_t *con);
void		dmtp_hist(connection_t *con);
void		dmtp_vrfy(connection_t *con);
void		dmtp_help(connection_t *con);
void		dmtp_verb(connection_t *con);

void 		dmtp_init(connection_t *con);
void		dmtp_invalid(connection_t *con);

/// session.c
void    dmtp_session_reset(connection_t *con);
void    dmtp_session_destroy(connection_t *con);

#endif
