
/**
 * @file /magma/providers/database/query.c
 *
 * @brief Functions used to execute tradition SQL query strings.
 */

#include "magma.h"

MYSQL_RES * sql_query_res_conn(stringer_t *query, uint32_t connection) {

	int_t state;
	MYSQL_RES *result;

	if ((state = mysql_real_query_d(pool_get_obj(sql_pool, connection), st_data_get(query), st_length_get(query))) != 0) {
		log_pedantic("An error occurred while executing a query. %s", mysql_error_d(pool_get_obj(sql_pool, connection)));
		return (MYSQL_RES *)NULL;
	}

	if ((result = mysql_store_result_d(pool_get_obj(sql_pool, connection))) == NULL) {
		log_pedantic("An error occurred while attempting to save the MySQL result set.");
	}

	return result;
}

int64_t sql_num_rows_conn(stringer_t *query, uint32_t connection) {

	int_t state;
	int64_t output;
	MYSQL_RES *result;

	if ((state = mysql_real_query_d(pool_get_obj(sql_pool, connection), st_data_get(query), st_length_get(query)))) {
		log_pedantic("An error occurred while executing a query. %s", mysql_error_d(pool_get_obj(sql_pool, connection)));
		return -1;
	}

	if ((result = mysql_store_result_d(pool_get_obj(sql_pool, connection))) == NULL) {
		log_pedantic("Recieved a NULL result set.");
		return -1;
	}

	output = mysql_num_rows_d(result);
	mysql_free_result_d(result);
	return output;
}

int64_t sql_insert_conn(stringer_t *query, uint32_t connection) {

	int_t state;
	int64_t id;

	if ((state = mysql_real_query_d(pool_get_obj(sql_pool, connection), st_data_get(query), st_length_get(query)))) {
		log_pedantic("An error occurred while executing a query. %s", mysql_error_d(pool_get_obj(sql_pool, connection)));
		return -1;
	}

	id = mysql_insert_id_d(pool_get_obj(sql_pool, connection));
	return id;
}

int64_t sql_write_conn(stringer_t *query, uint32_t connection) {

	int_t state;
	int64_t affected;

	if ((state = mysql_real_query_d(pool_get_obj(sql_pool, connection), st_data_get(query), st_length_get(query)))) {
		log_pedantic("An error occurred while executing a query. %s", mysql_error_d(pool_get_obj(sql_pool, connection)));
		return -1;
	}

	affected = mysql_affected_rows_d(pool_get_obj(sql_pool, connection));
	return affected;
}

/**
 * @brief	Execute a mysql statement.
 * @see		mysql_real_query()
 * @param	query		the mysql statement to be executed.
 * @param	connection	a connection id for the underlying mysql session.
 * @return	0 on success, or a non-zero number on error.
 */
int64_t sql_query_conn(stringer_t *query, uint32_t connection) {

	int_t state;

	if ((state = mysql_real_query_d(pool_get_obj(sql_pool, connection), st_data_get(query), st_length_get(query)))) {
		log_pedantic("An error occurred while executing a query. { query = %.*s / error = %s }", st_length_int(query), st_char_get(query), sql_error(pool_get_obj(sql_pool, connection)));
	}

	return state;
}

MYSQL_RES * sql_query_res(stringer_t *query) {

	MYSQL_RES *result;
	uint32_t connection;

	if (pool_pull(sql_pool, &connection) != PL_RESERVED) {
		log_pedantic("Unable to get an available connection for the query.");
		return (MYSQL_RES *)NULL;
	}

	result = sql_query_res_conn(query, connection);
	pool_release(sql_pool, connection);
	return result;
}

int64_t sql_num_rows(stringer_t *query) {

	int64_t output;
	uint32_t connection;

	if (pool_pull(sql_pool, &connection) != PL_RESERVED) {
		log_pedantic("Unable to get an available connection for the query.");
		return -1;
	}

	output = sql_num_rows_conn(query, connection);
	pool_release(sql_pool, connection);
	return output;
}

int64_t sql_insert(stringer_t *query) {

	int64_t id;
	uint32_t connection;

	if (pool_pull(sql_pool, &connection) != PL_RESERVED) {
		log_pedantic("Unable to get an available connection for the query.");
		return -1;
	}

	id = sql_insert_conn(query, connection);
	pool_release(sql_pool, connection);
	return id;
}

int64_t sql_write(stringer_t *query) {

	int64_t affected;
	uint32_t connection;

	if (pool_pull(sql_pool, &connection) != PL_RESERVED) {
		log_pedantic("Unable to get an available connection for the query.");
		return -1;
	}

	affected = sql_write_conn(query, connection);
	pool_release(sql_pool, connection);
	return affected;
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

	if (pool_pull(sql_pool, &connection) != PL_RESERVED) {
			log_info("Unable to get an available connection for the query.");
			return -1;
		}

	result = sql_query_conn(query, connection);
	pool_release(sql_pool, connection);
	return result;
}

