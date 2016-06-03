
/**
 * @file /magma/providers/database/stmts.c
 *
 * @brief	A collection of functions for working with the MySQL prepared statement interface.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

chr_t *queries[] = { QUERIES_INIT };

/**
 * @brief	Get the error number associated with a mysql statement.
 * @param	local	a pointer to the input mysql statement object.
 * @return	the errno associated with the specified mysql statement.
 */
uint_t stmt_errno(MYSQL_STMT *local) {

	if (local && mysql_stmt_errno_d(local)) {
		return mysql_stmt_errno_d(local);
	}
	else if (local && local->mysql && mysql_errno_d(local->mysql)) {
		return mysql_errno_d(local->mysql);
	}

	return 0;
}

/**
 * Return the error string for a mysql statement.
 * @param local A single MYSQL_STMT* parameter.
 * @return This function returns the mysql error string, or NULL if unable to retrieve it.
 */
const chr_t * stmt_error(MYSQL_STMT *local) {

	if (local && mysql_stmt_errno_d(local) && mysql_stmt_error_d(local)) {
		return mysql_stmt_error_d(local);
	}
	else if (local && local->mysql && mysql_errno_d(local->mysql) && mysql_error_d(local->mysql)) {
		return mysql_error_d(local->mysql);
	}

	return NULL;
}

/**
 * @brief	Initialize a mysql prepared statement.
 * @see		mysql_stmt_init()
 * @param	mysql	the underlying mysql connection.
 * @return	This function is a thin wrapper around the mysql library function mysql_stmt_init_d().
 */
MYSQL_STMT * stmt_open(MYSQL *mysql) {

	MYSQL_STMT *result;

	if (!(result = mysql_stmt_init_d(mysql))) {
		log_critical("Unable to create a prepared statement structure. { error = %s }", sql_error(mysql));
	}

	return result;
}

/*
 * @brief	Close a prepared mysql statement.
 * @param	local	the prepared mysql statement to be closed.
 * @return	This function returns no value.
 */
void stmt_close(MYSQL_STMT *local) {

	uint_t err;
	MYSQL *con = NULL;

	// Store the database connection handle.
	if (local && local->mysql) {
		con = local->mysql;
	}

	// If the database connection was closed for inactivity, an error is returned by the close function. Inactivity errors don't need to be logged.
	if (mysql_stmt_close_d(local) && con && (err = sql_errno(con)) && !(err & (CR_SERVER_LOST | CR_SERVER_GONE_ERROR))) {
		log_critical("Unable to close the statement. { error = %s }", sql_error(con));
	}

	return;
}

/*
 * @brief	Close all prepared mysql statements.
 * @return	This function takes no parameters and returns no value.
 */
void stmt_stop(void) {

	MYSQL_STMT **local;

	// Free the prepared statements.
	for (uint32_t i = 0; i < sizeof(queries) / sizeof(char *); i++) {
		if ((local = (MYSQL_STMT **)*((MYSQL_STMT **)&(stmts.select_domains) + i))) {
			for (uint32_t j = 0; j < magma.iface.database.pool.connections; j++) {
				if (*(local + j))
					stmt_close(*(local + j));
			}
			*((MYSQL_STMT **)&(stmts.select_domains) + i) = NULL;
			mm_free(local);
		}
	}

	return;
}

/*
 * @brief	Prepare a mysql statement from a query.
 * @param	group	the prepared mysql statement placeholder.
 * @param	query	the sql query statement as a null-terminated string.
 * @param	length	the length of the query specified as "query."
 * @return	false on failure, true on success.
 */
bool_t stmt_prepare(MYSQL_STMT *group, const char *query, unsigned long length) {

	int state;
	my_bool max_len = 1;
	unsigned long fetch = ULONG_MAX;

	if (mysql_stmt_attr_set_d(group, STMT_ATTR_UPDATE_MAX_LENGTH, &max_len)) {
		log_critical("Could not set the prepared statement option for calculating max length.");
		return false;
	}
	if (mysql_stmt_attr_set_d(group, STMT_ATTR_PREFETCH_ROWS, &fetch)) {
		log_critical("Could not configure the prepared statement to prefetch results.");
		return false;
	}
	if ((state = mysql_stmt_prepare_d(group, query, length))) {
		log_critical("Unable to prepare the SQL statement. { query = %.*s / error = %s }", (int)length, query, stmt_error(group));
		return false;
	}

	return true;
}

/**
 * @brief	Initialize the global array of mysql prepared statements.
 * @note	This function readies a copy of each prepared statement for every member of the global mysql connection pool.
 * @return	true on success or false on failure.
 */
bool_t stmt_start(void) {

	MYSQL_STMT **local;

	// Setup the prepared statement structure.
	if (sizeof(stmts) / sizeof(MYSQL_STMT *) != sizeof(queries) / sizeof(char *)) {
		log_critical("The number of queries doesn't match the number of statements.");
		return false;
	}
	else {
		mm_wipe(&stmts, sizeof(stmts));
	}

	for (uint32_t i = 0; i < sizeof(queries) / sizeof(char *); i++) {

		if (!(local = mm_alloc(magma.iface.database.pool.connections * sizeof(MYSQL_STMT *)))) {
			log_critical("Could not allocate the prepared statement group.");
			return false;
		}

		*((MYSQL_STMT **)&(stmts.select_domains) + i) = (MYSQL_STMT *)local;

		for (uint32_t j = 0; j < magma.iface.database.pool.connections; j++) {

			if (!(*(local + j) = stmt_open(pool_get_obj(sql_pool, j)))) {
				log_critical("Unable to create the prepared statement structure.");
				stmt_stop();
				return false;
			}

			if (!stmt_prepare(*(local + j), queries[i], ns_length_get(queries[i]))) {
				log_critical("Unable to prepare the statement.");
				stmt_stop();
				return false;
			}

		}
	}

	return true;
}

/*
 * @brief	Rebuild all prepared mysql statements on a specified mysql connection.
 * @param	connection	the target mysql connection identifier.
 * @return	false on failure, true on success.
 */
bool_t stmt_rebuild(uint32_t connection) {

	MYSQL_STMT **local;

	for (uint32_t i = 0; i < sizeof(queries) / sizeof(chr_t *); i++) {

		local = ((MYSQL_STMT **)*((MYSQL_STMT **)&(stmts.select_domains) + i));

		if (local) {
			stmt_close(*(local + connection));

			if (!(*(local + connection) = stmt_open(pool_get_obj(sql_pool, connection)))) {
				log_critical("Unable to create the prepared statement structure.");
				return false;
			}
			else if (!stmt_prepare(*(local + connection), queries[i], ns_length_get(queries[i]))) {
				log_critical("Unable to prepare the statement.");
				stmt_close(*(local + connection));
				*(local + connection) = NULL;
				return false;
			}
		}
	}

	return true;
}

/*
 * @brief	Reset the prepared statement on client and server.
 * @param	group		the target prepare mysql statement.
 * @param	connection	the mysql connection identifier.
 */
MYSQL_STMT * stmt_reset(MYSQL_STMT **group, uint32_t connection) {

	// Make sure we have a valid connection, and statement.
	// QUESTION: Where are we verifying the validity of the connection?
	if (!group) {
		log_pedantic("Passed a NULL statement pointer.");
		return NULL;
	}

	// Try preparing the statement.
	if (*(group + connection) == NULL || mysql_stmt_reset_d(*(group + connection))) {

		// If the reset failed, check and if necessary reconnect to the database. Then rebuild all of the prepared statement references associated with the connection.
		if (sql_ping(connection) < 0 || !stmt_rebuild(connection)) {
			log_critical("Unable to reset the statement.");
			return NULL;
		}
		return *(group + connection);
	}

	return *(group + connection);
}

/*
 * @brief	Bind parameters to a prepared mysql statement.
 * @param	group	the input prepared statement.
 * @param	bind	the parameters to be bound to "group."
 * @return	false on failure, or true on success
 */
bool_t stmt_bind_param(MYSQL_STMT *group, MYSQL_BIND *bind) {

	if (!group) {
		log_pedantic("Passed a NULL parameter.");
		return false;
	}

	// A null binding pointer indicates no parameters, which is possible.
	if (!bind)
		return true;

	if (mysql_stmt_bind_param_d(group, bind)) {
		log_critical("We were unable to bind our parameters structure to the prepared statement. { error = %s }", stmt_error(group));
		return false;
	}

	return true;
}

/**
 * @brief	Execute a prepared mysql statement over a specified connection.
 * @note	This function will also reset the prepared statement and bind its parameters.
 * @param	group		the mysql prepared statement to be executed.
 * @param	parameters	the parameters to be bound to the statement.
 * @param	connection	the mysql connection id.
 * @return	false on failure, or true on success.
 */
bool_t stmt_exec_conn(MYSQL_STMT **group, MYSQL_BIND *parameters, uint32_t connection) {

	MYSQL_STMT *local;

	if (!(local = stmt_reset(group, connection))) {
		log_info("Unable to reset the prepared statement.");
		return false;
	}

	if (stmt_bind_param(local, parameters) == false) {
		log_info("Unable to bind the parameters to the prepared statement.");
		return false;
	}

	if (mysql_stmt_execute_d(local)) {
		log_info("An error occurred while executing a prepared statement. { error = %s }", stmt_error(local));
		return false;
	}

	return true;
}

/**
 * @brief	Execute a prepared mysql statement.
 * @see		stmt_exec_conn()
 * @param	group		the mysql prepared statement to be executed.
 * @param	parameters	the parameters to be bound to the statement.
 * @return	false on failure, or true on success.
 */
bool_t stmt_exec(MYSQL_STMT **group, MYSQL_BIND *parameters) {

	bool_t result;
	uint32_t connection;

	// QUESTION: Why aren't we checking for PL_AVAILABLE?
	if (pool_pull(sql_pool, &connection) != PL_RESERVED) {
			log_info("Unable to get an available connection for the query.");
			return 0;
		}

	result = stmt_exec_conn(group, parameters, connection);
	pool_release(sql_pool, connection);
	return result;
}

/**
 * @brief	Execute a prepared mysql statement on a specified connection and return the result.
 * @param	group		the prepared mysql statement to be executed.
 * @param	parameters	the parameters to be passed with the query.
 * @param	connection	the mysql connection identifier.
 * @return	the result of the query, or NULL on failure.
 */
table_t * stmt_get_result_conn(MYSQL_STMT **group, MYSQL_BIND *parameters, uint32_t connection) {

	MYSQL_STMT *local;

	if (!(local = stmt_reset(group, connection))) {
		log_info("Unable to reset the prepared statement.");
		return NULL;
	}

	if (stmt_bind_param(local, parameters) == false) {
		log_info("Unable to bind the parameters to the prepared statement.");
		return NULL;
	}

	if (mysql_stmt_execute_d(local)) {
		log_info("An error occurred while executing a prepared statement. { error = %s }", stmt_error(local));
		return NULL;
	}

	return res_stmt_store(local);
}

/**
 * @brief	Execute a prepared mysql statement and return the result.
 * @param	group		the prepared mysql statement to be executed.
 * @param	parameters	the parameters to be passed with the query.
 * @return	the result of the query, or NULL on failure.
 */
table_t * stmt_get_result(MYSQL_STMT **group, MYSQL_BIND *parameters) {

	void *result;
	uint32_t connection;

	if (pool_pull(sql_pool, &connection) != PL_RESERVED) {
		log_info("Unable to get an available connection for the query.");
		return NULL;
	}

	result = stmt_get_result_conn(group, parameters, connection);
	pool_release(sql_pool, connection);
	return result;
}

/**
 * @brief	Execute a mysql prepared INSERT or UPDATE statement on a specified connection.
 * @see		mysql_stmt_insert_id()
 * @param	group		the mysql prepared statement to be executed.
 * @param	parameters	the parameters to be bound to the prepared statement.
 * @param	connection	the underlying connection on which the statement will be executed.
 * @return	0 on failure, or the last LAST_INSERT_ID() value returned for the mysql auto-increment column.
 */
uint64_t stmt_insert_conn(MYSQL_STMT **group, MYSQL_BIND *parameters, uint32_t connection) {

	MYSQL_STMT *local;

	if (!(local = stmt_reset(group, connection))) {
		log_info("Unable to reset the prepared statement.");
		return 0;
	}

	if (stmt_bind_param(local, parameters) == false) {
		log_info("Unable to bind the parameters to the prepared statement.");
		return 0;
	}

	if (mysql_stmt_execute_d(local)) {
		log_info("An error occurred while executing a prepared statement. { error = %s }", stmt_error(local));
		return 0;
	}

	return mysql_stmt_insert_id_d(local);
}

/**
 * @brief	Execute a mysql prepared INSERT or UPDATE statement.
 * @see		mysql_stmt_insert_id()
 * @param	group		the mysql prepared statement to be executed.
 * @param	parameters	the parameters to be bound to the prepared statement.
 * @return	0 on failure, or the last LAST_INSERT_ID() value returned for the mysql auto-increment column.
 */
uint64_t stmt_insert(MYSQL_STMT **group, MYSQL_BIND *parameters) {

	uint64_t result;
	uint32_t connection;

	if (pool_pull(sql_pool, &connection) != PL_RESERVED) {
			log_info("Unable to get an available connection for the query.");
			return 0;
		}

	result = stmt_insert_conn(group, parameters, connection);
	pool_release(sql_pool, connection);
	return result;
}

/**
 * @brief	Execute a statement on a specified mysql connection and return the number of affected rows.
 * @note	From the MySQL documentation (section 20.9.3.1): "Because mysql_affected_rows() returns an unsigned value, you can check for -1
 * 			by comparing the return value to (my_ulonglong)-1 (or to (my_ulonglong)~0, which is equivalent)."
 * @see		mysql_stmt_affected_rows()
 * @param	group		the prepared mysql statement to be executed.
 * @param	parameters	the parameters to be bound to the prepared statement.
 * @param	connection	the mysql connection id over which the specified statement will be executed.
 * @return	-1 on failure, 0 on failure or no rows affected, and a positive number on success.
*/
// QUESTION: These return types are totally messed up. All errors should return -1.
int64_t stmt_exec_affected_conn(MYSQL_STMT **group, MYSQL_BIND *parameters, uint32_t connection) {

	MYSQL_STMT *local;

	if (!(local = stmt_reset(group, connection))) {
		log_info("Unable to reset the prepared statement.");
		return 0;
	}

	if (stmt_bind_param(local, parameters) == false) {
		log_info("Unable to bind the parameters to the prepared statement.");
		return 0;
	}

	if (mysql_stmt_execute_d(local)) {
		log_info("An error occurred while executing a prepared statement. { error = %s }", stmt_error(local));
		return 0;
	}

	return mysql_stmt_affected_rows_d(local);
}

/**
 * @brief	Execute a statement and return the number of affected rows.
 * @note	From the MySQL documentation (section 20.9.3.1): "Because mysql_affected_rows() returns an unsigned value, you can check for -1
 * 			by comparing the return value to (my_ulonglong)-1 (or to (my_ulonglong)~0, which is equivalent)."
 * @see		mysql_stmt_affected_rows()
 * @param	group		the prepared mysql statement to be executed.
 * @param	parameters	the parameters to be bound to the prepared statement.
 * @return	-1 on failure, 0 on failure or no rows affected, and a positive number on success.
 */
// QUESTION: These return types are totally messed up. All errors should return -1.
int64_t stmt_exec_affected(MYSQL_STMT **group, MYSQL_BIND *parameters) {

	uint64_t result;
	uint32_t connection;

	if (pool_pull(sql_pool, &connection) != PL_RESERVED) {
			log_info("Unable to get an available connection for the query.");
			return 0;
		}

	result = stmt_exec_affected_conn(group, parameters, connection);
	pool_release(sql_pool, connection);
	return result;
}
