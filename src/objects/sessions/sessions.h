
/**
 * @file /magma/objects/sessions/sessions.h
 *
 * @brief	Functions for handling web sessions.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_OBJECTS_SESSIONS_H
#define MAGMA_OBJECTS_SESSIONS_H

/// sessions.c
session_t *   sess_create(connection_t *con, stringer_t *path, stringer_t *application);
void          sess_destroy(session_t *sess);
int_t         sess_get(connection_t *con, stringer_t *application, stringer_t *path, stringer_t *token);
uint64_t      sess_key(void);
uint64_t      sess_number(void);
void          sess_ref_add(session_t *sess);
void          sess_ref_dec(session_t *sess);
time_t        sess_ref_stamp(session_t *sess);
uint64_t      sess_ref_total(session_t *sess);
bool_t        sess_refresh_check(session_t *sess);
void          sess_refresh_flush(session_t *sess);
time_t        sess_refresh_stamp(session_t *sess);
void          sess_release(session_t *sess);
void          sess_serial_check(session_t *sess, uint64_t object);
stringer_t *  sess_token(session_t *sess);
void          sess_trigger(session_t *sess);
void          sess_update(session_t *sess);
void          sess_release_attachment(attachment_t *attachment);
void          sess_release_composition(composition_t *comp);

#endif

