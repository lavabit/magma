
/**
 * @file /magma/providers/database/query.c
 *
 * @brief Traditional SQL queries in string form.
 */

#include "magma.h"

/**
 * @brief	Execute a mysql statement.
 * @see		mysql_real_query()
 * @param	query		the mysql statement to be executed.
 * @param	connection	a connection id for the underlying mysql session.
 * @return	0 on success, or a non-zero number on error.
 */
int64_t sql_query_conn(stringer_t *query, uint32_t connection) {

	int state;

	// QUESTION: should we assume that pool_get_obj() shouldn't return NULL? What about sanity check on query?
	if ((state = mysql_real_query_d(pool_get_obj(sql_pool, connection), st_data_get(query), st_length_get(query)))) {
		log_pedantic("An error occurred while executing a query. { query = %.*s / error = %s }", st_length_int(query), st_char_get(query), sql_error(pool_get_obj(sql_pool, connection)));
	}

	return state;
}

/**
 * @brief	Pull a connection from the sql pool and execute a mysql statement.
 * @see		sql_query_conn()
 * @param	query	the mysql statement to be executed.
 * @return	0 on success, or non-zero on failure.
 */
int64_t sql_query(stringer_t *query) {

	uint64_t result;
	uint32_t connection;

	// QUESTION: Perhaps this -1 return should be differentiated from the "non-zero" error return of mysql_real_query_d()
	if (pool_pull(sql_pool, &connection) != PL_RESERVED) {
			log_info("Unable to get an available connection for the query.");
			return -1;
		}

	result = sql_query_conn(query, connection);
	pool_release(sql_pool, connection);
	return result;
}
