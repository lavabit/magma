
/**
 * @file /magma/engine/config/global/datatier.c
 *
 * @brief	Database interface for the engine module.
 */

#include "magma.h"

/**
 * @brief	Retrieve this computer's host number from the database.
 * @return	this host's numerical identifier, or 0 on failure.
 */
uint64_t config_fetch_host_number(void) {

	row_t *row;
	table_t *result;
	uint64_t output = 0;
	MYSQL_BIND parameters[1];
	mm_wipe(parameters, sizeof(parameters));

	// Host
	parameters[0].buffer_type = MYSQL_TYPE_STRING;
	parameters[0].buffer_length = ns_length_get(magma.host.name);
	parameters[0].buffer = magma.host.name;

	if (!(result = stmt_get_result(stmts.select_host_number, parameters)) || !(row = res_row_next(result)) || !(output = res_field_uint64(row, 0))) {
		log_error("The host entry could not be found. { host = %s }", magma.host.name);
	}

	if (result) res_table_free(result);
	return output;
}

/**
 * @brief	Retrieve the entire collection of configuration key/value pairs from the database.
 * @param	none	This function accepts no parameters.
 * @return	NULL on failure, or a database table of configuration key/value pairs on success.
 */
table_t * config_fetch_settings(void) {

	MYSQL_BIND parameters[1];
	mm_wipe(parameters, sizeof(parameters));

	// Host
	parameters[0].buffer_type = MYSQL_TYPE_STRING;
	parameters[0].buffer_length = ns_length_get(magma.host.name);
	parameters[0].buffer = magma.host.name;

	return stmt_get_result(stmts.select_config, parameters);
}

