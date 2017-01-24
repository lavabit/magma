
/**
 * @file /magma/servers/pop/pop.h
 *
 * @brief	The entry point for the POP server module.
 */

#ifndef MAGMA_SERVERS_POP_H
#define MAGMA_SERVERS_POP_H

/// commands.c
int_t   pop_compare(const void *compare, const void *command);
void    pop_process(connection_t *con);
void    pop_requeue(connection_t *con);
void    pop_sort(void);

/// mailbox.c
uint64_t          pop_get_last(inx_t *messages);
meta_message_t *  pop_get_message(inx_t *messages, uint64_t get);
uint64_t          pop_total_messages(inx_t *messages);
uint64_t          pop_total_size(inx_t *messages);

/// parse.c
bool_t        pop_num_parse(connection_t *con, uint64_t *outnum, bool_t required);
stringer_t *  pop_pass_parse(connection_t *con);
bool_t        pop_top_parse(connection_t *con, uint64_t *number, uint64_t *lines);
stringer_t *  pop_user_parse(connection_t *con);

/// pop.c
void   pop_capa(connection_t *con);
void   pop_dele(connection_t *con);
void   pop_init(connection_t *con);
void   pop_invalid(connection_t *con);
void   pop_last(connection_t *con);
void   pop_list(connection_t *con);
void   pop_noop(connection_t *con);
void   pop_pass(connection_t *con);
void   pop_quit(connection_t *con);
void   pop_retr(connection_t *con);
void   pop_rset(connection_t *con);
void   pop_starttls(connection_t *con);
void   pop_stat(connection_t *con);
void   pop_top(connection_t *con);
void   pop_uidl(connection_t *con);
void   pop_user(connection_t *con);

/// sessions.c
void    pop_session_destroy(connection_t *con);
int_t   pop_session_reset(connection_t *con);

#endif
