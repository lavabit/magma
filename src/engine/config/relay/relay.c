
/**
 *
 * @file /magma/engine/config/relay/relay.c
 *
 * @brief	Functions for handling relay server instance configurations.
 */

#include "magma.h"
#include "keys.h"

/**
 * @brief	Update the relay server counters to reflect the number of configured standard and premium relay servers.
 * @return	This function returns no value.
 */
void relay_counter(void) {

	uint32_t premium = 0, standard = 0;

	for (uint32_t i = 0; i < MAGMA_RELAY_INSTANCES; i++) {
		if (magma.relay.host[i] && magma.relay.host[i]->premium) {
			premium++;
		}
		else if (magma.relay.host[i] && !magma.relay.host[i]->premium) {
			standard++;
		}
	}

	magma.relay.count.premium = premium;
	magma.relay.count.standard = standard;
	return;
}

/**
 * @brief	Free magma's relay server configuration options.
 * @note	This function assumes all worker threads have finished, and should only be called during the normal shutdown process.
 * @param	This function returns no value.
 */
void relay_free(void) {

	for (uint32_t i = 0; i < MAGMA_RELAY_INSTANCES; i++) {

		for (uint64_t j = 0; magma.relay.host[i] && j < sizeof(relay_keys) / sizeof(relay_keys_t); j++) {
			switch (relay_keys[j].norm.type) {
			case (M_TYPE_BLOCK):
				if (*((void **)(((char *)magma.relay.host[i]) + relay_keys[j].offset))) {
					mm_free(*((void **)(((char *)magma.relay.host[i]) + relay_keys[j].offset)));
				}
				break;
			case (M_TYPE_NULLER):
				if (!ns_empty(*((char **)(((char *)magma.relay.host[i]) + relay_keys[j].offset)))) {
					ns_free(*((char **)(((char *)magma.relay.host[i]) + relay_keys[j].offset)));
				}
				break;
			case (M_TYPE_STRINGER):
				if (!st_empty(*((stringer_t **)(((char *)magma.relay.host[i]) + relay_keys[j].offset)))) {
					st_free(*((stringer_t **)(((char *)magma.relay.host[i]) + relay_keys[j].offset)));
				}
				break;
			default:
#ifdef MAGMA_PEDANTIC
			if (relay_keys[j].norm.type != M_TYPE_BOOLEAN && relay_keys[j].norm.type != M_TYPE_DOUBLE && relay_keys[j].norm.type != M_TYPE_FLOAT &&
					relay_keys[j].norm.type != M_TYPE_INT16 && relay_keys[j].norm.type != M_TYPE_INT32 && relay_keys[j].norm.type != M_TYPE_INT64 &&
					relay_keys[j].norm.type != M_TYPE_INT8 && relay_keys[j].norm.type != M_TYPE_UINT8 && relay_keys[j].norm.type != M_TYPE_UINT16 &&
					relay_keys[j].norm.type != M_TYPE_UINT32 && relay_keys[j].norm.type != M_TYPE_UINT64 && relay_keys[j].norm.type != M_TYPE_ENUM) {
				log_pedantic("Unexpected type. {type = %s = %u}", type(relay_keys[j].norm.type), relay_keys[j].norm.type);
			}
#endif
				break;
			}
		}

		if (magma.relay.host[i]) {
			mm_free(magma.relay.host[i]);
			magma.relay.host[i] = NULL;
		}
	}
	return;
}

/**
 * @brief	Allocate and initialize a magma server relay object, or return it if it already exists.
 * @param	number	a zero-based index into the global relay server table where the entry should be created and/or queried.
 * @return	NULL on failure, or a pointer to the requested relay server object on success.
 */
relay_t * relay_alloc(uint32_t number) {

	if (magma.relay.host[number])
		return magma.relay.host[number];

	if (!(magma.relay.host[number] = mm_alloc(sizeof(relay_t))))
		return NULL;

	// Loop through and set the default values.
	for (uint64_t i = 0; i < sizeof(relay_keys) / sizeof(relay_keys_t); i++) {
		if (!relay_keys[i].required && !relay_set_value(&relay_keys[i], magma.relay.host[number], NULL)) {
			log_critical("magma.iface.relay%s has an invalid default value.", relay_keys[i].name);
			return NULL;
		}
	}

	return magma.relay.host[number];
}
// QUESTION: What's up with the comments below? Where is the port validation?
/**
 * Iterates through all of the server structures and verifies that each was supplied with valid values for all of its required keys. This function also
 * applies the application specific validation rules. Specifically it verifies that only one server structure has been configured to use a given port number.
 */

/**
 * @brief	Validate all of the configured magma relay server instances.
 * @note	This makes set that all required config keys have been set, and that at least one host has been configured.
 * @return	true if all relay servers have been validated, or false on failure.
 */
bool_t relay_validate(void) {

	uint64_t hosts = 0;
	bool_t result = true;

	for (uint64_t i = 0; i < MAGMA_RELAY_INSTANCES; i++) {

		if (magma.relay.host[i]) {
			hosts++;
		}

		for (uint64_t j = 0; magma.relay.host[i] && j < sizeof(relay_keys) / sizeof(relay_keys_t); j++) {
			if (relay_keys[j].required) {
				switch (relay_keys[j].norm.type) {
				case (M_TYPE_NULLER):
					if (ns_empty(*((char **)(((char *)magma.relay.host[i]) + relay_keys[j].offset)))) {
						log_critical("magma.relay[%lu]%s is required and has not been set.", i, relay_keys[j].name);
						result = false;
					}
					break;
				case (M_TYPE_STRINGER):
					if (st_empty(*((stringer_t **)(((char *)magma.relay.host[i]) + relay_keys[j].offset)))) {
						log_critical("magma.relay[%lu]%s is required and has not been set.", i, relay_keys[j].name);
						result = false;
					}
					break;

				case (M_TYPE_INT8):
					if (*((int8_t *)(((char *)magma.relay.host[i]) + relay_keys[j].offset)) == relay_keys[j].norm.val.i8) {
						log_critical("magma.relay[%lu]%s is required and has not been set.", i, relay_keys[j].name);
						result = false;
					}
					break;

				case (M_TYPE_INT16):
					if (*((int16_t *)(((char *)magma.relay.host[i]) + relay_keys[j].offset)) == relay_keys[j].norm.val.i16) {
						log_critical("magma.relay[%lu]%s is required and has not been set.", i, relay_keys[j].name);
						result = false;
					}
					break;

				case (M_TYPE_INT32):
					if (*((int32_t *)(((char *)magma.relay.host[i]) + relay_keys[j].offset)) == relay_keys[j].norm.val.i32) {
						log_critical("magma.relay[%lu]%s is required and has not been set.", i, relay_keys[j].name);
						result = false;
					}
					break;

				case (M_TYPE_INT64):
					if (*((int64_t *)(((char *)magma.relay.host[i]) + relay_keys[j].offset)) == relay_keys[j].norm.val.i64) {
						log_critical("magma.relay[%lu]%s is required and has not been set.", i, relay_keys[j].name);
						result = false;
					}
					break;

				case (M_TYPE_UINT8):
					if (*((uint8_t *)(((char *)magma.relay.host[i]) + relay_keys[j].offset)) == relay_keys[j].norm.val.u8) {
						log_critical("magma.relay[%lu]%s is required and has not been set.", i, relay_keys[j].name);
						result = false;
					}
					break;

				case (M_TYPE_UINT16):
					if (*((uint16_t *)(((char *)magma.relay.host[i]) + relay_keys[j].offset)) == relay_keys[j].norm.val.u16) {
						log_critical("magma.relay[%lu]%s is required and has not been set.", i, relay_keys[j].name);
						result = false;
					}
					break;

				case (M_TYPE_UINT32):
					if (*((uint32_t *)(((char *)magma.relay.host[i]) + relay_keys[j].offset)) == relay_keys[j].norm.val.u32) {
						log_critical("magma.relay[%lu]%s is required and has not been set.", i, relay_keys[j].name);
						result = false;
					}
					break;

				case (M_TYPE_UINT64):
					if (*((uint64_t *)(((char *)magma.relay.host[i]) + relay_keys[j].offset)) == relay_keys[j].norm.val.u64) {
						log_critical("magma.relay[%lu]%s is required and has not been set.", i, relay_keys[j].name);
						result = false;
					}
					break;

					// Booleans must always be valid, since they only have two possible states. But since implementations vary, we check to make sure the
					// actual value matches what's been defined for true and false.
				case (M_TYPE_BOOLEAN):
					if (*((bool_t *)(((char *)magma.relay.host[i]) + relay_keys[j].offset)) != true && *((bool_t *)(((char *)magma.relay.host[i]) + relay_keys[j].offset)) != false) {
						log_critical("magma.relay[%lu]%s requires a valid boolean.", i, relay_keys[j].name);
						result = false;
					}
					break;

				default:
					log_pedantic("Unexpected type. {type = %s = %u}", type(relay_keys[j].norm.type), relay_keys[j].norm.type);
					break;
				}
			}
		}
	}

	// Make sure at least one host is configured.
	if (!hosts) {
		log_critical("magma.iface.relay requires at least one valid host.");
		result = false;
	}

	return result;
}

/*
 * @brief	Log the full contents of the magma relay server configuration settings.
 * @return	This function returns no value.
 */
void relay_output_settings(void) {

	for (uint32_t i = 0; i < MAGMA_RELAY_INSTANCES; i++) {

		if (magma.relay.host[i])
			log_info("\nRelay Instance %u\n--------------------------------------------------------------------------------", i);

		for (uint64_t j = 0; magma.relay.host[i] && j < sizeof(relay_keys) / sizeof(relay_keys_t); j++) {

			switch (relay_keys[j].norm.type) {
			case (M_TYPE_NULLER):
				if (ns_empty(*((char **)(((char *)magma.relay.host[i]) + relay_keys[j].offset))))
					log_info("magma.relay[%u]%s = NULL", i, relay_keys[j].name);
					else
					log_info("magma.relay[%u]%s = %s", i, relay_keys[j].name, *((char **)(((char *)magma.relay.host[i]) + relay_keys[j].offset)));
				break;

			case (M_TYPE_STRINGER):
				if (st_empty(*((stringer_t **)(((char *)magma.relay.host[i]) + relay_keys[j].offset))))
					log_info("magma.relay[%u]%s = NULL", i, relay_keys[j].name);
					else
					log_info("magma.relay[%u]%s = %.*s", i, relay_keys[j].name, st_length_int(*((stringer_t **)(((char *)magma.relay.host[i]) + relay_keys[j].offset))),
								st_char_get(*((stringer_t **)(((char *)magma.relay.host[i]) + relay_keys[j].offset))));
				break;

			case (M_TYPE_BOOLEAN):
				log_info("magma.relay[%u]%s = %s", i, relay_keys[j].name, (*((bool_t *) (((char *)magma.relay.host[i]) + relay_keys[j].offset)) == true ? "true" : "false"));
				break;

			case (M_TYPE_INT8):
				log_info("magma.relay[%u]%s = %hhi", i, relay_keys[j].name, *((int8_t *) (((char *)magma.relay.host[i]) + relay_keys[j].offset)));
				break;
			case (M_TYPE_INT16):
				log_info("magma.relay[%u]%s = %hi", i, relay_keys[j].name, *((int16_t *) (((char *)magma.relay.host[i]) + relay_keys[j].offset)));
				break;

			case (M_TYPE_INT32):
				log_info("magma.relay[%u]%s = %i", i, relay_keys[j].name, *((int32_t *) (((char *)magma.relay.host[i]) + relay_keys[j].offset)));
				break;

			case (M_TYPE_INT64):
				log_info("magma.relay[%u]%s = %li", i, relay_keys[j].name, *((int64_t *) (((char *)magma.relay.host[i]) + relay_keys[j].offset)));
				break;

			case (M_TYPE_UINT8):
				log_info("magma.relay[%u]%s = %hhu", i, relay_keys[j].name, *((uint8_t *) (((char *)magma.relay.host[i]) + relay_keys[j].offset)));
				break;
			case (M_TYPE_UINT16):
				log_info("magma.relay[%u]%s = %hu", i, relay_keys[j].name, *((uint16_t *) (((char *)magma.relay.host[i]) + relay_keys[j].offset)));
				break;
			case (M_TYPE_UINT32):
				log_info("magma.relay[%u]%s = %u", i, relay_keys[j].name, *((uint32_t *) (((char *)magma.relay.host[i]) + relay_keys[j].offset)));
				break;
			case (M_TYPE_UINT64):
				log_info("magma.relay[%u]%s = %lu", i, relay_keys[j].name, *((uint64_t *) (((char *)magma.relay.host[i]) + relay_keys[j].offset)));
				break;
			default:
				log_pedantic("Unexpected type. {type = %s = %u}", type(relay_keys[j].norm.type), relay_keys[j].norm.type);
				break;
			}
		}
	}

	return;
}

/**
 * @brief	Set a specified key for a relay server configuration entry.
 * @note	This function will allocate space for a copy of the key value, and return an error if it is not able to convert
 * 			it to the proper key data type.
 * @param	setting		a pointer to the relay server configuration key to be set.
 * @param	relay		a pointer to the relay server configuration entry to be adjusted.
 * @param	value		a managed string containing the new value of the config key.
 * @return	true if the value was successfully set, or false on error.
 */
bool_t relay_set_value(relay_keys_t *setting, relay_t *relay, stringer_t *value) {

	bool_t result = true;

	switch (setting->norm.type) {

	// Strings
	case (M_TYPE_NULLER):
		if (!ns_empty(*((char **)(((char *)relay) + setting->offset)))) {
			ns_free(*((char **)(((char *)relay) + setting->offset)));
			*((char **)(((char *)relay) + setting->offset)) = NULL;
		}
		if (!st_empty(value))
			*((char **)(((char *)relay) + setting->offset)) = ns_import(st_char_get(value), st_length_get(value));
		else if (!ns_empty(setting->norm.val.ns))
			*((char **)(((char *)relay) + setting->offset)) = ns_dupe(setting->norm.val.ns);
		break;

	case (M_TYPE_STRINGER):
		if (!st_empty(*((stringer_t **)(((char *)relay) + setting->offset)))) {
			st_free(*((stringer_t **)(((char *)relay) + setting->offset)));
			*((stringer_t **)(((char *)relay) + setting->offset)) = NULL;
		}
		if (!st_empty(value))
			*((stringer_t **)(((char *)relay) + setting->offset)) = st_dupe_opts(MANAGED_T | CONTIGUOUS | HEAP, value);
		else if (!st_empty(setting->norm.val.st))
			*((stringer_t **)(((char *)relay) + setting->offset)) = st_dupe_opts(MANAGED_T | CONTIGUOUS | HEAP, setting->norm.val.st);
		break;

		// Booleans
	case (M_TYPE_BOOLEAN):
		if (!st_empty(value)) {
			if (!st_cmp_ci_eq(value, CONSTANT("true")))
				*((bool_t *)(((char *)relay) + setting->offset)) = true;
			else if (!st_cmp_ci_eq(value, CONSTANT("false")))
				*((bool_t *)(((char *)relay) + setting->offset)) = false;
			else {
				log_critical("Invalid value for %s.", setting->name);
				result = false;
			}
		} else
			*((bool_t *)(((char *)relay) + setting->offset)) = setting->norm.val.binary;
		break;

		// Integers
	case (M_TYPE_INT8):
		if (!st_empty(value)) {
			if (!int8_conv_st(value, (int8_t *)(((char *)relay) + setting->offset))) {
				log_critical("Invalid value for %s.", setting->name);
				result = false;
			}
		} else
			*((int8_t *)(((char *)relay) + setting->offset)) = setting->norm.val.i8;
		break;

	case (M_TYPE_INT16):
		if (!st_empty(value)) {
			if (!uint16_conv_st(value, (uint16_t *)(((char *)relay) + setting->offset))) {
				log_critical("Invalid value for %s.", setting->name);
				result = false;
			}
		} else
			*((int16_t *)(((char *)relay) + setting->offset)) = setting->norm.val.u16;
		break;

	case (M_TYPE_INT32):
		if (!st_empty(value)) {
			if (!int32_conv_st(value, (int32_t *)(((char *)relay) + setting->offset))) {
				log_critical("Invalid value for %s.", setting->name);
				result = false;
			}
		} else
			*((int32_t *)(((char *)relay) + setting->offset)) = setting->norm.val.i32;
		break;

	case (M_TYPE_INT64):
		if (!st_empty(value)) {
			if (!int64_conv_st(value, (int64_t *)(((char *)relay) + setting->offset))) {
				log_critical("Invalid value for %s.", setting->name);
				result = false;
			}
		} else
			*((int64_t *)(((char *)relay) + setting->offset)) = setting->norm.val.i64;
		break;

		// Unsigned Integers
	case (M_TYPE_UINT8):
		if (!st_empty(value)) {
			if (!uint8_conv_st(value, (uint8_t *)(((char *)relay) + setting->offset))) {
				log_critical("Invalid value for %s.", setting->name);
				result = false;
			}
		} else
			*((uint8_t *)(((char *)relay) + setting->offset)) = setting->norm.val.u8;
		break;

	case (M_TYPE_UINT16):
		if (!st_empty(value)) {
			if (!uint16_conv_st(value, (uint16_t *)(((char *)relay) + setting->offset))) {
				log_critical("Invalid value for %s.", setting->name);
				result = false;
			}
		} else
			*((uint16_t *)(((char *)relay) + setting->offset)) = setting->norm.val.u16;
		break;

	case (M_TYPE_UINT32):
		if (!st_empty(value)) {
			if (!uint32_conv_st(value, (uint32_t *)(((char *)relay) + setting->offset))) {
				log_critical("Invalid value for %s.", setting->name);
				result = false;
			}
		} else
			*((uint32_t *)(((char *)relay) + setting->offset)) = setting->norm.val.u32;
		break;

	case (M_TYPE_UINT64):
		if (!st_empty(value)) {
			if (!uint64_conv_st(value, (uint64_t *)(((char *)relay) + setting->offset))) {
				log_critical("Invalid value for %s.", setting->name);
				result = false;
			}
		} else
			*((uint64_t *)(((char *)relay) + setting->offset)) = setting->norm.val.u64;
		break;

	default:
		log_critical("Invalid type. {name = %s / type = %s = %u}", setting->name, type(setting->norm.type), setting->norm.type);
		result = false;
		break;
	}


	relay_counter();
	return result;
}

/**
 * @brief	Set the value of a relay server configuration entry by name.
 * @note	This function is designed to operate on lines from a configuration source, like a file or database.
 * 			When a named server is referenced for the first time, a new relay server configuration instance will be allocated.
 * @param	name	a managed string containing the human-readable relay server configuration parameter to be set.
 * @param	value 	a managed string containing the new value of the relay server config key.
 * @return	true if the value was was successfully set, or false on failure.
 */
bool_t relay_config(stringer_t *name, stringer_t *value) {

	relay_t *relay;
	uint32_t serv_num;
	placer_t brack_val;
	bool_t match = false;

	// Make sure were passed a valid value.
	if (st_cmp_ci_starts(name, CONSTANT("magma.relay"))) {
		log_critical("%.*s is not a valid setting.", st_length_int(name), st_char_get(name));
		return false;
	}

	// Extract the inner bracket value, and convert it to a number. If either step fails, return an error.
	// We've hard coded an offset of 11 bytes to account for the length of the namespace.
	if (pl_empty((brack_val = bracket_extract_pl(st_char_get(name) + 11, st_length_get(name) - 11))) || !uint32_conv_bl(pl_char_get(brack_val), pl_length_get(brack_val), &serv_num)) {
		log_critical("%*.s is not a valid setting.", st_length_int(name), st_char_get(name));
		return false;
	}

	// Enforce the limit on server instances.
	if (serv_num >= MAGMA_RELAY_INSTANCES) {
		log_critical("%.*s is invalid, %u is the maximum number of servers possible.", st_length_int(name), st_char_get(name), MAGMA_RELAY_INSTANCES);
		return false;
	}

	// Get a pointer to the server structure. If necessary, allocate a new value.
	if (!(relay = relay_alloc(serv_num))) {
		log_critical("Unable to allocate a clean relay structure.");
		return false;
	}

	for (uint64_t i = 0; !match && i < sizeof(relay_keys) / sizeof(relay_keys_t); i++) {

		// We've hard coded an offset of 24 bytes to account for the length of the namespace, plus the two brackets..
		if (!st_cmp_ci_eq(PLACER(st_char_get(name) + 13 + pl_length_get(brack_val), st_length_get(name) - 13 - pl_length_get(brack_val)), NULLER(relay_keys[i].name))) {

			match = true;

			if (!relay_set_value(&relay_keys[i], relay, value)) {
				log_critical("%.*s has an invalid value.", st_length_int(name), st_char_get(name));
				return false;
			}

		}
	}

	if (!match) {
		log_critical("%.*s is not a valid setting.", st_length_int(name), st_char_get(name));
		return false;
	}

	return true;
}


/**
 * @brief	Log all relay key information to be returned via "magma -h".
 * @return	This function returns no value.
 */
void relay_output_help(void) {
	log_info("\nRelay options:");

	for (uint64_t j = 0; j < sizeof(relay_keys) / sizeof(relay_keys_t); j++) {
		log_options(M_LOG_LINE_FEED_DISABLE | M_LOG_TIME_DISABLE | M_LOG_FILE_DISABLE | M_LOG_LINE_DISABLE | M_LOG_FUNCTION_DISABLE |	M_LOG_STACK_TRACE_DISABLE, "\t");
		config_output_value_generic("magma.relay[n]", relay_keys[j].name, relay_keys[j].norm.type, &(relay_keys[j].norm.val.bl), relay_keys[j].required);
		log_info("\t\t%s", relay_keys[j].description);
	}

}
