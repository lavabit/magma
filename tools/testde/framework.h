
#ifndef __LAVABIT_FRAMEWORK_H__
#define __LAVABIT_FRAMEWORK_H__

#define REENTRANT
//#define DEBUG_FRAMEWORK
#define FRAMEWORK_VERSION "1.1.9"
//#define DEBUG_PRINT_SQL

#define CONFIG_LOG_METHOD_OUTPUT 0
#define CONFIG_LOG_METHOD_FILE 1
#define CONFIG_LOG_METHOD_SYSLOG 2

// LibC
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <pthread.h>
#include <dlfcn.h>
#include <unistd.h>
#include <fcntl.h>
#include <semaphore.h>
#include <netdb.h>
#include <signal.h>
#include <execinfo.h>
#include <errno.h>
#include <resolv.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <sys/resource.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <arpa/nameser.h>

// MySQL
#include <mysql.h>

// Open SSL
#include <ssl.h>

// SPF
//#include <spf.h>

// Libxml2
#include <xmlmemory.h>
#include <tree.h>
#include <valid.h>
#include <xpath.h>
#include <xpathInternals.h>
#include <parserInternals.h>
#include <xmlerror.h>

// ClamAV
#include <clamav.h>

// LZO
#include <lzoconf.h>
#include <lzo1x.h>

// The stringer data types.
typedef ssize_t sizer_t;
typedef char stringer_t;
typedef char reducer_t;

// The placer data type is NOT PORTABLE.
typedef struct {
	long size;
	long data;
} placer_t;

// The SQL result data types.
typedef char sql_row_t;
typedef char sql_result_t;

// Structures.
#include "structs.h"

// Logging.
#define lavalog(...) lavalog_i(__FILE__, __FUNCTION__, __LINE__, __VA_ARGS__); // TESTED
void lavalog_i(const char *file, const char *function, const int line, const char *format, ...) __attribute__((format(printf, 4, 5))); // TESTED

// For monitoring whether the process should continue.
int continue_processing(int value);

// Run unit tests. If the framework passes, one is returned.
int run_unit_tests(void); // TESTED

// The placer functions.
extern inline sizer_t size_pl(placer_t placer);
extern inline unsigned char * data_pl(placer_t placer);
extern inline placer_t set_pl(char *pointer, sizer_t length);

// Allocation.
char * allocate_ns(const sizer_t size); // TESTED
char * reallocate_ns(char *string, const sizer_t size); // TESTED
stringer_t * allocate_st(const sizer_t size); // TESTED
stringer_t * reallocate_st(stringer_t *string, const sizer_t size); // TESTED
void * allocate_bl(const sizer_t size); // TESTED
void * reallocate_bl(void *buffer, const sizer_t new_size, const sizer_t orig_size); // TESTED

// Deallocation.
extern inline void free_ns(char *string); // TESTED
extern inline void free_st(stringer_t *string); // TESTED
extern inline void free_bl(void *buffer); // TESTED

// Clear a block of memory.
extern inline void clear_st(stringer_t *string); // DONE
extern inline void clear_bl(void *buffer, const sizer_t size); // TESTED

// Move bytes around.
extern inline void move_bytes(char *start, char *from, int length); // DONE

// Return the size of strings.
extern inline sizer_t size_ns(const unsigned char *string); // TESTED
extern inline sizer_t size_st(const stringer_t *string); // TESTED
extern inline sizer_t used_st(const stringer_t *string); // TESTED
extern inline void set_used_st(stringer_t *string, const sizer_t used); // TESTED

// Handle stringers.
extern inline unsigned char * data_st(const stringer_t *string); // TESTED

// Importing null strings.
stringer_t * import_ns(const char *string); // TESTED
stringer_t * import_bl(const void *block, const sizer_t size); // TESTED

// Exporting null strings.
sizer_t export_ns(char **target, const stringer_t *string); // TESTED.

// Duplicating strings.
char * duplicate_ns(const char *string); // TESTED
char * duplicate_ns_amt(const char *string, sizer_t amount); // TESTED
stringer_t * duplicate_st(const stringer_t *string); // TESTED

// Merge strings.
stringer_t * merge_strings(const char *format, ...); // TESTED

// Appends a stringer onto a another stringer. Allocating as necessary.
stringer_t * append_st_st(stringer_t *left, stringer_t *right); // DONE
stringer_t * append_st_ns(stringer_t *left, char *right); // DONE

// Print into a stringer.
sizer_t sprintf_st(stringer_t *string, const char *format, ...) __attribute__((format(printf, 2, 3))); // TESTED

// Identical strings. Entire string must match.
int identical_st_ns(const stringer_t *left, const char *right); // TESTED
int identical_st_st(const stringer_t *left, const stringer_t *right); // TESTED
int identical_ns_ns(const char *left, const char *right); // TESTED
int identical_st_st_case(const stringer_t *left, const stringer_t *right); // TESTED
int identical_st_ns_case(const stringer_t *left, const char *right); // TESTED
int identical_ns_ns_case(const char *left, const char *right); // TESTED

// Check to see if the left string starts with the right string.
int starts_st_ns(const stringer_t *left, const char *right); // TESTED
int starts_ns_st(const char *left, const stringer_t *right); // TESTED
int starts_st_st(const stringer_t *left, const stringer_t *right); // TESTED
int starts_ns_ns(const char *left, const char *right); // TESTED
int starts_st_ns_case(const stringer_t *left, const char *right); // TESTED
int starts_ns_st_case(const char *left, const stringer_t *right); // TESTED
int starts_st_st_case(const stringer_t *left, const stringer_t *right); // TESTED
int starts_ns_ns_case(const char *left, const char *right); // TESTED
int starts_st_ns_amt(const stringer_t *left, const char *right, const sizer_t amount); // TESTED
int starts_ns_st_amt(const char *left, const stringer_t *right, const sizer_t amount); // TESTED
int starts_st_st_amt(const stringer_t *left, const stringer_t *right, const sizer_t amount); // TESTED
int starts_ns_ns_amt(const char *left, const char *right, const sizer_t amount); // TESTED
int starts_st_ns_case_amt(const stringer_t *left, const char *right, const sizer_t amount); // TESTED
int starts_ns_st_case_amt(const char *left, const stringer_t *right, const sizer_t amount); // TESTED
int starts_st_st_case_amt(const stringer_t *left, const stringer_t *right, const sizer_t amount); // TESTED
int starts_ns_ns_case_amt(const char *left, const  char *right, const sizer_t amount); // TESTED

// Search strings. Returns the location of the string.
sizer_t search_st_ns(const stringer_t *haystack, const char *needle); // TESTED
sizer_t search_st_st(const stringer_t *haystack, const stringer_t *needle); // TESTED
sizer_t search_ns_ns(const char *haystack, const char *needle); // TESTED
sizer_t search_ns_st(const char *haystack, const stringer_t *needle); // TESTED
sizer_t search_st_ns_amt(const stringer_t *haystack, sizer_t amount, const char *needle); // TESTED
sizer_t search_st_st_amt(const stringer_t *haystack, sizer_t amount, const stringer_t *needle); // TESTED
sizer_t search_ns_ns_amt(const char *haystack, sizer_t amount, const char *needle); // TESTED
sizer_t search_ns_st_amt(const char *haystack, sizer_t amount, const stringer_t *needle); // TESTED
sizer_t search_st_ns_case(const stringer_t *haystack, const char *needle); // TESTED
sizer_t search_st_st_case(const stringer_t *haystack, const stringer_t *needle); // TESTED
sizer_t search_ns_ns_case(const char *haystack, const char *needle); // TESTED
sizer_t search_ns_st_case(const char *haystack, const stringer_t *needle); // TESTED
sizer_t search_st_ns_case_amt(const stringer_t *haystack, sizer_t amount, const char *needle); // TESTED
sizer_t search_st_st_case_amt(const stringer_t *haystack, sizer_t amount, const stringer_t *needle); // TESTED
sizer_t search_ns_ns_case_amt(const char *haystack, sizer_t amount, const char *needle); // TESTED
sizer_t search_ns_st_case_amt(const char *haystack, sizer_t amount, const stringer_t *needle); // TESTED

// Copying strings.
int copy_st_st(stringer_t *target, const stringer_t *source); // TESTED
int copy_st_st_amt(stringer_t *target, const stringer_t *source, const sizer_t amount); // TESTED
int copy_st_ns(stringer_t *target, const char *source); // TESTED
int copy_st_ns_amt(stringer_t *target, const char *source, const sizer_t amount); // TESTED
int copy_ns_st(char *target, const stringer_t *source); // TESTED
int copy_ns_st_amt(char *target, const stringer_t *source, const sizer_t amount); // TESTED
int copy_ns_ns(char *target, const stringer_t *source); // TESTED
int copy_ns_ns_amt(char *target, const stringer_t *source, const sizer_t amount); // TESTED

// Functions for lowercasing a string.
extern inline void lowercase_st(stringer_t *string); // TESTED
extern inline void lowercase_ns(char *string, sizer_t length); // TESTED
extern inline char lowercase_c(char character); // TESTED

// Functions for uppercasing a string.
extern inline void uppercase_st(stringer_t *string); // TESTED
extern inline void uppercase_ns(char *string, sizer_t length); // TESTED
extern inline char uppercase_c(char character); // TESTED

// Encoding and decoding base64 strings.
stringer_t * encode_base64_st(const stringer_t *string); // TESTED
stringer_t * decode_base64_st(const stringer_t *string); // TESTED
stringer_t * encode_base64_ns(const char *string); // TESTED
stringer_t * decode_base64_ns(const char *string); // TESTED
stringer_t * encode_base64_st_amt(const stringer_t *string, sizer_t amount); // TESTED
stringer_t * decode_base64_st_amt(const stringer_t *string, sizer_t amount); // TESTED
stringer_t * encode_base64_ns_amt(const char *string, sizer_t amount); // TESTED
stringer_t * decode_base64_ns_amt(const char *string, sizer_t amount); // TESTED

// Encoding and decoding quoted-printable strings.
stringer_t * encode_qp_st(const stringer_t *string); // TESTED
stringer_t * decode_qp_st(const stringer_t *string); // TESTED
stringer_t * encode_qp_ns(const char *string); // TESTED
stringer_t * decode_qp_ns(const char *string); // TESTED
stringer_t * encode_qp_st_amt(const stringer_t *string, const sizer_t amount); // TESTED
stringer_t * decode_qp_st_amt(const stringer_t *string, const sizer_t amount); // TESTED
stringer_t * encode_qp_ns_amt(const char *string, const sizer_t amount); // TESTED
stringer_t * decode_qp_ns_amt(const char *string, const sizer_t amount); // TESTED
/*
// UU encode and decode functions.
stringer_t * encode_uu_st(const stringer_t *string);
stringer_t * decode_uu_st(const stringer_t *string);
stringer_t * encode_uu_ns(const char *string);
stringer_t * decode_uu_ns(const char *string);
stringer_t * encode_uu_st_amt(const stringer_t *string, sizer_t amount);
stringer_t * decode_uu_st_amt(const stringer_t *string, sizer_t amount);
stringer_t * encode_uu_ns_amt(const char *string, sizer_t amount);
stringer_t * decode_uu_ns_amt(const char *string, sizer_t amount);
*/
// Generate the hex code for a character. Encode returns a pointer to a static array.
extern inline char * encode_hex_c(char input, char *output); // TESTED
extern inline char decode_hex_c(char *input); // TESTED

// Replace strings inside a stringer. Returns the number of replacements.
int replace_st_ns_ns(stringer_t **target, const char *pattern, const char *replacement); // TESTED
int replace_st_ns_st(stringer_t **target, const char *pattern, const stringer_t *replacement); // TESTED
int replace_st_st_ns(stringer_t **target, stringer_t *pattern, const char *replacement); // TESTED
int replace_st_st_st(stringer_t **target, const stringer_t *pattern, const stringer_t *replacement); // TESTED
int replace_ns_ns_ns(char **target, const char *pattern, const char *replacement); // TESTED
int replace_ns_ns_st(char **target, const char *pattern, const stringer_t *replacement); // TESTED
int replace_ns_st_ns(char **target, const stringer_t *pattern, const char *replacement); // TESTED
int replace_ns_st_st(char **target, const stringer_t *pattern, const stringer_t *replacement); // TESTED

// Removal. Returns the number of instances removed.
int remove_st_ns(stringer_t **target, const char *pattern); // TESTED
int remove_st_st(stringer_t **target, const stringer_t *pattern); // TESTED
int remove_ns_ns(char **target, const char *pattern); // TESTED
int remove_ns_st(char **target, const stringer_t *pattern); // TESTED

// Convert strings to numbers.
int extract_ull(stringer_t *string, unsigned long long *number); // TESTED
int extract_ul(stringer_t *string, unsigned long *number); // TESTED
int extract_ui(stringer_t *string, unsigned int *number); // TESTED
int extract_us(stringer_t *string, unsigned short *number); // TESTED
int extract_ll(stringer_t *string, long long *number); // TESTED
int extract_l(stringer_t *string, long *number); // TESTED
int extract_i(stringer_t *string, int *number); // TESTED
int extract_s(stringer_t *string, short int *number); // TESTED
int extract_ull_ns(char *data, sizer_t length, unsigned long long *number); // TESTED
int extract_ul_ns(char *data, sizer_t length, unsigned long *number); // TESTED
int extract_ui_ns(char *data, sizer_t length, unsigned int *number); // TESTED
int extract_us_ns(char *data, sizer_t length, unsigned short *number); // TESTED
int extract_ll_ns(char *data, sizer_t length, long long *number); // TESTED
int extract_l_ns(char *data, sizer_t length, long *number); // TESTED
int extract_i_ns(char *data, sizer_t length, int *number); // TESTED
int extract_s_ns(char *data, sizer_t length, short int *number); // TESTED

// Extracting database values.
extern inline stringer_t * extract_string(char *string); // DONE
extern inline long long extract_number(char *string); // DONE
extern inline unsigned long long extract_unsigned_number(char *string); // DONE
/*
// Create strings from numbers. Allocate the stringer dynamically.
stringer_t * create_ull(unsigned long long number);
stringer_t * create_ul(unsigned long number);
stringer_t * create_ui(unsigned int number);
stringer_t * create_us(unsigned short number);
stringer_t * create_ll(long long number);
stringer_t * create_l(long number);
stringer_t * create_i(int number);
stringer_t * create_s(short number);
*/
// Determine the length of a number.
extern inline short length_ull(unsigned long long number); // TESTED
extern inline short length_ul(unsigned long long number); // TESTED
extern inline short length_ui(unsigned int number); // TESTED
extern inline short length_us(unsigned short number); // TESTED
extern inline short length_ll(long long number); // TESTED
extern inline short length_l(long number); // TESTED
extern inline short length_i(int number); // TESTED
extern inline short length_s(short number); // TESTED

// Tokenize functions.
stringer_t * get_token(const stringer_t *string, const char token, const unsigned int number); // TESTED
stringer_t * get_token_ns(const char *string, sizer_t length, const char token, const unsigned int number); // TESTED

// Network functions.
int session_read(session_common_t *session); // DONE
int session_readline(session_common_t *session); // DONE
int session_write(session_common_t *session, const stringer_t *string); // DONE
int session_write_ns(session_common_t *session, const char *string, sizer_t length); // DONE
int session_printf(session_common_t *session, const char *format, ...) __attribute__((format(printf, 2, 3))); // DONE
int network_read(int socket_descriptor, char *buffer, sizer_t length); // DONE
int network_write(int socket_descriptor, const stringer_t *string); // DONE
int network_write_ns(int socket_descriptor, const char *buffer, sizer_t length); // DONE
int network_readline(int socket_descriptor, char *buffer, sizer_t buffer_length, sizer_t *line_length, sizer_t *buffered_bytes); // DONE
int network_printf(int socket_descriptor, char *buffer, sizer_t buffer_length, const char *format, ...) __attribute__((format(printf, 4, 5))); // DONE

// Hashing functions.
int hash_sha512_password(hashed_password_t *hashed, stringer_t *username, stringer_t *password); // DONE

// Random string functions.
unsigned short random_us(void); // DONE
stringer_t * random_st_choices(sizer_t length, char *choices, sizer_t choices_len); // DONE

// Functions for handling XML documents.
int load_symbols_xml(void); // TESTED
int initialize_xml(void); // TESTED
void free_xml(void); // TESTED
stringer_t * version_xml(void); // DONE

// Generic XML functions
void xml_error(void *ctx, const char *format, ...); // TESTED

// Handle the XML parsers.
void xml_free_parser_ctx(xmlParserCtxtPtr ctx); // TESTED
xmlParserCtxtPtr xml_create_parser_ctx(void); // TESTED

// Handle XPATH.
void xml_free_xpath_ctx(xmlXPathContextPtr ctx); // TESTED
void xml_free_xpath_obj(xmlXPathObjectPtr obj); // TESTED
xmlXPathContextPtr xml_create_xpath_ctx(xmlDocPtr ctx); // TESTED
xmlXPathObjectPtr xml_xpath_eval(const char *xpath, xmlXPathContextPtr ctx); // TESTED

// Handle the XML document object.
void xml_free_doc(xmlDocPtr doc); // TESTED
xmlDocPtr xml_create_doc(xmlParserCtxtPtr ctx, const char * buffer, int size, const char * url, const char * encoding, int options); // TESTED

// Functions for handling XPATH queries.
int xml_get_xpath_i(xmlXPathContextPtr xpath_ctx, xmlChar *xpath_query); // TESTED
long xml_get_xpath_l(xmlXPathContextPtr xpath_ctx, xmlChar *xpath_query); // TESTED
short xml_get_xpath_s(xmlXPathContextPtr xpath_ctx, xmlChar *xpath_query); // TESTED
long long xml_get_xpath_ll(xmlXPathContextPtr xpath_ctx, xmlChar *xpath_query); // TESTED
unsigned int xml_get_xpath_ui(xmlXPathContextPtr xpath_ctx, xmlChar *xpath_query); // TESTED
unsigned long xml_get_xpath_ul(xmlXPathContextPtr xpath_ctx, xmlChar *xpath_query); // TESTED
unsigned short int xml_get_xpath_us(xmlXPathContextPtr xpath_ctx, xmlChar *xpath_query); // TESTED
unsigned long long xml_get_xpath_ull(xmlXPathContextPtr xpath_ctx, xmlChar *xpath_query); // TESTED
char * xml_get_xpath_ns(xmlXPathContextPtr xpath_ctx, xmlChar *xpath_query); // TESTED
stringer_t * xml_get_xpath_st(xmlXPathContextPtr xpath_ctx, xmlChar *xpath_query); // TESTED
sizer_t xml_get_xpath_node_count(xmlXPathContextPtr xpath_ctx, xmlChar *xpath_query); // TESTED

// Functions for handling IP addresses.
short int get_octet_v4_ns(const char *string, const unsigned short octet); // DONE
short int get_octet_v4(const stringer_t *string, const unsigned short octet); // TESTED

// Dynamically load the MySQL symbols.
int load_symbols_dataface(void); // TESTED

// Functions for managing the connection pool.
void free_dataface(void); // TESTED
stringer_t * version_dataface(void); // DONE
void free_dataface_thread(void); // DONE
int initialize_dataface(void); // TESTED
int initialize_dataface_thread(void); // DONE

// Functions for managing connections.
int db_get_conn(void); // DONE
void db_free_conn(int connection); // DONE

// Functions for managing transactions.
int start_tran(void); // TESTED
int send_begin(int connection); // DONE
int send_commit(int connection); // DONE
int send_rollback(int connection); // DONE
int commit_tran(int connection); // TESTED
void rollback_tran(int connection); // DONE
extern inline MYSQL * connection(int connection); // DONE

// Functions for handling SQL result sets.
extern inline void free_sql(sql_result_t *sql); // DONE
sql_result_t * store_db_result(MYSQL_STMT *stmt); // DONE
extern inline unsigned field_count_sql(sql_result_t *sql); // DONE
extern inline sql_row_t * fetch_row_sql(sql_result_t *sql); // DONE
extern inline int get_i_sql(sql_row_t *row, unsigned field); // DONE
extern inline void set_buf_sql(sql_row_t *sql, char *buffer); // DONE
extern inline long get_l_sql(sql_row_t *row, unsigned field); // DONE
extern inline char get_c_sql(sql_row_t *row, unsigned field); // DONE
extern inline float get_f_sql(sql_row_t *row, unsigned field); // DONE
extern inline short get_s_sql(sql_row_t *row, unsigned field); // DONE
extern inline double get_d_sql(sql_row_t *row, unsigned field); // DONE
extern inline unsigned long long row_count_sql(sql_result_t *sql); // DONE
extern inline unsigned get_ui_sql(sql_row_t *row, unsigned field); // DONE
extern inline sizer_t get_len_sql(sql_row_t *row, unsigned field); // DONE
extern inline char * get_char_sql(sql_row_t *row, unsigned field); // DONE
extern inline long long get_ll_sql(sql_row_t *row, unsigned field); // DONE
unsigned generate_bind_result(MYSQL_STMT *stmt, MYSQL_BIND **result); // DONE
sql_result_t * allocate_sql(unsigned long long rows, unsigned fields); // DONE
extern inline stringer_t * get_st_sql(sql_row_t *row, unsigned field); //DONE
extern inline unsigned long get_ul_sql(sql_row_t *row, unsigned field); // DONE
extern inline unsigned char get_uc_sql(sql_row_t *row, unsigned field); // DONE
extern inline unsigned short get_us_sql(sql_row_t *row, unsigned field); // DONE
extern inline void free_bind_result(unsigned number, MYSQL_BIND *result); // DONE
extern inline void set_row_sql(sql_result_t *sql, unsigned long long row); // DONE
extern inline unsigned long long get_ull_sql(sql_row_t *row, unsigned field); // DONE
extern inline sql_row_t * get_row_sql(sql_result_t *sql, unsigned long long row); // DONE
int store_sql_row(unsigned long long row, sql_result_t *result, MYSQL_BIND *output); // DONE

// Functions for handling prepared statements.
extern inline void free_stmt(MYSQL_STMT **holder); // DONE
extern inline MYSQL_STMT ** setup_stmt(char *sql); // DONE
extern inline int reset_stmt(MYSQL_STMT *stmt); // DONE
extern inline void close_stmt(MYSQL_STMT *stmt); // DONE
extern inline MYSQL_STMT * init_stmt(MYSQL *mysql); // DONE
extern inline int bind_param_stmt(MYSQL_STMT *stmt, MYSQL_BIND *bind); // DONE
extern inline int prepare_stmt(MYSQL_STMT *stmt, const char *query, unsigned long length); // DONE

// Functions for executing queries via the connection pool.
int exec_query(const stringer_t *query); // DONE
int exec_query_tran(const stringer_t *query, const int transaction); // DONE
int exec_query_ns(const char *query, const sizer_t length); // TESTED
int exec_query_tran_ns(const char *query, const sizer_t length, const int transaction); // TESTED
int exec_query_stmt(MYSQL_STMT **stmt, MYSQL_BIND *parameters); // DONE
int exec_query_stmt_tran(MYSQL_STMT **stmt, MYSQL_BIND *parameters, const int transaction);// DONE

// Function to execute a query and return how many rows the query produced.
unsigned long long exec_query_rows(const stringer_t *query); // DONE
unsigned long long exec_query_rows_trans(const stringer_t *query, const int transaction); // DONE
unsigned long long exec_query_rows_ns(const char *query, const sizer_t length); // TESTED
unsigned long long exec_query_rows_trans_ns(const char *query, const sizer_t length, const int transaction); // DONE
unsigned long long exec_query_rows_stmt(MYSQL_STMT **stmt, MYSQL_BIND *parameters); // DONE
unsigned long long exec_query_rows_stmt_tran(MYSQL_STMT **stmt, MYSQL_BIND *parameters, const int transaction); // DONE

// Functions for executing queries, and returing the results.
extern void free_res(MYSQL_RES *result); // TESTED
extern MYSQL_ROW get_row(MYSQL_RES *result); // TESTED
MYSQL_RES * exec_query_res(const stringer_t *query); // DONE
MYSQL_RES * exec_query_res_tran(const stringer_t *query, const int transaction); // DONE
MYSQL_RES * exec_query_res_ns(const char *query, const sizer_t length); // TESTED
MYSQL_RES * exec_query_res_tran_ns(const char *query, const sizer_t length, const int transaction); // DONE
sql_result_t * exec_query_res_stmt(MYSQL_STMT **stmt, MYSQL_BIND *parameters);
sql_result_t * exec_query_res_tran_stmt(MYSQL_STMT **stmt, MYSQL_BIND *parameters, const int transaction);

// Execute an insert statement, and return the identifier generated.
unsigned long long exec_insert(const stringer_t *query); // DONE
unsigned long long exec_insert_tran(const stringer_t *query, const int transaction); // DONE
unsigned long long exec_insert_ns(const char *query, const sizer_t length); // DONE
unsigned long long exec_insert_tran_ns(const char *query, const sizer_t length, const int transaction); // DONE
unsigned long long exec_insert_stmt(MYSQL_STMT **stmt, MYSQL_BIND *parameters); // DONE
unsigned long long exec_insert_stmt_tran(MYSQL_STMT **stmt, MYSQL_BIND *parameters, const int transaction); // DONE

// Execute a delete or update statement, and return the number of rows effected.
unsigned long long exec_write(const stringer_t *query); // DONE
unsigned long long exec_write_tran(const stringer_t *query, const int transaction); // DONE
unsigned long long exec_write_ns(const char *query, const sizer_t length); // DONE
unsigned long long exec_write_tran_ns(const char *query, const sizer_t length, const int transaction); // DONE
unsigned long long exec_write_stmt(MYSQL_STMT **stmt, MYSQL_BIND *parameters);
unsigned long long exec_write_stmt_tran(MYSQL_STMT **stmt, MYSQL_BIND *parameters, const int transaction);

// Functions for making query safe.
stringer_t * sql_safe_st(const stringer_t *string); // DONE
stringer_t * sql_safe_ns(const char *string); // DONE
stringer_t * sql_safe_ns_amt(const char *string, const sizer_t amount); // DONE

// The SSL library.
void free_ssl(void); // DONE
int initialize_ssl(void); // DONE
int load_symbols_ssl(void); // DONE
extern inline int reseed_ssl(void); // DONE
stringer_t * version_ssl(void); // DONE
int initialize_ssl_server(server_config_t *server); // DONE

// For reading and writing data via an SSL connection.
extern inline void ssl_free(SSL *ssl); // DONE
extern inline int ssl_accept(SSL *ssl); // DONE
extern inline SSL * ssl_new(SSL_CTX *ctx); // DONE
extern inline int ssl_read(SSL *ssl, void *buffer, int length); // DONE
extern inline void ssl_set_bio(SSL *ssl, BIO *rbio, BIO *wbio); // DONE
extern inline BIO * ssl_bio_new_socket(int sock, int close_flag); // DONE
extern inline int ssl_write(SSL *ssl, const void *buffer, int length); // DONE

// SSL connections.
extern inline int ssl_shutdown(SSL *ssl); // DONE
extern inline void ssl_ctx_free(SSL_CTX *ctx); // DONE

// Functions for interacting with lavacache.
void free_lavacache(void);
int initialize_lavacache(void);

// SPF.
int load_symbols_spf(void);

// ClamAV
void free_virus(void); // DONE
int refresh_virusdb(void); // DONE
int initialize_virus(void); // DONE
int load_symbols_virus(void); // DONE
const char * version_virus(void); // DONE

// LZO
char * version_lzo(void); // TESTED
int initialize_lzo(void); // TESTED
int load_symbols_lzo(void); // TESTED

reducer_t * allocate_rt(sizer_t size); // TESTED
stringer_t * compress_lzo(stringer_t *input); // TESTED
extern inline void free_rt(reducer_t *string); // TESTED
stringer_t * decompress_lzo(stringer_t *input); // TESTED
extern inline sizer_t size_rt(reducer_t *string); // TESTED
extern inline sizer_t used_rt(reducer_t *string); // TESTED
extern inline lzo_uint in_size_rt(reducer_t *string); // TESTED
extern inline sizer_t size_buf_rt(reducer_t *buffer); // TESTED
extern inline lzo_uint out_size_rt(reducer_t *string); // TESTED
extern inline lzo_uint32 in_check_rt(reducer_t *string); // TESTED
extern inline lzo_uint32 out_check_rt(reducer_t *string); // TESTED
extern inline unsigned char * data_buf_rt(reducer_t *buffer); // TESTED
extern inline void set_used_rt(reducer_t *buffer, sizer_t used); // TESTED
extern inline void set_in_size_rt(reducer_t *string, lzo_uint size); // TESTED
extern inline void set_out_size_rt(reducer_t *string, lzo_uint size); // TESTED
extern inline void set_in_check_rt(reducer_t *string, lzo_uint32 check); // TESTED
extern inline void set_out_check_rt(reducer_t *string, lzo_uint32 check); // TESTED

#endif
