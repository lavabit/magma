
/**
 * @file /magma/engine/config/relay/relay.h
 *
 * @brief The types and functions involved with creating and accessing the relay structures.
 */

#ifndef MAGMA_ENGINE_CONFIG_RELAY_H
#define MAGMA_ENGINE_CONFIG_RELAY_H

typedef struct {
	size_t offset; /* The location in memory to store the setting value. */
	multi_t norm; /* The default value. */
	chr_t *name; /* The search key/name of the setting. */
	chr_t *description; /* Description of the setting and its default value. */
	bool_t required; /* Is this setting required? */
} relay_keys_t;

typedef struct {
	bool_t secure; /* Use TLS. */
	bool_t premium; /* Reserve for premium users. */
	chr_t *name; /* The relay name. */
	uint32_t port; /* The relay port. */
} relay_t;

void relay_free (void);
bool_t relay_validate (void);
void relay_output_settings (void);
relay_t *relay_alloc (uint32_t number);
bool_t relay_config (stringer_t *name, stringer_t *value);
bool_t relay_set_value (relay_keys_t *setting, relay_t *relay, stringer_t *value);
void relay_output_help(void);

#endif
