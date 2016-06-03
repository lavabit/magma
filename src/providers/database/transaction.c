
/**
 * @file /magma/providers/database/transaction.c
 *
 * @brief MySQL transaction interface.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/// LOW: Standardize how SQL connection and transaction handles are passed around. Some functions use int64_t, others use
/// uint32_t. Perhaps we need to use a type def? Or perhaps create a similar type mapping for pool object handles?

struct {
	chr_t *command;
	size_t length;
} tran_commands[3] = {
		{ .command = "START TRANSACTION", .length = 17 },
		{ .command = "ROLLBACK", .length = 8 },
		{ .command = "COMMIT", .length = 6 }
};

/**
 * @brief	Pull a connection from the sql pool and start a transaction.
 * @return	-1 on failure, or a mysql connection id from the sql pool on success.
 */
int64_t tran_start(void) {

	int64_t result;
	uint32_t transaction;

	// QUESTION: Why aren't we using sql_query() for this whole process?
	if (pool_pull(sql_pool, &transaction) != PL_RESERVED) {
		log_info("Unable to get an available connection for the query.");
		return -1;
	}

	if ((result = sql_query_conn(PLACER(tran_commands[0].command, tran_commands[0].length), transaction))) {
		log_info("An error occurred while starting a transaction. { mysql_real_query = %li / error = %s}", result, sql_error(pool_get_obj(sql_pool, transaction)));
		pool_release(sql_pool, transaction);
		return -1;
	}

	return transaction;
}

/**
 * @brief	Rollback a pending transaction in the mysql database.
 * @param	transaction		the mysql connection identifier.
 * @return	0 on success, or a non-zero number on error.
 */
// QUESTION: Why is transaction passed as a 64bit and not 32bit id?
int64_t tran_rollback(int64_t transaction)  {

	int64_t result;

	if ((result = sql_query_conn(PLACER(tran_commands[1].command, tran_commands[1].length), transaction))) {
		log_info("An error occurred while committing the transaction. { mysql_real_query = %li / error = %s }", result, sql_error(pool_get_obj(sql_pool, transaction)));
		pool_release(sql_pool, transaction);
		return result;
	}

	pool_release(sql_pool, transaction);
	return result;
}

/**
 * @brief	Commit a transaction to the mysql database.
 * @param	transaction		the mysql connection identifier.
 * @return	0 on success, or a non-zero number on error.
 */
 // QUESTION: Why is the transaction 64 bit and not 32?
int64_t tran_commit(int64_t transaction)  {

	int64_t result;

	if ((result = sql_query_conn(PLACER(tran_commands[2].command, tran_commands[2].length), transaction))) {
		log_info("An error occurred while committing the transaction. { mysql_real_query = %li / error = %s }", result, sql_error(pool_get_obj(sql_pool, transaction)));
		pool_release(sql_pool, transaction);
		return result;
	}

	pool_release(sql_pool, transaction);
	return result;
}

