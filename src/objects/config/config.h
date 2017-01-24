
/**
 * @file /magma/objects/config/config.h
 *
 * @brief	The user configuration interface.
 */

#ifndef MAGMA_OBJECTS_CONFIG_H
#define MAGMA_OBJECTS_CONFIG_H

typedef struct __attribute__ ((__packed__)) {
	uint64_t flags;
	stringer_t *key, *value;
} user_config_entry_t;

// Possible flag values for user config entries.
enum {
	USER_CONF_STATUS_NONE = 0,
	USER_CONF_STATUS_CRITICAL = 1
};

typedef struct __attribute__ ((__packed__)) {
	inx_t *entries;
	uint64_t usernum, serial;
} user_config_t;

/// config.c
user_config_t *        user_config_alloc(uint64_t usernum);
user_config_t *        user_config_create(uint64_t usernum);
int_t                  user_config_edit(user_config_t *collection, stringer_t *key, stringer_t *value);
user_config_entry_t *  user_config_entry_alloc(stringer_t *key, stringer_t *value, uint64_t flags);
void                   user_config_entry_free(user_config_entry_t *entry);
void                   user_config_free(user_config_t *collection);
int_t                  user_config_update(user_config_t *collection);

/// datatier.c
int_t   user_config_delete(uint64_t usernum, stringer_t *key);
bool_t  user_config_fetch(user_config_t *collection);
int_t   user_config_upsert(uint64_t usernum, stringer_t *key, stringer_t *value, uint64_t flags);

#endif

