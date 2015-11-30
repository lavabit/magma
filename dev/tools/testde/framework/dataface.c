
#include "framework.h"

// Global Variables.
stringer_t *dataface_version = NULL;
short int *sql_connections_list = NULL;
MYSQL **sql_connections = NULL;
pthread_mutex_t sql_mutex = PTHREAD_MUTEX_INITIALIZER;
extern global_config_t config;

// Functions.
extern void *lavalib;
void (*my_once_free_d)(void) = NULL;
void (*mysql_thread_end_d)(void) = NULL;
int (*mysql_thread_init_d)(void) = NULL;
int (*mysql_thread_safe_d)(void) = NULL;
void (*mysql_close_d)(MYSQL *mysql) = NULL;
MYSQL * (*mysql_init_d)(MYSQL *mysql) = NULL;
my_bool (*mysql_stmt_close_d)(MYSQL_STMT *) = NULL;
const char * (*mysql_error_d)(MYSQL *mysql) = NULL;
int (*mysql_stmt_execute_d)(MYSQL_STMT *stmt) = NULL;
void (*mysql_free_result_d)(MYSQL_RES *result) = NULL;
my_ulonglong (*mysql_insert_id_d)(MYSQL *mysql) = NULL;
my_bool (*mysql_stmt_reset_d)(MYSQL_STMT *stmt) = NULL;
MYSQL_STMT * (*mysql_stmt_init_d)(MYSQL *mysql) = NULL;
unsigned long (*mysql_get_client_version_d)(void) = NULL;
MYSQL_RES * (*mysql_store_result_d)(MYSQL *mysql) = NULL;
MYSQL_ROW (*mysql_fetch_row_d)(MYSQL_RES *result) = NULL;
int (*mysql_stmt_store_result_d)(MYSQL_STMT *stmt) = NULL;
my_ulonglong (*mysql_num_rows_d)(MYSQL_RES *result) = NULL;
my_ulonglong (*mysql_affected_rows_d)(MYSQL *mysql) = NULL;
const char * (*mysql_stmt_error_d)(MYSQL_STMT *stmt) = NULL;
my_bool (*mysql_stmt_free_result_d)(MYSQL_STMT *stmt) = NULL;
my_ulonglong (*mysql_stmt_num_rows_d)(MYSQL_STMT *stmt) = NULL;
my_ulonglong (*mysql_stmt_insert_id_d)(MYSQL_STMT *stmt) = NULL;
my_bool (*mysql_stmt_bind_param_d)(MYSQL_STMT *stmt, MYSQL_BIND *bind) = NULL;
int (*mysql_real_query_d)(MYSQL *mysql, const char *query, unsigned long length) = NULL;
int (*mysql_stmt_prepare_d)(MYSQL_STMT *stmt, const char *query, unsigned long length) = NULL;
unsigned long (*mysql_escape_string_d)(char *to, const char *from, unsigned long length) = NULL;
int (*mysql_stmt_attr_set_d)(MYSQL_STMT *stmt, enum enum_stmt_attr_type option, const void *arg) = NULL;
MYSQL * (*mysql_real_connect_d)(MYSQL *mysql, const char *host, const char *user, const char *passwd, const char *db, unsigned int port, 
	const char *unix_socket, unsigned long client_flag) = NULL;

unsigned int (*mysql_num_fields_d)(MYSQL_RES *result) = NULL;
MYSQL_RES * (*mysql_stmt_result_metadata_d)(MYSQL_STMT *stmt) = NULL;	
int (*mysql_stmt_fetch_d)(MYSQL_STMT *stmt) = NULL;
my_bool (*mysql_stmt_bind_result_d)(MYSQL_STMT *stmt, MYSQL_BIND *bind) = NULL;
MYSQL_FIELD * (*mysql_fetch_field_d)(MYSQL_RES *result) = NULL;

int load_symbols_dataface(void) {
	
	if (lavalib == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The lava library pointer was NULL.");
		#endif
		return 0;
	}

	mysql_thread_safe_d = dlsym(lavalib, "mysql_thread_safe");
	if (mysql_thread_safe_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function mysql_thread_safe.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}
	
	mysql_init_d = dlsym(lavalib, "mysql_init");
	if (mysql_init_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function mysql_init.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}
	
	mysql_close_d = dlsym(lavalib, "mysql_close");
	if (mysql_close == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function mysql_close.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}
	
	mysql_free_result_d = dlsym(lavalib, "mysql_free_result");
	if (mysql_free_result == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function mysql_free_result.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}
	
	mysql_insert_id_d = dlsym(lavalib, "mysql_insert_id");
	if (mysql_insert_id_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function mysql_insert_id.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}
	
	mysql_store_result_d = dlsym(lavalib, "mysql_store_result");
	if (mysql_store_result_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function mysql_store_result.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}
	
	mysql_fetch_row_d = dlsym(lavalib, "mysql_fetch_row");
	if (mysql_fetch_row_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function mysql_fetch_row.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}
	
	mysql_affected_rows_d = dlsym(lavalib, "mysql_affected_rows");
	if (mysql_affected_rows_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function mysql_affected_rows.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}
	
	mysql_real_query_d = dlsym(lavalib, "mysql_real_query");
	if (mysql_real_query == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function mysql_real_query.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}
	
	mysql_escape_string_d = dlsym(lavalib, "mysql_escape_string");
	if (mysql_escape_string_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function mysql_escape_string.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}
	
	mysql_real_connect_d = dlsym(lavalib, "mysql_real_connect");
	if (mysql_real_connect_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function mysql_real_connect.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}
	
	mysql_error_d = dlsym(lavalib, "mysql_error");
	if (mysql_error_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function mysql_error.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}
	
	my_once_free_d = dlsym(lavalib, "my_once_free");
	if (my_once_free_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function my_once_free.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}
	
	mysql_thread_end_d = dlsym(lavalib, "mysql_thread_end");
	if (mysql_thread_end_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function mysql_thread_end.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}
	
	mysql_thread_init_d = dlsym(lavalib, "mysql_thread_init");
	if (mysql_thread_init_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function mysql_thread_init.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}
	
	mysql_num_rows_d = dlsym(lavalib, "mysql_num_rows");
	if (mysql_num_rows_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function mysql_num_rows.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}
	
	mysql_get_client_version_d = dlsym(lavalib, "mysql_get_client_version");
	if (mysql_get_client_version_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function mysql_get_client_version.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}
	
	mysql_stmt_init_d = dlsym(lavalib, "mysql_stmt_init");
	if (mysql_stmt_init_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function mysql_stmt_init.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}
	
	mysql_stmt_prepare_d = dlsym(lavalib, "mysql_stmt_prepare");
	if (mysql_stmt_prepare_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function mysql_stmt_prepare.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}
	
	mysql_stmt_close_d = dlsym(lavalib, "mysql_stmt_close");
	if (mysql_stmt_close_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function mysql_stmt_close.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}
	
	mysql_stmt_reset_d = dlsym(lavalib, "mysql_stmt_reset");
	if (mysql_stmt_reset_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function mysql_stmt_reset.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}
	
	mysql_stmt_bind_param_d = dlsym(lavalib, "mysql_stmt_bind_param");
	if (mysql_stmt_bind_param_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function mysql_stmt_bind_param.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}
	
	mysql_stmt_execute_d = dlsym(lavalib, "mysql_stmt_execute");
	if (mysql_stmt_execute_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function mysql_stmt_execute.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}
	
	mysql_stmt_insert_id_d = dlsym(lavalib, "mysql_stmt_insert_id");
	if (mysql_stmt_insert_id_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function mysql_stmt_insert_id.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}
	
	mysql_stmt_num_rows_d = dlsym(lavalib, "mysql_stmt_num_rows");
	if (mysql_stmt_num_rows_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function mysql_stmt_num_rows.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}
	
	mysql_stmt_store_result_d = dlsym(lavalib, "mysql_stmt_store_result");
	if (mysql_stmt_store_result_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function mysql_stmt_store_result.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}
	
	mysql_stmt_free_result_d = dlsym(lavalib, "mysql_stmt_free_result");
	if (mysql_stmt_free_result_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function mysql_stmt_free_result.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}
	
	mysql_stmt_error_d = dlsym(lavalib, "mysql_stmt_error");
	if (mysql_stmt_error_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function mysql_stmt_error.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}
	
	mysql_stmt_fetch_d = dlsym(lavalib, "mysql_stmt_fetch");
	if (mysql_stmt_fetch_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function mysql_stmt_fetch.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}
	mysql_stmt_bind_result_d = dlsym(lavalib, "mysql_stmt_bind_result");
	if (mysql_stmt_bind_result_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function mysql_stmt_bind_result.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}
	
	mysql_stmt_result_metadata_d = dlsym(lavalib, "mysql_stmt_result_metadata");
	if (mysql_stmt_result_metadata_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function mysql_stmt_result_metadata.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}
	
	mysql_num_fields_d = dlsym(lavalib, "mysql_num_fields");
	if (mysql_num_fields_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function mysql_num_fields.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}
	
	mysql_fetch_field_d = dlsym(lavalib, "mysql_fetch_field");
	if (mysql_fetch_field_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function mysql_fetch_field.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}
	
	mysql_stmt_attr_set_d = dlsym(lavalib, "mysql_stmt_attr_set");
	if (mysql_stmt_attr_set_d == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to establish a pointer to the function mysql_stmt_attr_set.");
		lavalog("%s", dlerror());
		#endif
		return 0;
	}
	
	#ifdef DEBUG_FRAMEWORK
	lavalog("The MySQL library symbols have been loaded.");
	#endif

	return 1;
}

MYSQL_RES * test_res(MYSQL_STMT **stmt, MYSQL_BIND *parameters) {

	int connection;
	MYSQL_STMT *local;
	MYSQL_RES *result;

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
	
	result = mysql_store_result_d(*(sql_connections + connection));
	db_free_conn(connection);
	
	return result;
	
}

stringer_t * version_dataface(void) {
	
	short version;
	short release;
	short major;
	stringer_t *result;
	unsigned long holder;
	
	if (dataface_version != NULL) {
		return dataface_version;
	}
	
	holder = mysql_get_client_version_d();
	if (holder == 0) {
		return NULL;
	}
	
	result = allocate_st(16);
	if (result == NULL) {
		return NULL;
	}
	
	version = holder % 100;
	holder /= 100;
	release = holder % 100;
	holder /= 100;
	major = holder % 100;
	
	sprintf_st(result, "%hi.%hi.%hi", major, release, version);
	dataface_version = result;
	return result;
}

// A front end for the MySQL escaper.
stringer_t * sql_safe_st(const stringer_t *string) {
	
	// Variables.
	sizer_t number;
	stringer_t *safe;
		
	if (string == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in a NULL pointer.");
		#endif
		return NULL;
	}
	
	safe = allocate_st((used_st(string) * 2) + 1);
	if (safe == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not allocate %i bytes for the safe version of the string.", (2 * used_st(string)) + 1);
		#endif
		return NULL;
	}
	
	number = mysql_escape_string_d(data_st(safe), data_st(string), used_st(string));
	if (number == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Recieved back a zero length string from the data escaper.");
		#endif
		free_st(safe);
		return NULL;
	}
	
	set_used_st(safe, number);
	return safe;
	
}

inline MYSQL * connection(int connection) {
	return *(sql_connections + connection);
}

inline MYSQL_STMT * init_stmt(MYSQL *mysql) {
	
	MYSQL_STMT *result;
	result = mysql_stmt_init_d(mysql);
	#ifdef DEBUG_FRAMEWORK
	if (result == NULL) {
		lavalog("Unable to create a prepared statement structure.");
	}
	#endif
	return result;
}

// A front end for the MySQL escaper.
stringer_t * sql_safe_ns(const char *string) {
	
	// Variables.
	sizer_t length;
	sizer_t number;
	stringer_t *safe;
		
	if (string == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in a NULL pointer.");
		#endif
		return NULL;
	}
	
	length = size_ns(string);
	if (length == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in a zero length string.");
		#endif
		return NULL;
	}
	
	safe = allocate_st((length * 2) + 1);
	if (safe == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not allocate %i bytes for the safe version of the string.", (2 * length) + 1);
		#endif
		return NULL;
	}
	
	number = mysql_escape_string_d(data_st(safe), string, length);
	if (number == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Recieved back a zero length string from the data escaper.");
		#endif
		free_st(safe);
		return NULL;
	}
	
	set_used_st(safe, number);
	return safe;
	
}

// A front end for the MySQL escaper.
stringer_t * sql_safe_ns_amt(const char *string, const sizer_t amount) {
	
	// Variables.
	sizer_t number;
	stringer_t *safe;
		
	if (string == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in a NULL pointer.");
		#endif
		return NULL;
	}
	
	if (amount == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed in a zero length string.");
		#endif
		return NULL;
	}
	
	safe = allocate_st((amount * 2) + 1);
	if (safe == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not allocate %i bytes for the safe version of the string.", (2 * amount) + 1);
		#endif
		return NULL;
	}
	
	number = mysql_escape_string_d(data_st(safe), string, amount);
	if (number == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Recieved back a zero length string from the data escaper.");
		#endif
		free_st(safe);
		return NULL;
	}
	
	set_used_st(safe, number);
	return safe;
	
}

void free_dataface_thread(void) {
	
	mysql_thread_end_d();
	
}

int initialize_dataface_thread(void) {
	
	if (mysql_thread_init_d() != 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not initialize the MySQL thread specific variables.");
		#endif
		return 0;
	}
	
	return 1;
}

// This will setup our various connections to the database.
int initialize_dataface(void) {
	
	// Variables.
	int increment;
	MYSQL *connection;
	
	// Lock the connection tables..
	pthread_mutex_lock(&sql_mutex);
	
	sql_connections_list = malloc(sizeof(short int) * config.db.connections);
	if (sql_connections_list == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not allocate %i bytes connections list.", sizeof(short int) * config.db.connections);
		#endif
		pthread_mutex_unlock(&sql_mutex);
		return 0;
	}
	
	sql_connections = malloc(sizeof(MYSQL *) * config.db.connections);
	if (sql_connections == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Could not allocate %i bytes connections list.", sizeof(MYSQL *) * config.db.connections);
		#endif
		free(sql_connections_list);
		pthread_mutex_unlock(&sql_mutex);
		return 0;
	}
	
	clear_bl(sql_connections_list, sizeof(short int) * config.db.connections);
	clear_bl(sql_connections, sizeof(MYSQL *) * config.db.connections);
	
	// Lock all of the connections, just in case.
	for (increment = 0; increment < config.db.connections; increment++) {
		*(sql_connections_list + increment) = 1;
	}
	
	// Make sure the MySQL library we linked against is thread-safe.
	if (mysql_thread_safe_d() == 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("The MySQL library does not appear to be thread-safe.");
		#endif
		free(sql_connections);
		free(sql_connections_list);
		pthread_mutex_unlock(&sql_mutex);
		return 0;
	}
	
	// Setup the Nerdshack MySQL connection pool.
	for (increment = 0; increment < config.db.connections; increment++) {
		
		*(sql_connections + increment) = mysql_init_d(NULL);
		if (*(sql_connections + increment) == NULL) {
			#ifdef DEBUG_FRAMEWORK
			lavalog("Could not initialize the MySQL connection pointer.");
			#endif
			pthread_mutex_unlock(&sql_mutex);
			return 0;
		}
		
		if (config.db.ip != NULL) {
			connection = mysql_real_connect_d(*(sql_connections + increment), config.db.ip, config.db.username, config.db.password, config.db.db, 0, NULL, 0);
		}
		else {
			connection = mysql_real_connect_d(*(sql_connections + increment), config.db.hostname, config.db.username, config.db.password, config.db.db, 0, NULL, 0);
		}
		
		if (connection == NULL) {
			#ifdef DEBUG_FRAMEWORK
			lavalog("MySQL connect error. %s", mysql_error_d(*(sql_connections + increment)));
			#endif
			pthread_mutex_unlock(&sql_mutex);
			return 0;
		}
		
		*(sql_connections_list + increment) = 0;
	}
	
	pthread_mutex_unlock(&sql_mutex);
	
	#ifdef DEBUG_FRAMEWORK
	lavalog("Database initialization complete.");
	#endif
	
	return 1;
}

// Closes the database connections, and cleans up any memory that was used.
void free_dataface(void) {
	
	int increment;
	
	// Lock the connection tables..
	pthread_mutex_lock(&sql_mutex);
	
	// Setup the Nerdshack MySQL connection pool.
	for (increment = 0; increment < config.db.connections; increment++) {
		mysql_close_d(*(sql_connections + increment));
	}
	
	free(sql_connections);
	free(sql_connections_list);
	
	sql_connections = NULL;
	sql_connections_list = NULL;
	
	my_once_free_d();
	mysql_thread_end_d();
	
	pthread_mutex_unlock(&sql_mutex);
	
	if (dataface_version != NULL) {
		free_st(dataface_version);
		dataface_version = NULL;
	}
	
	#ifdef DEBUG_FRAMEWORK
	lavalog("Database shutdown complete.");
	#endif
}

// Will return a number, indicating which connection from the pool to use.
int db_get_conn(void) {
	
	// Variables.
	int increment;
	int connection = -1;
	unsigned int iterations = 0;
	
	// Keep looping util its time to give up, or we have a connection.
	while (connection == -1 && iterations < config.db.iterations) {
	
		// Lock the connection tables..
		pthread_mutex_lock(&sql_mutex);
		
		// Loop through looking for a free connection.
		for (increment = 0; (increment < config.db.connections) && (connection == -1); increment++) {
			if (*(sql_connections_list + increment) == 0) {
				*(sql_connections_list + increment) = 1;
				connection = increment;
			}
		}
		
		// Unlock.
		pthread_mutex_unlock(&sql_mutex);
		
		iterations++;
		
		// If we didn't get a connection, goto sleep before trying again.
		if (connection == -1) {
			usleep(config.db.interval);
		}
	}
	
	// If we still couldn't get a connection, log the error.
	if (connection == -1) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to get free DB connection after iterating %i times, and sleeping %i microseconds.",
			config.db.iterations, config.db.interval);
		#endif
	}

	return connection;
	
}

// Will free the connection in our connection table.
void db_free_conn(int connection) {

	// This marks the connection as free.
	pthread_mutex_lock(&sql_mutex);
	
	*(sql_connections_list + connection) = 0;
	
	pthread_mutex_unlock(&sql_mutex);
	
	return;
	
}

// Starts a transaction on the current connection.
int start_tran(void)  {
	
	int state;
	int connection;
	static const char *transaction_sql = "BEGIN";
	
	// Get a connection.
	connection = db_get_conn();
	if (connection < 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to get an available connection for the transaction.");
		#endif
		return -1;
	}
	
	state = mysql_real_query_d(sql_connections[connection], transaction_sql, 5);
	if (state != 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to start a transaction. %s", mysql_error_d(*(sql_connections + connection)));
		#endif
		db_free_conn(connection);
		return -1;
	}
	
	return connection;
}

// Does not allocate the connection.
int send_begin(int connection) {
	
	int state;
	static const char *transaction_sql = "BEGIN";
	
	if (connection < 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed an invalid MySQL connection number.");
		#endif
		return -1;
	}
	
	state = mysql_real_query_d(sql_connections[connection], transaction_sql, 5);
	if (state != 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to start a transaction. %s", mysql_error_d(*(sql_connections + connection)));
		#endif
		return -1;
	}
	
	return 1;
}

// Commits the transaction and frees the connection.
int commit_tran(int connection) {
	
	int state;
	static const char *commit_sql = "COMMIT";
	
	// Sanity checks.
	if (connection < 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed an invalid MySQL connection number.");
		#endif
		return -1;
	}
	
	state = mysql_real_query_d(*(sql_connections + connection), commit_sql, 6);
	db_free_conn(connection);
	
	if (state != 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to commit a transaction. %s", mysql_error_d(*(sql_connections + connection)));
		#endif
		return 0;
	}
	
	return 1;
}

// Commit the transaction.
int send_commit(int connection) {
	
	int state;
	static const char *commit_sql = "COMMIT";
	
	// Sanity checks.
	if (connection < 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed an invalid MySQL connection number.");
		#endif
		return -1;
	}
	
	state = mysql_real_query_d(*(sql_connections + connection), commit_sql, 6);
	
	if (state != 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to commit a transaction. %s", mysql_error_d(*(sql_connections + connection)));
		#endif
		return 0;
	}
	
	return 1;
}

// Send a rollback command without freeing the connection.
int send_rollback(int connection) {
	
	int state;
	static const char *rollback_sql = "ROLLBACK";
	
	// Sanity checks.
	if (connection < 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed an invalid MySQL connection number.");
		#endif
		return -1;
	}
	
	state = mysql_real_query_d(*(sql_connections + connection), rollback_sql, 8);
	
	if (state != 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to send ROLLBACK. %s", mysql_error_d(*(sql_connections + connection)));
		#endif
		return 0;
	}
	
	return 1;
}

void rollback_tran(int connection) {
	
	int state;
	static const char *rollback_sql = "ROLLBACK";
	
	// Sanity checks.
	if (connection < 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed an invalid MySQL connection number.");
		#endif
		return;
	}
	
	state = mysql_real_query_d(*(sql_connections + connection), rollback_sql, 8);
	
	if (state != 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to send ROLLBACK. %s", mysql_error_d(*(sql_connections + connection)));
		#endif
		return;
	}
	
	db_free_conn(connection);
	
	return;
}

inline void free_res(MYSQL_RES *result) {
	
	if (result == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Asked to free a NULL pointer.");
		#endif
		return;
	}
	
	mysql_free_result_d(result);
	
	return;
}

inline MYSQL_ROW get_row(MYSQL_RES *result) {
	
	if (result == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Asked to return a row from a NULL pointer.");
		#endif
		return (MYSQL_ROW) NULL;
	}
	
	return mysql_fetch_row_d(result);
}

// Functions for executing queries via the connection pool.
int exec_query(const stringer_t *query) {
	
	int state;
	int connection;
	
	#ifdef DEBUG_PRINT_SQL
	lavalog("PRINT SQL: %.*s", used_st(query), data_st(query));
	#endif
	
	connection = db_get_conn();
	if (connection < 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to get an available connection for the query.");
		#endif
		return -1;
	}
	
	state = mysql_real_query_d(*(sql_connections + connection), data_st(query), used_st(query));
	
	#ifdef DEBUG_FRAMEWORK
	if (state != 0) {
		lavalog("An error occurred while executing a query. %s", mysql_error_d(*(sql_connections + connection)));
	}
	#endif
	
	db_free_conn(connection);
	
	return state;
}

int exec_query_tran(const stringer_t *query, const int transaction) {
	
	int state;
	
	#ifdef DEBUG_PRINT_SQL
	lavalog("PRINT SQL: %.*s", used_st(query), data_st(query));
	#endif
	
	if (transaction < 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed an invalid MySQL connection number.");
		#endif
		return -1;
	}
	
	state = mysql_real_query_d(*(sql_connections + transaction), data_st(query), used_st(query));
	
	#ifdef DEBUG_FRAMEWORK
	if (state != 0) {
		lavalog("An error occurred while executing a query. %s", mysql_error_d(*(sql_connections + transaction)));
	}
	#endif
	
	return state;
}

int exec_query_ns(const char *query, const sizer_t length) {
	
	int state;
	int connection;
	
	#ifdef DEBUG_PRINT_SQL
	lavalog("PRINT SQL: %.*s", length, query);
	#endif
	
	connection = db_get_conn();
	if (connection < 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to get an available connection for the query.");
		#endif
		return -1;
	}
	
	state = mysql_real_query_d(*(sql_connections + connection), query, length);
	
	#ifdef DEBUG_FRAMEWORK
	if (state != 0) {
		lavalog("An error occurred while executing a query. %s", mysql_error_d(*(sql_connections + connection)));
	}
	#endif
	
	db_free_conn(connection);
	
	return state;
}

int exec_query_tran_ns(const char *query, const sizer_t length, const int transaction) {
	
	int state;
	
	#ifdef DEBUG_PRINT_SQL
	lavalog("PRINT SQL: %.*s", length, query);
	#endif
	
	if (transaction < 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed an invalid MySQL connection number.");
		#endif
		return -1;
	}
	
	state = mysql_real_query_d(*(sql_connections + transaction), query, length);
	
	#ifdef DEBUG_FRAMEWORK
	if (state != 0) {
		lavalog("An error occurred while executing a query. %s", mysql_error_d(*(sql_connections + transaction)));
	}
	#endif
	
	return state;
}

// Function to execute a query, and only check whether it would return a row.
unsigned long long exec_query_rows(const stringer_t *query) {
	
	int state;
	int connection;
	unsigned long long output;
	MYSQL_RES *result;
	
	#ifdef DEBUG_PRINT_SQL
	lavalog("PRINT SQL: %.*s", used_st(query), data_st(query));
	#endif
	
	connection = db_get_conn();
	if (connection < 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to get an available connection for the query.");
		#endif
		return 0;
	}
	
	state = mysql_real_query_d(*(sql_connections + connection), data_st(query), used_st(query));
	if (state != 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("An error occurred while executing a query. %s", mysql_error_d(*(sql_connections + connection)));
		#endif
		db_free_conn(connection);
		return 0;
	}
	
	result = mysql_store_result_d(*(sql_connections + connection));
	db_free_conn(connection);
	
	if (result == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Recieved a NULL result set.");
		#endif
		return 0;
	}
	
	output = mysql_num_rows_d(result);
	mysql_free_result_d(result);
	
	return output;
}

unsigned long long exec_query_rows_trans(const stringer_t *query, const int transaction) {
	
	int state;
	unsigned long long output;
	MYSQL_RES *result;
	
	#ifdef DEBUG_PRINT_SQL
	lavalog("PRINT SQL: %.*s", used_st(query), data_st(query));
	#endif
	
	if (transaction < 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed an invalid MySQL connection number.");
		#endif
		return 0;
	}
	
	state = mysql_real_query_d(*(sql_connections + transaction), data_st(query), used_st(query));
	if (state != 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("An error occurred while executing a query. %s", mysql_error_d(*(sql_connections + transaction)));
		#endif
		return 0;
	}
	
	result = mysql_store_result_d(*(sql_connections + transaction));
	if (result == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Recieved a NULL result set.");
		#endif
		return 0;
	}
	
	output = mysql_num_rows_d(result);
	mysql_free_result_d(result);
	
	return output;
}

unsigned long long exec_query_rows_ns(const char *query, const sizer_t length) {
	
	int state;
	int connection;
	unsigned long long output;
	MYSQL_RES *result;
	
	#ifdef DEBUG_PRINT_SQL
	lavalog("PRINT SQL: %.*s", length, data_st(query));
	#endif
	
	connection = db_get_conn();
	if (connection < 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to get an available connection for the query.");
		#endif
		return 0;
	}
	
	state = mysql_real_query_d(*(sql_connections + connection), query, length);
	if (state != 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("An error occurred while executing a query. %s", mysql_error_d(*(sql_connections + connection)));
		#endif
		db_free_conn(connection);
		return 0;
	}
	
	result = mysql_store_result_d(*(sql_connections + connection));
	db_free_conn(connection);
	
	if (result == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Recieved a NULL result set.");
		#endif
		return 0;
	}
	
	output = mysql_num_rows_d(result);
	mysql_free_result_d(result);
	
	return output;
}

unsigned long long exec_query_rows_trans_ns(const char *query, const sizer_t length, const int transaction) {
	
	int state;
	unsigned long long output;
	MYSQL_RES *result;
	
	#ifdef DEBUG_PRINT_SQL
	lavalog("PRINT SQL: %.*s", length, query);
	#endif
	
	if (transaction < 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed an invalid MySQL connection number.");
		#endif
		return 0;
	}
	
	state = mysql_real_query_d(*(sql_connections + transaction), query, length);
	if (state != 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("An error occurred while executing a query. %s", mysql_error_d(*(sql_connections + transaction)));
		#endif
		return 0;
	}
	
	result = mysql_store_result_d(*(sql_connections + transaction));
	if (result == NULL) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Recieved a NULL result set.");
		#endif
		return 0;
	}
	
	output = mysql_num_rows_d(result);
	mysql_free_result_d(result);
	
	return output;
}

// Functions for executing queries, and returing the results.
MYSQL_RES *exec_query_res(const stringer_t *query) {
	
	int state;
	int connection;
	MYSQL_RES *result;
	
	#ifdef DEBUG_PRINT_SQL
	lavalog("PRINT SQL: %.*s", used_st(query), data_st(query));
	#endif

	connection = db_get_conn();
	if (connection < 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to get an available connection for the query.");
		#endif
		return (MYSQL_RES *)NULL;
	}
	
	state = mysql_real_query_d(*(sql_connections + connection), data_st(query), used_st(query));
	if (state != 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("An error occurred while executing a query. %s", mysql_error_d(*(sql_connections + connection)));
		#endif
		db_free_conn(connection);
		return (MYSQL_RES *)NULL;
	}
	
	result = mysql_store_result_d(*(sql_connections + connection));
	db_free_conn(connection);
	
	#ifdef DEBUG_FRAMEWORK
	if (result == NULL) {
		lavalog("An error occurred while attempting to save the MySQL result set.");
	}
	#endif
	
	return result;
}

MYSQL_RES *exec_query_res_tran(const stringer_t *query, const int transaction) {
	
	int state;
	MYSQL_RES *result;
	
	#ifdef DEBUG_PRINT_SQL
	lavalog("PRINT SQL: %.*s", used_st(query), data_st(query));
	#endif

	if (transaction < 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed an invalid MySQL connection number.");
		#endif
		return (MYSQL_RES *)NULL;
	}
	
	state = mysql_real_query_d(*(sql_connections + transaction), data_st(query), used_st(query));
	if (state != 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("An error occurred while executing a query. %s", mysql_error_d(*(sql_connections + transaction)));
		#endif
		return (MYSQL_RES *)NULL;
	}
	
	result = mysql_store_result_d(*(sql_connections + transaction));
	
	#ifdef DEBUG_FRAMEWORK
	if (result == NULL) {
		lavalog("An error occurred while attempting to save the MySQL result set.");
	}
	#endif
	
	return result;
}

MYSQL_RES *exec_query_res_ns(const char *query, const sizer_t length) {
	
	int state;
	int connection;
	MYSQL_RES *result;
	
	#ifdef DEBUG_PRINT_SQL
	lavalog("PRINT SQL: %.*s", length, query);
	#endif

	connection = db_get_conn();
	if (connection < 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to get an available connection for the query.");
		#endif
		return (MYSQL_RES *)NULL;
	}
	
	state = mysql_real_query_d(*(sql_connections + connection), query, length);
	if (state != 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("An error occurred while executing a query. %s", mysql_error_d(*(sql_connections + connection)));
		#endif
		db_free_conn(connection);
		return (MYSQL_RES *)NULL;
	}
	
	result = mysql_store_result_d(*(sql_connections + connection));
	db_free_conn(connection);
	
	#ifdef DEBUG_FRAMEWORK
	if (result == NULL) {
		lavalog("An error occurred while attempting to save the MySQL result set.");
	}
	#endif
	
	return result;
}

MYSQL_RES *exec_query_res_tran_ns(const char *query, const sizer_t length, const int transaction) {
	
	int state;
	MYSQL_RES *result;
	
	#ifdef DEBUG_PRINT_SQL
	lavalog("PRINT SQL: %.*s", length, query);
	#endif

	if (transaction < 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed an invalid MySQL connection number.");
		#endif
		return (MYSQL_RES *)NULL;
	}
	
	state = mysql_real_query_d(*(sql_connections + transaction), query, length);
	if (state != 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("An error occurred while executing a query. %s", mysql_error_d(*(sql_connections + transaction)));
		#endif
		return (MYSQL_RES *)NULL;
	}
	
	result = mysql_store_result_d(*(sql_connections + transaction));
	
	#ifdef DEBUG_FRAMEWORK
	if (result == NULL) {
		lavalog("An error occurred while attempting to save the MySQL result set.");
	}
	#endif
	
	return result;
}

// Execute an insert statement, and return the identifier generated.
unsigned long long exec_insert(const stringer_t *query) {
	
	int state;
	int connection;
	unsigned long long insertid;
	
	#ifdef DEBUG_PRINT_SQL
	lavalog("PRINT SQL: %.*s", used_st(query), data_st(query));
	#endif

	connection = db_get_conn();
	if (connection < 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to get an available connection for the query.");
		#endif
		return 0;
	}
	
	state = mysql_real_query_d(*(sql_connections + connection), data_st(query), used_st(query));
	if (state != 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("An error occurred while executing a query. %s", mysql_error_d(*(sql_connections + connection)));
		#endif
		db_free_conn(connection);
		return 0;
	}
	
	insertid = mysql_insert_id_d(*(sql_connections + connection));
	
	db_free_conn(connection);
	
	return insertid;
}

unsigned long long exec_insert_tran(const stringer_t *query, const int transaction) {
	
	int state;
	unsigned long long insertid;
	
	#ifdef DEBUG_PRINT_SQL
	lavalog("PRINT SQL: %.*s", used_st(query), data_st(query));
	#endif

	if (transaction < 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed an invalid MySQL connection number.");
		#endif
		return 0;
	}
	
	state = mysql_real_query_d(*(sql_connections + transaction), data_st(query), used_st(query));
	if (state != 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("An error occurred while executing a query. %s", mysql_error_d(*(sql_connections + transaction)));
		#endif
		return 0;
	}
	
	insertid = mysql_insert_id_d(*(sql_connections + transaction));
	
	return insertid;
}

unsigned long long exec_insert_ns(const char *query, const sizer_t length) {
	
	int state;
	int connection;
	unsigned long long insertid;
	
	#ifdef DEBUG_PRINT_SQL
	lavalog("PRINT SQL: %.*s", length, query);
	#endif

	connection = db_get_conn();
	if (connection < 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to get an available connection for the query.");
		#endif
		return 0;
	}
	
	state = mysql_real_query_d(*(sql_connections + connection), query, length);
	if (state != 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("An error occurred while executing a query. %s", mysql_error_d(*(sql_connections + connection)));
		#endif
		db_free_conn(connection);
		return 0;
	}
	
	insertid = mysql_insert_id_d(*(sql_connections + connection));
	
	db_free_conn(connection);
	
	return insertid;
}

unsigned long long exec_insert_tran_ns(const char *query, const sizer_t length, const int transaction) {
	
	int state;
	unsigned long long insertid;
	
	#ifdef DEBUG_PRINT_SQL
	lavalog("PRINT SQL: %.*s", length, query);
	#endif

	if (transaction < 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed an invalid MySQL connection number.");
		#endif
		return 0;
	}
	
	state = mysql_real_query_d(*(sql_connections + transaction), query, length);
	if (state != 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("An error occurred while executing a query. %s", mysql_error_d(*(sql_connections + transaction)));
		#endif
		return 0;
	}
	
	insertid = mysql_insert_id_d(*(sql_connections + transaction));
	
	return insertid;
}

// Execute a delete or update statement, and return the number of rows effected.
unsigned long long exec_write(const stringer_t *query) {
	
	int state;
	int connection;
	unsigned long long affected;
	
	#ifdef DEBUG_PRINT_SQL
	lavalog("PRINT SQL: %.*s", used_st(query), data_st(query));
	#endif

	connection = db_get_conn();
	if (connection < 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to get an available connection for the query.");
		#endif
		return 0;
	}
	
	state = mysql_real_query_d(*(sql_connections + connection), data_st(query), used_st(query));
	if (state != 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("An error occurred while executing a query. %s", mysql_error_d(*(sql_connections + connection)));
		#endif
		db_free_conn(connection);
		return 0;
	}
	
	affected = mysql_affected_rows_d(*(sql_connections + connection));
	
	db_free_conn(connection);
	
	return affected;
}

unsigned long long exec_write_tran(const stringer_t *query, const int transaction) {
	
	int state;
	unsigned long long affected;
	
	#ifdef DEBUG_PRINT_SQL
	lavalog("PRINT SQL: %.*s", used_st(query), data_st(query));
	#endif

	if (transaction < 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed an invalid MySQL connection number.");
		#endif
		return 0;
	}
	
	state = mysql_real_query_d(*(sql_connections + transaction), data_st(query), used_st(query));
	if (state != 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("An error occurred while executing a query. %s", mysql_error_d(*(sql_connections + transaction)));
		#endif
		return 0;
	}
	
	affected = mysql_affected_rows_d(*(sql_connections + transaction));
	
	return affected;
}

unsigned long long exec_write_ns(const char *query, const sizer_t length) {
	
	int state;
	int connection;
	unsigned long long affected;
	
	#ifdef DEBUG_PRINT_SQL
	lavalog("PRINT SQL: %.*s", length, query);
	#endif

	connection = db_get_conn();
	if (connection < 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Unable to get an available connection for the query.");
		#endif
		return 0;
	}
	
	state = mysql_real_query_d(*(sql_connections + connection), query, length);
	if (state != 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("An error occurred while executing a query. %s", mysql_error_d(*(sql_connections + connection)));
		#endif
		db_free_conn(connection);
		return 0;
	}
	
	affected = mysql_affected_rows_d(*(sql_connections + connection));
	
	db_free_conn(connection);
	
	return affected;
}

unsigned long long exec_write_tran_ns(const char *query, const sizer_t length, const int transaction) {
	
	int state;
	unsigned long long affected;
	
	#ifdef DEBUG_PRINT_SQL
	lavalog("PRINT SQL: %.*s", length, query);
	#endif

	if (transaction < 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("Passed an invalid MySQL connection number.");
		#endif
		return 0;
	}
	
	state = mysql_real_query_d(*(sql_connections + transaction), query, length);
	if (state != 0) {
		#ifdef DEBUG_FRAMEWORK
		lavalog("An error occurred while executing a query. %s", mysql_error_d(*(sql_connections + transaction)));
		#endif
		return 0;
	}
	
	affected = mysql_affected_rows_d(*(sql_connections + transaction));
	
	return affected;
}
