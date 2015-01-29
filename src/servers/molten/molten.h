
/**
 * @file /magma/servers/molten/molten.h
 *
 * @brief The entry point for the Molten server module.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_SERVERS_MOLTEN_H
#define MAGMA_SERVERS_MOLTEN_H

/// molten.c
void   molten_init(connection_t *con);
void   molten_invalid(connection_t *con);
void   molten_quit(connection_t *con);
void   molten_stats(connection_t *con);

/// commands.c
int_t   molten_compare(const void *compare, const void *command);
void    molten_parse(connection_t *con);
void    molten_sort(void);

/// sessions.c
void   molten_session_destroy(connection_t *con);

#endif
