
/**
 * @file /magma/providers/storage/data.c
 *
 * @brief The meta functions needed by the engine module.
 *
 * $Author$
 * $Date$
 * $Revision$
 *
 */

#include "magma.h"

/**
 * @brief	Insert a tank object into the mysql database.
 * @param	transaction		a mysql connection identifier for which a transaction has been started.
 * @param	hnum			the host number.
 * @param	tnum			the storage tank number.
 * @param	unum			the usernum of the user that owns the object.
 * @param	size			the length, in bytes, of the object to be stored.
 * @param	flags			0 on failure, or the objectnum field for the newly inserted object.
 */
uint64_t tank_insert_object(int64_t transaction, uint64_t hnum, uint64_t tnum, uint64_t unum, uint64_t size, uint64_t flags) {

	MYSQL_BIND parameters[5];
	mm_wipe(parameters, sizeof(parameters));

	// User
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].is_unsigned = true;
	parameters[0].buffer = &unum;

	// Host
	parameters[1].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[1].buffer_length = sizeof(uint64_t);
	parameters[1].is_unsigned = true;
	parameters[1].buffer = &hnum;

	// Tank
	parameters[2].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[2].buffer_length = sizeof(uint64_t);
	parameters[2].is_unsigned = true;
	parameters[2].buffer = &tnum;

	// Size
	parameters[3].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[3].buffer_length = sizeof(uint64_t);
	parameters[3].is_unsigned = true;
	parameters[3].buffer = &size;

	// Flags
	parameters[4].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[4].buffer_length = sizeof(uint64_t);
	parameters[4].is_unsigned = true;
	parameters[4].buffer = &flags;

	return stmt_insert_conn(stmts.insert_object, parameters, transaction);
}

/**
 * @brief	Delete a tank object from the mysql database.
 * @param	transaction		a mysql connection identifier for which a transaction has been started.
 * @param	hnum			the host number.
 * @param	tnum			the storage tank number.
 * @param	unum			the usernum of the user that owns the object.
 * @param	onum			the stored object id.
 * @return	false on failure, or true on success.
 */
// QUESTION: Why do we need to pass all these different parameters? Won't onum alone suffice?
bool_t tank_delete_object(int64_t transaction, uint64_t hnum, uint64_t tnum, uint64_t unum, uint64_t onum) {

	MYSQL_BIND parameters[4];
	mm_wipe(parameters, sizeof(parameters));

	// Object
	parameters[0].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[0].buffer_length = sizeof(uint64_t);
	parameters[0].is_unsigned = true;
	parameters[0].buffer = &onum;

	// Host
	parameters[1].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[1].buffer_length = sizeof(uint64_t);
	parameters[1].is_unsigned = true;
	parameters[1].buffer = &hnum;

	// Tank
	parameters[2].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[2].buffer_length = sizeof(uint64_t);
	parameters[2].is_unsigned = true;
	parameters[2].buffer = &tnum;

	// User
	parameters[3].buffer_type = MYSQL_TYPE_LONGLONG;
	parameters[3].buffer_length = sizeof(uint64_t);
	parameters[3].is_unsigned = true;
	parameters[3].buffer = &unum;

	if (stmt_exec_affected_conn(stmts.delete_object, parameters, transaction) != 1) {
		log_pedantic("Database object reference could not be deleted.");
		return false;
	}

	return true;
}

