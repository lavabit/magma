
/**
 * @file /magma/providers/database/database.h
 *
 * @brief	Functions used to interface with the database.
 *
 * $Author$
 * $Date$
 * $Revision$
*
*  MySQL Data Types
*
*  MYSQL_TYPE_TINY       for 8-bit integer variables. Normally it's
*                        'signed char' and 'unsigned char';
*  MYSQL_TYPE_SHORT      for 16-bit signed and unsigned variables. This
*                        is usually 'short' and 'unsigned short';
*  MYSQL_TYPE_LONG       for 32-bit signed and unsigned variables. It
*                        corresponds to 'int' and 'unsigned int' on
*                        vast majority of platforms. On IA-32 and some
*                        other 32-bit systems you can also use 'long'
*                        here;
*  MYSQL_TYPE_LONGLONG   64-bit signed or unsigned integer.  Stands for
*                        '[unsigned] long long' on most platforms;
*  MYSQL_TYPE_FLOAT      32-bit floating point type, 'float' on most
*                        systems;
*  MYSQL_TYPE_DOUBLE     64-bit floating point type, 'double' on most
*                        systems;
*  MYSQL_TYPE_TIME       broken-down time stored in MYSQL_TIME
*                        structure
*  MYSQL_TYPE_DATE       date stored in MYSQL_TIME structure
*  MYSQL_TYPE_DATETIME   datetime stored in MYSQL_TIME structure See
*                        more on how to use these types for sending
*                        dates and times below;
*  MYSQL_TYPE_STRING     character string, assumed to be in
*                        character-set-client. If character set of
*                        client is not equal to character set of
*                        column, value for this placeholder will be
*                        converted to destination character set before
*                        insert.
*  MYSQL_TYPE_BLOB       sequence of bytes. This sequence is assumed to
*                        be in binary character set (which is the same
*                        as no particular character set), and is never
*                        converted to any other character set. See also
*                        notes about supplying string/blob length
*                        below.
*  MYSQL_TYPE_NULL       special typecode for binding nulls.
*
 */

#ifndef MAGMA_PROVIDERS_EXTERNAL_DATABASE_H
#define MAGMA_PROVIDERS_EXTERNAL_DATABASE_H

/***
 * @typedef table_t
 *
 * rows = *(uint64_t *)(table);
 * fields = *(uint64_t *)(table + sizeof(uint64_t));
 * cursor = *(uint64_t *)(table + sizeof(uint64_t) + sizeof(uint64_t));
 *
 * row = (table + sizeof(uint64_t) + sizeof(uint64_t) + sizeof(uint64_t) + (number * (sizeof(chr_t *) + (fields * (sizeof(chr_t *) + sizeof(size_t))))));
 */
typedef char table_t;
typedef char row_t;
extern pool_t *sql_pool;

#define ISNULL(b) (my_bool *)&((my_bool){ b })

/// mysql.c
bool_t   lib_load_mysql(void);
const    char * lib_version_mysql();
const    char * serv_charset_mysql(void);
const    char * serv_schema_mysql(void);
const    char * serv_type_mysql(void);
const    char * serv_version_mysql(void);
uint_t   sql_errno(MYSQL *mysql);
const    chr_t * sql_error(MYSQL *mysql);
MYSQL *  sql_open(bool_t silent);
int_t    sql_ping(uint32_t connection);
bool_t   sql_start(void);
void     sql_stop(void);
bool_t   sql_thread_start(void);
void     sql_thread_stop(void);

/// query.c
int64_t sql_query(stringer_t *query);
int64_t sql_query_conn(stringer_t *query, uint32_t connection);

/// results.c
bool_t res_field_bool(row_t *row, uint64_t field);
bool_t res_row_store(uint64_t num, table_t *table, MYSQL_BIND *binding);
double_t res_field_double(row_t *row, uint64_t field);
float_t res_field_float(row_t *row, uint64_t field);
int16_t res_field_int16(row_t *row, uint64_t field);
int32_t res_field_int32(row_t *row, uint64_t field);
int64_t res_field_int64(row_t *row, uint64_t field);
int8_t res_field_int8(row_t *row, uint64_t field);
row_t * res_row_get(table_t *table, uint64_t row);
row_t * res_row_next(table_t *table);
size_t res_field_length(row_t *row, uint64_t field);
stringer_t * res_field_string(row_t *row, uint64_t field);
table_t * res_stmt_store(MYSQL_STMT *stmt);
table_t * res_table_alloc(uint64_t rows, uint64_t fields);
row_t * res_field_generic(row_t *row, uint64_t field, size_t typesize);
uint16_t res_field_uint16(row_t *row, uint64_t field);
uint32_t res_field_uint32(row_t *row, uint64_t field);
uint64_t res_bind_create(MYSQL_STMT *stmt, MYSQL_BIND **result);
uint64_t res_field_count(table_t *table);
uint64_t res_field_uint64(row_t *row, uint64_t field);
uint64_t res_row_count(table_t *table);
uint8_t res_field_uint8(row_t *row, uint64_t field);
void * res_field_block(row_t *row, uint64_t field);
void res_bind_free(MYSQL_STMT *stmt, MYSQL_BIND *binding, uint64_t number);
void res_row_set(row_t *row, chr_t *buffer);
void res_table_free(table_t *table);

/// stmts.c
bool_t        stmt_bind_param(MYSQL_STMT *group, MYSQL_BIND *bind);
void          stmt_close(MYSQL_STMT *local);
uint_t        stmt_errno(MYSQL_STMT *local);
const         chr_t * stmt_error(MYSQL_STMT *local);
bool_t        stmt_exec(MYSQL_STMT **group, MYSQL_BIND *parameters);
uint64_t      stmt_exec_affected(MYSQL_STMT **group, MYSQL_BIND *parameters);
uint64_t      stmt_exec_affected_conn(MYSQL_STMT **group, MYSQL_BIND *parameters, uint32_t connection);
bool_t        stmt_exec_conn(MYSQL_STMT **group, MYSQL_BIND *parameters, uint32_t connection);
table_t *     stmt_get_result(MYSQL_STMT **group, MYSQL_BIND *parameters);
table_t *     stmt_get_result_conn(MYSQL_STMT **group, MYSQL_BIND *parameters, uint32_t connection);
uint64_t      stmt_insert(MYSQL_STMT **group, MYSQL_BIND *parameters);
uint64_t      stmt_insert_conn(MYSQL_STMT **group, MYSQL_BIND *parameters, uint32_t connection);
MYSQL_STMT *  stmt_open(MYSQL *mysql);
bool_t        stmt_prepare(MYSQL_STMT *group, const char *query, unsigned long length);
bool_t        stmt_rebuild(uint32_t connection);
MYSQL_STMT *  stmt_reset(MYSQL_STMT **group, uint32_t connection);
bool_t        stmt_start(void);
void          stmt_stop(void);

/// transaction.c
int64_t tran_commit(int64_t transaction);
int64_t tran_rollback(int64_t transaction);
int64_t tran_start(void);

#endif

