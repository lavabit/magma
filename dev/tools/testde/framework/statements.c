
#include "framework.h"

extern global_config_t config;
extern MYSQL **sql_connections;

extern my_bool (*mysql_stmt_close_d)(MYSQL_STMT *);
extern int (*mysql_stmt_execute_d)(MYSQL_STMT *stmt);
extern my_bool (*mysql_stmt_reset_d)(MYSQL_STMT *stmt);
extern MYSQL_STMT * (*mysql_stmt_init_d)(MYSQL *mysql);
extern int (*mysql_stmt_store_result_d)(MYSQL_STMT *stmt);
extern const char * (*mysql_stmt_error_d)(MYSQL_STMT *stmt);
extern my_bool (*mysql_stmt_free_result_d)(MYSQL_STMT *stmt);
extern my_ulonglong (*mysql_stmt_num_rows_d)(MYSQL_STMT *stmt);
extern my_ulonglong (*mysql_stmt_insert_id_d)(MYSQL_STMT *stmt);
extern my_bool (*mysql_stmt_bind_param_d)(MYSQL_STMT *stmt, MYSQL_BIND *bind);
extern int (*mysql_stmt_prepare_d)(MYSQL_STMT *stmt, const char *query, unsigned long length);
extern int (*mysql_stmt_attr_set_d)(MYSQL_STMT *stmt, enum enum_stmt_attr_type option, const void *arg);

inline void free_stmt(MYSQL_STMT **holder) {
	
	int increment;
	
	if (holder != NULL) {
		for (increment = 0; increment < config.db.connections; increment++) {
			if (*(holder + increment) != NULL) {
				close_stmt(*(holder + increment));
			}
		}
		free_bl(holder);
	}
	
	return;
}

inline MYSQL_STMT ** setup_stmt(char *sql) {
	
	int increment;
	MYSQL_STMT **holder;
	
	if (sql == NULL) {
		return NULL;
	}
	
	holder = allocate_bl(config.db.connections * sizeof(MYSQL_STMT *));
	if (holder == NULL) {
		#ifdef DEBUG_COMMON
		lavalog("Could not allocate a holder for the prepared statment pointers.");
		#endif
		return NULL;
	}
	
	for (increment = 0; increment < config.db.connections; increment++) {
		
		*(holder + increment) = init_stmt(connection(increment));
		if (*(holder + increment) == NULL) {
			#ifdef DEBUG_SMTP
			lavalog("Unable to create the prepared statement structure.");
			#endif
			free_stmt(holder);
			return NULL;
		}
		
		if (prepare_stmt(*(holder + increment), sql, size_ns(sql)) != 1) {
			close_stmt(*(holder + increment));
			*(holder + increment) = NULL;
			#ifdef DEBUG_COMMON
			lavalog("Unable to prepare the statement.");
			#endif
			free_stmt(holder);
			return NULL;
		}
	}
	
	return holder;
}

inline int bind_param_stmt(MYSQL_STMT *stmt, MYSQL_BIND *bind) {
	
	#ifdef DEBUG_FRAMEWORK
	if (stmt == NULL || bind == NULL) {
		lavalog("We were passed a NULL pointer.");
		return 0;
	}
	#endif
	
	if (mysql_stmt_bind_param_d(stmt, bind) != 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("We were unable to bind our parameters structure to the prepared statement. %s", mysql_stmt_error_d(stmt));
		#endif
		return 0;
	}
	
	return 1;
}

inline int reset_stmt(MYSQL_STMT *stmt) {
	
	#ifdef DEBUG_FRAMEWORK
	if (stmt == NULL) {
		lavalog("We were passed a NULL statement pointer.");
		return 0;
	}
	#endif
	
	if (mysql_stmt_reset_d(stmt) != 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("We were unable to reset the statement. %s", mysql_stmt_error_d(stmt));
		#endif
		return 0;
	}
	return 1;
}

inline void close_stmt(MYSQL_STMT *stmt) {
	
	#ifdef DEBUG_FRAMEWORK
	if (mysql_stmt_close_d(stmt) != 0) {
		lavalog("Unable to close the statement. %s", mysql_stmt_error_d(stmt));
	}
	#else
	mysql_stmt_close_d(stmt);
	#endif
	
	return;
}

inline int prepare_stmt(MYSQL_STMT *stmt, const char *query, unsigned long length) {
	
	int state;
	my_bool max_len = 1;
	
	if (mysql_stmt_attr_set_d(stmt, STMT_ATTR_UPDATE_MAX_LENGTH, &max_len) != 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not set the prepared statement option for calculating max length.");
		#endif
		return 0;
	}
	
	state = mysql_stmt_prepare_d(stmt, query, length);
	if (state != 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to prepare the SQL statement. %s {%.*s}", mysql_stmt_error_d(stmt), (int)length, query);
		#endif
		return 0;
	}
	
	return 1;
}

unsigned long long exec_insert_stmt(MYSQL_STMT **stmt, MYSQL_BIND *parameters) {
	
	int state;
	int connection;
	MYSQL_STMT *local;
	unsigned long long result;
	
	connection = db_get_conn();
	if (connection < 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to get an available connection for the query.");
		#endif
		return 0;
	}
		
	local = *(stmt + connection);
	if (local == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to get a valid prepared statement pointer.");
		#endif
		db_free_conn(connection);
		return 0;
	}
	
	if (reset_stmt(local) != 1) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to get a reset the prepared statement.");
		#endif
		db_free_conn(connection);
		return 0;
	}
	
	if (bind_param_stmt(local, parameters) != 1) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to bind the parameters to the prepared statement.");
		#endif
		db_free_conn(connection);
		return 0;
	}
	
	state = mysql_stmt_execute_d(local);
	
	if (state != 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("An error occurred while executing a prepared statement. %s", mysql_stmt_error_d(local));
		#endif
		db_free_conn(connection);
		return 0;
	}
	
	result = mysql_stmt_insert_id_d(local);
	#ifdef DEBUG_FRAMEWORK
	if (result == 0) {
		lavalog("We did not get back a valid insert id. %s", mysql_stmt_error_d(local));
	}
	#endif
	
	db_free_conn(connection);
	return result;
}

unsigned long long exec_insert_stmt_tran(MYSQL_STMT **stmt, MYSQL_BIND *parameters, const int transaction) {
	
	int state;
	MYSQL_STMT *local;
	unsigned long long result;
		
	local = *(stmt + transaction);
	if (local == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to get a valid prepared statement pointer.");
		#endif
		return 0;
	}
	
	if (reset_stmt(local) != 1) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to get a reset the prepared statement.");
		#endif
		return 0;
	}
	
	if (bind_param_stmt(local, parameters) != 1) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to bind the parameters to the prepared statement.");
		#endif
		return 0;
	}
	
	state = mysql_stmt_execute_d(local);
	
	if (state != 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("An error occurred while executing a prepared statement. %s", mysql_stmt_error_d(local));
		#endif
		return 0;
	}
	
	result = mysql_stmt_insert_id_d(local);
	#ifdef DEBUG_FRAMEWORK
	if (result == 0) {
		lavalog("We did not get back a valid insert id. %s", mysql_stmt_error_d(local));
	}
	#endif
	
	return result;
}

int exec_query_stmt(MYSQL_STMT **stmt, MYSQL_BIND *parameters) {
	
	int state;
	int connection;
	MYSQL_STMT *local;
	
	connection = db_get_conn();
	if (connection < 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to get an available connection for the query.");
		#endif
		return -1;
	}
	
	local = *(stmt + connection);
	if (local == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to get a valid prepared statement pointer.");
		#endif
		db_free_conn(connection);
		return -1;
	}
	
	if (reset_stmt(local) != 1) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to get a reset the prepared statement.");
		#endif
		db_free_conn(connection);
		return -1;
	}
	
	if (bind_param_stmt(local, parameters) != 1) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to bind the parameters to the prepared statement.");
		#endif
		db_free_conn(connection);
		return -1;
	}
	
	state = mysql_stmt_execute_d(local);
	
	#ifdef DEBUG_FRAMEWORK
	if (state != 0) {
		lavalog("An error occurred while executing a prepared statement. %s", mysql_stmt_error_d(local));
	}
	#endif
	
	db_free_conn(connection);
	return state;
}


int exec_query_stmt_tran(MYSQL_STMT **stmt, MYSQL_BIND *parameters, const int transaction) {
	
	int state;
	MYSQL_STMT *local;
	
	local = *(stmt + transaction);
	if (local == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to get a valid prepared statement pointer.");
		#endif
		return -1;
	}
	
	if (reset_stmt(local) != 1) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to get a reset the prepared statement.");
		#endif
		return -1;
	}
	
	if (bind_param_stmt(local, parameters) != 1) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to bind the parameters to the prepared statement.");
		#endif
		return -1;
	}
	
	state = mysql_stmt_execute_d(local);
	
	#ifdef DEBUG_FRAMEWORK
	if (state != 0) {
		lavalog("An error occurred while executing a prepared statement. %s", mysql_stmt_error_d(local));
	}
	#endif
	
	return state;
}

unsigned long long exec_query_rows_stmt(MYSQL_STMT **stmt, MYSQL_BIND *parameters) {

	int connection;
	MYSQL_STMT *local;
	unsigned long long rows;
	
	connection = db_get_conn();
	if (connection < 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to get an available connection for the query.");
		#endif
		return 0;
	}
	
	local = *(stmt + connection);
	if (local == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to get a valid prepared statement pointer.");
		#endif
		db_free_conn(connection);
		return 0;
	}
	
	if (reset_stmt(local) != 1) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to get a reset the prepared statement.");
		#endif
		db_free_conn(connection);
		return 0;
	}
	
	if (bind_param_stmt(local, parameters) != 1) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to bind the parameters to the prepared statement.");
		#endif
		db_free_conn(connection);
		return 0;
	}
	
	if (mysql_stmt_execute_d(local) != 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("An error occurred while executing a prepared statement. %s", mysql_stmt_error_d(local));
		#endif
		db_free_conn(connection);
		return 0;
	}
	
	if (mysql_stmt_store_result_d(local) != 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("An error occurred while attempting to buffer a prepared statement result set. %s", mysql_stmt_error_d(local));
		#endif
		db_free_conn(connection);
		return 0;
	}
	
	// Store the number of rows.
	rows = mysql_stmt_num_rows_d(local);
	
	#ifdef DEBUG_FRAMEWORK
	if (mysql_stmt_free_result_d(local) != 0) {
		lavalog("An error occurred while attempting to free a prepared statment result buffer. %s", mysql_stmt_error_d(local));
	}
	#else
	mysql_stmt_free_result_d(local);
	#endif
	
	db_free_conn(connection);
	
	return rows;
}

unsigned long long exec_query_rows_stmt_tran(MYSQL_STMT **stmt, MYSQL_BIND *parameters, const int transaction) {
	
	MYSQL_STMT *local;
	unsigned long long rows;
	
	local = *(stmt + transaction);
	if (local == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to get a valid prepared statement pointer.");
		#endif
		return 0;
	}
	
	if (reset_stmt(local) != 1) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to get a reset the prepared statement.");
		#endif
		return 0;
	}
	
	if (bind_param_stmt(local, parameters) != 1) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to bind the parameters to the prepared statement.");
		#endif
		return 0;
	}
	
	if (mysql_stmt_execute_d(local) != 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("An error occurred while executing a prepared statement. %s", mysql_stmt_error_d(local));
		#endif
		return 0;
	}
	
	if (mysql_stmt_store_result_d(local) != 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("An error occurred while attempting to buffer a prepared statement result set. %s", mysql_stmt_error_d(local));
		#endif
		return 0;
	}
	
	// Store the number of rows.
	rows = mysql_stmt_num_rows_d(local);
	
	#ifdef DEBUG_FRAMEWORK
	if (mysql_stmt_free_result_d(local) != 0) {
		lavalog("An error occurred while attempting to free a prepared statment result buffer. %s", mysql_stmt_error_d(local));
	}
	#else
	mysql_stmt_free_result_d(local);
	#endif
	
	return rows;
}

sql_result_t * exec_query_res_stmt(MYSQL_STMT **stmt, MYSQL_BIND *parameters) {
	
	int connection;
	MYSQL_STMT *local;
	sql_result_t *result;
	
	connection = db_get_conn();
	if (connection < 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to get an available connection for the query.");
		#endif
		return NULL;
	}
	
	local = *(stmt + connection);
	if (local == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to get a valid prepared statement pointer.");
		#endif
		db_free_conn(connection);
		return NULL;
	}
	
	if (reset_stmt(local) != 1) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to get a reset the prepared statement.");
		#endif
		db_free_conn(connection);
		return NULL;
	}
	
	if (bind_param_stmt(local, parameters) != 1) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to bind the parameters to the prepared statement.");
		#endif
		db_free_conn(connection);
		return NULL;
	}
	
	if (mysql_stmt_execute_d(local) != 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("An error occurred while executing a prepared statement. %s", mysql_stmt_error_d(local));
		#endif
		db_free_conn(connection);
		return NULL;
	}
	
	result = store_db_result(local);
	if (result == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to store the database result.");
		#endif
		db_free_conn(connection);
		return NULL;
	}
	
	db_free_conn(connection);
	return result;
}

sql_result_t * exec_query_res_tran_stmt(MYSQL_STMT **stmt, MYSQL_BIND *parameters, const int transaction) {

	MYSQL_STMT *local;
	sql_result_t *result;
	
	local = *(stmt + transaction);
	if (local == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to get a valid prepared statement pointer.");
		#endif
		return NULL;
	}
	
	if (reset_stmt(local) != 1) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to get a reset the prepared statement.");
		#endif
		return NULL;
	}
	
	if (bind_param_stmt(local, parameters) != 1) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to bind the parameters to the prepared statement.");
		#endif
		return NULL;
	}
	
	if (mysql_stmt_execute_d(local) != 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("An error occurred while executing a prepared statement. %s", mysql_stmt_error_d(local));
		#endif
		return NULL;
	}
	
	result = store_db_result(local);
	if (result == NULL) {
		return NULL;
	}
	
	return result;
}

unsigned long long exec_write_stmt(MYSQL_STMT **stmt, MYSQL_BIND *parameters);
unsigned long long exec_write_stmt_tran(MYSQL_STMT **stmt, MYSQL_BIND *parameters, const int transaction);
