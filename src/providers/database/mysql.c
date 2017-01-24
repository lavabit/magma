
/**
 * @file /magma/providers/database/mysql.c
 *
 * @brief MYSQL Symbols.
 */

/*
 * The list of MySQL collations and character sets. The longest current charset is 8, and the longest collation is 20.
 *
 * Id      Charset          Collation
 * 1       big5             big5_chinese_ci
 * 2       latin2           latin2_czech_cs
 * 3       dec8             dec8_swedish_ci
 * 4       cp850            cp850_general_ci
 * 5       latin1           latin1_german1_ci
 * 6       hp8              hp8_english_ci
 * 7       koi8r            koi8r_general_ci
 * 8       latin1           latin1_swedish_ci
 * 9       latin2           latin2_general_ci
 * 10      swe7             swe7_swedish_ci
 * 11      ascii            ascii_general_ci
 * 12      ujis             ujis_japanese_ci
 * 13      sjis             sjis_japanese_ci
 * 14      cp1251           cp1251_bulgarian_ci
 * 15      latin1           latin1_danish_ci
 * 16      hebrew           hebrew_general_ci
 * 18      tis620           tis620_thai_ci
 * 19      euckr            euckr_korean_ci
 * 20      latin7           latin7_estonian_cs
 * 21      latin2           latin2_hungarian_ci
 * 22      koi8u            koi8u_general_ci
 * 23      cp1251           cp1251_ukrainian_ci
 * 24      gb2312           gb2312_chinese_ci
 * 25      greek            greek_general_ci
 * 26      cp1250           cp1250_general_ci
 * 27      latin2           latin2_croatian_ci
 * 28      gbk              gbk_chinese_ci
 * 29      cp1257           cp1257_lithuanian_ci
 * 30      latin5           latin5_turkish_ci
 * 31      latin1           latin1_german2_ci
 * 32      armscii8         armscii8_general_ci
 * 33      utf8             utf8_general_ci
 * 34      cp1250           cp1250_czech_cs
 * 35      ucs2             ucs2_general_ci
 * 36      cp866            cp866_general_ci
 * 37      keybcs2          keybcs2_general_ci
 * 38      macce            macce_general_ci
 * 39      macroman         macroman_general_ci
 * 40      cp852            cp852_general_ci
 * 41      latin7           latin7_general_ci
 * 42      latin7           latin7_general_cs
 * 43      macce            macce_bin
 * 44      cp1250           cp1250_croatian_ci
 * 47      latin1           latin1_bin
 * 48      latin1           latin1_general_ci
 * 49      latin1           latin1_general_cs
 * 50      cp1251           cp1251_bin
 * 51      cp1251           cp1251_general_ci
 * 52      cp1251           cp1251_general_cs
 * 53      macroman         macroman_bin
 * 57      cp1256           cp1256_general_ci
 * 58      cp1257           cp1257_bin
 * 59      cp1257           cp1257_general_ci
 * 63      binary           binary
 * 64      armscii8         armscii8_bin
 * 65      ascii            ascii_bin
 * 66      cp1250           cp1250_bin
 * 67      cp1256           cp1256_bin
 * 68      cp866            cp866_bin
 * 69      40885            dec8_bin
 * 70      greek            greek_bin
 * 71      hebrew           hebrew_bin
 * 72      hp8              hp8_bin
 * 73      keybcs2          keybcs2_bin
 * 74      koi8r            koi8r_bin
 * 75      koi8u            koi8u_bin
 * 77      latin2           latin2_bin
 * 78      latin5           latin5_bin
 * 79      latin7           latin7_bin
 * 80      cp850            cp850_bin
 * 81      cp852            cp852_bin
 * 82      swe7             swe7_bin
 * 83      utf8             utf8_bin
 * 84      big5             big5_bin
 * 85      euckr            euckr_bin
 * 86      gb2312           gb2312_bin
 * 87      gbk              gbk_bin
 * 88      sjis             sjis_bin
 * 89      tis620           tis620_bin
 * 90      ucs2             ucs2_bin
 * 91      ujis             ujis_bin
 * 92      geostd8          geostd8_general_ci
 * 93      geostd8          geostd8_bin
 * 94      latin1           latin1_spanish_ci
 * 95      cp932            cp932_japanese_ci
 * 96      cp932            cp932_bin
 * 97      eucjpms          eucjpms_japanese_ci
 * 98      eucjpms          eucjpms_bin
 * 99      cp1250           cp1250_polish_ci
 * 128     ucs2             ucs2_unicode_ci
 * 129     ucs2             ucs2_icelandic_ci
 * 130     ucs2             ucs2_latvian_ci
 * 131     ucs2             ucs2_romanian_ci
 * 132     ucs2             ucs2_slovenian_ci
 * 133     ucs2             ucs2_polish_ci
 * 134     ucs2             ucs2_estonian_ci
 * 135     ucs2             ucs2_spanish_ci
 * 136     ucs2             ucs2_swedish_ci
 * 137     ucs2             ucs2_turkish_ci
 * 138     ucs2             ucs2_czech_ci
 * 139     ucs2             ucs2_danish_ci
 * 140     ucs2             ucs2_lithuanian_ci
 * 141     ucs2             ucs2_slovak_ci
 * 142     ucs2             ucs2_spanish2_ci
 * 143     ucs2             ucs2_roman_ci
 * 144     ucs2             ucs2_persian_ci
 * 145     ucs2             ucs2_esperanto_ci
 * 146     ucs2             ucs2_hungarian_ci
 * 192     utf8             utf8_unicode_ci
 * 193     utf8             utf8_icelandic_ci
 * 194     utf8             utf8_latvian_ci
 * 195     utf8             utf8_romanian_ci
 * 196     utf8             utf8_slovenian_ci
 * 197     utf8             utf8_polish_ci
 * 198     utf8             utf8_estonian_ci
 * 199     utf8             utf8_spanish_ci
 * 200     utf8             utf8_swedish_ci
 * 201     utf8             utf8_turkish_ci
 * 202     utf8             utf8_czech_ci
 * 203     utf8             utf8_danish_ci
 * 204     utf8             utf8_lithuanian_ci
 * 205     utf8             utf8_slovak_ci
 * 206     utf8             utf8_spanish2_ci
 * 207     utf8             utf8_roman_ci
 * 208     utf8             utf8_persian_ci
 * 209     utf8             utf8_esperanto_ci
 * 210     utf8             utf8_hungarian_ci
 */

#include "magma.h"

pool_t *sql_pool = NULL;

struct {
	char lib_version[16];
	char serv_version[16];
	char serv_charset[16];
	char serv_schema[32];

	const chr_t *type_serv, *type_embed, *dash;
} sql = {
	.type_serv = "MySQL",
	.type_embed  = "Embedded",
	.dash = "-"
};

/**
 * @brief	Get the last error number for a mysql connection.
 * @param	mysql	a pointer to the MYSQL object of the connection to be queried.
 * @return	0 on failure, or the last mysql error message for the connection on success.
 */
uint_t sql_errno(MYSQL *mysql) {

	if (mysql && mysql_errno_d(mysql)) {
		return mysql_errno_d(mysql);
	}

	return 0;
}

/**
 * @brief	Get a human-readable error message for a mysql connection.
 * @param	mysql	a pointer to the MYSQL object of the connection to be queried.
 * @return	NULL on failure, or a pointer to a null-terminated string with the error message on success.
 */
const chr_t * sql_error(MYSQL *mysql) {
	if (mysql && mysql_errno_d(mysql) && mysql_error_d(mysql)) {
		return mysql_error_d(mysql);
	}

	return NULL;
}

/**
 * @brief	Shutdown and free the pool of mysql connections.
 * @return	This function returns no value.
 */
void sql_stop(void) {

	stmt_stop();

	// Close the SQL connections.
	for (uint32_t i = 0; i < magma.iface.database.pool.connections; i++) {
		mysql_close_d(pool_get_obj(sql_pool, i));
	}

	// Free the pool.
	pool_free(sql_pool);
	sql_pool = NULL;

	// Note that in the 5.0.X branch, mysql_library_end() is redefined as mysql_server_end(),
	// future library versions may require the correct call.
	mysql_server_end_d();
	my_once_free_d();

	return;
}

/**
 * @brief	Open up a new mysql connection to the magma-configured database server.
 * @note	The reconnect option will automatically be set on all new mysql connections.
 * @param	silent	if true, suppress logging of failure messages for this function.
 * @return	NULL on failure, or a pointer to a MSQL objection for the newly established connection on success.
 */
MYSQL * sql_open(bool_t silent) {

	MYSQL *con, *holder;
	my_bool recon = true;

	// From the MySQL 5.1 documentation (section 20.9.3.1): "Because mysql_affected_rows() returns an unsigned value, you can check for -1 by comparing the
	// return value to (my_ulonglong)-1 (or to (my_ulonglong)~0, which is equivalent)." To ensure that remains true, we explicitly test whether the two
	// statements are true.
	if ((my_ulonglong)-1 != (my_ulonglong)~0 || (my_ulonglong)-1 != UINT64_MAX) {
		if (!silent) {

			log_critical("The SQL ulonglong datatype did not pass the equivalency test needed by the SQL provider's error checking logic.");

			if ((my_ulonglong)-1 != (my_ulonglong)~0)
				log_critical("(my_ulonglong - 1 == %llu) != (%llu == my_ulonglong ~ 0)", (my_ulonglong)-1, (my_ulonglong)~0);

			if ((my_ulonglong)-1 != UINT64_MAX)
				log_critical("(my_ulonglong - 1 == %llu) != (%lu == UINT64_MAX)", (my_ulonglong)-1, UINT64_MAX);

		}

		return NULL;
	}

	else if (!(con = mysql_init_d(NULL))) {
		if (!silent) log_critical("Could not initialize the SQL connection structure.");
		return NULL;
	}

  // Set MYSQL_OPT_RECONNECT, because with MySQL 5 on it is set to false by default.
	else if (mysql_options_d(con, MYSQL_OPT_RECONNECT, &recon)) {
		if (!silent) log_critical("MySQL configuration terror. { error = %s }", sql_error(con));
		mysql_close_d(con);
		return NULL;
	}

	else if (!(holder = mysql_real_connect_d(con, magma.iface.database.host, magma.iface.database.user, magma.iface.database.password,
			magma.iface.database.schema, magma.iface.database.port, magma.iface.database.socket_path, 0))) {
		if (!silent) log_critical("MySQL connect error. { error = %s }", sql_error(con));
		mysql_close_d(con);
		return NULL;
	}

	// Prior to version 5.1.6 this option is incorrectly reset to its default value after connecting, so we need to call this function a second time.
	else if (mysql_options_d(con, MYSQL_OPT_RECONNECT, &recon)) {
		if (!silent) log_critical("MySQL configuration terror. { error = %s }", sql_error(con));
		mysql_close_d(con);
		return NULL;
	}

	/*
	 *
	 *
	 *  utf8_bin compare strings by the binary value of each character in the string
	 *  utf8_general_ci sorts by stripping away all accents and sorting as if it were ASCII
   *  utf8_unicode_ci uses the Unicode sort order, so it sorts correctly in more languages
	 *
	 */
//	if (mysql_set_character_set_d(con, "binary")) {
//		log_critical("MySQL configuration error. Unable to set the connection character set to binary. {%s}", mysql_error_d(con));
//		mysql_close_d(con);
//		return NULL;
//	}
//
//	log_pedantic("The MySQL connection is using the character set \"%s\".", mysql_character_set_name_d(con));
	return con;
}

/**
 * @brief	Ping the MySQL server via the provided connection. If the underlying TCP/IP socket has timed out, the ping function will will cause the MySQL client library
 * to reestablish the connection. Because a new connection is created the collection of prepared statements will need to be reinitialized.
 *
 * @warning If the connection timed out, the ping will trigger a reconnect, which invalidates the prepared statement references.
 * @code{.c}
 * if (sql_ping(connection) < 0 || !stmt_rebuild(connection)) { log_error("Invalid database connection."; return; }
 * @endcode
 *
 * @param connection The specific connection inside the pool that should be used for the ping.
 * @return Returns 1 if the library performed an automatic reconnect, 0 if the connection is active, and -1 if the reconnect failed.
 **/
int_t sql_ping(uint32_t connection) {

	uint64_t thread_id;

	// Store the current thread ID.
	thread_id = mysql_thread_id_d(pool_get_obj(sql_pool, connection));

	// Ping the connection.
	if (mysql_ping_d(pool_get_obj(sql_pool, connection))) {
		log_error("MySQL ping failed. Unable to reconnect with the server. { error = %s }", sql_error(pool_get_obj(sql_pool, connection)));
		return -1;
	}

	// And check whether the thread ID has changed.
	else if (mysql_thread_id_d(pool_get_obj(sql_pool, connection)) != thread_id) {
		return 1;
	}

	return 0;
}

/**
 * @brief	Load up the mysql subsystem.
 * @note	This function will check that all necessary database parameters have been supplied, and
 * 			that the supplied mysql library version is thread safe and initialized. It will also initialize
 * 			the pool of database connections, and
 * @return	true on success or false on failure.
 */
bool_t sql_start(void) {

	MYSQL *con;

	if (ns_empty(magma.iface.database.host) || ns_empty(magma.iface.database.user) ||
			ns_empty(magma.iface.database.schema) ||
			magma.iface.database.pool.connections == 0) {
		log_critical("A required MySQL connection parameter is missing or invalid.");
		return false;
	}

	// Initialize the MySQL library. This must be the first call into the library.
	// Note that in the 5.0.X branch, mysql_library_init() is redefined as mysql_server_init(),
	// future library versions may require the former call.
	if (mysql_server_init_d(0, NULL, NULL)) {
		log_critical("Could not initialize the MySQL library.");
		return false;
	}

	// Make sure the MySQL library we linked against is thread-safe.
	if (!mysql_thread_safe_d()) {
		log_critical("The MySQL library does not appear to be thread-safe.");
		return false;
	}

	// Allocate a pool structure for the SQL connections.
	if (!(sql_pool = pool_alloc(magma.iface.database.pool.connections, magma.iface.database.pool.timeout))) {
		log_critical("Could not allocate memory for the SQL connection pool.");
		return false;
	}

	// Loop through and open the required number of connections.
	for (uint32_t i = 0; i < magma.iface.database.pool.connections; i++) {
		if (!(con = sql_open(false))) {
			sql_stop();
			return false;
		}

		pool_set_obj(sql_pool, i, con);
	}

	if (!stmt_start()) {
		sql_stop();
		return false;
	}

	return true;
}

/**
 * @brief	Prepare the thread for exiting to destroy mysql thread specific variables.
 * @note	mysql_thread_end()
 * @return	This function returns no value.
 */
void sql_thread_stop(void) {
	mysql_thread_end_d();
	return;
}

/**
 * @brief	Initialize mysql thread specific variables for the calling thread.
 * @note	mysql_thread_init()
 * @return	0 on success or non-zero on error.
 */
bool_t sql_thread_start(void) {
	if (mysql_thread_init_d() != 0) {
		log_error("Could not initialize the MySQL thread specific variables.");
		return false;
	}

	return true;
}

/**
 * @brief	Determine whether the mysql library in use is embedded or not.
 * @return	sql.type_embed ("Embedded") or sql.type_serv ("MySQL");
 */
const char * serv_type_mysql(void) {

	const chr_t *ret;

	if (mysql_embedded_d()) {
		ret = sql.type_embed;
	}
	else {
		 ret = sql.type_serv;
	}

	return ret;
}

const char * serv_charset_mysql(void) {

	MYSQL *con;

	if (!(con = sql_open(true))) {
		return sql.dash;
	}
	else if (snprintf(sql.serv_charset, 16, "%s", mysql_character_set_name_d(con)) <= 0) {
		mm_wipe(sql.serv_charset, 16);
		return sql.dash;
	}

	mysql_close_d(con);
	return sql.serv_charset;
}

const char * serv_schema_mysql(void) {

	MYSQL *con;

	if (!(con = sql_open(true))) {
		return sql.dash;
	}
	else if (con->db && snprintf(sql.serv_schema, 32, "%s", con->db) <= 0) {
		mm_wipe(sql.serv_schema, 32);
		return sql.dash;
	}

	mysql_close_d(con);
	return sql.serv_schema;
}

/**
 * @brief	Return the server version string of the mysql database.
 * @return	a pointer to a character string containing the mysql server version information.
 */
const char * serv_version_mysql(void) {

	MYSQL *con;

	if (!(con = sql_open(true))) {
		return sql.dash;
	}
	else if (snprintf(sql.serv_version, sizeof(sql.serv_version), "%s", mysql_get_server_info_d(con)) <= 0) {
		mm_wipe(sql.serv_version, sizeof(sql.serv_version));
		return sql.dash;
	}

	mysql_close_d(con);
	return sql.serv_version;
}

/**
 * @brief	Return the version string of libmysql.
 * @return	a pointer to a character string containing the libmysql version information.
 */
const char * lib_version_mysql(void) {

	unsigned long holder;
	short version, release, major;

	if (!(holder = mysql_get_client_version_d())) {
		return NULL;
	}

	version = holder % 100;
	holder /= 100;
	release = holder % 100;
	holder /= 100;
	major = holder % 100;

	snprintf(sql.lib_version, sizeof(sql.lib_version), "%hi.%hi.%hi", major, release, version);
	return sql.lib_version;
}

/**
 * @brief	Initialize the libmysql and bind dynamically to the exported functions that are required.
 * @return	true on success or false on failure.
 */
bool_t lib_load_mysql(void) {

	symbol_t mysql[] = {
		M_BIND(my_once_free), M_BIND(mysql_affected_rows), M_BIND(mysql_character_set_name), M_BIND(mysql_close), M_BIND(mysql_embedded),
		M_BIND(mysql_errno), M_BIND(mysql_error), M_BIND(mysql_escape_string), M_BIND(mysql_fetch_field), M_BIND(mysql_fetch_row),
		M_BIND(mysql_free_result), M_BIND(mysql_get_client_version), M_BIND(mysql_get_server_info),	M_BIND(mysql_init),
		M_BIND(mysql_insert_id), M_BIND(mysql_num_fields), M_BIND(mysql_num_rows), M_BIND(mysql_options), M_BIND(mysql_ping),
		M_BIND(mysql_real_connect),	M_BIND(mysql_real_query), M_BIND(mysql_server_end),	M_BIND(mysql_server_init),
		M_BIND(mysql_set_character_set), M_BIND(mysql_stmt_affected_rows), M_BIND(mysql_stmt_attr_set),	M_BIND(mysql_stmt_bind_param),
		M_BIND(mysql_stmt_bind_result),	M_BIND(mysql_stmt_close), M_BIND(mysql_stmt_errno),	M_BIND(mysql_stmt_error),
		M_BIND(mysql_stmt_execute),	M_BIND(mysql_stmt_fetch), M_BIND(mysql_stmt_free_result), M_BIND(mysql_stmt_init),
		M_BIND(mysql_stmt_insert_id), M_BIND(mysql_stmt_num_rows), M_BIND(mysql_stmt_prepare), M_BIND(mysql_stmt_reset),
		M_BIND(mysql_stmt_result_metadata),	M_BIND(mysql_stmt_store_result), M_BIND(mysql_store_result), M_BIND(mysql_thread_end),
		M_BIND(mysql_thread_id), M_BIND(mysql_thread_init),	M_BIND(mysql_thread_safe)
	};

	if (lib_symbols(sizeof(mysql) / sizeof(symbol_t), mysql) != 1) {
		return false;
	}

	return true;
}
