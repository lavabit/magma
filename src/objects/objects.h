
/**
 * @file /magma/objects/objects.h
 *
 * @brief	Functions used to interface with objects.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#ifndef MAGMA_OBJECTS_H
#define MAGMA_OBJECTS_H

#include "auth/auth.h"
#include "meta/meta.h"
#include "warehouse/warehouse.h"
#include "folders/folders.h"
#include "contacts/contacts.h"
#include "messages/messages.h"
#include "config/config.h"
#include "cred/neue.h"
#include "users/users.h"
#include "mail/mail.h"
#include "sessions/sessions.h"

enum {
	OBJECT_USER,
	OBJECT_CONFIG,
	OBJECT_FOLDERS,
	OBJECT_MESSAGES,
	OBJECT_CONTACTS
};

typedef struct {
	inx_t *meta, *users, *sessions;
} object_cache_t;

extern object_cache_t objects;

/// locks.c
int_t   lock_get(stringer_t *key);
void    lock_release(stringer_t *key);
int_t   user_lock(uint64_t usernum);
void    user_unlock(uint64_t usernum);

/// objects.c
bool_t obj_cache_start(void);
void obj_cache_prune(void);
void obj_cache_stop(void);

/// serials.c
uint64_t serial_get(uint64_t type, uint64_t num);
uint64_t serial_increment(uint64_t type, uint64_t num);
uint64_t serial_reset(uint64_t type, uint64_t num);

#endif
