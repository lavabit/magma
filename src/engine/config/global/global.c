
/**
 *
 * @file /magma/engine/config/global/global.c
 *
 * @brief	Functions for handling the global configuration.
 */
#include "magma.h"
#include "keys.h"

__thread char threadBuffer[1024];
magma_t magma = { .config.file = "magma.config" };
bool_t exit_and_dump = false;
stringer_t *cmdline_config_data = NULL;

/// LOW: We should use use basename() and dirname() to cleanup path strings.

/**
 * @brief	Free all loaded magma configuration options.
 * @note	First all magma config keys will be freed, then the cache servers, relay servers, and magma servers.
 * @return	This function returns no value.
 */
void config_free(void) {

	for (uint64_t i = 0; i < sizeof(magma_keys) / sizeof(magma_keys_t); i++) {
		switch (magma_keys[i].norm.type) {

		case (M_TYPE_BLOCK):
			if (*((void **)(magma_keys[i].store))) {
				mm_free(*((void **)(magma_keys[i].store)));
			}
			break;
		case (M_TYPE_NULLER):
			if (*((char **)(magma_keys[i].store))) {
				ns_free(*((char **)(magma_keys[i].store)));
			}
			break;
		case (M_TYPE_STRINGER):

			// Intercept the blacklist config key.
			if (!st_cmp_cs_eq(NULLER(magma_keys[i].name), PLACER("magma.smtp.blacklist", 20))) {
				for (uint32_t j = 0; j < magma.smtp.blacklists.count; j++) {
					st_free(magma.smtp.blacklists.domain[j]);
				}
			}
			else if (*((stringer_t **)(magma_keys[i].store))) {
				st_free(*((stringer_t **)(magma_keys[i].store)));
			}
			break;
		default:
#ifdef MAGMA_PEDANTIC
			if (magma_keys[i].norm.type != M_TYPE_BOOLEAN && magma_keys[i].norm.type != M_TYPE_DOUBLE && magma_keys[i].norm.type != M_TYPE_FLOAT &&
					magma_keys[i].norm.type != M_TYPE_INT16 && magma_keys[i].norm.type != M_TYPE_INT32 && magma_keys[i].norm.type != M_TYPE_INT64 &&
					magma_keys[i].norm.type != M_TYPE_INT8 && magma_keys[i].norm.type != M_TYPE_UINT8 && magma_keys[i].norm.type != M_TYPE_UINT16 &&
					magma_keys[i].norm.type != M_TYPE_UINT32 && magma_keys[i].norm.type != M_TYPE_UINT64 && magma_keys[i].norm.type != M_TYPE_ENUM) {
				log_pedantic("Unexpected type. {type = %s = %u}", type(magma_keys[i].norm.type), magma_keys[i].norm.type);
			}
#endif
			break;
		}
	}

	cache_free();
	relay_free();
	servers_free();
	return;
}

/**
 * @brief	Output a key name and value in a generic way.
 * @param	prefix		a pointer to a null-terminated string containing an optional prefix to be printed before the supplied key name.
 * @param	name		a pointer to a null-terminated string containing the name of the key being output.
 * @param	type		an M_TYPE value specifying the multi-type of the key value.
 * @param	val			a void pointer to the value of the specified key, which will be printed in accordance with the supplied multi-type.
 * @param	required	a boolean value specifying whether the specified key is a required configuration option.
 * @return	This function returns no value.
 */
void config_output_value_generic(chr_t *prefix, chr_t *name, M_TYPE type, void *val, bool_t required) {

	chr_t *reqstr = "";

	if (!prefix) {
		prefix = "";
	}

	if (required) {
		reqstr = "*";
	}

	switch (type) {
	case (M_TYPE_NULLER):
		if (ns_empty(*((char **)(val))))
			log_info("%s%s%s = NULL", prefix, name, reqstr);
		else
			log_info("%s%s%s = %s", prefix, name, reqstr, *((char **)(val)));
		break;

	case (M_TYPE_STRINGER):
		// Intercept the blacklist config key->
		if (!st_cmp_cs_eq(NULLER(name), PLACER("magma.smtp.blacklist", 20))) {

			if (!magma.smtp.blacklists.count) {
				log_info("%s%s%s = NULL", prefix, name, reqstr);
			}

			for (uint32_t j = 0; j < magma.smtp.blacklists.count; j++) {
				log_info("%s%s%s = %.*s", prefix, name, reqstr, st_length_int(magma.smtp.blacklists.domain[j]), st_char_get(magma.smtp.blacklists.domain[j]));
			}
		}

		else if (st_empty(*((stringer_t **)(val))))
			log_info("%s%s%s = NULL", prefix, name, reqstr);
		else
			log_info("%s%s%s = %.*s", prefix, name, reqstr, st_length_int(*((stringer_t **)(val))), st_char_get(*((stringer_t **)(val))));
		break;

	case (M_TYPE_ENUM):

			if (!st_cmp_cs_eq(NULLER(name), CONSTANT(".protocol"))) {
				if (*((M_PROTOCOL *)((char *)val)) == MOLTEN)
					log_info("%s%s%s = MOLTEN", prefix, name, reqstr);
				else if (*((M_PROTOCOL *)((char *)val)) == HTTP)
					log_info("%s%s%s = HTTP", prefix, name, reqstr);
				else if (*((M_PROTOCOL *)((char *)val)) == POP)
					log_info("%s%s%s = POP", prefix, name, reqstr);
				else if (*((M_PROTOCOL *)((char *)val)) == IMAP)
					log_info("%s%s%s = IMAP", prefix, name, reqstr);
				else if (*((M_PROTOCOL *)((char *)val)) == SMTP)
					log_info("%s%s%s = SMTP", prefix, name, reqstr);
				else if (*((M_PROTOCOL *)((char *)val)) == DMTP)
					log_info("%s%s%s = DMTP", prefix, name, reqstr);
				else if (*((M_PROTOCOL *)((char *)val)) == SUBMISSION)
					log_info("%s%s%s = SUBMISSION", prefix, name, reqstr);
				else if (*((M_PROTOCOL *)((char *)val)) == GENERIC)
						log_info("%s%s%s = EMPTY", prefix, name, reqstr);
				else
					log_info("%s%s%s = [UNKNOWN]", prefix, name, reqstr);
			} else if (!st_cmp_cs_eq(NULLER(name), CONSTANT(".network.type"))) {
				if (*((M_PORT *)((char *)val)) == TCP_PORT)
					log_info("%s%s%s = TCP", prefix, name, reqstr);
				else if (*((M_PORT *)((char *)val)) == TLS_PORT)
					log_info("%s%s%s = TLS", prefix, name, reqstr);
				else
					log_info("%s%s%s = [UNKNOWN]", prefix, name, reqstr);
			}
		break;

	case (M_TYPE_BOOLEAN):
		log_info("%s%s%s = %s", prefix, name, reqstr, (*((bool_t *)(val)) ? "true" : "false"));
		break;

	case (M_TYPE_INT8):
		log_info("%s%s%s = %hhi", prefix, name, reqstr, *((int8_t *)(val)));
		break;
	case (M_TYPE_INT16):
		log_info("%s%s%s = %hi", prefix, name, reqstr, *((int16_t *)(val)));
		break;

	case (M_TYPE_INT32):
		log_info("%s%s%s = %i", prefix, name, reqstr, *((int32_t *)(val)));
		break;

	case (M_TYPE_INT64):
		log_info("%s%s%s = %li", prefix, name, reqstr, *((int64_t *)(val)));
		break;

	case (M_TYPE_UINT8):
		log_info("%s%s%s = %hhu", prefix, name, reqstr, *((uint8_t *)(val)));
		break;
	case (M_TYPE_UINT16):
		log_info("%s%s%s = %hu", prefix, name, reqstr, *((uint16_t *)(val)));
		break;
	case (M_TYPE_UINT32):
		log_info("%s%s%s = %u", prefix, name, reqstr, *((uint32_t *)(val)));
		break;
	case (M_TYPE_UINT64):
		log_info("%s%s%s = %lu", prefix, name, reqstr, *((uint64_t *)(val)));
		break;
	default:
		log_pedantic("Unexpected type. {type = %u}", type);
		break;
	}

	return;
}

/**
 * @brief	Log the contents of a magma configuration option.
 * @param	key		a pointer to the magma configuration key to be dumped.
 * @return	This function returns no value.
 */
void config_output_value(magma_keys_t *key) {

	switch (key->norm.type) {
	case (M_TYPE_NULLER):
		if (ns_empty(*((char **)(key->store))))
			log_info("%s = NULL", key->name);
		else
			log_info("%s = %s", key->name, *((char **)(key->store)));
		break;

	case (M_TYPE_STRINGER):
		// Intercept the blacklist config key->
		if (!st_cmp_cs_eq(NULLER(key->name), PLACER("magma.smtp.blacklist", 20))) {

			if (!magma.smtp.blacklists.count) {
				log_info("%s = NULL",key->name);
			}

			for (uint32_t j = 0; j < magma.smtp.blacklists.count; j++) {
				log_info("%s = %.*s",key->name, st_length_int(magma.smtp.blacklists.domain[j]), st_char_get(magma.smtp.blacklists.domain[j]));
			}
		}

		else if (st_empty(*((stringer_t **)(key->store))))
			log_info("%s = NULL", key->name);
		else
			log_info("%s = %.*s", key->name, st_length_int(*((stringer_t **)(key->store))), st_char_get(*((stringer_t **)(key->store))));
		break;

	case (M_TYPE_BOOLEAN):
		log_info("%s = %s", key->name, (*((bool_t *)(key->store)) ? "true" : "false"));
		break;

	case (M_TYPE_INT8):
		log_info("%s = %hhi", key->name, *((int8_t *)(key->store)));
		break;
	case (M_TYPE_INT16):
		log_info("%s = %hi", key->name, *((int16_t *)(key->store)));
		break;

	case (M_TYPE_INT32):
		log_info("%s = %i", key->name, *((int32_t *)(key->store)));
		break;

	case (M_TYPE_INT64):
		log_info("%s = %li", key->name, *((int64_t *)(key->store)));
		break;

	case (M_TYPE_UINT8):
		log_info("%s = %hhu", key->name, *((uint8_t *)(key->store)));
		break;
	case (M_TYPE_UINT16):
		log_info("%s = %hu", key->name, *((uint16_t *)(key->store)));
		break;
	case (M_TYPE_UINT32):
		log_info("%s = %u", key->name, *((uint32_t *)(key->store)));
		break;
	case (M_TYPE_UINT64):
		log_info("%s = %lu", key->name, *((uint64_t *)(key->store)));
		break;
	default:
		log_pedantic("Unexpected type. {type = %u}", key->norm.type);
		break;
	}

	return;
}

/**
 * @brief	Log a display of all active magma configuration settings.
 * @return	This function returns no value.
 */
void config_output_settings(void) {

	log_info("\n\nMagma Configuration\n--------------------------------------------------------------------------------");
	log_info("magma.host.name = %s", magma.host.name);
	log_info("magma.host.number = %lu", magma.host.number);
	log_info("magma.config_file = %s", magma.config.file);

	for (uint64_t i = 0; i < sizeof(magma_keys) / sizeof(magma_keys_t); i++) {
		config_output_value(&magma_keys[i]);
	}

	// Run through and output all of the cache config information.
	cache_output_settings();

	relay_output_settings();

	// Run through and output all of the server instances.
	servers_output_settings();

	log_info("\n");
	return;
}

/**
 * @brief	Log all magma config key settings, as well as server, relay server, and cache server settings.
 * @return	This function returns no value.
 */
void config_output_help(void) {
	log_info("Global options:");

	for (uint64_t i = 0; i < sizeof(magma_keys) / sizeof(magma_keys_t); i++) {
		log_options(M_LOG_LINE_FEED_DISABLE | M_LOG_TIME_DISABLE | M_LOG_FILE_DISABLE | M_LOG_LINE_DISABLE | M_LOG_FUNCTION_DISABLE |	M_LOG_STACK_TRACE_DISABLE, "\t");
		//config_output_value(&magma_keys[i]);
		config_output_value_generic("", magma_keys[i].name, magma_keys[i].norm.type, &(magma_keys[i].norm.val.bl), magma_keys[i].required);
		log_info("\t\t%s", magma_keys[i].description);
	}

	cache_output_help();
	relay_output_help();
	servers_output_help();

	log_info("\n");
	return;
}

/**
 * @brief	Validate all the user configuration settings.
 * @note	The steps are as follows:
 * 			1. Check to see that all required keys have been set.
 *			2. If magma.iface.virus.available is set, magma.iface.virus.signatures must be set.
 *			3. Make sure 10 <= magma.iface.cache.retry <= 86400
 *			4. Make sure 1 <= magma.iface.cache.timeout <= 3600
 *			5. Make sure magma.iface.cache.retry <= magma.iface.cache.timeout
 *			6. Make sure 40 <= magma.smtp.wrap_line_length <= 65535
 *			7. Make sure 8 <= magma.smtp.recipient_limit <= 32768
 *			8. Make sure 16 <= magma.smtp.relay_limit
 *			9. Make sure 16384 <= system_ulimit_max(RLIMIT_STACK)
 *			10. If magma.system.daemonize is set, make sure magma.output.file is not false
 *			11. If magma.output.file is enabled, magma.output.path must be set.
 *			12. If magma.dkim.enabled is set, then magma.dkim.domain, magma.dkim.selector, and magma.dkim.key must all be set.
 *			13. Validate all the configured magma servers, relay servers, and cache servers.
 *			14. Check all config key filenames and directories to ensure that they exist and are accessible.
 *			15. Make sure magma.admin.contact and point to valid email addresses, if they are specified.
 *			16. If magma.config.output_config is set, dump the current configuration.
 */
bool_t config_validate_settings(void) {

	int64_t limit;
	magma_keys_t *key;
	bool_t result = true;

	// Run through all of the magma_keys and make sure the required magma_keys have been set.
	for (uint64_t i = 0; i < sizeof(magma_keys) / sizeof(magma_keys_t); i++) {
		if (magma_keys[i].required && !magma_keys[i].set) {
			log_critical("%s is required and must be set.", magma_keys[i].name);
			result = false;
		}
	}

	// A special option. If we're calling --dump from the command line, we don't want to do full validation yet, and we also want to exit immediately afterwards.
	if (exit_and_dump) {
		config_output_settings();
		return false;
	}

	// Combination option checks.
	if (magma.iface.virus.available && !magma.iface.virus.signatures) {
		log_critical("magma.iface.virus.signatures is required and must be set when the virus scanner is enabled.");
		result = false;
	}
	else {
		CONFIG_CHECK_DIR_READABLE(magma.iface.virus.signatures);
	}

	// Cache retry time.
	if (magma.iface.cache.retry < 10) {
		log_critical("magma.iface.cache.retry is required to be 10 or larger.");
		result = false;
	}
	else if(magma.iface.cache.retry > 86400) {
		log_critical("magma.iface.cache.retry is required to be 86400 or smaller.");
		result = false;
	}

	// Cache server socket timeout.
	if (magma.iface.cache.timeout < 1) {
		log_critical("magma.iface.cache.timeout is required to be 1 or larger.");
		result = false;
	}
	else if (magma.iface.cache.timeout > 3600) {
		log_critical("magma.iface.cache.timeout is required to be 3600 or smaller.");
		result = false;
	}

	// Cache timing sanity check.
	if (magma.iface.cache.retry <= magma.iface.cache.timeout) {
		log_critical("magma.iface.cache.retry must be larger than magma.iface.cache.timeout.");
		result = false;
	}

	// Line wrapping range check.
	if (magma.smtp.wrap_line_length < 40) {
		log_critical("magma.smtp.wrap_line_length is required to be 40 or larger.");
		result = false;
	}
	else if (magma.smtp.wrap_line_length > 65535) {
		log_critical("magma.smtp.wrap_line_length is required to be 65535 or smaller.");
		result = false;
	}

	// The message recipient limit.
	if (magma.smtp.recipient_limit < 8) {
		log_critical("magma.magma.smtp.recipient_limit is required to be 8 or larger.");
		result = false;
	}
	else if (magma.smtp.recipient_limit > 32768) {
		log_critical("magma.magma.smtp.recipient_limit is required to be 32768 or smaller.");
		result = false;
	}

	// The relay limit
	if (magma.smtp.relay_limit < 16) {
		log_critical("magma.magma.smtp.relay_limit is required to be 16 or larger.");
		result = false;
	}
	else if (magma.smtp.relay_limit > 512) {
		log_critical("magma.magma.smtp.relay_limit is required to be 512 or smaller.");
		result = false;
	}

	// The legal thread stack range.
	if (magma.system.thread_stack_size < PTHREAD_STACK_MIN) {
		log_critical("magma.system.thread_stack_size is required to be %i or larger.", PTHREAD_STACK_MIN);
		result = false;
	}
	else if ((limit = system_ulimit_max(RLIMIT_STACK)) > 0 && magma.system.thread_stack_size > limit) {
		log_critical("magma.system.thread_stack_size is required to be %li or larger.", limit);
		result = false;
	}

	// Confirm that file based logging is being used if were asked to daemonize.
	if (magma.system.daemonize && !magma.output.file && (!(key = config_key_lookup(PLACER("magma.output.file", 17))) || key->set)) {
		log_critical("magma.output.file must not be set to false if the system has also been configured to daemonize.");
		result = false;
	}
	// If were asked to daemonize assume file based logging is now the default.
	else if (magma.system.daemonize && !magma.output.file) {
		magma.output.file = true;
	}

	if (magma.output.file && !magma.output.path) {
		log_critical("magma.output.path is required when file based logging is enabled.");
		result = false;
	}

	if (magma.dkim.enabled && (!magma.dkim.domain || !magma.dkim.selector || !magma.dkim.key)) {
		log_critical("If magma.dkim.enabled is set, then magma.dkim.domain, magma.dkim.selector, and magma.dkim.key must all be set!");
		result = false;
	}

	// Validate the magma server keys here.
	if (!servers_validate()) {
		result = false;
	}

	// Validate the relay server here.
	if (!relay_validate()) {
		result = false;
	}

	// Validate the cache server keys.
	if (!cache_validate()) {
		result = false;
	}

	// Validate read access to the shared object library..
	CONFIG_CHECK_FILE_READABLE(magma.library.file);

	// At this stage we only verify that we can read these keys. Deeper validation is done when
	// the associated module gets loaded.
	if (magma.dkim.key) CONFIG_CHECK_FILE_READABLE(st_char_get(magma.dkim.key));
	if (magma.dime.key) CONFIG_CHECK_FILE_READABLE(st_char_get(magma.dime.key));
	if (magma.dime.signet) CONFIG_CHECK_FILE_READABLE(st_char_get(magma.dime.signet));

	// Validate read access to the directories provided.
	CONFIG_CHECK_DIR_READABLE(magma.system.root_directory);
	CONFIG_CHECK_DIR_READABLE(magma.http.pages);
	CONFIG_CHECK_DIR_READABLE(magma.http.templates);
	CONFIG_CHECK_DIR_READABLE(magma.output.path);
	CONFIG_CHECK_DIR_READWRITE(magma.spool);

	// Finally, are the email addresses good?
	if (magma.admin.contact && !contact_business_valid_email(magma.admin.contact)) {
		log_critical("magma.admin.contact specified invalid email address.");
		result = false;
	}

	if (magma.admin.abuse && !contact_business_valid_email(magma.admin.abuse)) {
		log_critical("magma.admin.abuse specified invalid email address.");
		result = false;
	}

	// If the configuration is valid, dump the magma_keys.
	if (result && magma.config.output_config)
		config_output_settings();

	return result;
}

/**
 * @brief	Set the value of a global config key.
 * @note	This function will also free the value of the global config key if it has already previously been set.
 * @param	setting		a pointer to the global key to have its value adjusted.
 * @param	value		a managed string containing the new key value, or if NULL, the key's default value will be used.
 * @return	true if the specified key's value was set successfully, or false on failure.
 */
bool_t config_value_set(magma_keys_t *setting, stringer_t *value) {

	bool_t result = true;

	/// LOW: Realtime blacklist domains are handled using custom code because we don't yet have a generic type to store lists.
	if (!st_cmp_cs_eq(NULLER(setting->name), PLACER("magma.smtp.blacklist", 20))) {

		// When the default values are assigned an empty string is passed in. Returning false then tricks the system into
		// thinking the default value is wrong, so we just return true to avoid the issue.
		if (st_empty(value)) {
			return true;
		}

		// Were using a fixed array, so if we run out of room we have to reject config.
		if (magma.smtp.blacklists.count >= MAGMA_BLACKLIST_INSTANCES) {
			log_critical("magma.smtp.blacklist is limited to %u %s and the configuration currently contains more than %u %s.",
				MAGMA_BLACKLIST_INSTANCES, MAGMA_BLACKLIST_INSTANCES == 1 ? "domain" : "domains", MAGMA_BLACKLIST_INSTANCES,
				MAGMA_BLACKLIST_INSTANCES == 1 ? "domain" : "domains");
			return false;
		}

		// Make sure the targeted array slot is empty, and the string passed to us has data in it. If so duplicate the value and
		// and store it in the array. If anything fails, return false.
		else if (magma.smtp.blacklists.domain[magma.smtp.blacklists.count] || !(magma.smtp.blacklists.domain[magma.smtp.blacklists.count] =
			st_dupe_opts(MANAGED_T | CONTIGUOUS | HEAP, value))) {
			return false;
		}

		// Track the number of blacklist domains we have.
		magma.smtp.blacklists.count++;

		// Return true so the switch statement below doesn't corrupt the array.
		return true;
	}
	else if (!st_cmp_cs_eq(NULLER(setting->name), NULLER("magma.smtp.bypass_addr"))) {

			// When the default values are assigned an empty string is passed in. Returning false then tricks the system into
			// thinking the default value is wrong, so we just return true to avoid the issue.
			if (st_empty(value)) {
				return true;
			}

			if (!smtp_add_bypass_entry(value)) {
				log_critical("Unable to add smtp bypass entry { entry = %s }", st_char_get(value));
				return false;
			}

			// Return true so the switch statement below doesn't corrupt the array.
			return true;
		}


	switch (setting->norm.type) {

	// Strings
	case (M_TYPE_NULLER):
		if (!ns_empty(*((char **)(setting->store)))) {
			ns_free(*((char **)(setting->store)));
			*((char **)(setting->store)) = NULL;
		}
		if (!st_empty(value))
			*((char **)(setting->store)) = ns_import(st_char_get(value), st_length_get(value));
		else if (!ns_empty(setting->norm.val.ns))
			*((char **)(setting->store)) = ns_dupe(setting->norm.val.ns);
		break;

	case (M_TYPE_STRINGER):
		if (!st_empty(*((stringer_t **)(setting->store)))) {
			st_free(*((stringer_t **)(setting->store)));
			*((stringer_t **)(setting->store)) = NULL;
		}
		if (!st_empty(value))
			*((stringer_t **)(setting->store)) = st_dupe_opts(MANAGED_T | CONTIGUOUS | HEAP, value);
		else if (!st_empty(setting->norm.val.st))
			*((stringer_t **)(setting->store)) = st_dupe_opts(MANAGED_T | CONTIGUOUS | HEAP, setting->norm.val.st);
		break;

		// Booleans
	case (M_TYPE_BOOLEAN):
		if (!st_empty(value)) {
			if (!st_cmp_ci_eq(value, CONSTANT("true")))
				*((bool_t *)(setting->store)) = true;
			else if (!st_cmp_ci_eq(value, CONSTANT("false")))
				*((bool_t *)(setting->store)) = false;
			else {
				log_critical("Invalid value for %s.", setting->name);
				result = false;
			}
		} else
			*((bool_t *)(setting->store)) = setting->norm.val.binary;
		break;

		// Integers
	case (M_TYPE_INT8):
		if (!st_empty(value)) {
			if (!int8_conv_st(value, (int8_t *)(setting->store))) {
				log_critical("Invalid value for %s.", setting->name);
				result = false;
			}
		} else
			*((int8_t *)(setting->store)) = setting->norm.val.i8;
		break;

	case (M_TYPE_INT16):
		if (!st_empty(value)) {
			if (!uint16_conv_st(value, (uint16_t *)(setting->store))) {
				log_critical("Invalid value for %s.", setting->name);
				result = false;
			}
		} else
			*((int16_t *)(setting->store)) = setting->norm.val.u16;
		break;

	case (M_TYPE_INT32):
		if (!st_empty(value)) {
			if (!int32_conv_st(value, (int32_t *)(setting->store))) {
				log_critical("Invalid value for %s.", setting->name);
				result = false;
			}
		} else
			*((int32_t *)(setting->store)) = setting->norm.val.i32;
		break;

	case (M_TYPE_INT64):
		if (!st_empty(value)) {
			if (!int64_conv_st(value, (int64_t *)(setting->store))) {
				log_critical("Invalid value for %s.", setting->name);
				result = false;
			}
		} else
			*((int64_t *)(setting->store)) = setting->norm.val.i64;
		break;

		// Unsigned Integers
	case (M_TYPE_UINT8):
		if (!st_empty(value)) {
			if (!uint8_conv_st(value, (uint8_t *)(setting->store))) {
				log_critical("Invalid value for %s.", setting->name);
				result = false;
			}
		} else
			*((uint8_t *)(setting->store)) = setting->norm.val.u8;
		break;

	case (M_TYPE_UINT16):
		if (!st_empty(value)) {
			if (!uint16_conv_st(value, (uint16_t *)(setting->store))) {
				log_critical("Invalid value for %s.", setting->name);
				result = false;
			}
		} else
			*((uint16_t *)(setting->store)) = setting->norm.val.u16;
		break;

	case (M_TYPE_UINT32):
		if (!st_empty(value)) {
			if (!uint32_conv_st(value, (uint32_t *)(setting->store))) {
				log_critical("Invalid value for %s.", setting->name);
				result = false;
			}
		} else
			*((uint32_t *)(setting->store)) = setting->norm.val.u32;
		break;

	case (M_TYPE_UINT64):
		if (!st_empty(value)) {
			if (!uint64_conv_st(value, (uint64_t *)(setting->store))) {
				log_critical("Invalid value for %s.", setting->name);
				result = false;
			}
		} else
			*((uint64_t *)(setting->store)) = setting->norm.val.u64;
		break;

	default:
		log_critical("The %s setting definition is using an invalid type.", setting->name);
		result = false;
		break;
	}
	return result;
}

/**
 * @brief	Load all the default values for non-required configuration options.
 * @return	true if all default magma config values were successfully loaded, or false on failure.
 */
bool_t config_load_defaults(void) {

	// Run through and setup the default values.
	for (uint64_t i = 0; i < sizeof(magma_keys) / sizeof(magma_keys_t); i++) {
		if (!magma_keys[i].required && !config_value_set(&magma_keys[i], NULL)) {
			log_info("%s has an invalid default value.", magma_keys[i].name);
			config_free();
			return false;
		}
//		else if (magma_keys[i].required && magma_keys[i].norm.type == M_TYPE_BOOLEAN &&
//			magma_keys[i].norm.val.binary == false && !config_value_set(&magma_keys[i], NULL)) {
//			log_info("%s has an invalid default value.", magma_keys[i].name);
//			config_free();
//			return false;
//		}
	}

	return true;
}

/**
 * @brief	Get a magma config key by name.
 * @param	name	a managed string with the name of the magma config option to be looked up.
 * @return	NULL on failure or a pointer to the found magma key object on success.
 */
magma_keys_t * config_key_lookup(stringer_t *name) {

	magma_keys_t *result = NULL;

	for (uint64_t i = 0; !result && name && i < sizeof(magma_keys) / sizeof(magma_keys_t); i++) {
		if (!st_cmp_ci_eq(NULLER(magma_keys[i].name), name)) {
			result = &magma_keys[i];
		}
	}

	return result;
}

/**
 * @brief	Load the magma configuration file specified in magma.config.file, or from the command line.
 * @note	Parses the config file data into a series of name/value pairs, and make sure that for each key:
 *	 			If a config option was loaded from the file, the key must allow it to be configurable via the file, and
 * 				If the key is required, it may not contain an empty value.
 * 			Finally, this function sets the appropriate magma key corresponding to the config key.
 * 			All leftover keys not matched to global magma keys will be configured via servers, relay, and cache server options.
 * @return	true if all config file options were parsed and evaluated successfully, or false on failure.
 */
bool_t config_load_file_settings(void) {

	multi_t name;
	magma_keys_t *key;
	inx_cursor_t *cursor;
	nvp_t *config_pairs = NULL;
	stringer_t *file_data = NULL, *value;

	// Load the config file and convert it into a name/value pair structure.
	if (!(file_data = file_load(magma.config.file))) {
		return false;
	}
	else if (!(config_pairs = nvp_alloc())) {
		st_free(file_data);
		return false;
	}
	else if (nvp_parse(config_pairs, file_data) < 0) {
		nvp_free(config_pairs);
		st_free(file_data);
		return false;
	}
	else if (!(cursor = inx_cursor_alloc(config_pairs->pairs))) {
		nvp_free(config_pairs);
		st_free(file_data);
		return false;
	}

	// Raw file data isn't needed any longer so free.
	st_free(file_data);

	// Run through all of the magma_keys and see if there is a matching name/value pair.
	while (!mt_is_empty(name = inx_cursor_key_next(cursor))) {

		value = inx_cursor_value_active(cursor);

		if ((key = config_key_lookup(name.val.st))) {

			// Make sure the setting can be provided via the configuration file.
			if (!key->file && value) {
					log_critical("%s cannot be changed using the configuration file.", key->name);
					inx_cursor_free(cursor);
					nvp_free(config_pairs);
					return false;
			}

			// Make sure the required magma_keys are not set to NULL.
			else if (key->required && st_empty(value)) {
				log_critical("%s requires a legal value.", key->name);
				inx_cursor_free(cursor);
				nvp_free(config_pairs);
				return false;
			}

			// Attempt to set the value.
			else if (!config_value_set(key, value)) {
				inx_cursor_free(cursor);
				nvp_free(config_pairs);
				return false;
			}

			// If a legit value was provided, then record that we've set this parameter.
			key->set = true;
		}

		// If we haven't had a match yet, check if its a server param.
		else if (name.val.st && !st_cmp_ci_starts(name.val.st, CONSTANT("magma.servers"))) {
			servers_config(name.val.st, value);
		}

		// If we haven't had a match yet, check if its a relay instance.
		else if (name.val.st && !st_cmp_ci_starts(name.val.st, CONSTANT("magma.relay"))) {
			relay_config(name.val.st, value);
		}

		else if (name.val.st && !st_cmp_ci_starts(name.val.st, CONSTANT("magma.iface.cache.host"))) {
			cache_config(name.val.st, value);
		}

		else {
			log_critical("%.*s is not a valid setting.", st_length_int(name.val.st), st_char_get(name.val.st));
			inx_cursor_free(cursor);
			nvp_free(config_pairs);
			return false;
		}
	}

	inx_cursor_free(cursor);
	nvp_free(config_pairs);

	return true;
}

/**
 * @brief	Load all magma configuration options present in the database.
 * @note	Each key/value pair extracted from the database is submitted to the following logic:
 *	 			If a config option was loaded from the database, the key must allow it to be configurable via the database.
 *	 			Check to see that any key that has previously been set is allowed to be overwritten.
 * 				If the key is required, it may not contain an empty value.
 * 			Finally, this function sets the appropriate magma key corresponding to the config key.
 * 			All leftover keys not matched to global magma keys will be configured via servers, relay, and cache server options.
 * @return	true if all database config options were parsed and evaluated successfully, or false on failure.
 */
bool_t config_load_database_settings(void) {

	row_t *row;
	uint64_t rows;
	magma_keys_t *key;
	table_t *database_pairs;
	stringer_t *value, *name;

	if (!(magma.host.number = config_fetch_host_number()) || !(database_pairs = config_fetch_settings())) {
		return false;
	}

	// Loop through each of the row returned.
	rows = res_row_count(database_pairs);
	for (uint64_t i = 0; i < rows && (row = res_row_get(database_pairs, i)); i++) {

			name = PLACER(res_field_block(row, 0), res_field_length(row, 0));
			value = PLACER(res_field_block(row, 1), res_field_length(row, 1));

			if ((key = config_key_lookup(name))) {
				// Make sure the setting can be provided via the database.
				if (!key->database) {
					log_critical("%s cannot be changed using the database.", key->name);
					res_table_free(database_pairs);
					return false;
				}

				// Make sure the setting can be provided via the database.
				else if (key->set && !key->overwrite) {
					log_critical("%s has already been set and cannot be overwritten.", key->name);
					res_table_free(database_pairs);
					return false;
				}

				// Make sure the required magma_keys are not set to NULL.
				else if (key->required && st_empty(value)) {
					log_critical("%s requires a legal value.", key->name);
					res_table_free(database_pairs);
					return false;
				}

				// Attempt to set the value.
				else if (!config_value_set(key, value)) {
					res_table_free(database_pairs);
					return false;
				}

				// Record that we've set this parameter.
				key->set = true;
			}

			// If we haven't had a match yet, check if its a server param.
			else if (!st_cmp_ci_starts(name, CONSTANT("magma.servers"))) {
				servers_config(name, value);
			}

			// If we haven't had a match yet, check if its a relay instance.
			else if (!st_cmp_ci_starts(name, CONSTANT("magma.relay"))) {
				relay_config(name, value);
			}

			else if (!st_cmp_ci_starts(name, CONSTANT("magma.iface.cache.host"))) {
				cache_config(name, value);
			}

			// Otherwise if we still haven't matched a value, and it's not one of the valid keys that aren't stored in the
			// global configuration, we print the error and exit.
			else if (st_cmp_ci_eq(name, CONSTANT("magma.version"))) {
				log_critical("%.*s is not a valid setting.", st_length_int(name), st_char_get(name));
				res_table_free(database_pairs);
				return false;
			}

	}

	res_table_free(database_pairs);
	return true;
}

/**
 * @brief	Load all magma configuration options specified by the user on the command line.
 * @note	Each key/value pair extracted from the database is submitted to the following logic:
 *	 			If a config option was loaded from the database, the key must allow it to be configurable via the database.
 *	 			Check to see that any key that has previously been set is allowed to be overwritten.
 * 				If the key is required, it may not contain an empty value.
 * 			Finally, this function sets the appropriate magma key corresponding to the config key.
 * 			All leftover keys not matched to global magma keys will be configured via servers, relay, and cache server options.
 * @return	true if all database config options were parsed and evaluated successfully, or false on failure.
 */
bool_t config_load_cmdline_settings(void) {
	multi_t name;
	magma_keys_t *key;
	inx_cursor_t *cursor;
	nvp_t *config_pairs = NULL;
	stringer_t *value;

	// If not set, then bail out.
	if (!cmdline_config_data)
		return true;

	// Load the command line options and convert them into a name/value pair structure.
	if (!(config_pairs = nvp_alloc())) {
		st_free(cmdline_config_data);
		return false;
	}
	else if (nvp_parse(config_pairs, cmdline_config_data) < 0) {
		nvp_free(config_pairs);
		st_free(cmdline_config_data);
		return false;
	}
	else if (!(cursor = inx_cursor_alloc(config_pairs->pairs))) {
		nvp_free(config_pairs);
		st_free(cmdline_config_data);
		return false;
	}

	// Our command line config data won't be necessary anymore.
	st_free(cmdline_config_data);

	// Run through all of the magma_keys and see if there is a matching name/value pair.
	while (!mt_is_empty(name = inx_cursor_key_next(cursor))) {

		value = inx_cursor_value_active(cursor);

		if ((key = config_key_lookup(name.val.st))) {

			// Make sure the setting can be provided via the configuration file.
			if (!key->file && value) {
					log_critical("%s cannot be changed using command line option.", key->name);
					inx_cursor_free(cursor);
					nvp_free(config_pairs);
					return false;
			}
			// Make sure the required magma_keys are not set to NULL.
			else if (key->required && st_empty(value)) {
				log_critical("%s requires a legal value.", key->name);
				inx_cursor_free(cursor);
				nvp_free(config_pairs);
				return false;
			}

			// Attempt to set the value.
			else if (!config_value_set(key, value)) {
				inx_cursor_free(cursor);
				nvp_free(config_pairs);
				return false;
			}

			// If a legit value was provided, then record that we've set this parameter.
			key->set = true;
		}

		// If we haven't had a match yet, check if its a server param.
		else if (name.val.st && !st_cmp_ci_starts(name.val.st, CONSTANT("magma.servers"))) {
			servers_config(name.val.st, value);
		}

		// If we haven't had a match yet, check if its a relay instance.
		else if (name.val.st && !st_cmp_ci_starts(name.val.st, CONSTANT("magma.relay"))) {
			relay_config(name.val.st, value);
		}

		else if (name.val.st && !st_cmp_ci_starts(name.val.st, CONSTANT("magma.iface.cache.host"))) {
			cache_config(name.val.st, value);
		}

		else {
			log_critical("%.*s is not a valid setting.", st_length_int(name.val.st), st_char_get(name.val.st));
			inx_cursor_free(cursor);
			nvp_free(config_pairs);
			return false;
		}
	}

	inx_cursor_free(cursor);
	nvp_free(config_pairs);

	return true;
}
